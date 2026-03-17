#ifndef SLAVE_BUILD_PANEL_H
#define SLAVE_BUILD_PANEL_H

#include "raylib.h"
#include "building.h"
#include "unit.h"
#include "tilemap/coordinates.h"
#include "pathfinding.h"
#include <vector>
#include <string>

extern bool canAfford(Faction faction, const std::string& unitType);
extern void spendResources(Faction faction, const std::string& unitType);

extern int rome_food, rome_money, rome_food_reserved, rome_money_reserved;
extern int carth_food, carth_money, carth_food_reserved, carth_money_reserved;
extern int rome_food_cap, rome_money_cap, carth_food_cap, carth_money_cap;
extern const int QUESTORIUM_RESOURCE_CAP;

class PathfindingManager;
extern PathfindingManager pathfindingManager;

struct BuildingCost { int food; int money; };

inline BuildingCost getBuildingCost(BuildingType type) {
    if (type == QUESTORIUM_ROME)    return {20, 30};
    if (type == BARRACKS_ROME)      return {30, 50};  // Контуберній (Рим)
    if (type == BARRACKS_CARTHAGE)  return {30, 50};  // Лівійська палатка (Карфаген)
    return {0, 0};
}

// Повертає тип казарми для фракції
inline BuildingType getBarracksType(Faction f) {
    return (f == ROME) ? BARRACKS_ROME : BARRACKS_CARTHAGE;
}

class SlaveBuildPanel {
private:
    int selectedUnitIndex;
    std::vector<Unit>* units;
    std::vector<Building>* buildings;
    Faction playerFaction;

    Rectangle panelRect;
    Rectangle storageButton;   // Квесторій / склад
    Rectangle barracksButton;  // Казарма

public:
    SlaveBuildPanel() : selectedUnitIndex(-1), units(nullptr), buildings(nullptr), playerFaction(ROME) {
        panelRect      = {10, 950, 300, 100};
        storageButton  = {20,  985,  50,  50};
        barracksButton = {80,  985,  50,  50};
    }

    void init(std::vector<Unit>* u, std::vector<Building>* b, Faction faction) {
        units = u; buildings = b; playerFaction = faction;
        selectedUnitIndex = -1;
    }

    void setSelectedUnit(int index) { selectedUnitIndex = index; }

    bool isVisible() const {
        if (selectedUnitIndex < 0 || !units) return false;
        if (selectedUnitIndex >= (int)units->size()) return false;
        const Unit& u = (*units)[selectedUnitIndex];
        return u.unit_type == "slave" && u.faction == playerFaction;
    }

    void draw() const {
        if (!isVisible()) return;
        DrawRectangleRec(panelRect, {0, 0, 0, 200});
        DrawRectangleLines((int)panelRect.x, (int)panelRect.y,
                           (int)panelRect.width, (int)panelRect.height, WHITE);
        DrawText("Slave - Build", 20, 960, 16, WHITE);
        drawStorageButton();
        drawBarracksButton();
    }

    void handleClick(Vector2 mousePos) {
        if (!isVisible()) return;
        if (!CheckCollisionPointRec(mousePos, panelRect)) return;

        if (CheckCollisionPointRec(mousePos, storageButton) &&
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (canAffordStorage()) spawnBuilding(QUESTORIUM_ROME);
            else printf("[SlaveBuildPanel] Cannot afford storage\n");
        }

        if (CheckCollisionPointRec(mousePos, barracksButton) &&
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            BuildingType bt = getBarracksType(playerFaction);
            if (canAffordBarracks()) spawnBuilding(bt);
            else printf("[SlaveBuildPanel] Cannot afford barracks\n");
        }
    }

private:
    bool canAffordStorage() const {
        BuildingCost c = getBuildingCost(QUESTORIUM_ROME);
        int f = (playerFaction == ROME) ? rome_food - rome_food_reserved : carth_food - carth_food_reserved;
        int m = (playerFaction == ROME) ? rome_money - rome_money_reserved : carth_money - carth_money_reserved;
        return f >= c.food && m >= c.money;
    }

    bool canAffordBarracks() const {
        BuildingCost c = getBuildingCost(getBarracksType(playerFaction));
        int f = (playerFaction == ROME) ? rome_food - rome_food_reserved : carth_food - carth_food_reserved;
        int m = (playerFaction == ROME) ? rome_money - rome_money_reserved : carth_money - carth_money_reserved;
        return f >= c.food && m >= c.money;
    }

