# Дизайн документ: Генератор ізометричної тайлової мапи

## Огляд

Генератор ізометричної тайлової мапи - це система для процедурної генерації ігрових карт у форматі ізометричної проекції для гри "Пунічні війни: Табори". Система використовує алгоритм Perlin Noise для створення природних ландшафтів та забезпечує ефективний рендеринг з використанням Raylib.

### Ключові особливості
- Ізометрична проекція (2:1 ratio) для візуальної глибини
- Процедурна генерація з підтримкою seed для детермінованості
- Ефективний рендеринг з viewport culling
- Чотири типи місцевості: трава, вода, пісок, дорога
- Інтеграція з існуючою грою через Raylib

## Архітектура

### Загальна структура

```
tilemap_generator.h
├── TileMap (структура даних карти)
├── TerrainType (enum типів місцевості)
├── NoiseGenerator (Perlin/Simplex noise)
├── IsometricRenderer (рендеринг ізометрії)
├── CoordinateConverter (конвертація координат)
└── MapSerializer (збереження/завантаження)
```

### Математика ізометричної проекції

Ізометрична проекція перетворює 2D сітку в ромбовидне відображення:

**Формули конвертації (Grid → Screen):**
```
screenX = (col - row) * TILE_WIDTH_HALF
screenY = (col + row) * TILE_HEIGHT_HALF
```

**Формули конвертації (Screen → Grid):**
```
col = (screenX / TILE_WIDTH_HALF + screenY / TILE_HEIGHT_HALF) / 2
row = (screenY / TILE_HEIGHT_HALF - screenX / TILE_WIDTH_HALF) / 2
```

Де:
- `TILE_WIDTH_HALF = 64` (половина ширини ізометричного тайлу)
- `TILE_HEIGHT_HALF = 32` (половина висоти ізометричного тайлу)

## Компоненти та інтерфейси

### Система координат

```cpp
// Координати сітки (логічні)
struct GridCoords {
    int row;
    int col;
};

// Екранні координати (пікселі)
struct ScreenCoords {
    float x;
    float y;
};

// Конвертер координат
class CoordinateConverter {
public:
    static constexpr int TILE_WIDTH = 128;
    static constexpr int TILE_HEIGHT = 64;
    static constexpr int TILE_WIDTH_HALF = 64;
    static constexpr int TILE_HEIGHT_HALF = 32;
    
    static ScreenCoords gridToScreen(GridCoords grid);
    static GridCoords screenToGrid(ScreenCoords screen);
};
```

### Типи місцевості

```cpp
enum class TerrainType {
    GRASS = 0,    // Трава - базова прохідна місцевість
    WATER = 1,    // Вода - непрохідна
    SAND = 2,     // Пісок - прохідна
    ROAD = 3      // Дорога - прохідна зі збільшеною швидкістю
};

struct TerrainProperties {
    bool passable;
    float speed_modifier;
    Color debug_color;
    Rectangle tileset_rect;  // Позиція в тайлсеті
};

// Таблиця властивостей
const TerrainProperties TERRAIN_PROPS[] = {
    {true,  1.0f, GREEN,  {0, 0, 128, 64}},      // GRASS
    {false, 0.0f, BLUE,   {128, 0, 128, 64}},    // WATER
    {true,  1.0f, BEIGE,  {0, 64, 128, 64}},     // SAND
    {true,  1.5f, BROWN,  {128, 64, 128, 64}}    // ROAD
};
```

### Структура тайлової карти

```cpp
class TileMap {
private:
    int width;   // Ширина в тайлах
    int height;  // Висота в тайлах
    std::vector<std::vector<TerrainType>> tiles;
    unsigned int seed;
    
public:
    TileMap(int w, int h, unsigned int s = 0);
    
    // Доступ до тайлів
    TerrainType getTile(int row, int col) const;
    void setTile(int row, int col, TerrainType type);
    bool isValidCoord(int row, int col) const;
    
    // Властивості
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    unsigned int getSeed() const { return seed; }
    
    // Запити про місцевість
    bool isPassable(int row, int col) const;
    float getSpeedModifier(int row, int col) const;
};
```

### Генератор шуму

