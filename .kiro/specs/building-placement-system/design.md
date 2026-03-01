# Design Document: Building Placement System

## Overview

Система розміщення будівель забезпечує правильне розташування будівель на ізометричній тайловій сітці, блокування тайлів для патфайндингу, автоматичний спавн головних наметів фракцій, панель замовлення юнітів та коректне відображення ресурсів. Система інтегрується з існуючими компонентами: `CoordinateConverter`, `IsometricRenderer`, `PathfindingManager`, `Building`, `TileMap`.

## Architecture

### Existing Components (Already Implemented)

1. **CoordinateConverter** - конвертація між тайловими та екранними координатами
2. **IsometricRenderer** - рендеринг ізометричної мапи з viewport culling
3. **PathfindingManager** - система пошуку шляху з сіткою блокування
4. **Building** - структура будівлі з виробництвом юнітів
5. **TileMap** - тайлова карта з типами terrain

### New/Modified Components

1. **BuildingPlacer** - розміщення будівель на тайловій сітці
2. **FactionSpawner** - спавн головних наметів фракцій
3. **UnitOrderPanel** - UI панель замовлення юнітів
4. **ResourceDisplay** - відображення ресурсів на панелі
5. **ViewportCulling** - оптимізоване відображення мапи

## Components and Interfaces

### 1. BuildingPlacer

Відповідає за розміщення будівель на тайловій сітці та інтеграцію з патфайндингом.

```cpp
class BuildingPlacer {
private:
    PathfindingManager* pathfinding;
    TileMap* map;
    
public:
    // Ініціалізація
    void init(PathfindingManager* pf, TileMap* m);
    
    // Розміщення будівлі на тайлі
    bool placeBuilding(Building& building, int row, int col);
    
    // Видалення будівлі
    void removeBuilding(const Building& building);
    
    // Перевірка чи тайл вільний
    bool isTileFree(int row, int col) const;
    
    // Отримати тайлові координати будівлі
    GridCoords getBuildingTileCoords(const Building& building) const;
};
```

**Логіка розміщення:**
1. Перевірити чи тайл прохідний (`map->isPassable(row, col)`)
2. Перевірити чи тайл не зайнятий іншою будівлею
3. Конвертувати тайлові координати в екранні (`CoordinateConverter::gridToScreenWithOffset`)
4. Встановити позицію будівлі
5. Заблокувати тайл в pathfinding (`pathfinding->blockTile(row, col)`)

### 2. FactionSpawner

Відповідає за спавн головних наметів фракцій на випадкових позиціях.

```cpp
class FactionSpawner {
private:
    BuildingPlacer* placer;
    TileMap* map;
    std::vector<Building>* buildings;
    
public:
    // Ініціалізація
    void init(BuildingPlacer* bp, TileMap* m, std::vector<Building>* b);
    
    // Спавн HQ обох фракцій
    void spawnFactionHQs();
    
private:
    // Знайти випадковий вільний тайл
    GridCoords findRandomFreeTile() const;
    
    // Перевірити мінімальну відстань між HQ
    bool isMinDistanceSatisfied(const GridCoords& pos1, const GridCoords& pos2, int minDist) const;
    
    // Створити HQ для фракції
    void createHQ(Faction faction, const GridCoords& position);
};
```

**Логіка спавну:**
1. Знайти випадковий прохідний тайл для Rome HQ
2. Знайти випадковий прохідний тайл для Carthage HQ (мінімум 20 тайлів від Rome HQ)
3. Створити будівлі типу `HQ_ROME` та `HQ_CARTHAGE`
4. Розмістити їх через `BuildingPlacer::placeBuilding`

### 3. UnitOrderPanel

UI панель для замовлення юнітів з головного намету.

```cpp
class UnitOrderPanel {
private:
    int selectedBuildingIndex;
    std::vector<Building>* buildings;
    Faction playerFaction;
    
    // UI елементи
    Rectangle panelRect;
    Rectangle workerButton;
    
public:
    // Ініціалізація
    void init(std::vector<Building>* b, Faction faction);
    
    // Встановити вибрану будівлю
    void setSelectedBuilding(int index);
    
    // Малювання панелі
    void draw() const;
    
    // Обробка кліків
    void handleClick(Vector2 mousePos);
    
    // Чи панель видима
    bool isVisible() const;
    
private:
    // Малювання кнопки Worker
    void drawWorkerButton() const;
    
    // Перевірка чи можна дозволити Worker
    bool canAffordWorker() const;
};
```

