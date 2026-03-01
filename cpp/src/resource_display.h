#ifndef RESOURCE_DISPLAY_H
#define RESOURCE_DISPLAY_H

#include "raylib.h"
#include "building.h"

// Відображення ресурсів на верхній панелі
class ResourceDisplay {
private:
    Texture2D resourcePanel;
    Faction playerFaction;
    
    // Позиції тексту на панелі
    Vector2 foodTextPos;
    Vector2 moneyTextPos;
    
public:
    ResourceDisplay() : playerFaction(ROME) {
        foodTextPos = {0, 0};
        moneyTextPos = {0, 0};
    }
    
    // Ініціалізація
    void init(Texture2D panel, Faction faction) {
        resourcePanel = panel;
        playerFaction = faction;
        calculateTextPositions();
    }
    
    // Малювання ресурсів
    void draw(int food, int money, int foodReserved, int moneyReserved) const {
        // Обчислюємо доступні ресурси
        int availableFood = food - foodReserved;
        int availableMoney = money - moneyReserved;
        
        // Малюємо панель якщо завантажена
        if (resourcePanel.id > 0) {
            // Малюємо панель ресурсів зверху по центру
            float panelX = (1434 - resourcePanel.width) / 2.0f;
            DrawTexture(resourcePanel, (int)panelX, 10, WHITE);
            
            // Малюємо значення ресурсів на панелі
            // Ліва частина панелі - їжа (амфора)
            DrawText(TextFormat("%d", availableFood), (int)panelX + 280, 50, 28, {255, 255, 255, 255});
            
            // Права частина панелі - гроші (монета) - зміщено праворуч
            DrawText(TextFormat("%d", availableMoney), (int)panelX + 680, 50, 28, {255, 255, 255, 255});
        } else {
            // Fallback якщо панель не завантажилась
            DrawRectangle(0, 0, 1434, 80, {0, 0, 0, 180});
            
            if (playerFaction == ROME) {
                DrawText(TextFormat("ROME - Food: %d (%d) | Money: %d (%d)", 
                         availableFood, food, availableMoney, money), 
                         10, 10, 16, WHITE);
                DrawText("Your faction (available/total)", 10, 30, 14, LIGHTGRAY);
            } else {
                DrawText(TextFormat("CARTHAGE - Food: %d (%d) | Money: %d (%d)", 
                         availableFood, food, availableMoney, money), 
                         10, 10, 16, ORANGE);
                DrawText("Your faction (available/total)", 10, 30, 14, LIGHTGRAY);
            }
        }
    }
    
private:
    // Розрахувати позиції тексту
    void calculateTextPositions() {
        if (resourcePanel.id > 0) {
            float panelX = (1434 - resourcePanel.width) / 2.0f;
            foodTextPos = {panelX + 250, 50};
            moneyTextPos = {panelX + 750, 50};
        }
    }
};

#endif // RESOURCE_DISPLAY_H
