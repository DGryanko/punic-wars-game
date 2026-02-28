# 🎮 Швидкий старт: Генератор ізометричної тайлової мапи

## ✅ Що готово

Повністю функціональний генератор ізометричних тайлових карт з усіма функціями!

### Реалізовані компоненти:

1. ✅ **Система координат** (`coordinates.h`)
   - Конвертація Grid ↔ Screen
   - Ізометрична математика (2:1 ratio)

2. ✅ **Типи місцевості** (`terrain.h`)
   - 4 типи: Трава, Вода, Пісок, Дорога
   - Властивості: прохідність, швидкість

3. ✅ **Тайлова карта** (`tilemap.h/cpp`)
   - Ефективне зберігання
   - Валідація координат
   - Статистика карти

4. ✅ **Perlin Noise** (`noise.h/cpp`)
   - Багатооктавний шум
   - Налаштовувані параметри

5. ✅ **Генератор карти** (`map_generator.h/cpp`)
   - Процедурна генерація
   - Пошук стартових позицій
   - Налаштовувані параметри

6. ✅ **Ізометричний рендерер** (`isometric_renderer.h/cpp`)
   - Painter's Algorithm
   - Viewport Culling
   - Підсвічування тайлів

7. ✅ **Серіалізація** (`map_serializer.h/cpp`)
   - Збереження/завантаження карт
   - Текстовий формат

## 🚀 Як запустити

### Крок 1: Компіляція

```bash
cd cpp
compile_tilemap.bat
```

Або вручну:
```bash
g++ -std=c++17 -O2 -Isrc -o tilemap_test.exe src/tilemap_test.cpp src/tilemap/*.cpp -lraylib -lopengl32 -lgdi32 -lwinmm
```

### Крок 2: Запуск

```bash
tilemap_test.exe
```

## 🎨 Тайлсет

Генератор працює в двох режимах:

### 1. Debug режим (без тайлсету)
- Автоматично активується, якщо тайлсет не знайдено
- Малює кольорові ромби для кожного типу місцевості
- Зелений = Трава, Синій = Вода, Бежевий = Пісок, Коричневий = Дорога

### 2. Текстурний режим (з тайлсетом)
Створіть файл `assets/isometric_tileset.png` розміром 256x128:

```
[Трава 128x64] [Вода 128x64]
[Пісок 128x64] [Дорога 128x64]
```

Або використайте свій ізометричний тайлсет з 4 тайлами!

## 🎮 Керування

- **Стрілки** ← ↑ → ↓ - Прокрутка камери
- **Колесо миші** - Зум (0.5x - 2.0x)
- **SPACE** - Згенерувати нову карту
- **S** - Зберегти карту в `saved_map.txt`
- **L** - Завантажити карту з `saved_map.txt`
- **Миша** - Підсвічування тайлу

## 📊 Що відображається

- FPS (кадри за секунду)
- Seed (зерно генерації)
- Passable % (відсоток прохідних тайлів)
- Zoom (поточний зум)
- Hovered (координати тайлу під мишею)
- Червоне коло - Стартова позиція 1 (Рим)
- Синє коло - Стартова позиція 2 (Карфаген)

## 💻 Використання в коді

### Мінімальний приклад:

```cpp
#include "tilemap/tilemap_generator.h"

// Генерація
MapGenerator generator(12345);
TileMap map = generator.generate(50, 50);

// Рендеринг
IsometricRenderer renderer;
renderer.render(map);
```

### Повний приклад:

