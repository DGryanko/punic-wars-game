#ifndef UNIT_ORDER_PANEL_H
#define UNIT_ORDER_PANEL_H

#include "raylib.h"
#include "building.h"
#include <vector>
#include <string>

extern bool canAfford(Faction faction, const std::string& unitType);
extern void reserveResources(Faction faction, const std::string& unitType);
extern void spendResources(Faction faction, const std::string& unitType);
extern void unreserveResources(Faction faction, const std::string& unitType);

struct UnitCost {
    int food;
    int money;
};

inline UnitCost getUnitCost(const std::string& unitType) {
    if (unitType == "legionary")  return {30, 50};
    if (unitType == "phoenician") return {25, 60};
    if (unitType == "slave")      return {10, 20};
    return {0, 0};
}

class UnitOrderPanel {
private:
    int selectedBuildingIndex;
    std::vector<Building>* buildings;
    Faction playerFaction;

    Rectangle panelRect;
    Rectangle btn1;   // slave (HQ) або legionary/phoenician (barracks)
    Rectangle btn2;   // slave (barracks carthage)

    // Повертає тип юніта для кнопки залежно від будівлі
    std::string getBtn1Unit() const {
        const Building& b = (*buildings)[selectedBuildingIndex];
        if (b.type == HQ_ROME || b.type == HQ_CARTHAGE) return "slave";
        if (b.type == BARRACKS_ROME)     return "legionary";
        if (b.type == BARRACKS_CARTHAGE) return "phoenician";
        return "";
    }

public:
    UnitOrderPanel() : selectedBuildingIndex(-1), buildings(nullptr), playerFaction(ROME) {
        panelRect = {10, 950, 300, 100};
        btn1 = {20, 985, 50, 50};
        btn2 = {80, 985, 50, 50};
    }

    void init(std::vector<Building>* b, Faction faction) {
        buildings = b;
        playerFaction = faction;
        selectedBuildingIndex = -1;
    }

    void setSelectedBuilding(int index) { selectedBuildingIndex = index; }

    bool isVisible() const {
        if (selectedBuildingIndex < 0 || !buildings) return false;
        if (selectedBuildingIndex >= (int)buildings->size()) return false;
        const Building& b = (*buildings)[selectedBuildingIndex];
        if (b.faction != playerFaction) return false;
        return b.type == HQ_ROME || b.type == HQ_CARTHAGE ||
               b.type == BARRACKS_ROME || b.type == BARRACKS_CARTHAGE;
    }

    void draw() const {
        if (!isVisible()) return;
        const Building& b = (*buildings)[selectedBuildingIndex];

        DrawRectangleRec(panelRect, {0, 0, 0, 200});
        DrawRectangleLines((int)panelRect.x, (int)panelRect.y,
                           (int)panelRect.width, (int)panelRect.height, WHITE);
        DrawText(b.name.c_str(), 20, 960, 16, WHITE);

        drawUnitButton(btn1, getBtn1Unit());

        // Прогрес виробництва
        if (b.is_producing) {
            float progress = b.production_progress / b.production_time;
            DrawRectangle(20, 1040, (int)(260 * progress), 8, GREEN);
            DrawRectangleLines(20, 1040, 260, 8, WHITE);
            DrawText("PRODUCING...", 100, 1042, 12, WHITE);
        }
    }

    void handleClick(Vector2 mousePos) {
        if (!isVisible()) return;
        if (!CheckCollisionPointRec(mousePos, panelRect)) return;

        std::string unit1 = getBtn1Unit();
        if (!unit1.empty() && CheckCollisionPointRec(mousePos, btn1)) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (canAfford(playerFaction, unit1)) {
                    bool shouldPayNow = false;
                    if ((*buildings)[selectedBuildingIndex].startProduction(unit1, shouldPayNow)) {
                        reserveResources(playerFaction, unit1);
                        if (shouldPayNow) spendResources(playerFaction, unit1);
                    }
                }
            } else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
                if ((*buildings)[selectedBuildingIndex].cancelLastInQueue()) {
                    unreserveResources(playerFaction, unit1);
                }
            }
        }
    }

private:
    void drawUnitButton(Rectangle rect, const std::string& unitType) const {
        if (unitType.empty()) return;
        const Building& b = (*buildings)[selectedBuildingIndex];
        UnitCost cost = getUnitCost(unitType);
        bool affordable = canAfford(playerFaction, unitType);

        const char* label = "???";
        Color col = DARKGRAY;
        if (unitType == "slave")      { label = "SLV"; col = affordable ? BROWN    : DARKGRAY; }
        if (unitType == "legionary")  { label = "LEG"; col = affordable ? Color{139,0,0,255} : DARKGRAY; }
        if (unitType == "phoenician") { label = "PHO"; col = affordable ? DARKBLUE : DARKGRAY; }

        DrawRectangleRec(rect, col);
        DrawRectangleLines((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height, WHITE);
        DrawText(label, (int)rect.x + 5, (int)rect.y + 10, 12, WHITE);

        // Черга
        int queueCount = 0;
        if (b.is_producing && b.producing_unit == unitType) queueCount++;
        for (const auto& q : b.production_queue) if (q == unitType) queueCount++;
        if (queueCount > 0) {
            DrawCircle((int)rect.x + (int)rect.width - 8, (int)rect.y + 8, 8, RED);
            DrawText(TextFormat("%d", queueCount), (int)rect.x + (int)rect.width - 12, (int)rect.y + 3, 12, WHITE);
        }

        DrawText(TextFormat("F:%d", cost.food),  (int)rect.x, (int)rect.y + (int)rect.height + 2,  9, YELLOW);
        DrawText(TextFormat("M:%d", cost.money), (int)rect.x, (int)rect.y + (int)rect.height + 13, 9, YELLOW);
    }
};

#endif // UNIT_ORDER_PANEL_H
