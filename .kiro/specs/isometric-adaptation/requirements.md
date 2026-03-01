# Requirements Document: Isometric Game Adaptation

## Introduction

Адаптація існуючої RTS гри "Punic Wars: Castra" для роботи з ізометричною системою координат. Гра наразі використовує екранні координати (x, y), але потрібно перейти на grid-based систему з ізометричною проекцією.

## Glossary

- **Grid_Coordinates**: Координати в сітці карти (row, col)
- **Screen_Coordinates**: Координати на екрані в пікселях (x, y)
- **Isometric_Renderer**: Система рендерингу ізометричних об'єктів
- **Unit**: Ігровий юніт (легіонер, раб, фінікієць)
- **Building**: Будівля (HQ, казарма, квесторій)
- **Sprite**: Графічне зображення об'єкта
- **Debug_Mode**: Режим відображення без спрайтів (кольорові фігури)
- **Tile**: Один тайл карти (64x32 пікселі в ізометрії)

## Requirements

### Requirement 1: Grid-Based Coordinate System

**User Story:** As a developer, I want all game objects to use grid coordinates, so that they align with the isometric tilemap.

#### Acceptance Criteria

1. THE Unit SHALL store position as GridCoords instead of float x, y
2. THE Building SHALL store position as GridCoords instead of int x, y
3. THE ResourcePoint SHALL store position as GridCoords instead of int x, y
4. WHEN converting to screen coordinates, THE System SHALL use CoordinateConverter::gridToScreen
5. WHEN converting from screen coordinates, THE System SHALL use CoordinateConverter::screenToGrid

### Requirement 2: Isometric Unit Rendering

**User Story:** As a player, I want units to be displayed as isometric sprites, so that they match the visual style of the map.

#### Acceptance Criteria

1. THE Unit SHALL render at screen position calculated from grid coordinates
2. WHEN no sprite is loaded, THE Unit SHALL render as colored isometric diamond (debug mode)
3. THE Unit debug diamond SHALL be 32x16 pixels (half-tile size)
4. THE Unit SHALL have anchor point at bottom center of sprite
5. WHEN sprite is loaded, THE Unit SHALL render sprite at correct isometric position
6. THE Unit sprite SHALL have transparent background
7. THE Unit sprite SHALL be drawn with Painter's Algorithm (back-to-front sorting)

### Requirement 3: Isometric Building Rendering

**User Story:** As a player, I want buildings to be displayed as isometric sprites, so that they integrate with the map visually.

#### Acceptance Criteria

1. THE Building SHALL render at screen position calculated from grid coordinates
2. WHEN no sprite is loaded, THE Building SHALL render as colored isometric rectangle (debug mode)
3. THE Building debug shape SHALL be 64x32 pixels (one tile) or larger for multi-tile buildings
4. THE Building SHALL have anchor point at bottom center
5. WHEN sprite is loaded, THE Building SHALL render sprite at correct isometric position
6. THE Building sprite SHALL support multi-tile footprint (2x2, 3x3, etc.)
7. THE Building SHALL be drawn with Painter's Algorithm

### Requirement 4: Sprite Format Specification

**User Story:** As an artist, I want clear specifications for sprite dimensions, so that I can create compatible assets.

#### Acceptance Criteria

1. THE Unit sprite SHALL be PNG with transparency
2. THE Unit sprite SHALL have dimensions 64x64 pixels (recommended)
3. THE Unit sprite SHALL have anchor at bottom center (32, 64)
4. THE Building sprite SHALL be PNG with transparency
5. THE Building sprite SHALL have dimensions based on footprint: 1x1=128x96, 2x2=256x160, 3x3=384x224
6. THE Building sprite SHALL have anchor at bottom center
7. THE Sprite SHALL use isometric perspective (2:1 ratio)

### Requirement 5: Debug Mode Rendering

**User Story:** As a developer, I want debug mode rendering for all objects, so that I can test without sprites.

#### Acceptance Criteria

