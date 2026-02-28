# Оновлення: Динамічні кнопки

## Зміни

Додано систему динамічних кнопок, які автоматично підлаштовуються під довжину тексту.

## Нові файли

1. `src/ui_button.h` - заголовочний файл з класом DynamicButton
2. `src/ui_button.cpp` - реалізація динамічних кнопок

## Використані текстури

Кнопки складаються з трьох частин:
- `Button_left.png` - ліва частина кнопки
- `Button_base.png` - середня частина (розтягується)
- `Button_right.png` - права частина

Hover стани:
- `Button_hover_left.png`
- `Button_hover_base.png`
- `Button_hover_right.png`

## Функціонал

- Автоматичний розрахунок ширини кнопки на основі тексту
- Підтримка hover та pressed станів
- Плавна зміна кольору тексту при наведенні
- Fallback на прості прямокутники якщо текстури не завантажені

## Інтеграція

Кнопки інтегровані в:
- Головне меню (START GAME, SETTINGS, EXIT)
- Екран вибору фракції (ROME, CARTHAGE, BACK)
- Меню налаштувань (BACK)

## Використання

```cpp
// Створення кнопки (статична змінна для збереження стану)
static DynamicButton myButton(x, y, "Button Text", fontSize);

// Оновлення стану
Vector2 mousePos = GetMousePosition();
myButton.Update(mousePos);

// Малювання
myButton.Draw();

// Перевірка кліку
if (myButton.IsClicked()) {
    // Обробка кліку
}
```

## Компіляція

Оновлено `compile.bat` для включення `ui_button.cpp` в компіляцію.
