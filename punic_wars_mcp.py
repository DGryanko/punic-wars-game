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

## ПОТОЧНИЙ СТАН (Сесія 8 - АКТИВНА)
✅ Гра повністю працює та компілюється
✅ Ізометрична система координат (grid-based, 80x80 тайлів)
✅ A* pathfinding — юніти обходять будівлі
✅ Спрайти для всіх юнітів та будівель (ізометричні PNG)
✅ Обидві фракції (Рим і Карфаген) повністю реалізовані
✅ Панель замовлення юнітів для HQ та казарм обох фракцій
✅ Панель будівництва для рабів обох фракцій
✅ Автоматичний цикл збору ресурсів (раб → ресурс → база → ресурс)
✅ Динамічна музична система з 7 станами та ротацією треків
✅ Меню налаштувань з регулюванням гучності

## СТРУКТУРА ФАЙЛІВ
```
cpp/
├── src/
│   ├── main.cpp                  # Головний файл (~2200 рядків)
│   ├── building.h                # Система будівель (grid-based, спрайти, виробництво)
│   ├── unit.h                    # Система юнітів (A* pathfinding, збір ресурсів, бій)
│   ├── resource.h                # Ресурсні точки (їжа, золото)
│   ├── pathfinding.h             # A* pathfinding на grid
│   ├── unit_order_panel.h        # Панель замовлення юнітів (HQ + казарми обох фракцій)
│   ├── slave_build_panel.h       # Панель будівництва для рабів
│   ├── faction_spawner.h         # Спавн HQ обох фракцій на старті
│   ├── building_placer.h         # Розміщення будівель на grid
│   ├── isometric_sprite.h        # Рендеринг ізометричних спрайтів
│   ├── tilemap/
│   │   ├── tilemap.h             # Ізометрична карта (80x80)
│   │   ├── coordinates.h         # Конвертація grid ↔ screen координат
│   │   └── tilemap_generator.h   # Генерація карти
│   └── ...
├── assets/
│   ├── sprites/isometric/
│   │   ├── buildings/            # Praetorium, Skene, Contubernium, LibTent1-3, Questorium, Tentorium
│   │   └── units/                # legionary_rome, phoenician_carthage, slave_rome, slave_carthage
│   ├── sounds/                   # Музичні треки (13 файлів)
│   └── Background.png
├── compile.bat                   # Компіляція: .\compile.bat (з директорії cpp)
├── run.bat                       # Запуск гри
└── punic_wars.exe
```

## РЕАЛІЗОВАНІ ФУНКЦІЇ

### 1. МЕНЮ ТА ВИБІР ФРАКЦІЇ
- Головне меню з фоновим малюнком (START GAME / SETTINGS / EXIT)
- Фонова музика в меню
- Екран вибору фракції (ROME / CARTHAGE)
- Гравець керує своєю фракцією, AI — ворожою

### 2. ІЗОМЕТРИЧНА СИСТЕМА (НОВА)
- Grid 80x80 тайлів, ізометрична проекція
- `CoordinateConverter::gridToScreen()` / `screenToGrid()`
- Тайл: 64px ширина, 32px висота (ізометрично)
- Всі будівлі та юніти позиціонуються в grid координатах

### 3. A* PATHFINDING
- Повна реалізація A* на grid (`pathfinding.h`)
- Юніти обходять будівлі та непрохідні тайли
- `pathfindingManager.updateGrid(buildings)` — оновлення після будівництва
- Виправлено критичний баг: `isWalkable(ny, nx)` замість `isWalkable(nx, ny)`

### 4. БУДІВЛІ (9 типів)
- **HQ_ROME** (Praetorium.png) — виробляє рабів
- **HQ_CARTHAGE** (Skene.png) — виробляє рабів
- **BARRACKS_ROME** (Contubernium.png) — виробляє легіонерів (макс 8)
- **BARRACKS_CARTHAGE** (LibTent1.png) — виробляє фінікійців
- **QUESTORIUM_ROME** (Questorium.png) — склад ресурсів (x10 ліміт)
- LIBTENT_1/2/3, TENTORIUM — додаткові типи
- Footprint 2x2 тайли для всіх будівель
- Спавн юнітів: 2 тайли праворуч-вниз від будівлі

