#pragma once
#include "raylib.h"
#include "building.h"
#include <vector>
#include <queue>
#include <map>
#include <cmath>
#include <algorithm>

// ============================================================================
// A* PATHFINDING STRUCTURES
// ============================================================================

struct GridNode {
    int x, y;           // Координати в сітці
    float g, h, f;      // Вартості для A* (g=від старту, h=до цілі, f=g+h)
    GridNode* parent;   // Батьківський вузол для реконструкції шляху
    
    GridNode() : x(0), y(0), g(0), h(0), f(0), parent(nullptr) {}
    GridNode(int _x, int _y) : x(_x), y(_y), g(0), h(0), f(0), parent(nullptr) {}
};

// Компаратор для priority queue (мінімальна f вартість)
struct CompareNode {
    bool operator()(const GridNode* a, const GridNode* b) const {
        return a->f > b->f; // Більша f має нижчий пріоритет
    }
};

// ============================================================================
// NAVIGATION GRID
// ============================================================================

class NavigationGrid {
public:
    NavigationGrid() : width(0), height(0), cellSize(16) {}
    
    // Ініціалізація сітки
    void init(int mapWidth, int mapHeight, int cellSizePixels = 16) {
        cellSize = cellSizePixels;
        width = (mapWidth + cellSize - 1) / cellSize;
        height = (mapHeight + cellSize - 1) / cellSize;
        cells.resize(width * height, true); // Всі клітинки спочатку прохідні
    }
    
    // Перевірка чи клітинка прохідна
    bool isWalkable(int gridX, int gridY) const {
        if (gridX < 0 || gridX >= width || gridY < 0 || gridY >= height) {
            return false;
        }
        return cells[gridY * width + gridX];
    }
    
    // Конвертація координат: світ -> сітка
    void worldToGrid(int worldX, int worldY, int& gridX, int& gridY) const {
        gridX = worldX / cellSize;
        gridY = worldY / cellSize;
        
        // Bounds checking - обмежуємо координати в межах сітки
        if (gridX < 0) gridX = 0;
        if (gridX >= width) gridX = width - 1;
        if (gridY < 0) gridY = 0;
        if (gridY >= height) gridY = height - 1;
    }
    
    // Конвертація координат: сітка -> світ (центр клітинки)
    void gridToWorld(int gridX, int gridY, int& worldX, int& worldY) const {
        // Bounds checking - обмежуємо координати в межах сітки
        int clampedX = gridX;
        int clampedY = gridY;
        if (clampedX < 0) clampedX = 0;
        if (clampedX >= width) clampedX = width - 1;
        if (clampedY < 0) clampedY = 0;
        if (clampedY >= height) clampedY = height - 1;
        
        worldX = clampedX * cellSize + cellSize / 2;
        worldY = clampedY * cellSize + cellSize / 2;
    }
    
    // Позначити клітинку як перешкоду або вільну
    void markObstacle(int gridX, int gridY, bool blocked) {
        if (gridX >= 0 && gridX < width && gridY >= 0 && gridY < height) {
            cells[gridY * width + gridX] = !blocked;
        }
    }
    
    // Оновити сітку з будівель
    void updateFromBuildings(const std::vector<Building>& buildings) {
        // Спочатку очищаємо всі клітинки
        std::fill(cells.begin(), cells.end(), true);
        
        // Позначаємо будівлі як перешкоди
        for (const auto& building : buildings) {
            Rectangle rect = building.getRect();
            
            // Конвертуємо прямокутник будівлі в клітинки сітки
            int startX, startY, endX, endY;
            worldToGrid((int)rect.x, (int)rect.y, startX, startY);
            worldToGrid((int)(rect.x + rect.width), (int)(rect.y + rect.height), endX, endY);
            
            // Позначаємо всі клітинки в прямокутнику
            for (int y = startY; y <= endY; y++) {
                for (int x = startX; x <= endX; x++) {
                    markObstacle(x, y, true);
                }
            }
        }
    }
    
