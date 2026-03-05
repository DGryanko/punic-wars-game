# Kiro IDE Settings

## MCP Server Configuration

### Punic Wars Context Server

MCP сервер для швидкого доступу до контексту проєкту.

**Налаштування:**
- Файл: `.kiro/settings/mcp.json`
- Скрипт: `punic_wars_mcp.py` (в корені проєкту)
- Використовує відносні шляхи (працює з будь-якої директорії)

**Після клонування репозиторію:**
1. Переконайтесь що Python встановлений: `python --version`
2. Встановіть залежності: `pip install mcp`
3. Перезапустіть Kiro IDE або виконайте "MCP: Restart Server"

**Якщо MCP не працює:**
1. Відкрийте Command Palette (Ctrl+Shift+P)
2. Виконайте "MCP: Restart Server"
3. Виберіть "punic-wars-context"
4. Перевірте логи в панелі "MCP Servers"

**Доступні команди:**
- `get_project_context` - отримати контекст проєкту
- `get_game_features` - поточний стан функцій гри
- `search_code` - пошук в коді
- `get_compilation_info` - інформація про компіляцію

### Troubleshooting

**Проблема:** MCP сервер не запускається
- **Рішення:** Перевірте чи встановлений Python та пакет `mcp`
- **Команда:** `pip install mcp`

**Проблема:** Помилка "File not found"
- **Рішення:** Переконайтесь що файл `punic_wars_mcp.py` існує в корені проєкту
- **Команда:** `ls punic_wars_mcp.py` (Linux/Mac) або `dir punic_wars_mcp.py` (Windows)

**Проблема:** Зміни в конфігурації не застосовуються
- **Рішення:** Перезапустіть Kiro IDE або виконайте "MCP: Reconnect Servers"
