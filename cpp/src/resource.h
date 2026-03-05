#pragma once
#include "raylib.h"
#include "tilemap/coordinates.h"
#include "isometric_sprite.h"
#include <string>

// Типи ресурсів
enum ResourceType {
    FOOD_SOURCE,    // Їжа (ферма, ліс)
    GOLD_SOURCE     // Золото (рудник)
};

// Структура ресурсної точки
struct ResourcePoint {
    // НОВИЙ ПІДХІД: Grid coordinates (ізометрична система)
    GridCoords position;         // Поточні координати в сітці
    
    // COMPATIBILITY LAYER: Публічні змінні для старого коду
    // Ці змінні автоматично синхронізуються з grid координатами
    int x, y;                    // Screen coordinates (deprecated, але працюють)
    
    ResourceType type;           // Тип ресурсу
    int amount;                  // Кількість ресурсу
    int max_amount;              // Максимальна кількість
    std::string name;            // Назва для відображення
    bool depleted = false;       // Чи вичерпано ресурс
    
    // Isometric sprite system
    IsometricSprite sprite;      // Спрайт ресурсу
    bool useDebugRendering;      // Чи використовувати debug режим
    
    // Ініціалізація ресурсної точки
    void init(ResourceType resourceType, GridCoords startPos, int resourceAmount) {
        type = resourceType;
        position = startPos;
        amount = resourceAmount;
        max_amount = resourceAmount;
        depleted = false;
        useDebugRendering = true; // За замовчуванням використовуємо debug
        
        // COMPATIBILITY: Синхронізуємо screen coordinates
        syncScreenCoords();
        
        // Спроба завантажити спрайт
        std::string spritePath = "assets/sprites/isometric/resources/";
        if (type == FOOD_SOURCE) {
            spritePath += "food_source.png";
        } else {
            spritePath += "gold_source.png";
        }
        
        if (sprite.loadFromFile(spritePath.c_str())) {
            useDebugRendering = false;
        } else {
            TraceLog(LOG_INFO, "[RESOURCE] Using debug rendering for type %d", type);
        }
        
        // Встановлення назви залежно від типу
        switch (type) {
            case FOOD_SOURCE:
                name = "Food Source";
                break;
            case GOLD_SOURCE:
                name = "Gold Mine";
                break;
        }
    }
    
    // COMPATIBILITY: Синхронізація screen coordinates з grid coordinates
    void syncScreenCoords() {
        ScreenCoords screen = CoordinateConverter::gridToScreen(position);
        x = (int)screen.x;
        y = (int)screen.y;
    }
    
    // Отримати screen позицію (для рендерингу)
    ScreenCoords getScreenPosition() const {
        return CoordinateConverter::gridToScreen(position);
    }
    
    // Отримати grid позицію
    GridCoords getGridPosition() const {
        return position;
    }
    
    // Отримати прямокутник для колізій
    Rectangle getRect() const {
        ScreenCoords screenPos = getScreenPosition();
        return {screenPos.x - 20, screenPos.y - 20, 40, 40};
    }
    
    // Отримати колір залежно від типу
    Color getColor() const {
        if (depleted) return DARKGRAY;
        
        switch (type) {
            case FOOD_SOURCE:
                return GREEN;
            case GOLD_SOURCE:
                return GOLD;
            default:
                return GRAY;
        }
    }
    
    // Зібрати ресурс
    int harvest(int harvestAmount) {
        if (depleted) return 0;
        
        int actualHarvest = (harvestAmount > amount) ? amount : harvestAmount;
        amount -= actualHarvest;
        
        if (amount <= 0) {
            depleted = true;
            amount = 0;
        }
        
        return actualHarvest;
    }
    
    // Малювання ресурсної точки
    void draw() const {
        ScreenCoords screenPos = getScreenPosition();
        
        if (useDebugRendering) {
            // Debug режим - квадрат з літерою
            Color resourceColor = getColor();
            char letter = (type == FOOD_SOURCE) ? 'F' : 'G';
            sprite.drawDebugResource(screenPos, resourceColor, letter);
            
            // Кількість ресурсу під квадратом
            if (!depleted) {
                std::string amountText = std::to_string(amount);
                int textWidth = MeasureText(amountText.c_str(), 10);
                DrawText(amountText.c_str(), (int)screenPos.x - textWidth/2, (int)screenPos.y + 15, 10, WHITE);
            } else {
                DrawText("EMPTY", (int)screenPos.x - 20, (int)screenPos.y + 15, 8, RED);
            }
        } else {
            // Рендеринг спрайту
            Color tint = depleted ? DARKGRAY : WHITE;
            sprite.draw(screenPos, tint);
            
            // Кількість ресурсу
            if (!depleted) {
                std::string amountText = std::to_string(amount);
                int textWidth = MeasureText(amountText.c_str(), 10);
                DrawText(amountText.c_str(), (int)screenPos.x - textWidth/2, (int)screenPos.y + 20, 10, WHITE);
            }
        }
    }
    
    // Перевірка кліку
    bool isClicked(Vector2 mousePos) const {
        return CheckCollisionPointRec(mousePos, getRect());
    }
};