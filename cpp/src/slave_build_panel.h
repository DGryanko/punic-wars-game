#ifndef SLAVE_BUILD_PANEL_H
#define SLAVE_BUILD_PANEL_H

#include "raylib.h"
#include "building.h"
#include "unit.h"
#include "resource.h"
#include "tilemap/coordinates.h"
#include "pathfinding.h"
#include "building_texture_manager.h"
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

extern void moveUnitWithPath(Unit& unit, GridCoords targetPos);
extern void moveUnitWithPathToScreen(Unit& unit, ScreenCoords goalScreen);

extern std::vector<ResourcePoint> resources;

struct BuildingCost { int food; int money; };

inline BuildingCost getBuildingCost(BuildingType type) {
    switch (type) {
        case HQ_ROME:           return {0,  0};
        case HQ_CARTHAGE:       return {0,  0};
        case QUESTORIUM_ROME:   return {20, 30};
        case BARRACKS_ROME:     return {30, 50};
        case BARRACKS_CARTHAGE: return {30, 50};
        case LIBTENT_1:         return {20, 30};
        case LIBTENT_2:         return {30, 50};
        case LIBTENT_3:         return {40, 70};
        case TENTORIUM:         return {25, 40};
        default:                return {0,  0};
    }
}

// -----------------------------------------------------------------------
// Placement mode state (shared with main.cpp via extern)
// -----------------------------------------------------------------------
struct BuildPlacementState {
    bool active = false;
    BuildingType pendingType = HQ_ROME;
    GridCoords   hoverGrid   = {0, 0};
    bool         validSpot   = false;
};

// -----------------------------------------------------------------------
class SlaveBuildPanel {
public:
    // Placement state — read by main.cpp for ghost rendering & RMB confirm
    BuildPlacementState placement;

private:
    int selectedUnitIndex;
    std::vector<Unit>*     units;
    std::vector<Building>* buildings;
    Faction playerFaction;

    // Panel layout
    Rectangle panelRect;

    // Button slots (we'll lay them out dynamically, but keep fixed rects)
    // Slot 0: HQ,  Slot 1: Questorium,  Slot 2: Barracks
    // Slot 3: LibTent1 (Carthage only),  Slot 4: Harvest,  Slot 5: Dropoff
    static const int MAX_BUILD_SLOTS = 4;
    Rectangle buildSlots[MAX_BUILD_SLOTS]; // build buttons
    Rectangle harvestButton;
    Rectangle dropoffButton;

    // Which BuildingType each slot maps to (faction-dependent, set in draw/handle)
    BuildingType slotType[MAX_BUILD_SLOTS];
    int numBuildSlots = 0;

public:
    SlaveBuildPanel()
        : selectedUnitIndex(-1), units(nullptr), buildings(nullptr), playerFaction(ROME)
    {
        panelRect = {10, 930, 500, 120};

        // Fixed positions for up to 4 build slots (50x50 each, 10px gap)
        for (int i = 0; i < MAX_BUILD_SLOTS; i++) {
            buildSlots[i] = {20.0f + i * 62.0f, 970.0f, 52.0f, 52.0f};
        }
        harvestButton = {20.0f + MAX_BUILD_SLOTS * 62.0f,       970.0f, 80.0f, 52.0f};
        dropoffButton = {20.0f + MAX_BUILD_SLOTS * 62.0f + 90.0f, 970.0f, 80.0f, 52.0f};
    }

    void init(std::vector<Unit>* u, std::vector<Building>* b, Faction faction) {
        units = u; buildings = b; playerFaction = faction;
        selectedUnitIndex = -1;
        placement.active = false;
    }

    void setSelectedUnit(int index) {
        selectedUnitIndex = index;
        placement.active = false; // cancel placement on deselect
    }

    bool isVisible() const {
        if (selectedUnitIndex < 0 || !units) return false;
        if (selectedUnitIndex >= (int)units->size()) return false;
        const Unit& u = (*units)[selectedUnitIndex];
        return u.unit_type == "slave" && u.faction == playerFaction;
    }

    // Call every frame with current world-space mouse position (already in camera space)
    void updatePlacement(Vector2 worldMousePos) {
        if (!placement.active) return;
        placement.hoverGrid = CoordinateConverter::screenToGrid(
            ScreenCoords(worldMousePos.x, worldMousePos.y));
        placement.validSpot = isSpotFree(placement.hoverGrid, placement.pendingType);
    }

