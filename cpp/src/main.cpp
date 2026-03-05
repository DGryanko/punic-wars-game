#include "raylib.h"
#include "building.h"
#include "building_texture_manager.h"
#include "building_renderer.h"
#include "building_placer.h"
#include "faction_spawner.h"
#include "unit_order_panel.h"
#include "resource_display.h"
#include "unit.h"
#include "resource.h"
#include "pathfinding.h"
#include "ui_button.h"
#include "tilemap/tilemap_generator.h"
#include "tilemap/coordinates.h"
#include "render_queue.h"
#include <vector>
#include <cmath>
#include <ctime>

// Стани гри
enum GameState {
    MENU,
    SETTINGS,
    FACTION_SELECT,
    PLAYING,
    PAUSE_MENU,  // Нове: пауза в грі
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

// Налаштування відображення
struct DisplaySettings {
    bool isWindowedFullscreen = false;
    int windowWidth = 1434;
    int windowHeight = 1075;
    int savedX = 0;
    int savedY = 0;
};

AudioSettings audioSettings;
DisplaySettings displaySettings;

// Глобальні змінні
GameState currentState = MENU;
GameState returnFromSettings = MENU;  // Куди повертатись з налаштувань
RenderTexture2D gameSnapshot = {0};  // Знімок гри для фону паузи
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
Texture2D settingsBackground;
Texture2D pauseBackground;    // Фон меню паузи
Texture2D backgroundTexture;  // Загальна текстура для фону
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

// Системи розміщення будівель
BuildingPlacer* buildingPlacer = nullptr;
FactionSpawner* factionSpawner = nullptr;

// UI системи
UnitOrderPanel* unitOrderPanel = nullptr;
ResourceDisplay* resourceDisplay = nullptr;

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

// Функція для перемикання псевдоповноекранного режиму
void ToggleWindowedFullscreen() {
    if (!displaySettings.isWindowedFullscreen) {
        // Переходимо в псевдоповноекранний режим
        displaySettings.savedX = GetWindowPosition().x;
        displaySettings.savedY = GetWindowPosition().y;
        
        int monitorWidth = GetMonitorWidth(GetCurrentMonitor());
        int monitorHeight = GetMonitorHeight(GetCurrentMonitor());
        
        SetWindowState(FLAG_WINDOW_UNDECORATED);
        SetWindowPosition(0, 0);
        SetWindowSize(monitorWidth, monitorHeight);
        
        displaySettings.isWindowedFullscreen = true;
        printf("[DISPLAY] Switched to windowed fullscreen: %dx%d\n", monitorWidth, monitorHeight);
    } else {
        // Повертаємось у віконний режим
        ClearWindowState(FLAG_WINDOW_UNDECORATED);
        SetWindowSize(displaySettings.windowWidth, displaySettings.windowHeight);
        SetWindowPosition(displaySettings.savedX, displaySettings.savedY);
        
        displaySettings.isWindowedFullscreen = false;
        printf("[DISPLAY] Switched to windowed mode: %dx%d\n", displaySettings.windowWidth, displaySettings.windowHeight);
    }
}

// Функції для роботи з ресурсами
// UnitCost та getUnitCost визначені в unit_order_panel.h

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

// Знайти юніт за grid координатами
int findUnitAtGrid(GridCoords pos) {
    for (int i = 0; i < units.size(); i++) {
        if (units[i].faction == playerFaction) {
            GridCoords unitPos = units[i].getGridPosition();
            if (unitPos.row == pos.row && unitPos.col == pos.col) {
                return i;
            }
        }
    }
    return -1;
}

// Знайти будівлю за grid координатами
int findBuildingAtGrid(GridCoords pos) {
    for (int i = 0; i < buildings.size(); i++) {
        if (buildings[i].faction == playerFaction) {
            if (buildings[i].occupiesGridCell(pos)) {
                return i;
            }
        }
    }
    return -1;
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
    // Оновлюємо панель замовлення
    if (unitOrderPanel) {
        unitOrderPanel->setSelectedBuilding(-1);
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
    
    // Ініціалізація pathfinding
    pathfindingManager.init(1434, 1075);
    printf("[PATHFINDING] Navigation grid initialized: %dx%d cells\n", 
           pathfindingManager.getGrid().getWidth(), 
           pathfindingManager.getGrid().getHeight());
    
    // Ініціалізація систем розміщення
    if (!buildingPlacer) {
        buildingPlacer = new BuildingPlacer();
    }
    buildingPlacer->init(&pathfindingManager, gameMap);
    
    if (!factionSpawner) {
        factionSpawner = new FactionSpawner();
    }
    factionSpawner->init(buildingPlacer, gameMap, &buildings);
    
    // Автоматичний спавн головних наметів фракцій
    factionSpawner->spawnFactionHQs();
    
    // Фокусуємо камеру на HQ гравця з максимальним зумом
    for (const auto& building : buildings) {
        if (building.faction == playerFaction && 
            (building.type == HQ_ROME || building.type == HQ_CARTHAGE)) {
            mapCamera.target = {(float)building.x, (float)building.y};
            mapCamera.zoom = 1.5f;  // Максимальний зум на HQ
            printf("[CAMERA] Focused on player HQ at (%d, %d) with zoom %.1f\n", 
                   building.x, building.y, mapCamera.zoom);
            break;
        }
    }
    
    // Оновлюємо pathfinding grid з новими будівлями
    pathfindingManager.updateGrid(buildings);
    
    printf("[BUILDINGS] Initialization complete: %d buildings spawned\n", (int)buildings.size());
}

// Ініціалізація ресурсних точок
void InitResources() {
    resources.clear();
    // Ресурси прибрані для тестування будівель
}

// Створення юніта поруч з будівлею
void SpawnUnit(const Building& building, const std::string& unitType) {
    Unit newUnit;
    
    // Початкова позиція - праворуч від будівлі (screen coords)
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
    
    // Конвертуємо screen coords в grid coords для нової системи
    GridCoords spawnGridPos = CoordinateConverter::screenToGrid(ScreenCoords(spawnX, spawnY));
    newUnit.init(unitType, building.faction, spawnGridPos, isAI);
    units.push_back(newUnit);
    
    printf("[SPAWN] Unit spawned at grid(%d, %d) screen(%d, %d)\n", 
           spawnGridPos.row, spawnGridPos.col, spawnX, spawnY);
}

// Функція для малювання меню налаштувань
void DrawSettings() {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    
    // Спочатку малюємо замощену текстуру фону (по висоті)
    if (backgroundTexture.id > 0) {
        float scale = (float)screenH / backgroundTexture.height;
        int scaledWidth = backgroundTexture.width * scale;
        
        for (int x = 0; x < screenW; x += scaledWidth) {
            DrawTexturePro(
                backgroundTexture,
                {0, 0, (float)backgroundTexture.width, (float)backgroundTexture.height},
                {(float)x, 0, (float)scaledWidth, (float)screenH},
                {0, 0}, 0.0f, WHITE
            );
        }
    } else {
        ClearBackground({40, 35, 30, 255});
    }
    
    // Поверх малюємо фон налаштувань зі збереженням пропорцій (по висоті)
    if (settingsBackground.id > 0) {
        float scale = (float)screenH / settingsBackground.height;
        float drawWidth = settingsBackground.width * scale;
        float drawHeight = screenH;
        float offsetX = (screenW - drawWidth) / 2;
        
        DrawTexturePro(
            settingsBackground,
            {0, 0, (float)settingsBackground.width, (float)settingsBackground.height},
            {offsetX, 0, drawWidth, drawHeight},
            {0, 0}, 0.0f, WHITE
        );
    }
    
    // Заголовок відцентрований
    const char* title = "SETTINGS";
    int titleWidth = MeasureText(title, 40);
    DrawText(title, (screenW - titleWidth) / 2, screenH * 0.1f, 40, GOLD);
    
    // Позиції повзунків відцентровані
    int sliderWidth = 400;
    int sliderHeight = 20;
    int sliderX = (screenW - sliderWidth) / 2;
    int labelX = sliderX - 200;
    
    float startY = screenH * 0.25f;
    float spacing = 80;
    
    // Повзунок музики
    DrawText("Music Volume:", labelX, startY, 20, WHITE);
    Rectangle musicSlider = {(float)sliderX, startY, (float)sliderWidth, (float)sliderHeight};
    DrawRectangleRec(musicSlider, DARKGRAY);
    DrawRectangle(sliderX, startY, (int)(sliderWidth * audioSettings.musicVolume), sliderHeight, GREEN);
    DrawText(TextFormat("%.0f%%", audioSettings.musicVolume * 100), sliderX + sliderWidth + 20, startY, 20, WHITE);
    
    // Повзунок середовища
    DrawText("Ambient Volume:", labelX, startY + spacing, 20, WHITE);
    Rectangle ambientSlider = {(float)sliderX, startY + spacing, (float)sliderWidth, (float)sliderHeight};
    DrawRectangleRec(ambientSlider, DARKGRAY);
    DrawRectangle(sliderX, startY + spacing, (int)(sliderWidth * audioSettings.ambientVolume), sliderHeight, BLUE);
    DrawText(TextFormat("%.0f%%", audioSettings.ambientVolume * 100), sliderX + sliderWidth + 20, startY + spacing, 20, WHITE);
    
    // Повзунок ефектів
    DrawText("Effects Volume:", labelX, startY + spacing * 2, 20, WHITE);
    Rectangle effectsSlider = {(float)sliderX, startY + spacing * 2, (float)sliderWidth, (float)sliderHeight};
    DrawRectangleRec(effectsSlider, DARKGRAY);
    DrawRectangle(sliderX, startY + spacing * 2, (int)(sliderWidth * audioSettings.effectsVolume), sliderHeight, RED);
    DrawText(TextFormat("%.0f%%", audioSettings.effectsVolume * 100), sliderX + sliderWidth + 20, startY + spacing * 2, 20, WHITE);
    
    // Чекбокс повноекранного режиму
    DrawText("Fullscreen:", labelX, startY + spacing * 3, 20, WHITE);
    Rectangle checkboxRect = {(float)sliderX, startY + spacing * 3, 30, 30};
    DrawRectangleRec(checkboxRect, DARKGRAY);
    if (displaySettings.isWindowedFullscreen) {
        DrawRectangle(sliderX + 5, startY + spacing * 3 + 5, 20, 20, GREEN);
    }
    
    // Кнопка назад відцентрована
    DynamicButton backButton(0, startY + spacing * 4.5f, "BACK", 20);
    backButton.bounds.x = (screenW - backButton.bounds.width) / 2.0f;
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
    
    // Обробка чекбоксу повноекранного режиму
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, checkboxRect)) {
        ToggleWindowedFullscreen();
    }
    
