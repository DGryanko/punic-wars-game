# Design Document: Slave Resource Gathering System Fixes

## Overview

Система збору ресурсів рабами має три критичні проблеми:
1. **Відсутність автоматичного циклу** - раб не повертається до ресурсу після здачі
2. **Ігнорування будівель** - pathfinding не враховує будівлі як перешкоди
3. **Ненадійна здача ресурсів** - логіка dropoff не завжди спрацьовує

Стратегія виправлення складається з трьох компонентів:
1. **TileMap Integration** - додати метод `setPassable()` для динамічного управління прохідністю
2. **NavigationGrid Update** - реалізувати `updateFromBuildings()` для позначення будівель як перешкод
3. **Harvest Cycle State Machine** - створити автоматичний цикл збору з чіткими станами

## Architecture

### Component Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                     Game Loop (main.cpp)                     │
│  ┌────────────────────────────────────────────────────────┐ │
│  │  ProcessResourceHarvesting()                           │ │
│  │  - Перевіряє стан кожного раба                        │ │
│  │  - Керує state machine циклу збору                    │ │
│  └────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                      Unit (unit.h)                           │
│  ┌────────────────────────────────────────────────────────┐ │
│  │  State Fields:                                         │ │
│  │  - is_harvesting, is_moving                           │ │
│  │  - carrying_food, carrying_gold, max_carry_capacity   │ │
│  │  - assigned_resource_position (GridCoords)            │ │
│  │  - assigned_dropoff_position (GridCoords)             │ │
│  │                                                        │ │
│  │  Methods:                                              │ │
│  │  - assignResource(resource_pos, dropoff_pos)          │ │
│  │  - hasAssignedResource()                              │ │
│  │  - canCarryMore()                                     │ │
│  │  - dropResources(food, gold)                          │ │
│  └────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│              PathfindingManager (pathfinding.h)              │
│  ┌────────────────────────────────────────────────────────┐ │
│  │  NavigationGrid                                        │ │
│  │  - init(TileMap*)                                      │ │
│  │  - isWalkable(row, col) → TileMap.isPassable()       │ │
│  │  - updateFromBuildings(buildings)                     │ │
│  │                                                        │ │
│  │  AStarPathfinder                                       │ │
│  │  - findPath(start, goal, grid)                        │ │
│  │  - Обходить непрохідні тайли                         │ │
│  └────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                   TileMap (tilemap.h)                        │
│  ┌────────────────────────────────────────────────────────┐ │
│  │  Data:                                                 │ │
│  │  - tiles[row][col].terrain_type                       │ │
│  │  - tiles[row][col].is_passable                        │ │
│  │                                                        │ │
│  │  Methods:                                              │ │
│  │  - isPassable(row, col) → bool                        │ │
│  │  - setPassable(row, col, passable) [NEW]             │ │
│  └────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

## Components and Interfaces

### 1. TileMap Enhancement

**File**: `cpp/src/tilemap/tilemap.h`

**New Method**:
```cpp
void setPassable(int row, int col, bool passable) {
    if (row >= 0 && row < height && col >= 0 && col < width) {
        tiles[row][col].is_passable = passable;
    }
}
```

**Purpose**: Дозволити динамічно змінювати прохідність тайлів для будівель

### 2. NavigationGrid Building Integration

**File**: `cpp/src/pathfinding.h`

**Updated Method**:
```cpp
void updateFromBuildings(const std::vector<Building>& buildings) {
    if (!tileMap) return;
    
    // Спочатку скидаємо всі будівельні перешкоди
    // (припускаємо що TileMap зберігає оригінальну прохідність)
    
    // Позначаємо тайли під будівлями як непрохідні
    for (const auto& building : buildings) {
        GridCoords pos = building.getGridPosition();
        GridCoords footprint = building.getFootprint();
        
        for (int dr = 0; dr < footprint.row; dr++) {
            for (int dc = 0; dc < footprint.col; dc++) {
                int row = pos.row + dr;
                int col = pos.col + dc;
                // Використовуємо const_cast для модифікації TileMap
                const_cast<TileMap*>(tileMap)->setPassable(row, col, false);
            }
        }
    }
}
```

**Purpose**: Синхронізувати TileMap з будівлями на мапі

### 3. Unit Resource Assignment

**File**: `cpp/src/unit.h`

**New Fields**:
```cpp
GridCoords assigned_resource_position;  // Позиція призначеного ресурсу
GridCoords assigned_dropoff_position;   // Позиція будівлі здачі
bool has_assigned_resource;             // Чи є призначений ресурс
```

