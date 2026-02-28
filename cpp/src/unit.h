#pragma once
#include "raylib.h"
#include "building.h" // Для Faction enum
#include <string>
#include <cmath>

// Структура юніта
struct Unit {
    int x, y;                    // Поточні координати
    int target_x, target_y;      // Цільові координати для руху
    int hp, max_hp;              // Здоров'я
    Faction faction;             // Фракція
    std::string unit_type;       // Тип юніта
    bool selected = false;       // Чи вибраний юніт
    bool is_moving = false;      // Чи рухається юніт
    float speed = 0.85f;         // Швидкість руху (зменшено вдвоє з 1.7f)
    
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
    void init(std::string type, Faction unitFaction, int posX, int posY, bool aiControlled = false) {
        unit_type = type;
        faction = unitFaction;
        x = posX;
        y = posY;
        target_x = posX;
        target_y = posY;
        is_moving = false;
        is_ai_controlled = aiControlled;
        ai_timer = 0.0f;
        
        // Встановлення характеристик залежно від типу
        if (type == "legionary") {
            hp = max_hp = 100;
            speed = 0.85f;
            can_harvest = false; // Легіонери не збирають ресурси
            attack_damage = 25;
            attack_range = 30.0f;
            attack_cooldown = 1.5f;
        } else if (type == "phoenician") {
            hp = max_hp = 90;
            speed = 0.85f;
            can_harvest = false; // Фінікійці не збирають ресурси
            attack_damage = 30;
            attack_range = 25.0f;
            attack_cooldown = 1.2f;
        } else if (type == "slave") {
            hp = max_hp = 50;
            speed = 0.6f; // Раби повільніші (зменшено вдвоє з 1.2f)
            can_harvest = true; // Раби збирають ресурси
            max_carry_capacity = 15;
            attack_damage = 5; // Слабкі в бою
            attack_range = 15.0f;
            attack_cooldown = 2.0f;
        }
    }
    
