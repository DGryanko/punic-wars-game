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
# PUNIC WARS: CASTRA — КОНТЕКСТ ПРОЄКТУ (актуально)

## ПОТОЧНИЙ СТАН
- Гра повністю компілюється та запускається
- Ізометрична система координат (grid-based, 80x80 тайлів)
- A* pathfinding — юніти обходять будівлі
- Спрайти для всіх юнітів та будівель (ізометричні PNG)
- Обидві фракції (Рим і Карфаген) повністю реалізовані
- Анімаційна система (UnitAnimator): спрайтшіти або окремі файли, fallback ланцюжок
- Ghost-preview будівлі при розміщенні (реальний спрайт з пульсацією)
- Маркери руху: жовтий (рух), червоний (атака), зелений (збір ресурсів)
- Splash screen при завантаженні (Background.png + "Loading...")
- Збереження налаштувань у settings.ini (fullscreen, гучність)
- Симулятор анімацій: anim_simulator.py (Python/pygame)

## СТРУКТУРА ФАЙЛІВ
```
cpp/
├── src/
│   ├── main.cpp                  # Головний файл (~2600 рядків)
│   ├── building.h                # Система будівель
│   ├── unit.h                    # Система юнітів
│   ├── unit_animator.h           # Анімаційна система (AnimTrack, UnitAnimator)
│   ├── resource.h                # Ресурсні точки
│   ├── pathfinding.h             # A* pathfinding
│   ├── unit_order_panel.h        # Панель замовлення юнітів
│   ├── slave_build_panel.h       # Панель будівництва рабів (ghost preview)
│   ├── building_texture_manager.h/cpp  # Менеджер текстур будівель
│   ├── building_renderer.h/cpp   # Рендеринг будівель
│   ├── isometric_sprite.h/cpp    # Ізометричні спрайти
│   ├── faction_spawner.h         # Спавн ворожого HQ
│   ├── building_placer.h         # Розміщення будівель
│   └── tilemap/                  # Карта, координати, рендерер
├── assets/
│   ├── sprites/isometric/
│   │   ├── buildings/            # PNG спрайти будівель
│   │   └── units/                # Папки з анімаціями юнітів
│   ├── sounds/                   # 14 музичних треків
│   └── Background.png, Logo.png, ...
├── compile.bat                   # Компіляція (з директорії cpp)
├── build_release.bat             # Компіляція + інсталятор + ZIP
└── punic_wars.exe
anim_simulator.py                 # Симулятор анімацій (Python/pygame)
settings.ini                      # Збережені налаштування
```

## АНІМАЦІЙНА СИСТЕМА (unit_animator.h)
- AnimTrack: один анімаційний трек (спрайтшіт або окремі файли)
- UnitAnimator: tracks[4][4] — [AnimState][AnimDir]
- AnimState: ANIM_IDLE=0, ANIM_WALK=1, ANIM_ATTACK=2, ANIM_DEATH=3
- AnimDir: DIR_FRONT_LEFT=0, DIR_FRONT_RIGHT=1, DIR_BACK_LEFT=2, DIR_BACK_RIGHT=3
- loadSheets() шукає: basePath/name.png АБО basePath/name/name.png
- draw() fallback: той самий стан → idle → будь-який завантажений трек

## БУДІВЛІ (9 типів)
- HQ_ROME (Praetorium.png), HQ_CARTHAGE (Skene.png)
- BARRACKS_ROME (Contubernium.png), BARRACKS_CARTHAGE (LibTent1.png)
- QUESTORIUM_ROME (Questorium.png)
- LIBTENT_1/2/3, TENTORIUM
- Footprint 2x2 тайли, texture_offset {-192, -128}

## ЮНІТИ (3 типи)
- legionary (Рим): HP 100, урон 25, швидкість 0.68
- phoenician (Карфаген): HP 90, урон 30, швидкість 0.68
- slave (обидві): HP 50, урон 5, швидкість 0.48, збирає ресурси

## МАРКЕРИ РУХУ (main.cpp — MoveMarker)
```cpp
struct MoveMarker { Vector2 pos; float timer; bool isAttack; bool isHarvest; };
```
- Жовтий: звичайний рух (ПКМ на порожнє місце)
- Червоний: атака (ПКМ на ворога)
- Зелений: збір ресурсів (ПКМ на ресурс рабом)
- Анімація: два кружечки що зменшуються за ~1.25 сек

## ЗБЕРЕЖЕННЯ НАЛАШТУВАНЬ (settings.ini)
- fullscreen, music, ambient, effects
- LoadSettings() при старті, SaveSettings() при зміні

## КОМПІЛЯЦІЯ
```bash
# З директорії cpp:
.\\compile.bat
```

## ТЕХНІЧНІ ДЕТАЛІ
- C++ / Raylib 5.5 / g++ w64devkit
- Роздільна здатність: 1434x1075, 60 FPS
- Python симулятор: python anim_simulator.py (pygame)
"""

