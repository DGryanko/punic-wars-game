#include "isometric_renderer.h"
#include <algorithm>

IsometricRenderer::IsometricRenderer()
    : current_map(nullptr), highlight_enabled(false) {
    tileset.id = 0;
    
    // Ініціалізація камери
    camera.target = {0, 0};
    camera.offset = {512, 384};  // Центр екрану 1024x768
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
    
    visible_start = GridCoords(0, 0);
    visible_end = GridCoords(0, 0);
    highlighted_tile = GridCoords(-1, -1);
}

IsometricRenderer::~IsometricRenderer() {
    unloadTileset();
}

void IsometricRenderer::loadTileset(const char* filepath) {
    if (isTilesetLoaded()) {
        unloadTileset();
    }
    
    tileset = LoadTexture(filepath);
    
    if (tileset.id == 0) {
        TraceLog(LOG_ERROR, "Failed to load tileset: %s", filepath);
        // Створюємо fallback текстуру
        Image fallback = GenImageColor(256, 128, GRAY);
        tileset = LoadTextureFromImage(fallback);
        UnloadImage(fallback);
    } else {
        TraceLog(LOG_INFO, "Loaded tileset: %s (%dx%d)", filepath, tileset.width, tileset.height);
    }
}

void IsometricRenderer::unloadTileset() {
    if (isTilesetLoaded()) {
        UnloadTexture(tileset);
        tileset.id = 0;
    }
}

void IsometricRenderer::calculateVisibleTiles(const TileMap& map) {
    if (!isTilesetLoaded()) return;
    
    // Отримуємо межі екрану в світових координатах
    Vector2 top_left = GetScreenToWorld2D({0, 0}, camera);
    Vector2 bottom_right = GetScreenToWorld2D(
        {(float)GetScreenWidth(), (float)GetScreenHeight()}, 
        camera
    );
    
    // Конвертуємо в координати сітки
    GridCoords tl = CoordinateConverter::screenToGrid({top_left.x, top_left.y});
    GridCoords br = CoordinateConverter::screenToGrid({bottom_right.x, bottom_right.y});
    
    // Додаємо ВЕЛИКИЙ запас для перекриття тайлів та для ізометричної проекції
    // В ізометричній проекції видима область набагато більша ніж здається
    int padding = 20;  // Збільшено з 2 до 20
    visible_start.row = std::max(0, std::min(tl.row, br.row) - padding);
    visible_start.col = std::max(0, std::min(tl.col, br.col) - padding);
    visible_end.row = std::min(map.getHeight() - 1, std::max(tl.row, br.row) + padding);
    visible_end.col = std::min(map.getWidth() - 1, std::max(tl.col, br.col) + padding);
}

bool IsometricRenderer::isTileVisible(int row, int col) const {
    return row >= visible_start.row && row <= visible_end.row &&
           col >= visible_start.col && col <= visible_end.col;
}

void IsometricRenderer::renderTile(int row, int col, TerrainType type) {
    if (!isTilesetLoaded()) return;
    
    // Отримуємо екранні координати
    ScreenCoords screen = CoordinateConverter::gridToScreenWithOffset({row, col});
    
    // Отримуємо прямокутник з тайлсету
    Rectangle source = getTilesetRect(type);
    
    // Малюємо тайл
    DrawTextureRec(tileset, source, {screen.x, screen.y}, WHITE);
    
    // Якщо тайл підсвічений, малюємо рамку
    if (highlight_enabled && highlighted_tile.row == row && highlighted_tile.col == col) {
        // Малюємо напівпрозорий ромб поверх тайлу
        Color highlight_color = {255, 255, 0, 100};  // Жовтий напівпрозорий
        
        // Координати ромба (4 точки)
        Vector2 top = {screen.x + CoordinateConverter::TILE_WIDTH_HALF, screen.y};
        Vector2 right = {screen.x + CoordinateConverter::TILE_WIDTH, screen.y + CoordinateConverter::TILE_HEIGHT_HALF};
        Vector2 bottom = {screen.x + CoordinateConverter::TILE_WIDTH_HALF, screen.y + CoordinateConverter::TILE_HEIGHT};
        Vector2 left = {screen.x, screen.y + CoordinateConverter::TILE_HEIGHT_HALF};
        
        // Малюємо лінії ромба
        DrawLineEx(top, right, 2.0f, YELLOW);
        DrawLineEx(right, bottom, 2.0f, YELLOW);
        DrawLineEx(bottom, left, 2.0f, YELLOW);
        DrawLineEx(left, top, 2.0f, YELLOW);
    }
}

void IsometricRenderer::render(const TileMap& map) {
    if (!isTilesetLoaded()) {
        renderDebug(map);
        return;
    }
    
    current_map = &map;
    
    // Рендеримо всю карту без culling (80x80 не така велика)
    // Painter's algorithm: малюємо від заду до переду
    // Порядок: верхні ряди → нижні ряди, зліва направо
    for (int row = 0; row < map.getHeight(); row++) {
        for (int col = 0; col < map.getWidth(); col++) {
            TerrainType type = map.getTile(row, col);
            renderTile(row, col, type);
        }
    }
}

void IsometricRenderer::renderDebug(const TileMap& map) {
    current_map = &map;
    
    // Малюємо кольорові ромби для debug - всю карту
    for (int row = 0; row < map.getHeight(); row++) {
        for (int col = 0; col < map.getWidth(); col++) {
            TerrainType type = map.getTile(row, col);
            Color color = getDebugColor(type);
            
            ScreenCoords screen = CoordinateConverter::gridToScreenWithOffset({row, col});
            
            // Координати ромба (4 точки)
            Vector2 top = {screen.x + CoordinateConverter::TILE_WIDTH_HALF, screen.y};
            Vector2 right = {screen.x + CoordinateConverter::TILE_WIDTH, screen.y + CoordinateConverter::TILE_HEIGHT_HALF};
            Vector2 bottom = {screen.x + CoordinateConverter::TILE_WIDTH_HALF, screen.y + CoordinateConverter::TILE_HEIGHT};
            Vector2 left = {screen.x, screen.y + CoordinateConverter::TILE_HEIGHT_HALF};
            
            // Малюємо заповнений ромб
            DrawTriangle(top, right, bottom, color);
            DrawTriangle(top, bottom, left, color);
            
            // Малюємо контур
            DrawLineEx(top, right, 1.0f, BLACK);
            DrawLineEx(right, bottom, 1.0f, BLACK);
            DrawLineEx(bottom, left, 1.0f, BLACK);
            DrawLineEx(left, top, 1.0f, BLACK);
        }
    }
}

void IsometricRenderer::updateCamera(Vector2 target) {
    camera.target = target;
}

GridCoords IsometricRenderer::getHoveredTile() const {
    Vector2 mouse_pos = GetMousePosition();
    Vector2 world_pos = GetScreenToWorld2D(mouse_pos, camera);
    return CoordinateConverter::screenToGrid({world_pos.x, world_pos.y});
}

void IsometricRenderer::highlightTile(int row, int col) {
    highlighted_tile = GridCoords(row, col);
    highlight_enabled = true;
}