```cpp
class NoiseGenerator {
private:
    unsigned int seed;
    float scale;
    int octaves;
    float persistence;
    float lacunarity;
    
public:
    NoiseGenerator(unsigned int s, float sc = 0.1f);
    
    // Генерація Perlin Noise
    float noise2D(float x, float y);
    float fractalNoise2D(float x, float y);
    
    // Налаштування
    void setScale(float s) { scale = s; }
    void setOctaves(int o) { octaves = o; }
};
```

### Генератор карти

```cpp
struct GenerationParams {
    float water_level = 0.3f;      // Поріг для води
    float sand_level = 0.5f;       // Поріг для піску
    float grass_level = 0.9f;      // Поріг для трави
    float noise_scale = 0.1f;      // Масштаб шуму
    int octaves = 4;               // Кількість октав
};

class MapGenerator {
private:
    NoiseGenerator noise_gen;
    GenerationParams params;
    
public:
    MapGenerator(unsigned int seed);
    
    // Генерація карти
    TileMap generate(int width, int height);
    TileMap generate(int width, int height, const GenerationParams& p);
    
    // Пост-обробка
    void smoothTerrain(TileMap& map);
    void ensureConnectivity(TileMap& map);
    
    // Розміщення стартових позицій
    std::pair<GridCoords, GridCoords> findStartPositions(const TileMap& map);
};
```

### Ізометричний рендерер

```cpp
class IsometricRenderer {
private:
    Texture2D tileset;
    Camera2D camera;
    TileMap* current_map;
    
    // Viewport culling
    GridCoords visible_start;
    GridCoords visible_end;
    
public:
    IsometricRenderer();
    ~IsometricRenderer();
    
    // Завантаження ресурсів
    void loadTileset(const char* filepath);
    void unloadTileset();
    
    // Рендеринг
    void render(const TileMap& map);
    void renderTile(int row, int col, TerrainType type);
    void renderDebug(const TileMap& map);
    
    // Камера
    void setCamera(Camera2D cam) { camera = cam; }
    Camera2D getCamera() const { return camera; }
    void updateCamera(Vector2 target);
    
    // Viewport culling
    void calculateVisibleTiles(const TileMap& map);
    bool isTileVisible(int row, int col) const;
    
    // Взаємодія
    GridCoords getHoveredTile() const;
    void highlightTile(int row, int col);
};
```

### Painter's Algorithm для сортування

Ізометричні тайли повинні малюватися у правильному порядку для коректного перекриття:

```cpp
void IsometricRenderer::render(const TileMap& map) {
    calculateVisibleTiles(map);
    
    // Painter's algorithm: малюємо від заду до переду
    // Порядок: верхні ряди → нижні ряди
    for (int row = visible_start.row; row <= visible_end.row; row++) {
        // В межах ряду: зліва направо
        for (int col = visible_start.col; col <= visible_end.col; col++) {
            if (isTileVisible(row, col)) {
                TerrainType type = map.getTile(row, col);
                renderTile(row, col, type);
            }
        }
    }
}
```

### Серіалізація карти

```cpp
class MapSerializer {
public:
    // Збереження у JSON
    static bool saveToJSON(const TileMap& map, const char* filepath);
    static TileMap loadFromJSON(const char* filepath);
    
    // Збереження у бінарний формат
    static bool saveToBinary(const TileMap& map, const char* filepath);
    static TileMap loadFromBinary(const char* filepath);
};

// Формат JSON
{
    "width": 50,
    "height": 50,
    "seed": 12345,
    "tiles": [
        [0, 0, 1, 2, ...],  // Ряд 0
        [0, 1, 1, 2, ...],  // Ряд 1
        ...
    ]
}
```

## Моделі даних

### Розміри тайлів

Ізометричні тайли мають співвідношення 2:1:
- **Ширина тайлу**: 128 пікселів
- **Висота тайлу**: 64 пікселі
- **Половина ширини**: 64 пікселі (для формул)
- **Половина висоти**: 32 пікселі (для формул)

### Структура тайлсету

Тайлсет - це один PNG файл з усіма тайлами у сітці:

```
[Трава 128x64] [Вода 128x64]
[Пісок 128x64] [Дорога 128x64]
```

Розмір тайлсету: 256x128 пікселів (2x2 сітка)

### Таблиця типів місцевості

