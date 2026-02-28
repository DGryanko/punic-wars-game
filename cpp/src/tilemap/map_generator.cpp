#include "map_generator.h"
#include "raylib.h"
#include <cmath>
#include <random>

MapGenerator::MapGenerator(unsigned int seed)
    : noise_gen(seed) {
    noise_gen.setScale(params.noise_scale);
    noise_gen.setOctaves(params.octaves);
}

TerrainType MapGenerator::getTerrainFromNoise(float noise_value) const {
    if (noise_value < params.water_level) {
        return TerrainType::WATER;
    } else if (noise_value < params.sand_level) {
        return TerrainType::SAND;
    } else if (noise_value < params.grass_level) {
        return TerrainType::GRASS;
    } else {
        return TerrainType::ROAD;  // Рідкісні тайли доріг
    }
}

TileMap MapGenerator::generate(int width, int height) {
    return generate(width, height, params);
}

TileMap MapGenerator::generate(int width, int height, const GenerationParams& p) {
    // Валідація розміру
    if (width <= 0 || height <= 0) {
        TraceLog(LOG_WARNING, "Invalid map size: %dx%d, using 50x50", width, height);
        width = 50;
        height = 50;
    }
    
    if (width > 200 || height > 200) {
        TraceLog(LOG_WARNING, "Map too large: %dx%d, clamping to 200x200", width, height);
        width = (width > 200) ? 200 : width;
        height = (height > 200) ? 200 : height;
    }
    
    // Оновлюємо параметри
    params = p;
    noise_gen.setScale(params.noise_scale);
    noise_gen.setOctaves(params.octaves);
    
    // Створюємо карту
    TileMap map(width, height, noise_gen.getScale());
    
    // Генеруємо місцевість за допомогою шуму
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            float noise_value = noise_gen.fractalNoise2D(
                static_cast<float>(col),
                static_cast<float>(row)
            );
            
            TerrainType terrain = getTerrainFromNoise(noise_value);
            map.setTile(row, col, terrain);
        }
    }
    
    // Перевіряємо мінімальну прохідність (60%)
    float passable_pct = map.getPassablePercentage();
    if (passable_pct < 0.6f) {
        TraceLog(LOG_WARNING, "Map has only %.1f%% passable tiles (minimum 60%%)", passable_pct * 100.0f);
        // Можна додати логіку для регенерації або корекції
    }
    
    TraceLog(LOG_INFO, "Generated map %dx%d with %.1f%% passable tiles", 
             width, height, passable_pct * 100.0f);
    
    return map;
}

bool MapGenerator::isAreaPassable(const TileMap& map, int row, int col, int radius) const {
    for (int r = row - radius; r <= row + radius; r++) {
        for (int c = col - radius; c <= col + radius; c++) {
            if (!map.isValidCoord(r, c) || !map.isPassable(r, c)) {
                return false;
            }
        }
    }
    return true;
}

std::pair<GridCoords, GridCoords> MapGenerator::findStartPositions(const TileMap& map) {
    int width = map.getWidth();
    int height = map.getHeight();
    
    // Обчислюємо мінімальну відстань (30% діагоналі)
    float diagonal = std::sqrt(width * width + height * height);
    float min_distance = diagonal * 0.3f;
    
    std::mt19937 rng(map.getSeed());
    std::uniform_int_distribution<int> dist_row(2, height - 3);
    std::uniform_int_distribution<int> dist_col(2, width - 3);
    
    GridCoords pos1, pos2;
    int max_attempts = 1000;
    int attempts = 0;
    
    // Шукаємо першу позицію
    while (attempts < max_attempts) {
        int row = dist_row(rng);
        int col = dist_col(rng);
        
        if (map.isPassable(row, col) && isAreaPassable(map, row, col, 2)) {
            pos1 = GridCoords(row, col);
            break;
        }
        attempts++;
    }
    
    if (attempts >= max_attempts) {
        TraceLog(LOG_WARNING, "Could not find first start position, using default");
        pos1 = GridCoords(height / 4, width / 4);
    }
    
    // Шукаємо другу позицію на достатній відстані
    attempts = 0;
    while (attempts < max_attempts) {
        int row = dist_row(rng);
        int col = dist_col(rng);
        
        if (map.isPassable(row, col) && isAreaPassable(map, row, col, 2)) {
            // Перевіряємо відстань
            float dx = col - pos1.col;
            float dy = row - pos1.row;
            float distance = std::sqrt(dx * dx + dy * dy);
            
            if (distance >= min_distance) {
                pos2 = GridCoords(row, col);
                break;
            }
        }
        attempts++;
    }
    
    if (attempts >= max_attempts) {
        TraceLog(LOG_WARNING, "Could not find second start position, using default");
        pos2 = GridCoords(3 * height / 4, 3 * width / 4);
    }
    
    TraceLog(LOG_INFO, "Start positions: (%d, %d) and (%d, %d)", 
             pos1.row, pos1.col, pos2.row, pos2.col);
    
    return std::make_pair(pos1, pos2);
}
