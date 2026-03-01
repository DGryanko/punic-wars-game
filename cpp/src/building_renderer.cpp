#include "building_renderer.h"
#include <cstdio>

void BuildingRenderer::drawBuilding(const Building& building) {
    BuildingTextureManager& texMgr = BuildingTextureManager::getInstance();
    
    // Якщо використовуємо текстури і текстура є
    if (building.use_texture && texMgr.hasTexture(building.type)) {
        Texture2D texture = texMgr.getTexture(building.type);
        if (texture.id > 0) {
            drawWithTexture(building, texture);
        } else {
            printf("[RENDER] Warning: Texture ID is 0 for building type %d\n", building.type);
            drawFallback(building);
        }
    } else {
        // Fallback на кольоровий прямокутник
        if (!building.use_texture) {
            printf("[RENDER] Building not using texture\n");
        }
        if (!texMgr.hasTexture(building.type)) {
            printf("[RENDER] Texture not found for building type %d\n", building.type);
        }
        drawFallback(building);
    }
    
    // Малюємо індикатор виділення
    if (building.selected) {
        drawSelectionIndicator(building);
    }
    
    // Малюємо прогрес виробництва
    if (building.is_producing) {
        drawProductionProgress(building);
    }
    
    // Назви не малюємо, щоб не накладалися
    // drawBuildingName(building);
}

void BuildingRenderer::drawWithTexture(const Building& building, const Texture2D& texture) {
    // Розраховуємо позицію з урахуванням offset
    float drawX = building.x + building.texture_offset.x;
    float drawY = building.y + building.texture_offset.y;
    
    // Розраховуємо розмір з урахуванням scale
    float drawWidth = texture.width * building.texture_scale;
    float drawHeight = texture.height * building.texture_scale;
    
    // Малюємо текстуру
    Rectangle source = {0, 0, (float)texture.width, (float)texture.height};
    Rectangle dest = {drawX, drawY, drawWidth, drawHeight};
    Vector2 origin = {0, 0};
    
    DrawTexturePro(texture, source, dest, origin, 0.0f, WHITE);
}

void BuildingRenderer::drawFallback(const Building& building) {
    Color buildingColor = building.getColor();
    
    // Якщо вибрана, зробити яскравішою
    if (building.selected) {
        buildingColor.r = (unsigned char)(buildingColor.r * 1.3f);
        buildingColor.g = (unsigned char)(buildingColor.g * 1.3f);
        buildingColor.b = (unsigned char)(buildingColor.b * 1.3f);
    }
    
    // Малювання будівлі
    DrawRectangle(building.x, building.y, 80, 60, buildingColor);
    
    // Рамка
    DrawRectangleLines(building.x, building.y, 80, 60, WHITE);
}

void BuildingRenderer::drawSelectionIndicator(const Building& building) {
    // Малюємо жовту рамку навколо будівлі
    DrawRectangleLines(building.x - 2, building.y - 2, 84, 64, YELLOW);
    DrawRectangleLines(building.x - 3, building.y - 3, 86, 66, YELLOW);
}

void BuildingRenderer::drawProductionProgress(const Building& building) {
    float progress = building.production_progress / building.production_time;
    
    // Прогрес-бар під будівлею
    int barY = building.y + 65;
    DrawRectangle(building.x, barY, (int)(80 * progress), 5, GREEN);
    DrawRectangleLines(building.x, barY, 80, 5, WHITE);
}

void BuildingRenderer::drawBuildingName(const Building& building) {
    // Малюємо назву будівлі
    DrawText(building.name.c_str(), building.x + 5, building.y + 25, 10, WHITE);
}

void BuildingRenderer::drawAllBuildings(const std::vector<Building>& buildings) {
    for (const auto& building : buildings) {
        drawBuilding(building);
    }
}
