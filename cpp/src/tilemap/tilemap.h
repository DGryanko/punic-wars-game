#ifndef TILEMAP_H
#define TILEMAP_H

#include <vector>
#include <cstdint>
#include "terrain.h"
#include "coordinates.h"

class TileMap {
private:
    int width;   // Ширина в тайлах
    int height;  // Висота в тайлах
    std::vector<uint8_t> tiles;  // Одновимірний масив для оптимізації пам'яті
    std::vector<bool> dynamicObstacles;  // Динамічні перешкоди (будівлі)
    unsigned int seed;
    
public:
    // Конструктор
    TileMap(int w, int h, unsigned int s = 0);
    
    // Доступ до тайлів
    TerrainType getTile(int row, int col) const;
    void setTile(int row, int col, TerrainType type);
    bool isValidCoord(int row, int col) const;
    
    // Властивості карти
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    unsigned int getSeed() const { return seed; }
    
    // Запити про місцевість
    bool isPassable(int row, int col) const;
    float getSpeedModifier(int row, int col) const;
    
    // Динамічні перешкоди (будівлі)
    void setPassable(int row, int col, bool passable);
    
    // Статистика карти
    int countPassableTiles() const;
    float getPassablePercentage() const;
};

#endif // TILEMAP_H
