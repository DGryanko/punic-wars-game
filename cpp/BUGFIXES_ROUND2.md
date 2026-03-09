# Виправлення помилок - Раунд 2

## Виправлені проблеми

### 1. Прибрано "Adjust game settings" з налаштувань ✅
**Проблема:** В меню налаштувань відображався зайвий текст "Adjust game settings".

**Рішення:** Видалено рядок з `DrawSettings()` в `main.cpp`:
```cpp
// Видалено:
DrawText("Adjust game settings", 430, 510, 16, GRAY);
```

---

### 2. Виправлено блокування шляху будівлями ✅
**Проблема:** Метод `occupiesGridCell()` використовував неправильне зміщення (-2 row), через що pathfinding працював нестабільно.

**Рішення:** Спрощено логіку в `building.h`:

**Було:**
```cpp
bool occupiesGridCell(GridCoords cell) const {
    int offsetRow = -2;  // Неправильне зміщення!
    int offsetCol = 0;
    
    bool occupies = cell.row >= position.row + offsetRow && 
           cell.row < position.row + offsetRow + footprint.row &&
           cell.col >= position.col + offsetCol && 
           cell.col < position.col + offsetCol + footprint.col;
    return occupies;
}
```

**Стало:**
```cpp
bool occupiesGridCell(GridCoords cell) const {
    // Просто перевіряємо чи клітинка в межах footprint будівлі
    bool occupies = cell.row >= position.row && 
           cell.row < position.row + footprint.row &&
           cell.col >= position.col && 
           cell.col < position.col + footprint.col;
    return occupies;
}
```

**Результат:** Тепер pathfinding правильно визначає які клітинки зайняті будівлями.

---

### 3. Додано детальне логування для дебагу здачі ресурсів 🔍
**Проблема:** Раб не йшов здавати ресурси в квесторій, причина була незрозуміла.

**Рішення:** Додано детальне логування в `ProcessResourceHarvesting()`:

```cpp
printf("[DROPOFF] Slave full, searching for dropoff building. Total buildings: %d\n", (int)buildings.size());

// При перевірці кожної будівлі:
printf("[DROPOFF] Checking building: %s, faction=%d, type=%d, playerFaction=%d\n", 
       building.name.c_str(), building.faction, building.type, playerFaction);

// Коли знайдено квесторіум:
printf("[DROPOFF] Found Questorium at grid(%d,%d), distance=%.1f\n", 
       buildingPos.row, buildingPos.col, dist);

// При виборі будівлі:
printf("[DROPOFF] Selected %s at grid(%d,%d), distance=%.1f\n", 
       dropoffBuilding->name.c_str(), buildingPos.row, buildingPos.col, distToBuilding);

// При русі:
printf("[MOVE] Slave at grid(%d,%d), target grid(%d,%d), building grid(%d,%d), movingCorrect=%d\n",
       unitPos.row, unitPos.col, targetPos.row, targetPos.col,
       buildingPos.row, buildingPos.col, movingToCorrectTarget);
```

**Що це дає:**
- Можна побачити скільки будівель в грі
- Можна побачити які будівлі перевіряються
- Можна побачити чи знайдено квесторіум
- Можна побачити куди рухається раб
- Легко знайти проблему якщо щось не працює

---

### 4. Панель будівництва закривається при зміні фокусу ✅
**Проблема:** Коли вибирали іншу будівлю або юніта, панель будівництва раба залишалася відкритою.

**Рішення:** Додано очищення панелі в `clearAllSelections()`:

```cpp
void clearAllSelections() {
    selectedBuildingIndex = -1;
    selectedUnitIndex = -1;
    for (auto& building : buildings) {
        building.selected = false;
    }
    for (auto& unit : units) {
        unit.selected = false;
    }
    // Оновлюємо панель замовлення
    if (unitOrderPanel) {
        unitOrderPanel->setSelectedBuilding(-1);
    }
    // Оновлюємо панель будівництва (ДОДАНО)
    if (slaveBuildPanel) {
        slaveBuildPanel->setSelectedUnit(-1);
    }
}
```

**Результат:** Тепер панель будівництва автоматично закривається коли:
- Вибираєте іншого юніта
- Вибираєте будівлю
- Клікаєте на порожнє місце

---

## Тестування

### Як перевірити виправлення:

**Проблема 1 - Текст в налаштуваннях:**
1. Відкрийте меню налаштувань
2. Перевірте що немає тексту "Adjust game settings"

**Проблема 2 - Блокування шляху:**
1. Побудуйте кілька будівель
2. Виберіть юніта
3. Накажіть йому рухатися через будівлі
4. Перевірте що юніт стабільно обходить будівлі

**Проблема 3 - Здача ресурсів (з логами):**
1. Побудуйте квесторіум
2. Виберіть раба та відправте його збирати ресурси
3. Дивіться в консоль (game_log.txt) коли раб наповниться
4. Ви побачите логи типу:
   ```
   [DROPOFF] Slave full, searching for dropoff building. Total buildings: 3
   [DROPOFF] Checking building: Praetorium, faction=0, type=0, playerFaction=0
   [DROPOFF] Checking building: Questorium, faction=0, type=4, playerFaction=0
   [DROPOFF] Found Questorium at grid(42,38), distance=3.6
   [DROPOFF] Selected Questorium at grid(42,38), distance=3.6
   [MOVE] Slave at grid(40,35), target grid(42,38), building grid(42,38), movingCorrect=1
   ```

**Проблема 4 - Закриття панелі:**
1. Виберіть раба (панель відкриється)
2. Виберіть іншого юніта або будівлю
3. Перевірте що панель будівництва закрилася

---

## Типи будівель (для довідки)

```cpp
enum BuildingType {
    HQ_ROME = 0,           // Преторій
    HQ_CARTHAGE = 1,       // Головне шатро
    BARRACKS_ROME = 2,     // Контуберній (казарма)
    BARRACKS_CARTHAGE = 3, // Карфагенська казарма
    QUESTORIUM_ROME = 4,   // Квесторій (склад ресурсів)
    LIBTENT_1 = 5,         // Палатка лівійців рівень 1
    LIBTENT_2 = 6,         // Палатка лівійців рівень 2
    LIBTENT_3 = 7,         // Палатка лівійців рівень 3
    TENTORIUM = 8          // Тенторіум торговця
};
```

---

## Наступні кроки

Якщо раб все ще не йде до квесторіуму, перевірте логи:
1. Чи знаходиться квесторіум? (має бути `[DROPOFF] Found Questorium`)
2. Чи правильна фракція? (faction має дорівнювати playerFaction)
3. Чи правильний тип? (type має бути 4 для QUESTORIUM_ROME)
4. Чи рухається раб? (movingCorrect має бути 1)

Якщо в логах `[ERROR] No dropoff building found for slave!`, значить:
- Квесторіум не побудований
- Квесторіум іншої фракції
- Проблема з типом будівлі
