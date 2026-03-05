#pragma once
#include "raylib.h"
#include "building.h" // Для Faction enum
#include "tilemap/coordinates.h"
#include "isometric_sprite.h"
#include <string>
#include <cmath>

// Структура юніта
struct Unit {
    // НОВИЙ ПІДХІД: Grid coordinates (ізометрична система)
    GridCoords position;         // Поточні координати в сітці
    GridCoords target_position;  // Цільові координати для руху
    
    // Для плавної інтерполяції між тайлами
    float interpolation_progress; // 0.0 - 1.0
    ScreenCoords current_screen_pos; // Поточна екранна позиція для плавного руху
    
    // COMPATIBILITY LAYER: Публічні змінні для старого коду
    // Ці змінні автоматично синхронізуються з grid координатами
    int x, y;                    // Screen coordinates (deprecated, але працюють)
    int target_x, target_y;      // Target screen coordinates (deprecated)
    int hp, max_hp;              // Здоров'я
    Faction faction;             // Фракція
    std::string unit_type;       // Тип юніта
    bool selected = false;       // Чи вибраний юніт
    bool is_moving = false;      // Чи рухається юніт
    float speed = 0.35f;         // Швидкість руху
    
    // Isometric sprite system
    IsometricSprite sprite;      // Спрайт юніта
    bool useDebugRendering;      // Чи використовувати debug режим
    
    // AI поведінка
    bool is_ai_controlled = false; // Чи керується AI
    float ai_timer = 0.0f;         // Таймер для AI рішень
    float ai_decision_interval = 3.0f; // Інтервал прийняття рішень (секунди)
    
    // Збір ресурсів
    bool can_harvest = false;      // Чи може збирати ресурси
    bool is_harvesting = false;    // Чи зараз збирає ресурси
    int carrying_food = 0;         // Скільки їжі несе
    int carrying_gold = 0;         // Скільки золота несе
    int max_carry_capacity = 20;   // Максимальна вантажопідйомність
    int assigned_resource_x = -1;  // Призначена ресурсна точка X
    int assigned_resource_y = -1;  // Призначена ресурсна точка Y
    int dropoff_building_x = -1;   // Будівля для здачі X
    int dropoff_building_y = -1;   // Будівля для здачі Y
    
    // Бойові параметри
    int attack_damage = 0;         // Урон атаки
    float attack_range = 25.0f;    // Дальність атаки
    float attack_cooldown = 1.0f;  // Час між атаками (секунди)
    float last_attack_time = 0.0f; // Час останньої атаки
    bool is_attacking = false;     // Чи зараз атакує
    int target_unit_id = -1;       // ID цільового юніта для атаки
    
    // Pathfinding поля
    std::vector<Vector2> path;        // Поточний шлях
    int currentWaypointIndex = 0;     // Індекс поточної точки шляху
    float stuckTimer = 0.0f;          // Таймер для виявлення застрягання
    Vector2 lastPosition = {0, 0};    // Остання позиція для виявлення застрягання
    int unstuckAttempts = 0;          // Кількість спроб виходу з застрягання
    bool usePathfinding = true;       // Чи використовувати pathfinding
    
    // Ініціалізація юніта
    void init(std::string type, Faction unitFaction, GridCoords startPos, bool aiControlled = false) {
        unit_type = type;
        faction = unitFaction;
        position = startPos;
        target_position = startPos;
        interpolation_progress = 1.0f; // Початково не рухається
        current_screen_pos = CoordinateConverter::gridToScreen(position);
        is_moving = false;
        is_ai_controlled = aiControlled;
        ai_timer = 0.0f;
        useDebugRendering = true; // За замовчуванням використовуємо debug
        
        // COMPATIBILITY: Синхронізуємо screen coordinates
        syncScreenCoords();
        
        // Спроба завантажити спрайт
        std::string spritePath = "assets/sprites/isometric/units/" + type + "_";
        if (unitFaction == ROME) {
            spritePath += "rome.png";
        } else {
            spritePath += "carthage.png";
        }
        
        if (sprite.loadFromFile(spritePath.c_str())) {
            useDebugRendering = false;
        } else {
            TraceLog(LOG_INFO, "[UNIT] Using debug rendering for %s", type.c_str());
        }
        
        // Встановлення характеристик залежно від типу
        if (type == "legionary") {
            hp = max_hp = 100;
            speed = 0.35f;  // Швидкість руху
            can_harvest = false; // Легіонери не збирають ресурси
            attack_damage = 25;
            attack_range = 30.0f;
            attack_cooldown = 1.5f;
        } else if (type == "phoenician") {
            hp = max_hp = 90;
            speed = 0.35f;  // Швидкість руху
            can_harvest = false; // Фінікійці не збирають ресурси
            attack_damage = 30;
            attack_range = 25.0f;
            attack_cooldown = 1.2f;
        } else if (type == "slave") {
            hp = max_hp = 50;
            speed = 0.25f;  // Раби повільніші
            can_harvest = true; // Раби збирають ресурси
            max_carry_capacity = 15;
            attack_damage = 5; // Слабкі в бою
            attack_range = 15.0f;
            attack_cooldown = 2.0f;
        }
    }
    