    // Оновлення юніта (рух та AI)
    void update() {
        float currentTime = GetTime();
        
        // AI поведінка
        if (is_ai_controlled) {
            ai_timer += GetFrameTime();
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
        
        // Рух з pathfinding або старим способом
        if (is_moving && !is_attacking) {
            if (usePathfinding && hasPath()) {
                // Використовуємо pathfinding
                followPath(GetFrameTime());
                checkIfStuck(GetFrameTime());
            } else {
                // Старий спосіб руху (для сумісності)
                float dx = target_x - x;
                float dy = target_y - y;
                float distance = sqrt(dx * dx + dy * dy);
                
                if (distance < speed + 1.0f) {
                    x = target_x;
                    y = target_y;
                    is_moving = false;
                } else {
                    float move_x = (dx / distance) * speed;
                    float move_y = (dy / distance) * speed;
                    
                    int new_x = x + (int)round(move_x);
                    int new_y = y + (int)round(move_y);
                    
                    x = new_x;
                    y = new_y;
                }
            }
        }
    }
    
    // Атакувати ціль
    bool attackTarget(Unit& target) {
        float currentTime = GetTime();
        if (attack_damage > 0 && currentTime - last_attack_time >= attack_cooldown) {
            float distance = sqrt(pow(x - target.x, 2) + pow(y - target.y, 2));
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
            // Простий AI: рухатися в випадковому напрямку
            int random_x = 100 + rand() % 824; // В межах карти
            int random_y = 100 + rand() % 568;
            moveTo(random_x, random_y);
        }
    }
    
    // Встановити ціль для руху
    void moveTo(int newX, int newY) {
        target_x = newX;
        target_y = newY;
        is_moving = true;
        is_harvesting = false; // ВАЖЛИВО: припинити збір при русі
        printf("[MOVETO] Unit at (%d,%d) moving to (%d,%d), is_moving=true\n", x, y, newX, newY);
    }
    
    // Встановити шлях для pathfinding
    void setPath(const std::vector<Vector2>& newPath) {
        path = newPath;
        currentWaypointIndex = 0;
        stuckTimer = 0.0f;
        unstuckAttempts = 0;
        lastPosition = {(float)x, (float)y};
        
        if (!path.empty()) {
            is_moving = true;
            target_x = (int)path[0].x;
            target_y = (int)path[0].y;
        }
    }
    
    // Перевірити чи є шлях
    bool hasPath() const {
        return !path.empty() && currentWaypointIndex < path.size();
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
        float dx = waypoint.x - x;
        float dy = waypoint.y - y;
        float distance = sqrt(dx * dx + dy * dy);
        
        // Якщо досягли точки, переходимо до наступної
        if (distance < speed + 2.0f) {
            currentWaypointIndex++;
            
            if (currentWaypointIndex >= path.size()) {
                // Досягли кінця шляху
                x = (int)waypoint.x;
                y = (int)waypoint.y;
                is_moving = false;
                clearPath();
                return;
            }
            
            // Оновлюємо ціль до наступної точки
            waypoint = path[currentWaypointIndex];
            target_x = (int)waypoint.x;
            target_y = (int)waypoint.y;
        }
        
        // Рухаємося до поточної точки
        if (distance > 0) {
            float move_x = (dx / distance) * speed;
            float move_y = (dy / distance) * speed;
            
            x += (int)round(move_x);
            y += (int)round(move_y);
        }
    }
    
    // Перевірити чи юніт застряг
    void checkIfStuck(float deltaTime) {
        if (!is_moving || !usePathfinding) {
            return;
        }
        
        // Перевіряємо чи змінилася позиція
        float dx = x - lastPosition.x;
        float dy = y - lastPosition.y;
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
        
        x += offsetX;
        y += offsetY;
        
        printf("[UNSTUCK] Unit moved by (%d,%d) to (%d,%d)\n", offsetX, offsetY, x, y);
    }
    
    // Призначити ресурсну точку для збору
    void assignResource(int resourceX, int resourceY, int buildingX, int buildingY) {
        assigned_resource_x = resourceX;
        assigned_resource_y = resourceY;
        dropoff_building_x = buildingX;
        dropoff_building_y = buildingY;
        moveTo(resourceX, resourceY);
        printf("Unit assigned: resource(%d,%d) building(%d,%d)\n", resourceX, resourceY, buildingX, buildingY);
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
        return {(float)x - 8, (float)y - 8, 16, 16};
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
        Color unitColor = getColor();
        
        // Малювання юніта як кружечка
        DrawCircle(x, y, 8, unitColor);
        
        // Рамка
        DrawCircleLines(x, y, 8, selected ? YELLOW : WHITE);
        
        // Індикатор здоров'я
        if (hp < max_hp) {
            float healthPercent = (float)hp / max_hp;
            DrawRectangle(x - 8, y - 12, 16, 3, RED);
            DrawRectangle(x - 8, y - 12, (int)(16 * healthPercent), 3, GREEN);
        }
        
        // Індикатор ресурсів для збирачів
        if (can_harvest && (carrying_food > 0 || carrying_gold > 0)) {
            DrawRectangle(x - 8, y + 10, 16, 3, DARKGRAY);
            float carryPercent = (float)(carrying_food + carrying_gold) / max_carry_capacity;
            DrawRectangle(x - 8, y + 10, (int)(16 * carryPercent), 3, YELLOW);
        }
        
        // Індикатор збору ресурсів
        if (is_harvesting) {
            DrawCircleLines(x, y, 12, YELLOW);
        }
        
        // Якщо рухається, показати ціль
        if (is_moving) {
            DrawLine(x, y, target_x, target_y, YELLOW);
            DrawCircleLines(target_x, target_y, 5, YELLOW);
        }
    }
    
    // Перевірка кліку
    bool isClicked(Vector2 mousePos) const {
        return CheckCollisionPointCircle(mousePos, {(float)x, (float)y}, 8);
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