    // Returns true if RMB was consumed (placement confirmed or cancelled)
    bool handlePlacementInput(Vector2 worldMousePos) {
        if (!placement.active) return false;

        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
            if (placement.validSpot) {
                confirmPlacement(placement.pendingType, placement.hoverGrid);
            }
            placement.active = false;
            return true;
        }
        if (IsKeyPressed(KEY_ESCAPE)) {
            placement.active = false;
            return true;
        }
        return false;
    }

    void draw() const {
        if (!isVisible()) return;

        DrawRectangleRec(panelRect, {0, 0, 0, 200});
        DrawRectangleLines((int)panelRect.x, (int)panelRect.y,
                           (int)panelRect.width, (int)panelRect.height, WHITE);
        DrawText("Slave", 20, 945, 16, WHITE);

        // Build the slot list for current faction
        SlaveBuildPanel* self = const_cast<SlaveBuildPanel*>(this);
        self->buildSlotList();

        for (int i = 0; i < numBuildSlots; i++) {
            drawBuildSlot(i);
        }
        drawHarvestButton();
        drawDropoffButton();

        // Placement mode hint
        if (placement.active) {
            DrawText("RMB to place | ESC to cancel", 20, 910, 14, YELLOW);
        }
    }

    void handleClick(Vector2 mousePos) {
        if (!isVisible()) return;
        if (!CheckCollisionPointRec(mousePos, panelRect)) return;

        buildSlotList();

        for (int i = 0; i < numBuildSlots; i++) {
            if (CheckCollisionPointRec(mousePos, buildSlots[i]) &&
                IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                if (isSlotUnlocked(slotType[i]) && canAffordBuilding(slotType[i])) {
                    // Enter placement mode
                    placement.active = true;
                    placement.pendingType = slotType[i];
                    printf("[SlaveBuildPanel] Placement mode: type=%d\n", slotType[i]);
                } else {
                    printf("[SlaveBuildPanel] Cannot build type=%d (locked or no resources)\n", slotType[i]);
                }
                return;
            }
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

    // Draw ghost building at current hover position (call inside camera mode)
    void drawGhost() const {
        if (!placement.active) return;

        ScreenCoords sc = CoordinateConverter::gridToScreen(placement.hoverGrid);
        Color ghostColor = placement.validSpot
            ? Color{0, 255, 0, 120}
            : Color{255, 0, 0, 120};
        Color outlineColor = placement.validSpot ? GREEN : RED;

        // Спробуємо намалювати реальний спрайт будівлі
        BuildingTextureManager& texMgr = BuildingTextureManager::getInstance();
        bool drewSprite = false;
        if (texMgr.hasTexture(placement.pendingType)) {
            Texture2D tex = texMgr.getTexture(placement.pendingType);
            if (tex.id > 0) {
                // Визначаємо offset як у Building::init()
                Vector2 offset = {-192.0f, -128.0f};
                float drawX = sc.x + offset.x;
                float drawY = sc.y + offset.y;
                // Миготіння: sin хвиля між 0.4 і 1.0
                float pulse = 0.7f + 0.3f * sinf((float)GetTime() * 4.0f);
                Color tint = placement.validSpot
                    ? Color{(unsigned char)(0   * pulse), (unsigned char)(255 * pulse), (unsigned char)(0   * pulse), 200}
                    : Color{(unsigned char)(255 * pulse), (unsigned char)(0   * pulse), (unsigned char)(0   * pulse), 200};
                Rectangle src  = {0, 0, (float)tex.width, (float)tex.height};
                Rectangle dest = {drawX, drawY, (float)tex.width, (float)tex.height};
                DrawTexturePro(tex, src, dest, {0, 0}, 0.0f, tint);
                drewSprite = true;
            }
        }

        // Fallback: ромб якщо спрайту немає
        if (!drewSprite) {
            int hw = CoordinateConverter::TILE_WIDTH_HALF * 2;
            int hh = CoordinateConverter::TILE_HEIGHT_HALF * 2;
            Vector2 top   = {sc.x,      sc.y - (float)hh};
            Vector2 right = {sc.x + (float)hw, sc.y};
            Vector2 bot   = {sc.x,      sc.y + (float)hh};
            Vector2 left  = {sc.x - (float)hw, sc.y};
            DrawTriangle(top, left, bot, ghostColor);
            DrawTriangle(top, bot, right, ghostColor);
            DrawLineV(top, right, outlineColor);
            DrawLineV(right, bot, outlineColor);
            DrawLineV(bot, left, outlineColor);
            DrawLineV(left, top, outlineColor);
        }

        // Назва і ціна
        BuildingCost cost = getBuildingCost(placement.pendingType);
        const char* bname = getBuildingName(placement.pendingType);
        int labelY = (int)sc.y - CoordinateConverter::TILE_HEIGHT_HALF * 2 - 20;
        DrawText(bname, (int)sc.x - 30, labelY, 14, WHITE);
        DrawText(TextFormat("F:%d M:%d", cost.food, cost.money),
                 (int)sc.x - 30, labelY + 16, 12, YELLOW);
    }

private:
    // Build the slot list based on faction
    void buildSlotList() {
        numBuildSlots = 0;
        if (playerFaction == ROME) {
            slotType[numBuildSlots++] = HQ_ROME;
            slotType[numBuildSlots++] = QUESTORIUM_ROME;
            slotType[numBuildSlots++] = BARRACKS_ROME;
            slotType[numBuildSlots++] = TENTORIUM;
        } else {
            slotType[numBuildSlots++] = HQ_CARTHAGE;
            slotType[numBuildSlots++] = QUESTORIUM_ROME; // shared questorium
            slotType[numBuildSlots++] = BARRACKS_CARTHAGE;
            slotType[numBuildSlots++] = LIBTENT_1;
        }
    }

    // Check if a building type is unlocked (requires HQ to exist first)
    bool isSlotUnlocked(BuildingType type) const {
        if (type == HQ_ROME || type == HQ_CARTHAGE) {
            // HQ можна побудувати лише один раз
            return !playerHasHQ();
        }
        if (type == TENTORIUM) {
            // Тенторіум потребує Квесторіуму
            return playerHasHQ() && playerHasBuilding(QUESTORIUM_ROME);
        }
        // Всі інші будівлі потребують HQ
        return playerHasHQ();
    }

    bool playerHasBuilding(BuildingType type) const {
        if (!buildings) return false;
        for (const auto& b : *buildings) {
            if (b.faction == playerFaction && b.type == type) return true;
        }
        return false;
    }

    bool playerHasHQ() const {
        if (!buildings) return false;
        for (const auto& b : *buildings) {
            if (b.faction == playerFaction &&
                (b.type == HQ_ROME || b.type == HQ_CARTHAGE)) return true;
        }
        return false;
    }

    bool canAffordBuilding(BuildingType type) const {
        BuildingCost c = getBuildingCost(type);
        int f = (playerFaction == ROME) ? rome_food  - rome_food_reserved
                                        : carth_food - carth_food_reserved;
        int m = (playerFaction == ROME) ? rome_money  - rome_money_reserved
                                        : carth_money - carth_money_reserved;
        return f >= c.food && m >= c.money;
    }

    void drawBuildSlot(int i) const {
        BuildingType type = slotType[i];
        BuildingCost cost = getBuildingCost(type);
        bool unlocked = isSlotUnlocked(type);
        bool affordable = canAffordBuilding(type);
        bool active = placement.active && placement.pendingType == type;

        Color col;
        if (active)          col = {255, 200, 0, 220};
        else if (!unlocked)  col = {60, 60, 60, 220};
        else if (!affordable) col = {80, 40, 40, 220};
        else                 col = {40, 80, 40, 220};

        DrawRectangleRec(buildSlots[i], col);
        DrawRectangleLines((int)buildSlots[i].x, (int)buildSlots[i].y,
                           (int)buildSlots[i].width, (int)buildSlots[i].height,
                           active ? YELLOW : WHITE);

        const char* label = getBuildingShortName(type);
        DrawText(label, (int)buildSlots[i].x + 4, (int)buildSlots[i].y + 8, 11, WHITE);

        if (!unlocked) {
            DrawText("LOCK", (int)buildSlots[i].x + 4, (int)buildSlots[i].y + 22, 10, RED);
        } else {
            DrawText(TextFormat("F%d", cost.food),  (int)buildSlots[i].x + 2,  (int)buildSlots[i].y + 36, 9, YELLOW);
            DrawText(TextFormat("M%d", cost.money), (int)buildSlots[i].x + 28, (int)buildSlots[i].y + 36, 9, YELLOW);
        }
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

    static const char* getBuildingShortName(BuildingType type) {
        switch (type) {
            case HQ_ROME:           return "HQ";
            case HQ_CARTHAGE:       return "HQ";
            case QUESTORIUM_ROME:   return "QST";
            case BARRACKS_ROME:     return "CTB";
            case BARRACKS_CARTHAGE: return "LBT";
            case LIBTENT_1:         return "LT1";
            case LIBTENT_2:         return "LT2";
            case LIBTENT_3:         return "LT3";
            case TENTORIUM:         return "TNT";
            default:                return "???";
        }
    }

    static const char* getBuildingName(BuildingType type) {
        switch (type) {
            case HQ_ROME:           return "Praetorium";
            case HQ_CARTHAGE:       return "Main Tent";
            case QUESTORIUM_ROME:   return "Questorium";
            case BARRACKS_ROME:     return "Contubernium";
            case BARRACKS_CARTHAGE: return "Libyan Tent";
            case LIBTENT_1:         return "Libyan Tent I";
            case LIBTENT_2:         return "Libyan Tent II";
            case LIBTENT_3:         return "Libyan Tent III";
            case TENTORIUM:         return "Tentorium";
            default:                return "Building";
        }
    }

    // Check if a 2x2 footprint at pos is free
    bool isSpotFree(GridCoords pos, BuildingType /*type*/) const {
        const int FP = 2;
        for (int r = -1; r < FP + 1; r++) {
            for (int c = -1; c < FP + 1; c++) {
                GridCoords t = {pos.row + r, pos.col + c};
                if (t.row < 1 || t.row >= 79 || t.col < 1 || t.col >= 79) return false;
                if (!buildings) continue;
                for (const auto& b : *buildings) {
                    if (b.occupiesGridCell(t)) return false;
                }
            }
        }
        return true;
    }

    void confirmPlacement(BuildingType buildingType, GridCoords pos) {
        if (!canAffordBuilding(buildingType)) {
            printf("[SlaveBuildPanel] Cannot afford building\n");
            return;
        }

        Building newBuilding;
        newBuilding.init(buildingType, playerFaction, pos);
        newBuilding.is_under_construction = true;
        newBuilding.build_timer = 0.0f;
        buildings->push_back(newBuilding);
        pathfindingManager.updateGrid(*buildings);

        BuildingCost cost = getBuildingCost(buildingType);
        if (playerFaction == ROME) { rome_food -= cost.food; rome_money -= cost.money; }
        else                       { carth_food -= cost.food; carth_money -= cost.money; }

        printf("[SlaveBuildPanel] Building type=%d placed at grid(%d,%d)\n",
               buildingType, pos.row, pos.col);

        if (buildingType == QUESTORIUM_ROME) {
            if (playerFaction == ROME) { rome_food_cap = QUESTORIUM_RESOURCE_CAP; rome_money_cap = QUESTORIUM_RESOURCE_CAP; }
            else                       { carth_food_cap = QUESTORIUM_RESOURCE_CAP; carth_money_cap = QUESTORIUM_RESOURCE_CAP; }
        }
    }

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

        GridCoords dropoffPos = findNearestDropoff(slavePos);
        if (dropoffPos.row == -1) {
            printf("[SlaveBuildPanel] No dropoff building found\n");
            return;
        }

        slave.assignResource(nearest->getGridPosition(), dropoffPos);
        printf("[SlaveBuildPanel] Slave sent to harvest at grid(%d,%d)\n",
               nearest->getGridPosition().row, nearest->getGridPosition().col);
    }

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

    GridCoords findNearestDropoff(GridCoords from) const {
        GridCoords best = {-1, -1};
        float bestDist = 999999.0f;
        for (const auto& b : *buildings) {
            if (b.faction != playerFaction) continue;
            if (b.type != HQ_ROME && b.type != HQ_CARTHAGE && b.type != QUESTORIUM_ROME) continue;
            GridCoords bp = b.getGridPosition();
            float dist = sqrt(pow((float)(from.row - bp.row), 2) +
                              pow((float)(from.col - bp.col), 2));
            if (dist < bestDist) { bestDist = dist; best = bp; }
        }
        return best;
    }
};

#endif // SLAVE_BUILD_PANEL_H