    void drawStorageButton() const {
        BuildingCost c = getBuildingCost(QUESTORIUM_ROME);
        Color col = canAffordStorage() ? DARKGREEN : DARKGRAY;
        DrawRectangleRec(storageButton, col);
        DrawRectangleLines((int)storageButton.x, (int)storageButton.y,
                           (int)storageButton.width, (int)storageButton.height, WHITE);
        DrawText("QST", (int)storageButton.x + 5, (int)storageButton.y + 10, 12, WHITE);
        DrawText(TextFormat("F:%d", c.food),  (int)storageButton.x, (int)storageButton.y + 52, 9, YELLOW);
        DrawText(TextFormat("M:%d", c.money), (int)storageButton.x, (int)storageButton.y + 63, 9, YELLOW);
    }

    void drawBarracksButton() const {
        BuildingType bt = getBarracksType(playerFaction);
        BuildingCost c = getBuildingCost(bt);
        const char* label = (playerFaction == ROME) ? "CTB" : "LBT";
        Color col = canAffordBarracks() ? DARKBLUE : DARKGRAY;
        DrawRectangleRec(barracksButton, col);
        DrawRectangleLines((int)barracksButton.x, (int)barracksButton.y,
                           (int)barracksButton.width, (int)barracksButton.height, WHITE);
        DrawText(label, (int)barracksButton.x + 5, (int)barracksButton.y + 10, 12, WHITE);
        DrawText(TextFormat("F:%d", c.food),  (int)barracksButton.x, (int)barracksButton.y + 52, 9, YELLOW);
        DrawText(TextFormat("M:%d", c.money), (int)barracksButton.x, (int)barracksButton.y + 63, 9, YELLOW);
    }

    void spawnBuilding(BuildingType buildingType) const {
        const Unit& slave = (*units)[selectedUnitIndex];
        GridCoords buildPos = findFreeBuildSpot(slave.getGridPosition());
        if (buildPos.row == -1) {
            printf("[SlaveBuildPanel] No free spot found\n");
            return;
        }

        Building newBuilding;
        newBuilding.init(buildingType, playerFaction, buildPos);
        buildings->push_back(newBuilding);
        pathfindingManager.updateGrid(*buildings);

        BuildingCost cost = getBuildingCost(buildingType);
        if (playerFaction == ROME) { rome_food -= cost.food; rome_money -= cost.money; }
        else                       { carth_food -= cost.food; carth_money -= cost.money; }

        printf("[SlaveBuildPanel] Building type=%d built at grid(%d,%d)\n",
               buildingType, buildPos.row, buildPos.col);

        if (buildingType == QUESTORIUM_ROME) {
            if (playerFaction == ROME) { rome_food_cap = QUESTORIUM_RESOURCE_CAP; rome_money_cap = QUESTORIUM_RESOURCE_CAP; }
            else                       { carth_food_cap = QUESTORIUM_RESOURCE_CAP; carth_money_cap = QUESTORIUM_RESOURCE_CAP; }
        }
    }

    GridCoords findFreeBuildSpot(GridCoords center) const {
        for (int radius = 2; radius <= 5; radius++) {
            for (int dr = -radius; dr <= radius; dr++) {
                for (int dc = -radius; dc <= radius; dc++) {
                    if (abs(dr) + abs(dc) < radius * 2) continue;
                    GridCoords pos = {center.row + dr, center.col + dc};
                    if (pos.row < 1 || pos.row >= 78 || pos.col < 1 || pos.col >= 78) continue;

                    bool isFree = true;
                    for (int r = -1; r < 3 && isFree; r++) {
                        for (int c = -1; c < 3 && isFree; c++) {
                            GridCoords t = {pos.row + r, pos.col + c};
                            if (t.row < 0 || t.row >= 80 || t.col < 0 || t.col >= 80) { isFree = false; break; }
                            for (const auto& b : *buildings) {
                                if (b.occupiesGridCell(t)) { isFree = false; break; }
                            }
                        }
                    }
                    if (isFree) return pos;
                }
            }
        }
        return {-1, -1};
    }
};

#endif // SLAVE_BUILD_PANEL_H
