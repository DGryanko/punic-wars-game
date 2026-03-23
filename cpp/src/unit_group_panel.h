#pragma once
#include "raylib.h"
#include "unit.h"
#include <vector>
#include <string>

// Панель команд для вибраної групи юнітів
// Ліворуч: іконка типу + усереднене HP
// Праворуч: кнопки команд (3 ряди)
class UnitGroupPanel {
public:
    // Поточні налаштування групи (читаються ззовні)
    Unit::FormationType currentFormation = Unit::FORMATION_NONE;
    Unit::BehaviorType  currentBehavior  = Unit::BEHAVIOR_STAND;

    // Спрайти кнопок (опціонально — якщо не завантажені, малюємо текст)
    // Формат: assets/sprites/ui/group_panel/{name}.png
    // stand, passive, active, guard, line, square, chess, scout, scout_attack
    struct BtnSprite { Texture2D tex = {0}; bool loaded = false; };
    BtnSprite btnSprites[9]; // 9 кнопок

    UnitGroupPanel() {
        panelRect = { 10, 930, 520, 130 };
        _buildLayout();
    }

    void loadSprites() {
        const char* names[] = {
            "stand","passive","active","guard",
            "line","square","chess",
            "scout","scout_attack"
        };
        for (int i = 0; i < 9; i++) {
            std::string path = "assets/sprites/ui/group_panel/";
            path += names[i];
            path += ".png";
            if (FileExists(path.c_str())) {
                btnSprites[i].tex = LoadTexture(path.c_str());
                btnSprites[i].loaded = (btnSprites[i].tex.id > 0);
            }
        }
    }

    void unloadSprites() {
        for (auto& s : btnSprites)
            if (s.loaded) { UnloadTexture(s.tex); s.loaded = false; }
    }

    bool isVisible(const std::vector<Unit>& units, Faction playerFaction) const {
        // Показуємо тільки для бойових юнітів (не рабів)
        for (const auto& u : units)
            if (u.selected && u.faction == playerFaction && u.unit_type != "slave")
                return true;
        return false;
    }

    // Повертає true якщо клік поглинутий панеллю
    bool handleClick(Vector2 mousePos, std::vector<Unit>& units, Faction playerFaction) {
        if (!CheckCollisionPointRec(mousePos, panelRect)) return false;

        // Ряд 1: поведінка
        const Unit::BehaviorType behaviors[] = {
            Unit::BEHAVIOR_STAND, Unit::BEHAVIOR_PASSIVE,
            Unit::BEHAVIOR_ACTIVE, Unit::BEHAVIOR_GUARD
        };
        for (int i = 0; i < 4; i++) {
            if (CheckCollisionPointRec(mousePos, row1[i])) {
                currentBehavior = behaviors[i];
                _applyBehavior(units, playerFaction);
                return true;
            }
        }
        // Ряд 2: формація
        const Unit::FormationType formations[] = {
            Unit::FORMATION_LINE, Unit::FORMATION_SQUARE, Unit::FORMATION_CHESS
        };
        for (int i = 0; i < 3; i++) {
            if (CheckCollisionPointRec(mousePos, row2[i])) {
                currentFormation = formations[i];
                _applyFormation(units, playerFaction);
                return true;
            }
        }
        // Ряд 3: розвідка
        const Unit::BehaviorType scouts[] = {
            Unit::BEHAVIOR_SCOUT, Unit::BEHAVIOR_SCOUT_ATTACK
        };
        for (int i = 0; i < 2; i++) {
            if (CheckCollisionPointRec(mousePos, row3[i])) {
                currentBehavior = scouts[i];
                _applyBehavior(units, playerFaction);
                return true;
            }
        }
        return true; // поглинаємо будь-який клік по панелі
    }

