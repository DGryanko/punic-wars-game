# Design Document: Building Textures System

## Overview

Система текстур для будівель інтегрує PNG спрайти в існуючу систему будівель гри Punic Wars: Castra. Дизайн базується на існуючій архітектурі IsometricRenderer та розширює структуру Building для підтримки текстур.

## Architecture

Система складається з трьох основних компонентів:

1. **BuildingTextureManager** - управління завантаженням та зберіганням текстур
2. **Building (розширений)** - додає підтримку текстур до існуючої структури
3. **BuildingRenderer** - відповідає за малювання будівель з текстурами

```
┌─────────────────────┐
│   main.cpp          │
│  (Game Loop)        │
└──────────┬──────────┘
           │
           ├──> BuildingTextureManager (завантаження)
           │
           ├──> Building[] (дані)
           │
           └──> BuildingRenderer (малювання)
```

## Components and Interfaces

### 1. BuildingTextureManager

Singleton клас для управління текстурами будівель.

```cpp
class BuildingTextureManager {
private:
    static BuildingTextureManager* instance;
    std::map<BuildingType, Texture2D> textures;
    bool initialized;
    
    BuildingTextureManager();
    ~BuildingTextureManager();
    
public:
    static BuildingTextureManager& getInstance();
    
    // Завантаження всіх текстур
    void loadAllTextures();
    
    // Отримання текстури за типом
    Texture2D getTexture(BuildingType type) const;
    
    // Перевірка чи текстура завантажена
    bool hasTexture(BuildingType type) const;
    
    // Звільнення ресурсів
    void unloadAll();
    
private:
    // Завантаження окремої текстури
    bool loadTexture(BuildingType type, const char* filepath);
    
    // Створення fallback текстури
    Texture2D createFallbackTexture(BuildingType type);
};
```

**Шляхи до текстур:**
- `HQ_ROME` → `cpp/assets/sprites/Praetorium.png`
- `HQ_CARTHAGE` → `cpp/assets/sprites/MainTent.png` (поки fallback)
- `BARRACKS_ROME` → `cpp/assets/sprites/Contubernium.png` (поки fallback)
- `BARRACKS_CARTHAGE` → `cpp/assets/sprites/MercenaryCamp.png` (поки fallback)
- `QUESTORIUM_ROME` → `cpp/assets/sprites/Questorium.png` (поки fallback)

### 2. Building (розширення)

Додаємо поля та методи до існуючої структури Building:

```cpp
struct Building {
    // ... існуючі поля ...
    
    // Нові поля для текстур
    bool use_texture;           // Чи використовувати текстуру
    Vector2 texture_offset;     // Зміщення текстури відносно позиції
    float texture_scale;        // Масштаб текстури
    
    // Оновлений метод малювання
    void draw() const;
    
    // Новий метод для малювання з текстурою
    void drawWithTexture(const Texture2D& texture) const;
    
    // Метод для отримання розміру текстури
    Rectangle getTextureRect() const;
};
```

### 3. BuildingRenderer

Статичний клас для централізованого малювання будівель:

```cpp
class BuildingRenderer {
public:
    // Малювання однієї будівлі
    static void drawBuilding(const Building& building);
    
    // Малювання всіх будівель
    static void drawAllBuildings(const std::vector<Building>& buildings);
    
private:
    // Малювання індикатора виділення
    static void drawSelectionIndicator(const Building& building);
    
    // Малювання прогрес-бару виробництва
    static void drawProductionProgress(const Building& building);
    
    // Малювання назви будівлі
    static void drawBuildingName(const Building& building);
};
```

## Data Models

### BuildingType to Texture Mapping

```cpp
struct TextureInfo {
    const char* filepath;
    Vector2 offset;      // Зміщення для правильного позиціонування
    float scale;         // Масштаб (1.0 = оригінальний розмір)
};

const std::map<BuildingType, TextureInfo> BUILDING_TEXTURES = {
    {HQ_ROME, {"cpp/assets/sprites/Praetorium.png", {-20, -40}, 1.0f}},
    {HQ_CARTHAGE, {"cpp/assets/sprites/MainTent.png", {-20, -40}, 1.0f}},
    {BARRACKS_ROME, {"cpp/assets/sprites/Contubernium.png", {-15, -30}, 0.9f}},
    {BARRACKS_CARTHAGE, {"cpp/assets/sprites/MercenaryCamp.png", {-15, -30}, 0.9f}},
    {QUESTORIUM_ROME, {"cpp/assets/sprites/Questorium.png", {-15, -30}, 0.9f}}
};
```

### Texture Loading State

```cpp
enum TextureLoadState {
    NOT_LOADED,
    LOADING,
    LOADED,
    FAILED
};
```


