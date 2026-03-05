#pragma once
#include "unit.h"
#include "building.h"
#include "resource.h"
#include <vector>
#include <algorithm>

// Структура для об'єкта в черзі рендерингу
struct RenderableObject {
    enum Type { UNIT, BUILDING, RESOURCE };
    
    Type type;           // Тип об'єкта
    int index;           // Індекс в відповідному векторі
    int sortKey;         // Ключ для сортування (row + col)
    
    RenderableObject(Type t, int idx, int key) 
        : type(t), index(idx), sortKey(key) {}
};

// Клас для управління чергою рендерингу (Painter's Algorithm)
class RenderQueue {
private:
    std::vector<RenderableObject> queue;
    
public:
    // Очистити чергу
    void clear() {
        queue.clear();
    }
    
    // Додати юніт до черги
    void addUnit(int index, GridCoords pos, ScreenCoords screenPos) {
        // Використовуємо поточну екранну позицію для точного depth sorting
        // Конвертуємо екранні координати назад в grid для sortKey
        GridCoords currentGrid = CoordinateConverter::screenToGrid(screenPos);
        int sortKey = currentGrid.row + currentGrid.col;
        queue.push_back(RenderableObject(RenderableObject::UNIT, index, sortKey));
    }
    
    // Додати будівлю до черги
    void addBuilding(int index, GridCoords pos, GridCoords footprint) {
        // Для будівель використовуємо верхній лівий кут (базову позицію) для сортування
        // Це забезпечує правильний depth sorting в ізометричній проекції
        int sortKey = pos.row + pos.col;
        queue.push_back(RenderableObject(RenderableObject::BUILDING, index, sortKey));
    }
    
    // Додати ресурс до черги
    void addResource(int index, GridCoords pos) {
        int sortKey = pos.row + pos.col;
        queue.push_back(RenderableObject(RenderableObject::RESOURCE, index, sortKey));
    }
    
    // Сортувати чергу (back-to-front)
    void sort() {
        std::sort(queue.begin(), queue.end(), 
            [](const RenderableObject& a, const RenderableObject& b) {
                return a.sortKey < b.sortKey; // Менший sortKey = далі = малюємо першим
            });
    }
    
    // Відрендерити всі об'єкти в правильному порядку
    void render(const std::vector<Unit>& units,
                const std::vector<Building>& buildings,
                const std::vector<ResourcePoint>& resources) {
        for (const auto& obj : queue) {
            switch (obj.type) {
                case RenderableObject::UNIT:
                    if (obj.index >= 0 && obj.index < units.size()) {
                        units[obj.index].draw();
                    }
                    break;
                    
                case RenderableObject::BUILDING:
                    if (obj.index >= 0 && obj.index < buildings.size()) {
                        buildings[obj.index].draw();
                    }
                    break;
                    
                case RenderableObject::RESOURCE:
                    if (obj.index >= 0 && obj.index < resources.size()) {
                        resources[obj.index].draw();
                    }
                    break;
            }
        }
    }
    
    // Отримати розмір черги (для debug)
    int size() const {
        return queue.size();
    }
};
