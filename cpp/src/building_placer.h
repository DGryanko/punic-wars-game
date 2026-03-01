#ifndef BUILDING_PLACER_H
#define BUILDING_PLACER_H

#include "building.h"
#include "pathfinding.h"
#include "tilemap/tilemap.h"
#include "tilemap/coordinates.h"
#include <map>

// Система розміщення будівель на тайловій сітці
class BuildingPlacer {
private:
    PathfindingManager* pathfinding;
    TileMap* map;
    
    // Мапа зайнятих тайлів (row * 1000 + col -> building index)
    std::map<int, int> occupiedTiles;
    
public:
    BuildingPlacer() : pathfinding(nullptr), map(nullptr) {}
    
    // Ініціалізація
    void init(PathfindingManager* pf, TileMap* m) {
        pathfinding = pf;
        map = m;
        occupiedTiles.clear();
    }
    
    // Розміщення будівлі на тайлі
    bool placeBuilding(Building& building, int row, int col, int buildingIndex = -1) {
        // Перевірка валідності координат
        if (!map || !map->isValidCoord(row, col)) {
            printf("[BuildingPlacer] Invalid tile coordinates: (%d, %d)\n", row, col);
            return false;
        }
        
        // Перевірка чи тайл прохідний
        if (!map->isPassable(row, col)) {
            printf("[BuildingPlacer] Tile (%d, %d) is not passable\n", row, col);
            return false;
        }
        
        // Перевірка чи тайл вільний
        if (!isTileFree(row, col)) {
            printf("[BuildingPlacer] Tile (%d, %d) is already occupied\n", row, col);
            return false;
        }
        
        // Конвертуємо тайлові координати в екранні
        GridCoords gridPos(row, col);
        ScreenCoords screenPos = CoordinateConverter::gridToScreenWithOffset(gridPos);
        
        // Встановлюємо позицію будівлі
        building.x = (int)screenPos.x;
        building.y = (int)screenPos.y;
        
        // Зберігаємо тайлові координати в будівлі (додамо нові поля)
        building.tile_row = row;
        building.tile_col = col;
        
        // Позначаємо тайл як зайнятий
        int tileKey = row * 1000 + col;
        occupiedTiles[tileKey] = buildingIndex;
        
        // Блокуємо тайл в pathfinding
        if (pathfinding) {
            int gridX, gridY;
            pathfinding->getGrid().worldToGrid(building.x + 40, building.y + 30, gridX, gridY);
            const_cast<NavigationGrid&>(pathfinding->getGrid()).markObstacle(gridX, gridY, true);
        }
        
        printf("[BuildingPlacer] Building placed at tile (%d, %d), screen (%d, %d)\n", 
               row, col, building.x, building.y);
        
        return true;
    }
    
    // Видалення будівлі
    void removeBuilding(const Building& building) {
        // Знаходимо тайл будівлі
        int tileKey = building.tile_row * 1000 + building.tile_col;
        occupiedTiles.erase(tileKey);
        
        // Розблоковуємо тайл в pathfinding
        if (pathfinding) {
            int gridX, gridY;
            pathfinding->getGrid().worldToGrid(building.x + 40, building.y + 30, gridX, gridY);
            const_cast<NavigationGrid&>(pathfinding->getGrid()).markObstacle(gridX, gridY, false);
        }
        
        printf("[BuildingPlacer] Building removed from tile (%d, %d)\n", 
               building.tile_row, building.tile_col);
    }
    
    // Перевірка чи тайл вільний
    bool isTileFree(int row, int col) const {
        int tileKey = row * 1000 + col;
        return occupiedTiles.find(tileKey) == occupiedTiles.end();
    }
    
    // Отримати тайлові координати будівлі
    GridCoords getBuildingTileCoords(const Building& building) const {
        return GridCoords(building.tile_row, building.tile_col);
    }
    
    // Очистити всі зайняті тайли
    void clear() {
        occupiedTiles.clear();
    }
};

#endif // BUILDING_PLACER_H