    // Отримати розміри сітки
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getCellSize() const { return cellSize; }
    
    // Отримати сусідів для A* (8 напрямків)
    std::vector<GridNode> getNeighbors(int gridX, int gridY) const {
        std::vector<GridNode> neighbors;
        
        // 8 напрямків: вгору-ліво, вгору, вгору-право, ліво, право, вниз-ліво, вниз, вниз-право
        const int dx[] = {-1, 0, 1, -1, 1, -1, 0, 1};
        const int dy[] = {-1, -1, -1, 0, 0, 1, 1, 1};
        
        for (int i = 0; i < 8; i++) {
            int nx = gridX + dx[i];
            int ny = gridY + dy[i];
            
            // Перевіряємо чи сусід валідний і прохідний
            if (isWalkable(nx, ny)) {
                GridNode neighbor(nx, ny);
                neighbors.push_back(neighbor);
            }
        }
        
        return neighbors;
    }
    
private:
    int width, height;      // Розмір сітки в клітинках
    int cellSize;           // Розмір клітинки в пікселях
    std::vector<bool> cells; // Масив прохідності клітинок
};

// ============================================================================
// A* PATHFINDING
// ============================================================================

class AStarPathfinder {
public:
    // Знайти шлях від start до goal
    std::vector<Vector2> findPath(Vector2 start, Vector2 goal, const NavigationGrid& grid) {
        // Конвертуємо світові координати в сітку
        int startX, startY, goalX, goalY;
        grid.worldToGrid((int)start.x, (int)start.y, startX, startY);
        grid.worldToGrid((int)goal.x, (int)goal.y, goalX, goalY);
        
        // Якщо стартова позиція заблокована, знаходимо найближчу вільну клітинку
        if (!grid.isWalkable(startX, startY)) {
            bool foundFree = false;
            // Шукаємо в радіусі 3 клітинки
            for (int radius = 1; radius <= 3 && !foundFree; radius++) {
                for (int dy = -radius; dy <= radius && !foundFree; dy++) {
                    for (int dx = -radius; dx <= radius && !foundFree; dx++) {
                        int nx = startX + dx;
                        int ny = startY + dy;
                        if (grid.isWalkable(nx, ny)) {
                            startX = nx;
                            startY = ny;
                            foundFree = true;
                        }
                    }
                }
            }
            
            if (!foundFree) {
                printf("[A*] Cannot find free start position!\n");
                return std::vector<Vector2>();
            }
        }
        
        // Перевіряємо чи ціль досяжна
        if (!grid.isWalkable(goalX, goalY)) {
            printf("[A*] Goal is not walkable!\n");
            return std::vector<Vector2>(); // Порожній шлях
        }
        
        // Ініціалізація
        std::priority_queue<GridNode*, std::vector<GridNode*>, CompareNode> openList;
        std::map<int, GridNode*> allNodes; // Всі створені вузли для очищення пам'яті
        std::map<int, bool> closedList;
        
        // Створюємо стартовий вузол
        GridNode* startNode = new GridNode(startX, startY);
        startNode->g = 0;
        startNode->h = heuristic(startX, startY, goalX, goalY);
        startNode->f = startNode->g + startNode->h;
        
        openList.push(startNode);
        allNodes[startY * grid.getWidth() + startX] = startNode;
        
        GridNode* goalNode = nullptr;
        int nodesExplored = 0;
        const int MAX_NODES = 500; // Обмеження для продуктивності
        
        // Основний цикл A*
        while (!openList.empty() && nodesExplored < MAX_NODES) {
            // Отримуємо вузол з найменшою f
            GridNode* current = openList.top();
            openList.pop();
            
            int currentKey = current->y * grid.getWidth() + current->x;
            
            // Пропускаємо якщо вже оброблений
            if (closedList[currentKey]) {
                continue;
            }
            
            closedList[currentKey] = true;
            nodesExplored++;
            
            // Перевіряємо чи досягли цілі
            if (current->x == goalX && current->y == goalY) {
                goalNode = current;
                break;
            }
            
            // Перевіряємо всіх сусідів (8 напрямків)
            const int dx[] = {-1, 0, 1, -1, 1, -1, 0, 1};
            const int dy[] = {-1, -1, -1, 0, 0, 1, 1, 1};
            const float costs[] = {1.414f, 1.0f, 1.414f, 1.0f, 1.0f, 1.414f, 1.0f, 1.414f}; // Діагональ = √2
            
            for (int i = 0; i < 8; i++) {
                int nx = current->x + dx[i];
                int ny = current->y + dy[i];
                int neighborKey = ny * grid.getWidth() + nx;
                
                // Перевіряємо чи сусід валідний
                if (!grid.isWalkable(nx, ny) || closedList[neighborKey]) {
                    continue;
                }
                
                // Обчислюємо нову вартість g
                float newG = current->g + costs[i];
                
                // Перевіряємо чи вузол вже існує
                GridNode* neighbor = allNodes[neighborKey];
                if (neighbor == nullptr) {
                    // Створюємо новий вузол
                    neighbor = new GridNode(nx, ny);
                    neighbor->g = newG;
                    neighbor->h = heuristic(nx, ny, goalX, goalY);
                    neighbor->f = neighbor->g + neighbor->h;
                    neighbor->parent = current;
                    
                    allNodes[neighborKey] = neighbor;
                    openList.push(neighbor);
                } else if (newG < neighbor->g) {
                    // Знайшли кращий шлях до цього вузла
                    neighbor->g = newG;
                    neighbor->f = neighbor->g + neighbor->h;
                    neighbor->parent = current;
                    openList.push(neighbor); // Додаємо знову з новою вартістю
                }
            }
        }
        
        // Реконструюємо шлях
        std::vector<Vector2> path;
        if (goalNode != nullptr) {
            path = reconstructPath(goalNode, grid);
            path = smoothPath(path, grid);
        }
        
        // Очищаємо пам'ять
        for (auto& pair : allNodes) {
            delete pair.second;
        }
        
        return path;
    }
    
private:
    // Евристика: Manhattan distance
    float heuristic(int x1, int y1, int x2, int y2) const {
        return (float)(abs(x1 - x2) + abs(y1 - y2));
    }
    
