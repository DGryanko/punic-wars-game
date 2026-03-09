#ifndef SLAVE_BUILD_PANEL_H
#define SLAVE_BUILD_PANEL_H

#include "raylib.h"
#include "building.h"
#include "unit.h"
#include "tilemap/coordinates.h"
#include "pathfinding.h"
#include <vector>
#include <string>

// Зовнішні функції для роботи з ресурсами
extern bool canAfford(Faction faction, const std::string& unitType);
extern void spendResources(Faction faction, const std::string& unitType);

// Зовнішні змінні ресурсів
extern int rome_food, rome_money, rome_food_reserved, rome_money_reserved;
extern int carth_food, carth_money, carth_food_reserved, carth_money_reserved;

// Зовнішні системи
class PathfindingManager;
extern PathfindingManager pathfindingManager;

// Вартість будівель
struct BuildingCost {
    int food;
    int money;
};

inline BuildingCost getBuildingCost(BuildingType type) {
    if (type == QUESTORIUM_ROME) {
        return {20, 30};  // Зменшено: було 50, 100
    } else if (type == BARRACKS_ROME) {
        return {30, 50};  // Контуберній
    }
    return {0, 0};
}

// UI панель для будівництва рабом
class SlaveBuildPanel {
private:
    int selectedUnitIndex;
    std::vector<Unit>* units;
    std::vector<Building>* buildings;
    Faction playerFaction;
    
    // UI елементи
    Rectangle panelRect;
    Rectangle questoriumButton;
    Rectangle barracksButton;
    
public:
    SlaveBuildPanel() : selectedUnitIndex(-1), units(nullptr), buildings(nullptr), playerFaction(ROME) {
        panelRect = {10, 950, 300, 100};
        questoriumButton = {20, 985, 50, 50};
        barracksButton = {80, 985, 50, 50};  // Друга кнопка праворуч
    }
    
    // Ініціалізація
    void init(std::vector<Unit>* u, std::vector<Building>* b, Faction faction) {
        units = u;
        buildings = b;
        playerFaction = faction;
        selectedUnitIndex = -1;
    }
    
    // Встановити вибраного раба
    void setSelectedUnit(int index) {
        selectedUnitIndex = index;
    }
    
    // Малювання панелі
    void draw() const {
        if (!isVisible()) return;
        
        const Unit& selected = (*units)[selectedUnitIndex];
        
        // Панель знизу зліва
        DrawRectangleRec(panelRect, {0, 0, 0, 200});
        DrawRectangleLines((int)panelRect.x, (int)panelRect.y, (int)panelRect.width, (int)panelRect.height, WHITE);
        
        // Назва юніта
        DrawText("Slave - Build", 20, 960, 16, WHITE);
        
        // Малюємо кнопки
        drawQuestoriumButton();
        drawBarracksButton();
    }
    
    // Обробка кліків
    void handleClick(Vector2 mousePos) {
        if (!isVisible()) return;
        
        // Перевірка кліку на панелі
        if (!CheckCollisionPointRec(mousePos, panelRect)) return;
        
        Unit& selected = (*units)[selectedUnitIndex];
        
        // Обробка кліку на кнопку Questorium
        if (CheckCollisionPointRec(mousePos, questoriumButton)) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (canAffordQuestorium()) {
                    spawnBuilding(selected, QUESTORIUM_ROME);
                } else {
                    printf("[SlaveBuildPanel] Cannot afford Questorium\n");
                }
            }
        }
        
        // Обробка кліку на кнопку Barracks
        if (CheckCollisionPointRec(mousePos, barracksButton)) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (canAffordBarracks()) {
                    spawnBuilding(selected, BARRACKS_ROME);
                } else {
                    printf("[SlaveBuildPanel] Cannot afford Barracks\n");
                }
            }
        }
    }
    
    // Чи панель видима
    bool isVisible() const {
        if (selectedUnitIndex < 0 || !units) return false;
        if (selectedUnitIndex >= units->size()) return false;
        
        const Unit& selected = (*units)[selectedUnitIndex];
        
        // Панель видима тільки для рабів гравця
        return selected.unit_type == "slave" && selected.faction == playerFaction;
    }
    
