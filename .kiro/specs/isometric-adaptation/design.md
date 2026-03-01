# Design Document: Isometric Game Adaptation

## Overview

Адаптація гри "Punic Wars: Castra" для повної інтеграції з ізометричною системою координат. Всі ігрові об'єкти (юніти, будівлі, ресурси) будуть переведені з екранних координат на grid-based систему з ізометричним рендерингом.

## Architecture

### Current System (Before)
```
Unit/Building/Resource
  ├─ float x, y (screen coordinates)
  ├─ render() → DrawCircle/DrawRectangle at (x, y)
  └─ movement → direct x, y manipulation
```

### New System (After)
```
Unit/Building/Resource
  ├─ GridCoords position (row, col)
  ├─ render() → Convert to screen coords → Draw isometric sprite/debug shape
  ├─ movement → grid-based pathfinding
  └─ IsometricSprite (optional, fallback to debug)
```

## Components and Interfaces

### 1. Coordinate System Integration

**Existing (from tilemap):**
```cpp
struct GridCoords {
    int row;
    int col;
};

struct ScreenCoords {
    float x;
    float y;
};

class CoordinateConverter {
    static ScreenCoords gridToScreen(GridCoords grid);
    static GridCoords screenToGrid(ScreenCoords screen);
};
```

**Usage in game objects:**
```cpp
// Unit example
class Unit {
    GridCoords position;  // NEW: grid position
    
    ScreenCoords getScreenPosition() const {
        return CoordinateConverter::gridToScreen(position);
    }
};
```


### 2. Isometric Sprite System

**New class: IsometricSprite**
```cpp
class IsometricSprite {
private:
    Texture2D texture;
    bool loaded;
    Vector2 anchorPoint;  // Bottom center
    
public:
    IsometricSprite();
    bool loadFromFile(const char* filepath);
    void unload();
    bool isLoaded() const;
    
    void draw(ScreenCoords position, Color tint = WHITE);
    void drawDebug(ScreenCoords position, Color color, const char* label);
};
```

**Debug rendering functions:**
```cpp
// For units - diamond shape
void drawDebugUnit(ScreenCoords pos, Color color, const char* type);

// For buildings - isometric rectangle
void drawDebugBuilding(ScreenCoords pos, int width, int height, Color color, const char* type);

// For resources - square with letter
void drawDebugResource(ScreenCoords pos, Color color, char letter);
```

### 3. Updated Unit Class

**Modified unit.h:**
```cpp
class Unit {
private:
    GridCoords position;           // NEW: grid coordinates
    IsometricSprite sprite;        // NEW: sprite system
    bool useDebugRendering;        // NEW: fallback flag
    
    // Keep for smooth interpolation
    float interpolation_x, interpolation_y;
    
public:
    // Constructor loads sprite, falls back to debug if not found
    Unit(UnitType type, Faction faction, GridCoords startPos);
    
    // Movement now works with grid
    void moveTo(GridCoords target, const TileMap& map);
    void update(float deltaTime);
    
    // Rendering
    void render() const;
    ScreenCoords getScreenPosition() const;
    GridCoords getGridPosition() const;
};
```

### 4. Updated Building Class

**Modified building.h:**
```cpp
class Building {
private:
    GridCoords position;           // NEW: grid coordinates
    GridCoords footprint;          // NEW: size in tiles (e.g. 2x2, 3x3)
    IsometricSprite sprite;        // NEW: sprite system
    bool useDebugRendering;        // NEW: fallback flag
    
public:
    Building(BuildingType type, Faction faction, GridCoords pos);
    
    void render() const;
    Rectangle getGridRect() const;  // Returns grid-space rectangle
    bool occupiesGridCell(GridCoords cell) const;
};
```


### 5. Depth Sorting System

**New class: RenderQueue**
```cpp
struct RenderableObject {
    enum Type { UNIT, BUILDING, RESOURCE };
    Type type;
    int index;  // Index in respective vector
    int sortKey;  // row + col for sorting
};

class RenderQueue {
private:
    std::vector<RenderableObject> queue;
    
public:
    void clear();
    void addUnit(int index, GridCoords pos);
    void addBuilding(int index, GridCoords pos);
    void addResource(int index, GridCoords pos);
    
    void sort();  // Sort by sortKey (back to front)
    void render(const std::vector<Unit>& units,
                const std::vector<Building>& buildings,
                const std::vector<ResourcePoint>& resources);
};
```

**Usage in DrawGame():**
```cpp
void DrawGame() {
    // 1. Render tilemap (background)
    mapRenderer->render(*gameMap);
    
    // 2. Build render queue
    RenderQueue renderQueue;
    for (int i = 0; i < units.size(); i++) {
        renderQueue.addUnit(i, units[i].getGridPosition());
    }
    for (int i = 0; i < buildings.size(); i++) {
        renderQueue.addBuilding(i, buildings[i].getGridPosition());
    }
    for (int i = 0; i < resources.size(); i++) {
        renderQueue.addResource(i, resources[i].getGridPosition());
    }
    
    // 3. Sort and render
    renderQueue.sort();
    renderQueue.render(units, buildings, resources);
    
    // 4. Render UI (always on top)
    DrawUI();
}
```

### 6. Mouse Interaction System

