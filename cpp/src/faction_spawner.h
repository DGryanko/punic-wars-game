#ifndef FACTION_SPAWNER_H
#define FACTION_SPAWNER_H

#include "building_placer.h"
#include "tilemap/tilemap.h"
#include "building.h"
#include "game_constants.h"
#include "debug_logger.h"
#include <vector>
#include <cstdlib>
#include <cmath>

// Конфігурація спавну
struct SpawnConfiguration {
    int minDistanceBetweenHQs = GameConstants::Spawn::MIN_HQ_DISTANCE;
    int maxSpawnAttempts = GameConstants::Spawn::MAX_SPAWN_ATTEMPTS;
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
    
    // Спавн HQ обох фракцій (старий метод — залишено для сумісності)
    void spawnFactionHQs() {
        spawnEnemyHQ(ROME);   // fallback: спавн обох
        spawnEnemyHQ(CARTHAGE);
    }

    // Спавн тільки ворожого HQ (гравець будує свій сам)
    void spawnEnemyHQ(Faction playerFaction) {
        if (!placer || !map || !buildings) {
            LOG_ERROR("[FactionSpawner] Error: Not initialized!\n");
            return;
        }

        Faction enemyFaction = (playerFaction == ROME) ? CARTHAGE : ROME;

        LOG_SPAWN("[FactionSpawner] Spawning enemy HQ for faction %d...\n", enemyFaction);

        GridCoords enemyPos = findRandomFreeTile();
        if (enemyPos.row == -1) {
            LOG_ERROR("[FactionSpawner] Cannot find free tile for enemy HQ, using fallback\n");
            enemyPos = (enemyFaction == ROME) ? GridCoords(5, 5) : GridCoords(60, 60);
        }

        createHQ(enemyFaction, enemyPos);
        LOG_SPAWN("[FactionSpawner] Enemy HQ spawned at (%d,%d)\n", enemyPos.row, enemyPos.col);
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
            if (attempts == GameConstants::Spawn::REDUCE_DISTANCE_AFTER) {
                reducedMinDist = minDist / 2;
                LOG_SPAWN("[FactionSpawner] Reducing min distance to %d after %d attempts\n", 
                          reducedMinDist, GameConstants::Spawn::REDUCE_DISTANCE_AFTER);
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
            LOG_SPAWN("[FactionSpawner] %s HQ spawned at tile (%d, %d) at world pos (%d, %d)\n", 
                      (faction == ROME) ? "Rome" : "Carthage", 
                      position.row, position.col,
                      (*buildings)[buildingIndex].x, (*buildings)[buildingIndex].y);
        } else {
            LOG_ERROR("[FactionSpawner] Error: Failed to place %s HQ at tile (%d, %d)\n", 
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
