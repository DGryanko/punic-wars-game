# РЕКОМЕНДАЦІЇ З РЕФАКТОРИНГУ

## Виконано ✅

### 1. Створено централізовані константи
**Файл**: `cpp/src/game_constants.h`

Всі magic numbers тепер в одному місці:
- Розміри карти (80x80)
- Радіуси та відстані (40, 20, 200 пікселів)
- Початкові ресурси (200/100 для Риму, 150/200 для Карфагену)
- UI розміри (1434x1075)
- Debug прапорці

**Переваги**:
- Легко змінювати баланс гри
- Немає дублювання значень
- Самодокументований код

### 2. Створено debug logger
**Файл**: `cpp/src/debug_logger.h`

Централізоване логування з категоріями:
- `LOG_PATHFINDING` - логи pathfinding (вимкнено за замовчуванням)
- `LOG_CLICK` - логи кліків (вимкнено за замовчуванням)
- `LOG_SPAWN` - логи spawn (увімкнено)
- `LOG_BUILDING` - логи будівель (увімкнено)
- `LOG_ERROR` - помилки (завжди увімкнено)
- `LOG_WARNING` - попередження (завжди увімкнено)

**Переваги**:
- Можна вимкнути категорії логів без зміни коду
- Менше засмічення консолі
- Легше дебажити конкретні системи

### 3. Оновлено building.h
Замінено `printf` на `LOG_CLICK` для методу `isClicked()`.

## Рекомендації для подальшого рефакторингу 📋

### Пріоритет 1: Замінити printf на logger

**Файли для оновлення**:
1. `cpp/src/main.cpp` - замінити всі `printf("[FIND]"` на `LOG_FIND`
2. `cpp/src/main.cpp` - замінити всі `printf("[CLICK]"` на `LOG_CLICK`
3. `cpp/src/faction_spawner.h` - замінити на `LOG_SPAWN`
4. `cpp/src/building_placer.h` - замінити на `LOG_BUILDING`
5. `cpp/src/building_texture_manager.cpp` - замінити на `LOG_TEXTURE`
6. `cpp/src/unit.h` - замінити pathfinding логи на `LOG_PATHFINDING`

**Приклад заміни**:
```cpp
// Було:
printf("[FIND] Looking for building at grid (%d, %d)\n", pos.row, pos.col);

// Стало:
LOG_FIND("[FIND] Looking for building at grid (%d, %d)\n", pos.row, pos.col);
```

### Пріоритет 2: Використати константи замість magic numbers

**Файли для оновлення**:
1. `cpp/src/main.cpp`:
   ```cpp
   // Було:
   int rome_food = 200;
   int rome_money = 100;
   
   // Стало:
   int rome_food = GameConstants::StartingResources::ROME_FOOD;
   int rome_money = GameConstants::StartingResources::ROME_MONEY;
   ```

2. `cpp/src/main.cpp` - InitResources():
   ```cpp
   // Було:
   for (int i = 0; i < 8; i++) { // 8 food sources
   
   // Стало:
   for (int i = 0; i < GameConstants::ResourcePoints::FOOD_SOURCES_COUNT; i++) {
   ```

3. `cpp/src/unit.h` - радіус збору:
   ```cpp
   // Було:
   if (dist < 40.0f) {
   
   // Стало:
   if (dist < GameConstants::RESOURCE_COLLECTION_RADIUS) {
   ```

### Пріоритет 3: Виправити TODO коментарі

**cpp/src/unit.h** (рядок 319):
```cpp
// TODO: Update pathfinding to work with GridCoords
// For now, pathfinding is disabled
```
→ Відновити pathfinding з grid координатами

**cpp/src/pathfinding.h** (рядок 458):
```cpp
// TODO: перевіряти чи шлях перетинається з областю
```
→ Додати перевірку перетину шляху з областю

### Пріоритет 4: Видалити дублювання коду

**Патерн**: Багато схожих функцій для різних фракцій
```cpp
// Замість:
int rome_food, rome_money, rome_food_reserved, rome_money_reserved;
int carth_food, carth_money, carth_food_reserved, carth_money_reserved;

// Краще:
struct FactionResources {
    int food;
    int money;
    int food_reserved;
    int money_reserved;
};
FactionResources resources[2];  // ROME=0, CARTHAGE=1
```

### Пріоритет 5: Розділити main.cpp

**Проблема**: main.cpp має 1600+ рядків

**Рішення**: Розділити на модулі:
- `game_state.h/cpp` - стани гри, ресурси фракцій
- `audio_manager.h/cpp` - музична система
- `input_handler.h/cpp` - обробка кліків та керування
- `game_loop.h/cpp` - основний цикл гри

## Переваги рефакторингу

1. **Читабельність** - код легше розуміти
2. **Підтримка** - легше знаходити та виправляти баги
3. **Розширюваність** - легше додавати нові функції
4. **Продуктивність** - можна вимкнути непотрібні логи
5. **Тестування** - легше писати тести для модулів

## Як використовувати нові файли

### game_constants.h
```cpp
#include "game_constants.h"

// Використання:
if (distance < GameConstants::ENEMY_DETECTION_RADIUS) {
    // Ворог виявлений
}

int startFood = GameConstants::StartingResources::ROME_FOOD;
```

### debug_logger.h
```cpp
#include "debug_logger.h"

// Використання:
LOG_CLICK("[CLICK] Mouse at (%d, %d)\n", x, y);
LOG_ERROR("Failed to load texture: %s\n", filename);
LOG_WARNING("Resource depleted at (%d, %d)\n", row, col);

// Вимкнути категорію в game_constants.h:
constexpr bool ENABLE_CLICK_LOGS = false;
```

## Наступні кроки

1. Додати `#include "game_constants.h"` та `#include "debug_logger.h"` в main.cpp
2. Замінити всі printf на відповідні LOG_* макроси
3. Замінити всі magic numbers на константи
4. Протестувати що все працює
5. Зробити коміт з описом рефакторингу

---
**Статус**: Створено інфраструктуру для рефакторингу
**Наступний крок**: Застосувати зміни до main.cpp та інших файлів
