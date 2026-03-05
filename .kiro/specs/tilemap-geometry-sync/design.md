# TileMap Geometry Synchronization Bugfix Design

## Overview

Після адаптації гри до ізометричної проекції виявлено критичну проблему несинхронізації геометрії між візуалізацією та pathfinding. NavigationGrid використовує прямокутну сітку 16x16 пікселів, тоді як візуалізація працює з ізометричними тайлами 128x64 пікселів через CoordinateConverter. Це призводить до невідповідності між візуальним розташуванням об'єктів та областями взаємодії (pathfinding, mouse clicks, collision detection).

Стратегія виправлення: замінити прямокутну піксельну сітку NavigationGrid на ізометричну сітку тайлів, використовуючи GridCoords та CoordinateConverter для всіх конвертацій координат. NavigationGrid має працювати в термінах тайлів (row, col), а не пікселів, і делегувати перевірку прохідності до TileMap.isPassable().

## Glossary

- **Bug_Condition (C)**: Умова, що викликає баг - NavigationGrid використовує прямокутну піксельну сітку 16x16 замість ізометричної сітки тайлів
- **Property (P)**: Бажана поведінка - NavigationGrid має використовувати GridCoords та CoordinateConverter для синхронізації з ізометричною геометрією
- **Preservation**: Існуюча функціональність A* pathfinding, кешування шляхів та обробка команд руху має залишитися незмінною
- **NavigationGrid**: Клас у `cpp/src/pathfinding.h`, що управляє сіткою прохідності для pathfinding
- **CoordinateConverter**: Клас у `cpp/src/tilemap/coordinates.h`, що конвертує між GridCoords (row, col) та ScreenCoords (x, y) для ізометричної проекції
- **TileMap**: Клас у `cpp/src/tilemap/tilemap.h`, що зберігає типи місцевості та надає метод isPassable(row, col)
- **GridCoords**: Структура логічних координат сітки (row, col)
- **ScreenCoords**: Структура екранних координат у пікселях (x, y)
- **cellSize**: Поточний розмір клітинки NavigationGrid у пікселях (16), який має бути замінений на тайлову систему

## Bug Details

### Fault Condition

Баг проявляється коли NavigationGrid виконує будь-яку операцію з координатами або прохідністю. Система використовує прямокутну піксельну математику (ділення/множення на cellSize=16) замість ізометричної конвертації через CoordinateConverter та перевірки прохідності через TileMap.

**Formal Specification:**
```
FUNCTION isBugCondition(operation)
  INPUT: operation of type NavigationGridOperation
  OUTPUT: boolean
  
  RETURN (operation.type IN ['init', 'worldToGrid', 'gridToWorld', 'updateFromBuildings', 'isWalkable'])
         AND operation.usesCellSize == true
         AND operation.usesRectangularMath == true
         AND NOT operation.usesCoordinateConverter
         AND NOT operation.usesTileMapPassability
END FUNCTION
```

### Examples

- **init(mapWidth, mapHeight, cellSize=16)**: Ініціалізує сітку з cellSize=16 пікселів, створюючи сітку 80x60 клітинок для мапи 1280x960 пікселів, замість використання розмірів TileMap (наприклад, 20x15 тайлів)

- **worldToGrid(screenX=640, screenY=320)**: Конвертує екранні координати через ділення на 16 (gridX=40, gridY=20), замість використання CoordinateConverter.screenToGrid() для отримання GridCoords(row, col)

- **gridToWorld(gridX=10, gridY=5)**: Конвертує координати сітки через множення на 16 (worldX=168, worldY=88), замість використання CoordinateConverter.gridToScreen() для отримання центру ізометричного тайлу

- **updateFromBuildings()**: Конвертує Rectangle будівлі в клітинки через worldToGrid() з прямокутною математикою, замість використання GridCoords будівлі для позначення тайлів як перешкод

- **isWalkable(gridX, gridY)**: Перевіряє прохідність через внутрішній масив cells[], замість делегування до TileMap.isPassable(row, col)

- **Edge case - getNeighbors(gridX=79, gridY=59)**: Повертає сусідів на межі сітки 80x60, але ці координати не відповідають реальним тайлам мапи

## Expected Behavior

### Preservation Requirements

**Unchanged Behaviors:**
- A* pathfinding алгоритм має продовжувати працювати з 8 напрямками руху та евристикою Manhattan distance
- PathfindingManager має продовжувати кешувати шляхи та інвалідувати кеш при зміні перешкод
- HandleClicks має продовжувати використовувати GridCoords для цільової позиції при командах руху
- NavigationGrid.getNeighbors() має продовжувати перевіряти прохідність сусідніх клітинок
- PathfindingManager має продовжувати обмежувати кількість обчислень на кадр (maxCalculationsPerFrame)