| Тип | Прохідність | Модифікатор швидкості | Колір (debug) | Позиція в тайлсеті |
|-----|-------------|----------------------|---------------|-------------------|
| Трава | Так | 1.0x | Зелений | (0, 0) |
| Вода | Ні | 0.0x | Синій | (128, 0) |
| Пісок | Так | 1.0x | Бежевий | (0, 64) |
| Дорога | Так | 1.5x | Коричневий | (128, 64) |

### Параметри генерації за замовчуванням

```cpp
GenerationParams DEFAULT_PARAMS = {
    .water_level = 0.3f,    // 30% карти - вода
    .sand_level = 0.5f,     // 20% карти - пісок
    .grass_level = 0.9f,    // 40% карти - трава
    .noise_scale = 0.1f,    // Середній масштаб деталей
    .octaves = 4            // 4 октави для деталізації
};
```

## Властивості коректності

*Властивість - це характеристика або поведінка, яка повинна залишатися істинною для всіх валідних виконань системи - по суті, формальне твердження про те, що система повинна робити. Властивості служать мостом між специфікаціями, зрозумілими людині, та гарантіями коректності, які можна перевірити машиною.*

### Властивість 1: Конвертація координат є оборотною (round-trip)
*Для будь-яких* валідних координат сітки (row, col), конвертація в екранні координати та назад повинна повернути оригінальні координати сітки
**Validates: Requirements 1.6, 1.7**

### Властивість 2: Детермінованість генерації
*Для будь-якого* seed-значення, генерація карти з однаковими параметрами повинна створювати ідентичну карту
**Validates: Requirements 3.1**

### Властивість 3: Мінімальна прохідність карти
*Для будь-якої* згенерованої карти, мінімум 60% тайлів повинні бути прохідними (не вода)
**Validates: Requirements 3.5**

### Властивість 4: Властивості місцевості відповідають типу
*Для будь-якого* тайлу з типом місцевості, його властивості (прохідність, модифікатор швидкості) повинні відповідати специфікації типу
**Validates: Requirements 2.5, 2.6**

### Властивість 5: Валідність координат
*Для будь-яких* координат (row, col), якщо вони виходять за межі карти, система повинна повернути false при перевірці isValidCoord()
**Validates: Requirements 1.3**

### Властивість 6: Відстань між стартовими позиціями
*Для будь-якої* згенерованої карти, відстань між двома стартовими позиціями повинна бути не менше 30% діагоналі карти
**Validates: Requirements 5.2**

### Властивість 7: Стартові позиції на прохідній місцевості
*Для будь-якої* стартової позиції, тайл повинен бути прохідним (не вода)
**Validates: Requirements 5.3**

### Властивість 8: Область навколо стартової позиції прохідна
*Для будь-якої* стартової позиції, всі тайли в радіусі 2 тайли (область 5x5) повинні бути прохідними
**Validates: Requirements 5.4**

### Властивість 9: Серіалізація є оборотною (round-trip)
*Для будь-якої* карти, збереження в файл та завантаження назад повинно створити ідентичну карту
**Validates: Requirements 7.3**

### Властивість 10: Viewport culling не пропускає видимі тайли
*Для будь-якої* позиції камери, всі тайли, що потрапляють у viewport, повинні бути включені в розрахунок видимих тайлів
**Validates: Requirements 6.3**

## Обробка помилок

### Стратегія обробки помилок

1. **Валідація координат**: Перевірка меж перед доступом до масиву
2. **Fallback значення**: Повернення безпечних значень за замовчуванням
3. **Логування**: Запис помилок для налагодження
4. **Graceful degradation**: Продовження роботи при частковій втраті функціональності

### Типи помилок та їх обробка

#### Помилки координат
```cpp
TerrainType TileMap::getTile(int row, int col) const {
    if (!isValidCoord(row, col)) {
        // Логування помилки
        TraceLog(LOG_WARNING, "Invalid tile coordinates: (%d, %d)", row, col);
        // Повернення безпечного значення
        return TerrainType::GRASS;
    }
    return tiles[row][col];
}
```

#### Помилки завантаження тайлсету
```cpp
void IsometricRenderer::loadTileset(const char* filepath) {
    tileset = LoadTexture(filepath);
    if (tileset.id == 0) {
        TraceLog(LOG_ERROR, "Failed to load tileset: %s", filepath);
        // Створення fallback текстури
        Image fallback = GenImageColor(256, 128, GRAY);
        tileset = LoadTextureFromImage(fallback);
        UnloadImage(fallback);
    }
}
```

