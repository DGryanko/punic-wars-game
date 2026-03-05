#ifndef FACTION_SPAWNER_H
#define FACTION_SPAWNER_H

#include "building_placer.h"
#include "tilemap/tilemap.h"
#include "building.h"
#include <vector>
#include <cstdlib>
#include <cmath>

// Конфігурація спавну
struct SpawnConfiguration {
    int minDistanceBetweenHQs = 20;  // Мінімальна відстань між HQ
    int maxSpawnAttempts = 100;      // Максимум спроб знайти позицію
};

// Система спавну головних наметів фракцій
class FactionSpawner {
private:
    BuildingPlacer* placer;
    TileMap* map;
    std::vector<Building>* buildings;
    SpawnConfiguration config;
    
public:
    FactionSpawner() : placer(nullptr), map(nullptr), buildings(nullptr) {}
    
    // Ініціалізація
    void init(BuildingPlacer* bp, TileMap* m, std::vector<Building>* b) {
        placer = bp;
        map = m;
        buildings = b;
    }
    
    // Встановити конфігурацію
    void setConfig(const SpawnConfiguration& cfg) {
        config = cfg;
    }
    
    // Спавн HQ обох фракцій
    void spawnFactionHQs() {
        if (!placer || !map || !buildings) {
            printf("[FactionSpawner] Error: Not initialized!\n");
            return;
        }
        
        printf("[FactionSpawner] Spawning faction HQs...\n");
        
        // Знайти позицію для Rome HQ
        GridCoords romePos = findRandomFreeTile();
        if (romePos.row == -1) {
            printf("[FactionSpawner] Error: Cannot find free tile for Rome HQ, using fallback (5, 5)\n");
            romePos = GridCoords(5, 5);
        }
        
        // Знайти позицію для Carthage HQ (з мінімальною відстанню)
        GridCoords carthagePos = findRandomFreeTileWithDistance(romePos, config.minDistanceBetweenHQs);
        if (carthagePos.row == -1) {
            printf("[FactionSpawner] Error: Cannot find free tile for Carthage HQ with min distance, using fallback (45, 45)\n");
            carthagePos = GridCoords(45, 45);
        }
        
        // Створити HQ
        createHQ(ROME, romePos);
        createHQ(CARTHAGE, carthagePos);
        
        printf("[FactionSpawner] HQs spawned successfully\n");
    }
    
private:
    // Знайти випадковий вільний тайл
    GridCoords findRandomFreeTile() const {
        // HQ має footprint 3x3, тому потрібно залишити місце
        const int HQ_FOOTPRINT = 3;
        int maxRow = map->getHeight() - HQ_FOOTPRINT;
        int maxCol = map->getWidth() - HQ_FOOTPRINT;
        
        for (int attempt = 0; attempt < config.maxSpawnAttempts; attempt++) {
            int row = rand() % maxRow;
            int col = rand() % maxCol;
            
            // Перевіряємо що всі тайли footprint прохідні та вільні
            bool allTilesFree = true;
            for (int r = 0; r < HQ_FOOTPRINT && allTilesFree; r++) {
                for (int c = 0; c < HQ_FOOTPRINT && allTilesFree; c++) {
                    if (!map->isPassable(row + r, col + c) || !placer->isTileFree(row + r, col + c)) {
                        allTilesFree = false;
                    }
                }
            }
            
            if (allTilesFree) {
                return GridCoords(row, col);
            }
        }
        
        return GridCoords(-1, -1); // Не знайдено
    }
    
    // Знайти випадковий вільний тайл з мінімальною відстанню від іншої точки
    GridCoords findRandomFreeTileWithDistance(const GridCoords& from, int minDist) const {
        int attempts = 0;
        int reducedMinDist = minDist;
        
        // HQ має footprint 3x3, тому потрібно залишити місце
        const int HQ_FOOTPRINT = 3;
        int maxRow = map->getHeight() - HQ_FOOTPRINT;
        int maxCol = map->getWidth() - HQ_FOOTPRINT;
        
        while (attempts < config.maxSpawnAttempts) {
            int row = rand() % maxRow;
            int col = rand() % maxCol;
            
            // Перевіряємо що всі тайли footprint прохідні та вільні
            bool allTilesFree = true;
            for (int r = 0; r < HQ_FOOTPRINT && allTilesFree; r++) {
                for (int c = 0; c < HQ_FOOTPRINT && allTilesFree; c++) {
                    if (!map->isPassable(row + r, col + c) || !placer->isTileFree(row + r, col + c)) {
                        allTilesFree = false;
                    }
                }
            }
            
            if (allTilesFree && isMinDistanceSatisfied(from, GridCoords(row, col), reducedMinDist)) {
                return GridCoords(row, col);
            }
            
            attempts++;
            
            // Після 50 спроб зменшуємо мінімальну відстань на 50%
            if (attempts == 50) {
                reducedMinDist = minDist / 2;
                printf("[FactionSpawner] Reducing min distance to %d after 50 attempts\n", reducedMinDist);
            }
        }
        
        return GridCoords(-1, -1); // Не знайдено
    }
    
    // Перевірити мінімальну відстань між HQ
    bool isMinDistanceSatisfied(const GridCoords& pos1, const GridCoords& pos2, int minDist) const {
        int manhattanDist = abs(pos1.row - pos2.row) + abs(pos1.col - pos2.col);
        return manhattanDist >= minDist;
    }
    
    // Створити HQ для фракції
    void createHQ(Faction faction, const GridCoords& position) {
        Building hq;
        BuildingType type = (faction == ROME) ? HQ_ROME : HQ_CARTHAGE;
        hq.init(type, faction, position);
        
        int buildingIndex = buildings->size();
        if (placer->placeBuilding(hq, position.row, position.col, buildingIndex)) {
            buildings->push_back(hq);
            printf("[FactionSpawner] %s HQ spawned at tile (%d, %d) at world pos (%d, %d)\n", 
                   (faction == ROME) ? "Rome" : "Carthage", 
                   position.row, position.col,
                   (*buildings)[buildingIndex].x, (*buildings)[buildingIndex].y);
        } else {
            printf("[FactionSpawner] Error: Failed to place %s HQ at tile (%d, %d)\n", 
                   (faction == ROME) ? "Rome" : "Carthage", 
                   position.row, position.col);
        }
    }
    
    // Отримати позицію останнього створеного HQ (для фокусування камери)
    bool getLastHQPosition(int& worldX, int& worldY) const {
        if (buildings && !buildings->empty()) {
            const Building& lastBuilding = buildings->back();
            worldX = lastBuilding.x;
            worldY = lastBuilding.y;
            return true;
        }
        return false;
    }
};

#endif // FACTION_SPAWNER_H
