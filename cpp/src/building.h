#pragma once
#include "raylib.h"
#include <string>
#include <vector>

// Фракції
enum Faction {
    ROME,
    CARTHAGE
};

// Типи будівель (мінімальний набір для MVP)
enum BuildingType {
    HQ_ROME,        // Преторій
    HQ_CARTHAGE,    // Головне шатро
    BARRACKS_ROME,  // Контуберній (казарма)
    BARRACKS_CARTHAGE, // Карфагенська казарма
    QUESTORIUM_ROME    // Квесторій (склад ресурсів)
};

// Структура будівлі
struct Building {
    int x, y;                    // Координати
    BuildingType type;           // Тип будівлі
    Faction faction;             // Фракція
    bool selected = false;       // Чи вибрана будівля
    std::string name;            // Назва для відображення
    
    // Система виробництва
    bool is_producing = false;   // Чи виробляє юніт
    float production_time = 0.0f; // Час виробництва (секунди)
    float production_progress = 0.0f; // Прогрес виробництва
    std::string producing_unit = ""; // Тип юніта, що виробляється
    int units_produced = 0;      // Кількість вироблених юнітів (для обмежень)
    
    // Черга виробництва
    std::vector<std::string> production_queue; // Черга юнітів для виробництва
    
    // Ініціалізація будівлі
    void init(BuildingType buildingType, Faction buildingFaction, int posX, int posY) {
        type = buildingType;
        faction = buildingFaction;
        x = posX;
        y = posY;
        
        // Встановлення назви залежно від типу
        switch (type) {
            case HQ_ROME:
                name = "Praetorium";
                break;
            case HQ_CARTHAGE:
                name = "Main Tent";
                break;
            case BARRACKS_ROME:
                name = "Contubernium";
                break;
            case BARRACKS_CARTHAGE:
                name = "Mercenary Camp";
                break;
            case QUESTORIUM_ROME:
                name = "Questorium";
                break;
        }
    }
    
    // Отримати прямокутник для колізій
    Rectangle getRect() const {
        return {(float)x, (float)y, 80, 60};
    }
    
    // Отримати колір залежно від фракції
    Color getColor() const {
        switch (faction) {
            case ROME:
                return RED;
            case CARTHAGE:
                return BLUE;
            default:
                return GRAY;
        }
    }
    
    // Малювання будівлі
    void draw() const {
        Color buildingColor = getColor();
        
        // Якщо вибрана, зробити яскравішою
        if (selected) {
            buildingColor.r = (unsigned char)(buildingColor.r * 1.3f);
            buildingColor.g = (unsigned char)(buildingColor.g * 1.3f);
            buildingColor.b = (unsigned char)(buildingColor.b * 1.3f);
        }
        
        // Малювання будівлі
        DrawRectangle(x, y, 80, 60, buildingColor);
        
        // Рамка
        DrawRectangleLines(x, y, 80, 60, selected ? YELLOW : WHITE);
        
        // Назва будівлі
        DrawText(name.c_str(), x + 5, y + 25, 10, WHITE);
        
        // Прогрес виробництва
        if (is_producing) {
            float progress = production_progress / production_time;
            DrawRectangle(x, y + 65, (int)(80 * progress), 5, GREEN);
            DrawRectangleLines(x, y + 65, 80, 5, WHITE);
        }
    }
    
    // Почати виробництво юніта або додати до черги
    bool startProduction(const std::string& unitType) {
        // Перевірка обмежень
        if (type == BARRACKS_ROME && unitType == "legionary" && units_produced >= 8) {
            return false; // Максимум 8 легіонерів на контуберній
        }
        
        if (!is_producing) {
            // Почати виробництво відразу
            is_producing = true;
            producing_unit = unitType;
            production_progress = 0.0f;
            
            // Встановлення часу виробництва (збільшено вдвоє)
            if (unitType == "legionary") {
                production_time = 10.0f; // 10 секунд (було 5)
            } else if (unitType == "phoenician") {
                production_time = 9.0f; // 9 секунд (було 4.5)
            } else if (unitType == "slave") {
                production_time = 6.0f; // 6 секунд (було 3)
            }
        } else {
            // Додати до черги
            production_queue.push_back(unitType);
        }
        
        return true;
    }
    
    // Скасувати останній елемент черги
    bool cancelLastInQueue() {
        if (!production_queue.empty()) {
            production_queue.pop_back();
            return true;
        }
        return false;
    }
    
    // Отримати розмір черги
    int getQueueSize() const {
        return production_queue.size();
    }
    
    // Оновлення виробництва
    void updateProduction(float deltaTime) {
        if (is_producing) {
            production_progress += deltaTime;
            if (production_progress >= production_time) {
                // Виробництво завершено
                is_producing = false;
                production_progress = 0.0f;
                units_produced++;
                
                // Почати наступний елемент з черги
                if (!production_queue.empty()) {
                    std::string nextUnit = production_queue.front();
                    production_queue.erase(production_queue.begin());
                    
                    // Почати виробництво наступного юніта
                    is_producing = true;
                    producing_unit = nextUnit;
                    production_progress = 0.0f;
                    
                    if (nextUnit == "legionary") {
                        production_time = 10.0f;
                    } else if (nextUnit == "phoenician") {
                        production_time = 9.0f;
                    } else if (nextUnit == "slave") {
                        production_time = 6.0f;
                    }
                }
            }
        }
    }
    
    // Перевірка, чи завершено виробництво
    bool isProductionComplete() const {
        return !is_producing && production_progress == 0.0f && !producing_unit.empty();
    }
    
    // Отримати тип вироблюваного юніта та скинути стан
    std::string getProducedUnit() {
        std::string unit = producing_unit;
        producing_unit = "";
        return unit;
    }
    
    // Перевірка кліку
    bool isClicked(Vector2 mousePos) const {
        return CheckCollisionPointRec(mousePos, getRect());
    }
};