#### Помилки серіалізації
```cpp
TileMap MapSerializer::loadFromJSON(const char* filepath) {
    if (!FileExists(filepath)) {
        TraceLog(LOG_ERROR, "Map file not found: %s", filepath);
        // Повернення порожньої карти
        return TileMap(10, 10);
    }
    
    // Спроба завантаження
    try {
        // ... парсинг JSON ...
    } catch (const std::exception& e) {
        TraceLog(LOG_ERROR, "Failed to parse map file: %s", e.what());
        return TileMap(10, 10);
    }
}
```

#### Помилки генерації
```cpp
TileMap MapGenerator::generate(int width, int height) {
    if (width <= 0 || height <= 0) {
        TraceLog(LOG_WARNING, "Invalid map size: %dx%d, using 50x50", width, height);
        width = 50;
        height = 50;
    }
    
    if (width > 200 || height > 200) {
        TraceLog(LOG_WARNING, "Map too large: %dx%d, clamping to 200x200", width, height);
        width = std::min(width, 200);
        height = std::min(height, 200);
    }
    
    // ... генерація ...
}
```

## Стратегія тестування

### Подвійний підхід до тестування
- **Юніт-тести**: Перевіряють конкретні приклади, граничні випадки та умови помилок
- **Property-based тести**: Перевіряють універсальні властивості на багатьох згенерованих входах
- Обидва типи тестів є взаємодоповнюючими та необхідними для повного покриття

### Конфігурація Property-Based Testing
- **Бібліотека**: Використання Catch2 з генераторами для C++
- **Мінімум ітерацій**: 100 ітерацій на кожен property-тест
- **Теги тестів**: Формат **Feature: tile-map-generator, Property {номер}: {текст властивості}**

### Баланс юніт-тестування
- Юніт-тести корисні для конкретних прикладів та граничних випадків
- Уникати надмірної кількості юніт-тестів - property-based тести покривають багато входів
- Юніт-тести повинні фокусуватися на:
  - Конкретних прикладах, що демонструють правильну поведінку
  - Точках інтеграції між компонентами
  - Граничних випадках та умовах помилок
- Property-тести повинні фокусуватися на:
  - Універсальних властивостях для всіх входів
  - Всебічному покритті входів через рандомізацію

### Приклади тестів

#### Property-Based Test
```cpp
TEST_CASE("Feature: tile-map-generator, Property 1: Конвертація координат є оборотною") {
    SECTION("Grid → Screen → Grid повертає оригінальні координати") {
        auto row = GENERATE(take(100, random(0, 99)));
        auto col = GENERATE(take(100, random(0, 99)));
        
        GridCoords original = {row, col};
        ScreenCoords screen = CoordinateConverter::gridToScreen(original);
        GridCoords result = CoordinateConverter::screenToGrid(screen);
        
        REQUIRE(result.row == original.row);
        REQUIRE(result.col == original.col);
    }
}
```

#### Unit Test
```cpp
TEST_CASE("Конвертація координат - конкретні приклади") {
    SECTION("Тайл (2, 1) конвертується в екранні координати (64, 96)") {
        GridCoords grid = {2, 1};
        ScreenCoords screen = CoordinateConverter::gridToScreen(grid);
        
        REQUIRE(screen.x == 64.0f);
        REQUIRE(screen.y == 96.0f);
    }
    
    SECTION("Тайл (0, 0) конвертується в екранні координати (0, 0)") {
        GridCoords grid = {0, 0};
        ScreenCoords screen = CoordinateConverter::gridToScreen(grid);
        
        REQUIRE(screen.x == 0.0f);
        REQUIRE(screen.y == 0.0f);
    }
}
```

#### Integration Test
```cpp
TEST_CASE("Генерація та рендеринг карти") {
    SECTION("Згенерована карта може бути відрендерена без помилок") {
        MapGenerator generator(12345);
        TileMap map = generator.generate(50, 50);
        
        IsometricRenderer renderer;
        renderer.loadTileset("assets/tileset.png");
        
        // Не повинно викликати помилок
        REQUIRE_NOTHROW(renderer.render(map));
        
        renderer.unloadTileset();
    }
}
```