**Scope:**
Всі операції, що НЕ пов'язані з конвертацією координат або перевіркою прохідності, мають залишитися повністю незмінними. Це включає:
- Логіку A* алгоритму (відкритий/закритий список, обчислення f/g/h)
- Кешування шляхів у PathfindingManager
- Обробку черги запитів pathfinding
- Обмеження обчислень на кадр

## Hypothesized Root Cause

На основі аналізу коду, найбільш ймовірні причини:

1. **Legacy Rectangular Grid Design**: NavigationGrid була спроектована для прямокутної проекції з cellSize=16 пікселів і не була адаптована під ізометричну геометрію
   - Використовує cellSize для ділення/множення координат
   - Зберігає власний масив прохідності cells[] замість делегування до TileMap

2. **Missing CoordinateConverter Integration**: NavigationGrid не використовує CoordinateConverter для конвертації координат
   - worldToGrid() використовує просте ділення на cellSize
   - gridToWorld() використовує просте множення на cellSize
   - Немає інтеграції з GridCoords/ScreenCoords структурами

3. **Duplicate Passability Data**: NavigationGrid зберігає власний масив cells[] замість використання TileMap.isPassable()
   - updateFromBuildings() оновлює cells[] на основі Building.getRect()
   - isWalkable() перевіряє cells[] замість TileMap.isPassable()
   - Це створює дублювання даних та можливість десинхронізації

4. **Pixel-Based Initialization**: init() приймає розміри мапи в пікселях та cellSize, замість отримання розмірів безпосередньо з TileMap
   - Обчислює width/height через ділення на cellSize
   - Не синхронізується з реальними розмірами TileMap

## Correctness Properties

Property 1: Fault Condition - NavigationGrid Uses Isometric Tile Geometry

_For any_ operation where NavigationGrid конвертує координати або перевіряє прохідність, система SHALL використовувати GridCoords та CoordinateConverter для ізометричної конвертації, а TileMap.isPassable() для перевірки прохідності тайлів.

**Validates: Requirements 2.1, 2.2, 2.3, 2.4, 2.5, 2.6**

Property 2: Preservation - A* Pathfinding Logic Unchanged

_For any_ pathfinding operation що НЕ включає конвертацію координат або перевірку прохідності (A* алгоритм, кешування, обробка черги), система SHALL виконувати точно таку ж логіку як і до виправлення, зберігаючи всю функціональність pathfinding.

**Validates: Requirements 3.1, 3.2, 3.3, 3.4, 3.5**

## Fix Implementation

### Changes Required

Припускаючи, що наш аналіз причин правильний:

**File**: `cpp/src/pathfinding.h`

**Class**: `NavigationGrid`

**Specific Changes**:

1. **Remove cellSize and Pixel-Based Math**: Видалити cellSize member variable та всю піксельну математику
   - Видалити `int cellSize` з класу
   - Видалити параметр `cellSizePixels` з init()
   - Видалити всі операції ділення/множення на cellSize

2. **Replace worldToGrid() with ScreenCoords → GridCoords Conversion**: Замінити прямокутну конвертацію на ізометричну
   - Змінити сигнатуру: `GridCoords worldToGrid(const ScreenCoords& screen) const`
   - Використовувати `CoordinateConverter::screenToGrid(screen)`
   - Видалити bounds checking (CoordinateConverter вже повертає валідні GridCoords)

3. **Replace gridToWorld() with GridCoords → ScreenCoords Conversion**: Замінити прямокутну конвертацію на ізометричну
   - Змінити сигнатуру: `ScreenCoords gridToWorld(const GridCoords& grid) const`
   - Використовувати `CoordinateConverter::gridToScreen(grid)`
   - Видалити bounds checking та обчислення центру клітинки

4. **Replace cells[] with TileMap Reference**: Делегувати перевірку прохідності до TileMap
   - Додати `const TileMap* tileMap` member variable
   - Змінити init() для прийняття `const TileMap* map`
   - Видалити `std::vector<bool> cells`
   - Змінити isWalkable() для виклику `tileMap->isPassable(row, col)`

5. **Refactor updateFromBuildings() to Use GridCoords**: Використовувати GridCoords будівель замість Rectangle
   - Змінити логіку для роботи з Building.gridPos (GridCoords)
   - Видалити конвертацію Rectangle через worldToGrid()
   - Позначати тайли як перешкоди через GridCoords

6. **Update getNeighbors() to Use TileMap Bounds**: Використовувати розміри TileMap для перевірки меж
   - Змінити перевірку меж на `tileMap->isValidCoord(row, col)`
   - Використовувати `tileMap->getWidth()` та `tileMap->getHeight()`

7. **Update Getters**: Адаптувати getter методи
   - Змінити getWidth()/getHeight() для повернення `tileMap->getWidth()/getHeight()`
   - Видалити getCellSize() (більше не потрібен)

## Testing Strategy

### Validation Approach

Стратегія тестування використовує двофазний підхід: спочатку виявити counterexamples, що демонструють баг на невиправленому коді, потім перевірити, що виправлення працює коректно та зберігає існуючу поведінку.