### 5. ЮНІТИ (3 типи)

**Легіонер (legionary_rome.png) — Рим**
- HP: 100, Урон: 25, Дальність: 30, Перезарядка: 1.5с
- Швидкість: 0.68359375f
- Вартість: 30 їжі + 50 грошей, Час: 10с

**Фінікієць (phoenician_carthage.png) — Карфаген**
- HP: 90, Урон: 30, Дальність: 25, Перезарядка: 1.2с
- Швидкість: 0.68359375f
- Вартість: 25 їжі + 60 грошей, Час: 9с

**Раб (slave_rome.png / slave_carthage.png) — обидві фракції**
- HP: 50, Урон: 5, Дальність: 15, Перезарядка: 2.0с
- Швидкість: 0.48828125f
- Вартість: 10 їжі + 20 грошей, Час: 6с
- Може збирати ресурси та будувати будівлі

### 6. ПАНЕЛЬ ЗАМОВЛЕННЯ ЮНІТІВ (`unit_order_panel.h`)
- Показується при виборі: HQ_ROME, HQ_CARTHAGE, BARRACKS_ROME, BARRACKS_CARTHAGE
- Тільки для будівель фракції гравця
- HQ → кнопка "SLV" (раб)
- BARRACKS_ROME → кнопка "LEG" (легіонер)
- BARRACKS_CARTHAGE → кнопка "PHO" (фінікієць)
- ЛКМ — замовити, ПКМ — скасувати останній у черзі
- Показує чергу та прогрес виробництва

### 7. ПАНЕЛЬ БУДІВНИЦТВА РАБІВ (`slave_build_panel.h`)
- Показується при виборі раба фракції гравця
- Кнопка "QST" — побудувати Questorium (склад)
- Кнопка "CTB"/"LBT" — побудувати казарму (Contubernium/LibTent залежно від фракції)
- Будівля з'являється поруч з рабом

### 8. СИСТЕМА РЕСУРСІВ
- FOOD_SOURCE та GOLD_SOURCE на карті
- Автоматичний цикл збору: ПКМ по ресурсу → раб → ресурс → база → ресурс
- Радіус збору: 2.0 grid тайли
- Швидкість збору: 2 одиниці за кадр
- Здача ресурсів: до HQ або Questorium

### 9. БОЙОВА СИСТЕМА
- Автоатака ворогів у радіусі
- ПКМ по ворогу — наказ атакувати
- Індикатори здоров'я

### 10. КЕРУВАННЯ
- ЛКМ — вибір юніта/будівлі
- Перетягування ЛКМ — drag selection
- Подвійний клік — вибрати всіх того типу
- ПКМ на землі — рух (A* pathfinding)
- ПКМ на ресурсі — збирати (тільки раби)
- ПКМ на ворогу — атакувати

### 11. АУДІО СИСТЕМА
- 7 музичних станів з автоматичним перемиканням
- Меню налаштувань гучності

## БАЛАНС ГРИ

### Початкові ресурси
- **Рим**: 200 їжі, 100 грошей
- **Карфаген**: 150 їжі, 200 грошей

### Швидкості
- **Воїни**: 0.68359375f
- **Раби**: 0.48828125f

## КОМПІЛЯЦІЯ ТА ЗАПУСК

```bash
# З директорії cpp:
.\compile.bat    # Компіляція
.\run.bat        # Запуск
```

## ВІДОМІ ВИПРАВЛЕНІ БАГИ
- moveTo() більше не скидає has_assigned_resource
- Юніти не застрягають у будівлях при підході до dropoff
- A* pathfinding: виправлено row/col swap у isWalkable()
- Панелі UI переініціалізуються при виборі фракції (playerFaction fix)
- Спавн юнітів: 2 тайли від будівлі, шукає вільний тайл
- isClicked() будівлі: grid-based + sprite fallback

## ТЕХНІЧНІ ДЕТАЛІ
- **Мова**: C++, **Бібліотека**: Raylib 5.5
- **Компілятор**: g++ (w64devkit)
- **Роздільна здатність**: 1434x1075, **FPS**: 60
- **Компіляція**: `.\compile.bat` з директорії `cpp`
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