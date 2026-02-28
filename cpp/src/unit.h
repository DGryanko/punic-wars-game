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
        
        // Рух з перевіркою колізій
        if (is_moving && !is_attacking) {
            // Обчислення відстані до цілі
            float dx = target_x - x;
            float dy = target_y - y;
            float distance = sqrt(dx * dx + dy * dy);
            
            // Якщо близько до цілі, зупинитися
            if (distance < speed + 1.0f) {
                x = target_x;
                y = target_y;
                is_moving = false;
            } else {
                // Рух до цілі з нормалізованим вектором
                float move_x = (dx / distance) * speed;
                float move_y = (dy / distance) * speed;
                
                // Нова позиція
                int new_x = x + (int)round(move_x);
                int new_y = y + (int)round(move_y);
                
                // Простий рух (колізії будуть перевірені в main.cpp)
                x = new_x;
                y = new_y;
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
        is_harvesting = false; // Припинити збір при русі
    }
    
    // Призначити ресурсну точку для збору
    void assignResource(int resourceX, int resourceY, int buildingX, int buildingY) {
        assigned_resource_x = resourceX;
        assigned_resource_y = resourceY;
        dropoff_building_x = buildingX;
        dropoff_building_y = buildingY;
        moveTo(resourceX, resourceY);
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