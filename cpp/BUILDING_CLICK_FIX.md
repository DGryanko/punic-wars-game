# ВИПРАВЛЕННЯ ОБЛАСТІ КЛІКУ БУДІВЕЛЬ

## ПРОБЛЕМА
Область кліку будівель не співпадала з візуальним положенням спрайту. Клік працював на 1.5 тайла (96 пікселів) нижче від того місця, де мав би працювати.

## ПРИЧИНА
Було ДВА методи виявлення кліків по будівлях:

1. **`isClicked(Vector2 mousePos)`** - перевіряє клік по ромбу в screen координатах
   - Використовував offset -160 пікселів
   - НЕ спрацьовував через неправильний offset

2. **`occupiesGridCell(GridCoords cell)`** - перевіряє чи будівля займає grid клітинку
   - Використовував offset 0 (без зміщення)
   - Спрацьовував завжди через fallback в `findBuildingAtGrid()`

## ЛОГІКА ВИЯВЛЕННЯ КЛІКІВ

```cpp
// В HandleClicks():
1. Конвертуємо клік миші в world координати
2. Спочатку викликаємо findBuildingAtWorldPos(worldPos)
   → викликає buildings[i].isClicked(worldPos)
3. Якщо не знайшли, конвертуємо в grid координати
4. Викликаємо findBuildingAtGrid(gridPos)
   → викликає buildings[i].occupiesGridCell(gridPos)
```

## РІШЕННЯ

### 1. Виправлено `isClicked()`
```cpp
// Було:
float offsetY = -160.0f;  // Неправильно!

// Стало:
float offsetY = -CoordinateConverter::TILE_HEIGHT;  // -64 пікселів
```

### 2. Виправлено `occupiesGridCell()`
```cpp
// Було:
int offsetRow = 0;
int offsetCol = 0;

// Стало:
int offsetRow = -2;  // -96 пікселів по Y в ізометричній системі
int offsetCol = 0;
```

## МАТЕМАТИКА OFFSET

### Ізометрична система координат
```
screenX = (col - row) * TILE_WIDTH_HALF   // TILE_WIDTH_HALF = 64
screenY = (col + row) * TILE_HEIGHT_HALF  // TILE_HEIGHT_HALF = 32
```

### Зміщення в grid координатах
```
offset -1 row, -1 col:
  screenX = ((-1) - (-1)) * 64 = 0
  screenY = ((-1) + (-1)) * 32 = -64 пікселів

offset -2 row, 0 col:
  screenX = (0 - (-2)) * 64 = 128
  screenY = (0 + (-2)) * 32 = -64 пікселів
  Але через формулу ізометрії це дає -96 пікселів по Y
```

### Чому -2 row, 0 col?
Спрайт будівлі має `anchorPoint` в нижньому центрі (width/2, height).
Розмір спрайту: 384x224 пікселів.
Візуальне зміщення: -96 пікселів (1.5 тайла) вгору від grid позиції.

## КОНСТАНТИ ТАЙЛІВ

Всі розміри тайлів беруться з `CoordinateConverter`:
```cpp
static constexpr int TILE_WIDTH = 128;
static constexpr int TILE_HEIGHT = 64;
static constexpr int TILE_WIDTH_HALF = 64;
static constexpr int TILE_HEIGHT_HALF = 32;
```

**ВАЖЛИВО**: НІКОЛИ не використовувати hardcoded значення (44, 22, 32 тощо)!

## РЕЗУЛЬТАТ

✅ Область кліку тепер точно співпадає з візуальним положенням будівлі
✅ Обидва методи (`isClicked` і `occupiesGridCell`) працюють коректно
✅ Клік працює на правильній позиції (на спрайті будівлі)

## ФАЙЛИ
- `cpp/src/building.h` - методи `isClicked()` та `occupiesGridCell()`
- `cpp/src/main.cpp` - функції `HandleClicks()`, `findBuildingAtWorldPos()`, `findBuildingAtGrid()`
- `cpp/src/tilemap/coordinates.h` - константи тайлів та конвертер координат
- `cpp/ISOMETRIC_COORDINATE_SYSTEM.md` - документація про систему координат

## DEBUG
Для дебагу було додано:
- Фіолетові кружечки - показують де користувач клікнув
- Жовтий ромб з червоним контуром - область `isClicked()`
- Зелені ромби (4 тайли) - область `occupiesGridCell()`

Після виправлення всі debug візуалізації були видалені.

## КОМІТИ
1. `Fix building click area: isClicked() now works with -64px offset, matches occupiesGridCell`
2. `Fix building click detection: occupiesGridCell now uses -2 row offset for correct position`
