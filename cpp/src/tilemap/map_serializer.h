#ifndef MAP_SERIALIZER_H
#define MAP_SERIALIZER_H

#include "tilemap.h"
#include <string>

class MapSerializer {
public:
    // Збереження у простий текстовий формат
    static bool saveToFile(const TileMap& map, const char* filepath);
    
    // Завантаження з файлу
    static TileMap loadFromFile(const char* filepath);
    
    // Перевірка існування файлу
    static bool fileExists(const char* filepath);
};

#endif // MAP_SERIALIZER_H
