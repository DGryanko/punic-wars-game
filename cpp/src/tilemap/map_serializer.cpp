#include "map_serializer.h"
#include "raylib.h"
#include <fstream>
#include <sstream>

bool MapSerializer::fileExists(const char* filepath) {
    return FileExists(filepath);
}

bool MapSerializer::saveToFile(const TileMap& map, const char* filepath) {
    std::ofstream file(filepath);
    
    if (!file.is_open()) {
        TraceLog(LOG_ERROR, "Failed to open file for writing: %s", filepath);
        return false;
    }
    
    // Записуємо заголовок
    file << "# Punic Wars Tilemap\n";
    file << "width=" << map.getWidth() << "\n";
    file << "height=" << map.getHeight() << "\n";
    file << "seed=" << map.getSeed() << "\n";
    file << "# Tiles (0=GRASS, 1=WATER, 2=SAND, 3=ROAD)\n";
    
    // Записуємо тайли
    for (int row = 0; row < map.getHeight(); row++) {
        for (int col = 0; col < map.getWidth(); col++) {
            TerrainType type = map.getTile(row, col);
            file << static_cast<int>(type);
            if (col < map.getWidth() - 1) {
                file << ",";
            }
        }
        file << "\n";
    }
    
    file.close();
    TraceLog(LOG_INFO, "Saved map to: %s", filepath);
    return true;
}

TileMap MapSerializer::loadFromFile(const char* filepath) {
    if (!fileExists(filepath)) {
        TraceLog(LOG_ERROR, "Map file not found: %s", filepath);
        return TileMap(10, 10);  // Повертаємо порожню карту
    }
    
    std::ifstream file(filepath);
    
    if (!file.is_open()) {
        TraceLog(LOG_ERROR, "Failed to open file for reading: %s", filepath);
        return TileMap(10, 10);
    }
    
    int width = 0, height = 0;
    unsigned int seed = 0;
    std::string line;
    
    // Читаємо заголовок
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        if (line.find("width=") == 0) {
            width = std::stoi(line.substr(6));
        } else if (line.find("height=") == 0) {
            height = std::stoi(line.substr(7));
        } else if (line.find("seed=") == 0) {
            seed = std::stoul(line.substr(5));
        } else {
            // Перший рядок з тайлами - виходимо з циклу заголовка
            break;
        }
    }
    
    if (width <= 0 || height <= 0) {
        TraceLog(LOG_ERROR, "Invalid map dimensions in file: %s", filepath);
        file.close();
        return TileMap(10, 10);
    }
    
    // Створюємо карту
    TileMap map(width, height, seed);
    
    // Читаємо тайли (перший рядок вже в line)
    int row = 0;
    do {
        if (line.empty() || line[0] == '#') continue;
        
        std::stringstream ss(line);
        std::string value;
        int col = 0;
        
        while (std::getline(ss, value, ',')) {
            if (col < width && row < height) {
                int terrain_int = std::stoi(value);
                TerrainType terrain = static_cast<TerrainType>(terrain_int);
                map.setTile(row, col, terrain);
            }
            col++;
        }
        
        row++;
    } while (std::getline(file, line) && row < height);
    
    file.close();
    TraceLog(LOG_INFO, "Loaded map from: %s (%dx%d)", filepath, width, height);
    return map;
}
