# Design Document: Pathfinding System

## Overview

Система pathfinding для Punic Wars: Castra використовує алгоритм A* для пошуку оптимальних шляхів руху юнітів. Система включає навігаційну сітку, кешування шляхів, механізм виходу з застряглих ситуацій та динамічне уникнення перешкод.

## Architecture

### High-Level Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    Game Loop (main.cpp)                  │
└────────────────────┬────────────────────────────────────┘
                     │
                     ▼
┌─────────────────────────────────────────────────────────┐
│              Pathfinding Manager                         │
│  - Request path calculations                             │
│  - Manage calculation queue                              │
│  - Update navigation grid                                │
└────────┬──────────────────────┬─────────────────────────┘
         │                      │
         ▼                      ▼
┌──────────────────┐   ┌──────────────────────┐
│ Navigation Grid  │   │   A* Algorithm       │
│  - Cell data     │   │  - Open/closed lists │
│  - Obstacle map  │   │  - Heuristic calc    │
└──────────────────┘   └──────────────────────┘
         │                      │
         └──────────┬───────────┘
                    ▼
         ┌─────────────────────┐
         │    Path Cache       │
         │  - Stored paths     │
         │  - Invalidation     │
         └─────────────────────┘
                    │
                    ▼
         ┌─────────────────────┐
         │   Unit Movement     │
         │  - Follow waypoints │
         │  - Unstuck logic    │
         └─────────────────────┘
```

## Components and Interfaces

### 1. NavigationGrid Class

```cpp
class NavigationGrid {
public:
    // Ініціалізація сітки
    void init(int mapWidth, int mapHeight, int cellSize);
    
    // Перевірка чи клітинка прохідна
    bool isWalkable(int gridX, int gridY) const;
    
    // Конвертація координат
    void worldToGrid(int worldX, int worldY, int& gridX, int& gridY) const;
    void gridToWorld(int gridX, int gridY, int& worldX, int& worldY) const;
    
    // Оновлення перешкод
    void markObstacle(int gridX, int gridY, bool blocked);
    void updateFromBuildings(const std::vector<Building>& buildings);
    
    // Отримання сусідів для A*
    std::vector<GridNode> getNeighbors(int gridX, int gridY) const;
    
private:
    int width, height;      // Розмір сітки в клітинках
    int cellSize;           // Розмір клітинки в пікселях
    std::vector<bool> cells; // Масив прохідності клітинок
};
```

### 2. AStar Algorithm

```cpp
struct GridNode {
    int x, y;           // Координати в сітці
    float g, h, f;      // Вартості для A*
    GridNode* parent;   // Батьківський вузол
};

class AStarPathfinder {
public:
    // Знайти шлях від start до goal
    std::vector<Vector2> findPath(
        Vector2 start, 
        Vector2 goal, 
        const NavigationGrid& grid
    );
    
private:
    // Обчислити евристику (Manhattan distance)
    float heuristic(int x1, int y1, int x2, int y2) const;
    
    // Реконструювати шлях з вузлів
    std::vector<Vector2> reconstructPath(GridNode* goalNode) const;
    
    // Спрощення шляху (видалення зайвих точок)
    std::vector<Vector2> smoothPath(
        const std::vector<Vector2>& path,
        const NavigationGrid& grid
    ) const;
};
```

### 3. PathfindingManager

```cpp
struct PathRequest {
    int unitId;
    Vector2 start;
    Vector2 goal;
    float priority;
};

class PathfindingManager {
public:
    // Запит на обчислення шляху
    void requestPath(int unitId, Vector2 start, Vector2 goal, float priority = 1.0f);
    
    // Оновлення (викликається кожен кадр)
    void update(float deltaTime);
    
    // Отримати обчислений шлях
    bool getPath(int unitId, std::vector<Vector2>& outPath);
    
    // Оновити навігаційну сітку
    void updateGrid(const std::vector<Building>& buildings);
    
    // Інвалідувати кеш в області
    void invalidatePathsInArea(Rectangle area);
    
private:
    NavigationGrid grid;
    AStarPathfinder pathfinder;
    std::queue<PathRequest> requestQueue;
    std::map<int, std::vector<Vector2>> pathCache;
    
    int maxCalculationsPerFrame = 5;
};
```

### 4. Unit Path Following (розширення Unit структури)

```cpp
// Додаткові поля в Unit структурі
struct Unit {
    // ... існуючі поля ...
    
    // Pathfinding поля
    std::vector<Vector2> path;        // Поточний шлях
    int currentWaypointIndex = 0;     // Індекс поточної точки шляху
    float stuckTimer = 0.0f;          // Таймер для виявлення застрягання
    Vector2 lastPosition = {0, 0};    // Остання позиція для виявлення застрягання
    int unstuckAttempts = 0;          // Кількість спроб виходу з застрягання
    
