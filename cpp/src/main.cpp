#include "raylib.h"
#include "building.h"
#include "unit.h"
#include "resource.h"
#include "pathfinding.h"
#include "ui_button.h"
#include "tilemap/tilemap_generator.h"
#include <vector>
#include <cmath>
#include <ctime>

// Стани гри
enum GameState {
    MENU,
    SETTINGS,
    FACTION_SELECT,
    PLAYING,
    EXIT
};

// Музичні стани
enum MusicState {
    MUSIC_MENU,           // Головне меню
    MUSIC_AMBIENT,        // Амбієнт карти (фон геймплею)
    MUSIC_BATTLE_ROME,    // Бойова музика Риму
    MUSIC_BATTLE_CARTHAGE,// Бойова музика Карфагену
    MUSIC_SUSPENSE,       // Напруга (ворог атакує)
    MUSIC_PEACEFUL,       // Мирна (будівництво)
    MUSIC_DEFEAT          // Поразка
};

// Налаштування звуку
struct AudioSettings {
    float musicVolume = 0.5f;      // Музика (0.0 - 1.0)
    float ambientVolume = 0.5f;    // Середовище (0.0 - 1.0)
    float effectsVolume = 0.5f;    // Ефекти (0.0 - 1.0)
};

AudioSettings audioSettings;

// Глобальні змінні
GameState currentState = MENU;
Faction playerFaction = ROME; // Фракція гравця
int rome_food = 200;
int rome_money = 100;
int rome_food_reserved = 0;  // Зарезервовані ресурси
int rome_money_reserved = 0;
int carth_food = 150;
int carth_money = 200;
int carth_food_reserved = 0;
int carth_money_reserved = 0;

// Музика та звуки
Music menuMusic;
Music ambientMusic[2];          // 2 амбієнт треки
Music battleMusicRome[2];       // 2 бойові треки Риму
Music battleMusicCarthage[2];   // 2 бойові треки Карфагену
Music suspenseMusic[2];         // 2 треки напруги
Music peacefulMusic[2];         // 2 мирні треки
Music defeatMusic[2];           // 2 треки поразки
Music currentMusic;
Music nextMusic;                // Наступний трек для crossfade
MusicState currentMusicState = MUSIC_MENU;
MusicState previousMusicState = MUSIC_MENU;
Texture2D menuBackground;
Texture2D factionBackground;
bool audioInitialized = false;

// Текстури UI
Texture2D cursorIdle;
Texture2D cursorHover;
Texture2D cursorClick;
Texture2D resourcePanel;
Texture2D buttonBase;
Texture2D buttonHover;
Texture2D gameLogo;
bool logoLoaded = false;

// Кастомний шрифт
Font customFont;
bool fontLoaded = false;

// Crossfade параметри
bool isCrossfading = false;
float crossfadeTimer = 0.0f;
const float CROSSFADE_DURATION = 2.0f; // 2 секунди для плавного переходу

// Таймери для музичної логіки
float combatTimer = 0.0f;           // Скільки часу йде бій
float peacefulTimer = 0.0f;         // Скільки часу без бою
const float COMBAT_MUSIC_DELAY = 3.0f;    // 3 секунди бою для бойової музики
const float PEACEFUL_MUSIC_DELAY = 10.0f; // 10 секунд без бою для мирної музики

// Списки об'єктів
std::vector<Building> buildings;
std::vector<Unit> units;
std::vector<ResourcePoint> resources;
int selectedBuildingIndex = -1;
int selectedUnitIndex = -1;

// Pathfinding система
PathfindingManager pathfindingManager;

// Генератор ізометричної карти
MapGenerator* mapGenerator = nullptr;
TileMap* gameMap = nullptr;
IsometricRenderer* mapRenderer = nullptr;
Camera2D mapCamera = {0};

// Система виділення областю
bool isDragging = false;
Vector2 dragStart = {0, 0};
Vector2 dragEnd = {0, 0};

// Система подвійного кліку
float lastClickTime = 0.0f;
int lastClickedUnit = -1;
const float DOUBLE_CLICK_TIME = 0.5f;

// Система курсорів
enum CursorType {
    CURSOR_DEFAULT,
    CURSOR_HARVEST,
    CURSOR_ATTACK
};

CursorType currentCursor = CURSOR_DEFAULT;

// Функція для перемикання музики з плавним переходом
void SwitchMusic(MusicState newState) {
    if (currentMusicState == newState || !audioInitialized) return;
    
    // Вибираємо нову музику (випадковий трек з двох)
    int randomTrack = rand() % 2; // 0 або 1
    
    switch (newState) {
        case MUSIC_MENU:
            nextMusic = menuMusic;
            break;
        case MUSIC_AMBIENT:
            nextMusic = ambientMusic[randomTrack];
            break;
        case MUSIC_BATTLE_ROME:
            nextMusic = battleMusicRome[randomTrack];
            break;
        case MUSIC_BATTLE_CARTHAGE:
            nextMusic = battleMusicCarthage[randomTrack];
            break;
        case MUSIC_SUSPENSE:
            nextMusic = suspenseMusic[randomTrack];
            break;
        case MUSIC_PEACEFUL:
            nextMusic = peacefulMusic[randomTrack];
            break;
        case MUSIC_DEFEAT:
            nextMusic = defeatMusic[randomTrack];
            break;
    }
    
    // Починаємо crossfade
    isCrossfading = true;
    crossfadeTimer = 0.0f;
    
    // Запускаємо новий трек з нульовою гучністю
    SetMusicVolume(nextMusic, 0.0f);
    PlayMusicStream(nextMusic);
    
    previousMusicState = currentMusicState;
    currentMusicState = newState;
}

// Функція для оновлення crossfade
void UpdateCrossfade() {
    if (!isCrossfading || !audioInitialized) return;
    
    crossfadeTimer += GetFrameTime();
    float progress = crossfadeTimer / CROSSFADE_DURATION;
    
    if (progress >= 1.0f) {
        // Завершуємо crossfade
        progress = 1.0f;
        isCrossfading = false;
        StopMusicStream(currentMusic);
        currentMusic = nextMusic;
    }
    
    // Плавно зменшуємо гучність старого треку
    float oldVolume = audioSettings.musicVolume * (1.0f - progress);
    SetMusicVolume(currentMusic, oldVolume);
    
    // Плавно збільшуємо гучність нового треку
    float newVolume = audioSettings.musicVolume * progress;
    SetMusicVolume(nextMusic, newVolume);
}

// Функція для оновлення музичного стану в грі
void UpdateGameMusic() {
    if (!audioInitialized) return;
    
    // Оновлюємо crossfade якщо активний
    if (isCrossfading) {
        UpdateCrossfade();
        UpdateMusicStream(nextMusic); // Оновлюємо обидва треки під час crossfade
    }
    
    // Перевіряємо чи трек закінчився і запускаємо новий випадковий
    if (!IsMusicStreamPlaying(currentMusic) && !isCrossfading) {
        // Трек закінчився, запускаємо новий випадковий того ж типу
        int randomTrack = rand() % 2;
        
        switch (currentMusicState) {
            case MUSIC_AMBIENT:
                currentMusic = ambientMusic[randomTrack];
                break;
            case MUSIC_BATTLE_ROME:
                currentMusic = battleMusicRome[randomTrack];
                break;
            case MUSIC_BATTLE_CARTHAGE:
                currentMusic = battleMusicCarthage[randomTrack];
                break;
            case MUSIC_SUSPENSE:
                currentMusic = suspenseMusic[randomTrack];
                break;
            case MUSIC_PEACEFUL:
                currentMusic = peacefulMusic[randomTrack];
                break;
            case MUSIC_DEFEAT:
                currentMusic = defeatMusic[randomTrack];
                break;
            default:
                break;
        }
        
        SetMusicVolume(currentMusic, audioSettings.musicVolume);
        PlayMusicStream(currentMusic);
    }
    
    // Перевіряємо чи є бій
    bool isInCombat = false;
    bool enemyNearby = false;
    
    for (const auto& unit : units) {
        if (unit.faction == playerFaction) {
            // Перевіряємо чи атакує цей юніт
            if (unit.is_attacking || unit.target_unit_id >= 0) {
                isInCombat = true;
                break;
            }
            
            // Перевіряємо чи є вороги поблизу (в радіусі 200 пікселів)
            for (const auto& enemy : units) {
                if (enemy.faction != playerFaction) {
                    float dist = sqrt(pow(unit.x - enemy.x, 2) + pow(unit.y - enemy.y, 2));
                    if (dist < 200) {
                        enemyNearby = true;
                        break;
                    }
                }
            }
            if (enemyNearby) break;
        }
    }
    
    // Оновлюємо таймери
    if (isInCombat) {
        combatTimer += GetFrameTime();
        peacefulTimer = 0.0f;
    } else {
        peacefulTimer += GetFrameTime();
        combatTimer = 0.0f;
    }
    
    // Логіка перемикання музики за пріоритетами
    if (enemyNearby && !isInCombat && currentMusicState != MUSIC_SUSPENSE) {
        // Пріоритет 5: Ворог поблизу але ще не б'ються
        SwitchMusic(MUSIC_SUSPENSE);
    }
    else if (isInCombat && combatTimer > COMBAT_MUSIC_DELAY) {
        // Пріоритет 3-4: Бойова музика (залежить від фракції)
        MusicState battleState = (playerFaction == ROME) ? MUSIC_BATTLE_ROME : MUSIC_BATTLE_CARTHAGE;
        if (currentMusicState != battleState) {
            SwitchMusic(battleState);
        }
    }
    else if (!isInCombat && !enemyNearby && peacefulTimer > PEACEFUL_MUSIC_DELAY) {
        // Пріоритет 8: Мирна музика (будівництво)
        if (currentMusicState != MUSIC_PEACEFUL) {
            SwitchMusic(MUSIC_PEACEFUL);
        }
    }
    else if (!isInCombat && !enemyNearby && peacefulTimer > 2.0f && peacefulTimer <= PEACEFUL_MUSIC_DELAY) {
        // Пріоритет 2: Амбієнт (фон геймплею)
        if (currentMusicState != MUSIC_AMBIENT && currentMusicState != MUSIC_PEACEFUL) {
            SwitchMusic(MUSIC_AMBIENT);
        }
    }
}

