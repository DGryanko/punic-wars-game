#!/usr/bin/env python3
"""
Punic Wars: Castra MCP Server
Спеціалізований MCP сервер для розробки гри Punic Wars: Castra
"""

import asyncio
import json
from pathlib import Path
from mcp.server import Server
from mcp.types import Tool, TextContent
import mcp.server.stdio

# Ініціалізація MCP сервера
app = Server("punic-wars-context")

# Базовий шлях до проєкту
PROJECT_ROOT = Path(__file__).parent

def read_file_safe(filepath: Path) -> str:
    """Безпечне читання файлу"""
    try:
        if filepath.exists():
            return filepath.read_text(encoding='utf-8')
        return f"Файл {filepath} не знайдено"
    except Exception as e:
        return f"Помилка читання {filepath}: {str(e)}"

def get_project_summary() -> str:
    """Повертає короткий опис проєкту"""
    return """
# PUNIC WARS: CASTRA - КОНТЕКСТ ПРОЄКТУ

## ПОТОЧНИЙ СТАН (Сесія 4 - ЗАВЕРШЕНА)
✅ Гра повністю працює та компілюється
✅ Розмір вікна: 1434x1075 (збільшено на 40%)
✅ Всі основні RTS механіки реалізовані та протестовані
✅ Динамічна музична система з 7 станами та ротацією треків
✅ Автоматичний збір ресурсів працює коректно
✅ Меню налаштувань з регулюванням гучності

## СТРУКТУРА ФАЙЛІВ
```
cpp/
├── src/
│   ├── main.cpp          # Головний файл (1600+ рядків)
│   ├── building.h        # Система будівель з чергою виробництва
│   ├── unit.h            # Система юнітів (рух, бій, збір ресурсів)
│   └── resource.h        # Ресурсні точки (їжа, золото)
├── assets/
│   ├── sounds/
│   │   ├── Punic wars_ Castra.mp3                      # Меню музика
│   │   ├── Punic wars_ Castra Battlemusic Rome.mp3     # Бойова музика Риму
│   │   ├── Punic wars_ Castra Battlemusic Carthage.mp3 # Бойова музика Карфагену
│   │   └── ... (інші треки)
│   └── Background.png    # Фон меню
├── compile.bat           # Компіляція: g++ з Raylib
├── run.bat              # Запуск гри
├── DEVELOPMENT_LOG.md   # Повний лог розробки
├── AUDIO_FEATURES.md    # Документація аудіо системи
├── SESSION_CONTEXT.md   # Контекст для наступної сесії
└── punic_wars.exe       # Виконуваний файл
```

## РЕАЛІЗОВАНІ ФУНКЦІЇ

### 1. МЕНЮ ТА ВИБІР ФРАКЦІЇ
- Головне меню з фоновим малюнком (START GAME / SETTINGS / EXIT)
- Фонова музика в меню
- Екран вибору фракції (ROME / CARTHAGE)
- Гравець керує своєю фракцією, AI - ворожою

### 2. БУДІВЛІ (5 типів)
- **HQ_ROME** (Praetorium) - головна будівля, виробляє рабів
- **HQ_CARTHAGE** (Main Tent) - головна будівля Карфагену
- **BARRACKS_ROME** (Contubernium) - казарма, виробляє легіонерів (макс 8)
- **BARRACKS_CARTHAGE** - казарма Карфагену, виробляє фінікійців
- **QUESTORIUM_ROME** - склад для здачі ресурсів

### 3. ЮНІТИ (3 типи з повними характеристиками)

**Легіонер (Legionary) - Рим**
- HP: 100, Урон: 25, Дальність: 30, Перезарядка: 1.5с
- Швидкість: 0.85f
- Вартість: 30 їжі + 50 грошей
- Час виробництва: 10с

**Фінікієць (Phoenician) - Карфаген**
- HP: 90, Урон: 30, Дальність: 25, Перезарядка: 1.2с
- Швидкість: 0.85f
- Вартість: 25 їжі + 60 грошей
- Час виробництва: 9с

**Раб (Slave) - обидві фракції**
- HP: 50, Урон: 5, Дальність: 15, Перезарядка: 2.0с
- Швидкість: 0.6f
- Вартість: 10 їжі + 20 грошей
- Час виробництва: 6с
- **Може збирати ресурси**

### 4. СИСТЕМА РЕСУРСІВ
- **FOOD_SOURCE** - точки їжі (зелені квадрати з "F")
- **GOLD_SOURCE** - точки золота (жовті квадрати з "G")
- Обмежена кількість ресурсів (400-500 їжі, 300-350 золота)
- **Автоматичний цикл збору** (Виправлено! Сесія 4):
  - ПКМ по ресурсу → раб призначається до цього ресурсу
  - Раб йде до ресурсу → збирає → йде до бази → здає → повертається до ресурсу
  - Працює в циклі поки ресурс не вичерпається
  - Радіус збору: 40 пікселів
  - Раби автоматично починають збирати якщо стоять біля ресурсу
- Швидкість збору: 2 одиниці за кадр

### 5. ВИРОБНИЦТВО ЮНІТІВ
- **Черга виробництва** - можна замовляти кілька юнітів
- **ЛКМ на іконці** - додати юніт до черги
- **ПКМ на іконці** - скасувати останній у черзі (повертає ресурси)
- **Число в куточку** - показує кількість у черзі
- **Прогрес-бар** - показує прогрес виробництва поточного юніта

### 6. БОЙОВА СИСТЕМА
- Юніти автоматично атакують ворогів у радіусі дії
- **ПКМ по ворогу** - наказ атакувати конкретну ціль
- Система урону та здоров'я
- Мертві юніти (HP ≤ 0) видаляються з гри
- Візуальні індикатори здоров'я (зелені смужки)

### 7. КОЛІЗІЇ
- **Юніти + будівлі** - юніти не проходять крізь будівлі
- **Юніти + юніти** - юніти не проходять один через одного
- Автоматична зупинка при колізії

### 8. КЕРУВАННЯ

**Виділення юнітів:**
- **ЛКМ** - вибрати один юніт
- **Перетягування ЛКМ** - виділити область (drag selection)
- **Подвійний клік** - виділити всі юніти цього типу на екрані

**Команди:**
- **ПКМ на землі** - наказ руху
- **ПКМ на ресурсі** - збирати ресурси (тільки для рабів)
- **ПКМ на ворогу** - атакувати (для воїнів)
- **ESC в грі** - повернутися в меню

**Курсори:**
- **Зелений "H"** - збір ресурсів (над ресурсними точками)
- **Червоний "X"** - атака (над ворожими юнітами)
- **Стандартний** - звичайні дії

### 9. UI/UX
- **HUD** - показує ресурси гравця (їжа, гроші) у верхньому лівому куті
- **Панель замовлення** - з'являється при виборі будівлі (як в Козаках)
- **Інформація про вибране** - характеристики юніта/будівлі
- **Статистика** - кількість юнітів кожного типу
- **Прогрес виробництва** - візуальний індикатор

### 10. AI ПРОТИВНИКА
- Ворожі будівлі автоматично виробляють юніти
- Ворожі юніти патрулюють та атакують гравця
- Базова стратегія захисту та нападу

### 11. АУДІО СИСТЕМА (ОНОВЛЕНО! Сесія 4)
**Динамічна музична система з 7 станами та пріоритетами:**
1. **Меню** (пріоритет 1) - `Punic wars_ Castra.mp3` - грає в головному меню
2. **Амбієнт** (пріоритет 2) - 2 треки, ротація - фонова музика геймплею
3. **Бойова Рим** (пріоритет 3) - 2 треки, ротація - бій за Рим
4. **Бойова Карфаген** (пріоритет 4) - 2 треки, ротація - бій за Карфаген
5. **Напруга** (пріоритет 5) - 2 треки, ротація - ворог атакує/поблизу
6. **Мирна** (пріоритет 6) - 2 треки, ротація - розвиток табору
7. **Поразка** (пріоритет 7) - 2 треки, ротація - кінець гри

**Автоматичне перемикання на основі ігрової ситуації:**
- Ворог в радіусі 200px → Напруга
- Бій 3+ секунди → Бойова музика (залежить від фракції гравця)
- Мир 10+ секунд → Мирна музика
- Мир 2-10 секунд → Амбієнт
- Треки автоматично ротуються випадково при завершенні

**Всього музичних файлів:** 13 (1 меню + 6 станів × 2 треки)

**Візуальні покращення:**
- Фоновий малюнок в головному меню (`Background.png`)
- Напівпрозорий оверлей для кращої читабельності тексту

### 12. МЕНЮ НАЛАШТУВАНЬ (НОВЕ! Сесія 3)
**Доступ:** Кнопка "SETTINGS" в головному меню

**Повзунки гучності (0-100%):**
- **Music Volume** (Музика) - контролює гучність фонової музики
- **Ambient Volume** (Середовище) - зарезервовано для звуків середовища
- **Effects Volume** (Ефекти) - зарезервовано для звукових ефектів

**Функції:**
- Регулювання гучності в реальному часі
- Візуальні індикатори (зелений/синій/червоний)
- Відсотки поруч з кожним повзунком
- Кнопка "BACK" для повернення в меню

## БАЛАНС ГРИ

### Початкові ресурси
- **Рим**: 200 їжі, 100 грошей
- **Карфаген**: 150 їжі, 200 грошей

### Швидкості (зменшено вдвоє для балансу)
- **Воїни**: 0.85f
- **Раби**: 0.6f

### Час виробництва (збільшено вдвоє)
- **Легіонер**: 10 секунд
- **Фінікієць**: 9 секунд
- **Раб**: 6 секунд

### Обмеження
- Максимум 8 легіонерів на одну казарму
- Немає обмежень для інших юнітів

## КОМПІЛЯЦІЯ ТА ЗАПУСК

### Команда компіляції
```bash
C:\\raylib\\w64devkit\\bin\\g++.exe src\\main.cpp -o punic_wars.exe ^
  -IC:\\raylib\\raylib\\include ^
  -LC:\\raylib\\raylib\\lib ^
  -lraylib -lopengl32 -lgdi32 -lwinmm
```

### Швидкий запуск
```bash
cd cpp
.\\compile.bat    # Компіляція
.\\run.bat        # Запуск
```

## НАСТУПНІ КРОКИ (Пріоритети)

### Пріоритет 1: Основний геймплей
- [ ] Умови перемоги/поразки
- [ ] Більше типів юнітів (Веліт, Пельтаст, Нумідієць)
- [ ] Покращення AI (стратегія, захист бази)
- [ ] Система морального духу (з оригінального ТЗ)

### Пріоритет 2: Візуал та звук
- [x] Фонова музика (меню та бій)
- [x] Динамічна музична система з 7 станами
- [x] Ротація треків (2 треки на стан)
- [x] Меню налаштувань звуку
- [x] Фоновий малюнок меню
- [ ] Звукові ефекти (атака, виробництво, збір)
- [ ] Звуки середовища
- [ ] Спрайти для юнітів замість кружечків
- [ ] Спрайти для будівель замість прямокутників

### Пріоритет 3: Контент
- [ ] Кампанія з 3 місіями
- [ ] Більше типів ресурсів
- [ ] Покращення будівель (як Шатер лівійців 6→8→10)
- [ ] Технології/апгрейди

### Пріоритет 4: Поліш
- [ ] Міні-карта
- [ ] Збереження/завантаження гри
- [ ] Збереження налаштувань звуку
- [ ] Туторіал

## ТЕХНІЧНІ ДЕТАЛІ

### Використані технології
- **Мова**: C++
- **Бібліотека**: Raylib 5.5
- **Компілятор**: g++ (w64devkit)
- **Платформа**: Windows
- **Роздільна здатність**: 1434x1075
- **FPS**: 60

### Аудіо система
- **Raylib Audio Device** - ініціалізація аудіо
- **Music Streams** - потокове відтворення музики
- **Volume Control** - динамічне регулювання гучності
- **Формат**: MP3 (48000 Hz, 32 bit, Stereo)
- **Динамічна система**: 7 музичних станів з автоматичним перемиканням
- **Ротація треків**: кожен стан має 2 треки що чергуються випадково
- **Контекстне перемикання**: музика змінюється на основі ігрової ситуації

### Архітектура коду
- **Модульна структура**: building.h, unit.h, resource.h окремо від main.cpp
- **Стани гри**: MENU → SETTINGS → FACTION_SELECT → PLAYING
- **Система вибору**: ЛКМ для вибору, ПКМ для команд
- **Візуальний фідбек**: виділення, індикатори здоров'я, лінії руху
- **AudioSettings структура**: зберігає налаштування гучності

### Розміри об'єктів
- **Юніти**: кружечки 16px радіус
- **Будівлі**: прямокутники 80x60px
- **Ресурсні точки**: квадрати 40x40px

## ВІДОМІ ПРОБЛЕМИ
Немає критичних проблем. Гра стабільна, всі аудіо функції працюють коректно.

## КОРИСНІ КОМАНДИ

### Розробка
```bash
# Компіляція після змін
taskkill /f /im punic_wars.exe 2>nul & .\\compile.bat

# Вбити процес якщо завис
taskkill /f /im punic_wars.exe
```

## ДОКУМЕНТАЦІЯ
- `cpp/AUDIO_FEATURES.md` - детальна документація аудіо системи
- `cpp/DEVELOPMENT_LOG.md` - повний лог розробки
- `SESSION_CONTEXT.md` - контекст для наступної сесії

---
**Статус**: ✅ Повністю функціональна RTS гра з динамічною музичною системою
**Останнє оновлення**: Сесія 4 завершена
**Використано токенів**: ~165k / 200k (82%)
"""

