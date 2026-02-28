# Технічне завдання: "Пунічні війни: Табори"
## 2D RTS міні-гра на Raylib

---

## 🎯 Концепт гри

**Жанр:** 2D Real-Time Strategy (міні-RTS)  
**Сетінг:** Пунічні війни (Рим vs Карфаген)  
**Фокус:** Управління військовими таборами з унікальними особливостями фракцій  
**Розмір карти:** 1024x768 пікселів  
**Максимум юнітів:** 50-100 для оптимальної продуктивності  

---

## 🏛️ Фракції та їх особливості

### Рим
**Філософія:** Громадянський обов'язок та дисципліна

**Будівлі:**
- **Преторій** (HQ) — Головний намет, сховище 200/200 ресурсів
- **Квесторій** — Сховище ресурсів (макс 1000/1000), вартість 50 їжі + 50 грошей
- **Контуберній** — Прямокутний шатер для 8 легіонерів, вартість 80 їжі + 60 грошей
- **Тенторіум** — Ринок торговця (обмін 2 їжа ↔ 1 гроші), вартість 30 їжі + 30 грошей

**Юніти:**
- **Легіонер:** HP 100, атака 15, захист 12, швидкість 1.0 (меч + щит)
- **Веліт:** HP 70, атака 10, захист 5, швидкість 1.3, дальність 5 (дротики)
- **Раб:** HP 50, атака 3, захист 1, збір ресурсів

**Особливість моралі:**
- ❌ **Без їжі:** Втеча (0% боєздатності)
- ⚠️ **Без грошей:** Громадянський обов'язок (80% боєздатності)
- ✅ **Є все:** Повна боєздатність (100%)

### Карфаген
**Філософія:** Найманці та східна розкіш

**Будівлі:**
- **Головне шатро** (HQ) — Помпезне шатро, сховище 1500 грошей, +2 гроші/хв
- **Шатер лівійців** — Округлий/конічний, покращення 6→8→10 місць
  - Рівень 1: 6 фінікійців (60 їжі + 40 грошей)
  - Рівень 2: 8 місць (+40 їжі + 30 грошей)
  - Рівень 3: 10 місць (+50 їжі + 40 грошей)
- **Загін коней + халабуди** — 4 нумідійські вершники (100 їжі + 80 грошей)
- **Сховище їжі** — Окреме сховище 800 їжі (40 їжі + 20 грошей)

**Юніти:**
- **Фінікієць:** HP 90, атака 14, захист 10, швидкість 1.0 (спис + круглий щит)
- **Пельтаст:** HP 60, атака 8 дальня/4 ближня, захист 3, швидкість 1.4, дальність 4
- **Нумідієць:** HP 80, атака 12, захист 6, швидкість 2.5 (вершник)
- **Раб:** HP 45, атака 2, захист 1, збір ресурсів

**Особливість моралі:**
- ❌ **Без грошей:** Повстання найманців (80% боєздатності)
- ⚠️ **Без їжі:** Голод (80% боєздатності)
- ✅ **Є все:** Повна боєздатність (100%)

---

## 💰 Економічна система

### Ресурси
- **Їжа:** Полювання (3/сек), рибалка (2/сек), збирання (1/сек), ферма (4/сек)
- **Гроші:** Ринок, напади на каравани, пасивний дохід

### Раби (обидві фракції)
- **Ефективність:** 70% від вільних робітників
- **Завдання:** Полювання, рибалка, збирання, ферма, носіння води (мораль)
- **Характеристики:** Слабкі в бою, дешеві у виробництві

### Ринок
- **Обмін:** 2 їжа ↔ 1 гроші
- **Доступ:** Тенторіум (Рим), можливо інтеграція в Головне шатро (Карфаген)

---

## 🏆 Умови перемоги

1. **Військова:** Знищити всі будівлі ворожого табору
2. **Економічна:** Накопичити 5000 загальних ресурсів (їжа + гроші)

---

## 🛠️ Технічний стек: Raylib

### Чому Raylib?
- ✅ **Простота:** Один хедер `raylib.h`, мінімум налаштувань
- ✅ **Компактність:** Фінальна гра ~30-80 МБ
- ✅ **Вбудовані функції:** Спрайти, звук, UI, колізії
- ✅ **Швидкий старт:** Компіляція одним рядком
- ✅ **RTS-приклади:** Готові зразки на GitHub