**Логіка відображення:**
1. Панель відображається тільки коли вибрано HQ гравця
2. Панель розташована знизу екрану (y = 950, width = 300, height = 100)
3. Кнопка Worker показує вартість (їжа: 10, гроші: 20)
4. Кнопка активна тільки якщо є достатньо ресурсів
5. Клік на кнопку запускає виробництво через `Building::startProduction`

### 4. ResourceDisplay

Відображення ресурсів на верхній панелі.

```cpp
class ResourceDisplay {
private:
    Texture2D resourcePanel;
    Faction playerFaction;
    
    // Позиції тексту на панелі
    Vector2 foodTextPos;
    Vector2 moneyTextPos;
    
public:
    // Ініціалізація
    void init(Texture2D panel, Faction faction);
    
    // Малювання ресурсів
    void draw(int food, int money, int foodReserved, int moneyReserved) const;
    
private:
    // Розрахувати позиції тексту
    void calculateTextPositions();
};
```

**Логіка відображення:**
1. Панель малюється зверху по центру екрану
2. Їжа відображається зліва (x = panelX + 250, y = 50)
3. Гроші відображаються справа (x = panelX + 750, y = 50)
4. Відображаються доступні ресурси (total - reserved)
5. Загальна кількість юнітів НЕ відображається

### 5. ViewportCulling (Modification to IsometricRenderer)

Оптимізоване відображення тайлів у в'юпорті.

**Існуючий метод `calculateVisibleTiles` потребує виправлення:**

```cpp
void IsometricRenderer::calculateVisibleTiles(const TileMap& map) {
    // Отримати межі екрану в світових координатах
    Vector2 screenTopLeft = GetScreenToWorld2D({0, 0}, camera);
    Vector2 screenBottomRight = GetScreenToWorld2D(
        {GetScreenWidth(), GetScreenHeight()}, camera
    );
    
    // Додати culling margin (2 тайли)
    const int CULLING_MARGIN = 2;
    
    // Конвертувати в тайлові координати
    GridCoords topLeft = CoordinateConverter::screenToGrid(
        {screenTopLeft.x, screenTopLeft.y}
    );
    GridCoords bottomRight = CoordinateConverter::screenToGrid(
        {screenBottomRight.x, screenBottomRight.y}
    );
    
    // Застосувати margin та обмеження
    visible_start.row = std::max(0, topLeft.row - CULLING_MARGIN);
    visible_start.col = std::max(0, topLeft.col - CULLING_MARGIN);
    visible_end.row = std::min(map.getHeight() - 1, bottomRight.row + CULLING_MARGIN);
    visible_end.col = std::min(map.getWidth() - 1, bottomRight.col + CULLING_MARGIN);
}
```

**Логіка рендерингу:**
1. Розрахувати видимі тайли на початку кадру
2. Малювати тільки тайли в межах `visible_start` до `visible_end`
3. Додати margin 2 тайли за межами екрану
4. Оновлювати при русі камери або зміні зуму

## Data Models

### BuildingTileData

```cpp
struct BuildingTileData {
    int row;
    int col;
    BuildingType type;
    Faction faction;
};
```

### SpawnConfiguration

```cpp
struct SpawnConfiguration {
    int minDistanceBetweenHQs = 20;  // Мінімальна відстань між HQ
    int maxSpawnAttempts = 100;      // Максимум спроб знайти позицію
};
```

## Correctness Properties

*A property is a characteristic or behavior that should hold true across all valid executions of a system-essentially, a formal statement about what the system should do. Properties serve as the bridge between human-readable specifications and machine-verifiable correctness guarantees.*

### Property 1: Building placement on single tile

*For any* building and any valid tile coordinates (row, col), when the building is placed using `BuildingPlacer::placeBuilding`, the building's screen position should correspond exactly to the tile's screen coordinates calculated by `CoordinateConverter::gridToScreenWithOffset`.

**Validates: Requirements 1.1, 1.2, 1.4**

### Property 2: Tile blocking consistency

*For any* building placed on tile (row, col), the pathfinding grid should mark that tile as non-walkable, and when the building is removed, the tile should become walkable again.

**Validates: Requirements 2.1, 2.2**

### Property 3: Pathfinding avoids blocked tiles

*For any* path calculated by PathfindingManager, the path should not contain any tiles that are blocked by buildings.

**Validates: Requirements 2.3**

### Property 4: HQ minimum distance

*For any* two faction HQs spawned by FactionSpawner, the Manhattan distance between their tile coordinates should be at least 20 tiles.

**Validates: Requirements 3.3**

### Property 5: HQ spawns on passable tiles

*For any* HQ spawned by FactionSpawner, the tile at the HQ's position should be passable according to the TileMap.

**Validates: Requirements 3.1, 3.2**