    // Методи
    void setPath(const std::vector<Vector2>& newPath);
    void followPath(float deltaTime);
    void checkIfStuck(float deltaTime);
    void tryUnstuck();
    bool hasPath() const;
    void clearPath();
};
```

## Data Models

### Grid Cell

```cpp
// Клітинка сітки (1 байт на клітинку)
struct Cell {
    bool walkable : 1;      // Чи можна пройти
    bool permanent : 1;     // Постійна перешкода (будівля)
    bool temporary : 1;     // Тимчасова перешкода (юніт)
    uint8_t reserved : 5;   // Резерв для майбутнього
};
```

### Path Data

```cpp
struct PathData {
    std::vector<Vector2> waypoints;  // Точки шляху
    float timestamp;                 // Час створення
    int unitId;                      // ID юніта
    bool valid;                      // Чи шлях ще валідний
};
```

## Correctness Properties

*A property is a characteristic or behavior that should hold true across all valid executions of a system—essentially, a formal statement about what the system should do. Properties serve as the bridge between human-readable specifications and machine-verifiable correctness guarantees.*

### Property 1: Path Validity
*For any* calculated path from point A to point B, all waypoints in the path should be on walkable cells in the navigation grid.
**Validates: Requirements 2.1, 2.3**

### Property 2: Path Optimality
*For any* two points A and B with a valid path, the A* algorithm should return a path with length no more than 1.5x the straight-line distance (accounting for obstacles).
**Validates: Requirements 2.1, 2.2**

### Property 3: Grid Consistency
*For any* obstacle added or removed, the navigation grid should correctly update all affected cells within one frame.
**Validates: Requirements 1.2, 1.5**

### Property 4: Waypoint Progression
*For any* unit following a path, when it reaches waypoint N, it should move to waypoint N+1 or stop if N is the last waypoint.
**Validates: Requirements 3.1, 3.2, 3.3**

### Property 5: Unstuck Guarantee
*For any* unit that is stuck for more than 2 seconds, the system should attempt to unstuck it within 3 attempts or recalculate the path.
**Validates: Requirements 6.1, 6.2, 6.3**

### Property 6: Performance Bound
*For any* frame, the pathfinding system should calculate at most 5 paths and complete within 16ms.
**Validates: Requirements 8.1, 8.2**

### Property 7: Cache Invalidation
*For any* obstacle change in area R, all cached paths that intersect with R should be marked as invalid.
**Validates: Requirements 5.2**

### Property 8: Path Completion
*For any* unit with a valid path, if the unit follows the path without interruption, it should reach the goal position within a reasonable time (path_length / unit_speed * 1.2).
**Validates: Requirements 3.3**

## Error Handling

### No Path Found
- **Scenario**: A* cannot find a path to the destination
- **Handling**: Return empty path, unit stays in place, log warning
- **User Feedback**: Unit shows "cannot reach" indicator

### Path Blocked During Movement
- **Scenario**: Obstacle appears on unit's current path
- **Handling**: Recalculate path from current position
- **Fallback**: If recalculation fails, try local avoidance

### Unit Stuck
- **Scenario**: Unit cannot move for 2+ seconds
- **Handling**: 
  1. Try moving to adjacent free cell
  2. If fails, try 3 times with different directions
  3. If still fails, recalculate entire path
  4. Last resort: teleport to nearest free cell

### Performance Degradation
- **Scenario**: Too many pathfinding requests
- **Handling**: Queue requests, prioritize player units
- **Limit**: Max 5 calculations per frame

### Memory Overflow
- **Scenario**: Path cache grows too large
- **Handling**: Remove oldest paths, limit cache to 100 paths
- **Prevention**: Auto-clear paths older than 10 seconds

## Testing Strategy

### Unit Tests
- Grid coordinate conversion (world ↔ grid)
- Cell walkability checks
- Obstacle marking/unmarking
- Heuristic calculation
- Path reconstruction
- Path smoothing algorithm

### Property-Based Tests
- **Property 1**: Generate random start/goal, verify all waypoints are walkable
- **Property 2**: Generate random obstacles, verify path length is reasonable
- **Property 3**: Add/remove random obstacles, verify grid updates correctly
- **Property 4**: Simulate unit movement, verify waypoint progression
- **Property 5**: Simulate stuck scenarios, verify unstuck mechanism
- **Property 6**: Generate many path requests, verify performance limits
- **Property 7**: Change obstacles, verify cache invalidation
- **Property 8**: Simulate complete path following, verify arrival

### Integration Tests
- Multiple units pathfinding simultaneously
- Units avoiding each other dynamically
- Pathfinding with resource harvesting
- Pathfinding with combat
- Large maps with many obstacles

### Performance Tests
- 50 units pathfinding simultaneously (should maintain 60 FPS)
- Path calculation time (should be < 16ms)
- Memory usage (should be < 10MB)
- Grid update time (should be < 1ms)

## Implementation Notes

### Grid Size Calculation
- Map size: 1434x1075 pixels
- Cell size: 16x16 pixels
- Grid dimensions: 90x68 cells
- Memory: ~6KB for grid data

### A* Optimization
- Use binary heap for open list (O(log n) operations)
- Limit node exploration to 500 nodes maximum
- Early exit if goal is unreachable
- Cache heuristic values when possible

### Path Smoothing
- Remove collinear waypoints
- Use line-of-sight checks to skip intermediate points
- Reduce waypoint count by ~40% on average

### Diagonal Movement
- Allow 8-directional movement
- Diagonal cost: √2 ≈ 1.414
- Straight cost: 1.0

### Integration Steps
1. Add pathfinding.h header file
2. Create PathfindingManager global instance
3. Initialize grid in InitBuildings()
4. Replace Unit::update() movement logic
5. Add path following to Unit::followPath()
6. Update collision detection to use grid
7. Add debug visualization (optional)
