# Punic Wars: Castra MCP Server

## Що це?
Спеціалізований MCP (Model Context Protocol) сервер для економії токенів під час розробки гри. Замість того, щоб кожен раз читати всі файли проєкту, можна швидко отримати контекст через MCP.

## Встановлення

1. Встановити залежності:
```bash
pip install -r requirements.txt
```

2. MCP сервер автоматично налаштований в `.kiro/settings/mcp.json`

## Доступні команди

### `get_project_context`
Отримати повний або частковий контекст проєкту:
- `section: "all"` - весь контекст
- `section: "code"` - тільки код (main.cpp, *.h файли)  
- `section: "specs"` - тільки специфікації
- `section: "log"` - тільки лог розробки
- `section: "current_state"` - поточний стан функцій

### `get_game_features`
Отримати поточний стан всіх функцій гри, систем та налаштувань.

### `search_code`
Пошук в коді проєкту:
- `query` - пошуковий запит
- `file_type` - тип файлів ("cpp", "h", "md", "all")

### `get_compilation_info`
Інформація про компіляцію, налаштування Raylib та команди запуску.

### `update_dev_log`
Додати новий запис до логу розробки.

## Приклади використання

```python
# Отримати весь контекст проєкту
context = await mcp_client.call_tool("get_project_context", {"section": "all"})

# Отримати тільки код
code = await mcp_client.call_tool("get_project_context", {"section": "code"})

# Пошук функції
results = await mcp_client.call_tool("search_code", {
    "query": "HandleClicks", 
    "file_type": "cpp"
})

# Поточний стан гри
features = await mcp_client.call_tool("get_game_features", {})
```

## Переваги

1. **Економія токенів** - не потрібно читати всі файли кожен раз
2. **Швидкість** - миттєвий доступ до контексту
3. **Структурованість** - організований доступ до різних частин проєкту
4. **Пошук** - швидкий пошук по коду
5. **Актуальність** - автоматичне оновлення контексту

## Структура проєкту, що кешується

- **Код**: main.cpp, building.h, unit.h, resource.h
- **Специфікації**: requirements.md, design.md, tasks.md  
- **Лог**: DEVELOPMENT_LOG.md
- **Стан**: поточні функції, системи, налаштування

## Налаштування

MCP сервер налаштовано в `.kiro/settings/mcp.json` з автоматичним схваленням основних команд для зручності використання.