@app.list_tools()
async def list_tools() -> list[Tool]:
    """Список доступних інструментів"""
    return [
        Tool(
            name="get_project_context",
            description="Отримати контекст проєкту (all/code/specs/log/current_state)",
            inputSchema={
                "type": "object",
                "properties": {
                    "section": {
                        "type": "string",
                        "description": "Секція контексту: all, code, specs, log, current_state",
                        "enum": ["all", "code", "specs", "log", "current_state"]
                    }
                },
                "required": ["section"]
            }
        ),
        Tool(
            name="get_game_features",
            description="Отримати поточний стан всіх функцій гри",
            inputSchema={
                "type": "object",
                "properties": {}
            }
        ),
        Tool(
            name="search_code",
            description="Пошук в коді проєкту",
            inputSchema={
                "type": "object",
                "properties": {
                    "query": {
                        "type": "string",
                        "description": "Пошуковий запит"
                    },
                    "file_type": {
                        "type": "string",
                        "description": "Тип файлів: cpp, h, md, all",
                        "enum": ["cpp", "h", "md", "all"]
                    }
                },
                "required": ["query"]
            }
        ),
        Tool(
            name="get_compilation_info",
            description="Інформація про компіляцію та налаштування",
            inputSchema={
                "type": "object",
                "properties": {}
            }
        )
    ]