1. WHEN sprite file does not exist, THE System SHALL use debug rendering
2. THE Debug unit SHALL be colored diamond: Rome=RED, Carthage=BLUE, Slave=YELLOW
3. THE Debug building SHALL be colored rectangle with type label
4. THE Debug shape SHALL have black outline for visibility
5. THE Debug mode SHALL display object type text above shape
6. THE System SHALL log when using debug mode for each object type

### Requirement 6: Mouse Interaction with Isometric Objects

**User Story:** As a player, I want to click on units and buildings in isometric view, so that I can select and command them.

#### Acceptance Criteria

1. WHEN player clicks, THE System SHALL convert screen coordinates to grid coordinates
2. THE System SHALL check for objects at clicked grid position
3. THE System SHALL use diamond-shaped hit detection for units
4. THE System SHALL use rectangular hit detection for buildings
5. WHEN multiple objects overlap, THE System SHALL select the frontmost object
6. THE Selection SHALL highlight selected object with outline

### Requirement 7: Movement in Isometric Space

**User Story:** As a player, I want units to move smoothly in isometric space, so that movement looks natural.

#### Acceptance Criteria

1. WHEN unit moves, THE Unit SHALL interpolate between grid positions
2. THE Unit SHALL move in grid-aligned directions (8 directions)
3. THE Unit SHALL update grid position when reaching destination
4. THE Pathfinding SHALL work with grid coordinates
5. THE Unit SHALL face movement direction (8 directional sprites optional)

### Requirement 8: Building Placement in Isometric Grid

**User Story:** As a player, I want to place buildings on the isometric grid, so that they align with tiles.

#### Acceptance Criteria

1. WHEN placing building, THE System SHALL snap to grid coordinates
2. THE System SHALL check if grid cells are passable
3. THE System SHALL check if grid cells are not occupied
4. THE System SHALL show placement preview with green/red tint
5. THE Building SHALL occupy multiple grid cells based on footprint
6. THE System SHALL prevent placement on water tiles

### Requirement 9: Resource Points in Isometric Space

**User Story:** As a player, I want resource points to appear on the isometric map, so that I can gather resources.

#### Acceptance Criteria

1. THE ResourcePoint SHALL use grid coordinates
2. THE ResourcePoint SHALL render as isometric icon or sprite
3. WHEN no sprite loaded, THE ResourcePoint SHALL render as colored square with letter
4. THE ResourcePoint SHALL be clickable for harvesting commands
5. THE ResourcePoint SHALL display remaining resources as text

### Requirement 10: Camera Control for Isometric View

**User Story:** As a player, I want to pan and zoom the isometric camera, so that I can view different parts of the map.

#### Acceptance Criteria

1. THE Camera SHALL support panning with arrow keys or edge scrolling
2. THE Camera SHALL support zooming with mouse wheel
3. THE Camera SHALL have minimum zoom of 0.3x and maximum of 2.0x
4. THE Camera SHALL clamp to map boundaries
5. THE Camera SHALL center on selected units with hotkey
6. THE Camera SHALL smoothly interpolate when centering

### Requirement 11: Depth Sorting (Painter's Algorithm)

**User Story:** As a player, I want objects to render in correct depth order, so that closer objects appear in front.

#### Acceptance Criteria

1. THE System SHALL sort all objects by grid row + col before rendering
2. THE System SHALL render in back-to-front order (low row+col first)
3. THE System SHALL handle multi-tile buildings correctly
4. THE System SHALL render map tiles first, then objects
5. THE System SHALL render UI elements last (always on top)

### Requirement 12: Sprite Loading System

**User Story:** As a developer, I want a flexible sprite loading system, so that sprites can be added incrementally.

#### Acceptance Criteria

1. THE System SHALL attempt to load sprites from assets/sprites/isometric/
2. WHEN sprite file not found, THE System SHALL fall back to debug rendering
3. THE System SHALL log which sprites are loaded and which use debug mode
4. THE System SHALL support hot-reloading of sprites (optional)
5. THE Sprite filenames SHALL follow convention: unit_type_faction.png, building_type_faction.png