    // COMPATIBILITY LAYER: Синхронізація screen coordinates з grid coordinates
    void syncScreenCoords() {
        ScreenCoords screen = CoordinateConverter::gridToScreen(position);
        x = (int)screen.x;
        y = (int)screen.y;
        ScreenCoords targetScreen = CoordinateConverter::gridToScreen(target_position);
        target_x = (int)targetScreen.x;
        target_y = (int)targetScreen.y;
    }
    
    // Отримати екранну позицію з grid координат
    ScreenCoords getScreenPosition() const {
        return current_screen_pos;
    }
    
    // Отримати grid позицію
    GridCoords getGridPosition() const {
        return position;
    }
    
    // Оновлення юніта (рух та AI)
    void update(const std::vector<Building>* buildings = nullptr) {
        float currentTime = GetTime();
        float deltaTime = GetFrameTime();
        
        // AI поведінка
        if (is_ai_controlled) {
            ai_timer += deltaTime;
            if (ai_timer >= ai_decision_interval) {
                ai_timer = 0.0f;
                makeAIDecision();
            }
        }
        
        // Бойова логіка
        if (attack_damage > 0 && !is_harvesting) {
            // Оновлення часу атаки
            if (is_attacking && currentTime - last_attack_time >= attack_cooldown) {
                is_attacking = false;
            }
        }
        
        // Рух між grid позиціями з постійною швидкістю
        if (is_moving && !is_attacking) {
            if (!(position == target_position)) {
                // Зберігаємо стару позицію для відкату при колізії
                ScreenCoords oldScreenPos = current_screen_pos;
                
                // Рухаємося з постійною швидкістю в пікселях за секунду
                ScreenCoords targetScreen = CoordinateConverter::gridToScreen(target_position);
                
                // Обчислюємо відстань до цілі
                float dx = targetScreen.x - current_screen_pos.x;
                float dy = targetScreen.y - current_screen_pos.y;
                float distance = sqrt(dx * dx + dy * dy);
                
                if (distance > 0.1f) {
                    // Нормалізуємо напрямок і рухаємося з постійною швидкістю
                    float moveDistance = speed * deltaTime * 100.0f; // 100 пікселів/сек при speed=1.0
                    
                    if (moveDistance >= distance) {
                        // Досягли цільової позиції
                        position = target_position;
                        current_screen_pos = targetScreen;
                        is_moving = false;
                    } else {
                        // Рухаємося до цілі
                        float ratio = moveDistance / distance;
                        current_screen_pos.x += dx * ratio;
                        current_screen_pos.y += dy * ratio;
                    }
                    
                    // Перевірка колізій з будівлями
                    if (buildings != nullptr) {
                        Rectangle unitRect = {current_screen_pos.x - 8, current_screen_pos.y - 8, 16, 16};
                        bool hasCollision = false;
                        
                        for (const auto& building : *buildings) {
                            if (CheckCollisionRecs(unitRect, building.getCollisionRect())) {
                                hasCollision = true;
                                break;
                            }
                        }
                        
                        // Якщо є колізія, відкочуємо позицію
                        if (hasCollision) {
                            current_screen_pos = oldScreenPos;
                            // Зупиняємо рух - юніт застряг
                            is_moving = false;
                            printf("[COLLISION] Unit stopped due to building collision\n");
                        }
                    }
                } else {
                    // Вже на місці
                    position = target_position;
                    current_screen_pos = targetScreen;
                    is_moving = false;
                }
            } else {
                is_moving = false;
            }
        }
        
        // COMPATIBILITY: Синхронізуємо screen coordinates після руху
        syncScreenCoords();
    }
    
