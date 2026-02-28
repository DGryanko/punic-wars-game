#include "tilemap.h"
#include "raylib.h"

TileMap::TileMap(int w, int h, unsigned int s)
    : width(w), height(h), seed(s) {
    // Ініціалізуємо масив тайлів
    tiles.resize(width * height, static_cast<uint8_t>(TerrainType::GRASS));
}

TerrainType TileMap::getTile(int row, int col) const {
    if (!isValidCoord(row, col)) {
        TraceLog(LOG_WARNING, "Invalid tile coordinates: (%d, %d)", row, col);
        return TerrainType::GRASS;  // Безпечне значення за замовчуванням
    }
    return static_cast<TerrainType>(tiles[row * width + col]);
}

void TileMap::setTile(int row, int col, TerrainType type) {
    if (!isValidCoord(row, col)) {
        TraceLog(LOG_WARNING, "Cannot set tile at invalid coordinates: (%d, %d)", row, col);
        return;
    }
    tiles[row * width + col] = static_cast<uint8_t>(type);
}

bool TileMap::isValidCoord(int row, int col) const {
    return row >= 0 && row < height && col >= 0 && col < width;
}

bool TileMap::isPassable(int row, int col) const {
    if (!isValidCoord(row, col)) {
        return false;
    }
    TerrainType type = getTile(row, col);
    return ::isPassable(type);  // Використовуємо глобальну функцію з terrain.h
}

float TileMap::getSpeedModifier(int row, int col) const {
    if (!isValidCoord(row, col)) {
        return 0.0f;
    }
    TerrainType type = getTile(row, col);
    return ::getSpeedModifier(type);  // Використовуємо глобальну функцію з terrain.h
}

int TileMap::countPassableTiles() const {
    int count = 0;
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            if (isPassable(row, col)) {
                count++;
            }
        }
    }
    return count;
}

float TileMap::getPassablePercentage() const {
    int total = width * height;
    if (total == 0) return 0.0f;
    return (float)countPassableTiles() / (float)total;
}
