#pragma once
#include "unit.h"
#include "tilemap/coordinates.h"
#include <vector>
#include <cmath>

// ─── Sub-tile slot offsets ────────────────────────────────────────────────────
// Тайл — ізометричний ромб 64x32 пікселів.
// Ізометричні осі:
//   вісь "right" (col+1): screen (+32, +16)
//   вісь "down"  (row+1): screen (-32, +16)
//
// Ділимо тайл на 3x3 = 9 слотів вздовж ізометричних осей.
// Крок по осі right: (32/3, 16/3) ≈ (10.7, 5.3)
// Крок по осі down:  (-32/3, 16/3) ≈ (-10.7, 5.3)
//
// Слоти (ізометрична сітка, r=рядок вниз, c=колонка вправо):
//   (r,c): r∈{-1,0,1}, c∈{-1,0,1}
//   screen_x = c*10.7 + r*(-10.7)
//   screen_y = c*5.3  + r*5.3
//
// Нумерація 0-8: рядок*3+колонка (r+1)*3+(c+1)
//   0=(r-1,c-1)  1=(r-1,c0)  2=(r-1,c+1)
//   3=(r0, c-1)  4=(r0, c0)  5=(r0, c+1)
//   6=(r+1,c-1)  7=(r+1,c0)  8=(r+1,c+1)

static const float ISO_STEP = 32.0f / 3.0f; // ~10.67

// Ізометричний offset для слоту (ri, ci) де ri,ci ∈ {-1,0,1}
inline ScreenCoords isoSlotOffset(int ri, int ci) {
    return {
        (ci - ri) * ISO_STEP,
        (ci + ri) * (ISO_STEP * 0.5f)
    };
}

static const ScreenCoords SLOT_OFFSETS[9] = {
    isoSlotOffset(-1,-1), isoSlotOffset(-1, 0), isoSlotOffset(-1, 1),
    isoSlotOffset( 0,-1), isoSlotOffset( 0, 0), isoSlotOffset( 0, 1),
    isoSlotOffset( 1,-1), isoSlotOffset( 1, 0), isoSlotOffset( 1, 1),
};

// Порядок заповнення: центр → навколо
static const int SLOT_FILL_ORDER[9] = { 4, 3, 5, 1, 7, 0, 2, 6, 8 };

// ─── Slot management ─────────────────────────────────────────────────────────

inline int findFreeSlot(const std::vector<Unit>& units, GridCoords tile, int excludeIdx = -1) {
    bool used[9] = {};
    for (int i = 0; i < (int)units.size(); i++) {
        if (i == excludeIdx) continue;
        if (units[i].position == tile && units[i].slot_index >= 0 && units[i].slot_index < 9)
            used[units[i].slot_index] = true;
    }
    for (int s : SLOT_FILL_ORDER)
        if (!used[s]) return s;
    return -1;
}

inline void assignSlot(Unit& unit, const std::vector<Unit>& units, int unitIdx) {
    int slot = findFreeSlot(units, unit.position, unitIdx);
    if (slot < 0) slot = 4;
    unit.slot_index = slot;
    unit.slot_offset = SLOT_OFFSETS[slot];
}

inline void releaseSlot(Unit& unit) {
    unit.slot_index = -1;
    unit.slot_offset = {0, 0};
}

// ─── Formation targets ────────────────────────────────────────────────────────
// Ізометричні кроки між юнітами у формації (1 тайл)
// right = (+64, +32) → між сусідніми юнітами в шерензі беремо 1 тайл
static const float F_STEP_X = 64.0f; // 1 тайл по ізометричній осі X (screen)
static const float F_STEP_Y = 32.0f; // 1 тайл по ізометричній осі Y (screen)

inline std::vector<ScreenCoords> buildFormationTargets(
    ScreenCoords center,
    Unit::FormationType formation,
    int count,
    const std::string& unitType)
{
    std::vector<ScreenCoords> targets;
    targets.reserve(count);

    // Ізометричні одиничні вектори:
    //   "вправо по шерензі" (col+1): (+F_STEP_X/2, +F_STEP_Y/2) = (+32, +16)
    //   "назад (наступний ряд)" (row+1): (-F_STEP_X/2, +F_STEP_Y/2) = (-32, +16)
    const float RX =  F_STEP_X * 0.5f; // +32
    const float RY =  F_STEP_Y * 0.5f; // +16
    const float BX = -F_STEP_X * 0.5f; // -32 (назад-ліво)
    const float BY =  F_STEP_Y * 0.5f; // +16

    bool isLine = (formation == Unit::FORMATION_LINE)
               || (formation == Unit::FORMATION_NONE
                   && (unitType == "legionary" || unitType == "phoenician")
                   && count > 1);

    if (isLine) {
        // Шеренга: до 10 в ряд, наступний ряд позаду
        const int ROW_SIZE = 10;
        int rowCount = std::min(count, ROW_SIZE);
        for (int i = 0; i < count; i++) {
            int c = i % ROW_SIZE;
            int r = i / ROW_SIZE;
            // Центруємо ряд
            float halfRow = (std::min(count - r * ROW_SIZE, ROW_SIZE) - 1) * 0.5f;
            float cx = (c - halfRow) * RX * 2 + r * BX * 2;
            float cy = (c - halfRow) * RY * 2 + r * BY * 2;
            targets.push_back({ center.x + cx, center.y + cy });
        }
    } else if (formation == Unit::FORMATION_SQUARE) {
        int side = (int)ceil(sqrt((double)count));
        for (int i = 0; i < count; i++) {
            int c = i % side;
            int r = i / side;
            float cx = (c - (side-1)*0.5f) * RX * 2 + r * BX * 2;
            float cy = (c - (side-1)*0.5f) * RY * 2 + r * BY * 2;
            targets.push_back({ center.x + cx, center.y + cy });
        }
    } else if (formation == Unit::FORMATION_CHESS) {
        const int ROW_SIZE = 5;
        for (int i = 0; i < count; i++) {
            int c = i % ROW_SIZE;
            int r = i / ROW_SIZE;
            float shift = (r % 2 == 1) ? RX : 0.0f;
            float cx = (c - (ROW_SIZE-1)*0.5f) * RX * 2 + shift + r * BX * 2;
            float cy = (c - (ROW_SIZE-1)*0.5f) * RY * 2 + r * BY * 2;
            targets.push_back({ center.x + cx, center.y + cy });
        }
    } else {
        // FORMATION_NONE для не-бойових або одиночних: розкидаємо по тайлах навколо
        // Спіраль тайлів: (dr, dc) в ізометричних grid coords
        const int DR[] = { 0, 1, 0,-1, 1,-1, 1,-1, 2, 0,-2, 0, 2,-2, 2,-2};
        const int DC[] = { 0, 0, 1, 0,-1, 1,-1,-1, 0, 2, 0,-2, 2,-2,-2, 2};
        const int MAX_TILES = 16;
        int placed = 0;
        for (int t = 0; t < MAX_TILES && placed < count; t++) {
            // screen offset тайлу (dr,dc)
            float tx = (DC[t] - DR[t]) * F_STEP_X * 0.5f * 2;
            float ty = (DC[t] + DR[t]) * F_STEP_Y * 0.5f * 2;
            int onTile = std::min(9, count - placed);
            for (int s = 0; s < onTile; s++) {
                targets.push_back({
                    center.x + tx + SLOT_OFFSETS[SLOT_FILL_ORDER[s]].x,
                    center.y + ty + SLOT_OFFSETS[SLOT_FILL_ORDER[s]].y
                });
                placed++;
            }
        }
    }

    return targets;
}
