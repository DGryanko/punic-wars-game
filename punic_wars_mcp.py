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

## ПОТОЧНИЙ СТАН (Сесія 9)
- Гра повністю працює та компілюється
- Ізометрична система координат (grid-based, 80x80 тайлів)
- A* pathfinding — юніти обходять будівлі
- Спрайти для всіх юнітів та будівель (ізометричні PNG)
- Обидві фракції (Рим і Карфаген) повністю реалізовані
- Панель замовлення юнітів для HQ та казарм обох фракцій
- Панель будівництва для рабів (+ кнопки Harvest/Drop off)
- Автоматичний цикл збору ресурсів (раб -> ресурс -> база -> ресурс)
- Бойова система: автоатака юнітів і будівель у радіусі 5 тайлів
- HP будівель з візуальним баром
- Динамічна музична система з 7 станами та ротацією треків
- Edge scroll (скрол камери при підведенні миші до краю екрана, 400px/сек)
- Точні точки шляху (рух до точної позиції кліку, не до центру тайлу)
- ПКМ лише для вибору цілі та будівництва (пересування камери через ПКМ прибрано)

## СТРУКТУРА ФАЙЛІВ
```
cpp/
├── src/
│   ├── main.cpp                  # Головний файл (~2250 рядків)
│   ├── building.h                # Система будівель (grid-based, спрайти, HP, виробництво)
│   ├── unit.h                    # Система юнітів (A* pathfinding, збір ресурсів, бій, exact_target_screen)
│   ├── resource.h                # Ресурсні точки (їжа, золото)
│   ├── pathfinding.h             # A* pathfinding на grid
│   ├── unit_order_panel.h        # Панель замовлення юнітів (HQ + казарми обох фракцій)
│   ├── slave_build_panel.h       # Панель будівництва для рабів (QST, CTB/LBT, Harvest, Drop off)
│   ├── faction_spawner.h         # Спавн HQ обох фракцій на старті
│   ├── building_placer.h         # Розміщення будівель на grid
│   ├── isometric_sprite.h        # Рендеринг ізометричних спрайтів
│   ├── tilemap/
│   │   ├── tilemap.h             # Ізометрична карта (80x80)
│   │   ├── coordinates.h         # Конвертація grid <-> screen координат
│   │   └── tilemap_generator.h   # Генерація карти
│   └── ...
├── assets/
│   ├── sprites/isometric/
│   │   ├── buildings/            # Praetorium, Skene, Contubernium, LibTent1-3, Questorium, Tentorium
│   │   └── units/                # legionary_rome, phoenician_carthage, slave_rome, slave_carthage
│   ├── sounds/                   # Музичні треки (13 файлів)
│   └── Background.png
├── compile.bat                   # Компіляція: .\\compile.bat (з директорії cpp)
├── run.bat                       # Запуск гри
└── punic_wars.exe
```

## РЕАЛІЗОВАНІ ФУНКЦІЇ

### 1. МЕНЮ ТА ВИБІР ФРАКЦІЇ
- Головне меню з фоновим малюнком (START GAME / SETTINGS / EXIT)
- Фонова музика в меню
- Екран вибору фракції (ROME / CARTHAGE)
- Гравець керує своєю фракцією, AI — ворожою

### 2. ІЗОМЕТРИЧНА СИСТЕМА
- Grid 80x80 тайлів, ізометрична проекція
- CoordinateConverter::gridToScreen() / screenToGrid()
- Тайл: 64px ширина, 32px висота (ізометрично)
- Всі будівлі та юніти позиціонуються в grid координатах

