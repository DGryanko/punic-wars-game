#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include "raylib.h"
#include <string>

// Структура для динамічної кнопки
struct DynamicButton {
    // Текстури частин кнопки
    static Texture2D leftPart;
    static Texture2D basePart;
    static Texture2D rightPart;
    static Texture2D leftPartHover;
    static Texture2D basePartHover;
    static Texture2D rightPartHover;
    
    // Параметри кнопки
    Rectangle bounds;
    std::string text;
    bool isHovered;
    bool isPressed;
    
    // Ініціалізація статичних текстур (викликати один раз при старті)
    static void LoadTextures();
    static void UnloadTextures();
    
    // Конструктор
    DynamicButton(float x, float y, const std::string& buttonText, int fontSize = 16);
    
    // Оновлення стану кнопки
    void Update(Vector2 mousePos);
    
    // Малювання кнопки
    void Draw();
    
    // Перевірка кліку
    bool IsClicked();
    
private:
    int fontSize;
    int textWidth;
    int minWidth;
    
    // Розрахунок ширини кнопки на основі тексту
    void CalculateWidth();
};

#endif // UI_BUTTON_H
