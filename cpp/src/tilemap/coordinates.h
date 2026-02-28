#ifndef COORDINATES_H
#define COORDINATES_H

#include <cmath>

// Координати сітки (логічні)
struct GridCoords {
    int row;
    int col;
    
    GridCoords() : row(0), col(0) {}
    GridCoords(int r, int c) : row(r), col(c) {}
    
    bool operator==(const GridCoords& other) const {
        return row == other.row && col == other.col;
    }
};

// Екранні координати (пікселі)
struct ScreenCoords {
    float x;
    float y;
    
    ScreenCoords() : x(0.0f), y(0.0f) {}
    ScreenCoords(float px, float py) : x(px), y(py) {}
};

// Конвертер координат для ізометричної проекції
class CoordinateConverter {
public:
    // Константи розмірів тайлів
    static constexpr int TILE_WIDTH = 128;
    static constexpr int TILE_HEIGHT = 64;
    static constexpr int TILE_WIDTH_HALF = 64;
    static constexpr int TILE_HEIGHT_HALF = 32;
    
    // Конвертація Grid → Screen
    // Формула: screenX = (col - row) * TILE_WIDTH_HALF
    //          screenY = (col + row) * TILE_HEIGHT_HALF
    static ScreenCoords gridToScreen(const GridCoords& grid) {
        ScreenCoords screen;
        screen.x = (grid.col - grid.row) * TILE_WIDTH_HALF;
        screen.y = (grid.col + grid.row) * TILE_HEIGHT_HALF;
        return screen;
    }
    
    // Конвертація Screen → Grid
    // Формула: col = (screenX / TILE_WIDTH_HALF + screenY / TILE_HEIGHT_HALF) / 2
    //          row = (screenY / TILE_HEIGHT_HALF - screenX / TILE_WIDTH_HALF) / 2
    static GridCoords screenToGrid(const ScreenCoords& screen) {
        GridCoords grid;
        float col_f = (screen.x / TILE_WIDTH_HALF + screen.y / TILE_HEIGHT_HALF) / 2.0f;
        float row_f = (screen.y / TILE_HEIGHT_HALF - screen.x / TILE_WIDTH_HALF) / 2.0f;
        
        grid.col = static_cast<int>(std::round(col_f));
        grid.row = static_cast<int>(std::round(row_f));
        return grid;
    }
    
    // Конвертація Grid → Screen з offset для малювання (верхній кут тайлу)
    static ScreenCoords gridToScreenWithOffset(const GridCoords& grid) {
        ScreenCoords screen = gridToScreen(grid);
        // Зміщуємо на половину ширини тайлу вліво, щоб малювати від верхнього кута
        screen.x -= TILE_WIDTH_HALF;
        return screen;
    }
};

#endif // COORDINATES_H