@app.call_tool()
async def call_tool(name: str, arguments: dict) -> list[TextContent]:
    """Обробка викликів інструментів"""
    
    if name == "get_project_context":
        section = arguments.get("section", "all")
        
        if section == "all":
            result = get_project_summary()
        elif section == "code":
            main_cpp = read_file_safe(PROJECT_ROOT / "cpp" / "src" / "main.cpp")
            building_h = read_file_safe(PROJECT_ROOT / "cpp" / "src" / "building.h")
            unit_h = read_file_safe(PROJECT_ROOT / "cpp" / "src" / "unit.h")
            resource_h = read_file_safe(PROJECT_ROOT / "cpp" / "src" / "resource.h")
            result = f"# КОД ПРОЄКТУ\n\n## main.cpp\n{main_cpp}\n\n## building.h\n{building_h}\n\n## unit.h\n{unit_h}\n\n## resource.h\n{resource_h}"
        elif section == "specs":
            spec_path = PROJECT_ROOT / ".kiro" / "specs" / "2d-strategy-game"
            req = read_file_safe(spec_path / "requirements.md")
            design = read_file_safe(spec_path / "design.md")
            tasks = read_file_safe(spec_path / "tasks.md")
            result = f"# СПЕЦИФІКАЦІЇ\n\n## Requirements\n{req}\n\n## Design\n{design}\n\n## Tasks\n{tasks}"
        elif section == "log":
            log = read_file_safe(PROJECT_ROOT / "cpp" / "DEVELOPMENT_LOG.md")
            result = f"# ЛОГ РОЗРОБКИ\n\n{log}"
        elif section == "current_state":
            result = get_project_summary()
        else:
            result = "Невідома секція"
            
        return [TextContent(type="text", text=result)]
    
    elif name == "get_game_features":
        result = get_project_summary()
        return [TextContent(type="text", text=result)]
    
    elif name == "search_code":
        query = arguments.get("query", "")
        file_type = arguments.get("file_type", "all")
        
        results = []
        search_paths = []
        
        if file_type in ["cpp", "all"]:
            search_paths.append(PROJECT_ROOT / "cpp" / "src" / "main.cpp")
        if file_type in ["h", "all"]:
            search_paths.extend([
                PROJECT_ROOT / "cpp" / "src" / "building.h",
                PROJECT_ROOT / "cpp" / "src" / "unit.h",
                PROJECT_ROOT / "cpp" / "src" / "resource.h"
            ])
        if file_type in ["md", "all"]:
            search_paths.append(PROJECT_ROOT / "cpp" / "DEVELOPMENT_LOG.md")
        
        for path in search_paths:
            if path.exists():
                content = path.read_text(encoding='utf-8')
                if query.lower() in content.lower():
                    lines = content.split('\n')
                    matching_lines = [f"{i+1}: {line}" for i, line in enumerate(lines) if query.lower() in line.lower()]
                    results.append(f"\n## {path.name}\n" + "\n".join(matching_lines[:10]))
        
        result = f"# РЕЗУЛЬТАТИ ПОШУКУ: '{query}'\n" + "\n".join(results) if results else f"Нічого не знайдено для '{query}'"
        return [TextContent(type="text", text=result)]
    
    elif name == "get_compilation_info":
        compile_bat = read_file_safe(PROJECT_ROOT / "cpp" / "compile.bat")
        setup = read_file_safe(PROJECT_ROOT / "cpp" / "SETUP.md")
        result = f"# ІНФОРМАЦІЯ ПРО КОМПІЛЯЦІЮ\n\n## compile.bat\n{compile_bat}\n\n## SETUP.md\n{setup}"
        return [TextContent(type="text", text=result)]
    
    else:
        return [TextContent(type="text", text=f"Невідомий інструмент: {name}")]

async def main():
    """Запуск MCP сервера"""
    async with mcp.server.stdio.stdio_server() as (read_stream, write_stream):
        await app.run(
            read_stream,
            write_stream,
            app.create_initialization_options()
        )

if __name__ == "__main__":
    asyncio.run(main())