**Updated Methods**:
```cpp
void assignResource(GridCoords resource_pos, GridCoords dropoff_pos) {
    assigned_resource_position = resource_pos;
    assigned_dropoff_position = dropoff_pos;
    has_assigned_resource = true;
}

bool hasAssignedResource() const {
    return has_assigned_resource;
}

void clearResourceAssignment() {
    has_assigned_resource = false;
    assigned_resource_position = GridCoords(-1, -1);
    assigned_dropoff_position = GridCoords(-1, -1);
}
```

### 4. Harvest Cycle State Machine

**File**: `cpp/src/main.cpp`

**States**:
```cpp
enum HarvestState {
    MOVING_TO_RESOURCE,   // Рухається до ресурсу
    HARVESTING,           // Збирає ресурс
    MOVING_TO_DROPOFF,    // Рухається до будівлі здачі
    DROPPING_RESOURCES    // Здає ресурси
};
```

**State Transitions**:
```
MOVING_TO_RESOURCE → HARVESTING (коли досяг ресурсу)
HARVESTING → MOVING_TO_DROPOFF (коли інвентар повний)
MOVING_TO_DROPOFF → DROPPING_RESOURCES (коли досяг будівлі)
DROPPING_RESOURCES → MOVING_TO_RESOURCE (після здачі)
```

## Data Models

### Unit State Extension

```cpp
class Unit {
    // Existing fields
    bool is_harvesting;
    bool is_moving;
    int carrying_food;
    int carrying_gold;
    int max_carry_capacity;
    
    // New fields for resource gathering
    GridCoords assigned_resource_position;
    GridCoords assigned_dropoff_position;
    bool has_assigned_resource;
    
    // Methods
    void assignResource(GridCoords resource_pos, GridCoords dropoff_pos);
    bool hasAssignedResource() const;
    void clearResourceAssignment();
    bool canCarryMore() const;
    void dropResources(int& food, int& gold);
};
```

### TileMap Passability

```cpp
struct Tile {
    TerrainType terrain_type;
    bool is_passable;  // Може бути змінено динамічно
};

class TileMap {
    std::vector<std::vector<Tile>> tiles;
    
    bool isPassable(int row, int col) const;
    void setPassable(int row, int col, bool passable);  // NEW
};
```

## Correctness Properties

*A property is a characteristic or behavior that should hold true across all valid executions of a system-essentially, a formal statement about what the system should do. Properties serve as the bridge between human-readable specifications and machine-verifiable correctness guarantees.*

### Property 1: Harvest Cycle Completion

*For any* раб з призначеним ресурсом, якщо ресурс не вичерпаний і будівля здачі існує, раб SHALL завершити повний цикл збору (рух до ресурсу → збір → рух до будівлі → здача → повернення) без зовнішнього втручання.

**Validates: Requirements 1.1, 1.2, 1.3, 1.4, 1.5**

### Property 2: Building Obstacle Avoidance

*For any* шлях обчислений pathfinding, жоден тайл у шляху не SHALL бути зайнятий будівлею (тобто всі тайли в шляху мають бути прохідними згідно TileMap.isPassable()).

**Validates: Requirements 2.1, 2.2, 2.3, 2.4, 2.5**

### Property 3: Resource Conservation

*For any* раб що здає ресурси, сума ресурсів до здачі (carrying_food + carrying_gold) SHALL дорівнювати сумі ресурсів доданих до фракції (delta_rome_food + delta_rome_money), і інвентар раба SHALL бути порожнім після здачі.

**Validates: Requirements 3.3, 3.4, 3.5**

### Property 4: TileMap Building Synchronization

*For any* будівля на мапі, всі тайли в межах footprint будівлі SHALL бути непрохідними в TileMap (isPassable() повертає false), і pathfinding SHALL обходити ці тайли.

**Validates: Requirements 4.1, 4.2, 4.3, 4.4, 4.5**

### Property 5: Resource Assignment Persistence

*For any* раб з призначеним ресурсом, координати ресурсу та будівлі здачі SHALL зберігатися протягом всього циклу збору, поки ресурс не вичерпається або раб не отримає нову команду.

**Validates: Requirements 5.1, 5.2, 5.3, 5.4, 5.5**

## Error Handling

### Resource Not Found
- **Scenario**: Ресурс вичерпався під час циклу збору
- **Handling**: Очистити assigned_resource, зупинити is_harvesting, вивести лог

### Dropoff Building Not Found
- **Scenario**: Немає доступної будівлі здачі (Questorium або HQ)
- **Handling**: Зупинити цикл збору, вивести помилку в лог, раб очікує команд