### Встановлення (Windows)

1. **Скачати Raylib:**
   ```
   raylib.com → Download → Windows Installer
   ```
   (Включає MinGW компілятор)

2. **Перевірити встановлення:**
   ```cpp
   #include "raylib.h"
   
   int main() {
       InitWindow(1024, 768, "Punic Wars Test");
       while (!WindowShouldClose()) {
           BeginDrawing();
           ClearBackground(DARKGREEN);
           DrawText("Raylib працює!", 400, 350, 20, WHITE);
           EndDrawing();
       }
       CloseWindow();
       return 0;
   }
   ```

3. **Компіляція:**
   ```bash
   gcc main.cpp -o game.exe -lraylib -lopengl32 -lgdi32 -lwinmm
   ```

---

## 📁 Структура проєкту

```
punic_wars/
├── src/
│   ├── main.cpp           # Головний файл гри
│   ├── faction.h          # Система фракцій та моралі
│   ├── building.h         # Будівлі та їх логіка
│   ├── unit.h             # Юніти та бойова система
│   ├── resource.h         # Економіка та раби
│   ├── ui.h               # Інтерфейс користувача
│   └── game_state.h       # Стан гри та перемога
├── assets/
│   ├── sprites/
│   │   ├── buildings/     # Спрайти будівель
│   │   ├── units/         # Спрайти юнітів
│   │   └── ui/            # Елементи інтерфейсу
│   ├── sounds/
│   │   ├── effects/       # Звукові ефекти
│   │   └── music/         # Фонова музика
│   └── maps/
│       └── terrain.png    # Текстура карти
├── docs/
│   ├── requirements.md    # Вимоги (Kiro spec)
│   ├── design.md          # Дизайн документ
│   └── tasks.md           # План реалізації
└── CMakeLists.txt         # Система збірки (опціонально)
```

---

## 🎨 Генерація спрайтів - JSON промпти

### Будівлі Риму

```json
{
  "praetorium": {
    "prompt": "Roman military command tent, rectangular, red and gold fabric, eagle standard, 64x64 pixels, top-down view, ancient Rome style",
    "size": "64x64",
    "style": "pixel art, isometric"
  },
  "quaestorium": {
    "prompt": "Roman storage building, wooden structure, multiple rooms, supply crates visible, 64x64 pixels, top-down view",
    "size": "64x64", 
    "style": "pixel art, isometric"
  },
  "contubernium": {
    "prompt": "Rectangular Roman military tent, brown leather, 8 sleeping areas visible, campfire in center, 64x64 pixels, top-down view",
    "size": "64x64",
    "style": "pixel art, isometric"
  },
  "tentorium": {
    "prompt": "Roman merchant tent, colorful fabrics, trade goods displayed, market stall appearance, 48x48 pixels, top-down view",
    "size": "48x48",
    "style": "pixel art, isometric"
  }
}
```

### Будівлі Карфагену

```json
{
  "carthage_hq": {
    "prompt": "Luxurious Carthaginian command tent, circular, purple and gold silk, eastern ornaments, 64x64 pixels, top-down view",
    "size": "64x64",
    "style": "pixel art, isometric"
  },
  "libyan_tent_lv1": {
    "prompt": "Round Carthaginian military tent, conical shape, 6 sleeping areas, tribal patterns, 48x48 pixels, top-down view",
    "size": "48x48",
    "style": "pixel art, isometric"
  },
  "libyan_tent_lv2": {
    "prompt": "Larger round Carthaginian tent, 8 sleeping areas, more decorations, tribal patterns, 56x56 pixels, top-down view",
    "size": "56x56",
    "style": "pixel art, isometric"
  },
  "libyan_tent_lv3": {
    "prompt": "Large round Carthaginian tent, 10 sleeping areas, rich decorations, tribal patterns, 64x64 pixels, top-down view",
    "size": "64x64",
    "style": "pixel art, isometric"
  },
  "horse_pen": {
    "prompt": "Open horse enclosure with woven huts, 4 horses visible, Numidian style shelters, 64x64 pixels, top-down view",
    "size": "64x64",
    "style": "pixel art, isometric"
  },
  "food_storage": {
    "prompt": "Carthaginian food storage tent, grain sacks, amphoras, desert style, 48x48 pixels, top-down view",
    "size": "48x48",
    "style": "pixel art, isometric"
  }
}
```