### Property 6: Unit order panel visibility

*For any* selected building, the UnitOrderPanel should be visible if and only if the building is an HQ of the player's faction.

**Validates: Requirements 4.1, 4.5**

### Property 7: Worker button availability

*For any* state where the UnitOrderPanel is visible, the Worker button should be enabled if and only if the player has sufficient resources (food >= 10 and money >= 20).

**Validates: Requirements 4.3**

### Property 8: Resource display accuracy

*For any* game state, the ResourceDisplay should show available resources equal to (total resources - reserved resources) for both food and money.

**Validates: Requirements 5.3, 5.4**

### Property 9: Resource display positioning

*For any* rendered frame, the food value should be displayed at position (panelX + 250, 50) and money value at position (panelX + 750, 50), where panelX is the centered panel position.

**Validates: Requirements 5.1, 5.2**

### Property 10: Viewport culling completeness

*For any* camera position and zoom level, all tiles that are visible on screen should be included in the range [visible_start, visible_end] calculated by `calculateVisibleTiles`.

**Validates: Requirements 6.1, 6.2, 6.4**

### Property 11: Culling margin coverage

*For any* calculated visible tile range, the range should extend at least 2 tiles beyond the screen boundaries in all directions (or to map edge).

**Validates: Requirements 6.3**

### Property 12: No fixed tile limit

*For any* map size and camera configuration, the number of rendered tiles should be determined by viewport culling, not by a fixed limit (e.g., not limited to 12 tiles).

**Validates: Requirements 6.5**

## Error Handling

### BuildingPlacer Errors

1. **Tile out of bounds**: Return false, log warning
2. **Tile not passable**: Return false, log warning
3. **Tile already occupied**: Return false, log warning
4. **Pathfinding not initialized**: Return false, log error

### FactionSpawner Errors

1. **No free tiles found**: Log error, use fallback positions (0,0) and (49,49)
2. **Cannot satisfy minimum distance**: Reduce minimum distance by 50%, retry
3. **Max spawn attempts exceeded**: Log error, use best found positions

### UnitOrderPanel Errors

1. **Invalid building index**: Hide panel, log warning
2. **Building not HQ**: Hide panel (expected behavior)
3. **Insufficient resources**: Disable button, show cost in red

### ResourceDisplay Errors

1. **Panel texture not loaded**: Use fallback text rendering
2. **Negative resources**: Clamp to 0, log warning

## Testing Strategy

### Unit Tests

1. **CoordinateConverter round-trip**: Test grid→screen→grid conversion
2. **BuildingPlacer tile blocking**: Test that placed buildings block tiles
3. **FactionSpawner distance**: Test minimum distance between HQs
4. **UnitOrderPanel visibility**: Test panel shows only for player HQ
5. **ResourceDisplay calculation**: Test available = total - reserved

### Property-Based Tests

Property-based tests will use a C++ property testing library (e.g., RapidCheck or custom implementation with random generation).

**Test Configuration:**
- Minimum 100 iterations per property test
- Each test tagged with feature name and property number
- Tests integrated into existing test suite

**Property Test 1: Building placement consistency**
- Generate random valid tile coordinates
- Place building, verify screen position matches expected
- Tag: `Feature: building-placement-system, Property 1`

**Property Test 2: Tile blocking round-trip**
- Generate random buildings and positions
- Place building, verify tile blocked
- Remove building, verify tile unblocked
- Tag: `Feature: building-placement-system, Property 2`

**Property Test 3: Pathfinding avoidance**
- Generate random map with buildings
- Calculate paths, verify no blocked tiles in path
- Tag: `Feature: building-placement-system, Property 3`

**Property Test 4: HQ distance constraint**
- Generate multiple HQ spawn configurations
- Verify distance >= 20 tiles
- Tag: `Feature: building-placement-system, Property 4`

**Property Test 5: Resource display accuracy**
- Generate random resource states
- Verify displayed = total - reserved
- Tag: `Feature: building-placement-system, Property 8`

### Integration Tests

1. **Full spawn sequence**: Test complete faction HQ spawning
2. **Building placement workflow**: Test place→block→pathfind→remove
3. **UI interaction**: Test click HQ → panel shows → order worker
4. **Viewport culling**: Test camera movement updates visible tiles

### Performance Tests

1. **Viewport culling performance**: Measure FPS with 50x50 map
2. **Building placement performance**: Measure time to place 100 buildings
3. **Pathfinding with buildings**: Measure pathfinding time with 50 buildings

**Performance Requirements:**
- Maintain 60 FPS on 50x50 map with 50 buildings
- Building placement < 1ms per building
- Viewport culling calculation < 2ms per frame
