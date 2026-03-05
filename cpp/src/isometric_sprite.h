#pragma once
#include "raylib.h"
#include "tilemap/coordinates.h"

// IsometricSprite class for rendering isometric sprites with debug fallback
class IsometricSprite {
private:
    Texture2D texture;
    bool loaded;
    Vector2 anchorPoint;  // Bottom center anchor
    int width, height;
    
public:
    // Constructor
    IsometricSprite();
    
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
    
    // Draw sprite at screen position
    void draw(ScreenCoords position, Color tint = WHITE) const;
    
    // Debug rendering functions
    static void drawDebugUnit(ScreenCoords pos, Color color, const char* label);
    static void drawDebugBuilding(ScreenCoords pos, int widthTiles, int heightTiles, Color color, const char* label);
    static void drawDebugResource(ScreenCoords pos, Color color, char letter);
};
