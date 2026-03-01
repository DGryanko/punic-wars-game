#pragma once
#include "raylib.h"
#include "building.h"
#include "building_texture_manager.h"
#include <vector>

// Статичний клас для малювання будівель
class BuildingRenderer {
public:
    // Малювання однієї будівлі
    static void drawBuilding(const Building& building);
    
    // Малювання всіх будівель
    static void drawAllBuildings(const std::vector<Building>& buildings);
    
    // Малювання будівлі з текстурою
    static void drawWithTexture(const Building& building, const Texture2D& texture);
    
private:
    // Малювання індикатора виділення
    static void drawSelectionIndicator(const Building& building);
    
    // Малювання прогрес-бару виробництва
    static void drawProductionProgress(const Building& building);
    
    // Малювання назви будівлі
    static void drawBuildingName(const Building& building);
    
    // Fallback малювання (кольоровий прямокутник)
    static void drawFallback(const Building& building);
};