    void draw(const std::vector<Unit>& units, Faction playerFaction) const {
        // Збираємо статистику (тільки бойові юніти)
        int count = 0; int totalHp = 0; int totalMaxHp = 0;
        std::string unitType = "";
        for (const auto& u : units) {
            if (u.selected && u.faction == playerFaction && u.unit_type != "slave") {
                count++;
                totalHp    += u.hp;
                totalMaxHp += u.max_hp;
                if (unitType.empty()) unitType = u.unit_type;
            }
        }
        if (count == 0) return;

        // Фон панелі
        DrawRectangleRec(panelRect, {0, 0, 0, 200});
        DrawRectangleLinesEx(panelRect, 1, {180, 160, 100, 200});

        // --- Ліва частина: іконка + HP ---
        float avgHpPct = (totalMaxHp > 0) ? (float)totalHp / totalMaxHp : 1.0f;
        Color hpColor = avgHpPct > 0.6f ? GREEN : (avgHpPct > 0.3f ? YELLOW : RED);

        // Іконка (квадрат-заглушка або спрайт)
        Rectangle iconRect = { panelRect.x + 8, panelRect.y + 8, 64, 64 };
        DrawRectangleRec(iconRect, {60, 50, 40, 255});
        DrawRectangleLinesEx(iconRect, 1, {180, 160, 100, 200});
        // Тип юніта (перша літера)
        char ch[2] = { (char)toupper(unitType[0]), 0 };
        DrawText(ch, (int)iconRect.x + 22, (int)iconRect.y + 18, 28, WHITE);

        // Кількість
        DrawText(TextFormat("x%d", count),
                 (int)iconRect.x + 4, (int)iconRect.y + 68, 14, LIGHTGRAY);

        // HP бар
        Rectangle hpBg  = { iconRect.x, iconRect.y + 82, 64, 10 };
        Rectangle hpBar = { hpBg.x, hpBg.y, hpBg.width * avgHpPct, hpBg.height };
        DrawRectangleRec(hpBg,  {40, 40, 40, 200});
        DrawRectangleRec(hpBar, hpColor);
        DrawRectangleLinesEx(hpBg, 1, {180, 160, 100, 150});

        // --- Кнопки ---
        // Ряд 1: поведінка
        const char* r1Labels[] = {"Stand","Passive","Active","Guard"};
        const Unit::BehaviorType r1Beh[] = {
            Unit::BEHAVIOR_STAND, Unit::BEHAVIOR_PASSIVE,
            Unit::BEHAVIOR_ACTIVE, Unit::BEHAVIOR_GUARD
        };
        for (int i = 0; i < 4; i++) {
            bool active = (currentBehavior == r1Beh[i]);
            _drawBtn(row1[i], r1Labels[i], btnSprites[i], active);
        }
        // Ряд 2: формація
        const char* r2Labels[] = {"Line","Square","Chess"};
        const Unit::FormationType r2Form[] = {
            Unit::FORMATION_LINE, Unit::FORMATION_SQUARE, Unit::FORMATION_CHESS
        };
        for (int i = 0; i < 3; i++) {
            bool active = (currentFormation == r2Form[i]);
            _drawBtn(row2[i], r2Labels[i], btnSprites[4+i], active);
        }
        // Ряд 3: розвідка
        const char* r3Labels[] = {"Scout","Scout Atk"};
        const Unit::BehaviorType r3Beh[] = {
            Unit::BEHAVIOR_SCOUT, Unit::BEHAVIOR_SCOUT_ATTACK
        };
        for (int i = 0; i < 2; i++) {
            bool active = (currentBehavior == r3Beh[i]);
            _drawBtn(row3[i], r3Labels[i], btnSprites[7+i], active);
        }
    }

    Rectangle getPanelRect() const { return panelRect; }

private:
    Rectangle panelRect;
    Rectangle row1[4]; // Stand, Passive, Active, Guard
    Rectangle row2[3]; // Line, Square, Chess
    Rectangle row3[2]; // Scout, Scout Attack

    void _buildLayout() {
        float bx = panelRect.x + 90; // після іконки
        float by = panelRect.y + 6;
        float bw = 54, bh = 34, gap = 4;

        for (int i = 0; i < 4; i++)
            row1[i] = { bx + i*(bw+gap), by,        bw, bh };
        for (int i = 0; i < 3; i++)
            row2[i] = { bx + i*(bw+gap), by+bh+gap, bw, bh };
        for (int i = 0; i < 2; i++)
            row3[i] = { bx + i*(bw+gap), by+2*(bh+gap), bw, bh };
    }

    void _drawBtn(Rectangle r, const char* label, const BtnSprite& spr, bool active) const {
        Color bg = active ? Color{120, 100, 50, 230} : Color{40, 35, 25, 200};
        Color border = active ? Color{255, 220, 80, 255} : Color{140, 120, 80, 180};
        DrawRectangleRec(r, bg);
        DrawRectangleLinesEx(r, 1, border);
        if (spr.loaded) {
            DrawTexturePro(spr.tex,
                {0,0,(float)spr.tex.width,(float)spr.tex.height},
                {r.x+2, r.y+2, r.width-4, r.height-4},
                {0,0}, 0, WHITE);
        } else {
            int fs = 10;
            int tw = MeasureText(label, fs);
            DrawText(label, (int)(r.x + (r.width-tw)/2), (int)(r.y + (r.height-fs)/2), fs, WHITE);
        }
    }

    void _applyBehavior(std::vector<Unit>& units, Faction playerFaction) {
        for (auto& u : units)
            if (u.selected && u.faction == playerFaction && u.unit_type != "slave")
                u.behavior = currentBehavior;
    }

    void _applyFormation(std::vector<Unit>& units, Faction playerFaction) {
        for (auto& u : units)
            if (u.selected && u.faction == playerFaction && u.unit_type != "slave")
                u.formation = currentFormation;
    }
};
