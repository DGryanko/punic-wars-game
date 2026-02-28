#ifndef MAP_GENERATOR_H
#define MAP_GENERATOR_H

#include "tilemap.h"
#include "noise.h"
#include "coordinates.h"
#include <utility>

// Параметри генерації карти
struct GenerationParams {
    float water_level = 0.3f;      // Поріг для води (0.0 - 0.3)
    float sand_level = 0.5f;       // Поріг для піску (0.3 - 0.5)
    float grass_level = 0.9f;      // Поріг для трави (0.5 - 0.9)
    float noise_scale = 0.1f;      // Масштаб шуму
    int octaves = 4;               // Кількість октав для fractal noise
    
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