### Юніти Риму

```json
{
  "roman_legionary": {
    "prompt": "Roman legionary, red tunic, rectangular shield (scutum), gladius sword, helmet with plume, 32x32 pixels, top-down view",
    "size": "32x32",
    "style": "pixel art, 4 directions animation"
  },
  "roman_velites": {
    "prompt": "Roman light infantry, leather armor, round shield, javelins, wolf skin cap, 32x32 pixels, top-down view",
    "size": "32x32", 
    "style": "pixel art, 4 directions animation"
  },
  "roman_slave": {
    "prompt": "Roman slave worker, simple tunic, carrying tools, humble appearance, 24x24 pixels, top-down view",
    "size": "24x24",
    "style": "pixel art, 4 directions animation"
  }
}
```

### Юніти Карфагену

```json
{
  "phoenician_infantry": {
    "prompt": "Carthaginian Phoenician warrior, round shield, long spear, eastern armor, colorful tunic, 32x32 pixels, top-down view",
    "size": "32x32",
    "style": "pixel art, 4 directions animation"
  },
  "peltast": {
    "prompt": "Carthaginian peltast, light armor, small shield, javelins, agile appearance, 32x32 pixels, top-down view",
    "size": "32x32",
    "style": "pixel art, 4 directions animation"
  },
  "numidian_cavalry": {
    "prompt": "Numidian horseman, no saddle, tribal clothing, horse and rider, desert warrior, 40x40 pixels, top-down view",
    "size": "40x40",
    "style": "pixel art, 4 directions animation"
  },
  "carthage_slave": {
    "prompt": "Carthaginian slave worker, eastern clothing, carrying supplies, desert style, 24x24 pixels, top-down view",
    "size": "24x24",
    "style": "pixel art, 4 directions animation"
  }
}
```

### UI елементи

```json
{
  "resource_panel": {
    "prompt": "Ancient UI panel, stone texture, food and coin icons, Roman/Carthaginian style, 200x60 pixels",
    "size": "200x60",
    "style": "pixel art, UI design"
  },
  "building_buttons": {
    "prompt": "Set of building construction buttons, ancient style icons, stone/wood texture, 48x48 each",
    "size": "48x48",
    "style": "pixel art, icon set"
  },
  "morale_indicator": {
    "prompt": "Morale bar with ancient military symbols, eagle for Rome, elephant for Carthage, 100x20 pixels",
    "size": "100x20", 
    "style": "pixel art, UI element"
  }
}
```

---

## 🚀 Покроковий план розробки

### Фаза 1: Базова структура (1-2 дні)
1. Налаштування Raylib проєкту
2. Створення основного циклу гри
3. Базовий рендеринг карти 1024x768
4. Система ресурсів (їжа/гроші)

### Фаза 2: Будівлі та економіка (2-3 дні)
1. Система будівель для обох фракцій
2. Розміщення та будівництво
3. Сховища ресурсів
4. Базова система рабів

### Фаза 3: Юніти та бойова система (3-4 дні)
1. Створення та управління юнітами
2. Базовий ШІ для руху
3. Система бою (атака/захист/HP)
4. Система моралі фракцій

### Фаза 4: Геймплей та баланс (2-3 дні)
1. Умови перемоги
2. Балансування економіки
3. Тестування та оптимізація
4. Базовий ШІ противника

### Фаза 5: Поліровка (1-2 дні)
1. Звукові ефекти
2. Покращення UI
3. Фінальне тестування
4. Збірка релізу

---

## 📋 Технічні вимоги

- **Роздільна здатність:** 1024x768 пікселів
- **FPS:** 60 кадрів за секунду
- **Максимум юнітів:** 100 одиниць
- **Розмір гри:** До 150 МБ
- **Платформа:** Windows (можливо Linux/Mac)
- **Мова:** C++ з Raylib

---

## 🎯 Готовність до реалізації

Цей документ містить всю необхідну інформацію для початку розробки в Kiro.dev. Наступні кроки:

1. **Генерація спрайтів** за JSON промптами
2. **Створення дизайн-документу** в Kiro specs
3. **Розбивка на задачі** для поетапної реалізації
4. **Початок кодування** з базової структури Raylib

Готовий до запуску! 🚀