    // Реконструювати шлях з вузлів
    std::vector<Vector2> reconstructPath(GridNode* goalNode, const NavigationGrid& grid) const {
        std::vector<Vector2> path;
        GridNode* current = goalNode;
        
        while (current != nullptr) {
            int worldX, worldY;
            grid.gridToWorld(current->x, current->y, worldX, worldY);
            path.push_back({(float)worldX, (float)worldY});
            current = current->parent;
        }
        
        // Реверсуємо шлях (від старту до цілі)
        std::reverse(path.begin(), path.end());
        return path;
    }
    
    // Спрощення шляху (видалення зайвих точок)
    std::vector<Vector2> smoothPath(const std::vector<Vector2>& path, const NavigationGrid& grid) const {
        if (path.size() <= 2) {
            return path;
        }
        
        std::vector<Vector2> smoothed;
        smoothed.push_back(path[0]);
        
        int current = 0;
        while (current < path.size() - 1) {
            int farthest = current + 1;
            
            // Знаходимо найдальшу точку з прямою видимістю
            for (int i = current + 2; i < path.size(); i++) {
                if (hasLineOfSight(path[current], path[i], grid)) {
                    farthest = i;
                }
            }
            
            smoothed.push_back(path[farthest]);
            current = farthest;
        }
        
        return smoothed;
    }
    