    // Обробка кнопки назад
    if (backButton.IsClicked()) {
        currentState = returnFromSettings;  // Повертаємось туди звідки прийшли
    }
    
    // Інструкції
    DrawText("Adjust game settings", 430, 510, 16, GRAY);
    
    // Малювання курсора
    DrawCustomCursor();
}

// Функція для малювання меню паузи
void DrawPauseMenu() {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    
    // Спочатку малюємо замощену текстуру фону (по висоті)
    if (backgroundTexture.id > 0) {
        float scale = (float)screenH / backgroundTexture.height;
        int scaledWidth = backgroundTexture.width * scale;
        
        for (int x = 0; x < screenW; x += scaledWidth) {
            DrawTexturePro(
                backgroundTexture,
                {0, 0, (float)backgroundTexture.width, (float)backgroundTexture.height},
                {(float)x, 0, (float)scaledWidth, (float)screenH},
                {0, 0}, 0.0f, WHITE
            );
        }
    } else {
        ClearBackground({40, 35, 30, 255});
    }
    
    // Поверх малюємо основне зображення паузи зі збереженням пропорцій (по висоті)
    if (pauseBackground.id > 0) {
        float scale = (float)screenH / pauseBackground.height;
        float drawWidth = pauseBackground.width * scale;
        float drawHeight = screenH;
        float offsetX = (screenW - drawWidth) / 2;
        
        DrawTexturePro(
            pauseBackground,
            {0, 0, (float)pauseBackground.width, (float)pauseBackground.height},
            {offsetX, 0, drawWidth, drawHeight},
            {0, 0}, 0.0f, WHITE
        );
    } else {
        // Fallback - напівпрозоре затемнення
        DrawRectangle(0, 0, screenW, screenH, {0, 0, 0, 180});
    }
    
    // Заголовок відцентрований
    const char* title = "PAUSE";
    int titleWidth = MeasureText(title, 50);
    DrawText(title, (screenW - titleWidth) / 2, screenH * 0.2f, 50, GOLD);
    
    // Створюємо динамічні кнопки відцентровані
    float centerX = screenW / 2.0f;
    float startY = screenH * 0.35f;
    float buttonSpacing = 110;
    
    DynamicButton resumeButton(0, startY, "RESUME", 24);
    DynamicButton settingsButton(0, startY + buttonSpacing, "SETTINGS", 24);
    DynamicButton mainMenuButton(0, startY + buttonSpacing * 2, "MAIN MENU", 24);
    DynamicButton exitButton(0, startY + buttonSpacing * 3, "EXIT", 24);
    
    resumeButton.bounds.x = centerX - resumeButton.bounds.width / 2.0f;
    settingsButton.bounds.x = centerX - settingsButton.bounds.width / 2.0f;
    mainMenuButton.bounds.x = centerX - mainMenuButton.bounds.width / 2.0f;
    exitButton.bounds.x = centerX - exitButton.bounds.width / 2.0f;
    
    // Оновлюємо стан кнопок
    Vector2 mousePos = GetMousePosition();
    resumeButton.Update(mousePos);
    settingsButton.Update(mousePos);
    mainMenuButton.Update(mousePos);
    exitButton.Update(mousePos);
    
    // Малюємо кнопки
    resumeButton.Draw();
    settingsButton.Draw();
    mainMenuButton.Draw();
    exitButton.Draw();
    
    // Обробка кліків
    if (resumeButton.IsClicked()) {
        currentState = PLAYING;
    }
    if (settingsButton.IsClicked()) {
        returnFromSettings = PAUSE_MENU;  // Запам'ятовуємо що повертаємось в паузу
        currentState = SETTINGS;
    }
    if (mainMenuButton.IsClicked()) {
        currentState = MENU;
        // Повертаємось до меню музики
        if (audioInitialized) {
            SwitchMusic(MUSIC_MENU);
        }
    }
    if (exitButton.IsClicked()) {
        currentState = EXIT;
    }
    
    // Малювання курсора
    DrawCustomCursor();
}

