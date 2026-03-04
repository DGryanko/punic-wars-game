#ifndef MAP_GENERATOR_H
#define MAP_GENERATOR_H

#include "tilemap.h"
#include "noise.h"
#include "coordinates.h"
#include <utility>

// Параметри генерації карти
struct GenerationParams {
    float water_level = 0.2f;      // Поріг для води (менше води)
    float sand_level = 0.35f;      // Поріг для піску
    float grass_level = 0.95f;     // Поріг для трави (більше трави)
    float noise_scale = 0.05f;     // Менший масштаб = більші області
    int octaves = 3;               // Менше октав = більш гладкі переходи
    
    GenerationParams() = default;
};

class MapGenerator {
private:
    NoiseGenerator noise_gen;
    GenerationParams params;
    
    // Визначення типу місцевості за шумовим значенням
    TerrainType getTerrainFromNoise(float noise_value) const;
    
    // Перевірка області навколо позиції на прохідність
    bool isAreaPassable(const TileMap& map, int row, int col, int radius) const;
    
public:
    MapGenerator(unsigned int seed);
    
    // Генерація карти з параметрами за замовчуванням
    TileMap generate(int width, int height);
    
    // Генерація карти з кастомними параметрами
    TileMap generate(int width, int height, const GenerationParams& p);
    
    // Пошук стартових позицій для двох фракцій
    std::pair<GridCoords, GridCoords> findStartPositions(const TileMap& map);
    
    // Налаштування параметрів
    void setParams(const GenerationParams& p) { params = p; }
    GenerationParams getParams() const { return params; }
};

#endif // MAP_GENERATOR_H
