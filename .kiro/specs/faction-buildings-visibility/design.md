# Faction Buildings Visibility Bugfix Design

## Overview

Головні будівлі фракцій (HQ Rome та HQ Carthage) не відображаються на мапі після вибору фракції, хоча система `FactionSpawner` створює їх через `spawnFactionHQs()`. Проблема полягає в неузгодженості координат між `Building::init()` та `BuildingPlacer::placeBuilding()`. Метод `Building::init()` встановлює `position` та синхронізує `x, y` через `syncScreenCoords()` (використовує `gridToScreen`), але потім `BuildingPlacer::placeBuilding()` перезаписує `x, y` використовуючи `gridToScreenWithOffset()`, не оновлюючи `position`. Це призводить до того, що `Building::getScreenPosition()` повертає координати, які не відповідають фактичним `x, y`, і будівлі рендеряться за межами видимої області або в некоректних позиціях.

Стратегія виправлення: забезпечити узгодженість між `position` (grid coordinates) та `x, y` (screen coordinates) шляхом оновлення `BuildingPlacer::placeBuilding()` для правильного встановлення `position` замість прямого перезапису `x, y`.

## Glossary

- **Bug_Condition (C)**: Умова, що викликає баг - коли `BuildingPlacer::placeBuilding()` перезаписує `x, y` координати будівлі без оновлення `position`, створюючи неузгодженість координат
- **Property (P)**: Бажана поведінка - будівлі мають відображатись на мапі з узгодженими координатами, де `getScreenPosition()` повертає ті самі координати, що зберігаються в `x, y`
- **Preservation**: Існуюча поведінка розміщення будівель гравцем через `BuildingPlacer`, рендеринг юнітів та ресурсів, керування камерою - все це має залишатись незмінним
- **Building::position**: Grid координати будівлі (GridCoords), які є джерелом істини для позиції
- **Building::x, y**: Screen координати будівлі (compatibility layer), які мають синхронізуватись з `position`
- **syncScreenCoords()**: Метод, що синхронізує `x, y` з `position` використовуючи `gridToScreen()`
- **gridToScreen()**: Конвертує grid координати в screen координати (центр тайлу)
- **gridToScreenWithOffset()**: Конвертує grid координати в screen координати з offset (центр мінус TILE_WIDTH_HALF)
- **FactionSpawner**: Система автоматичного спавну головних будівель фракцій
- **BuildingPlacer**: Система розміщення будівель на тайловій сітці з валідацією

## Bug Details

### Fault Condition

Баг проявляється коли `FactionSpawner::spawnFactionHQs()` створює головні будівлі фракцій під час ініціалізації гри. Метод `Building::init()` встановлює `position` та викликає `syncScreenCoords()`, який конвертує grid координати в screen координати через `gridToScreen()`. Потім `BuildingPlacer::placeBuilding()` перезаписує `x, y` використовуючи `gridToScreenWithOffset()`, але не оновлює `position`. Це створює неузгодженість: `position` містить оригінальні grid координати, але `x, y` містять інші screen координати. Коли `Building::draw()` викликає `getScreenPosition()`, він конвертує `position` через `gridToScreen()`, отримуючи координати, які відрізняються від `x, y` на 64 пікселі по осі X.

**Formal Specification:**
```
FUNCTION isBugCondition(building)
  INPUT: building of type Building
  OUTPUT: boolean
  
  screenPosFromPosition := CoordinateConverter::gridToScreen(building.position)
  screenPosFromXY := ScreenCoords(building.x, building.y)
  
  RETURN building was created by FactionSpawner
         AND BuildingPlacer::placeBuilding was called on building
         AND screenPosFromPosition.x != screenPosFromXY.x
         AND abs(screenPosFromPosition.x - screenPosFromXY.x) == 64
END FUNCTION
```

### Examples

- **Rome HQ at tile (5, 5)**: `Building::init()` встановлює `position = (5, 5)` та `syncScreenCoords()` встановлює `x = 0, y = 320` (через `gridToScreen`). Потім `BuildingPlacer::placeBuilding()` перезаписує `x = -64, y = 320` (через `gridToScreenWithOffset`). Коли `draw()` викликає `getScreenPosition()`, він повертає `(0, 320)`, але фактичні координати в `x, y` це `(-64, 320)`. Будівля рендериться не там, де очікується.

- **Carthage HQ at tile (45, 45)**: `Building::init()` встановлює `position = (45, 45)` та `x = 0, y = 2880`. `BuildingPlacer::placeBuilding()` перезаписує `x = -64, y = 2880`. `getScreenPosition()` повертає `(0, 2880)`, але `x, y` містять `(-64, 2880)`. Неузгодженість координат.

- **Player-placed building at tile (10, 10)**: Гравець розміщує будівлю через UI. `BuildingPlacer::placeBuilding()` викликається, але `Building::init()` вже встановив правильні координати. Та сама проблема неузгодженості виникає.

- **Edge case - building at tile (0, 0)**: `gridToScreen(0, 0)` повертає `(0, 0)`, `gridToScreenWithOffset(0, 0)` повертає `(-64, 0)`. Будівля може рендеритись за межами видимої області.