    // Атакувати ціль
    bool attackTarget(Unit& target) {
        float currentTime = GetTime();
        if (attack_damage > 0 && currentTime - last_attack_time >= attack_cooldown) {
            ScreenCoords myPos = getScreenPosition();
            ScreenCoords targetPos = target.getScreenPosition();
            float distance = sqrt(pow(myPos.x - targetPos.x, 2) + pow(myPos.y - targetPos.y, 2));
            if (distance <= attack_range) {
                target.takeDamage(attack_damage);
                last_attack_time = currentTime;
                is_attacking = true;
                return true;
            }
        }
        return false;
    }
    
    // Отримати урон
    void takeDamage(int damage) {
        hp -= damage;
        if (hp < 0) hp = 0;
    }
    
    // Перевірити, чи юніт мертвий
    bool isDead() const {
        return hp <= 0;
    }
    
    // AI прийняття рішень
    void makeAIDecision() {
        if (!is_moving) {
            // Простий AI: рухатися в випадковому напрямку в grid координатах
            // Карта 80x80 тайлів
            int random_row = rand() % 80;
            int random_col = rand() % 80;
            moveTo(GridCoords(random_row, random_col));
        }
    }
    
    // Встановити ціль для руху (grid coordinates)
    void moveTo(GridCoords newPos) {
        // Перевірка меж мапи (80x80)
        const int MAP_SIZE = 80;
        if (newPos.row < 0 || newPos.row >= MAP_SIZE || 
            newPos.col < 0 || newPos.col >= MAP_SIZE) {
            printf("[MOVETO] Target position (%d,%d) is out of map bounds (0-79), ignoring\n", 
                   newPos.row, newPos.col);
            return;
        }
        
        // ВАЖЛИВО: Не перевіряємо колізії тут, бо будівлі займають багато клітинок
        // Pathfinding система має обходити будівлі
        
        target_position = newPos;
        interpolation_progress = 0.0f;
        is_moving = true;
        is_harvesting = false; // ВАЖЛИВО: припинити збір при русі
        syncScreenCoords(); // COMPATIBILITY: Оновити screen coords
        printf("[MOVETO] Unit at (%d,%d) moving to (%d,%d), is_moving=true\n", 
               position.row, position.col, newPos.row, newPos.col);
    }
    
    // COMPATIBILITY: Встановити ціль для руху (screen coordinates - старий API)
    void moveTo(int screen_x, int screen_y) {
        GridCoords gridPos = CoordinateConverter::screenToGrid(ScreenCoords(screen_x, screen_y));
        moveTo(gridPos);
    }
    
    // Встановити ціль для руху (screen coordinates - конвертує в grid)
    void moveToScreen(ScreenCoords screenPos) {
        GridCoords gridPos = CoordinateConverter::screenToGrid(screenPos);
        moveTo(gridPos);
    }
    
    // Встановити шлях для pathfinding (simplified - disabled for now)
    void setPath(const std::vector<Vector2>& newPath) {
        // TODO: Update pathfinding to work with GridCoords
        // For now, pathfinding is disabled
        path.clear();
        // NOTE: Pathfinding logs temporarily disabled
        // printf("[PATHFINDING] Pathfinding temporarily disabled during grid coord migration\n");
    }
    
    // Перевірити чи є шлях
    bool hasPath() const {
        return false; // Disabled during migration
    }
    
    // Очистити шлях
    void clearPath() {
        path.clear();
        currentWaypointIndex = 0;
        is_moving = false;
    }
    
    // Слідувати по шляху
    void followPath(float deltaTime) {
        if (!hasPath()) {
            return;
        }
        
        // Отримуємо поточну точку шляху
        Vector2 waypoint = path[currentWaypointIndex];
        
        // Обчислюємо відстань до точки
        float dx = waypoint.x - (float)x;
        float dy = waypoint.y - (float)y;
        float distance = sqrt(dx * dx + dy * dy);
        
        // Якщо досягли точки, переходимо до наступної
        if (distance < speed + 2.0f) {
            currentWaypointIndex++;
            
            if (currentWaypointIndex >= path.size()) {
                // Досягли кінця шляху
                moveTo((int)waypoint.x, (int)waypoint.y); // COMPATIBILITY: використовуємо screen coords
                is_moving = false;
                clearPath();
                return;
            }
            
            // Оновлюємо ціль до наступної точки
            waypoint = path[currentWaypointIndex];
            moveTo((int)waypoint.x, (int)waypoint.y); // COMPATIBILITY
        }
        
        // Рух відбувається через update() з інтерполяцією
    }
    
