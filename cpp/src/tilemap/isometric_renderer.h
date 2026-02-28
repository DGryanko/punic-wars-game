#ifndef ISOMETRIC_RENDERER_H
#define ISOMETRIC_RENDERER_H

#include "raylib.h"
#include "tilemap.h"
#include "coordinates.h"

class IsometricRenderer {
private:
    Texture2D tileset;
    Camera2D camera;
    const TileMap* current_map;
    
    // Viewport culling
    GridCoords visible_start;
    GridCoords visible_end;
    
    // Підсвічування тайлу
    GridCoords highlighted_tile;
    bool highlight_enabled;
    
public:
    IsometricRenderer();
    ~IsometricRenderer();
    
    // Завантаження ресурсів
    void loadTileset(const char* filepath);
    void unloadTileset();
    bool isTilesetLoaded() const { return tileset.id != 0; }
    
    // Рендеринг
    void render(const TileMap& map);
    void renderTile(int row, int col, TerrainType type);
    void renderDebug(const TileMap& map);
    
    // Камера
    void setCamera(const Camera2D& cam) { camera = cam; }
    Camera2D getCamera() const { return camera; }
    void updateCamera(Vector2 target);
    
    // Viewport culling
    void calculateVisibleTiles(const TileMap& map);
    bool isTileVisible(int row, int col) const;
    
    // Взаємодія з мишею
    GridCoords getHoveredTile() const;
    void highlightTile(int row, int col);
    void disableHighlight() { highlight_enabled = false; }
};

#endif // ISOMETRIC_RENDERER_H
