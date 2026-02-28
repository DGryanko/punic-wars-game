#ifndef TERRAIN_H
#define TERRAIN_H

#include "raylib.h"

// Типи місцевості
enum class TerrainType {
    GRASS = 0,    // Трава - базова прохідна місцевість
    WATER = 1,    // Вода - непрохідна
    SAND = 2,     // Пісок - прохідна
    ROAD = 3      // Дорога - прохідна зі збільшеною швидкістю
};

// Властивості місцевості
struct TerrainProperties {
    bool passable;           // Чи можна проходити
    float speed_modifier;    // Модифікатор швидкості (1.0 = нормально, 1.5 = швидше, 0.0 = непрохідно)
    Color debug_color;       // Колір для debug рендерингу
    Rectangle tileset_rect;  // Позиція в тайлсеті (x, y, width, height)
    
    TerrainProperties(bool p, float sm, Color dc, Rectangle tr)
        : passable(p), speed_modifier(sm), debug_color(dc), tileset_rect(tr) {}
};

// Таблиця властивостей для кожного типу місцевості
// Розташування в тайлсеті 256x128 (2x2 сітка):
// [Трава 128x64] [Вода 128x64]
// [Пісок 128x64] [Дорога 128x64]
const TerrainProperties TERRAIN_PROPS[] = {
    // GRASS: прохідна, нормальна швидкість, зелений колір, позиція (0, 0)
    TerrainProperties(true, 1.0f, GREEN, {0, 0, 128, 64}),
    
    // WATER: непрохідна, нульова швидкість, синій колір, позиція (128, 0)
    TerrainProperties(false, 0.0f, BLUE, {128, 0, 128, 64}),
    
    // SAND: прохідна, нормальна швидкість, бежевий колір, позиція (0, 64)
    TerrainProperties(true, 1.0f, BEIGE, {0, 64, 128, 64}),
    
    // ROAD: прохідна, збільшена швидкість, коричневий колір, позиція (128, 64)
    TerrainProperties(true, 1.5f, BROWN, {128, 64, 128, 64})
};

// Допоміжні функції для роботи з місцевістю
inline bool isPassable(TerrainType type) {
    return TERRAIN_PROPS[static_cast<int>(type)].passable;
}

inline float getSpeedModifier(TerrainType type) {
    return TERRAIN_PROPS[static_cast<int>(type)].speed_modifier;
}

inline Color getDebugColor(TerrainType type) {
    return TERRAIN_PROPS[static_cast<int>(type)].debug_color;
}

inline Rectangle getTilesetRect(TerrainType type) {
    return TERRAIN_PROPS[static_cast<int>(type)].tileset_rect;
}

#endif // TERRAIN_H