// Функції для роботи з ресурсами
struct UnitCost {
    int food;
    int money;
};

UnitCost getUnitCost(const std::string& unitType) {
    if (unitType == "legionary") {
        return {30, 50}; // 30 їжі, 50 грошей
    } else if (unitType == "phoenician") {
        return {25, 60}; // 25 їжі, 60 грошей (дорожчі найманці)
    } else if (unitType == "slave") {
        return {10, 20}; // 10 їжі, 20 грошей
    }
    return {0, 0};
}

bool canAfford(Faction faction, const std::string& unitType) {
    UnitCost cost = getUnitCost(unitType);
    if (faction == ROME) {
        int available_food = rome_food - rome_food_reserved;
        int available_money = rome_money - rome_money_reserved;
        return available_food >= cost.food && available_money >= cost.money;
    } else {
        int available_food = carth_food - carth_food_reserved;
        int available_money = carth_money - carth_money_reserved;
        return available_food >= cost.food && available_money >= cost.money;
    }
}

void reserveResources(Faction faction, const std::string& unitType) {
    UnitCost cost = getUnitCost(unitType);
    if (faction == ROME) {
        rome_food_reserved += cost.food;
        rome_money_reserved += cost.money;
    } else {
        carth_food_reserved += cost.food;
        carth_money_reserved += cost.money;
    }
}

void spendResources(Faction faction, const std::string& unitType) {
    UnitCost cost = getUnitCost(unitType);
    if (faction == ROME) {
        rome_food -= cost.food;
        rome_money -= cost.money;
        rome_food_reserved -= cost.food;
        rome_money_reserved -= cost.money;
    } else {
        carth_food -= cost.food;
        carth_money -= cost.money;
        carth_food_reserved -= cost.food;
        carth_money_reserved -= cost.money;
    }
}

void unreserveResources(Faction faction, const std::string& unitType) {
    UnitCost cost = getUnitCost(unitType);
    if (faction == ROME) {
        rome_food_reserved -= cost.food;
        rome_money_reserved -= cost.money;
    } else {
        carth_food_reserved -= cost.food;
        carth_money_reserved -= cost.money;
    }
}

// Функції для виділення
void selectUnitsInArea(Rectangle area) {
    for (auto& unit : units) {
        if (unit.faction == playerFaction) {
            Vector2 unitPos = {(float)unit.x, (float)unit.y};
            if (CheckCollisionPointRec(unitPos, area)) {
                unit.selected = true;
            }
        }
    }
}

void selectAllUnitsOfType(const std::string& unitType) {
    for (auto& unit : units) {
        if (unit.faction == playerFaction && unit.unit_type == unitType) {
            unit.selected = true;
        }
    }
}

void clearAllSelections() {
    selectedBuildingIndex = -1;
    selectedUnitIndex = -1;
    for (auto& building : buildings) {
        building.selected = false;
    }
    for (auto& unit : units) {
        unit.selected = false;
    }
}

// Функція для оновлення курсора
void UpdateCursor() {
    Vector2 mousePos = GetMousePosition();
    currentCursor = CURSOR_DEFAULT;
    
    // Перевіряємо, чи є вибрані юніти
    bool hasSelectedHarvesters = false;
    bool hasSelectedSoldiers = false;
    
    for (const auto& unit : units) {
        if (unit.selected && unit.faction == playerFaction) {
            if (unit.can_harvest) {
                hasSelectedHarvesters = true;
            } else if (unit.attack_damage > 0) {
                hasSelectedSoldiers = true;
            }
        }
    }
    
    // Перевіряємо наведення на ресурси
    if (hasSelectedHarvesters) {
        for (const auto& resource : resources) {
            if (!resource.depleted && CheckCollisionPointRec(mousePos, resource.getRect())) {
                currentCursor = CURSOR_HARVEST;
                return;
            }
        }
    }
    
    // Перевіряємо наведення на ворожих юнітів
    if (hasSelectedSoldiers) {
        for (const auto& unit : units) {
            if (unit.faction != playerFaction) {
                // Збільшуємо радіус перевірки до 16 пікселів
                if (CheckCollisionPointCircle(mousePos, {(float)unit.x, (float)unit.y}, 16)) {
                    currentCursor = CURSOR_ATTACK;
                    return;
                }
            }
        }
    }
}

// Функція для малювання курсора
void DrawCustomCursor() {
    Vector2 mousePos = GetMousePosition();
    
    // Вибираємо текстуру курсора
    Texture2D* cursorTexture = &cursorIdle;
    
    switch (currentCursor) {
        case CURSOR_HARVEST:
            cursorTexture = &cursorHover;
            break;
        case CURSOR_ATTACK:
            cursorTexture = &cursorClick;
            break;
        case CURSOR_DEFAULT:
        default:
            cursorTexture = &cursorIdle;
            break;
    }
    
    // Малюємо курсор якщо текстура завантажена
    if (cursorTexture->id > 0) {
        // Малюємо курсор з верхнім лівим кутом в позиції миші (стандартна поведінка курсора)
        DrawTextureV(*cursorTexture, mousePos, WHITE);
    } else {
        // Fallback - малюємо червоний квадрат якщо текстура не завантажена
        DrawCircle((int)mousePos.x, (int)mousePos.y, 5, RED);
    }
}

