#pragma once
#include "raylib.h"
#include "building.h"
#include <cstdio>

// External resource variables
extern int rome_food, rome_money, rome_food_cap, rome_money_cap;
extern int carth_food, carth_money, carth_food_cap, carth_money_cap;

// Tentorium Trade Panel
// Shows when a TENTORIUM building is selected.
// Trades: 5 food -> 1 coin  |  1 coin -> 10 food
struct TentoriumPanel {
    Rectangle panelRect   = {10, 870, 340, 110};
    Rectangle btnFoodCoin = {20,  900, 150, 50};
    Rectangle btnCoinFood = {180, 900, 150, 50};

    Faction playerFaction = ROME;
    bool visible = false;

    Texture2D foodIcon = {0};
    Texture2D coinIcon = {0};

    void init(Faction faction) {
        playerFaction = faction;
        if (FileExists("assets/sprites/isometric/resources/food_source.png"))
            foodIcon = LoadTexture("assets/sprites/isometric/resources/food_source.png");
        if (FileExists("assets/sprites/isometric/resources/gold_source.png"))
            coinIcon = LoadTexture("assets/sprites/isometric/resources/gold_source.png");
    }

    void unload() {
        if (foodIcon.id > 0) UnloadTexture(foodIcon);
        if (coinIcon.id > 0) UnloadTexture(coinIcon);
    }

    void setVisible(bool v) { visible = v; }
    bool isVisible() const { return visible; }

    // Returns true if click was consumed
    bool handleClick(Vector2 mousePos) {
        if (!visible) return false;
        if (!CheckCollisionPointRec(mousePos, panelRect)) return false;

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(mousePos, btnFoodCoin)) {
                doTrade(5, 0, 0, 1);
                return true;
            }
            if (CheckCollisionPointRec(mousePos, btnCoinFood)) {
                doTrade(0, 1, 10, 0);
                return true;
            }
        }
        return false;
    }

    void draw() const {
        if (!visible) return;

        DrawRectangleRec(panelRect, {20, 15, 10, 210});
        DrawRectangleLinesEx(panelRect, 1, {180, 140, 60, 255});
        DrawText("Tentorium - Trade", (int)panelRect.x + 8, (int)panelRect.y + 8, 14, {220, 180, 80, 255});

        int food = (playerFaction == ROME) ? rome_food : carth_food;
        int coin = (playerFaction == ROME) ? rome_money : carth_money;

        // Button: 5 food -> 1 coin
        bool canBuyCoins = food >= 5;
        Color c1 = canBuyCoins ? Color{40, 80, 40, 220} : Color{60, 40, 40, 220};
        DrawRectangleRec(btnFoodCoin, c1);
        DrawRectangleLinesEx(btnFoodCoin, 1, WHITE);
        drawIcon(foodIcon, btnFoodCoin.x + 4, btnFoodCoin.y + 9);
        DrawText("x5",  (int)btnFoodCoin.x + 34, (int)btnFoodCoin.y + 14, 13, YELLOW);
        DrawText("->",  (int)btnFoodCoin.x + 60, (int)btnFoodCoin.y + 14, 13, WHITE);
        drawIcon(coinIcon, btnFoodCoin.x + 84, btnFoodCoin.y + 9);
        DrawText("x1",  (int)btnFoodCoin.x + 114, (int)btnFoodCoin.y + 14, 13, YELLOW);

        // Button: 1 coin -> 10 food
        bool canBuyFood = coin >= 1;
        Color c2 = canBuyFood ? Color{40, 80, 40, 220} : Color{60, 40, 40, 220};
        DrawRectangleRec(btnCoinFood, c2);
        DrawRectangleLinesEx(btnCoinFood, 1, WHITE);
        drawIcon(coinIcon, btnCoinFood.x + 4, btnCoinFood.y + 9);
        DrawText("x1",  (int)btnCoinFood.x + 34, (int)btnCoinFood.y + 14, 13, YELLOW);
        DrawText("->",  (int)btnCoinFood.x + 60, (int)btnCoinFood.y + 14, 13, WHITE);
        drawIcon(foodIcon, btnCoinFood.x + 84, btnCoinFood.y + 9);
        DrawText("x10", (int)btnCoinFood.x + 114, (int)btnCoinFood.y + 14, 13, YELLOW);
    }

private:
    void drawIcon(Texture2D tex, float x, float y) const {
        if (tex.id > 0)
            DrawTexturePro(tex, {0,0,(float)tex.width,(float)tex.height},
                           {x, y, 28, 28}, {0,0}, 0, WHITE);
        else
            DrawRectangle((int)x, (int)y, 28, 28, {80,80,80,180});
    }

    void doTrade(int costFood, int costCoin, int gainFood, int gainCoin) const {
        int& food = (playerFaction == ROME) ? rome_food : carth_food;
        int& coin = (playerFaction == ROME) ? rome_money : carth_money;
        int  foodCap = (playerFaction == ROME) ? rome_food_cap : carth_food_cap;
        int  coinCap = (playerFaction == ROME) ? rome_money_cap : carth_money_cap;

        if (food < costFood || coin < costCoin) return;
        food -= costFood;
        coin -= costCoin;
        int newFood = food + gainFood;
        int newCoin = coin + gainCoin;
        food = (newFood > foodCap) ? foodCap : newFood;
        coin = (newCoin > coinCap) ? coinCap : newCoin;
        printf("[TENTORIUM] Trade: -%dF -%dC +%dF +%dC\n", costFood, costCoin, gainFood, gainCoin);
    }
};