private:
    // Малювання кнопки Questorium
    void drawQuestoriumButton() const {
        BuildingCost cost = getBuildingCost(QUESTORIUM_ROME);
        bool canAffordBuilding = canAffordQuestorium();
        
        Color buttonColor = canAffordBuilding ? DARKGREEN : DARKGRAY;
        DrawRectangleRec(questoriumButton, buttonColor);
        DrawRectangleLines((int)questoriumButton.x, (int)questoriumButton.y, (int)questoriumButton.width, (int)questoriumButton.height, WHITE);
        
        // Іконка (тимчасово текст)
        DrawText("QST", 25, 995, 12, WHITE);
        
        // Вартість
        DrawText(TextFormat("F:%d M:%d", cost.food, cost.money), 80, 1000, 10, WHITE);
    }
    
    // Малювання кнопки Barracks
    void drawBarracksButton() const {
        BuildingCost cost = getBuildingCost(BARRACKS_ROME);
        bool canAffordBuilding = canAffordBarracks();
        
        Color buttonColor = canAffordBuilding ? DARKBLUE : DARKGRAY;
        DrawRectangleRec(barracksButton, buttonColor);
        DrawRectangleLines((int)barracksButton.x, (int)barracksButton.y, (int)barracksButton.width, (int)barracksButton.height, WHITE);
        
        // Іконка (тимчасово текст)
        DrawText("CTB", 85, 995, 12, WHITE);
        
        // Вартість
        DrawText(TextFormat("F:%d M:%d", cost.food, cost.money), 140, 1000, 10, WHITE);
    }
    
    // Перевірка чи можна дозволити Questorium
    bool canAffordQuestorium() const {
        BuildingCost cost = getBuildingCost(QUESTORIUM_ROME);
        
        if (playerFaction == ROME) {
            int available_food = rome_food - rome_food_reserved;
            int available_money = rome_money - rome_money_reserved;
            return available_food >= cost.food && available_money >= cost.money;
        } else {
            int available_food = carth_food - carth_food_reserved;
            int available_money = carth_money - carth_money_reserved;
            return available_food >= cost.food && available_money >= cost.money;
        }
    }
    
    // Перевірка чи можна дозволити Barracks
    bool canAffordBarracks() const {
        BuildingCost cost = getBuildingCost(BARRACKS_ROME);
        
        if (playerFaction == ROME) {
            int available_food = rome_food - rome_food_reserved;
            int available_money = rome_money - rome_money_reserved;
            return available_food >= cost.food && available_money >= cost.money;
        } else {
            int available_food = carth_food - carth_food_reserved;
            int available_money = carth_money - carth_money_reserved;
            return available_food >= cost.food && available_money >= cost.money;
        }
    }
    
    // Спавн будівлі (універсальний метод)
    void spawnBuilding(const Unit& slave, BuildingType buildingType) const {
        // Отримуємо позицію раба
        GridCoords slavePos = slave.getGridPosition();
        
        // Шукаємо вільне місце на відстані 2-3 тайли
        GridCoords buildPos = findFreeBuildSpot(slavePos);
        
        if (buildPos.row == -1) {
            printf("[SlaveBuildPanel] No free spot found for building\n");
            return;
        }
        
        // Створюємо будівлю
        Building newBuilding;
        newBuilding.init(buildingType, playerFaction, buildPos);
        buildings->push_back(newBuilding);
        
        // ВАЖЛИВО: Оновлюємо pathfinding grid після додавання будівлі
        pathfindingManager.updateGrid(*buildings);
        printf("[SlaveBuildPanel] Pathfinding grid updated after building placement\n");
        
        // Витрачаємо ресурси
        BuildingCost cost = getBuildingCost(buildingType);
        
        if (playerFaction == ROME) {
            rome_food -= cost.food;
            rome_money -= cost.money;
        } else {
            carth_food -= cost.food;
            carth_money -= cost.money;
        }
        
        const char* buildingName = (buildingType == QUESTORIUM_ROME) ? "Questorium" : "Barracks";
        printf("[SlaveBuildPanel] %s built at grid(%d, %d)\n", buildingName, buildPos.row, buildPos.col);
    }
    
    // Знайти вільне місце для будівництва
    GridCoords findFreeBuildSpot(GridCoords center) const {
        // Перевіряємо позиції на відстані 2-3 тайли від центру
        for (int radius = 2; radius <= 3; radius++) {
            for (int dr = -radius; dr <= radius; dr++) {
                for (int dc = -radius; dc <= radius; dc++) {
                    // Перевіряємо що відстань саме radius (не менше)
                    if (abs(dr) + abs(dc) < radius * 2) continue;
                    
                    GridCoords checkPos = {center.row + dr, center.col + dc};
                    
                    // Перевіряємо що позиція в межах карти (80x80)
                    if (checkPos.row < 0 || checkPos.row >= 80 || 
                        checkPos.col < 0 || checkPos.col >= 80) {
                        continue;
                    }
                    
                    // Перевіряємо що місце вільне (2x2 тайли для будівлі + 1 тайл відступу)
                    bool isFree = true;
                    // Перевіряємо 3x3 область (2x2 будівля + 1 тайл відступу з кожного боку)
                    for (int r = -1; r < 3 && isFree; r++) {
                        for (int c = -1; c < 3 && isFree; c++) {
                            GridCoords tilePos = {checkPos.row + r, checkPos.col + c};
                            
                            // Перевіряємо межі
                            if (tilePos.row < 0 || tilePos.row >= 80 || 
                                tilePos.col < 0 || tilePos.col >= 80) {
                                isFree = false;
                                break;
                            }
                            
                            // Перевіряємо чи не зайнято іншою будівлею
                            for (const auto& building : *buildings) {
                                if (building.occupiesGridCell(tilePos)) {
                                    isFree = false;
                                    break;
                                }
                            }
                        }
                    }
                    
                    if (isFree) {
                        return checkPos;
                    }
                }
            }
        }
        
        return {-1, -1}; // Не знайшли вільного місця
    }
};

#endif // SLAVE_BUILD_PANEL_H