// Функція для обробки збору ресурсів
void ProcessResourceHarvesting() {
    for (auto& unit : units) {
        if (unit.faction == playerFaction && unit.can_harvest) {
            // Якщо раб має призначену ресурсну точку
            if (unit.hasAssignedResource()) {
                // Перевіряємо чи ресурс ще існує
                bool resourceExists = false;
                for (const auto& resource : resources) {
                    if (!resource.depleted) {
                        float dist = sqrt(pow(unit.assigned_resource_x - (resource.x + 20), 2) + 
                                        pow(unit.assigned_resource_y - (resource.y + 20), 2));
                        if (dist < 5) {
                            resourceExists = true;
                            break;
                        }
                    }
                }
                
                // Якщо ресурс вичерпано, скидаємо призначення
                if (!resourceExists) {
                    unit.assigned_resource_x = -1;
                    unit.assigned_resource_y = -1;
                    unit.stopHarvesting();
                    continue;
                }
                
                // Якщо раб повний, йти здавати ресурси
                if (!unit.canCarryMore() && (unit.carrying_food > 0 || unit.carrying_gold > 0)) {
                    // КРИТИЧНО: примусово зупинити збір
                    unit.is_harvesting = false;
                    
                    // Перевірка, чи поруч з будівлею для здачі
                    float distToBuilding = sqrt(pow(unit.x - unit.dropoff_building_x, 2) + 
                                               pow(unit.y - unit.dropoff_building_y, 2));
                    
                    if (distToBuilding < 50) {
                        // Здати ресурси
                        int food, gold;
                        unit.dropResources(food, gold);
                        
                        if (playerFaction == ROME) {
                            rome_food += food;
                            rome_money += gold;
                        } else {
                            carth_food += food;
                            carth_money += gold;
                        }
                        
                        printf("[DROP] Resources dropped: food=%d, gold=%d. Total: food=%d money=%d\n", 
                               food, gold, rome_food, rome_money);
                        // Повернутися до ресурсу
                        unit.moveTo(unit.assigned_resource_x, unit.assigned_resource_y);
                        printf("[RETURN] Returning to resource at (%d,%d)\n", 
                               unit.assigned_resource_x, unit.assigned_resource_y);
                    } else {
                        // Перевіряємо чи раб рухається до правильної цілі
                        bool movingToCorrectTarget = (unit.target_x == unit.dropoff_building_x && 
                                                     unit.target_y == unit.dropoff_building_y);
                        
                        // Дебаг лог кожні 30 кадрів
                        static int debugCounter = 0;
                        if (++debugCounter % 30 == 0) {
                            printf("[FULL DEBUG] Slave at (%d,%d) full, target=(%d,%d), dropoff=(%d,%d), is_moving=%d, movingToCorrect=%d\n",
                                   unit.x, unit.y, unit.target_x, unit.target_y, 
                                   unit.dropoff_building_x, unit.dropoff_building_y,
                                   unit.is_moving, movingToCorrectTarget);
                        }
                        
                        // Якщо не рухається до будівлі, примусово відправити
                        if (!movingToCorrectTarget) {
                            unit.moveTo(unit.dropoff_building_x, unit.dropoff_building_y);
                            printf("[FORCE MOVE] Forcing move to dropoff building at (%d, %d)\n", 
                                   unit.dropoff_building_x, unit.dropoff_building_y);
                        }
                    }
                }
                // Якщо раб не повний і поруч з ресурсом, збирати
                else {
                    bool foundNearbyResource = false;
                    for (auto& resource : resources) {
                        if (!resource.depleted) {
                            float distance = sqrt(pow(unit.x - (resource.x + 20), 2) + 
                                                pow(unit.y - (resource.y + 20), 2));
                            if (distance < 40) {
                                // Почати збір
                                unit.startHarvesting();
                                
                                // Збираємо ресурс
                                int harvestAmount = 1; // 1 одиниця за кадр (зменшено в 2 рази)
                                int harvested = resource.harvest(harvestAmount);
                                
                                if (harvested > 0) {
                                    if (resource.type == FOOD_SOURCE) {
                                        unit.addResources(harvested, 0);
                                    } else if (resource.type == GOLD_SOURCE) {
                                        unit.addResources(0, harvested);
                                    }
                                    
                                    // Логуємо тільки кожні 30 кадрів (раз на пів секунди)
                                    static int harvestLogCounter = 0;
                                    if (++harvestLogCounter % 30 == 0) {
                                        printf("[HARVEST] Slave at (%d,%d) harvesting. Carrying: %d/%d\n", 
                                               unit.x, unit.y, unit.carrying_food + unit.carrying_gold, unit.max_carry_capacity);
                                    }
                                }
                                
                                foundNearbyResource = true;
                                break;
                            }
                        }
                    }
                    
                    // Якщо не знайшли ресурс поблизу і не рухаємося, йти до призначеного ресурсу
                    if (!foundNearbyResource && !unit.is_moving) {
                        float distToAssigned = sqrt(pow(unit.x - unit.assigned_resource_x, 2) + 
                                                   pow(unit.y - unit.assigned_resource_y, 2));
                        if (distToAssigned > 5) {
                            unit.moveTo(unit.assigned_resource_x, unit.assigned_resource_y);
                            printf("[GOTO] Slave moving to assigned resource at (%d,%d)\n", 
                                   unit.assigned_resource_x, unit.assigned_resource_y);
                        }
                    }
                }
            }
            // Старий код для рабів без призначеної точки (ручне керування або після скидання)
            else if (!unit.is_moving) {
                // Перевіряємо, чи юніт поруч з ресурсом - ЗАВЖДИ збирати якщо є місце
                for (auto& resource : resources) {
                    if (!resource.depleted && unit.canCarryMore()) {
                        float distance = sqrt(pow(unit.x - (resource.x + 20), 2) + pow(unit.y - (resource.y + 20), 2));
                        if (distance < 40) { // Збільшено радіус з 30 до 40
                            // Почати збір
                            unit.startHarvesting();
                            
                            // Збираємо ресурс
                            int harvestAmount = 1; // 1 одиниця за кадр (зменшено в 2 рази)
                            int harvested = resource.harvest(harvestAmount);
                            
                            if (harvested > 0) {
                                if (resource.type == FOOD_SOURCE) {
                                    unit.addResources(harvested, 0);
                                } else if (resource.type == GOLD_SOURCE) {
                                    unit.addResources(0, harvested);
                                }
                            }
                            
                            break;
                        }
                    }
                }
                
                // Перевіряємо, чи юніт поруч з квесторієм або HQ для здачі ресурсів
                if (unit.carrying_food > 0 || unit.carrying_gold > 0) {
                    // Спочатку шукаємо квесторіум
                    bool foundQuestorium = false;
                    for (const auto& building : buildings) {
                        if (building.faction == playerFaction && building.type == QUESTORIUM_ROME) {
                            float distance = sqrt(pow(unit.x - (building.x + 40), 2) + pow(unit.y - (building.y + 30), 2));
                            if (distance < 50) {
                                // Здати ресурси
                                int food, gold;
                                unit.dropResources(food, gold);
                                
                                if (playerFaction == ROME) {
                                    rome_food += food;
                                    rome_money += gold;
                                } else {
                                    carth_food += food;
                                    carth_money += gold;
                                }
                                
                                unit.stopHarvesting();
                                foundQuestorium = true;
                                break;
                            }
                        }
                    }
                    
                    // Якщо не знайшли квесторіум, шукаємо HQ
                    if (!foundQuestorium) {
                        for (const auto& building : buildings) {
                            if (building.faction == playerFaction && 
                                (building.type == HQ_ROME || building.type == HQ_CARTHAGE)) {
                                float distance = sqrt(pow(unit.x - (building.x + 40), 2) + pow(unit.y - (building.y + 30), 2));
                                if (distance < 50) {
                                    // Здати ресурси
                                    int food, gold;
                                    unit.dropResources(food, gold);
                                    
                                    if (playerFaction == ROME) {
                                        rome_food += food;
                                        rome_money += gold;
                                    } else {
                                        carth_food += food;
                                        carth_money += gold;
                                    }
                                    
                                    unit.stopHarvesting();
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// Ініціалізація будівель
void InitBuildings() {
    buildings.clear();
    
    // Римський табір
    Building romeHQ;
    romeHQ.init(HQ_ROME, ROME, 100, 100);
    buildings.push_back(romeHQ);
    
    // Римська казарма
    Building romeBarracks;
    romeBarracks.init(BARRACKS_ROME, ROME, 200, 100);
    buildings.push_back(romeBarracks);
    
    // Карфагенський табір
    Building carthageHQ;
    carthageHQ.init(HQ_CARTHAGE, CARTHAGE, 800, 600);
    buildings.push_back(carthageHQ);
    
    // Карфагенська казарма
    Building carthageBarracks;
    carthageBarracks.init(BARRACKS_CARTHAGE, CARTHAGE, 700, 600);
    buildings.push_back(carthageBarracks);
    
    // Римський квесторій
    Building romeQuestorium;
    romeQuestorium.init(QUESTORIUM_ROME, ROME, 300, 100);
    buildings.push_back(romeQuestorium);
    
    // Ініціалізація pathfinding
    pathfindingManager.init(1434, 1075);
    pathfindingManager.updateGrid(buildings);
    printf("[PATHFINDING] Navigation grid initialized: %dx%d cells\n", 
           pathfindingManager.getGrid().getWidth(), 
           pathfindingManager.getGrid().getHeight());
}

// Ініціалізація ресурсних точок
void InitResources() {
    resources.clear();
    
    // Ресурси їжі
    ResourcePoint food1;
    food1.init(FOOD_SOURCE, 300, 200, 500);
    resources.push_back(food1);
    
    ResourcePoint food2;
    food2.init(FOOD_SOURCE, 600, 400, 400);
    resources.push_back(food2);
    
    // Ресурси золота
    ResourcePoint gold1;
    gold1.init(GOLD_SOURCE, 150, 350, 300);
    resources.push_back(gold1);
    
    ResourcePoint gold2;
    gold2.init(GOLD_SOURCE, 750, 250, 350);
    resources.push_back(gold2);
}

// Створення юніта поруч з будівлею
void SpawnUnit(const Building& building, const std::string& unitType) {
    Unit newUnit;
    
    // Початкова позиція - праворуч від будівлі
    int spawnX = building.x + 90;
    int spawnY = building.y + 30;
    
    // Шукаємо найближчу вільну клітинку
    int gridX, gridY;
    pathfindingManager.getGrid().worldToGrid(spawnX, spawnY, gridX, gridY);
    
    bool foundFree = false;
    // Шукаємо в радіусі до 5 клітинок
    for (int radius = 0; radius <= 5 && !foundFree; radius++) {
        for (int dy = -radius; dy <= radius && !foundFree; dy++) {
            for (int dx = -radius; dx <= radius && !foundFree; dx++) {
                int nx = gridX + dx;
                int ny = gridY + dy;
                if (pathfindingManager.getGrid().isWalkable(nx, ny)) {
                    // Конвертуємо назад в світові координати
                    pathfindingManager.getGrid().gridToWorld(nx, ny, spawnX, spawnY);
                    foundFree = true;
                }
            }
        }
    }
    
    if (!foundFree) {
        printf("[SPAWN] Warning: Could not find free spawn position for unit!\n");
    }
    
    // Визначаємо, чи має юніт керуватися AI
    bool isAI = (building.faction != playerFaction);
    
    newUnit.init(unitType, building.faction, spawnX, spawnY, isAI);
    units.push_back(newUnit);
    
    printf("[SPAWN] Unit spawned at (%d, %d)\n", spawnX, spawnY);
}

// Функція для малювання меню налаштувань
void DrawSettings() {
    ClearBackground(BLACK);
    
    // Заголовок
    DrawText("SETTINGS", 450, 100, 40, GOLD);
    
    // Позиції повзунків
    int sliderX = 400;
    int sliderWidth = 400;
    int sliderHeight = 20;
    
    // Повзунок музики
    DrawText("Music Volume:", 200, 220, 20, WHITE);
    Rectangle musicSlider = {(float)sliderX, 220, (float)sliderWidth, (float)sliderHeight};
    DrawRectangleRec(musicSlider, DARKGRAY);
    DrawRectangle(sliderX, 220, (int)(sliderWidth * audioSettings.musicVolume), sliderHeight, GREEN);
    DrawText(TextFormat("%.0f%%", audioSettings.musicVolume * 100), 820, 220, 20, WHITE);
    
    // Повзунок середовища
    DrawText("Ambient Volume:", 200, 300, 20, WHITE);
    Rectangle ambientSlider = {(float)sliderX, 300, (float)sliderWidth, (float)sliderHeight};
    DrawRectangleRec(ambientSlider, DARKGRAY);
    DrawRectangle(sliderX, 300, (int)(sliderWidth * audioSettings.ambientVolume), sliderHeight, BLUE);
    DrawText(TextFormat("%.0f%%", audioSettings.ambientVolume * 100), 820, 300, 20, WHITE);
    
    // Повзунок ефектів
    DrawText("Effects Volume:", 200, 380, 20, WHITE);
    Rectangle effectsSlider = {(float)sliderX, 380, (float)sliderWidth, (float)sliderHeight};
    DrawRectangleRec(effectsSlider, DARKGRAY);
    DrawRectangle(sliderX, 380, (int)(sliderWidth * audioSettings.effectsVolume), sliderHeight, RED);
    DrawText(TextFormat("%.0f%%", audioSettings.effectsVolume * 100), 820, 380, 20, WHITE);
    
    // Кнопка назад (без static)
    DynamicButton backButton(450, 500, "BACK", 20);
    Vector2 mousePos = GetMousePosition();
    backButton.Update(mousePos);
    backButton.Draw();
    
    // Обробка повзунків
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(mousePos, musicSlider)) {
            audioSettings.musicVolume = (mousePos.x - sliderX) / sliderWidth;
            if (audioSettings.musicVolume < 0) audioSettings.musicVolume = 0;
            if (audioSettings.musicVolume > 1) audioSettings.musicVolume = 1;
            if (audioInitialized) {
                SetMusicVolume(currentMusic, audioSettings.musicVolume);
            }
        }
        if (CheckCollisionPointRec(mousePos, ambientSlider)) {
            audioSettings.ambientVolume = (mousePos.x - sliderX) / sliderWidth;
            if (audioSettings.ambientVolume < 0) audioSettings.ambientVolume = 0;
            if (audioSettings.ambientVolume > 1) audioSettings.ambientVolume = 1;
        }
        if (CheckCollisionPointRec(mousePos, effectsSlider)) {
            audioSettings.effectsVolume = (mousePos.x - sliderX) / sliderWidth;
            if (audioSettings.effectsVolume < 0) audioSettings.effectsVolume = 0;
            if (audioSettings.effectsVolume > 1) audioSettings.effectsVolume = 1;
        }
    }
    
    // Обробка кнопки назад
    if (backButton.IsClicked()) {
        currentState = MENU;
    }
    
    // Інструкції
    DrawText("Adjust audio settings", 430, 450, 16, GRAY);
    
    // Малювання курсора
    DrawCustomCursor();
}

// Функція для малювання меню
void DrawMenu() {
    ClearBackground(BLACK);
    
    // Малюємо фон якщо завантажений (масштабуємо до розміру вікна)
    if (menuBackground.id > 0) {
        // Розраховуємо масштаб для заповнення екрану
        float scaleX = 1434.0f / menuBackground.width;
        float scaleY = 1075.0f / menuBackground.height;
        float scale = (scaleX > scaleY) ? scaleX : scaleY; // Беремо більший масштаб
        
        // Центруємо фон
        float drawWidth = menuBackground.width * scale;
        float drawHeight = menuBackground.height * scale;
        float offsetX = (1434 - drawWidth) / 2;
        float offsetY = (1075 - drawHeight) / 2;
        
        DrawTexturePro(
            menuBackground,
            {0, 0, (float)menuBackground.width, (float)menuBackground.height},
            {offsetX, offsetY, drawWidth, drawHeight},
            {0, 0},
            0.0f,
            WHITE
        );
    }
    
    // Напівпрозорий оверлей для кращої читабельності - ПРИБРАНО
    // DrawRectangle(0, 0, 1434, 1075, {0, 0, 0, 150});
    
    // Малюємо логотип якщо завантажений
    if (logoLoaded && gameLogo.id > 0) {
        // Розраховуємо розмір логотипу (зменшуємо до 15% від оригінального розміру)
        float logoScale = 0.15f;
        float logoWidth = gameLogo.width * logoScale;
        float logoHeight = gameLogo.height * logoScale;
        
        // Центруємо логотип вгорі
        float logoX = (1434 - logoWidth) / 2;
        float logoY = 80;
        
        DrawTexturePro(
            gameLogo,
            {0, 0, (float)gameLogo.width, (float)gameLogo.height},
            {logoX, logoY, logoWidth, logoHeight},
            {0, 0},
            0.0f,
            WHITE
        );
    }
    
    // Підзаголовок прибрано - не потрібен
    
    // Створюємо динамічні кнопки з нормальним шрифтом (без static, щоб позиції оновлювалися)
    // Відцентровуємо кнопки по горизонталі (ширина екрану 1434)
    // Кнопки мають бути симетричні відносно центру
    // Опускаємо кнопки нижче (додаємо висоту однієї кнопки ~92px)
    DynamicButton startButton(400, 440, "START GAME", 24);
    DynamicButton settingsButton(400, 550, "SETTINGS", 24);
    DynamicButton exitButton(400, 660, "EXIT", 24);
    
    // Розраховуємо центр для кожної кнопки
    float centerX = 1434.0f / 2.0f;
    startButton.bounds.x = centerX - startButton.bounds.width / 2.0f;
    settingsButton.bounds.x = centerX - settingsButton.bounds.width / 2.0f;
    exitButton.bounds.x = centerX - exitButton.bounds.width / 2.0f;
    
    // Оновлюємо стан кнопок
    Vector2 mousePos = GetMousePosition();
    startButton.Update(mousePos);
    settingsButton.Update(mousePos);
    exitButton.Update(mousePos);
    
    // Малюємо кнопки
    startButton.Draw();
    settingsButton.Draw();
    exitButton.Draw();
    
    // Обробка кліків
    if (startButton.IsClicked()) {
        currentState = FACTION_SELECT; // Переходимо до вибору фракції
    }
    if (settingsButton.IsClicked()) {
        currentState = SETTINGS;
    }
    if (exitButton.IsClicked()) {
        currentState = EXIT;
    }
    
    // Малювання курсора
    DrawCustomCursor();
}

// Функція для екрану вибору фракції
void DrawFactionSelect() {
    ClearBackground(BLACK);
    
    // Малюємо фон якщо завантажений (масштабуємо до розміру вікна)
    if (factionBackground.id > 0) {
        float scaleX = 1434.0f / factionBackground.width;
        float scaleY = 1075.0f / factionBackground.height;
        float scale = (scaleX > scaleY) ? scaleX : scaleY;
        
        float drawWidth = factionBackground.width * scale;
        float drawHeight = factionBackground.height * scale;
        float offsetX = (1434 - drawWidth) / 2;
        float offsetY = (1075 - drawHeight) / 2;
        
        DrawTexturePro(
            factionBackground,
            {0, 0, (float)factionBackground.width, (float)factionBackground.height},
            {offsetX, offsetY, drawWidth, drawHeight},
            {0, 0},
            0.0f,
            WHITE
        );
    }
    
    // Заголовок
    DrawText("CHOOSE YOUR FACTION", 420, 150, 30, GOLD);
    
    // Створюємо динамічні кнопки (без static)
    DynamicButton romeButton(400, 350, "ROME", 30);
    DynamicButton carthageButton(400, 460, "CARTHAGE", 30);
    DynamicButton backButton(400, 650, "BACK", 20);
    
    // Відцентровуємо кнопки
    float centerX = 1434.0f / 2.0f;
    romeButton.bounds.x = centerX - romeButton.bounds.width / 2.0f;
    carthageButton.bounds.x = centerX - carthageButton.bounds.width / 2.0f;
    backButton.bounds.x = centerX - backButton.bounds.width / 2.0f;
    
    // Оновлюємо стан кнопок
    Vector2 mousePos = GetMousePosition();
    romeButton.Update(mousePos);
    carthageButton.Update(mousePos);
    backButton.Update(mousePos);
    
    // Малюємо кнопки
    romeButton.Draw();
    carthageButton.Draw();
    backButton.Draw();
    
    // Обробка кліків
    if (romeButton.IsClicked()) {
        playerFaction = ROME;
        currentState = PLAYING;
        InitBuildings();
        InitResources();
        units.clear();
        // Запускаємо амбієнт музику для геймплею
        if (audioInitialized) {
            SwitchMusic(MUSIC_AMBIENT);
        }
    }
    if (carthageButton.IsClicked()) {
        playerFaction = CARTHAGE;
        currentState = PLAYING;
        InitBuildings();
        InitResources();
        units.clear();
        // Запускаємо амбієнт музику для геймплею
        if (audioInitialized) {
            SwitchMusic(MUSIC_AMBIENT);
        }
    }
    if (backButton.IsClicked()) {
        currentState = MENU;
        // Повертаємось до меню музики
        if (audioInitialized) {
            SwitchMusic(MUSIC_MENU);
        }
    }
    
    // Малювання курсора
    DrawCustomCursor();
}

// Функція для малювання панелі замовлення юнітів
void DrawUnitOrderPanel() {
    if (selectedBuildingIndex >= 0) {
        const Building& selected = buildings[selectedBuildingIndex];
        
        // Панель знизу зліва
        Rectangle panelRect = {10, 950, 300, 100}; // Оновлено для нового розміру вікна
        DrawRectangleRec(panelRect, {0, 0, 0, 200});
        DrawRectangleLines((int)panelRect.x, (int)panelRect.y, (int)panelRect.width, (int)panelRect.height, WHITE);
        
        // Назва будівлі
        DrawText(selected.name.c_str(), 20, 960, 16, WHITE);
        
        // Кнопки для замовлення юнітів
        int buttonY = 985;
        int buttonSize = 50;
        int buttonSpacing = 60;
        
        if (selected.type == BARRACKS_ROME && playerFaction == ROME) {
            // Кнопка легіонера
            Rectangle legionaryButton = {20, (float)buttonY, (float)buttonSize, (float)buttonSize};
            UnitCost cost = getUnitCost("legionary");
            bool canAffordUnit = canAfford(ROME, "legionary");
            bool canProduce = selected.units_produced < 8;
            
            Color buttonColor = (canAffordUnit && canProduce) ? DARKGREEN : DARKGRAY;
            DrawRectangleRec(legionaryButton, buttonColor);
            DrawRectangleLines((int)legionaryButton.x, (int)legionaryButton.y, (int)legionaryButton.width, (int)legionaryButton.height, WHITE);
            
            // Іконка (тимчасово текст)
            DrawText("LEG", 25, buttonY + 15, 12, WHITE);
            
            // Черга (число в куточку)
            int queueCount = 0;
            if (selected.is_producing && selected.producing_unit == "legionary") queueCount++;
            for (const auto& queued : selected.production_queue) {
                if (queued == "legionary") queueCount++;
            }
            if (queueCount > 0) {
                DrawCircle(65, buttonY + 5, 8, RED);
                DrawText(TextFormat("%d", queueCount), 62, buttonY + 1, 12, WHITE);
            }
            
            // Вартість
            DrawText(TextFormat("F:%d M:%d", cost.food, cost.money), 80, buttonY + 5, 12, WHITE);
            DrawText(TextFormat("%d/8", selected.units_produced), 80, buttonY + 20, 12, WHITE);
            
            // Обробка кліків
            Vector2 mousePos = GetMousePosition();
            if (CheckCollisionPointRec(mousePos, legionaryButton)) {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (canAffordUnit && canProduce) {
                        bool shouldPayNow = false;
                        if (buildings[selectedBuildingIndex].startProduction("legionary", shouldPayNow)) {
                            if (shouldPayNow) {
                                // Резервуємо і одразу витрачаємо
                                reserveResources(ROME, "legionary");
                                spendResources(ROME, "legionary");
                                printf("Started producing legionary (paid now)\n");
                            } else {
                                // Тільки резервуємо
                                reserveResources(ROME, "legionary");
                                printf("Queued legionary (reserved resources)\n");
                            }
                        }
                    } else {
                        printf("Cannot afford legionary or max units reached\n");
                    }
                } else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
                    if (buildings[selectedBuildingIndex].cancelLastInQueue()) {
                        // Повертаємо зарезервовані ресурси
                        unreserveResources(ROME, "legionary");
                        printf("Cancelled legionary production (unreserved)\n");
                    }
                }
            }
            
        } else if (selected.type == BARRACKS_CARTHAGE && playerFaction == CARTHAGE) {
            // Кнопка фінікійця
            Rectangle phoenicianButton = {20, (float)buttonY, (float)buttonSize, (float)buttonSize};
            UnitCost cost = getUnitCost("phoenician");
            bool canAffordUnit = canAfford(CARTHAGE, "phoenician");
            bool canProduce = true;
            
            Color buttonColor = (canAffordUnit && canProduce) ? DARKBLUE : DARKGRAY;
            DrawRectangleRec(phoenicianButton, buttonColor);
            DrawRectangleLines((int)phoenicianButton.x, (int)phoenicianButton.y, (int)phoenicianButton.width, (int)phoenicianButton.height, WHITE);
            
            DrawText("PHO", 25, buttonY + 15, 12, WHITE);
            
            // Черга
            int queueCount = 0;
            if (selected.is_producing && selected.producing_unit == "phoenician") queueCount++;
            for (const auto& queued : selected.production_queue) {
                if (queued == "phoenician") queueCount++;
            }
            if (queueCount > 0) {
                DrawCircle(65, buttonY + 5, 8, RED);
                DrawText(TextFormat("%d", queueCount), 62, buttonY + 1, 12, WHITE);
            }
            
            DrawText(TextFormat("F:%d M:%d", cost.food, cost.money), 80, buttonY + 5, 12, WHITE);
            
            Vector2 mousePos = GetMousePosition();
            if (CheckCollisionPointRec(mousePos, phoenicianButton)) {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (canAffordUnit && canProduce) {
                        bool shouldPayNow = false;
                        if (buildings[selectedBuildingIndex].startProduction("phoenician", shouldPayNow)) {
                            if (shouldPayNow) {
                                reserveResources(CARTHAGE, "phoenician");
                                spendResources(CARTHAGE, "phoenician");
                                printf("Started producing phoenician (paid now)\n");
                            } else {
                                reserveResources(CARTHAGE, "phoenician");
                                printf("Queued phoenician (reserved resources)\n");
                            }
                        }
                    } else {
                        printf("Cannot afford phoenician\n");
                    }
                } else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
                    if (buildings[selectedBuildingIndex].cancelLastInQueue()) {
                        unreserveResources(CARTHAGE, "phoenician");
                        printf("Cancelled phoenician production (unreserved)\n");
                    }
                }
            }
            
        } else if (selected.type == HQ_ROME && playerFaction == ROME) {
            // Кнопка раба
            Rectangle slaveButton = {20, (float)buttonY, (float)buttonSize, (float)buttonSize};
            UnitCost cost = getUnitCost("slave");
            bool canAffordUnit = canAfford(ROME, "slave");
            bool canProduce = true;
            
            Color buttonColor = (canAffordUnit && canProduce) ? BROWN : DARKGRAY;
            DrawRectangleRec(slaveButton, buttonColor);
            DrawRectangleLines((int)slaveButton.x, (int)slaveButton.y, (int)slaveButton.width, (int)slaveButton.height, WHITE);
            
            DrawText("SLV", 25, buttonY + 15, 12, WHITE);
            
            // Черга
            int queueCount = 0;
            if (selected.is_producing && selected.producing_unit == "slave") queueCount++;
            for (const auto& queued : selected.production_queue) {
                if (queued == "slave") queueCount++;
            }
            if (queueCount > 0) {
                DrawCircle(65, buttonY + 5, 8, RED);
                DrawText(TextFormat("%d", queueCount), 62, buttonY + 1, 12, WHITE);
            }
            
            DrawText(TextFormat("F:%d M:%d", cost.food, cost.money), 80, buttonY + 5, 12, WHITE);
            
            Vector2 mousePos = GetMousePosition();
            if (CheckCollisionPointRec(mousePos, slaveButton)) {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (canAffordUnit && canProduce) {
                        bool shouldPayNow = false;
                        if (buildings[selectedBuildingIndex].startProduction("slave", shouldPayNow)) {
                            if (shouldPayNow) {
                                reserveResources(ROME, "slave");
                                spendResources(ROME, "slave");
                                printf("Started producing slave (paid now)\n");
                            } else {
                                reserveResources(ROME, "slave");
                                printf("Queued slave (reserved resources)\n");
                            }
                        }
                    } else {
                        printf("Cannot afford slave\n");
                    }
                } else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
                    if (buildings[selectedBuildingIndex].cancelLastInQueue()) {
                        unreserveResources(ROME, "slave");
                        printf("Cancelled slave production (unreserved)\n");
                    }
                }
            }
        }
        
        // Прогрес виробництва
        if (selected.is_producing) {
            float progress = selected.production_progress / selected.production_time;
            DrawRectangle(20, 1040, (int)(260 * progress), 8, GREEN);
            DrawRectangleLines(20, 1040, 260, 8, WHITE);
            DrawText("PRODUCING...", 100, 1042, 12, WHITE);
        }
    }
}

// Обробка кліків
void HandleClicks() {
    Vector2 mousePos = GetMousePosition();
    float currentTime = GetTime();
    
    // Спочатку перевіряємо клік по панелі замовлення юнітів
    if (selectedBuildingIndex >= 0) {
        Rectangle panelRect = {10, 950, 300, 100}; // Оновлено для нового розміру вікна
        if (CheckCollisionPointRec(mousePos, panelRect)) {
            // Клік по панелі - не обробляємо тут, обробляється в DrawUnitOrderPanel
            return;
        }
    }
    
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        // Початок перетягування для виділення області
        isDragging = true;
        dragStart = mousePos;
        dragEnd = mousePos;
        
        // Перевірка подвійного кліку
        bool isDoubleClick = false;
        if (currentTime - lastClickTime < DOUBLE_CLICK_TIME && lastClickedUnit >= 0) {
            isDoubleClick = true;
            // Виділити всі юніти того ж типу
            if (lastClickedUnit < units.size()) {
                selectAllUnitsOfType(units[lastClickedUnit].unit_type);
            }
        }
        
        if (!isDoubleClick) {
            // Скидаємо попередній вибір
            clearAllSelections();
            
            // Спочатку перевіряємо клік по юнітах (вони менші)
            for (int i = 0; i < units.size(); i++) {
                // Перевіряємо тільки юніти гравця
                if (units[i].faction == playerFaction && units[i].isClicked(mousePos)) {
                    units[i].selected = true;
                    selectedUnitIndex = i;
                    lastClickedUnit = i;
                    lastClickTime = currentTime;
                    return; // Вийти, щоб не перевіряти будівлі
                }
            }
            
            // Потім перевіряємо клік по будівлях
            for (int i = 0; i < buildings.size(); i++) {
                // Перевіряємо тільки будівлі гравця
                if (buildings[i].faction == playerFaction && buildings[i].isClicked(mousePos)) {
                    buildings[i].selected = true;
                    selectedBuildingIndex = i;
                    return;
                }
            }
        }
        
        lastClickTime = currentTime;
    }
    
    // Оновлення перетягування
    if (isDragging && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        dragEnd = mousePos;
    }
    
    // Завершення перетягування
    if (isDragging && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        isDragging = false;
        
        // Створити прямокутник виділення
        Rectangle selectionRect = {
            (float)fmin(dragStart.x, dragEnd.x),
            (float)fmin(dragStart.y, dragEnd.y),
            (float)fabs(dragEnd.x - dragStart.x),
            (float)fabs(dragEnd.y - dragStart.y)
        };
        
        // Якщо область достатньо велика, виділити юніти в ній
        if (selectionRect.width > 10 && selectionRect.height > 10) {
            selectUnitsInArea(selectionRect);
        }
    }
    
    // Правий клік для руху вибраних юнітів або збору ресурсів
    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
        bool hasSelectedUnits = false;
        for (const auto& unit : units) {
            if (unit.selected && unit.faction == playerFaction) {
                hasSelectedUnits = true;
                break;
            }
        }
        
        if (hasSelectedUnits) {
            // Перевіряємо клік по ворожих юнітах для атаки
            bool foundEnemy = false;
            for (int i = 0; i < units.size(); i++) {
                if (units[i].faction != playerFaction && units[i].isClicked(mousePos)) {
                    // Відправляємо бойових юнітів атакувати
                    for (auto& unit : units) {
                        if (unit.selected && unit.faction == playerFaction && unit.attack_damage > 0) {
                            unit.moveTo(units[i].x, units[i].y);
                            unit.target_unit_id = i;
                        }
                    }
                    foundEnemy = true;
                    break;
                }
            }
            
            if (!foundEnemy) {
                // Перевіряємо клік по ресурсах для збирачів
                bool foundResource = false;
                for (int i = 0; i < resources.size(); i++) {
                    if (!resources[i].depleted && resources[i].isClicked(mousePos)) {
                        // Знаходимо найближчий КВЕСТОРІУМ для здачі ресурсів
                        int nearestBuildingX = -1;
                        int nearestBuildingY = -1;
                        float minDist = 999999.0f;
                        
                        for (const auto& building : buildings) {
                            if (building.faction == playerFaction && building.type == QUESTORIUM_ROME) {
                                float dist = sqrt(pow(resources[i].x - building.x, 2) + pow(resources[i].y - building.y, 2));
                                if (dist < minDist) {
                                    minDist = dist;
                                    nearestBuildingX = building.x + 40;
                                    nearestBuildingY = building.y + 30;
                                }
                            }
                        }
                        
                        // Якщо не знайшли квесторіум, шукаємо HQ
                        if (nearestBuildingX == -1) {
                            for (const auto& building : buildings) {
                                if (building.faction == playerFaction && 
                                    (building.type == HQ_ROME || building.type == HQ_CARTHAGE)) {
                                    float dist = sqrt(pow(resources[i].x - building.x, 2) + pow(resources[i].y - building.y, 2));
                                    if (dist < minDist) {
                                        minDist = dist;
                                        nearestBuildingX = building.x + 40;
                                        nearestBuildingY = building.y + 30;
                                    }
                                }
                            }
                        }
                        
                        // Відправляємо збирачів до ресурсу з призначенням
                        for (auto& unit : units) {
                            if (unit.selected && unit.faction == playerFaction && unit.can_harvest) {
                                if (nearestBuildingX != -1) {
                                    unit.assignResource(resources[i].x + 20, resources[i].y + 20, 
                                                      nearestBuildingX, nearestBuildingY);
                                    printf("Slave assigned to resource at (%d, %d), dropoff at (%d, %d)\n", 
                                           resources[i].x + 20, resources[i].y + 20, nearestBuildingX, nearestBuildingY);
                                }
                            }
                        }
                        foundResource = true;
                        break;
                    }
                }
                
                // Якщо не клікнули по ресурсу або ворогу, звичайний рух
                if (!foundResource) {
                    // Використовуємо pathfinding для руху
                    for (int i = 0; i < units.size(); i++) {
                        if (units[i].selected && units[i].faction == playerFaction) {
                            // Запитуємо шлях через pathfinding manager
                            Vector2 start = {(float)units[i].x, (float)units[i].y};
                            Vector2 goal = {mousePos.x, mousePos.y};
                            pathfindingManager.requestPath(i, start, goal, 1.0f);
                            
                            units[i].target_unit_id = -1; // Скидаємо ціль атаки
                            // Скидаємо призначений ресурс при ручному русі
                            if (units[i].can_harvest) {
                                units[i].assigned_resource_x = -1;
                                units[i].assigned_resource_y = -1;
                            }
                            
                            printf("[PATHFINDING] Requested path for unit %d from (%d,%d) to (%.0f,%.0f)\n", 
                                   i, units[i].x, units[i].y, mousePos.x, mousePos.y);
                        }
                    }
                }
            }
        }
    }
}

