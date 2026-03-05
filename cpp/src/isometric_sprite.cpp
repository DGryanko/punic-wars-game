#include "isometric_sprite.h"
#include <cmath>

IsometricSprite::IsometricSprite() {
    loaded = false;
    texture.id = 0;
    width = 0;
    height = 0;
    anchorPoint = {0, 0};
    filepath = "";
}

// Copy constructor - перезавантажуємо текстуру з того самого файлу
IsometricSprite::IsometricSprite(const IsometricSprite& other) {
    loaded = false;
    texture.id = 0;
    width = other.width;
    height = other.height;
    anchorPoint = other.anchorPoint;
    filepath = other.filepath;
    
    // Якщо оригінал мав завантажену текстуру, завантажуємо її знову
    if (!filepath.empty() && other.loaded) {
        loadFromFile(filepath.c_str());
    }
}

// Assignment operator - перезавантажуємо текстуру
IsometricSprite& IsometricSprite::operator=(const IsometricSprite& other) {
    if (this != &other) {
        // Вивантажуємо стару текстуру
        unload();
        
        // Копіюємо параметри
        width = other.width;
        height = other.height;
        anchorPoint = other.anchorPoint;
        filepath = other.filepath;
        
        // Якщо оригінал мав завантажену текстуру, завантажуємо її знову
        if (!filepath.empty() && other.loaded) {
            loadFromFile(filepath.c_str());
        }
    }
    return *this;
}

IsometricSprite::~IsometricSprite() {
    unload();
}

bool IsometricSprite::loadFromFile(const char* filepath) {
    // Зберігаємо шлях для можливого перезавантаження
    this->filepath = filepath;
    
    if (FileExists(filepath)) {
        texture = LoadTexture(filepath);
        if (texture.id > 0) {
            loaded = true;
            width = texture.width;
            height = texture.height;
            // Anchor at bottom center
            anchorPoint = {(float)width / 2.0f, (float)height};
            TraceLog(LOG_INFO, "[SPRITE] Loaded: %s (%dx%d)", filepath, width, height);
            return true;
        }
    }
    TraceLog(LOG_WARNING, "[SPRITE] Not found, using debug mode: %s", filepath);
    loaded = false;
    return false;
}

void IsometricSprite::unload() {
    if (loaded && texture.id > 0) {
        UnloadTexture(texture);
        texture.id = 0;
        loaded = false;
    }
}

bool IsometricSprite::isLoaded() const {
    return loaded;
}

void IsometricSprite::draw(ScreenCoords position, Color tint, Vector2 offset) const {
    if (loaded) {
        // Draw sprite with anchor at bottom center + custom offset
        Vector2 drawPos = {
            position.x - anchorPoint.x + offset.x,
            position.y - anchorPoint.y + offset.y
        };
        DrawTextureV(texture, drawPos, tint);
    }
}

// Debug rendering: Unit as isometric diamond (32x16 pixels)
void IsometricSprite::drawDebugUnit(ScreenCoords pos, Color color, const char* label) {
    // Draw isometric diamond shape (32x16 pixels - half tile size)
    Vector2 points[4] = {
        {pos.x, pos.y - 8},      // Top
        {pos.x + 16, pos.y},     // Right
        {pos.x, pos.y + 8},      // Bottom
        {pos.x - 16, pos.y}      // Left
    };
    
    // Fill diamond
    DrawTriangle(points[0], points[1], points[2], color);
    DrawTriangle(points[0], points[2], points[3], color);
    
    // Black outline for visibility
    DrawLineV(points[0], points[1], BLACK);
    DrawLineV(points[1], points[2], BLACK);
    DrawLineV(points[2], points[3], BLACK);
    DrawLineV(points[3], points[0], BLACK);
    
    // Draw label above shape
    if (label) {
        int textWidth = MeasureText(label, 10);
        DrawText(label, (int)(pos.x - textWidth / 2), (int)(pos.y - 20), 10, BLACK);
    }
}

// Debug rendering: Building as isometric rectangle
void IsometricSprite::drawDebugBuilding(ScreenCoords pos, int widthTiles, int heightTiles, Color color, const char* label) {
    // Calculate isometric rectangle dimensions
    // Each tile is 64x32 in isometric view
    int isoWidth = widthTiles * 64;
    int isoHeight = heightTiles * 32;
    
    // Draw isometric rectangle (simplified as parallelogram)
    Vector2 topLeft = {pos.x - isoWidth / 2.0f, pos.y - isoHeight};
    Vector2 topRight = {pos.x + isoWidth / 2.0f, pos.y - isoHeight};
    Vector2 bottomRight = {pos.x + isoWidth / 2.0f, pos.y};
    Vector2 bottomLeft = {pos.x - isoWidth / 2.0f, pos.y};
    
    // Fill rectangle
    DrawTriangle(topLeft, topRight, bottomRight, color);
    DrawTriangle(topLeft, bottomRight, bottomLeft, color);
    
    // Black outline
    DrawLineV(topLeft, topRight, BLACK);
    DrawLineV(topRight, bottomRight, BLACK);
    DrawLineV(bottomRight, bottomLeft, BLACK);
    DrawLineV(bottomLeft, topLeft, BLACK);
    
    // Draw label in center
    if (label) {
        int textWidth = MeasureText(label, 10);
        DrawText(label, (int)(pos.x - textWidth / 2), (int)(pos.y - isoHeight / 2), 10, WHITE);
    }
}

// Debug rendering: Resource as square with letter
void IsometricSprite::drawDebugResource(ScreenCoords pos, Color color, char letter) {
    // Draw 24x24 square
    Rectangle rect = {pos.x - 12, pos.y - 12, 24, 24};
    DrawRectangleRec(rect, color);
    DrawRectangleLinesEx(rect, 2, BLACK);
    
    // Draw letter in center
    char text[2] = {letter, '\0'};
    int textWidth = MeasureText(text, 16);
    DrawText(text, (int)(pos.x - textWidth / 2), (int)(pos.y - 8), 16, WHITE);
}
