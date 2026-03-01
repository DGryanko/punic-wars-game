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
    HQ_ROME,           // Преторій
    HQ_CARTHAGE,       // Головне шатро
    BARRACKS_ROME,     // Контуберній (казарма)
    BARRACKS_CARTHAGE, // Карфагенська казарма
    QUESTORIUM_ROME,   // Квесторій (склад ресурсів)
    LIBTENT_1,         // Палатка лівійців рівень 1
    LIBTENT_2,         // Палатка лівійців рівень 2
    LIBTENT_3,         // Палатка лівійців рівень 3
    TENTORIUM          // Тенторіум торговця
};

// Структура будівлі
struct Building {
    int x, y;                    // Координати
    int tile_row, tile_col;      // Тайлові координати
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
    
    // Текстури
    bool use_texture = true;        // Чи використовувати текстуру
    Vector2 texture_offset = {0, 0}; // Зміщення текстури відносно позиції
    float texture_scale = 1.0f;     // Масштаб текстури
    
    // Ініціалізація будівлі
    void init(BuildingType buildingType, Faction buildingFaction, int posX, int posY) {
        type = buildingType;
        faction = buildingFaction;
        x = posX;
        y = posY;
        tile_row = 0;
        tile_col = 0;
        
        // Встановлення назви залежно від типу
        switch (type) {
            case HQ_ROME:
                name = "Praetorium";
                texture_offset = {-192, -112};  // Центруємо більшу текстуру
                texture_scale = 1.0f;           // Повний розмір (384x224)
                break;
            case HQ_CARTHAGE:
                name = "Main Tent";
                texture_offset = {-192, -112};
                texture_scale = 1.0f;
                break;
            case BARRACKS_ROME:
                name = "Contubernium";
                texture_offset = {-192, -112};
                texture_scale = 1.0f;
                break;
            case BARRACKS_CARTHAGE:
                name = "Mercenary Camp";
                texture_offset = {-192, -112};
                texture_scale = 1.0f;
                break;
            case QUESTORIUM_ROME:
                name = "Questorium";
                texture_offset = {-192, -112};
                texture_scale = 1.0f;
                break;
            case LIBTENT_1:
                name = "Libyan Tent I";
                texture_offset = {-192, -112};
                texture_scale = 1.0f;
                break;
            case LIBTENT_2:
                name = "Libyan Tent II";
                texture_offset = {-192, -112};
                texture_scale = 1.0f;
                break;
            case LIBTENT_3:
                name = "Libyan Tent III";
                texture_offset = {-192, -112};
                texture_scale = 1.0f;
                break;
            case TENTORIUM:
                name = "Tentorium";
                texture_offset = {-192, -112};
                texture_scale = 1.0f;
                break;
        }
        
        use_texture = true;
    }
    
    // Отримати прямокутник для колізій з іншими об'єктами (маленький)
    Rectangle getCollisionRect() const {
        return {(float)x, (float)y, 80, 60};
    }
    
    // Отримати прямокутник для кліків (враховує текстуру)
    Rectangle getRect() const {
        if (use_texture) {
            // Використовуємо розмір текстури для кліків
            return {
                (float)x + texture_offset.x,
                (float)y + texture_offset.y,
                384 * texture_scale,  // Ширина текстури
                224 * texture_scale   // Висота текстури
            };
        }
        return {(float)x, (float)y, 80, 60};
    }
    
    // Отримати прямокутник для текстури
    Rectangle getTextureRect() const {
        return {
            (float)x + texture_offset.x,
            (float)y + texture_offset.y,
            80 * texture_scale,
            60 * texture_scale
        };
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
    bool startProduction(const std::string& unitType, bool& shouldPayNow) {
        // Перевірка обмежень
        if (type == BARRACKS_ROME && unitType == "legionary" && units_produced >= 8) {
            shouldPayNow = false;
            return false; // Максимум 8 легіонерів на контуберній
        }
        
        if (!is_producing) {
            // Почати виробництво відразу - ПЛАТИМО ЗАРАЗ
            is_producing = true;
            producing_unit = unitType;
            production_progress = 0.0f;
            shouldPayNow = true; // Платимо зараз
            
            // Встановлення часу виробництва (збільшено вдвоє)
            if (unitType == "legionary") {
                production_time = 10.0f; // 10 секунд (було 5)
            } else if (unitType == "phoenician") {
                production_time = 9.0f; // 9 секунд (було 4.5)
            } else if (unitType == "slave") {
                production_time = 6.0f; // 6 секунд (було 3)
            }
        } else {
            // Додати до черги - НЕ ПЛАТИМО ЗАРАЗ
            production_queue.push_back(unitType);
            shouldPayNow = false; // Заплатимо пізніше
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
    void updateProduction(float deltaTime, std::string& startedUnit, std::string& completedUnit) {
        startedUnit = ""; // За замовчуванням нічого не почалося
        completedUnit = ""; // За замовчуванням нічого не завершилось
        
        if (is_producing) {
            production_progress += deltaTime;
            if (production_progress >= production_time) {
                // Виробництво завершено
                completedUnit = producing_unit; // Зберігаємо завершений юніт
                is_producing = false;
                production_progress = 0.0f;
                units_produced++;
                producing_unit = ""; // Очищаємо
                
                // Почати наступний елемент з черги
                if (!production_queue.empty()) {
                    std::string nextUnit = production_queue.front();
                    production_queue.erase(production_queue.begin());
                    
                    // Почати виробництво наступного юніта
                    is_producing = true;
                    producing_unit = nextUnit;
                    production_progress = 0.0f;
                    startedUnit = nextUnit; // Повідомляємо що почався новий юніт
                    
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