## Correctness Properties

A property is a characteristic or behavior that should hold true across all valid executions of a system—essentially, a formal statement about what the system should do. Properties serve as the bridge between human-readable specifications and machine-verifiable correctness guarantees.

### Property 1: Texture Caching

*For any* BuildingType, requesting the same texture multiple times SHALL return the same Texture2D object (same texture ID), ensuring textures are loaded only once.

**Validates: Requirements 1.4, 4.1**

### Property 2: Automatic Texture Assignment

*For any* newly created Building of a supported type, the System SHALL automatically assign a valid texture path corresponding to that building type.

**Validates: Requirements 3.2**

### Property 3: Faction-Specific Textures (Rome)

*For any* Building with faction ROME, the assigned texture SHALL be from the Roman texture set (texture path contains "Praetorium", "Contubernium", or "Questorium").

**Validates: Requirements 3.4**

### Property 4: Faction-Specific Textures (Carthage)

*For any* Building with faction CARTHAGE, the assigned texture SHALL be from the Carthaginian texture set (texture path contains "MainTent" or "MercenaryCamp").

**Validates: Requirements 3.5**

### Property 5: Viewport Culling

*For any* Building positioned outside the visible screen area, the rendering method SHALL NOT be invoked, ensuring only visible buildings are drawn.

**Validates: Requirements 4.3**

## Error Handling

### Texture Loading Failures

1. **Missing File**: If texture file not found, log error and create fallback colored texture
2. **Invalid Format**: If file format unsupported, log error and use fallback
3. **Memory Issues**: If texture too large, log error and use smaller fallback

```cpp
Texture2D BuildingTextureManager::loadTexture(BuildingType type, const char* filepath) {
    if (!FileExists(filepath)) {
        TraceLog(LOG_ERROR, "Texture not found: %s", filepath);
        return createFallbackTexture(type);
    }
    
    Texture2D texture = LoadTexture(filepath);
    
    if (texture.id == 0) {
        TraceLog(LOG_ERROR, "Failed to load texture: %s", filepath);
        return createFallbackTexture(type);
    }
    
    return texture;
}
```

### Fallback Strategy

Якщо текстура не завантажується, система використовує кольорові прямокутники (існуюча поведінка):
- Rome buildings: RED
- Carthage buildings: BLUE

### Runtime Errors

- **Null Texture**: Перевірка `texture.id != 0` перед малюванням
- **Invalid Building Type**: Використання default case в switch statements
- **Memory Leaks**: Proper cleanup в destructor та unloadAll()

## Testing Strategy

### Unit Tests

Специфічні приклади та edge cases:

1. **Texture Loading**
   - Test loading existing PNG file
   - Test loading non-existent file (fallback)
   - Test loading all 5 building types

2. **Building Initialization**
   - Test each building type has correct texture path
   - Test Roman buildings get Roman textures
   - Test Carthaginian buildings get Carthaginian textures

3. **Backward Compatibility**
   - Test all existing Building methods still work
   - Test fallback to colored rectangles when texture missing
   - Test selection, production, and interaction features

### Property-Based Tests

Universal properties across all inputs (minimum 100 iterations each):

1. **Property Test: Texture Caching**
   - Generate random sequences of texture requests
   - Verify same BuildingType always returns same texture ID
   - **Feature: building-textures, Property 1: Texture Caching**

2. **Property Test: Automatic Texture Assignment**
   - Generate random Buildings of all supported types
   - Verify each has valid, non-empty texture path
   - **Feature: building-textures, Property 2: Automatic Texture Assignment**

3. **Property Test: Roman Faction Textures**
   - Generate random Roman buildings of various types
   - Verify all have Roman texture paths
   - **Feature: building-textures, Property 3: Faction-Specific Textures (Rome)**

4. **Property Test: Carthaginian Faction Textures**
   - Generate random Carthaginian buildings of various types
   - Verify all have Carthaginian texture paths
   - **Feature: building-textures, Property 4: Faction-Specific Textures (Carthage)**

5. **Property Test: Viewport Culling**
   - Generate random building positions (some visible, some not)
   - Verify only visible buildings trigger draw calls
   - **Feature: building-textures, Property 5: Viewport Culling**

### Integration Tests

- Test full game loop with textured buildings
- Test interaction between BuildingTextureManager and Building
- Test rendering pipeline with multiple buildings
- Test performance with 20+ buildings on screen

### Testing Framework

Use existing C++ testing framework or add:
- **Catch2** or **Google Test** for unit tests
- **RapidCheck** or **QuickCheck++** for property-based tests
- Minimum 100 iterations per property test
