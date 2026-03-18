#pragma once
#include "raylib.h"
#include "building.h" // Для Faction enum
#include "tilemap/coordinates.h"
#include "isometric_sprite.h"
#include "unit_animator.h"
#include <string>
#include <cmath>

extern bool showDebugVisuals; // з main.cpp

// Структура юніта
struct Unit {
    // НОВИЙ ПІДХІД: Grid coordinates (ізометрична система)
    GridCoords position;         // Поточні координати в сітці
    GridCoords target_position;  // Цільові координати для руху
    
    // Для плавної інтерполяції між тайлами
    float interpolation_progress; // 0.0 - 1.0
    ScreenCoords current_screen_pos; // Поточна екранна позиція для плавного руху
    ScreenCoords exact_target_screen; // Точна екранна ціль (не прив'язана до grid)
    
    // COMPATIBILITY LAYER: Публічні змінні для старого коду
    // Ці змінні автоматично синхронізуються з grid координатами
    int x, y;                    // Screen coordinates (deprecated, але працюють)
    int target_x, target_y;      // Target screen coordinates (deprecated)
    int hp, max_hp;              // Здоров'я
    Faction faction;             // Фракція
    std::string unit_type;       // Тип юніта
    bool selected = false;       // Чи вибраний юніт
    bool is_moving = false;      // Чи рухається юніт
    float speed = 0.546875f;     // Швидкість руху
    
    // Isometric sprite system
    IsometricSprite sprite;      // Статичний спрайт (fallback якщо немає анімацій)
    UnitAnimator    animator;    // Повна анімаційна система
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
    int assigned_resource_x = -1;  // Призначена ресурсна точка X (deprecated - screen coords)
    int assigned_resource_y = -1;  // Призначена ресурсна точка Y (deprecated - screen coords)
    int dropoff_building_x = -1;   // Будівля для здачі X (deprecated - screen coords)
    int dropoff_building_y = -1;   // Будівля для здачі Y (deprecated - screen coords)
    
    // NEW: Grid-based resource assignment
    GridCoords assigned_resource_position;  // Призначена ресурсна точка (grid coords)
    GridCoords assigned_dropoff_position;   // Будівля для здачі (grid coords)
    bool has_assigned_resource;             // Чи є призначений ресурс
    
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
    GridCoords final_destination;     // Кінцева ціль (не проміжний waypoint)
    