```cpp
#include "raylib.h"
#include "tilemap/tilemap_generator.h"

int main() {
    InitWindow(1024, 768, "My Game");
    
    // Генерація карти
    MapGenerator generator(time(nullptr));
    GenerationParams params;
    params.water_level = 0.3f;
    TileMap map = generator.generate(50, 50, params);
    
    // Стартові позиції
    auto positions = generator.findStartPositions(map);
    
    // Рендерер
    IsometricRenderer renderer;
    renderer.loadTileset("assets/tileset.png");
    
    // Камера
    Camera2D camera = {0};
    camera.offset = {512, 384};
    renderer.setCamera(camera);
    
    while (!WindowShouldClose()) {
        // Прокрутка
        if (IsKeyDown(KEY_RIGHT)) camera.target.x += 5;
        renderer.setCamera(camera);
        
        BeginDrawing();
        ClearBackground(BLACK);
        
        BeginMode2D(camera);
        renderer.render(map);
        EndMode2D();
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
```

## 🔧 Налаштування генерації

```cpp
GenerationParams params;
params.water_level = 0.3f;    // Більше води
params.sand_level = 0.5f;     // Більше піску
params.grass_level = 0.9f;    // Більше трави
params.noise_scale = 0.05f;   // Більші області (менше деталей)
params.octaves = 6;           // Більше деталізації

TileMap map = generator.generate(100, 100, params);
```

## 📁 Структура файлів

```
cpp/
├── src/
│   ├── tilemap/
│   │   ├── coordinates.h              ✅ Конвертація координат
│   │   ├── terrain.h                  ✅ Типи місцевості
│   │   ├── tilemap.h/cpp              ✅ Структура карти
│   │   ├── noise.h/cpp                ✅ Perlin Noise
│   │   ├── map_generator.h/cpp        ✅ Генератор
│   │   ├── isometric_renderer.h/cpp   ✅ Рендерер
│   │   ├── map_serializer.h/cpp       ✅ Серіалізація
│   │   └── tilemap_generator.h        ✅ Головний header
│   └── tilemap_test.cpp               ✅ Тестовий додаток
├── compile_tilemap.bat                ✅ Скрипт компіляції
├── TILEMAP_README.md                  ✅ Повна документація
└── TILEMAP_QUICKSTART.md              ✅ Цей файл
```

## 🎯 Наступні кроки

### 1. Інтеграція в main.cpp

Додайте в існуючу гру:

```cpp
#include "tilemap/tilemap_generator.h"

// В main():
MapGenerator map_gen(time(nullptr));
TileMap game_map = map_gen.generate(50, 50);
IsometricRenderer map_renderer;
map_renderer.loadTileset("assets/isometric_tileset.png");

// В game loop:
map_renderer.render(game_map);
```

### 2. Адаптація юнітів

Змініть систему юнітів на використання координат сітки:

```cpp
struct Unit {
    GridCoords position;  // Замість x, y
    
    void moveTo(GridCoords target, const TileMap& map) {
        if (map.isPassable(target.row, target.col)) {
            position = target;
        }
    }
    
    void render() {
        ScreenCoords screen = CoordinateConverter::gridToScreen(position);
        DrawTexture(sprite, screen.x, screen.y, WHITE);
    }
};
```

### 3. Додайте свої тайли

Створіть ізометричні тайли 128x64 та зберіть їх у тайлсет 256x128.

## ❓ Troubleshooting

### Помилка компіляції
- Перевірте, що Raylib встановлено
- Перевірте шлях до Raylib в Makefile

### Чорний екран
- Перевірте, що камера налаштована
- Спробуйте змінити `camera.offset`

### Тайли не відображаються
- Генератор працює в debug режимі (кольорові ромби)
- Додайте тайлсет в `assets/isometric_tileset.png`

### Низька продуктивність
- Viewport culling автоматично активний
- Зменшіть розмір карти (50x50 замість 100x100)

## 📝 Приклад виводу

```
[INFO] Generated map 50x50 with 68.2% passable tiles
[INFO] Start positions: (12, 8) and (38, 42)
[INFO] Loaded tileset: assets/isometric_tileset.png (256x128)
[INFO] Saved map to: saved_map.txt
```

## 🎉 Готово!

Генератор повністю функціональний та готовий до використання!

Насолоджуйтесь створенням ізометричних карт! 🗺️
