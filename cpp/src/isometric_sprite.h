#pragma once
#include "raylib.h"
#include "tilemap/coordinates.h"
#include <string>

// IsometricSprite class for rendering isometric sprites with debug fallback
class IsometricSprite {
private:
    Texture2D texture;
    bool loaded;
    Vector2 anchorPoint;  // Bottom center anchor
    int width, height;
    std::string filepath;  // Зберігаємо шлях для перезавантаження
    
public:
    // Constructor
    IsometricSprite();
    
    // Copy constructor
    IsometricSprite(const IsometricSprite& other);
    
    // Assignment operator
    IsometricSprite& operator=(const IsometricSprite& other);
    
    // Destructor
    ~IsometricSprite();
    
    // Load sprite from file
    bool loadFromFile(const char* filepath);
    
    // Unload sprite
    void unload();
    
    // Check if sprite is loaded
    bool isLoaded() const;
    
    // Get dimensions
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    
    // Draw sprite at screen position with optional offset
    void draw(ScreenCoords position, Color tint = WHITE, Vector2 offset = {0, 0}) const;
    
    // Debug rendering functions
    static void drawDebugUnit(ScreenCoords pos, Color color, const char* label);
    static void drawDebugBuilding(ScreenCoords pos, int widthTiles, int heightTiles, Color color, const char* label);
    static void drawDebugResource(ScreenCoords pos, Color color, char letter);
};
