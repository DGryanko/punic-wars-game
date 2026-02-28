# Генератор ізометричної тайлової мапи

Повнофункціональний генератор ізометричних тайлових карт для гри "Пунічні війни: Табори".

## Особливості

✅ **Ізометрична проекція** - Правильна математика конвертації координат (2:1 ratio)
✅ **Процедурна генерація** - Perlin Noise для природних ландшафтів
✅ **4 типи місцевості** - Трава, вода, пісок, дорога
✅ **Painter's Algorithm** - Правильне сортування для перекриття тайлів
✅ **Viewport Culling** - Оптимізація рендерингу (тільки видимі тайли)
✅ **Стартові позиції** - Автоматичний пошук збалансованих позицій для двох фракцій
✅ **Серіалізація** - Збереження та завантаження карт
✅ **Інтерактивність** - Підсвічування тайлів, прокрутка, зум

## Структура файлів

```
cpp/src/tilemap/
├── coordinates.h           - Структури координат та конвертер
├── terrain.h              - Типи місцевості та їх властивості
├── tilemap.h/cpp          - Структура тайлової карти
├── noise.h/cpp            - Генератор Perlin Noise
├── map_generator.h/cpp    - Генератор карти
├── isometric_renderer.h/cpp - Ізометричний рендерер
├── map_serializer.h/cpp   - Серіалізація карт
└── tilemap_generator.h    - Головний header (включає все)
```

## Компіляція

### Windows (MinGW)

```bash
cd cpp
mingw32-make -f Makefile_tilemap
```

### Запуск тесту

```bash
tilemap_test.exe
```

## Використання в коді

### Базовий приклад

```cpp
#include "tilemap/tilemap_generator.h"

// Створення генератора
MapGenerator generator(12345);  // seed

// Генерація карти 50x50
TileMap map = generator.generate(50, 50);

// Пошук стартових позицій
auto positions = generator.findStartPositions(map);

// Рендеринг
IsometricRenderer renderer;
renderer.loadTileset("assets/isometric_tileset.png");
renderer.render(map);
```

### Налаштування параметрів

```cpp
GenerationParams params;
params.water_level = 0.3f;    // 30% води
params.sand_level = 0.5f;     // 20% піску
params.grass_level = 0.9f;    // 40% трави
params.noise_scale = 0.1f;    // Масштаб деталей
params.octaves = 4;           // Деталізація

TileMap map = generator.generate(50, 50, params);
```

### Збереження та завантаження

```cpp
// Збереження
MapSerializer::saveToFile(map, "my_map.txt");

// Завантаження
TileMap loaded_map = MapSerializer::loadFromFile("my_map.txt");
```

## Формат тайлсету

Тайлсет повинен бути PNG файлом розміром 256x128 пікселів (сітка 2x2):

```
[Трава 128x64] [Вода 128x64]
[Пісок 128x64] [Дорога 128x64]
```

Розташування:
- Трава: (0, 0, 128, 64)
- Вода: (128, 0, 128, 64)
- Пісок: (0, 64, 128, 64)
- Дорога: (128, 64, 128, 64)

## Керування в тестовому додатку

- **Стрілки** - Прокрутка камери
- **Колесо миші** - Зум
- **SPACE** - Згенерувати нову карту
- **S** - Зберегти карту
- **L** - Завантажити карту

## Математика ізометричної проекції

### Grid → Screen

```
screenX = (col - row) * 64
screenY = (col + row) * 32
```

### Screen → Grid

```
col = (screenX / 64 + screenY / 32) / 2
row = (screenY / 32 - screenX / 64) / 2
```

## Властивості місцевості

| Тип | Прохідність | Модифікатор швидкості | Колір (debug) |
|-----|-------------|----------------------|---------------|
| Трава | Так | 1.0x | Зелений |
| Вода | Ні | 0.0x | Синій |
| Пісок | Так | 1.0x | Бежевий |
| Дорога | Так | 1.5x | Коричневий |

## Продуктивність

- Генерація карти 50x50: < 100мс
- Генерація карти 100x100: < 500мс
- Рендеринг: 60 FPS з viewport culling

## Інтеграція з існуючою грою

### 1. Додати include

```cpp
#include "tilemap/tilemap_generator.h"
```

### 2. Згенерувати карту при запуску

```cpp
MapGenerator generator(time(nullptr));
TileMap map = generator.generate(50, 50);
```

### 3. Адаптувати систему юнітів

```cpp
struct Unit {
    GridCoords grid_pos;  // Замість екранних координат
    
    ScreenCoords getScreenPos() const {
        return CoordinateConverter::gridToScreen(grid_pos);
    }
    
    void moveTo(GridCoords target) {
        if (map.isPassable(target.row, target.col)) {
            grid_pos = target;
        }
    }
};
```

## Приклад виводу

```
[INFO] Generated map 50x50 with 68.2% passable tiles
[INFO] Start positions: (12, 8) and (38, 42)
[INFO] Loaded tileset: assets/isometric_tileset.png (256x128)
```

## Troubleshooting

### Тайлсет не завантажується
- Перевірте, що файл існує: `assets/isometric_tileset.png`
- Система автоматично перейде в debug режим (кольорові ромби)

### Карта не відображається
- Перевірте, що камера налаштована правильно
- Спробуйте збільшити zoom або змінити offset

### Низька продуктивність
- Зменшіть розмір карти
- Viewport culling автоматично оптимізує рендеринг

## Ліцензія

Частина проекту "Пунічні війни: Табори"