## Expected Behavior

### Preservation Requirements

**Unchanged Behaviors:**
- Розміщення будівель гравцем через `BuildingPlacer` під час гри має продовжувати працювати коректно
- Рендеринг юнітів та ресурсів через `RenderQueue` має залишатись незмінним
- Керування камерою (WASD, стрілки, миша, зум) має продовжувати працювати плавно
- Система pathfinding та блокування тайлів будівлями має залишатись незмінною
- Перевірки валідності координат та прохідності тайлів в `BuildingPlacer` мають залишатись незмінними
- Очищення будівель через `buildings.clear()` при перезапуску гри має продовжувати працювати

**Scope:**
Всі входи, які НЕ включають створення будівель через `FactionSpawner` або `BuildingPlacer`, мають бути повністю незачеплені цим виправленням. Це включає:
- Взаємодію з UI (меню, налаштування, пауза)
- Рендеринг інших об'єктів (юніти, ресурси, тайли)
- Систему виробництва юнітів в будівлях
- Систему вибору будівель та відображення інформації

## Hypothesized Root Cause

На основі аналізу коду, найбільш ймовірні причини:

1. **Coordinate Mismatch in BuildingPlacer**: `BuildingPlacer::placeBuilding()` перезаписує `building.x` та `building.y` використовуючи `gridToScreenWithOffset()`, але не оновлює `building.position`. Це створює неузгодженість між `position` (джерело істини) та `x, y` (compatibility layer).
   - `Building::init()` встановлює `position` та викликає `syncScreenCoords()`, який використовує `gridToScreen()`
   - `BuildingPlacer::placeBuilding()` потім перезаписує `x, y` через `gridToScreenWithOffset()`, який віднімає 64 пікселі
   - `Building::getScreenPosition()` конвертує `position` через `gridToScreen()`, ігноруючи `x, y`
   - Результат: `getScreenPosition()` повертає координати, які відрізняються від `x, y` на 64 пікселі

2. **Incorrect Use of gridToScreenWithOffset**: `BuildingPlacer` використовує `gridToScreenWithOffset()` для встановлення `x, y`, але цей метод призначений для малювання тайлів (верхній кут), а не для позиціонування будівель (центр).

3. **Missing Position Update**: `BuildingPlacer::placeBuilding()` не оновлює `building.position` після встановлення `x, y`, порушуючи інваріант, що `position` є джерелом істини.

4. **Rendering Uses getScreenPosition()**: `Building::draw()` викликає `getScreenPosition()`, який конвертує `position`, а не використовує `x, y` напряму. Це означає, що будівлі рендеряться не там, де `BuildingPlacer` їх розмістив.

## Correctness Properties

Property 1: Fault Condition - Coordinate Consistency for Faction HQs

_For any_ building created by `FactionSpawner::spawnFactionHQs()` and placed by `BuildingPlacer::placeBuilding()`, the fixed code SHALL ensure that `getScreenPosition()` returns screen coordinates that match the `x, y` fields, eliminating the 64-pixel offset discrepancy and ensuring buildings render at their intended positions on the map.

**Validates: Requirements 2.1, 2.3, 2.4**

Property 2: Preservation - Non-Faction Building Behavior

_For any_ building placement that does NOT involve `FactionSpawner` (player-placed buildings, buildings created through other systems), the fixed code SHALL produce exactly the same coordinate behavior as the original code, preserving all existing functionality for manual building placement and rendering.

**Validates: Requirements 3.2, 3.3, 3.5**

## Fix Implementation

### Changes Required

Assuming our root cause analysis is correct:

**File**: `cpp/src/building_placer.h`

**Function**: `BuildingPlacer::placeBuilding()`

**Specific Changes**:
1. **Remove Direct x, y Assignment**: Видалити рядки, що перезаписують `building.x` та `building.y` через `gridToScreenWithOffset()`
   - Видалити: `building.x = (int)screenPos.x;`
   - Видалити: `building.y = (int)screenPos.y;`

2. **Update building.position Instead**: Встановити `building.position` з переданих grid координат
   - Додати: `building.position = GridCoords(row, col);`

3. **Call syncScreenCoords()**: Викликати `building.syncScreenCoords()` для синхронізації `x, y` з `position`
   - Додати: `building.syncScreenCoords();`

4. **Update tile_row, tile_col**: Зберегти тайлові координати (compatibility layer)
   - Залишити: `building.tile_row = row;`
   - Залишити: `building.tile_col = col;`

5. **Verify Coordinate Consistency**: Після змін, `building.x, y` будуть синхронізовані з `building.position` через `gridToScreen()`, і `getScreenPosition()` поверне ті самі координати

