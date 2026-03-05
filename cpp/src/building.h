#pragma once
#include "raylib.h"
#include "tilemap/coordinates.h"
#include "isometric_sprite.h"
#include <string>
#include <vector>
#include <cmath>

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
    // НОВИЙ ПІДХІД: Grid coordinates (ізометрична система)
    GridCoords position;         // Поточні координати в сітці
    GridCoords footprint;        // Розмір будівлі в тайлах (rows, cols)
    
    // COMPATIBILITY LAYER: Публічні змінні для старого коду
    // Ці змінні автоматично синхронізуються з grid координатами
    int x, y;                    // Screen coordinates (deprecated, але працюють)
    int tile_row, tile_col;      // Тайлові координати (deprecated)
    
    BuildingType type;           // Тип будівлі
    Faction faction;             // Фракція
    bool selected = false;       // Чи вибрана будівля
    std::string name;            // Назва для відображення
    
    // Isometric sprite system
    IsometricSprite sprite;      // Спрайт будівлі
    bool useDebugRendering;      // Чи використовувати debug режим
    
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
    void init(BuildingType buildingType, Faction buildingFaction, GridCoords startPos) {
        type = buildingType;
        faction = buildingFaction;
        position = startPos;
        useDebugRendering = true; // За замовчуванням використовуємо debug
        
        // Встановлення footprint залежно від типу
        // Всі будівлі мають основу 2x2 тайли
        footprint = {2, 2};
        
        // COMPATIBILITY: Синхронізуємо screen coordinates
        syncScreenCoords();
        
        // Спроба завантажити спрайт
        std::string spritePath = "assets/sprites/isometric/buildings/";
        switch (type) {
            case HQ_ROME:
                spritePath += "Praetorium.png";
                break;
            case HQ_CARTHAGE:
                spritePath += "Skene.png";
                break;
            case BARRACKS_ROME:
                spritePath += "Contubernium.png";
                break;
            case BARRACKS_CARTHAGE:
                spritePath += "LibTent1.png";
                break;
            case QUESTORIUM_ROME:
                spritePath += "Questorium.png";
                break;
            case LIBTENT_1:
                spritePath += "LibTent1.png";
                break;
            case LIBTENT_2:
                spritePath += "LibTent2.png";
                break;
            case LIBTENT_3:
                spritePath += "LibTent3.png";
                break;
            case TENTORIUM:
                spritePath += "Tentorium.png";
                break;
        }
        
        if (sprite.loadFromFile(spritePath.c_str())) {
            useDebugRendering = false;
        } else {
            TraceLog(LOG_INFO, "[BUILDING] Using debug rendering for type %d", type);
        }
        
        // Встановлення назви залежно від типу
        switch (type) {
            case HQ_ROME:
                name = "Praetorium";
                texture_offset = {-192, -128};  // Зміщення для ізометричної проекції
                texture_scale = 1.0f;           // Повний розмір (384x224)
                break;
            case HQ_CARTHAGE:
                name = "Main Tent";
                texture_offset = {-192, -128};  // Зміщення для ізометричної проекції
                texture_scale = 1.0f;
                break;
            case BARRACKS_ROME:
                name = "Contubernium";
                texture_offset = {-192, -128};  // Зміщення для ізометричної проекції
                texture_scale = 1.0f;
                break;
            case BARRACKS_CARTHAGE:
                name = "Mercenary Camp";
                texture_offset = {-192, -128};  // Зміщення для ізометричної проекції
                texture_scale = 1.0f;
                break;
            case QUESTORIUM_ROME:
                name = "Questorium";
                texture_offset = {-192, -128};  // Зміщення для ізометричної проекції
                texture_scale = 1.0f;
                break;
            case LIBTENT_1:
                name = "Libyan Tent I";
                texture_offset = {-192, -128};  // Зміщення для ізометричної проекції
                texture_scale = 1.0f;
                break;
            case LIBTENT_2:
                name = "Libyan Tent II";
                texture_offset = {-192, -128};  // Зміщення для ізометричної проекції
                texture_scale = 1.0f;
                break;
            case LIBTENT_3:
                name = "Libyan Tent III";
                texture_offset = {-192, -128};  // Зміщення для ізометричної проекції
                texture_scale = 1.0f;
                break;
            case TENTORIUM:
                name = "Tentorium";
                texture_offset = {-192, -128};  // Зміщення для ізометричної проекції
                texture_scale = 1.0f;
                break;
        }
        
        use_texture = true;
    }
    
    // COMPATIBILITY: Синхронізація screen coordinates з grid coordinates
    void syncScreenCoords() {
        ScreenCoords screen = CoordinateConverter::gridToScreen(position);
        x = (int)screen.x;
        y = (int)screen.y;
        tile_row = position.row;
        tile_col = position.col;
    }
    
    // Отримати screen позицію (для рендерингу)
    ScreenCoords getScreenPosition() const {
        return CoordinateConverter::gridToScreen(position);
    }
    
    // Отримати grid позицію
    GridCoords getGridPosition() const {
        return position;
    }
    
    // Перевірка чи займає будівля дану grid клітинку
    bool occupiesGridCell(GridCoords cell) const {
        // ВАЖЛИВО: Спрайт будівлі візуально зміщений
        // Область кліку має бути зміщена на -1 row, -1 col
        int offsetRow = -1;
        int offsetCol = -1;
        
        bool occupies = cell.row >= position.row + offsetRow && 
               cell.row < position.row + offsetRow + footprint.row &&
               cell.col >= position.col + offsetCol && 
               cell.col < position.col + offsetCol + footprint.col;
        
        if (occupies) {
            printf("[occupiesGridCell] HIT! Building '%s' at grid(%d,%d) with offset(%d,%d) occupies cell(%d,%d)\n",
                   name.c_str(), position.row, position.col, offsetRow, offsetCol, cell.row, cell.col);
        }
        
        return occupies;
    }
    
    // Отримати прямокутник для колізій з іншими об'єктами
    Rectangle getCollisionRect() const {
        ScreenCoords screenPos = getScreenPosition();
        // Зміщуємо область колізії вгору на 160 пікселів
        float offsetY = -160.0f;
        // Використовуємо розміри тайлів з CoordinateConverter
        float halfWidth = footprint.col * CoordinateConverter::TILE_WIDTH_HALF;
        float halfHeight = footprint.row * CoordinateConverter::TILE_HEIGHT_HALF;
        
        // Прямокутник що приблизно відповідає ромбу
        return {
            screenPos.x - halfWidth, 
            screenPos.y + offsetY - halfHeight, 
            halfWidth * 2, 
            halfHeight * 2
        };
    }
    
    // Отримати прямокутник для кліків (footprint будівлі в ізометричній проекції)
    Rectangle getRect() const {
        ScreenCoords screenPos = getScreenPosition();
        
        // Зміщуємо область кліку вгору на 160 пікселів
        float offsetY = -160.0f;
        
        // Використовуємо константи з CoordinateConverter
        float halfWidth = footprint.col * CoordinateConverter::TILE_WIDTH_HALF;
        float halfHeight = footprint.row * CoordinateConverter::TILE_HEIGHT_HALF;
        
        // Центруємо прямокутник відносно позиції будівлі з offset
        return {
            screenPos.x - halfWidth,
            screenPos.y + offsetY - halfHeight,
            halfWidth * 2,
            halfHeight * 2
        };
    }
    
    // Отримати прямокутник для текстури
    Rectangle getTextureRect() const {
        ScreenCoords screenPos = getScreenPosition();
        return {
            screenPos.x + texture_offset.x,
            screenPos.y + texture_offset.y,
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
        ScreenCoords screenPos = getScreenPosition();
        
        if (useDebugRendering) {
            // Debug режим - ізометричний прямокутник
            Color buildingColor = getColor();
            
            // Якщо вибрана, зробити яскравішою
            if (selected) {
                buildingColor.r = (unsigned char)(buildingColor.r * 1.3f);
                buildingColor.g = (unsigned char)(buildingColor.g * 1.3f);
                buildingColor.b = (unsigned char)(buildingColor.b * 1.3f);
            }
            
            // Малюємо debug прямокутник з урахуванням footprint
            int widthPixels = footprint.col * 64;  // 64 пікселі на тайл по ширині
            int heightPixels = footprint.row * 32; // 32 пікселі на тайл по висоті
            sprite.drawDebugBuilding(screenPos, widthPixels, heightPixels, buildingColor, name.c_str());
        } else {
            // Рендеринг спрайту
            Color tint = WHITE;
            if (selected) {
                // Пульсуюче виділення (яскраве жовте)
                float pulse = (sin(GetTime() * 3.0f) + 1.0f) / 2.0f; // 0.0 до 1.0
                unsigned char brightness = (unsigned char)(200 + pulse * 55); // 200-255
                tint = {brightness, brightness, 150, 255}; // Яскраво-жовтий
            }
            sprite.draw(screenPos, tint);
        }
        
        // Прогрес виробництва (поверх спрайту/debug)
        if (is_producing) {
            float progress = production_progress / production_time;
            int barWidth = 80;
            int barX = (int)screenPos.x - barWidth / 2;
            int barY = (int)screenPos.y + 10; // Трохи нижче центру
            
            DrawRectangle(barX, barY, (int)(barWidth * progress), 5, GREEN);
            DrawRectangleLines(barX, barY, barWidth, 5, WHITE);
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
    
    // Перевірка кліку по ізометричному ромбу
    bool isClicked(Vector2 mousePos) const {
        ScreenCoords screenPos = getScreenPosition();
        
        // Зміщуємо область кліку вгору на 64 пікселів (1 тайл)
        // Це відповідає offset -1 row, -1 col в occupiesGridCell
        float offsetY = -CoordinateConverter::TILE_HEIGHT;  // -64 пікселів
        
        // Переводимо mouse position в локальні координати відносно центру будівлі
        float localX = mousePos.x - screenPos.x;
        float localY = mousePos.y - (screenPos.y + offsetY);
        
        // Для ізометричного ромба використовуємо формулу:
        // |localX / halfWidth| + |localY / halfHeight| <= 1
        // Використовуємо розміри тайлів з CoordinateConverter
        
        float halfWidth = footprint.col * CoordinateConverter::TILE_WIDTH_HALF;
        float halfHeight = footprint.row * CoordinateConverter::TILE_HEIGHT_HALF;
        
        // Перевірка чи точка всередині ромба
        float normalized = fabs(localX / halfWidth) + fabs(localY / halfHeight);
        
        bool result = normalized <= 1.0f;
        
        if (result) {
            printf("[isClicked] HIT! Building '%s' at grid(%d,%d), screen(%.1f,%.1f), mouse(%.1f,%.1f), normalized=%.2f\n",
                   name.c_str(), position.row, position.col, screenPos.x, screenPos.y, mousePos.x, mousePos.y, normalized);
        }
        
        return result;
    }
};