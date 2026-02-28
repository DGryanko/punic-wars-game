# Requirements Document: Pathfinding System

## Introduction

Система пошуку шляху (pathfinding) для RTS гри Punic Wars: Castra. Система повинна забезпечити ефективний рух юнітів навколо перешкод (будівель та інших юнітів) з використанням алгоритму A*.

## Glossary

- **Pathfinding_System**: Система пошуку оптимального шляху для руху юнітів
- **Navigation_Grid**: Сітка навігації, що розбиває ігрову карту на клітинки
- **A_Star_Algorithm**: Алгоритм A* для пошуку найкоротшого шляху
- **Obstacle**: Перешкода (будівля або інший юніт)
- **Waypoint**: Точка на шляху руху юніта
- **Path_Cache**: Кеш збережених шляхів для оптимізації
- **Unit**: Ігровий юніт, що потребує руху
- **Collision_Detection**: Система виявлення колізій

## Requirements

### Requirement 1: Navigation Grid

**User Story:** Як розробник, я хочу мати сітку навігації, щоб розбити карту на клітинки для алгоритму пошуку шляху.

#### Acceptance Criteria

1. THE Navigation_Grid SHALL divide the game map into cells of 16x16 pixels
2. WHEN an obstacle is present, THE Navigation_Grid SHALL mark corresponding cells as blocked
3. WHEN the game starts, THE Navigation_Grid SHALL initialize with all buildings marked as obstacles
4. THE Navigation_Grid SHALL provide a method to check if a cell is walkable
5. THE Navigation_Grid SHALL update dynamically when obstacles move or are removed

### Requirement 2: A* Pathfinding Algorithm

**User Story:** Як гравець, я хочу щоб юніти знаходили оптимальний шлях до цілі, щоб вони не застрягали біля перешкод.

#### Acceptance Criteria

1. WHEN a unit needs to move to a target, THE A_Star_Algorithm SHALL calculate the shortest walkable path
2. THE A_Star_Algorithm SHALL use Manhattan distance as the heuristic function
3. WHEN no path exists, THE A_Star_Algorithm SHALL return an empty path
4. THE A_Star_Algorithm SHALL consider diagonal movement with appropriate cost
5. WHEN calculating paths, THE A_Star_Algorithm SHALL complete within 16ms (one frame at 60 FPS)

### Requirement 3: Path Following

**User Story:** Як гравець, я хочу щоб юніти плавно рухалися по знайденому шляху, щоб рух виглядав природно.

#### Acceptance Criteria

1. WHEN a unit has a path, THE Unit SHALL move to the next waypoint
2. WHEN a unit reaches a waypoint, THE Unit SHALL automatically move to the next waypoint in the path
3. WHEN a unit reaches the final waypoint, THE Unit SHALL stop moving
4. THE Unit SHALL maintain smooth movement between waypoints
5. WHEN a unit's path is blocked, THE Unit SHALL recalculate the path

### Requirement 4: Dynamic Obstacle Avoidance

**User Story:** Як гравець, я хочу щоб юніти обходили інших юнітів, щоб вони не застрягали в натовпі.

#### Acceptance Criteria

1. WHEN a unit encounters another unit, THE Pathfinding_System SHALL treat it as a temporary obstacle
2. WHEN a unit is blocked by another unit, THE Unit SHALL wait briefly before recalculating
3. THE Pathfinding_System SHALL not mark moving units as permanent obstacles in the grid
4. WHEN multiple units move to the same area, THE Pathfinding_System SHALL allow them to pass through each other with slight delays
5. THE Unit SHALL use local avoidance for small obstacles within 32 pixels

### Requirement 5: Path Caching and Optimization

**User Story:** Як розробник, я хочу оптимізувати обчислення шляхів, щоб гра працювала плавно навіть з багатьма юнітами.

#### Acceptance Criteria

1. WHEN a path is calculated, THE Path_Cache SHALL store it for reuse
2. WHEN obstacles change, THE Path_Cache SHALL invalidate affected paths
3. THE Pathfinding_System SHALL limit pathfinding calculations to 5 per frame maximum
4. WHEN multiple units have the same destination, THE Pathfinding_System SHALL reuse calculated paths
5. THE Path_Cache SHALL automatically clear paths older than 10 seconds

### Requirement 6: Unstuck Mechanism

**User Story:** Як гравець, я хочу щоб юніти автоматично виходили з застряглих ситуацій, щоб не потрібно було вручну їх переміщувати.

#### Acceptance Criteria

1. WHEN a unit doesn't move for 2 seconds while trying to move, THE Unit SHALL be considered stuck
2. WHEN a unit is stuck, THE Unit SHALL try to move to a nearby free position
3. WHEN a unit cannot unstuck itself after 3 attempts, THE Unit SHALL recalculate the entire path
4. THE Unit SHALL log stuck situations for debugging
5. WHEN a unit successfully unstucks, THE Unit SHALL resume following its path

### Requirement 7: Integration with Existing Systems

**User Story:** Як розробник, я хочу інтегрувати pathfinding з існуючими системами, щоб не порушити поточну функціональність.

#### Acceptance Criteria

1. THE Pathfinding_System SHALL integrate with the existing Unit structure
2. THE Pathfinding_System SHALL work with the existing resource harvesting system
3. THE Pathfinding_System SHALL work with the existing combat system
4. WHEN a unit is harvesting, THE Pathfinding_System SHALL not interfere with harvesting behavior
5. THE Pathfinding_System SHALL maintain backward compatibility with manual unit movement

### Requirement 8: Performance Requirements

**User Story:** Як гравець, я хочу щоб гра працювала плавно, щоб pathfinding не викликав лагів.

#### Acceptance Criteria

1. THE Pathfinding_System SHALL maintain 60 FPS with up to 50 units
2. WHEN calculating a path, THE A_Star_Algorithm SHALL explore maximum 500 nodes
3. THE Navigation_Grid SHALL update in less than 1ms when obstacles change
4. THE Pathfinding_System SHALL use less than 10MB of memory for path storage
5. WHEN the game has 100+ units, THE Pathfinding_System SHALL prioritize player units for pathfinding
