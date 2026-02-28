#include "ui_button.h"
#include <algorithm>

// Ініціалізація статичних змінних
Texture2D DynamicButton::leftPart = {0};
Texture2D DynamicButton::basePart = {0};
Texture2D DynamicButton::rightPart = {0};
Texture2D DynamicButton::leftPartHover = {0};
Texture2D DynamicButton::basePartHover = {0};
Texture2D DynamicButton::rightPartHover = {0};

void DynamicButton::LoadTextures() {
    leftPart = LoadTexture("assets/sprites/Button_left.png");
    basePart = LoadTexture("assets/sprites/Button_base.png");
    rightPart = LoadTexture("assets/sprites/Button_right.png");
    leftPartHover = LoadTexture("assets/sprites/Button_hover_left.png");
    basePartHover = LoadTexture("assets/sprites/Button_hover_base.png");
    rightPartHover = LoadTexture("assets/sprites/Button_hover_right.png");
    
    printf("[UI] Button textures loaded: left=%d, base=%d, right=%d\n", 
           leftPart.id, basePart.id, rightPart.id);
}

void DynamicButton::UnloadTextures() {
    if (leftPart.id > 0) UnloadTexture(leftPart);
    if (basePart.id > 0) UnloadTexture(basePart);
    if (rightPart.id > 0) UnloadTexture(rightPart);
    if (leftPartHover.id > 0) UnloadTexture(leftPartHover);
    if (basePartHover.id > 0) UnloadTexture(basePartHover);
    if (rightPartHover.id > 0) UnloadTexture(rightPartHover);
}

DynamicButton::DynamicButton(float x, float y, const std::string& buttonText, int fontSize)
    : text(buttonText), fontSize(fontSize), isHovered(false), isPressed(false) {
    
    CalculateWidth();
    
    // Висота кнопки - фіксована, нормального розміру (229 * 0.4 = ~92 пікселі)
    float height = 229 * 0.4f; // Масштабована висота текстури
    bounds = {x, y, (float)minWidth, height};
}

void DynamicButton::CalculateWidth() {
    // Вимірюємо ширину тексту
    textWidth = MeasureText(text.c_str(), fontSize);
    
    // Фіксовані розміри для нормальних кнопок (з урахуванням масштабу 0.4)
    int leftWidth = 327 * 0.4f;  // ~130 пікселів
    int rightWidth = 327 * 0.4f; // ~130 пікселів
    
    // Мінімальна ширина середньої частини (для тексту + відступи)
    int padding = 60; // Відступи з боків тексту
    int middleWidth = textWidth + padding;
    
    // Загальна ширина
    minWidth = leftWidth + middleWidth + rightWidth;
}

void DynamicButton::Update(Vector2 mousePos) {
    isHovered = CheckCollisionPointRec(mousePos, bounds);
    isPressed = isHovered && IsMouseButtonDown(MOUSE_LEFT_BUTTON);
}

void DynamicButton::Draw() {
    // Вибираємо текстури залежно від стану
    Texture2D* left = isHovered ? &leftPartHover : &leftPart;
    Texture2D* base = isHovered ? &basePartHover : &basePart;
    Texture2D* right = isHovered ? &rightPartHover : &rightPart;
    
    // Якщо текстури не завантажені, малюємо fallback
    if (left->id == 0 || base->id == 0 || right->id == 0) {
        Color buttonColor = isPressed ? DARKGRAY : (isHovered ? GRAY : LIGHTGRAY);
        DrawRectangleRec(bounds, buttonColor);
        DrawRectangleLinesEx(bounds, 2, BLACK);
        
        // Центруємо текст
        int textX = bounds.x + (bounds.width - textWidth) / 2;
        int textY = bounds.y + (bounds.height - fontSize) / 2;
        DrawText(text.c_str(), textX, textY, fontSize, BLACK);
        return;
    }
    
    // Масштаб для зменшення текстур (робимо їх у 2.5 рази меншими)
    float scale = 0.4f;
    int leftWidth = left->width * scale;
    int rightWidth = right->width * scale;
    
    // Перекриття - бокові частини заходять на центральну
    int overlap = 20; // Пікселів перекриття з кожного боку
    
    // Центральна частина вужча - віднімаємо перекриття з обох боків
    int middleWidth = bounds.width - leftWidth - rightWidth + (overlap * 2);
    
    // СПОЧАТКУ малюємо середню частину (вона має бути ПІД боковими)
    // Зміщуємо її вліво на величину перекриття
    Rectangle sourceRec = {0, 0, (float)base->width, (float)base->height};
    Rectangle destRec = {bounds.x + leftWidth - overlap, bounds.y, (float)middleWidth, bounds.height};
    DrawTexturePro(*base, sourceRec, destRec, {0, 0}, 0, WHITE);
    
    // ПОТІМ малюємо ліву частину (масштабовану) - вона перекриває центр
    Rectangle leftSource = {0, 0, (float)left->width, (float)left->height};
    Rectangle leftDest = {bounds.x, bounds.y, (float)leftWidth, bounds.height};
    DrawTexturePro(*left, leftSource, leftDest, {0, 0}, 0, WHITE);
    
    // І праву частину (масштабовану) - вона теж перекриває центр
    Rectangle rightSource = {0, 0, (float)right->width, (float)right->height};
    Rectangle rightDest = {bounds.x + bounds.width - rightWidth, bounds.y, (float)rightWidth, bounds.height};
    DrawTexturePro(*right, rightSource, rightDest, {0, 0}, 0, WHITE);
    
    // Малюємо текст по центру (зверху всього)
    int textX = bounds.x + (bounds.width - textWidth) / 2;
    int textY = bounds.y + (bounds.height - fontSize) / 2 + 2; // +2 для кращого вирівнювання
    
    // Малюємо чорне обведення (8 напрямків)
    int outlineSize = 2;
    for (int ox = -outlineSize; ox <= outlineSize; ox++) {
        for (int oy = -outlineSize; oy <= outlineSize; oy++) {
            if (ox != 0 || oy != 0) {
                DrawText(text.c_str(), textX + ox, textY + oy, fontSize, BLACK);
            }
        }
    }
    
    // Малюємо білий текст зверху
    DrawText(text.c_str(), textX, textY, fontSize, WHITE);
}

bool DynamicButton::IsClicked() {
    return isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}
