#ifndef SLAVE_BUILD_PANEL_H
#define SLAVE_BUILD_PANEL_H

#include "raylib.h"
#include "building.h"
#include "unit.h"
#include "resource.h"
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

// Оголошення moveUnitWithPath та moveUnitWithPathToScreen з main.cpp
extern void moveUnitWithPath(Unit& unit, GridCoords targetPos);
extern void moveUnitWithPathToScreen(Unit& unit, ScreenCoords goalScreen);

// Зовнішній список ресурсів
extern std::vector<ResourcePoint> resources;

struct BuildingCost { int food; int money; };

inline BuildingCost getBuildingCost(BuildingType type) {
    if (type == QUESTORIUM_ROME)    return {20, 30};
    if (type == BARRACKS_ROME)      return {30, 50};
    if (type == BARRACKS_CARTHAGE)  return {30, 50};
    return {0, 0};
}

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
    Rectangle storageButton;
    Rectangle barracksButton;
    Rectangle harvestButton;   // Зібрати ресурс
    Rectangle dropoffButton;   // Скинути ресурс

public:
    SlaveBuildPanel() : selectedUnitIndex(-1), units(nullptr), buildings(nullptr), playerFaction(ROME) {
        panelRect      = {10, 940, 380, 110};
        storageButton  = {20,  975,  50,  50};
        barracksButton = {80,  975,  50,  50};
        harvestButton  = {160, 975,  90,  50};
        dropoffButton  = {260, 975,  90,  50};
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
        DrawText("Slave", 20, 950, 16, WHITE);
        drawStorageButton();
        drawBarracksButton();
        drawHarvestButton();
        drawDropoffButton();
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

        if (CheckCollisionPointRec(mousePos, harvestButton) &&
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            cmdHarvest();
        }

        if (CheckCollisionPointRec(mousePos, dropoffButton) &&
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            cmdDropoff();
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

    void drawHarvestButton() const {
        DrawRectangleRec(harvestButton, {0, 100, 0, 220});
        DrawRectangleLines((int)harvestButton.x, (int)harvestButton.y,
                           (int)harvestButton.width, (int)harvestButton.height, WHITE);
        DrawText("Harvest", (int)harvestButton.x + 5, (int)harvestButton.y + 10, 12, WHITE);
        DrawText("(3 tiles)", (int)harvestButton.x + 5, (int)harvestButton.y + 28, 10, LIGHTGRAY);
    }

    void drawDropoffButton() const {
        DrawRectangleRec(dropoffButton, {100, 60, 0, 220});
        DrawRectangleLines((int)dropoffButton.x, (int)dropoffButton.y,
                           (int)dropoffButton.width, (int)dropoffButton.height, WHITE);
        DrawText("Drop off", (int)dropoffButton.x + 5, (int)dropoffButton.y + 10, 12, WHITE);
        DrawText("(nearest)", (int)dropoffButton.x + 5, (int)dropoffButton.y + 28, 10, LIGHTGRAY);
    }

    // Знайти найближчий ресурс в радіусі 3 тайлів від раба
    void cmdHarvest() const {
        if (selectedUnitIndex < 0 || !units) return;
        Unit& slave = (*units)[selectedUnitIndex];
        GridCoords slavePos = slave.getGridPosition();

        const int HARVEST_RADIUS = 3;
        ResourcePoint* nearest = nullptr;
        float nearestDist = 999999.0f;

        for (auto& res : resources) {
            if (res.depleted) continue;
            GridCoords rp = res.getGridPosition();
            float dist = sqrt(pow((float)(slavePos.row - rp.row), 2) +
                              pow((float)(slavePos.col - rp.col), 2));
            if (dist <= HARVEST_RADIUS && dist < nearestDist) {
                nearestDist = dist;
                nearest = &res;
            }
        }

        if (!nearest) {
            printf("[SlaveBuildPanel] No resource within %d tiles\n", HARVEST_RADIUS);
            return;
        }

        // Знаходимо найближчу будівлю здачі
        GridCoords dropoffPos = findNearestDropoff(slavePos);
        if (dropoffPos.row == -1) {
            printf("[SlaveBuildPanel] No dropoff building found\n");
            return;
        }

        slave.assignResource(nearest->getGridPosition(), dropoffPos);
        printf("[SlaveBuildPanel] Slave sent to harvest at grid(%d,%d)\n",
               nearest->getGridPosition().row, nearest->getGridPosition().col);
    }

    // Відправити раба до найближчої точки здачі
    void cmdDropoff() const {
        if (selectedUnitIndex < 0 || !units) return;
        Unit& slave = (*units)[selectedUnitIndex];
        GridCoords slavePos = slave.getGridPosition();

        GridCoords dropoffPos = findNearestDropoff(slavePos);
        if (dropoffPos.row == -1) {
            printf("[SlaveBuildPanel] No dropoff building found\n");
            return;
        }

        moveUnitWithPath(slave, dropoffPos);
        printf("[SlaveBuildPanel] Slave sent to dropoff at grid(%d,%d)\n",
               dropoffPos.row, dropoffPos.col);
    }

    // Знайти найближчу будівлю здачі ресурсів (HQ або Questorium)
    GridCoords findNearestDropoff(GridCoords from) const {
        GridCoords best = {-1, -1};
        float bestDist = 999999.0f;
        for (const auto& b : *buildings) {
            if (b.faction != playerFaction) continue;
            if (b.type != HQ_ROME && b.type != HQ_CARTHAGE && b.type != QUESTORIUM_ROME) continue;
            GridCoords bp = b.getGridPosition();
            float dist = sqrt(pow((float)(from.row - bp.row), 2) +
                              pow((float)(from.col - bp.col), 2));
            if (dist < bestDist) {
                bestDist = dist;
                best = bp;
            }
        }
        return best;
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

// (class SlaveBuildPanel defined above)