// Функція для малювання ігрового екрану
void DrawGame() {
    // Зелений фон карти
    ClearBackground({34, 85, 34, 255});
    
    // Обробка ESC - повернення в меню
    if (IsKeyPressed(KEY_ESCAPE)) {
        currentState = MENU;
        // Повертаємось до меню музики
        if (audioInitialized) {
            SwitchMusic(MUSIC_MENU);
        }
        return;
    }
    
    // Оновлення pathfinding manager
    pathfindingManager.update(GetFrameTime());
    
    // Перевіряємо чи є готові шляхи для юнітів
    for (int i = 0; i < units.size(); i++) {
        std::vector<Vector2> path;
        if (pathfindingManager.getPath(i, path)) {
            if (!path.empty()) {
                units[i].setPath(path);
                printf("[PATHFINDING] Unit %d received path with %d waypoints\n", i, (int)path.size());
            } else {
                printf("[PATHFINDING] Unit %d received empty path - destination unreachable\n", i);
            }
        }
        
        // Якщо юніт застряг і очистив шлях, але все ще хоче рухатися
        if (units[i].is_moving && !units[i].hasPath() && units[i].usePathfinding) {
            // Автоматично запитуємо новий шлях до останньої цілі
            if (units[i].target_x != units[i].x || units[i].target_y != units[i].y) {
                Vector2 start = {(float)units[i].x, (float)units[i].y};
                Vector2 goal = {(float)units[i].target_x, (float)units[i].target_y};
                pathfindingManager.requestPath(i, start, goal, 1.0f);
                printf("[PATHFINDING] Auto-requesting new path for stuck unit %d from (%d,%d) to (%d,%d)\n", 
                       i, units[i].x, units[i].y, units[i].target_x, units[i].target_y);
            }
        }
    }
    
    // Обробка введення
    HandleClicks();
    
    // Оновлення курсора
    UpdateCursor();
    
    // Обробка збору ресурсів
    ProcessResourceHarvesting();
    
    // Оновлення юнітів з перевіркою колізій та боєм
    for (int i = 0; i < units.size(); i++) {
        // Видаляємо мертвих юнітів
        if (units[i].isDead()) {
            units.erase(units.begin() + i);
            i--;
            continue;
        }
        
        // Зберігаємо стару позицію
        int old_x = units[i].x;
        int old_y = units[i].y;
        
        // Оновлюємо юніт
        units[i].update();
        
        // Проста перевірка колізій з будівлями (відкат позиції)
        Rectangle unitRect = {(float)units[i].x - 8, (float)units[i].y - 8, 16, 16};
        bool hasCollision = false;
        
        for (const auto& building : buildings) {
            if (CheckCollisionRecs(unitRect, building.getRect())) {
                hasCollision = true;
                break;
            }
        }
        
        // Якщо є колізія з будівлею, повертаємо на стару позицію
        if (hasCollision) {
            units[i].x = old_x;
            units[i].y = old_y;
        }
        
        // Бойова логіка - пошук ворогів поблизу
        if (units[i].attack_damage > 0 && !units[i].is_harvesting) {
            for (int j = 0; j < units.size(); j++) {
                if (i != j && units[i].faction != units[j].faction) {
                    float distance = sqrt(pow(units[i].x - units[j].x, 2) + pow(units[i].y - units[j].y, 2));
                    if (distance <= units[i].attack_range) {
                        units[i].attackTarget(units[j]);
                        break; // Атакуємо тільки одного ворога за раз
                    }
                }
            }
        }
    }
    
    // Оновлення будівель та виробництва
    float deltaTime = GetFrameTime();
    for (auto& building : buildings) {
        std::string startedUnit = "";
        std::string completedUnit = "";
        building.updateProduction(deltaTime, startedUnit, completedUnit);
        
        // Якщо почався новий юніт з черги - витратити ресурси
        if (!startedUnit.empty()) {
            spendResources(building.faction, startedUnit);
            printf("Started producing %s from queue (paid now)\n", startedUnit.c_str());
        }
        
        // Якщо завершився юніт - створити його
        if (!completedUnit.empty()) {
            SpawnUnit(building, completedUnit);
            printf("Completed producing %s\n", completedUnit.c_str());
        }
        
        // AI для ворожих будівель
        if (building.faction != playerFaction && !building.is_producing) {
            // Простий AI: виробляти юніти кожні 10 секунд
            static float ai_building_timer = 0.0f;
            ai_building_timer += deltaTime;
            
            if (ai_building_timer >= 10.0f) {
                ai_building_timer = 0.0f;
                
                if (building.type == BARRACKS_ROME) {
                    if (canAfford(building.faction, "legionary")) {
                        bool shouldPayNow = false;
                        if (building.startProduction("legionary", shouldPayNow)) {
                            if (shouldPayNow) {
                                reserveResources(building.faction, "legionary");
                                spendResources(building.faction, "legionary");
                            } else {
                                reserveResources(building.faction, "legionary");
                            }
                        }
                    }
                } else if (building.type == BARRACKS_CARTHAGE) {
                    if (canAfford(building.faction, "phoenician")) {
                        bool shouldPayNow = false;
                        if (building.startProduction("phoenician", shouldPayNow)) {
                            if (shouldPayNow) {
                                reserveResources(building.faction, "phoenician");
                                spendResources(building.faction, "phoenician");
                            } else {
                                reserveResources(building.faction, "phoenician");
                            }
                        }
                    }
                } else if (building.type == HQ_CARTHAGE) {
                    if (canAfford(building.faction, "phoenician")) {
                        bool shouldPayNow = false;
                        if (building.startProduction("phoenician", shouldPayNow)) {
                            if (shouldPayNow) {
                                reserveResources(building.faction, "phoenician");
                                spendResources(building.faction, "phoenician");
                            } else {
                                reserveResources(building.faction, "phoenician");
                            }
                        }
                    }
                }
            }
        }
        
        // AI для ворожих будівель
        if (building.faction != playerFaction && !building.is_producing) {
            // Простий AI: виробляти юніти кожні 10 секунд
            static float ai_building_timer = 0.0f;
            ai_building_timer += deltaTime;
            
            if (ai_building_timer >= 10.0f) {
                ai_building_timer = 0.0f;
                
                if (building.type == BARRACKS_ROME) {
                    if (canAfford(building.faction, "legionary")) {
                        bool shouldPayNow = false;
                        if (building.startProduction("legionary", shouldPayNow)) {
                            if (shouldPayNow) {
                                reserveResources(building.faction, "legionary");
                                spendResources(building.faction, "legionary");
                            } else {
                                reserveResources(building.faction, "legionary");
                            }
                        }
                    }
                } else if (building.type == BARRACKS_CARTHAGE) {
                    if (canAfford(building.faction, "phoenician")) {
                        bool shouldPayNow = false;
                        if (building.startProduction("phoenician", shouldPayNow)) {
                            if (shouldPayNow) {
                                reserveResources(building.faction, "phoenician");
                                spendResources(building.faction, "phoenician");
                            } else {
                                reserveResources(building.faction, "phoenician");
                            }
                        }
                    }
                } else if (building.type == HQ_CARTHAGE) {
                    if (canAfford(building.faction, "phoenician")) {
                        bool shouldPayNow = false;
                        if (building.startProduction("phoenician", shouldPayNow)) {
                            if (shouldPayNow) {
                                reserveResources(building.faction, "phoenician");
                                spendResources(building.faction, "phoenician");
                            } else {
                                reserveResources(building.faction, "phoenician");
                            }
                        }
                    }
                }
            }
        }
    }
    
    // HUD - панель ресурсів з текстурою
    if (resourcePanel.id > 0) {
        // Малюємо панель ресурсів зверху по центру
        float panelX = (1434 - resourcePanel.width) / 2.0f;
        DrawTexture(resourcePanel, (int)panelX, 10, WHITE);
        
        // Малюємо значення ресурсів на панелі
        if (playerFaction == ROME) {
            int available_food = rome_food - rome_food_reserved;
            int available_money = rome_money - rome_money_reserved;
            
            // Позиції для тексту (підлаштовуємо під панель)
            // Ліва частина панелі - їжа (амфора) - приблизно на 1/4 ширини панелі
            DrawText(TextFormat("%d", available_food), (int)panelX + 250, 50, 24, {255, 255, 255, 255});
            
            // Права частина панелі - гроші (монета) - приблизно на 3/4 ширини панелі
            DrawText(TextFormat("%d", available_money), (int)panelX + 750, 50, 24, {255, 255, 255, 255});
        } else {
            int available_food = carth_food - carth_food_reserved;
            int available_money = carth_money - carth_money_reserved;
            
            DrawText(TextFormat("%d", available_food), (int)panelX + 250, 50, 24, {255, 255, 255, 255});
            DrawText(TextFormat("%d", available_money), (int)panelX + 750, 50, 24, {255, 255, 255, 255});
        }
    } else {
        // Fallback якщо панель не завантажилась
        DrawRectangle(0, 0, 1434, 80, {0, 0, 0, 180});
        
        if (playerFaction == ROME) {
            int available_food = rome_food - rome_food_reserved;
            int available_money = rome_money - rome_money_reserved;
            DrawText(TextFormat("ROME - Food: %d (%d) | Money: %d (%d)", 
                     available_food, rome_food, available_money, rome_money), 
                     10, 10, 16, WHITE);
            DrawText("Your faction (available/total)", 10, 30, 14, LIGHTGRAY);
        } else {
            int available_food = carth_food - carth_food_reserved;
            int available_money = carth_money - carth_money_reserved;
            DrawText(TextFormat("CARTHAGE - Food: %d (%d) | Money: %d (%d)", 
                     available_food, carth_food, available_money, carth_money), 
                     10, 10, 16, ORANGE);
            DrawText("Your faction (available/total)", 10, 30, 14, LIGHTGRAY);
        }
    }
    
    // Малювання ізометричної карти (фон)
    if (mapRenderer && gameMap) {
        mapRenderer->setCamera(mapCamera);
        if (mapRenderer->isTilesetLoaded()) {
            mapRenderer->render(*gameMap);
        } else {
            mapRenderer->renderDebug(*gameMap);
        }
    }
    
    // Малювання всіх будівель
    for (const auto& building : buildings) {
        building.draw();
    }
    
    // Малювання всіх ресурсних точок
    for (const auto& resource : resources) {
        resource.draw();
    }
    
    // Малювання всіх юнітів
    for (const auto& unit : units) {
        unit.draw();
    }
    
    // Інформація про вибраний об'єкт
    if (selectedBuildingIndex >= 0) {
        const Building& selected = buildings[selectedBuildingIndex];
        std::string info = "Selected: " + selected.name;
        if (selected.faction == ROME) {
            info += " (Rome)";
        } else {
            info += " (Carthage)";
        }
        
        // Додаткова інформація про можливості будівлі
        if (selected.type == BARRACKS_ROME) {
            UnitCost cost = getUnitCost("legionary");
            info += " - Legionary (Food:" + std::to_string(cost.food) + " Money:" + std::to_string(cost.money) + ")";
            info += " [" + std::to_string(selected.units_produced) + "/8]";
        } else if (selected.type == BARRACKS_CARTHAGE) {
            UnitCost cost = getUnitCost("phoenician");
            info += " - Phoenician (Food:" + std::to_string(cost.food) + " Money:" + std::to_string(cost.money) + ")";
        } else if (selected.type == HQ_ROME) {
            UnitCost cost = getUnitCost("slave");
            info += " - Slave (Food:" + std::to_string(cost.food) + " Money:" + std::to_string(cost.money) + ")";
        } else if (selected.type == HQ_CARTHAGE) {
            UnitCost cost = getUnitCost("phoenician");
            info += " - Phoenician (Food:" + std::to_string(cost.food) + " Money:" + std::to_string(cost.money) + ")";
        }
        
        if (selected.is_producing) {
            info += " [PRODUCING...]";
        }
        
        DrawText(info.c_str(), 10, 50, 14, YELLOW);
    } else if (selectedUnitIndex >= 0) {
        const Unit& selected = units[selectedUnitIndex];
        std::string info = "Selected: " + selected.getDisplayName();
        info += " (HP: " + std::to_string(selected.hp) + "/" + std::to_string(selected.max_hp) + ")";
        if (!selected.is_ai_controlled) {
            info += " - Right click to move";
        } else {
            info += " - AI controlled";
        }
        DrawText(info.c_str(), 10, 50, 14, YELLOW);
    }
    
    // Статистика
    DrawText(TextFormat("Units: %d", (int)units.size()), 800, 50, 14, LIGHTGRAY);
    
    // Малювання області виділення
    if (isDragging) {
        Rectangle selectionRect = {
            (float)fmin(dragStart.x, dragEnd.x),
            (float)fmin(dragStart.y, dragEnd.y),
            (float)fabs(dragEnd.x - dragStart.x),
            (float)fabs(dragEnd.y - dragStart.y)
        };
        DrawRectangleLines((int)selectionRect.x, (int)selectionRect.y, (int)selectionRect.width, (int)selectionRect.height, YELLOW);
        DrawRectangle((int)selectionRect.x, (int)selectionRect.y, (int)selectionRect.width, (int)selectionRect.height, {255, 255, 0, 50});
    }
    
    // Панель замовлення юнітів
    DrawUnitOrderPanel();
    
    // Малювання курсора
    DrawCustomCursor();
    
    // Інструкції
    DrawText("ESC - Menu | LMB - Select | RMB - Move/Attack/Harvest | Drag - Area | 2xClick - All type", 10, 1040, 14, LIGHTGRAY);
    
    // Перевірка виходу в меню
    if (IsKeyPressed(KEY_ESCAPE)) {
        currentState = MENU;
    }
}