**Alternative Approach** (if the above doesn't work):
Якщо проблема в тому, що `Building::draw()` використовує `getScreenPosition()` замість `x, y`, можна змінити `Building::draw()` для використання `x, y` напряму. Але це менш бажано, оскільки порушує принцип, що `position` є джерелом істини.

## Testing Strategy

### Validation Approach

Стратегія тестування використовує двофазний підхід: спочатку виявити контрприклади, що демонструють баг на невиправленому коді, потім перевірити, що виправлення працює коректно та зберігає існуючу поведінку.

### Exploratory Fault Condition Checking

**Goal**: Виявити контрприклади, що демонструють баг ДО впровадження виправлення. Підтвердити або спростувати аналіз першопричини. Якщо спростуємо, потрібно буде переформулювати гіпотезу.

**Test Plan**: Написати тести, що створюють будівлі через `FactionSpawner` та перевіряють узгодженість координат. Запустити ці тести на НЕВИПРАВЛЕНОМУ коді для спостереження за помилками та розуміння першопричини.

**Test Cases**:
1. **Coordinate Consistency Test**: Створити HQ через `FactionSpawner`, перевірити що `getScreenPosition()` повертає ті самі координати, що `x, y` (провалиться на невиправленому коді)
2. **Rome HQ Visibility Test**: Створити Rome HQ, перевірити що він рендериться в межах видимої області карти (може провалитись на невиправленому коді)
3. **Carthage HQ Visibility Test**: Створити Carthage HQ, перевірити що він рендериться в межах видимої області карти (може провалитись на невиправленому коді)
4. **Offset Discrepancy Test**: Перевірити різницю між `getScreenPosition().x` та `x` - має бути 64 пікселі на невиправленому коді (провалиться на невиправленому коді)

**Expected Counterexamples**:
- `getScreenPosition()` повертає координати, які відрізняються від `x, y` на 64 пікселі по осі X
- Можливі причини: `BuildingPlacer` використовує `gridToScreenWithOffset()` замість оновлення `position`, `Building::draw()` використовує `getScreenPosition()` замість `x, y`

### Fix Checking

**Goal**: Перевірити, що для всіх входів, де умова багу виконується, виправлена функція виробляє очікувану поведінку.

**Pseudocode:**
```
FOR ALL building WHERE isBugCondition(building) DO
  result := BuildingPlacer::placeBuilding_fixed(building, row, col)
  screenPosFromPosition := building.getScreenPosition()
  screenPosFromXY := ScreenCoords(building.x, building.y)
  ASSERT screenPosFromPosition.x == screenPosFromXY.x
  ASSERT screenPosFromPosition.y == screenPosFromXY.y
END FOR
```

### Preservation Checking

**Goal**: Перевірити, що для всіх входів, де умова багу НЕ виконується, виправлена функція виробляє той самий результат, що й оригінальна функція.

**Pseudocode:**
```
FOR ALL building WHERE NOT isBugCondition(building) DO
  original_x := building.x
  original_y := building.y
  BuildingPlacer::placeBuilding_fixed(building, row, col)
  ASSERT building.x == original_x OR coordinate_system_is_consistent
  ASSERT building.y == original_y OR coordinate_system_is_consistent
END FOR
```

**Testing Approach**: Property-based testing рекомендується для preservation checking, оскільки:
- Він автоматично генерує багато тестових випадків по всьому домену входів
- Він виявляє крайні випадки, які ручні unit тести можуть пропустити
- Він надає сильні гарантії, що поведінка незмінна для всіх не-багових входів

**Test Plan**: Спостерігати поведінку на НЕВИПРАВЛЕНОМУ коді спочатку для розміщення будівель гравцем, потім написати property-based тести, що захоплюють цю поведінку.

**Test Cases**:
1. **Player Building Placement Preservation**: Спостерігати, що розміщення будівель гравцем працює коректно на невиправленому коді, потім написати тест для перевірки, що це продовжує працювати після виправлення
2. **Pathfinding Grid Update Preservation**: Спостерігати, що pathfinding grid оновлюється коректно на невиправленому коді, потім написати тест для перевірки, що це продовжує працювати після виправлення
3. **Occupied Tiles Tracking Preservation**: Спостерігати, що відстеження зайнятих тайлів працює коректно на невиправленому коді, потім написати тест для перевірки, що це продовжує працювати після виправлення

### Unit Tests

- Тест координатної узгодженості для HQ Rome
- Тест координатної узгодженості для HQ Carthage
- Тест, що `getScreenPosition()` повертає ті самі координати, що `x, y` після виправлення
- Тест крайніх випадків (будівля на тайлі (0, 0), будівля на краю карти)
- Тест, що камера фокусується на HQ гравця після виправлення

### Property-Based Tests

- Генерувати випадкові grid координати та перевіряти узгодженість координат після `placeBuilding()`
- Генерувати випадкові конфігурації будівель та перевіряти збереження поведінки розміщення гравцем
- Тестувати, що всі будівлі, створені через `FactionSpawner`, відображаються в межах видимої області карти

### Integration Tests

- Тест повного потоку гри: вибір фракції → створення HQ → фокусування камери → відображення на мапі
- Тест перемикання між фракціями та перевірка, що HQ відображаються коректно для обох
- Тест, що візуальний зворотний зв'язок (вибір будівлі, прогрес виробництва) працює коректно після виправлення
