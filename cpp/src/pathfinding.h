#pragma once
#include "raylib.h"
#include "building.h"
#include "tilemap/tilemap.h"
#include "tilemap/coordinates.h"
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

// Forward declaration
class TileMap;

class NavigationGrid {
public:
    NavigationGrid() : tileMap(nullptr), width(0), height(0) {}
    
    // Ініціалізація сітки з TileMap
    void init(const TileMap* map) {
        tileMap = map;
        if (tileMap) {
            width = tileMap->getWidth();
            height = tileMap->getHeight();
        }
    }
    
    // Перевірка чи клітинка прохідна (делегує до TileMap)
    bool isWalkable(int row, int col) const {
        if (!tileMap) return false;
        if (row < 0 || row >= height || col < 0 || col >= width) {
            return false;
        }
        return tileMap->isPassable(row, col);
    }
    
    // Конвертація координат: екран -> сітка (ізометрична)
    GridCoords worldToGrid(const ScreenCoords& screen) const {
        return CoordinateConverter::screenToGrid(screen);
    }
    
    // Конвертація координат: сітка -> екран (ізометрична)
    ScreenCoords gridToWorld(const GridCoords& grid) const {
        return CoordinateConverter::gridToScreen(grid);
    }
    
    // Позначити клітинку як перешкоду (тепер не використовується, бо TileMap керує прохідністю)
    void markObstacle(int row, int col, bool blocked) {
        // Deprecated: TileMap тепер керує прохідністю
        // Залишаємо для сумісності, але нічого не робимо
    }
    
    // Оновити сітку з будівель (використовує GridCoords)
    void updateFromBuildings(const std::vector<Building>& buildings) {
        // Примітка: Тепер будівлі мають GridCoords позиції
        // TileMap керує прохідністю, тому ця функція може бути спрощена
        // або видалена в майбутньому. Поки залишаємо для сумісності.
        
        // TODO: Можливо потрібно оновлювати TileMap напряму,
        // позначаючи тайли під будівлями як непрохідні
    }
    
    // Отримати розміри сітки (в тайлах)
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    
    // Отримати сусідів для A* (8 напрямків)
    std::vector<GridNode> getNeighbors(int row, int col) const {
        std::vector<GridNode> neighbors;
        
        // 8 напрямків: вгору-ліво, вгору, вгору-право, ліво, право, вниз-ліво, вниз, вниз-право
        const int dx[] = {-1, 0, 1, -1, 1, -1, 0, 1};
        const int dy[] = {-1, -1, -1, 0, 0, 1, 1, 1};
        
        for (int i = 0; i < 8; i++) {
            int nrow = row + dy[i];
            int ncol = col + dx[i];
            
            // Перевіряємо чи сусід валідний і прохідний
            if (isWalkable(nrow, ncol)) {
                GridNode neighbor(ncol, nrow);  // GridNode використовує (x, y) = (col, row)
                neighbors.push_back(neighbor);
            }
        }
        
        return neighbors;
    }
    
private:
    const TileMap* tileMap;  // Посилання на TileMap для прохідності
    int width, height;       // Розмір сітки в тайлах (кешовано з TileMap)
};

// ============================================================================
// A* PATHFINDING
// ============================================================================

class AStarPathfinder {
public:
    // Знайти шлях від start до goal
    std::vector<Vector2> findPath(Vector2 start, Vector2 goal, const NavigationGrid& grid) {
        // Конвертуємо світові координати в сітку (ізометрична конвертація)
        GridCoords startGrid = grid.worldToGrid(ScreenCoords(start.x, start.y));
        GridCoords goalGrid = grid.worldToGrid(ScreenCoords(goal.x, goal.y));
        
        int startX = startGrid.col;
        int startY = startGrid.row;
        int goalX = goalGrid.col;
        int goalY = goalGrid.row;
        
        // Якщо стартова позиція заблокована, знаходимо найближчу вільну клітинку
        if (!grid.isWalkable(startY, startX)) {
            bool foundFree = false;
            // Шукаємо в радіусі 3 клітинки
            for (int radius = 1; radius <= 3 && !foundFree; radius++) {
                for (int dy = -radius; dy <= radius && !foundFree; dy++) {
                    for (int dx = -radius; dx <= radius && !foundFree; dx++) {
                        int nrow = startY + dy;
                        int ncol = startX + dx;
                        if (grid.isWalkable(nrow, ncol)) {
                            startX = ncol;
                            startY = nrow;
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
        if (!grid.isWalkable(goalY, goalX)) {
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
            // Конвертуємо grid координати в screen координати (ізометрична конвертація)
            GridCoords gridPos(current->y, current->x);  // row, col
            ScreenCoords screenPos = grid.gridToWorld(gridPos);
            path.push_back({screenPos.x, screenPos.y});
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
        GridCoords startGrid = grid.worldToGrid(ScreenCoords(start.x, start.y));
        GridCoords endGrid = grid.worldToGrid(ScreenCoords(end.x, end.y));
        
        int x0 = startGrid.col;
        int y0 = startGrid.row;
        int x1 = endGrid.col;
        int y1 = endGrid.row;
        
        // Алгоритм Брезенхема для лінії
        int dx = abs(x1 - x0);
        int dy = abs(y1 - y0);
        int sx = (x0 < x1) ? 1 : -1;
        int sy = (y0 < y1) ? 1 : -1;
        int err = dx - dy;
        
        int x = x0, y = y0;
        while (true) {
            if (!grid.isWalkable(y, x)) {  // row, col
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
    
    // Ініціалізація з TileMap
    void init(const TileMap* tileMap) {
        grid.init(tileMap);
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