### Path Not Found
- **Scenario**: Pathfinding не може знайти шлях до ресурсу або будівлі
- **Handling**: Спробувати знайти найближчий вільний тайл, якщо не вдається - зупинити цикл

### Building Blocks Path
- **Scenario**: Будівля розміщена на шляху раба
- **Handling**: Перерахувати шлях з урахуванням нової перешкоди

## Testing Strategy

### Unit Tests

1. **TileMap.setPassable() Test**
   - Перевірити що setPassable(row, col, false) робить тайл непрохідним
   - Перевірити що setPassable(row, col, true) робить тайл прохідним
   - Перевірити bounds checking (координати за межами мапи)

2. **NavigationGrid.updateFromBuildings() Test**
   - Створити тестову будівлю 2x2 на позиції (10, 10)
   - Викликати updateFromBuildings()
   - Перевірити що тайли (10,10), (10,11), (11,10), (11,11) непрохідні
   - Перевірити що сусідні тайли залишаються прохідними

3. **Unit.assignResource() Test**
   - Призначити ресурс на позиції (5, 5) з dropoff на (10, 10)
   - Перевірити що assigned_resource_position == (5, 5)
   - Перевірити що assigned_dropoff_position == (10, 10)
   - Перевірити що hasAssignedResource() == true

4. **Resource Dropoff Test**
   - Створити раба з carrying_food=10, carrying_gold=5
   - Розмістити раба біля Questorium (distance < 3.0)
   - Викликати логіку здачі
   - Перевірити що rome_food збільшилась на 10
   - Перевірити що rome_money збільшилась на 5
   - Перевірити що carrying_food == 0 і carrying_gold == 0

### Property-Based Tests

1. **Property 1: Harvest Cycle Completion**
   - Генерувати випадкові позиції ресурсу та будівлі
   - Створити раба і призначити ресурс
   - Симулювати game loop поки цикл не завершиться
   - Перевірити що раб пройшов всі стани циклу
   - Перевірити що ресурси були здані

2. **Property 2: Building Obstacle Avoidance**
   - Генерувати випадкові будівлі на мапі
   - Викликати updateFromBuildings()
   - Генерувати випадкові start/goal позиції
   - Обчислити шлях через pathfinding
   - Перевірити що жоден тайл шляху не зайнятий будівлею

3. **Property 3: Resource Conservation**
   - Генерувати випадкові значення carrying_food та carrying_gold
   - Зберегти початкові значення rome_food та rome_money
   - Викликати логіку здачі ресурсів
   - Перевірити що delta дорівнює carrying values
   - Перевірити що інвентар порожній

### Integration Tests

1. **Full Harvest Cycle Test**
   - Створити повну ігрову сцену з рабом, ресурсом, Questorium
   - Відправити раба до ресурсу (правий клік)
   - Симулювати game loop
   - Перевірити що раб:
     - Дійшов до ресурсу
     - Зібрав ресурси
     - Дійшов до Questorium
     - Здав ресурси
     - Повернувся до ресурсу
   - Перевірити що ресурси додані до фракції

2. **Building Pathfinding Test**
   - Створити сцену з будівлями між стартом і ціллю
   - Відправити юніта до цілі
   - Перевірити що юніт обходить будівлі
   - Перевірити що юніт не застряє

3. **Multiple Slaves Test**
   - Створити 3 раби і 1 ресурс
   - Відправити всіх до ресурсу
   - Перевірити що всі раби збирають і здають ресурси
   - Перевірити що немає конфліктів або застрягань

## Implementation Notes

### Phase 1: TileMap Enhancement
- Додати метод `setPassable()` до TileMap
- Тестувати що метод коректно змінює прохідність

### Phase 2: NavigationGrid Integration
- Реалізувати `updateFromBuildings()` для позначення будівель
- Викликати після кожного розміщення/видалення будівлі
- Тестувати що pathfinding обходить будівлі

### Phase 3: Unit Resource Assignment
- Додати поля для збереження позицій ресурсу та dropoff
- Реалізувати методи assignResource() та clearResourceAssignment()
- Оновити логіку HandleClicks для призначення ресурсу

### Phase 4: Harvest Cycle State Machine
- Реалізувати state machine в ProcessResourceHarvesting()
- Додати логіку для кожного стану
- Тестувати повний цикл збору

### Phase 5: Integration and Testing
- Інтегрувати всі компоненти
- Запустити integration tests
- Виправити знайдені баги
