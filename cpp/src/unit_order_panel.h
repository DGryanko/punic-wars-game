#ifndef UNIT_ORDER_PANEL_H
#define UNIT_ORDER_PANEL_H

#include "raylib.h"
#include "building.h"
#include <vector>
#include <string>

// Зовнішні функції для роботи з ресурсами (оголошені в main.cpp)
extern bool canAfford(Faction faction, const std::string& unitType);
extern void reserveResources(Faction faction, const std::string& unitType);
extern void spendResources(Faction faction, const std::string& unitType);
extern void unreserveResources(Faction faction, const std::string& unitType);

struct UnitCost {
    int food;
    int money;
};

inline UnitCost getUnitCost(const std::string& unitType) {
    if (unitType == "legionary") {
        return {30, 50};
    } else if (unitType == "phoenician") {
        return {25, 60};
    } else if (unitType == "slave") {
        return {10, 20};
    }
    return {0, 0};
}

// UI панель для замовлення юнітів
class UnitOrderPanel {
private:
    int selectedBuildingIndex;
    std::vector<Building>* buildings;
    Faction playerFaction;
    
    // UI елементи
    Rectangle panelRect;
    Rectangle workerButton;
    
public:
    UnitOrderPanel() : selectedBuildingIndex(-1), buildings(nullptr), playerFaction(ROME) {
        panelRect = {10, 950, 300, 100};
        workerButton = {20, 985, 50, 50};
    }
    
    // Ініціалізація
    void init(std::vector<Building>* b, Faction faction) {
        buildings = b;
        playerFaction = faction;
        selectedBuildingIndex = -1;
    }
    
    // Встановити вибрану будівлю
    void setSelectedBuilding(int index) {
        selectedBuildingIndex = index;
    }
    
    // Малювання панелі
    void draw() const {
        if (!isVisible()) return;
        
        const Building& selected = (*buildings)[selectedBuildingIndex];
        
        // Панель знизу зліва
        DrawRectangleRec(panelRect, {0, 0, 0, 200});
        DrawRectangleLines((int)panelRect.x, (int)panelRect.y, (int)panelRect.width, (int)panelRect.height, WHITE);
        
        // Назва будівлі
        DrawText(selected.name.c_str(), 20, 960, 16, WHITE);
        
        // Малюємо кнопку Worker
        drawWorkerButton();
        
        // Прогрес виробництва
        if (selected.is_producing) {
            float progress = selected.production_progress / selected.production_time;
            DrawRectangle(20, 1040, (int)(260 * progress), 8, GREEN);
            DrawRectangleLines(20, 1040, 260, 8, WHITE);
            DrawText("PRODUCING...", 100, 1042, 12, WHITE);
        }
    }
    
    // Обробка кліків
    void handleClick(Vector2 mousePos) {
        if (!isVisible()) return;
        
        // Перевірка кліку на панелі (щоб не обробляти клік поза панеллю)
        if (!CheckCollisionPointRec(mousePos, panelRect)) return;
        
        const Building& selected = (*buildings)[selectedBuildingIndex];
        
        // Обробка кліку на кнопку Worker
        if (selected.type == HQ_ROME || selected.type == HQ_CARTHAGE) {
            if (CheckCollisionPointRec(mousePos, workerButton)) {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (canAffordWorker()) {
                        bool shouldPayNow = false;
                        if ((*buildings)[selectedBuildingIndex].startProduction("slave", shouldPayNow)) {
                            if (shouldPayNow) {
                                reserveResources(playerFaction, "slave");
                                spendResources(playerFaction, "slave");
                                printf("[UnitOrderPanel] Started producing slave (paid now)\n");
                            } else {
                                reserveResources(playerFaction, "slave");
                                printf("[UnitOrderPanel] Queued slave (reserved resources)\n");
                            }
                        }
                    } else {
                        printf("[UnitOrderPanel] Cannot afford slave\n");
                    }
                } else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
                    if ((*buildings)[selectedBuildingIndex].cancelLastInQueue()) {
                        unreserveResources(playerFaction, "slave");
                        printf("[UnitOrderPanel] Cancelled slave production (unreserved)\n");
                    }
                }
            }
        }
    }
    
    // Чи панель видима
    bool isVisible() const {
        if (selectedBuildingIndex < 0 || !buildings) return false;
        if (selectedBuildingIndex >= buildings->size()) return false;
        
        const Building& selected = (*buildings)[selectedBuildingIndex];
        
        // Панель видима тільки для HQ гравця
        return (selected.type == HQ_ROME || selected.type == HQ_CARTHAGE) && 
               selected.faction == playerFaction;
    }
    
private:
    // Малювання кнопки Worker
    void drawWorkerButton() const {
        const Building& selected = (*buildings)[selectedBuildingIndex];
        
        if (selected.type == HQ_ROME || selected.type == HQ_CARTHAGE) {
            UnitCost cost = getUnitCost("slave");
            bool canAffordUnit = canAffordWorker();
            
            Color buttonColor = canAffordUnit ? BROWN : DARKGRAY;
            DrawRectangleRec(workerButton, buttonColor);
            DrawRectangleLines((int)workerButton.x, (int)workerButton.y, (int)workerButton.width, (int)workerButton.height, WHITE);
            
            // Іконка (тимчасово текст)
            DrawText("SLV", 25, 995, 12, WHITE);
            
            // Черга (число в куточку)
            int queueCount = 0;
            if (selected.is_producing && selected.producing_unit == "slave") queueCount++;
            for (const auto& queued : selected.production_queue) {
                if (queued == "slave") queueCount++;
            }
            if (queueCount > 0) {
                DrawCircle(65, 990, 8, RED);
                DrawText(TextFormat("%d", queueCount), 62, 986, 12, WHITE);
            }
            
            // Вартість
            DrawText(TextFormat("F:%d M:%d", cost.food, cost.money), 80, 990, 12, WHITE);
        }
    }
    
    // Перевірка чи можна дозволити Worker
    bool canAffordWorker() const {
        return canAfford(playerFaction, "slave");
    }
};

#endif // UNIT_ORDER_PANEL_H