// Функція для малювання меню
void DrawMenu() {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    
    // Спочатку малюємо замощену текстуру фону (по висоті)
    if (backgroundTexture.id > 0) {
        float scale = (float)screenH / backgroundTexture.height;
        int scaledWidth = backgroundTexture.width * scale;
        
        for (int x = 0; x < screenW; x += scaledWidth) {
            DrawTexturePro(
                backgroundTexture,
                {0, 0, (float)backgroundTexture.width, (float)backgroundTexture.height},
                {(float)x, 0, (float)scaledWidth, (float)screenH},
                {0, 0}, 0.0f, WHITE
            );
        }
    } else {
        ClearBackground({40, 35, 30, 255});
    }
    
    // Поверх малюємо основне зображення зі збереженням пропорцій (по висоті)
    if (menuBackground.id > 0) {
        float scale = (float)screenH / menuBackground.height;
        float drawWidth = menuBackground.width * scale;
        float drawHeight = screenH;
        float offsetX = (screenW - drawWidth) / 2;
        
        DrawTexturePro(
            menuBackground,
            {0, 0, (float)menuBackground.width, (float)menuBackground.height},
            {offsetX, 0, drawWidth, drawHeight},
            {0, 0}, 0.0f, WHITE
        );
    }
    
    // Малюємо логотип якщо завантажений
    if (logoLoaded && gameLogo.id > 0) {
        float logoScale = 0.15f;
        float logoWidth = gameLogo.width * logoScale;
        float logoHeight = gameLogo.height * logoScale;
        
        float logoX = (screenW - logoWidth) / 2;
        float logoY = screenH * 0.1f;
        
        DrawTexturePro(
            gameLogo,
            {0, 0, (float)gameLogo.width, (float)gameLogo.height},
            {logoX, logoY, logoWidth, logoHeight},
            {0, 0},
            0.0f,
            WHITE
        );
    }
    
    // Створюємо динамічні кнопки відцентровані
    float centerX = screenW / 2.0f;
    float startY = screenH * 0.45f;
    float buttonSpacing = 110;
    
    DynamicButton startButton(0, startY, "START GAME", 24);
    DynamicButton settingsButton(0, startY + buttonSpacing, "SETTINGS", 24);
    DynamicButton exitButton(0, startY + buttonSpacing * 2, "EXIT", 24);
    
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
        returnFromSettings = MENU;  // Запам'ятовуємо що повертаємось в меню
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
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    
    // Спочатку малюємо замощену текстуру фону (по висоті)
    if (backgroundTexture.id > 0) {
        float scale = (float)screenH / backgroundTexture.height;
        int scaledWidth = backgroundTexture.width * scale;
        
        for (int x = 0; x < screenW; x += scaledWidth) {
            DrawTexturePro(
                backgroundTexture,
                {0, 0, (float)backgroundTexture.width, (float)backgroundTexture.height},
                {(float)x, 0, (float)scaledWidth, (float)screenH},
                {0, 0}, 0.0f, WHITE
            );
        }
    } else {
        ClearBackground({40, 35, 30, 255});
    }
    
    // Поверх малюємо основне зображення зі збереженням пропорцій (по висоті)
    if (factionBackground.id > 0) {
        float scale = (float)screenH / factionBackground.height;
        float drawWidth = factionBackground.width * scale;
        float drawHeight = screenH;
        float offsetX = (screenW - drawWidth) / 2;
        
        DrawTexturePro(
            factionBackground,
            {0, 0, (float)factionBackground.width, (float)factionBackground.height},
            {offsetX, 0, drawWidth, drawHeight},
            {0, 0}, 0.0f, WHITE
        );
    }
    
    // Заголовок відцентрований
    const char* title = "CHOOSE YOUR FACTION";
    int titleWidth = MeasureText(title, 30);
    DrawText(title, (screenW - titleWidth) / 2, screenH * 0.15f, 30, GOLD);
    
    // Створюємо динамічні кнопки відцентровані
    float centerX = screenW / 2.0f;
    float startY = screenH * 0.4f;
    float buttonSpacing = 110;
    
    DynamicButton romeButton(0, startY, "ROME", 30);
    DynamicButton carthageButton(0, startY + buttonSpacing, "CARTHAGE", 30);
    DynamicButton backButton(0, startY + buttonSpacing * 3, "BACK", 20);
    
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

// Обробка кліків
void HandleClicks() {
    Vector2 mousePos = GetMousePosition();
    float currentTime = GetTime();
    
    // Спочатку перевіряємо клік по панелі замовлення юнітів
    if (unitOrderPanel && unitOrderPanel->isVisible()) {
        unitOrderPanel->handleClick(mousePos);
        // Якщо клік був на панелі, не обробляємо далі
        Rectangle panelRect = {10, 950, 300, 100};
        if (CheckCollisionPointRec(mousePos, panelRect)) {
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
            
            // НОВИЙ ПІДХІД: Конвертуємо клік в grid координати
            ScreenCoords screenPos = {mousePos.x, mousePos.y};
            GridCoords gridPos = CoordinateConverter::screenToGrid(screenPos);
            
            // Спочатку перевіряємо клік по юнітах (вони менші) - використовуємо screen coords для точності
            bool foundUnit = false;
            for (int i = 0; i < units.size(); i++) {
                // Перевіряємо тільки юніти гравця
                if (units[i].faction == playerFaction && units[i].isClicked(mousePos)) {
                    units[i].selected = true;
                    selectedUnitIndex = i;
                    lastClickedUnit = i;
                    lastClickTime = currentTime;
                    foundUnit = true;
                    break;
                }
            }
            
            // Якщо не знайшли юніт, перевіряємо будівлі за grid координатами
            if (!foundUnit) {
                int buildingIndex = findBuildingAtGrid(gridPos);
                if (buildingIndex >= 0) {
                    buildings[buildingIndex].selected = true;
                    selectedBuildingIndex = buildingIndex;
                    // Оновлюємо панель замовлення
                    if (unitOrderPanel) {
                        unitOrderPanel->setSelectedBuilding(buildingIndex);
                    }
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
            // Конвертуємо клік в grid координати для руху
            ScreenCoords screenPos = {mousePos.x, mousePos.y};
            GridCoords gridPos = CoordinateConverter::screenToGrid(screenPos);
            
            // Перевіряємо клік по ворожих юнітах для атаки
            bool foundEnemy = false;
            for (int i = 0; i < units.size(); i++) {
                if (units[i].faction != playerFaction && units[i].isClicked(mousePos)) {
                    // Відправляємо бойових юнітів атакувати
                    for (auto& unit : units) {
                        if (unit.selected && unit.faction == playerFaction && unit.attack_damage > 0) {
                            // Рухаємося до grid позиції ворога
                            unit.moveTo(units[i].getGridPosition());
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
                        GridCoords nearestBuildingPos = {-1, -1};
                        float minDist = 999999.0f;
                        
                        for (const auto& building : buildings) {
                            if (building.faction == playerFaction && building.type == QUESTORIUM_ROME) {
                                GridCoords buildingPos = building.getGridPosition();
                                GridCoords resourcePos = resources[i].getGridPosition();
                                float dist = sqrt(pow(buildingPos.row - resourcePos.row, 2) + 
                                                pow(buildingPos.col - resourcePos.col, 2));
                                if (dist < minDist) {
                                    minDist = dist;
                                    nearestBuildingPos = buildingPos;
                                }
                            }
                        }
                        
                        // Якщо не знайшли квесторіум, шукаємо HQ
                        if (nearestBuildingPos.row == -1) {
                            for (const auto& building : buildings) {
                                if (building.faction == playerFaction && 
                                    (building.type == HQ_ROME || building.type == HQ_CARTHAGE)) {
                                    GridCoords buildingPos = building.getGridPosition();
                                    GridCoords resourcePos = resources[i].getGridPosition();
                                    float dist = sqrt(pow(buildingPos.row - resourcePos.row, 2) + 
                                                    pow(buildingPos.col - resourcePos.col, 2));
                                    if (dist < minDist) {
                                        minDist = dist;
                                        nearestBuildingPos = buildingPos;
                                    }
                                }
                            }
                        }
                        
                        // Відправляємо збирачів до ресурсу з призначенням
                        for (auto& unit : units) {
                            if (unit.selected && unit.faction == playerFaction && unit.can_harvest) {
                                if (nearestBuildingPos.row != -1) {
                                    unit.assignResource(resources[i].getGridPosition(), nearestBuildingPos);
                                    printf("Slave assigned to resource at grid(%d, %d), dropoff at grid(%d, %d)\n", 
                                           resources[i].getGridPosition().row, resources[i].getGridPosition().col,
                                           nearestBuildingPos.row, nearestBuildingPos.col);
                                }
                            }
                        }
                        foundResource = true;
                        break;
                    }
                }
                
                // Якщо не клікнули по ресурсу або ворогу, звичайний рух до grid позиції
                if (!foundResource) {
                    // Рухаємо юніти до grid координат
                    for (int i = 0; i < units.size(); i++) {
                        if (units[i].selected && units[i].faction == playerFaction) {
                            // Використовуємо grid координати для руху
                            units[i].moveTo(gridPos);
                            units[i].target_unit_id = -1; // Скидаємо ціль атаки
                            
                            // Скидаємо призначений ресурс при ручному русі
                            if (units[i].can_harvest) {
                                units[i].assigned_resource_x = -1;
                                units[i].assigned_resource_y = -1;
                            }
                            
                            printf("[MOVE] Unit %d moving to grid(%d,%d)\n", i, gridPos.row, gridPos.col);
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
    
    // Керування камерою (WASD + стрілки)
    const float cameraSpeed = 300.0f * GetFrameTime(); // 300 пікселів/секунду
    
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
        mapCamera.target.y -= cameraSpeed;
    }
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
        mapCamera.target.y += cameraSpeed;
    }
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
        mapCamera.target.x -= cameraSpeed;
    }
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
        mapCamera.target.x += cameraSpeed;
    }
    
    // Керування камерою мишею (перетягування середньою кнопкою або правою)
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        Vector2 delta = GetMouseDelta();
        mapCamera.target.x -= delta.x;
        mapCamera.target.y -= delta.y;
    }
    
    // Зум камери (коліщатко миші)
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        mapCamera.zoom += wheel * 0.1f;
        // Обмеження зуму
        if (mapCamera.zoom < 0.5f) mapCamera.zoom = 0.5f;
        if (mapCamera.zoom > 4.0f) mapCamera.zoom = 4.0f;
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
            if (CheckCollisionRecs(unitRect, building.getCollisionRect())) {
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
    
    // Початок режиму 2D камери для карти та об'єктів
    BeginMode2D(mapCamera);
    
    // Малювання ізометричної карти (фон)
    if (mapRenderer && gameMap) {
        mapRenderer->setCamera(mapCamera);
        if (mapRenderer->isTilesetLoaded()) {
            mapRenderer->render(*gameMap);
        } else {
            mapRenderer->renderDebug(*gameMap);
        }
    }
    
    // НОВИЙ ПІДХІД: Використовуємо RenderQueue для правильного depth sorting
    RenderQueue renderQueue;
    
    // Додаємо всі об'єкти до черги
    for (int i = 0; i < units.size(); i++) {
        renderQueue.addUnit(i, units[i].getGridPosition());
    }
    
    for (int i = 0; i < buildings.size(); i++) {
        renderQueue.addBuilding(i, buildings[i].getGridPosition(), buildings[i].footprint);
    }
    
    for (int i = 0; i < resources.size(); i++) {
        renderQueue.addResource(i, resources[i].getGridPosition());
    }
    
    // Сортуємо та рендеримо в правильному порядку (back-to-front)
    renderQueue.sort();
    renderQueue.render(units, buildings, resources);
    
    // Кінець режиму 2D камери
    EndMode2D();
    
    // HUD - панель ресурсів (нова система)
    if (resourceDisplay) {
        if (playerFaction == ROME) {
            resourceDisplay->draw(rome_food, rome_money, rome_food_reserved, rome_money_reserved);
        } else {
            resourceDisplay->draw(carth_food, carth_money, carth_food_reserved, carth_money_reserved);
        }
    }
    
    // Інформація про вибраний об'єкт (малюється поверх камери)
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
    
    // Панель замовлення юнітів (нова система)
    if (unitOrderPanel) {
        unitOrderPanel->draw();
    }
    
    // Малювання курсора
    DrawCustomCursor();
    
    // Кнопка меню в правому верхньому куті
    int screenW = GetScreenWidth();
    DynamicButton menuButton(screenW - 150, 10, "MENU", 20);
    Vector2 mousePos = GetMousePosition();
    menuButton.Update(mousePos);
    menuButton.Draw();
    
    if (menuButton.IsClicked()) {
        currentState = PAUSE_MENU;
    }
    
    // Інструкції
    DrawText("LMB - Select | RMB - Move/Attack/Harvest | Drag - Area | 2xClick - All type", 10, 1040, 14, LIGHTGRAY);
    
    // Перевірка виходу в меню паузи (залишаємо Escape як альтернативу)
    if (IsKeyPressed(KEY_ESCAPE)) {
        currentState = PAUSE_MENU;
    }
}

int main() {
    // Ініціалізація вікна (збільшено на 40%)
    const int screenWidth = 1434; // 1024 * 1.4
    const int screenHeight = 1075; // 768 * 1.4
    
    InitWindow(screenWidth, screenHeight, "Punic Wars: Castra");
    SetExitKey(0); // Вимикаємо автоматичне закриття на Escape
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
    
    // Завантаження фону налаштувань
    settingsBackground = LoadTexture("assets/Settings_background.png");
    
    // Завантаження фону меню паузи
    pauseBackground = LoadTexture("assets/Pause_background.png");
    if (pauseBackground.id > 0) {
        printf("[TEXTURE] Pause background loaded: %dx%d\n", pauseBackground.width, pauseBackground.height);
    } else {
        printf("[TEXTURE] Warning: Pause background not loaded!\n");
    }
    
    // Завантаження загальної текстури фону (SVG не підтримується, використаємо PNG якщо є)
    backgroundTexture = LoadTexture("assets/background_texture.png");
    if (backgroundTexture.id == 0) {
        // Якщо PNG немає, створюємо fallback текстуру
        Image fallbackImg = GenImageColor(256, 256, {40, 35, 30, 255});
        backgroundTexture = LoadTextureFromImage(fallbackImg);
        UnloadImage(fallbackImg);
        printf("[TEXTURE] Using fallback background texture\n");
    } else {
        printf("[TEXTURE] Background texture loaded: %dx%d\n", backgroundTexture.width, backgroundTexture.height);
    }
    
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
    
    // Завантаження текстур будівель
    printf("[TEXTURE] Loading building textures...\n");
    BuildingTextureManager::getInstance().loadAllTextures();
    
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
    
    // Ініціалізація генератора карти (збільшено розмір до 80x80)
    mapGenerator = new MapGenerator(time(nullptr));
    gameMap = new TileMap(mapGenerator->generate(80, 80));
    mapRenderer = new IsometricRenderer();
    
    // Завантаження тайлсету
    if (FileExists("assets/isometric_tileset.png")) {
        mapRenderer->loadTileset("assets/isometric_tileset.png");
        printf("[TILEMAP] Tileset loaded successfully\n");
    } else {
        printf("[TILEMAP] Warning: Tileset not found, using debug rendering\n");
    }
    
    // Налаштування камери для карти (початкова позиція - центр карти)
    // Для ізометричної проекції карти 80x80:
    // Центр знаходиться в точці (row=40, col=40)
    // screenX = (40 - 40) * 64 = 0
    // screenY = (40 + 40) * 32 = 2560
    // Але щоб бачити всю карту, потрібно відступити вгору
    mapCamera.target = {0, 1280};  // Зміщуємо камеру вище для кращого огляду
    mapCamera.offset = {screenWidth / 2.0f, screenHeight / 2.0f};
    mapCamera.rotation = 0.0f;
    mapCamera.zoom = 0.5f;  // Зменшуємо зум щоб бачити більше карти
    mapRenderer->setCamera(mapCamera);
    
    printf("[TILEMAP] Map generated: %dx%d, %.1f%% passable\n", 
           gameMap->getWidth(), gameMap->getHeight(), 
           gameMap->getPassablePercentage() * 100.0f);
    
    // Ініціалізація UI систем
    unitOrderPanel = new UnitOrderPanel();
    unitOrderPanel->init(&buildings, playerFaction);
    
    resourceDisplay = new ResourceDisplay();
    resourceDisplay->init(resourcePanel, playerFaction);
    
    printf("[UI] UI systems initialized\n");
    
    // Створюємо RenderTexture для знімку гри
    gameSnapshot = LoadRenderTexture(screenWidth, screenHeight);
    printf("[UI] Game snapshot texture created: %dx%d\n", screenWidth, screenHeight);
    
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
            case PAUSE_MENU:
                DrawPauseMenu();
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
    UnloadTexture(settingsBackground);
    UnloadTexture(pauseBackground);
    UnloadTexture(backgroundTexture);
    if (logoLoaded) UnloadTexture(gameLogo);
    if (fontLoaded) UnloadFont(customFont);
    UnloadTexture(cursorIdle);
    UnloadTexture(cursorHover);
    UnloadTexture(cursorClick);
    UnloadTexture(resourcePanel);
    UnloadTexture(buttonBase);
    UnloadTexture(buttonHover);
    
    // Вивантаження RenderTexture для знімку гри
    if (gameSnapshot.id > 0) {
        UnloadRenderTexture(gameSnapshot);
    }
    
    // Вивантаження текстур динамічних кнопок
    DynamicButton::UnloadTextures();
    
    // Очищення генератора карти
    if (mapRenderer) {
        mapRenderer->unloadTileset();
        delete mapRenderer;
    }
    if (gameMap) delete gameMap;
    if (mapGenerator) delete mapGenerator;
    
    // Очищення систем розміщення
    if (buildingPlacer) delete buildingPlacer;
    if (factionSpawner) delete factionSpawner;
    if (unitOrderPanel) delete unitOrderPanel;
    if (resourceDisplay) delete resourceDisplay;
    
    CloseAudioDevice();
    
    // Закриття
    CloseWindow();
    return 0;
}