**Updated click handling:**
```cpp
void HandleClicks() {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();
        
        // Convert to grid coordinates
        ScreenCoords screenPos = {mousePos.x, mousePos.y};
        GridCoords gridPos = CoordinateConverter::screenToGrid(screenPos);
        
        // Check for objects at this grid position
        selectedUnitIndex = findUnitAtGrid(gridPos);
        if (selectedUnitIndex < 0) {
            selectedBuildingIndex = findBuildingAtGrid(gridPos);
        }
    }
}

int findUnitAtGrid(GridCoords pos) {
    for (int i = 0; i < units.size(); i++) {
        if (units[i].getGridPosition() == pos) {
            return i;
        }
    }
    return -1;
}
```


## Data Models

### GridCoords (existing)
```cpp
struct GridCoords {
    int row;
    int col;
    
    bool operator==(const GridCoords& other) const {
        return row == other.row && col == other.col;
    }
};
```

### IsometricSprite
```cpp
class IsometricSprite {
    Texture2D texture;
    bool loaded;
    Vector2 anchorPoint;
    int width, height;
    
    // Methods defined above
};
```

### Building Footprints
```cpp
struct BuildingFootprint {
    int rows;
    int cols;
    
    static BuildingFootprint getFootprint(BuildingType type) {
        switch(type) {
            case HQ_ROME:
            case HQ_CARTHAGE:
                return {3, 3};
            case BARRACKS_ROME:
            case BARRACKS_CARTHAGE:
            case QUESTORIUM_ROME:
                return {2, 2};
            default:
                return {1, 1};
        }
    }
};
```

## Error Handling

### Sprite Loading Failures
```cpp
IsometricSprite::IsometricSprite() {
    loaded = false;
    texture.id = 0;
}

bool IsometricSprite::loadFromFile(const char* filepath) {
    if (FileExists(filepath)) {
        texture = LoadTexture(filepath);
        if (texture.id > 0) {
            loaded = true;
            TraceLog(LOG_INFO, "[SPRITE] Loaded: %s", filepath);
            return true;
        }
    }
    TraceLog(LOG_WARNING, "[SPRITE] Not found, using debug mode: %s", filepath);
    loaded = false;
    return false;
}
```

### Grid Boundary Checks
```cpp
bool TileMap::isValidCoord(int row, int col) const {
    return row >= 0 && row < height && col >= 0 && col < width;
}

// Use in movement
void Unit::moveTo(GridCoords target, const TileMap& map) {
    if (map.isValidCoord(target.row, target.col) && 
        map.isPassable(target.row, target.col)) {
        position = target;
    }
}
```

### Collision Detection
```cpp
bool Building::occupiesGridCell(GridCoords cell) const {
    return cell.row >= position.row && 
           cell.row < position.row + footprint.rows &&
           cell.col >= position.col && 
           cell.col < position.col + footprint.cols;
}
```


## Testing Strategy

### Unit Tests
1. **Coordinate conversion tests**
   - Test gridToScreen and screenToGrid are inverses
   - Test boundary cases (0,0), (49,49)
   - Test negative coordinates

2. **Sprite loading tests**
   - Test successful load
   - Test fallback to debug mode
   - Test unload and reload

3. **Collision detection tests**
   - Test unit-building collision
   - Test building footprint overlap
   - Test grid boundary checks

4. **Depth sorting tests**
   - Test objects sort correctly by row+col
   - Test multi-tile buildings sort correctly

### Integration Tests
1. **Click-to-select test**
   - Click on unit → unit selected
   - Click on building → building selected
   - Click on empty tile → nothing selected

2. **Movement test**
   - Right-click → unit moves to grid position
   - Unit follows grid-aligned path
   - Unit stops at obstacles

3. **Rendering test**
   - All objects render in correct depth order
   - Debug mode works when sprites missing
   - Sprites render at correct positions

### Manual Testing Checklist
- [ ] Units render as diamonds in debug mode
- [ ] Buildings render as rectangles in debug mode
- [ ] Sprites load and display correctly
- [ ] Objects sort correctly (no z-fighting)
- [ ] Mouse clicks select correct objects
- [ ] Units move smoothly in grid space
- [ ] Buildings snap to grid when placed
- [ ] Camera panning works correctly
- [ ] Zoom works at all levels (0.3x - 2.0x)
- [ ] Game runs at 60 FPS with 100+ objects

## Implementation Plan

### Phase 1: Core Coordinate System (2-3 hours)
1. Update Unit class to use GridCoords
2. Update Building class to use GridCoords
3. Update ResourcePoint class to use GridCoords
4. Update all movement code to use grid coordinates
5. Test basic functionality

### Phase 2: Debug Rendering (1-2 hours)
1. Implement drawDebugUnit (diamond)
2. Implement drawDebugBuilding (rectangle)
3. Implement drawDebugResource (square)
4. Update render() methods to use debug shapes
5. Test visual appearance

### Phase 3: Sprite System (2-3 hours)
1. Create IsometricSprite class
2. Implement sprite loading with fallback
3. Update Unit to use IsometricSprite
4. Update Building to use IsometricSprite
5. Test sprite loading and rendering

### Phase 4: Depth Sorting (1-2 hours)
1. Create RenderQueue class
2. Implement sorting algorithm
3. Update DrawGame() to use RenderQueue
4. Test depth sorting with multiple objects

### Phase 5: Mouse Interaction (1-2 hours)
1. Update click handling to use grid coordinates
2. Implement grid-based selection
3. Update movement commands to use grid
4. Test clicking and commanding units

### Phase 6: Polish and Testing (2-3 hours)
1. Create sprite guide documentation
2. Test all features thoroughly
3. Fix bugs and edge cases
4. Optimize performance if needed

**Total estimated time: 9-15 hours**