### Тестування продуктивності

```cpp
TEST_CASE("Продуктивність генерації карти") {
    SECTION("Карта 50x50 генерується за менше 100мс") {
        MapGenerator generator(12345);
        
        auto start = std::chrono::high_resolution_clock::now();
        TileMap map = generator.generate(50, 50);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        REQUIRE(duration.count() < 100);
    }
}
```

## Оптимізація продуктивності

### Viewport Culling

Рендеримо тільки видимі тайли:

```cpp
void IsometricRenderer::calculateVisibleTiles(const TileMap& map) {
    // Отримуємо межі екрану в світових координатах
    Vector2 top_left = GetScreenToWorld2D({0, 0}, camera);
    Vector2 bottom_right = GetScreenToWorld2D(
        {(float)GetScreenWidth(), (float)GetScreenHeight()}, 
        camera
    );
    
    // Конвертуємо в координати сітки з запасом
    GridCoords tl = CoordinateConverter::screenToGrid({top_left.x, top_left.y});
    GridCoords br = CoordinateConverter::screenToGrid({bottom_right.x, bottom_right.y});
    
    // Додаємо запас для перекриття тайлів
    visible_start.row = std::max(0, tl.row - 2);
    visible_start.col = std::max(0, tl.col - 2);
    visible_end.row = std::min(map.getHeight() - 1, br.row + 2);
    visible_end.col = std::min(map.getWidth() - 1, br.col + 2);
}
```

### Кешування текстур

Завантажуємо тайлсет один раз:

```cpp
// ❌ Погано - завантаження кожен кадр
void render() {
    Texture2D tileset = LoadTexture("tileset.png");
    // ... рендеринг ...
    UnloadTexture(tileset);
}

// ✅ Добре - завантаження один раз
void init() {
    tileset = LoadTexture("tileset.png");
}

void render() {
    // ... використання tileset ...
}

void cleanup() {
    UnloadTexture(tileset);
}
```

### Оптимізація пам'яті

Використовуємо компактне зберігання:

```cpp
// Замість std::vector<std::vector<TerrainType>>
// Використовуємо одновимірний масив
class TileMap {
private:
    std::vector<uint8_t> tiles;  // TerrainType як uint8_t
    int width, height;
    
public:
    TerrainType getTile(int row, int col) const {
        return static_cast<TerrainType>(tiles[row * width + col]);
    }
};
```

## Інтеграція з існуючою грою

### Додавання до main.cpp

```cpp
#include "tilemap_generator.h"

int main() {
    InitWindow(1024, 768, "Punic Wars");
    
    // Генерація карти
    MapGenerator generator(time(nullptr));
    TileMap map = generator.generate(50, 50);
    
    // Ініціалізація рендерера
    IsometricRenderer renderer;
    renderer.loadTileset("assets/isometric_tileset.png");
    
    // Налаштування камери
    Camera2D camera = {0};
    camera.target = {0, 0};
    camera.offset = {512, 384};  // Центр екрану
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
    renderer.setCamera(camera);
    
    while (!WindowShouldClose()) {
        // Оновлення камери (прокрутка)
        if (IsKeyDown(KEY_RIGHT)) camera.target.x += 5;
        if (IsKeyDown(KEY_LEFT)) camera.target.x -= 5;
        if (IsKeyDown(KEY_DOWN)) camera.target.y += 5;
        if (IsKeyDown(KEY_UP)) camera.target.y -= 5;
        renderer.setCamera(camera);
        
        // Рендеринг
        BeginDrawing();
        ClearBackground(BLACK);
        
        BeginMode2D(camera);
        renderer.render(map);
        EndMode2D();
        
        // UI
        DrawFPS(10, 10);
        
        EndDrawing();
    }
    
    renderer.unloadTileset();
    CloseWindow();
    return 0;
}
```

### Інтеграція з системою юнітів

Юніти повинні використовувати координати сітки:

```cpp
struct Unit {
    GridCoords grid_pos;  // Логічна позиція
    // ... інші поля ...
    
    ScreenCoords getScreenPos() const {
        return CoordinateConverter::gridToScreen(grid_pos);
    }
    
    void moveTo(GridCoords target) {
        if (map->isPassable(target.row, target.col)) {
            grid_pos = target;
        }
    }
};
```