    // Ініціалізація юніта
    void init(std::string type, Faction unitFaction, GridCoords startPos, bool aiControlled = false) {
        unit_type = type;
        faction = unitFaction;
        position = startPos;
        target_position = startPos;
        interpolation_progress = 1.0f; // Початково не рухається
        current_screen_pos = CoordinateConverter::gridToScreen(position);
        exact_target_screen = current_screen_pos;
        is_moving = false;
        is_ai_controlled = aiControlled;
        ai_timer = 0.0f;
        useDebugRendering = true; // За замовчуванням використовуємо debug
        has_assigned_resource = false; // NEW: Ініціалізація
        assigned_resource_position = GridCoords(-1, -1); // NEW
        assigned_dropoff_position = GridCoords(-1, -1); // NEW
        final_destination = startPos; // NEW: Ініціалізація кінцевої цілі
        
        // COMPATIBILITY: Синхронізуємо screen coordinates
        syncScreenCoords();
        
        // Завантажуємо анімації
        // Шлях: assets/sprites/isometric/units/{type}_{faction}/
        std::string factionSuffix = (unitFaction == ROME) ? "rome" : "carthage";
        std::string animBasePath = "assets/sprites/isometric/units/" + type + "_" + factionSuffix;

        // Спочатку пробуємо спрайтшіти (рекомендований формат)
        animator.loadSheets(animBasePath, 64, 64);
        // Якщо шітів немає — пробуємо окремі файли
        if (!animator.loaded) animator.load(animBasePath, 64, 64);

        if (animator.loaded) {
            useDebugRendering = false;
            TraceLog(LOG_INFO, "[UNIT] Animator loaded for %s_%s", type.c_str(), factionSuffix.c_str());
        } else {
            // Fallback: статичний спрайт (старий формат legionary_rome.png)
            if (sprite.loadFromFile((animBasePath + ".png").c_str())) {
                useDebugRendering = false;
                TraceLog(LOG_INFO, "[UNIT] Static sprite loaded for %s_%s", type.c_str(), factionSuffix.c_str());
            } else {
                TraceLog(LOG_INFO, "[UNIT] Using debug rendering for %s_%s", type.c_str(), factionSuffix.c_str());
            }
        }
        
        // Встановлення характеристик залежно від типу
        if (type == "legionary") {
            hp = max_hp = 100;
            speed = 0.68359375f;
            attack_damage = 25;
            attack_range = 30.0f;
            attack_cooldown = 1.5f;
        } else if (type == "phoenician") {
            hp = max_hp = 90;
            speed = 0.68359375f; // Швидкість руху
            can_harvest = false; // Фінікійці не збирають ресурси
            attack_damage = 30;
            attack_range = 25.0f;
            attack_cooldown = 1.2f;
        } else if (type == "slave") {
            hp = max_hp = 50;
            speed = 0.48828125f; // Раби повільніші
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
                // Не перебиваємо бойову логіку (переслідування/атаку)
                // makeAIDecision() тільки якщо не атакуємо і не переслідуємо ворога
                if (!is_attacking && target_unit_id < 0) {
                    makeAIDecision();
                }
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
                // Рухаємося до поточного waypoint
                ScreenCoords targetScreen = CoordinateConverter::gridToScreen(target_position);
                
                float dx = targetScreen.x - current_screen_pos.x;
                float dy = targetScreen.y - current_screen_pos.y;
                float distance = sqrt(dx * dx + dy * dy);
                
                if (distance > 0.1f) {
                    // Оновлюємо аніматор: рух
                    animator.setState(ANIM_WALK, UnitAnimator::dirFromMovement(dx, dy));
                    float moveDistance = speed * deltaTime * 100.0f;
                    
                    if (moveDistance >= distance) {
                        // Досягли поточного waypoint
                        position = target_position;
                        current_screen_pos = targetScreen;
                        
                        if (hasPath()) {
                            currentWaypointIndex++;
                            if (currentWaypointIndex < (int)path.size()) {
                                GridCoords nextWp = CoordinateConverter::screenToGrid(
                                    ScreenCoords(path[currentWaypointIndex].x, path[currentWaypointIndex].y));
                                target_position = nextWp;
                                syncScreenCoords();
                            } else {
                                // Шлях завершено — рухаємося до точної кінцевої точки
                                path.clear();
                                currentWaypointIndex = 0;
                                float edx = exact_target_screen.x - current_screen_pos.x;
                                float edy = exact_target_screen.y - current_screen_pos.y;
                                if (sqrt(edx*edx + edy*edy) > 2.0f) {
                                    // Ще не дійшли до точної цілі — продовжуємо рух
                                    current_screen_pos.x += edx * (moveDistance / sqrt(edx*edx + edy*edy));
                                    current_screen_pos.y += edy * (moveDistance / sqrt(edx*edx + edy*edy));
                                } else {
                                    current_screen_pos = exact_target_screen;
                                    is_moving = false;
                                }
                            }
                        } else {
                            is_moving = false;
                        }
                    } else {
                        float ratio = moveDistance / distance;
                        current_screen_pos.x += dx * ratio;
                        current_screen_pos.y += dy * ratio;
                    }
                } else {
                    position = target_position;
                    current_screen_pos = targetScreen;
                    
                    if (hasPath()) {
                        currentWaypointIndex++;
                        if (currentWaypointIndex < (int)path.size()) {
                            GridCoords nextWp = CoordinateConverter::screenToGrid(
                                ScreenCoords(path[currentWaypointIndex].x, path[currentWaypointIndex].y));
                            target_position = nextWp;
                            syncScreenCoords();
                        } else {
                            path.clear();
                            currentWaypointIndex = 0;
                            is_moving = false;
                        }
                    } else {
                        is_moving = false;
                    }
                }
            } else {
                // Вже на grid target — переходимо до наступного waypoint
                if (hasPath()) {
                    currentWaypointIndex++;
                    if (currentWaypointIndex < (int)path.size()) {
                        GridCoords nextWp = CoordinateConverter::screenToGrid(
                            ScreenCoords(path[currentWaypointIndex].x, path[currentWaypointIndex].y));
                        target_position = nextWp;
                        syncScreenCoords();
                    } else {
                        path.clear();
                        currentWaypointIndex = 0;
                        // Доходимо до точної кінцевої точки
                        float edx = exact_target_screen.x - current_screen_pos.x;
                        float edy = exact_target_screen.y - current_screen_pos.y;
                        if (sqrt(edx*edx + edy*edy) > 2.0f) {
                            float moveDistance = speed * deltaTime * 100.0f;
                            float dist = sqrt(edx*edx + edy*edy);
                            current_screen_pos.x += edx * (moveDistance / dist);
                            current_screen_pos.y += edy * (moveDistance / dist);
                        } else {
                            current_screen_pos = exact_target_screen;
                            is_moving = false;
                        }
                    }
                } else {
                    is_moving = false;
                }
            }
        }
        
        // COMPATIBILITY: Синхронізуємо screen coordinates після руху
        syncScreenCoords();

        // Оновлюємо стан аніматора
        if (hp <= 0) {
            animator.setState(ANIM_DEATH, animator.currentDir);
        } else if (is_attacking) {
            animator.setState(ANIM_ATTACK, animator.currentDir);
        } else if (!is_moving) {
            animator.setState(ANIM_IDLE, animator.currentDir);
        }
        // WALK вже встановлюється всередині блоку руху вище
        animator.update(GetFrameTime());
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
        