### Exploratory Fault Condition Checking

**Goal**: Виявити counterexamples, що демонструють баг ДО впровадження виправлення. Підтвердити або спростувати аналіз причин. Якщо спростуємо, потрібно буде переформулювати гіпотезу.

**Test Plan**: Написати тести, що порівнюють результати NavigationGrid з очікуваними результатами CoordinateConverter та TileMap. Запустити ці тести на НЕВИПРАВЛЕНОМУ коді для спостереження помилок та розуміння причин.

**Test Cases**:
1. **Coordinate Conversion Mismatch Test**: Порівняти NavigationGrid.worldToGrid() з CoordinateConverter.screenToGrid() для різних екранних координат (провалиться на невиправленому коді - різні результати)
2. **Grid to World Mismatch Test**: Порівняти NavigationGrid.gridToWorld() з CoordinateConverter.gridToScreen() для різних GridCoords (провалиться на невиправленому коді - різні результати)
3. **Passability Mismatch Test**: Порівняти NavigationGrid.isWalkable() з TileMap.isPassable() для різних тайлів (може провалитися на невиправленому коді - різні результати через updateFromBuildings)
4. **Grid Size Mismatch Test**: Порівняти NavigationGrid.getWidth()/getHeight() з TileMap.getWidth()/getHeight() (провалиться на невиправленому коді - різні розміри)

**Expected Counterexamples**:
- NavigationGrid повертає координати сітки, що не відповідають GridCoords з CoordinateConverter
- Можливі причини: використання cellSize=16 замість ізометричної конвертації, прямокутна математика замість ізометричної, дублювання даних прохідності

### Fix Checking

**Goal**: Перевірити, що для всіх операцій, де виконується умова багу, виправлена функція виробляє очікувану поведінку.

**Pseudocode:**
```
FOR ALL operation WHERE isBugCondition(operation) DO
  result := NavigationGrid_fixed.execute(operation)
  ASSERT usesCoordinateConverter(result)
  ASSERT usesTileMapPassability(result)
  ASSERT NOT usesCellSize(result)
  ASSERT NOT usesRectangularMath(result)
END FOR
```

### Preservation Checking

**Goal**: Перевірити, що для всіх операцій, де умова багу НЕ виконується, виправлена функція виробляє той самий результат, що й оригінальна функція.

**Pseudocode:**
```
FOR ALL operation WHERE NOT isBugCondition(operation) DO
  ASSERT NavigationGrid_original.execute(operation) = NavigationGrid_fixed.execute(operation)
END FOR
```

**Testing Approach**: Property-based testing рекомендується для preservation checking, тому що:
- Він автоматично генерує багато тестових випадків по всьому домену вхідних даних
- Він виявляє крайні випадки, які ручні unit тести можуть пропустити
- Він надає сильні гарантії, що поведінка незмінна для всіх операцій, що не пов'язані з багом

**Test Plan**: Спостерігати поведінку на НЕВИПРАВЛЕНОМУ коді спочатку для A* pathfinding, кешування та обробки черги, потім написати property-based тести, що захоплюють цю поведінку.

**Test Cases**:
1. **A* Algorithm Preservation**: Спостерігати, що A* знаходить шляхи на невиправленому коді, потім написати тест для перевірки, що ті самі шляхи знаходяться після виправлення (з урахуванням нової системи координат)
2. **Path Caching Preservation**: Спостерігати, що кешування працює на невиправленому коді, потім написати тест для перевірки, що кешування продовжує працювати після виправлення
3. **Request Queue Preservation**: Спостерігати, що черга запитів обробляється на невиправленому коді, потім написати тест для перевірки, що обробка продовжує працювати після виправлення
4. **Neighbor Generation Preservation**: Спостерігати, що getNeighbors() повертає 8 напрямків на невиправленому коді, потім написати тест для перевірки, що логіка 8 напрямків зберігається після виправлення

### Unit Tests

- Тестувати конвертацію координат для різних GridCoords та ScreenCoords
- Тестувати перевірку прохідності для різних типів місцевості
- Тестувати крайні випадки (координати за межами мапи, негативні координати)
- Тестувати, що розміри сітки відповідають розмірам TileMap

### Property-Based Tests

- Генерувати випадкові GridCoords та перевіряти, що worldToGrid(gridToWorld(grid)) == grid
- Генерувати випадкові ScreenCoords та перевіряти, що gridToWorld(worldToGrid(screen)) близько до screen
- Генерувати випадкові стани гри та перевіряти, що pathfinding знаходить валідні шляхи
- Тестувати, що всі операції, що не пов'язані з конвертацією координат, виробляють ті самі результати

### Integration Tests

- Тестувати повний flow pathfinding з новою системою координат
- Тестувати, що юніти рухаються по центрах ізометричних тайлів
- Тестувати, що mouse clicks коректно конвертуються в GridCoords для pathfinding
- Тестувати, що будівлі коректно позначаються як перешкоди в новій системі координат