int main() {
    // Ініціалізація вікна (збільшено на 40%)
    const int screenWidth = 1434; // 1024 * 1.4
    const int screenHeight = 1075; // 768 * 1.4
    
    InitWindow(screenWidth, screenHeight, "Punic Wars: Castra");
    SetTargetFPS(60);
    
    // Ініціалізація аудіо
    InitAudioDevice();
    
    // Завантаження музики
    menuMusic = LoadMusicStream("assets/sounds/Punic wars_ Castra.mp3");
    ambientMusic[0] = LoadMusicStream("assets/sounds/Punic wars_ Castra Ambient.mp3");
    ambientMusic[1] = LoadMusicStream("assets/sounds/Punic wars_ Castra Ambient 2.mp3");
    battleMusicRome[0] = LoadMusicStream("assets/sounds/Punic wars_ Castra Battlemusic Rome.mp3");
    battleMusicRome[1] = LoadMusicStream("assets/sounds/Punic wars_ Castra Battlemusic Rome 2.mp3");
    battleMusicCarthage[0] = LoadMusicStream("assets/sounds/Punic wars_ Castra Battlemusic Carthage.mp3");
    battleMusicCarthage[1] = LoadMusicStream("assets/sounds/Punic wars_ Castra Battlemusic Carthage 2.mp3");
    suspenseMusic[0] = LoadMusicStream("assets/sounds/Punic wars_ Castra Suspense.mp3");
    suspenseMusic[1] = LoadMusicStream("assets/sounds/Punic wars_ Castra Suspense 2.mp3");
    peacefulMusic[0] = LoadMusicStream("assets/sounds/Punic wars_ Castra Peaceful.mp3");
    peacefulMusic[1] = LoadMusicStream("assets/sounds/Punic wars_ Castra Peaceful 2.mp3");
    defeatMusic[0] = LoadMusicStream("assets/sounds/Punic wars_ Castra Defeat.mp3");
    defeatMusic[1] = LoadMusicStream("assets/sounds/Punic wars_ Castra Defeat 2.mp3");
    
    // Завантаження фону меню
    menuBackground = LoadTexture("assets/Background.png");
    
    // Завантаження фону вибору фракції
    factionBackground = LoadTexture("assets/Background2.png");
    
    // Завантаження логотипу
    gameLogo = LoadTexture("assets/sprites/Logo.png");
    if (gameLogo.id > 0) {
        logoLoaded = true;
        printf("[TEXTURE] Logo loaded: %dx%d\n", gameLogo.width, gameLogo.height);
    } else {
        printf("[TEXTURE] Warning: Logo not loaded!\n");
    }
    
    // Завантаження кастомного шрифту
    customFont = LoadFontEx("assets/fonts/GameFont.ttf", 48, 0, 0);
    if (customFont.texture.id > 0) {
        fontLoaded = true;
        printf("[FONT] Custom font loaded successfully\n");
    } else {
        printf("[FONT] Warning: Custom font not loaded, using default\n");
    }
    
    // Завантаження текстур UI
    cursorIdle = LoadTexture("assets/sprites/Cursor_idle.png");
    cursorHover = LoadTexture("assets/sprites/Cursor_hover.png");
    cursorClick = LoadTexture("assets/sprites/Cursor_click.png");
    resourcePanel = LoadTexture("assets/sprites/ResourcePanel.png");
    buttonBase = LoadTexture("assets/sprites/Button_base.png");
    buttonHover = LoadTexture("assets/sprites/Button_hover_base.png");
    
    // Завантаження текстур для динамічних кнопок
    DynamicButton::LoadTextures();
    
    // Перевірка завантаження текстур (виводимо в файл для дебагу)
    FILE* debugFile = fopen("texture_debug.txt", "w");
    if (debugFile) {
        fprintf(debugFile, "Cursor Idle ID: %d, Size: %dx%d\n", cursorIdle.id, cursorIdle.width, cursorIdle.height);
        fprintf(debugFile, "Cursor Hover ID: %d, Size: %dx%d\n", cursorHover.id, cursorHover.width, cursorHover.height);
        fprintf(debugFile, "Cursor Click ID: %d, Size: %dx%d\n", cursorClick.id, cursorClick.width, cursorClick.height);
        fprintf(debugFile, "Resource Panel ID: %d, Size: %dx%d\n", resourcePanel.id, resourcePanel.width, resourcePanel.height);
        fprintf(debugFile, "Button Base ID: %d, Size: %dx%d\n", buttonBase.id, buttonBase.width, buttonBase.height);
        fprintf(debugFile, "Button Hover ID: %d, Size: %dx%d\n", buttonHover.id, buttonHover.width, buttonHover.height);
        fclose(debugFile);
    }
    
    // Приховуємо системний курсор
    HideCursor();
    
    // Ініціалізація генератора карти
    mapGenerator = new MapGenerator(time(nullptr));
    gameMap = new TileMap(mapGenerator->generate(50, 50));
    mapRenderer = new IsometricRenderer();
    
    // Завантаження тайлсету
    if (FileExists("assets/isometric_tileset.png")) {
        mapRenderer->loadTileset("assets/isometric_tileset.png");
        printf("[TILEMAP] Tileset loaded successfully\n");
    } else {
        printf("[TILEMAP] Warning: Tileset not found, using debug rendering\n");
    }
    
    // Налаштування камери для карти
    mapCamera.target = {0, 800};  // Центр карти 50x50
    mapCamera.offset = {screenWidth / 2.0f, screenHeight / 2.0f};
    mapCamera.rotation = 0.0f;
    mapCamera.zoom = 0.5f;
    mapRenderer->setCamera(mapCamera);
    
    printf("[TILEMAP] Map generated: %dx%d, %.1f%% passable\n", 
           gameMap->getWidth(), gameMap->getHeight(), 
           gameMap->getPassablePercentage() * 100.0f);
    
    // Перевірка завантаження
    if (menuMusic.frameCount > 0 && ambientMusic[0].frameCount > 0 && 
        battleMusicRome[0].frameCount > 0 && battleMusicCarthage[0].frameCount > 0) {
        audioInitialized = true;
        currentMusic = menuMusic;
        currentMusicState = MUSIC_MENU;
        SetMusicVolume(currentMusic, audioSettings.musicVolume);
        PlayMusicStream(currentMusic); // Запускаємо меню музику
    }
    
    // Головний цикл гри
    while (!WindowShouldClose() && currentState != EXIT) {
        // Оновлюємо музику
        if (audioInitialized) {
            UpdateMusicStream(currentMusic);
            
            // Оновлюємо музичний стан в грі
            if (currentState == PLAYING) {
                UpdateGameMusic();
            }
        }
        
        BeginDrawing();
        
        switch (currentState) {
            case MENU:
                DrawMenu();
                break;
            case SETTINGS:
                DrawSettings();
                break;
            case FACTION_SELECT:
                DrawFactionSelect();
                break;
            case PLAYING:
                DrawGame();
                break;
            case EXIT:
                break;
        }
        
        EndDrawing();
    }
    
    // Вивантаження ресурсів
    if (audioInitialized) {
        UnloadMusicStream(menuMusic);
        UnloadMusicStream(ambientMusic[0]);
        UnloadMusicStream(ambientMusic[1]);
        UnloadMusicStream(battleMusicRome[0]);
        UnloadMusicStream(battleMusicRome[1]);
        UnloadMusicStream(battleMusicCarthage[0]);
        UnloadMusicStream(battleMusicCarthage[1]);
        UnloadMusicStream(suspenseMusic[0]);
        UnloadMusicStream(suspenseMusic[1]);
        UnloadMusicStream(peacefulMusic[0]);
        UnloadMusicStream(peacefulMusic[1]);
        UnloadMusicStream(defeatMusic[0]);
        UnloadMusicStream(defeatMusic[1]);
    }
    UnloadTexture(menuBackground);
    UnloadTexture(factionBackground);
    if (logoLoaded) UnloadTexture(gameLogo);
    if (fontLoaded) UnloadFont(customFont);
    UnloadTexture(cursorIdle);
    UnloadTexture(cursorHover);
    UnloadTexture(cursorClick);
    UnloadTexture(resourcePanel);
    UnloadTexture(buttonBase);
    UnloadTexture(buttonHover);
    
    // Вивантаження текстур динамічних кнопок
    DynamicButton::UnloadTextures();
    
    // Очищення генератора карти
    if (mapRenderer) {
        mapRenderer->unloadTileset();
        delete mapRenderer;
    }
    if (gameMap) delete gameMap;
    if (mapGenerator) delete mapGenerator;
    
    CloseAudioDevice();
    
    // Закриття
    CloseWindow();
    return 0;
}