        // ВАЖЛИВО: Скасувати цикл збору тільки якщо це команда від гравця (не від AI/harvesting)
        // Перевіряємо через окремий флаг - НЕ скасовуємо тут, бо ProcessResourceHarvesting
        // сам викликає moveTo() і не повинен скидати призначення
        
        // ВАЖЛИВО: Не перевіряємо колізії тут, бо будівлі займають багато клітинок
        // Pathfinding система має обходити будівлі
        
        target_position = newPos;
        interpolation_progress = 0.0f;
        is_moving = true;
        is_harvesting = false; // ВАЖЛИВО: припинити збір при русі
        final_destination = newPos; // Оновлюємо кінцеву ціль
        syncScreenCoords(); // COMPATIBILITY: Оновити screen coords
        printf("[MOVETO] Unit at (%d,%d) moving to (%d,%d), is_moving=true\n", 
               position.row, position.col, newPos.row, newPos.col);
    }
    
    // Команда руху від гравця - скасовує призначення ресурсу
    void moveToByPlayer(GridCoords newPos) {
        if (has_assigned_resource) {
            clearResourceAssignment();
            printf("[MOVETO] Clearing resource assignment due to new movement command\n");
        }
        moveTo(newPos);
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
    
    // Встановити шлях для pathfinding
    void setPath(const std::vector<Vector2>& newPath) {
        if (newPath.empty()) return;
        path = newPath;
        currentWaypointIndex = 0;
        is_moving = true;
        is_harvesting = false;
        // Перший waypoint стає target_position (grid для collision)
        GridCoords firstWp = CoordinateConverter::screenToGrid(ScreenCoords(path[0].x, path[0].y));
        target_position = firstWp;
        // Точна screen-ціль — остання точка шляху
        exact_target_screen = ScreenCoords(path.back().x, path.back().y);
        // Кінцева ціль — остання точка шляху в grid
        final_destination = CoordinateConverter::screenToGrid(ScreenCoords(path.back().x, path.back().y));
        syncScreenCoords();
    }
    
    // Встановити точну screen-ціль (без прив'язки до grid)
    void setExactTarget(ScreenCoords screenTarget) {
        exact_target_screen = screenTarget;
    }
    
    // Перевірити чи є шлях
    bool hasPath() const {
        return !path.empty() && currentWaypointIndex < (int)path.size();
    }
    
    // Очистити шлях
    void clearPath() {
        path.clear();
        currentWaypointIndex = 0;
        is_moving = false;
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
        // NEW: Зберігаємо grid coordinates
        assigned_resource_position = resourcePos;
        assigned_dropoff_position = buildingPos;
        
        // COMPATIBILITY: Конвертуємо в screen coords для старого коду
        ScreenCoords resScreen = CoordinateConverter::gridToScreen(resourcePos);
        ScreenCoords buildScreen = CoordinateConverter::gridToScreen(buildingPos);
        assigned_resource_x = (int)resScreen.x;
        assigned_resource_y = (int)resScreen.y;
        dropoff_building_x = (int)buildScreen.x;
        dropoff_building_y = (int)buildScreen.y;
        
        // ВАЖЛИВО: moveTo() скидає has_assigned_resource, тому встановлюємо після
        target_position = resourcePos;
        interpolation_progress = 0.0f;
        is_moving = true;
        is_harvesting = false;
        syncScreenCoords();
        
        // Встановлюємо флаг ПІСЛЯ руху, щоб moveTo() не скинув його
        has_assigned_resource = true;
        
        printf("Unit assigned: resource(%d,%d) building(%d,%d)\n", 
               resourcePos.row, resourcePos.col, buildingPos.row, buildingPos.col);
    }
    
    // NEW: Очистити призначення ресурсу
    void clearResourceAssignment() {
        has_assigned_resource = false;
        assigned_resource_position = GridCoords(-1, -1);
        assigned_dropoff_position = GridCoords(-1, -1);
        // COMPATIBILITY: Очищаємо старі поля
        assigned_resource_x = -1;
        assigned_resource_y = -1;
        dropoff_building_x = -1;
        dropoff_building_y = -1;
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
        return has_assigned_resource;
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
        if (useDebugRendering || (!animator.loaded && !sprite.isLoaded())) {
            IsometricSprite::drawDebugUnit(screenPos, unitColor, label);
        } else if (animator.loaded) {
            // Нова система: повна анімація
            Color tint = selected ? Color{220, 220, 150, 255} : WHITE;
            animator.draw(screenPos, tint);
        } else {
            // Fallback: статичний спрайт
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
        
        // Якщо рухається, показати ціль (тільки в debug режимі)
        if (is_moving && showDebugVisuals) {
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