### 3. A* PATHFINDING
- Повна реалізація A* на grid (pathfinding.h)
- Юніти обходять будівлі та непрохідні тайли
- pathfindingManager.updateGrid(buildings) — оновлення після будівництва
- moveUnitWithPath(unit, GridCoords) — рух до grid позиції
- moveUnitWithPathToScreen(unit, ScreenCoords) — рух до точної screen позиції
- exact_target_screen в Unit — зберігає точну кінцеву точку шляху (не прив'язану до центру тайлу)

### 4. БУДІВЛІ (9 типів)
- HQ_ROME (Praetorium.png) — HP 500, виробляє рабів
- HQ_CARTHAGE (Skene.png) — HP 500, виробляє рабів
- BARRACKS_ROME (Contubernium.png) — HP 300, виробляє легіонерів (макс 8)
- BARRACKS_CARTHAGE (LibTent1.png, назва "Libyan Tent") — HP 300, виробляє фінікійців
- QUESTORIUM_ROME (Questorium.png) — HP 200, склад ресурсів (x10 ліміт)
- LIBTENT_1/2/3, TENTORIUM — HP 150
- Footprint 2x2 тайли для всіх будівель
- HP бар відображається тільки коли будівля пошкоджена
- Знищені будівлі видаляються з вектора + оновлюється pathfinding grid

### 5. ЮНІТИ (3 типи)

Легіонер (legionary_rome.png) — Рим:
- HP: 100, Урон: 25, Дальність: 30px, Перезарядка: 1.5с
- Швидкість: 0.68359375f
- Вартість: 30 їжі + 50 грошей, Час: 10с

Фінікієць (phoenician_carthage.png) — Карфаген:
- HP: 90, Урон: 30, Дальність: 25px, Перезарядка: 1.2с
- Швидкість: 0.68359375f
- Вартість: 25 їжі + 60 грошей, Час: 9с

Раб (slave_rome.png / slave_carthage.png) — обидві фракції:
- HP: 50, Урон: 5, Дальність: 15px, Перезарядка: 2.0с
- Швидкість: 0.48828125f
- Вартість: 10 їжі + 20 грошей, Час: 6с
- Може збирати ресурси та будувати будівлі

### 6. ПАНЕЛЬ ЗАМОВЛЕННЯ ЮНІТІВ (unit_order_panel.h)
- Показується при виборі: HQ_ROME, HQ_CARTHAGE, BARRACKS_ROME, BARRACKS_CARTHAGE
- Тільки для будівель фракції гравця
- HQ -> кнопка "SLV" (раб)
- BARRACKS_ROME -> кнопка "LEG" (легіонер)
- BARRACKS_CARTHAGE -> кнопка "PHO" (фінікієць)
- ЛКМ — замовити, ПКМ — скасувати останній у черзі
- Показує чергу та прогрес виробництва

### 7. ПАНЕЛЬ БУДІВНИЦТВА РАБІВ (slave_build_panel.h)
- Показується при виборі раба фракції гравця
- Панель: {10, 940, 380, 110}
- Кнопка "QST" — побудувати Questorium (20 їжі + 30 грошей)
- Кнопка "CTB"/"LBT" — побудувати казарму (30 їжі + 50 грошей)
- Кнопка "Harvest" — знайти ресурс у радіусі 3 тайлів і відправити раба збирати
- Кнопка "Drop off" — відправити раба до найближчого HQ або Questorium

### 8. СИСТЕМА РЕСУРСІВ
- FOOD_SOURCE та GOLD_SOURCE на карті
- Радіус збору: 2 тайли
- Швидкість збору: 1 одиниця/сек
- Здача ресурсів: до HQ або Questorium

### 9. БОЙОВА СИСТЕМА
- Автоатака: пошук ворогів у радіусі 320px (screen coords)
- Пошук ворогів використовує screen coords (getScreenPosition()) — бачить рухомих юнітів
- Атака будівель: радіус attack_range * 5.0f (~150px)
- Переслідування будівель: іде до findNearestWalkableTile(bGrid)
- AI makeAIDecision не перебиває атаку (пропускає якщо is_attacking або target_unit_id >= 0)
- Легіонери не застигають при атаці будівель (is_attacking НЕ виставляється для будівель)
- Індикатори шляху відображаються для вибраних юнітів

### 10. КЕРУВАННЯ КАМЕРОЮ
- WASD / стрілки — рух камери
- Середня кнопка миші — перетягування камери
- ПКМ — НЕ переміщує камеру (тільки вибір цілі та будівництво)
- Edge scroll: миша в межах 20px від краю екрана -> камера рухається 400px/сек

### 11. КЕРУВАННЯ ЮНІТАМИ
- ЛКМ — вибір юніта / будівлі
- Перетягування ЛКМ — drag selection
- ПКМ на порожньому місці — рух до точної позиції кліку (не до центру тайлу)
- ПКМ на ресурсі — збирати ресурс
- ПКМ на ворогу — атакувати

### 12. МУЗИЧНА СИСТЕМА
- 7 станів: MENU, PEACE, TENSION, BATTLE, VICTORY, DEFEAT, BUILDING
- Ротація треків у кожному стані
- Crossfade між станами
- Меню налаштувань гучності

## БАЛАНС ГРИ

Початкові ресурси:
- Рим: 200 їжі, 100 грошей
- Карфаген: 150 їжі, 200 грошей

Швидкості:
- Воїни (легіонер, фінікієць): 0.68359375f
- Раби: 0.48828125f

## КОМПІЛЯЦІЯ ТА ЗАПУСК
```bash
# З директорії cpp
.\\compile.bat    # Компіляція
.\\run.bat        # Запуск
```

## ВАЖЛИВІ ТЕХНІЧНІ ДЕТАЛІ

### Unit.h — exact_target_screen
- ScreenCoords exact_target_screen — точна кінцева точка шляху
- Ініціалізується в init() як current_screen_pos
- setPath() виставляє exact_target_screen = path.back()
- Після проходження всіх waypoints юніт продовжує рух до exact_target_screen (до 2px)

### main.cpp — moveUnitWithPathToScreen
- void moveUnitWithPathToScreen(Unit& unit, ScreenCoords goalScreen)
- Конвертує goalScreen в grid для A*, зберігає goalScreen як exact_target_screen
- Використовується при ПКМ кліку гравця для точного руху

### Атака будівель
- Радіус: attack_range * 5.0f (~150px)
- is_attacking НЕ виставляється для атаки будівель (щоб юніт не застигав)
- Переслідування: findNearestWalkableTile(bGrid)

## ВИПРАВЛЕНІ БАГИ (Сесія 9)
- Фінікійці не атакували рухомих юнітів -> виправлено: пошук через screen coords
- Фінікійці не атакували намети -> виправлено: атака будівель через screen coords
- Легіонери застигали при атаці будівель -> виправлено: прибрано is_attacking = true для будівель
- Точки шляху прив'язувались до вершин тайлів -> виправлено: exact_target_screen
- ПКМ переміщував камеру -> виправлено: прибрано RMB pan

## ТЕХНІЧНІ ДЕТАЛІ
- Мова: C++, Бібліотека: Raylib 5.5
- Компілятор: g++ (w64devkit)
- Роздільна здатність: 1434x1024, 60 FPS
- Компіляція: .\\compile.bat з директорії cpp
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
