# Punic Wars: Castra - C++ Implementation

## Швидкий старт

1. **Встановіть Raylib** (див. SETUP.md)
2. **Скомпілюйте гру:**
   ```bash
   # Windows
   compile.bat
   
   # Або вручну
   g++ src/main.cpp -o punic_wars.exe -lraylib -lopengl32 -lgdi32 -lwinmm
   ```
3. **Запустіть гру:**
   ```bash
   # Windows
   run.bat
   
   # Або вручну
   ./punic_wars.exe
   ```

## Поточний стан

🎉 **SMOKE TEST MVP ГОТОВИЙ ДО ТЕСТУВАННЯ!**

✅ **Завершені завдання:**
- Налаштування проєкту та структури
- Базове вікно з меню (Старт/Вийти)
- Система будівель (3 типи)
- Система юнітів (2 типи)
- Вибір та пересування юнітів
- HUD з ресурсами

🔄 **Наступне завдання:** Встановити Raylib та протестувати гру

**Див. READY_TO_TEST.md для детальних інструкцій запуску**

## Структура проєкту

```
cpp/
├── src/
│   ├── main.cpp           # Головний файл гри
│   ├── building.h         # Система будівель (TODO)
│   ├── unit.h             # Юніти та їх поведінка (TODO)
│   └── ...                # Інші модулі
├── assets/
│   ├── sprites/           # Спрайти будівель та юнітів
│   └── sounds/            # Звукові ефекти
├── tests/                 # Тести (unit + property-based)
├── compile.bat            # Скрипт компіляції для Windows
├── run.bat                # Скрипт запуску для Windows
├── Makefile               # Для систем з make
└── SETUP.md               # Інструкції встановлення Raylib
```

## Компіляція

### Windows
```bash
compile.bat          # Автоматичний пошук компілятора
```

### Linux/Mac (з make)
```bash
make quick          # Швидка компіляція
make run           # Компіляція + запуск
```

### Ручна компіляція
```bash
g++ src/main.cpp -o punic_wars.exe -lraylib -lopengl32 -lgdi32 -lwinmm
```

## Залежності

- **Raylib** - 2D/3D ігровий рушій
- **MinGW/GCC** - C++ компілятор
- **Catch2** - Для тестування (буде додано пізніше)