    // Перевірка прямої видимості між двома точками
    bool hasLineOfSight(Vector2 start, Vector2 end, const NavigationGrid& grid) const {
        int x0, y0, x1, y1;
        grid.worldToGrid((int)start.x, (int)start.y, x0, y0);
        grid.worldToGrid((int)end.x, (int)end.y, x1, y1);
        
        // Алгоритм Брезенхема для лінії
        int dx = abs(x1 - x0);
        int dy = abs(y1 - y0);
        int sx = (x0 < x1) ? 1 : -1;
        int sy = (y0 < y1) ? 1 : -1;
        int err = dx - dy;
        
        int x = x0, y = y0;
        while (true) {
            if (!grid.isWalkable(x, y)) {
                return false;
            }
            
            if (x == x1 && y == y1) {
                break;
            }
            
            int e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x += sx;
            }
            if (e2 < dx) {
                err += dx;
                y += sy;
            }
        }
        
        return true;
    }
};

// ============================================================================
// PATHFINDING MANAGER
// ============================================================================

struct PathRequest {
    int unitId;
    Vector2 start;
    Vector2 goal;
    float priority;
    
    PathRequest(int id, Vector2 s, Vector2 g, float p = 1.0f) 
        : unitId(id), start(s), goal(g), priority(p) {}
};

struct PathData {
    std::vector<Vector2> waypoints;
    float timestamp;
    bool valid;
    
    PathData() : timestamp(0), valid(false) {}
    PathData(const std::vector<Vector2>& wp, float time) 
        : waypoints(wp), timestamp(time), valid(true) {}
};

class PathfindingManager {
public:
    PathfindingManager() : maxCalculationsPerFrame(5) {}
    
    // Ініціалізація
    void init(int mapWidth, int mapHeight) {
        grid.init(mapWidth, mapHeight, 16);
    }
    
    // Запит на обчислення шляху
    void requestPath(int unitId, Vector2 start, Vector2 goal, float priority = 1.0f) {
        requestQueue.push(PathRequest(unitId, start, goal, priority));
    }
    
    // Оновлення (викликається кожен кадр)
    void update(float deltaTime) {
        int calculationsThisFrame = 0;
        
        while (!requestQueue.empty() && calculationsThisFrame < maxCalculationsPerFrame) {
            PathRequest request = requestQueue.front();
            requestQueue.pop();
            
            // Обчислюємо шлях
            std::vector<Vector2> path = pathfinder.findPath(request.start, request.goal, grid);
            
            // Зберігаємо в кеш
            pathCache[request.unitId] = PathData(path, GetTime());
            
            calculationsThisFrame++;
        }
        
        // Очищаємо старі шляхи (>10 секунд)
        float currentTime = GetTime();
        for (auto it = pathCache.begin(); it != pathCache.end(); ) {
            if (currentTime - it->second.timestamp > 10.0f) {
                it = pathCache.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    // Отримати обчислений шлях
    bool getPath(int unitId, std::vector<Vector2>& outPath) {
        auto it = pathCache.find(unitId);
        if (it != pathCache.end() && it->second.valid) {
            outPath = it->second.waypoints;
            return true;
        }
        return false;
    }
    
    // Оновити навігаційну сітку
    void updateGrid(const std::vector<Building>& buildings) {
        grid.updateFromBuildings(buildings);
        
        // Інвалідуємо всі кешовані шляхи
        for (auto& pair : pathCache) {
            pair.second.valid = false;
        }
    }
    
    // Інвалідувати шляхи в області
    void invalidatePathsInArea(Rectangle area) {
        // Спрощена версія: інвалідуємо всі шляхи
        // TODO: перевіряти чи шлях перетинається з областю
        for (auto& pair : pathCache) {
            pair.second.valid = false;
        }
    }
    
    // Отримати сітку (для дебагу)
    const NavigationGrid& getGrid() const { return grid; }
    
private:
    NavigationGrid grid;
    AStarPathfinder pathfinder;
    std::queue<PathRequest> requestQueue;
    std::map<int, PathData> pathCache;
    int maxCalculationsPerFrame;
};