    // Перевірити чи юніт застряг
    void checkIfStuck(float deltaTime) {
        if (!is_moving || !usePathfinding) {
            return;
        }
        
        // Перевіряємо чи змінилася позиція
        float dx = (float)x - lastPosition.x;
        float dy = (float)y - lastPosition.y;
        float distMoved = sqrt(dx * dx + dy * dy);
        
        if (distMoved < 0.5f) {
            // Не рухається
            stuckTimer += deltaTime;
            
            if (stuckTimer > 1.0f) {  // Зменшено з 2.0f до 1.0f
                // Застряг!
                printf("[STUCK] Unit at (%d,%d) stuck for %.1f seconds\n", x, y, stuckTimer);
                tryUnstuck();
                stuckTimer = 0.0f;
            }
        } else {
            // Рухається нормально
            stuckTimer = 0.0f;
            unstuckAttempts = 0;  // Скидаємо лічильник спроб
            lastPosition = {(float)x, (float)y};
        }
    }
    
    // Спробувати вийти з застряглої ситуації
    void tryUnstuck() {
        unstuckAttempts++;
        
        printf("[UNSTUCK] Unit at (%d,%d) stuck! Attempt #%d\n", x, y, unstuckAttempts);
        
        if (unstuckAttempts >= 2) {
            // Після 2 спроб - очищаємо шлях, щоб перерахувати через pathfinding
            printf("[UNSTUCK] Unit at (%d,%d) clearing path after %d attempts - will request new path\n", 
                   x, y, unstuckAttempts);
            clearPath();
            unstuckAttempts = 0;
            stuckTimer = 0.0f;
            // Встановлюємо is_moving = true, щоб гравець міг клікнути знову
            is_moving = false;
            return;
        }
        
        // Перша спроба - рухнутися в випадковому напрямку
        const int offsets[] = {-24, -16, 0, 16, 24};
        int offsetX = offsets[rand() % 5];
        int offsetY = offsets[rand() % 5];
        
        // Не рухатися на місці
        if (offsetX == 0 && offsetY == 0) {
            offsetX = 16;
        }
        
        // COMPATIBILITY: Рухаємо через moveTo для синхронізації
        moveTo(x + offsetX, y + offsetY);
        
        printf("[UNSTUCK] Unit moved by (%d,%d) to (%d,%d)\n", offsetX, offsetY, x, y);
    }
    
    // Призначити ресурсну точку для збору
    void assignResource(GridCoords resourcePos, GridCoords buildingPos) {
        // Конвертуємо в screen coords для compatibility з існуючим кодом
        ScreenCoords resScreen = CoordinateConverter::gridToScreen(resourcePos);
        ScreenCoords buildScreen = CoordinateConverter::gridToScreen(buildingPos);
        assigned_resource_x = (int)resScreen.x;
        assigned_resource_y = (int)resScreen.y;
        dropoff_building_x = (int)buildScreen.x;
        dropoff_building_y = (int)buildScreen.y;
        moveTo(resourcePos);
        printf("Unit assigned: resource(%d,%d) building(%d,%d)\n", 
               resourcePos.row, resourcePos.col, buildingPos.row, buildingPos.col);
    }
    
    // COMPATIBILITY: Призначити ресурсну точку (screen coordinates - старий API)
    void assignResource(int res_x, int res_y, int build_x, int build_y) {
        assigned_resource_x = res_x;
        assigned_resource_y = res_y;
        dropoff_building_x = build_x;
        dropoff_building_y = build_y;
        moveTo(res_x, res_y);
        printf("Unit assigned (screen): resource(%d,%d) building(%d,%d)\n", 
               res_x, res_y, build_x, build_y);
    }
    
    // Перевірити, чи призначена ресурсна точка
    bool hasAssignedResource() const {
        return assigned_resource_x != -1 && assigned_resource_y != -1;
    }
    
    // Почати збір ресурсів
    void startHarvesting() {
        if (can_harvest) {
            is_harvesting = true;
            is_moving = false;
        }
    }
    
    // Припинити збір ресурсів
    void stopHarvesting() {
        is_harvesting = false;
    }
    
    // Перевірити, чи може нести більше ресурсів
    bool canCarryMore() const {
        return (carrying_food + carrying_gold) < max_carry_capacity;
    }
    
    // Додати ресурси до інвентарю
    void addResources(int food, int gold) {
        int totalSpace = max_carry_capacity - (carrying_food + carrying_gold);
        int totalToAdd = food + gold;
        
        if (totalToAdd <= totalSpace) {
            carrying_food += food;
            carrying_gold += gold;
        } else {
            // Додаємо пропорційно
            float ratio = (float)totalSpace / totalToAdd;
            carrying_food += (int)(food * ratio);
            carrying_gold += (int)(gold * ratio);
        }
        
        // Якщо повний, зупинити збір
        if (!canCarryMore()) {
            stopHarvesting();
        }
    }
    
