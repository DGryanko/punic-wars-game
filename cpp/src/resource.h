#pragma once
#include "raylib.h"
#include <string>

// Типи ресурсів
enum ResourceType {
    FOOD_SOURCE,    // Їжа (ферма, ліс)
    GOLD_SOURCE     // Золото (рудник)
};

// Структура ресурсної точки
struct ResourcePoint {
    int x, y;                    // Координати
    ResourceType type;           // Тип ресурсу
    int amount;                  // Кількість ресурсу
    int max_amount;              // Максимальна кількість
    std::string name;            // Назва для відображення
    bool depleted = false;       // Чи вичерпано ресурс
    
    // Ініціалізація ресурсної точки
    void init(ResourceType resourceType, int posX, int posY, int resourceAmount) {
        type = resourceType;
        x = posX;
        y = posY;
        amount = resourceAmount;
        max_amount = resourceAmount;
        depleted = false;
        
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
    
    // Отримати прямокутник для колізій
    Rectangle getRect() const {
        return {(float)x, (float)y, 40, 40};
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
        Color resourceColor = getColor();
        
        // Малювання ресурсу як квадрата
        DrawRectangle(x, y, 40, 40, resourceColor);
        DrawRectangleLines(x, y, 40, 40, WHITE);
        
        // Іконка типу ресурсу
        if (type == FOOD_SOURCE) {
            DrawText("F", x + 15, y + 15, 16, WHITE);
        } else if (type == GOLD_SOURCE) {
            DrawText("G", x + 15, y + 15, 16, WHITE);
        }
        
        // Кількість ресурсу
        if (!depleted) {
            DrawText(TextFormat("%d", amount), x + 5, y + 25, 10, WHITE);
        } else {
            DrawText("EMPTY", x + 2, y + 25, 8, RED);
        }
    }
    
    // Перевірка кліку
    bool isClicked(Vector2 mousePos) const {
        return CheckCollisionPointRec(mousePos, getRect());
    }
};