    // Скинути ресурси (при доставці до бази)
    void dropResources(int& food_out, int& gold_out) {
        food_out = carrying_food;
        gold_out = carrying_gold;
        carrying_food = 0;
        carrying_gold = 0;
    }
    
    // Отримати прямокутник для колізій
    Rectangle getRect() const {
        ScreenCoords screenPos = getScreenPosition();
        return {screenPos.x - 16, screenPos.y - 16, 32, 32};
    }
    
    // Отримати колір залежно від фракції
    Color getColor() const {
        Color baseColor;
        switch (faction) {
            case ROME:
                baseColor = MAROON;
                break;
            case CARTHAGE:
                baseColor = DARKBLUE;
                break;
            default:
                baseColor = GRAY;
                break;
        }
        
        // Якщо вибраний, зробити яскравішим
        if (selected) {
            baseColor.r = (unsigned char)(baseColor.r * 1.5f);
            baseColor.g = (unsigned char)(baseColor.g * 1.5f);
            baseColor.b = (unsigned char)(baseColor.b * 1.5f);
        }
        
        return baseColor;
    }
    
    // Малювання юніта
    void draw() const {
        ScreenCoords screenPos = getScreenPosition();
        
        // Вибір кольору залежно від фракції
        Color unitColor;
        const char* label = nullptr;
        
        if (unit_type == "legionary") {
            unitColor = RED;
            label = "LEG";
        } else if (unit_type == "phoenician") {
            unitColor = BLUE;
            label = "PHO";
        } else if (unit_type == "slave") {
            unitColor = YELLOW;
            label = "SLV";
        } else {
            unitColor = GRAY;
            label = "???";
        }
        
        // Якщо вибраний, зробити яскравішим
        if (selected) {
            unitColor.r = (unsigned char)(unitColor.r * 1.3f);
            unitColor.g = (unsigned char)(unitColor.g * 1.3f);
            unitColor.b = (unsigned char)(unitColor.b * 1.3f);
        }
        
        // Малювання спрайта або debug shape
        if (useDebugRendering || !sprite.isLoaded()) {
            IsometricSprite::drawDebugUnit(screenPos, unitColor, label);
        } else {
            sprite.draw(screenPos, WHITE);
        }
        
        // Індикатор здоров'я
        if (hp < max_hp) {
            float healthPercent = (float)hp / max_hp;
            DrawRectangle((int)screenPos.x - 16, (int)screenPos.y - 20, 32, 3, RED);
            DrawRectangle((int)screenPos.x - 16, (int)screenPos.y - 20, (int)(32 * healthPercent), 3, GREEN);
        }
        
        // Індикатор ресурсів для збирачів
        if (can_harvest && (carrying_food > 0 || carrying_gold > 0)) {
            DrawRectangle((int)screenPos.x - 16, (int)screenPos.y + 10, 32, 3, DARKGRAY);
            float carryPercent = (float)(carrying_food + carrying_gold) / max_carry_capacity;
            DrawRectangle((int)screenPos.x - 16, (int)screenPos.y + 10, (int)(32 * carryPercent), 3, YELLOW);
        }
        
        // Індикатор збору ресурсів
        if (is_harvesting) {
            DrawCircleLines((int)screenPos.x, (int)screenPos.y, 20, YELLOW);
        }
        
        // Якщо рухається, показати ціль
        if (is_moving) {
            ScreenCoords targetScreen = CoordinateConverter::gridToScreen(target_position);
            DrawLine((int)screenPos.x, (int)screenPos.y, (int)targetScreen.x, (int)targetScreen.y, YELLOW);
            DrawCircleLines((int)targetScreen.x, (int)targetScreen.y, 8, YELLOW);
        }
        
        // Рамка виділення
        if (selected) {
            DrawCircleLines((int)screenPos.x, (int)screenPos.y, 18, YELLOW);
        }
    }
    
    // Перевірка кліку
    bool isClicked(Vector2 mousePos) const {
        ScreenCoords screenPos = getScreenPosition();
        return CheckCollisionPointCircle(mousePos, {screenPos.x, screenPos.y}, 16);
    }
    
    // Отримати назву юніта для відображення
    std::string getDisplayName() const {
        if (unit_type == "legionary") {
            return "Legionary";
        } else if (unit_type == "phoenician") {
            return "Phoenician";
        } else if (unit_type == "slave") {
            return "Slave";
        }
        return unit_type;
    }
};