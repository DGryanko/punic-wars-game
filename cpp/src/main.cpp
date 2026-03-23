#include "raylib.h"
#include <algorithm>
#include <cstring>
#include "building.h"
#include "building_texture_manager.h"
#include "building_renderer.h"
#include "building_placer.h"
#include "faction_spawner.h"
#include "unit_order_panel.h"
#include "slave_build_panel.h"
#include "resource_display.h"
#include "unit.h"
#include "resource.h"
#include "pathfinding.h"
#include "ui_button.h"
#include "tilemap/tilemap_generator.h"
#include "tilemap/coordinates.h"
#include "render_queue.h"
#include "game_constants.h"
#include "debug_logger.h"
#include <vector>
#include <cmath>
#include <ctime>

// Стани гри
enum GameState {
    MENU,
    SETTINGS,
    FACTION_SELECT,
    PLAYING,
    PAUSE_MENU,
    VICTORY_SCREEN,
    DEFEAT_SCREEN,
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
    MUSIC_DEFEAT,         // Поразка
    MUSIC_VICTORY         // Перемога
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
    int windowWidth = GameConstants::UI::WINDOW_WIDTH;
    int windowHeight = GameConstants::UI::WINDOW_HEIGHT;
    int savedX = 0;
    int savedY = 0;
};

AudioSettings audioSettings;
DisplaySettings displaySettings;

// Debug налаштування
bool showDebugVisuals = false; // Показувати debug візуалізацію (кліки, шляхи)

// Глобальні змінні
GameState currentState = MENU;
GameState returnFromSettings = MENU;  // Куди повертатись з налаштувань
RenderTexture2D gameSnapshot = {0};  // Знімок гри для фону паузи
Faction playerFaction = ROME; // Фракція гравця
int rome_food = GameConstants::StartingResources::ROME_FOOD;
int rome_money = GameConstants::StartingResources::ROME_MONEY;
int rome_food_reserved = 0;  // Зарезервовані ресурси
int rome_money_reserved = 0;
int carth_food = GameConstants::StartingResources::CARTHAGE_FOOD;
int carth_money = GameConstants::StartingResources::CARTHAGE_MONEY;
int carth_food_reserved = 0;
int carth_money_reserved = 0;

// Ліміти зберігання ресурсів
int rome_food_cap = 500;
int rome_money_cap = 500;
int carth_food_cap = 500;
int carth_money_cap = 500;
const int BASE_RESOURCE_CAP = 500;
const int QUESTORIUM_RESOURCE_CAP = 5000; // x10 при будівництві квесторія

// Музика та звуки
Music menuMusic;
Music ambientMusic[2];          // 2 амбієнт треки
Music battleMusicRome[2];       // 2 бойові треки Риму
Music battleMusicCarthage[2];   // 2 бойові треки Карфагену
Music suspenseMusic[2];         // 2 треки напруги
Music peacefulMusic[2];         // 2 мирні треки
Music defeatMusic[2];           // 2 треки поразки
Music victoryMusic;             // Трек перемоги Риму
Music currentMusic;
Music nextMusic;                // Наступний трек для crossfade
MusicState currentMusicState = MUSIC_MENU;
MusicState previousMusicState = MUSIC_MENU;
Texture2D menuBackground;
Texture2D factionBackground;
Texture2D settingsBackground;
Texture2D pauseBackground;    // Фон меню паузи
Texture2D victoryBackground;  // Фон екрану перемоги
Texture2D defeatBackground;   // Фон екрану поразки
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
SlaveBuildPanel* slaveBuildPanel = nullptr;
ResourceDisplay* resourceDisplay = nullptr;

// Система виділення областю
bool isDragging = false;
Vector2 dragStart = {0, 0};
Vector2 dragEnd = {0, 0};

// DEBUG: Останні кліки для візуалізації
std::vector<Vector2> debugClicks;
const int MAX_DEBUG_CLICKS = 10;

// Маркери руху (жовтий = рух, червоний = атака)
struct MoveMarker {
    Vector2 pos;
    float   timer;
    bool    isAttack;   // червоний
    bool    isHarvest;  // зелений
};
std::vector<MoveMarker> moveMarkers;

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
        case MUSIC_VICTORY:
            nextMusic = victoryMusic;
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
            case MUSIC_VICTORY:
                currentMusic = victoryMusic;
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
    // Не перемикаємо якщо грає музика перемоги або поразки
    if (currentMusicState == MUSIC_VICTORY || currentMusicState == MUSIC_DEFEAT) {
        return;
    }

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
        displaySettings.savedX = GetWindowPosition().x;
        displaySettings.savedY = GetWindowPosition().y;
        int monitorWidth = GetMonitorWidth(GetCurrentMonitor());
        int monitorHeight = GetMonitorHeight(GetCurrentMonitor());
        SetWindowState(FLAG_WINDOW_UNDECORATED);
        SetWindowPosition(0, 0);
        SetWindowSize(monitorWidth, monitorHeight);
        displaySettings.isWindowedFullscreen = true;
        LOG_BUILDING("[DISPLAY] Switched to windowed fullscreen: %dx%d\n", monitorWidth, monitorHeight);
    } else {
        ClearWindowState(FLAG_WINDOW_UNDECORATED);
        SetWindowSize(displaySettings.windowWidth, displaySettings.windowHeight);
        SetWindowPosition(displaySettings.savedX, displaySettings.savedY);
        displaySettings.isWindowedFullscreen = false;
        LOG_BUILDING("[DISPLAY] Switched to windowed mode: %dx%d\n", displaySettings.windowWidth, displaySettings.windowHeight);
    }
}

static const char* SETTINGS_FILE = "settings.ini";

void SaveSettings() {
    FILE* f = fopen(SETTINGS_FILE, "w");
    if (!f) return;
    fprintf(f, "fullscreen=%d\n",      displaySettings.isWindowedFullscreen ? 1 : 0);
    fprintf(f, "music=%.3f\n",         audioSettings.musicVolume);
    fprintf(f, "ambient=%.3f\n",       audioSettings.ambientVolume);
    fprintf(f, "effects=%.3f\n",       audioSettings.effectsVolume);
    fclose(f);
}

void LoadSettings() {
    FILE* f = fopen(SETTINGS_FILE, "r");
    if (!f) return;
    char key[32]; float val;
    while (fscanf(f, "%31[^=]=%f\n", key, &val) == 2) {
        if      (strcmp(key, "fullscreen") == 0) displaySettings.isWindowedFullscreen = (val > 0.5f);
        else if (strcmp(key, "music")      == 0) audioSettings.musicVolume   = val;
        else if (strcmp(key, "ambient")    == 0) audioSettings.ambientVolume = val;
        else if (strcmp(key, "effects")    == 0) audioSettings.effectsVolume = val;
    }
    fclose(f);
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
    LOG_FIND("[FIND] Looking for building at grid (%d, %d), player faction: %d\n", 
             pos.row, pos.col, playerFaction);
    LOG_FIND("[FIND] Total buildings: %d\n", (int)buildings.size());
    
    for (int i = 0; i < buildings.size(); i++) {
        LOG_FIND("[FIND] Building %d: %s, faction: %d, position: (%d, %d), footprint: (%d, %d)\n",
                 i, buildings[i].name.c_str(), buildings[i].faction,
                 buildings[i].position.row, buildings[i].position.col,
                 buildings[i].footprint.row, buildings[i].footprint.col);
        
        if (buildings[i].faction == playerFaction) {
            if (buildings[i].occupiesGridCell(pos)) {
                LOG_FIND("[FIND] Found matching building at index %d\n", i);
                return i;
            }
        }
    }
    LOG_FIND("[FIND] No building found\n");
    return -1;
}

// Альтернативний метод: шукати будівлю по світовим координатам (враховує текстуру)
int findBuildingAtWorldPos(Vector2 worldPos) {
    LOG_FIND("[FIND_WORLD] Looking for building at world (%.1f, %.1f), player faction: %d\n", 
             worldPos.x, worldPos.y, playerFaction);
    
    for (int i = 0; i < buildings.size(); i++) {
        if (buildings[i].faction == playerFaction) {
            // Використовуємо isClicked() замість getRect() для точної перевірки ромба
            if (buildings[i].isClicked(worldPos)) {
                LOG_FIND("[FIND_WORLD] Found matching building at index %d\n", i);
                return i;
            }
        }
    }
    LOG_FIND("[FIND_WORLD] No building found\n");
    return -1;
}

// Перевірка чи grid клітинка зайнята будівлею
bool isGridCellOccupiedByBuilding(GridCoords pos) {
    for (const auto& building : buildings) {
        if (building.occupiesGridCell(pos)) {
            return true;
        }
    }
    return false;
}

// Перевірка чи шлях до цілі перетинається з будівлею
bool isPathBlockedByBuilding(GridCoords from, GridCoords to) {
    // Проста перевірка - чи ціль зайнята будівлею
    if (isGridCellOccupiedByBuilding(to)) {
        return true;
    }
    return false;
}

// Знайти найближчий прохідний тайл поруч з позицією (для руху до будівель)
GridCoords findNearestWalkableTile(GridCoords target) {
    if (!isGridCellOccupiedByBuilding(target)) {
        return target; // Вже вільний
    }
    // Шукаємо по спіралі навколо цілі
    for (int radius = 1; radius <= 5; radius++) {
        for (int dr = -radius; dr <= radius; dr++) {
            for (int dc = -radius; dc <= radius; dc++) {
                if (abs(dr) != radius && abs(dc) != radius) continue; // Тільки периметр
                GridCoords candidate = {target.row + dr, target.col + dc};
                if (candidate.row < 0 || candidate.row >= 80 || 
                    candidate.col < 0 || candidate.col >= 80) continue;
                if (!isGridCellOccupiedByBuilding(candidate)) {
                    return candidate;
                }
            }
        }
    }
    return target; // Fallback
}

// Рух юніта з A* pathfinding (обходить будівлі) — по grid координатах
void moveUnitWithPath(Unit& unit, GridCoords targetPos) {
    ScreenCoords startScreen = unit.getScreenPosition();
    ScreenCoords goalScreen = CoordinateConverter::gridToScreen(targetPos);
    
    AStarPathfinder pathfinder;
    std::vector<Vector2> path = pathfinder.findPath(
        {startScreen.x, startScreen.y},
        {goalScreen.x, goalScreen.y},
        pathfindingManager.getGrid()
    );
    
    if (!path.empty()) {
        unit.setPath(path);
    } else {
        unit.moveTo(targetPos);
    }
}

// Рух юніта з A* pathfinding — по точних world (screen) координатах
void moveUnitWithPathToScreen(Unit& unit, ScreenCoords goalScreen) {
    ScreenCoords startScreen = unit.getScreenPosition();
    
    AStarPathfinder pathfinder;
    std::vector<Vector2> path = pathfinder.findPath(
        {startScreen.x, startScreen.y},
        {goalScreen.x, goalScreen.y},
        pathfindingManager.getGrid()
    );
    
    if (!path.empty()) {
        unit.setPath(path);
    } else {
        // Fallback: конвертуємо в grid і рухаємо напряму
        GridCoords gridPos = CoordinateConverter::screenToGrid(goalScreen);
        unit.moveTo(gridPos);
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
    // Оновлюємо панель замовлення
    if (unitOrderPanel) {
        unitOrderPanel->setSelectedBuilding(-1);
    }
    // Оновлюємо панель будівництва
    if (slaveBuildPanel) {
        slaveBuildPanel->setSelectedUnit(-1);
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
        if (unit.faction != playerFaction || !unit.can_harvest) continue;
        
        // Якщо раб має призначену ресурсну точку - автоматичний цикл
        if (unit.hasAssignedResource()) {
            GridCoords unitPos = unit.getGridPosition();
            GridCoords resourcePos = unit.assigned_resource_position;
            GridCoords dropoffPos = unit.assigned_dropoff_position;
            
            // Перевіряємо чи ресурс ще існує
            bool resourceExists = false;
            ResourcePoint* targetResource = nullptr;
            for (auto& resource : resources) {
                if (!resource.depleted) {
                    GridCoords resPos = resource.getGridPosition();
                    if (resPos.row == resourcePos.row && resPos.col == resourcePos.col) {
                        resourceExists = true;
                        targetResource = &resource;
                        break;
                    }
                }
            }
            
            // Якщо ресурс вичерпано, скидаємо призначення
            if (!resourceExists) {
                unit.clearResourceAssignment();
                unit.stopHarvesting();
                LOG_BUILDING("[HARVEST] Resource depleted, stopping cycle\n");
                continue;
            }
            
            // STATE MACHINE: Автоматичний цикл збору
            
            // STATE 1: Якщо раб повний - йти здавати ресурси
            if (!unit.canCarryMore() && (unit.carrying_food > 0 || unit.carrying_gold > 0)) {
                unit.is_harvesting = false;
                
                // Обчислюємо відстань до будівлі здачі
                float distToDropoff = sqrt(pow(unitPos.row - dropoffPos.row, 2) + 
                                          pow(unitPos.col - dropoffPos.col, 2));
                
                // Якщо поруч з будівлею (в межах 2 тайлів)
                if (distToDropoff < 2.0f) {
                    // Здати ресурси (з урахуванням ліміту)
                    int food, gold;
                    unit.dropResources(food, gold);
                    
                    if (unit.faction == ROME) {
                        rome_food = std::min(rome_food + food, rome_food_cap);
                        rome_money = std::min(rome_money + gold, rome_money_cap);
                    } else {
                        carth_food = std::min(carth_food + food, carth_food_cap);
                        carth_money = std::min(carth_money + gold, carth_money_cap);
                    }
                    
                    LOG_BUILDING("[DROP] Resources dropped: food=%d, gold=%d. Total: food=%d money=%d\n", 
                           food, gold, rome_food, rome_money);
                    
                    // Повернутися до ресурсу
                    moveUnitWithPath(unit, resourcePos);
                    printf("[RETURN] Returning to resource at grid(%d,%d)\n", resourcePos.row, resourcePos.col);
                } else {
                    // Рухаємося до будівлі здачі
                    if (!unit.is_moving || !(unit.final_destination == dropoffPos)) {
                        moveUnitWithPath(unit, dropoffPos);
                        printf("[MOVE_TO_DROPOFF] Moving to dropoff at grid(%d,%d), distance=%.1f\n", 
                               dropoffPos.row, dropoffPos.col, distToDropoff);
                    }
                }
            }
            // STATE 2: Якщо раб не повний - збирати ресурс
            else {
                // Обчислюємо відстань до ресурсу
                float distToResource = sqrt(pow(unitPos.row - resourcePos.row, 2) + 
                                           pow(unitPos.col - resourcePos.col, 2));
                
                // Якщо поруч з ресурсом (в межах 2 тайлів)
                if (distToResource < 2.0f) {
                    // Почати збір
                    unit.startHarvesting();
                    unit.is_moving = false;
                    
                    // Збираємо ресурс
                    int harvestAmount = 1; // 1 одиниця за кадр
                    int harvested = targetResource->harvest(harvestAmount);
                    
                    if (harvested > 0) {
                        if (targetResource->type == FOOD_SOURCE) {
                            unit.addResources(harvested, 0);
                        } else if (targetResource->type == GOLD_SOURCE) {
                            unit.addResources(0, harvested);
                        }
                        
                        // Логуємо тільки кожні 30 кадрів
                        static int harvestLogCounter = 0;
                        if (++harvestLogCounter % 30 == 0) {
                            LOG_BUILDING("[HARVEST] Slave harvesting. Carrying: %d/%d\n", 
                                   unit.carrying_food + unit.carrying_gold, unit.max_carry_capacity);
                        }
                    }
                } else {
                    // Рухаємося до ресурсу
                    if (!unit.is_moving || !(unit.final_destination == resourcePos)) {
                        moveUnitWithPath(unit, resourcePos);
                        printf("[MOVE_TO_RESOURCE] Moving to resource at grid(%d,%d), distance=%.1f\n", 
                               resourcePos.row, resourcePos.col, distToResource);
                    }
                }
            }
        }
    }
}

// Ініціалізація будівель
void InitBuildings() {
    buildings.clear();
    
    // Ініціалізація pathfinding з TileMap
    pathfindingManager.init(gameMap);
    // NOTE: Pathfinding logs temporarily disabled to reduce console spam
    // printf("[PATHFINDING] Navigation grid initialized: %dx%d tiles\n", 
    //        pathfindingManager.getGrid().getWidth(), 
    //        pathfindingManager.getGrid().getHeight());
    
    // Ініціалізація систем розміщення
    if (!buildingPlacer) {
        buildingPlacer = new BuildingPlacer();
    }
    buildingPlacer->init(&pathfindingManager, gameMap);
    
    if (!factionSpawner) {
        factionSpawner = new FactionSpawner();
    }
    factionSpawner->init(buildingPlacer, gameMap, &buildings);
    
    // Спавн тільки ворожого HQ (гравець будує свій сам)
    factionSpawner->spawnEnemyHQ(playerFaction);
    
    // Оновлюємо pathfinding grid з новими будівлями
    pathfindingManager.updateGrid(buildings);
    
    // Переініціалізуємо UI панелі з правильною фракцією
    if (unitOrderPanel) {
        unitOrderPanel->init(&buildings, playerFaction);
    }
    if (slaveBuildPanel) {
        slaveBuildPanel->init(&units, &buildings, playerFaction);
    }
    
    printf("[BUILDINGS] Initialization complete: %d buildings spawned\n", (int)buildings.size());
}

// Ініціалізація ресурсних точок
void InitResources() {
    resources.clear();
    
    // Генеруємо випадкові ресурсні точки на мапі
    const int NUM_FOOD_SOURCES = 8;  // 8 джерел їжі
    const int NUM_GOLD_SOURCES = 6;  // 6 джерел золота
    const int MAX_ATTEMPTS = 100;    // Максимум спроб знайти вільне місце
    
    LOG_SPAWN("[RESOURCES] Spawning resource points...\n");
    
    // Спавн джерел їжі
    for (int i = 0; i < NUM_FOOD_SOURCES; i++) {
        bool placed = false;
        for (int attempt = 0; attempt < MAX_ATTEMPTS && !placed; attempt++) {
            int row = rand() % gameMap->getHeight();
            int col = rand() % gameMap->getWidth();
            
            // Перевіряємо що тайл прохідний та вільний
            if (gameMap->isPassable(row, col) && buildingPlacer->isTileFree(row, col)) {
                // Перевіряємо що немає інших ресурсів поблизу (мінімум 3 тайли)
                bool tooClose = false;
                for (const auto& res : resources) {
                    int dist = abs(res.position.row - row) + abs(res.position.col - col);
                    if (dist < 3) {
                        tooClose = true;
                        break;
                    }
                }
                
                if (!tooClose) {
                    ResourcePoint food;
                    food.init(FOOD_SOURCE, GridCoords(row, col), 500 + rand() % 500); // 500-1000 їжі
                    resources.push_back(food);
                    placed = true;
                    LOG_SPAWN("[RESOURCES] Food source spawned at (%d, %d) with %d amount\n", 
                              row, col, food.amount);
                }
            }
        }
        
        if (!placed) {
            LOG_WARNING("[RESOURCES] Warning: Could not place food source %d\n", i);
        }
    }
    
    // Спавн джерел золота
    for (int i = 0; i < NUM_GOLD_SOURCES; i++) {
        bool placed = false;
        for (int attempt = 0; attempt < MAX_ATTEMPTS && !placed; attempt++) {
            int row = rand() % gameMap->getHeight();
            int col = rand() % gameMap->getWidth();
            
            // Перевіряємо що тайл прохідний та вільний
            if (gameMap->isPassable(row, col) && buildingPlacer->isTileFree(row, col)) {
                // Перевіряємо що немає інших ресурсів поблизу (мінімум 3 тайли)
                bool tooClose = false;
                for (const auto& res : resources) {
                    int dist = abs(res.position.row - row) + abs(res.position.col - col);
                    if (dist < 3) {
                        tooClose = true;
                        break;
                    }
                }
                
                if (!tooClose) {
                    ResourcePoint gold;
                    gold.init(GOLD_SOURCE, GridCoords(row, col), 300 + rand() % 400); // 300-700 золота
                    resources.push_back(gold);
                    placed = true;
                    LOG_SPAWN("[RESOURCES] Gold source spawned at (%d, %d) with %d amount\n", 
                              row, col, gold.amount);
                }
            }
        }
        
        if (!placed) {
            LOG_WARNING("[RESOURCES] Warning: Could not place gold source %d\n", i);
        }
    }
    
    LOG_SPAWN("[RESOURCES] Initialization complete: %d resources spawned\n", (int)resources.size());
}

// Створення юніта поруч з будівлею
void SpawnUnit(const Building& building, const std::string& unitType) {
    Unit newUnit;
    
    // Спавн на 2 тайли праворуч-вниз від будівлі (вихід з Praetorium)
    GridCoords buildingPos = building.getGridPosition();
    GridCoords spawnGrid = {
        buildingPos.row + building.footprint.row + 1,  // нижче будівлі
        buildingPos.col + building.footprint.col + 1   // правіше будівлі
    };
    
    // Шукаємо вільний тайл поруч зі спавн-позицією
    bool foundFree = false;
    for (int radius = 0; radius <= 5 && !foundFree; radius++) {
        for (int dy = -radius; dy <= radius && !foundFree; dy++) {
            for (int dx = -radius; dx <= radius && !foundFree; dx++) {
                int nrow = spawnGrid.row + dy;
                int ncol = spawnGrid.col + dx;
                if (nrow >= 0 && nrow < 80 && ncol >= 0 && ncol < 80) {
                    if (pathfindingManager.getGrid().isWalkable(nrow, ncol)) {
                        spawnGrid = {nrow, ncol};
                        foundFree = true;
                    }
                }
            }
        }
    }
    
    if (!foundFree) {
        LOG_SPAWN("[SPAWN] Warning: Could not find free spawn position for unit!\n");
    }
    
    bool isAI = (building.faction != playerFaction);
    newUnit.init(unitType, building.faction, spawnGrid, isAI);
    units.push_back(std::move(newUnit));
    
    printf("[SPAWN] Unit '%s' spawned at grid(%d, %d)\n", unitType.c_str(), spawnGrid.row, spawnGrid.col);
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

    // Чекбокс debug візуалізації
    DrawText("Debug visuals:", labelX, startY + spacing * 4, 20, WHITE);
    Rectangle debugCheckRect = {(float)sliderX, startY + spacing * 4, 30, 30};
    DrawRectangleRec(debugCheckRect, DARKGRAY);
    DrawRectangleLines((int)debugCheckRect.x, (int)debugCheckRect.y, 30, 30, WHITE);
    if (showDebugVisuals) {
        DrawRectangle(sliderX + 5, (int)(startY + spacing * 4) + 5, 20, 20, ORANGE);
    }
    DrawText("(click markers, unit paths)", sliderX + 40, (int)(startY + spacing * 4) + 5, 16, LIGHTGRAY);

    // Кнопка назад відцентрована
    DynamicButton backButton(0, startY + spacing * 5.5f, "BACK", 20);
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
        SaveSettings();
    }

    // Обробка чекбоксу debug візуалізації
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, debugCheckRect)) {
        showDebugVisuals = !showDebugVisuals;
    }
    
    // Обробка кнопки назад
    if (backButton.IsClicked()) {
        SaveSettings();
        currentState = returnFromSettings;
    }
    
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

// ─── Допоміжна функція: малює фон екрану (victory або defeat) ───────────────
static void DrawEndScreenBackground(Texture2D& bg) {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    if (bg.id > 0) {
        float scale = (float)screenH / bg.height;
        float drawWidth = bg.width * scale;
        float offsetX = (screenW - drawWidth) / 2.0f;
        DrawTexturePro(bg,
            {0, 0, (float)bg.width, (float)bg.height},
            {offsetX, 0, drawWidth, (float)screenH},
            {0, 0}, 0.0f, WHITE);
    } else {
        ClearBackground({20, 15, 10, 255});
    }
}

// ─── Екран перемоги ──────────────────────────────────────────────────────────
void DrawVictoryScreen() {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    DrawEndScreenBackground(victoryBackground);

    // Заголовок
    const char* title = "VICTORY!";
    int titleSize = 64;
    int titleW = MeasureText(title, titleSize);
    DrawText(title, (screenW - titleW) / 2, (int)(screenH * 0.18f), titleSize, GOLD);

    // Кнопки
    float centerX = screenW / 2.0f;
    float startY  = screenH * 0.38f;
    float spacing = 110.0f;

    DynamicButton newGameBtn(0, startY,              "START GAME",    24);
    DynamicButton menuBtn   (0, startY + spacing,    "MAIN MENU",     24);
    DynamicButton exitBtn   (0, startY + spacing * 2,"EXIT",          24);

    newGameBtn.bounds.x = centerX - newGameBtn.bounds.width / 2.0f;
    menuBtn.bounds.x    = centerX - menuBtn.bounds.width    / 2.0f;
    exitBtn.bounds.x    = centerX - exitBtn.bounds.width    / 2.0f;

    Vector2 mousePos = GetMousePosition();
    newGameBtn.Update(mousePos);
    menuBtn.Update(mousePos);
    exitBtn.Update(mousePos);

    newGameBtn.Draw();
    menuBtn.Draw();
    exitBtn.Draw();

    if (newGameBtn.IsClicked()) {
        currentState = FACTION_SELECT;
        if (audioInitialized) SwitchMusic(MUSIC_MENU);
    }
    if (menuBtn.IsClicked()) {
        currentState = MENU;
        if (audioInitialized) SwitchMusic(MUSIC_MENU);
    }
    if (exitBtn.IsClicked()) {
        currentState = EXIT;
    }

    DrawCustomCursor();
}

// ─── Екран поразки ───────────────────────────────────────────────────────────
void DrawDefeatScreen() {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    DrawEndScreenBackground(defeatBackground);

    // Заголовок
    const char* title = "DEFEAT";
    int titleSize = 64;
    int titleW = MeasureText(title, titleSize);
    DrawText(title, (screenW - titleW) / 2, (int)(screenH * 0.18f), titleSize, RED);

    // Кнопки
    float centerX = screenW / 2.0f;
    float startY  = screenH * 0.38f;
    float spacing = 110.0f;

    DynamicButton newGameBtn(0, startY,              "START GAME",    24);
    DynamicButton menuBtn   (0, startY + spacing,    "MAIN MENU",     24);
    DynamicButton exitBtn   (0, startY + spacing * 2,"EXIT",          24);

    newGameBtn.bounds.x = centerX - newGameBtn.bounds.width / 2.0f;
    menuBtn.bounds.x    = centerX - menuBtn.bounds.width    / 2.0f;
    exitBtn.bounds.x    = centerX - exitBtn.bounds.width    / 2.0f;

    Vector2 mousePos = GetMousePosition();
    newGameBtn.Update(mousePos);
    menuBtn.Update(mousePos);
    exitBtn.Update(mousePos);

    newGameBtn.Draw();
    menuBtn.Draw();
    exitBtn.Draw();

    if (newGameBtn.IsClicked()) {
        currentState = FACTION_SELECT;
        if (audioInitialized) SwitchMusic(MUSIC_MENU);
    }
    if (menuBtn.IsClicked()) {
        currentState = MENU;
        if (audioInitialized) SwitchMusic(MUSIC_MENU);
    }
    if (exitBtn.IsClicked()) {
        currentState = EXIT;
    }

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
        // Спавн раба на випадковій вільній позиції (як раніше HQ)
        {
            GridCoords spawnPos = {-1, -1};
            for (int attempt = 0; attempt < 200 && spawnPos.row == -1; attempt++) {
                int row = 5 + rand() % 70;
                int col = 5 + rand() % 70;
                if (pathfindingManager.getGrid().isWalkable(row, col)) {
                    spawnPos = {row, col};
                }
            }
            if (spawnPos.row == -1) spawnPos = {10, 10};
            Unit slave;
            slave.init("slave", playerFaction, spawnPos, false);
            units.push_back(std::move(slave));
            // Фокус камери на рабі
            ScreenCoords sc = CoordinateConverter::gridToScreen(spawnPos);
            mapCamera.target = {sc.x, sc.y};
            mapCamera.zoom = 1.5f;
        }
        if (audioInitialized) SwitchMusic(MUSIC_AMBIENT);
    }
    if (carthageButton.IsClicked()) {
        playerFaction = CARTHAGE;
        currentState = PLAYING;
        InitBuildings();
        InitResources();
        units.clear();
        // Спавн раба на випадковій вільній позиції (як раніше HQ)
        {
            GridCoords spawnPos = {-1, -1};
            for (int attempt = 0; attempt < 200 && spawnPos.row == -1; attempt++) {
                int row = 5 + rand() % 70;
                int col = 5 + rand() % 70;
                if (pathfindingManager.getGrid().isWalkable(row, col)) {
                    spawnPos = {row, col};
                }
            }
            if (spawnPos.row == -1) spawnPos = {10, 10};
            Unit slave;
            slave.init("slave", playerFaction, spawnPos, false);
            units.push_back(std::move(slave));
            // Фокус камери на рабі
            ScreenCoords sc = CoordinateConverter::gridToScreen(spawnPos);
            mapCamera.target = {sc.x, sc.y};
            mapCamera.zoom = 1.5f;
        }
        if (audioInitialized) SwitchMusic(MUSIC_AMBIENT);
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
    Vector2 worldMousePos = GetScreenToWorld2D(mousePos, mapCamera);

    // --- Placement mode: update ghost + handle RMB confirm ---
    if (slaveBuildPanel && slaveBuildPanel->placement.active) {
        slaveBuildPanel->updatePlacement(worldMousePos);
        if (slaveBuildPanel->handlePlacementInput(worldMousePos)) {
            return; // consumed
        }
        // While in placement mode, block all other clicks
        return;
    }

    // Спочатку перевіряємо клік по панелі замовлення юнітів (UI координати, без камери)
    if (unitOrderPanel && unitOrderPanel->isVisible()) {
        unitOrderPanel->handleClick(mousePos);
        // Якщо клік був на панелі, не обробляємо далі
        Rectangle panelRect = {10, 950, 300, 100};
        if (CheckCollisionPointRec(mousePos, panelRect)) {
            return;
        }
    }
    
    // Перевіряємо клік по панелі будівництва рабом
    if (slaveBuildPanel && slaveBuildPanel->isVisible()) {
        slaveBuildPanel->handleClick(mousePos);
        // Якщо клік був на панелі, не обробляємо далі
        Rectangle panelRect = {10, 930, 500, 120};
        if (CheckCollisionPointRec(mousePos, panelRect)) {
            return;
        }
    }
    
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        
        // DEBUG: Зберігаємо клік для візуалізації
        debugClicks.push_back(worldMousePos);
        if (debugClicks.size() > MAX_DEBUG_CLICKS) {
            debugClicks.erase(debugClicks.begin());
        }
        
        // Початок перетягування для виділення області
        isDragging = true;
        dragStart = worldMousePos;
        dragEnd = worldMousePos;
        
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
            
            // НОВИЙ ПІДХІД: Конвертуємо клік в grid координати (використовуємо світові координати)
            ScreenCoords screenPos = {worldMousePos.x, worldMousePos.y};
            GridCoords gridPos = CoordinateConverter::screenToGrid(screenPos);
            
            // Спочатку перевіряємо клік по юнітах (вони менші) - використовуємо world coords
            bool foundUnit = false;
            for (int i = 0; i < units.size(); i++) {
                // Перевіряємо тільки юніти гравця
                if (units[i].faction == playerFaction && units[i].isClicked(worldMousePos)) {
                    units[i].selected = true;
                    selectedUnitIndex = i;
                    lastClickedUnit = i;
                    lastClickTime = currentTime;
                    foundUnit = true;
                    
                    // Оновлюємо панель будівництва для рабів
                    if (slaveBuildPanel && units[i].unit_type == "slave") {
                        slaveBuildPanel->setSelectedUnit(i);
                        LOG_CLICK("[CLICK] Updated slave build panel\n");
                    }
                    break;
                }
            }
            
            // Якщо не знайшли юніт, перевіряємо будівлі
            if (!foundUnit) {
                LOG_CLICK("[CLICK] Mouse at screen (%.1f, %.1f) -> world (%.1f, %.1f) -> grid (%d, %d)\n", 
                          mousePos.x, mousePos.y, worldMousePos.x, worldMousePos.y, gridPos.row, gridPos.col);
                
                // Спочатку пробуємо знайти по світовим координатам (точніше для текстур)
                int buildingIndex = findBuildingAtWorldPos(worldMousePos);
                
                // Якщо не знайшли, пробуємо по grid координатам
                if (buildingIndex < 0) {
                    buildingIndex = findBuildingAtGrid(gridPos);
                }
                
                LOG_CLICK("[CLICK] Found building index: %d\n", buildingIndex);
                
                if (buildingIndex >= 0) {
                    buildings[buildingIndex].selected = true;
                    selectedBuildingIndex = buildingIndex;
                    LOG_CLICK("[CLICK] Selected building: %s at grid (%d, %d)\n", 
                              buildings[buildingIndex].name.c_str(),
                              buildings[buildingIndex].position.row,
                              buildings[buildingIndex].position.col);
                    
                    // Оновлюємо панель замовлення
                    if (unitOrderPanel) {
                        unitOrderPanel->setSelectedBuilding(buildingIndex);
                        LOG_CLICK("[CLICK] Updated unit order panel\n");
                    }
                }
            }
        }
        
        lastClickTime = currentTime;
    }
    
    // Оновлення перетягування (використовуємо світові координати)
    if (isDragging && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        Vector2 worldMousePos = GetScreenToWorld2D(GetMousePosition(), mapCamera);
        dragEnd = worldMousePos;
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
        // ВАЖЛИВО: Конвертуємо координати миші в світові координати з урахуванням камери
        Vector2 worldMousePos = GetScreenToWorld2D(mousePos, mapCamera);
        
        bool hasSelectedUnits = false;
        for (const auto& unit : units) {
            if (unit.selected && unit.faction == playerFaction) {
                hasSelectedUnits = true;
                break;
            }
        }
        
        if (hasSelectedUnits) {
            // Конвертуємо клік в grid координати для руху (використовуємо світові координати)
            ScreenCoords screenPos = {worldMousePos.x, worldMousePos.y};
            GridCoords gridPos = CoordinateConverter::screenToGrid(screenPos);
            
            LOG_CLICK("[RIGHT_CLICK] Mouse at screen (%.1f, %.1f) -> world (%.1f, %.1f) -> grid (%d, %d)\n",
                   mousePos.x, mousePos.y, worldMousePos.x, worldMousePos.y, gridPos.row, gridPos.col);
            
            // Перевіряємо клік по ворожих юнітах для атаки (використовуємо світові координати)
            bool foundEnemy = false;
            for (int i = 0; i < units.size(); i++) {
                if (units[i].faction != playerFaction && units[i].isClicked(worldMousePos)) {
                    // Відправляємо бойових юнітів атакувати
                    for (auto& unit : units) {
                        if (unit.selected && unit.faction == playerFaction && unit.attack_damage > 0) {
                            // Рухаємося до grid позиції ворога
                            unit.moveTo(units[i].getGridPosition());
                            unit.target_unit_id = i;
                        }
                    }
                    foundEnemy = true;
                    // Маркер атаки (червоний)
                    ScreenCoords esc = units[i].getScreenPosition();
                    moveMarkers.push_back({Vector2{esc.x, esc.y}, 1.0f, true, false});
                    break;
                }
            }
            
            if (!foundEnemy) {
                // Перевіряємо клік по ресурсах для збирачів (використовуємо світові координати)
                bool foundResource = false;
                for (int i = 0; i < resources.size(); i++) {
                    if (!resources[i].depleted && resources[i].isClicked(worldMousePos)) {
                        // Знаходимо найближчий КВЕСТОРІУМ або ПРАЕТОРІЙ для здачі ресурсів
                        GridCoords nearestBuildingPos = {-1, -1};
                        float minDist = 999999.0f;
                        
                        for (const auto& building : buildings) {
                            if (building.faction == playerFaction && 
                                (building.type == QUESTORIUM_ROME ||
                                 building.type == HQ_ROME ||
                                 building.type == HQ_CARTHAGE)) {
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
                        
                        // Відправляємо збирачів до ресурсу з призначенням
                        for (auto& unit : units) {
                            if (unit.selected && unit.faction == playerFaction && unit.can_harvest) {
                                if (nearestBuildingPos.row != -1) {
                                    // Знаходимо прохідний тайл поруч з будівлею здачі
                                    GridCoords walkableDropoff = findNearestWalkableTile(nearestBuildingPos);
                                    unit.assignResource(resources[i].getGridPosition(), walkableDropoff);
                                    printf("Slave assigned to resource at grid(%d, %d), dropoff at grid(%d, %d)\n", 
                                           resources[i].getGridPosition().row, resources[i].getGridPosition().col,
                                           walkableDropoff.row, walkableDropoff.col);
                                }
                            }
                        }
                        foundResource = true;
                        // Маркер збору (зелений)
                        ScreenCoords rsc = resources[i].getScreenPosition();
                        moveMarkers.push_back({Vector2{rsc.x, rsc.y}, 1.0f, false, true});
                        break;
                    }
                }
                
                // Якщо не клікнули по ресурсу або ворогу, звичайний рух до точної позиції
                if (!foundResource) {
                    // Знаходимо найближчий прохідний тайл (якщо клікнули на будівлю)
                    GridCoords targetGrid = findNearestWalkableTile(gridPos);
                    
                    // Рухаємо юніти до точних world координат кліку (якщо тайл вільний)
                    // або до найближчого вільного тайлу
                    bool useExactPos = (targetGrid.row == gridPos.row && targetGrid.col == gridPos.col);
                    
                    for (int i = 0; i < units.size(); i++) {
                        if (units[i].selected && units[i].faction == playerFaction) {
                            units[i].moveToByPlayer(targetGrid);
                            units[i].target_unit_id = -1;
                            if (useExactPos) {
                                moveUnitWithPathToScreen(units[i], {worldMousePos.x, worldMousePos.y});
                            } else {
                                moveUnitWithPath(units[i], targetGrid);
                            }
                            printf("[MOVE] Unit %d moving to world(%.1f,%.1f)\n", i, worldMousePos.x, worldMousePos.y);
                        }
                    }
                    // Маркер руху (жовтий)
                    moveMarkers.push_back({worldMousePos, 1.0f, false, false});
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
    
    // Edge scroll — скрол камери при підведенні миші до краю екрана
    {
        const float EDGE_ZONE = 20.0f;   // пікселів від краю
        const float EDGE_SPEED = 400.0f * GetFrameTime();
        Vector2 mp = GetMousePosition();
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();
        if (mp.x < EDGE_ZONE)        mapCamera.target.x -= EDGE_SPEED;
        if (mp.x > sw - EDGE_ZONE)   mapCamera.target.x += EDGE_SPEED;
        if (mp.y < EDGE_ZONE)        mapCamera.target.y -= EDGE_SPEED;
        if (mp.y > sh - EDGE_ZONE)   mapCamera.target.y += EDGE_SPEED;
    }
    
    // Керування камерою мишею (перетягування середньою кнопкою)
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
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
                // NOTE: Pathfinding logs temporarily disabled
                // printf("[PATHFINDING] Unit %d received path with %d waypoints\n", i, (int)path.size());
            } else {
                // NOTE: Pathfinding logs temporarily disabled
                // printf("[PATHFINDING] Unit %d received empty path - destination unreachable\n", i);
            }
        }
        
        // Якщо юніт застряг і очистив шлях, але все ще хоче рухатися
        if (units[i].is_moving && !units[i].hasPath() && units[i].usePathfinding) {
            // Автоматично запитуємо новий шлях до останньої цілі
            if (units[i].target_x != units[i].x || units[i].target_y != units[i].y) {
                // Використовуємо GridCoords для pathfinding (виправлення Bug Condition 3)
                GridCoords startGrid = units[i].getGridPosition();
                GridCoords goalGrid = units[i].target_position;
                ScreenCoords startScreen = CoordinateConverter::gridToScreen(startGrid);
                ScreenCoords goalScreen = CoordinateConverter::gridToScreen(goalGrid);
                pathfindingManager.requestPath(i, {startScreen.x, startScreen.y}, {goalScreen.x, goalScreen.y}, 1.0f);
                // NOTE: Pathfinding logs temporarily disabled
                // printf("[PATHFINDING] Auto-requesting new path for stuck unit %d from grid(%d,%d) to grid(%d,%d)\n", 
                //        i, startGrid.row, startGrid.col, goalGrid.row, goalGrid.col);
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

            // Перевірка умови перемоги/поразки після кожної смерті юніта
            {
                Faction enemyFaction = (playerFaction == ROME) ? CARTHAGE : ROME;

                bool enemyHasBuildings = false;
                bool playerHasBuildings = false;
                for (const auto& b : buildings) {
                    if (b.faction == enemyFaction)  enemyHasBuildings = true;
                    if (b.faction == playerFaction) playerHasBuildings = true;
                }

                bool enemyHasCombatUnits = false;
                bool playerHasCombatUnits = false;
                for (const auto& u : units) {
                    if (u.unit_type == "slave") continue;
                    if (u.faction == enemyFaction)  enemyHasCombatUnits = true;
                    if (u.faction == playerFaction) playerHasCombatUnits = true;
                }

                // Живий = є будівлі АБО є бойові юніти (раби не рятують)
                bool enemyAlive  = enemyHasBuildings  || enemyHasCombatUnits;
                bool playerAlive = playerHasBuildings || playerHasCombatUnits;

                if (!enemyAlive) {
                    currentState = VICTORY_SCREEN;
                    if (audioInitialized) SwitchMusic(MUSIC_VICTORY);
                } else if (!playerAlive) {
                    currentState = DEFEAT_SCREEN;
                    if (audioInitialized) SwitchMusic(MUSIC_DEFEAT);
                }
            }

            continue;
        }
        
        // Оновлюємо юніт з перевіркою колізій
        units[i].update(&buildings);
        
        // Бойова логіка - пошук ворогів поблизу та переслідування
        if (units[i].attack_damage > 0 && !units[i].is_harvesting) {
            // Радіус агресії в пікселях (5 тайлів ≈ 5 * 64 = 320 пікселів по ширині)
            const float AGGRO_RANGE_PX = 320.0f;
            
            ScreenCoords myScreen = units[i].getScreenPosition();
            
            // Знаходимо найближчого ворожого юніта в радіусі агресії
            // Використовуємо current_screen_pos щоб бачити юнітів що рухаються
            int nearestEnemy = -1;
            float nearestDist = AGGRO_RANGE_PX + 1.0f;
            
            for (int j = 0; j < (int)units.size(); j++) {
                if (i == j || units[i].faction == units[j].faction) continue;
                ScreenCoords enemyScreen = units[j].getScreenPosition();
                float pixDist = sqrt(pow(myScreen.x - enemyScreen.x, 2) +
                                     pow(myScreen.y - enemyScreen.y, 2));
                if (pixDist < nearestDist) {
                    nearestDist = pixDist;
                    nearestEnemy = j;
                }
            }
            
            if (nearestEnemy >= 0) {
                ScreenCoords enemyScreen = units[nearestEnemy].getScreenPosition();
                float pixelDist = sqrt(pow(myScreen.x - enemyScreen.x, 2) +
                                       pow(myScreen.y - enemyScreen.y, 2));
                
                if (pixelDist <= units[i].attack_range) {
                    // В радіусі — атакуємо, зупиняємо рух
                    units[i].clearPath();
                    units[i].is_moving = false;
                    units[i].attackTarget(units[nearestEnemy]);
                } else {
                    // Не в радіусі — переслідуємо по поточній екранній позиції ворога
                    GridCoords enemyGrid = units[nearestEnemy].getGridPosition();
                    if (!units[i].is_moving || !(units[i].final_destination == enemyGrid)) {
                        moveUnitWithPath(units[i], enemyGrid);
                    }
                }
            } else {
                // Немає ворожих юнітів — шукаємо ворожі будівлі в радіусі агресії
                int nearestEnemyBuilding = -1;
                float nearestBuildingDist = AGGRO_RANGE_PX + 1.0f;
                for (int b = 0; b < (int)buildings.size(); b++) {
                    if (buildings[b].faction == units[i].faction) continue;
                    ScreenCoords bScreen = buildings[b].getScreenPosition();
                    float pixDist = sqrt(pow(myScreen.x - bScreen.x, 2) +
                                         pow(myScreen.y - bScreen.y, 2));
                    if (pixDist < nearestBuildingDist) {
                        nearestBuildingDist = pixDist;
                        nearestEnemyBuilding = b;
                    }
                }
                if (nearestEnemyBuilding >= 0) {
                    ScreenCoords bScreen = buildings[nearestEnemyBuilding].getScreenPosition();
                    float pixelDist = sqrt(pow(myScreen.x - bScreen.x, 2) +
                                           pow(myScreen.y - bScreen.y, 2));
                    float currentTime = GetTime();
                    // Атакуємо будівлю якщо в межах attack_range * 5 (будівлі великі)
                    const float BUILD_ATTACK_RANGE = units[i].attack_range * 5.0f;
                    if (pixelDist <= BUILD_ATTACK_RANGE) {
                        // НЕ виставляємо is_attacking=true щоб не блокувати рух
                        if (currentTime - units[i].last_attack_time >= units[i].attack_cooldown) {
                            buildings[nearestEnemyBuilding].takeDamage(units[i].attack_damage);
                            units[i].last_attack_time = currentTime;
                        }
                    } else {
                        // Переслідуємо будівлю — йдемо до найближчого прохідного тайлу поруч
                        GridCoords bGrid = buildings[nearestEnemyBuilding].getGridPosition();
                        GridCoords walkable = findNearestWalkableTile(bGrid);
                        if (!units[i].is_moving || !(units[i].final_destination == walkable)) {
                            moveUnitWithPath(units[i], walkable);
                        }
                    }
                }
            }
        }
    }
    
    // Оновлення будівель та виробництва
    float deltaTime = GetFrameTime();
    std::string startedUnit = "";
    for (int bi = 0; bi < (int)buildings.size(); bi++) {
        // Видаляємо знищені будівлі
        if (buildings[bi].isDead()) {
            printf("[BUILDING] Building '%s' destroyed!\n", buildings[bi].name.c_str());
            // Скидаємо вибір якщо знищена вибрана будівля
            if (selectedBuildingIndex == bi) {
                selectedBuildingIndex = -1;
                if (unitOrderPanel) unitOrderPanel->setSelectedBuilding(-1);
            } else if (selectedBuildingIndex > bi) {
                selectedBuildingIndex--;
                if (unitOrderPanel) unitOrderPanel->setSelectedBuilding(selectedBuildingIndex);
            }
            buildings.erase(buildings.begin() + bi);
            pathfindingManager.updateGrid(buildings);
            bi--;

            // ── Перевірка умови перемоги / поразки ──────────────────────────
            {
                Faction enemyFaction = (playerFaction == ROME) ? CARTHAGE : ROME;

                // Будівлі
                bool enemyHasBuildings = false;
                bool playerHasBuildings = false;
                for (const auto& b : buildings) {
                    if (b.faction == enemyFaction)  enemyHasBuildings = true;
                    if (b.faction == playerFaction) playerHasBuildings = true;
                }

                // Бойові юніти (не раби)
                bool enemyHasCombatUnits = false;
                bool playerHasCombatUnits = false;
                for (const auto& u : units) {
                    if (u.unit_type == "slave") continue;
                    if (u.faction == enemyFaction)  enemyHasCombatUnits = true;
                    if (u.faction == playerFaction) playerHasCombatUnits = true;
                }

                bool enemyAlive  = enemyHasBuildings  || enemyHasCombatUnits;
                bool playerAlive = playerHasBuildings || playerHasCombatUnits;

                if (!enemyAlive) {
                    currentState = VICTORY_SCREEN;
                    if (audioInitialized) SwitchMusic(MUSIC_VICTORY);
                } else if (!playerAlive) {
                    currentState = DEFEAT_SCREEN;
                    if (audioInitialized) SwitchMusic(MUSIC_DEFEAT);
                }
            }
            // ────────────────────────────────────────────────────────────────

            continue;
        }
        
        Building& building = buildings[bi];
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
                
                if (building.type == HQ_ROME || building.type == HQ_CARTHAGE) {
                    if (canAfford(building.faction, "slave")) {
                        bool shouldPayNow = false;
                        if (building.startProduction("slave", shouldPayNow)) {
                            if (shouldPayNow) {
                                reserveResources(building.faction, "slave");
                                spendResources(building.faction, "slave");
                            } else {
                                reserveResources(building.faction, "slave");
                            }
                        }
                    }
                } else if (building.type == BARRACKS_ROME) {
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
                }
            }
        }
        
    } // end building loop
    
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
        renderQueue.addUnit(i, units[i].getGridPosition(), units[i].getScreenPosition());
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
    
    // Малювання області виділення (всередині режиму камери, бо координати світові)
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
    
    // DEBUG: Малюємо останні кліки (в світових координатах)
    if (showDebugVisuals) {
        for (size_t i = 0; i < debugClicks.size(); i++) {
            float alpha = (float)(i + 1) / debugClicks.size();
            DrawCircleV(debugClicks[i], 8.0f, {255, 0, 255, (unsigned char)(alpha * 255)});
            DrawCircleLines((int)debugClicks[i].x, (int)debugClicks[i].y, 8, {255, 255, 255, (unsigned char)(alpha * 255)});
        }
    }
    
    // Кінець режиму 2D камери
    // Ghost будівлі при placement mode (всередині камери — world coords)
    if (slaveBuildPanel) slaveBuildPanel->drawGhost();

    // Маркери руху — кружечки що зменшуються
    float dt = GetFrameTime();
    for (auto& m : moveMarkers) m.timer -= dt * 0.8f; // ~1.25 сек
    moveMarkers.erase(
        std::remove_if(moveMarkers.begin(), moveMarkers.end(),
                       [](const MoveMarker& m){ return m.timer <= 0.0f; }),
        moveMarkers.end());
    for (const auto& m : moveMarkers) {
        float r = m.timer * 24.0f;
        unsigned char alpha = (unsigned char)(m.timer * 220.0f);
        Color c = m.isAttack   ? Color{255, 60,  60,  alpha}
                : m.isHarvest  ? Color{60,  220, 60,  alpha}
                               : Color{255, 220, 0,   alpha};
        DrawCircleLines((int)m.pos.x, (int)m.pos.y, r, c);
        DrawCircleLines((int)m.pos.x, (int)m.pos.y, r * 0.5f, c);
    }

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
    
    // Панель замовлення юнітів (нова система)
    if (unitOrderPanel) {
        unitOrderPanel->draw();
    }
    
    // Панель будівництва рабом
    if (slaveBuildPanel) {
        slaveBuildPanel->draw();
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
    SetExitKey(0);
    SetTargetFPS(60);

    // Завантажуємо збережені налаштування і одразу застосовуємо fullscreen
    LoadSettings();
    if (displaySettings.isWindowedFullscreen) {
        displaySettings.isWindowedFullscreen = false;
        int monitorWidth  = GetMonitorWidth(GetCurrentMonitor());
        int monitorHeight = GetMonitorHeight(GetCurrentMonitor());
        displaySettings.savedX = (monitorWidth  - displaySettings.windowWidth)  / 2;
        displaySettings.savedY = (monitorHeight - displaySettings.windowHeight) / 2;
        SetWindowState(FLAG_WINDOW_UNDECORATED);
        SetWindowPosition(0, 0);
        SetWindowSize(monitorWidth, monitorHeight);
        displaySettings.isWindowedFullscreen = true;
    }

    // ── Splash screen ────────────────────────────────────────────────────────
    // Малюємо заставку одразу, ще до завантаження решти ресурсів
    {
        Texture2D splash = LoadTexture("assets/Background.png");
        BeginDrawing();
        ClearBackground(BLACK);
        if (splash.id > 0) {
            int sw = GetScreenWidth(), sh = GetScreenHeight();
            float scale = fmaxf((float)sw / splash.width, (float)sh / splash.height);
            int dw = (int)(splash.width  * scale);
            int dh = (int)(splash.height * scale);
            DrawTexturePro(splash,
                {0, 0, (float)splash.width, (float)splash.height},
                {(sw - dw) * 0.5f, (sh - dh) * 0.5f, (float)dw, (float)dh},
                {0, 0}, 0.0f, WHITE);
        }
        // Напис "Loading..."
        const char* loadingText = "Loading...";
        int fontSize = 28;
        int tw = MeasureText(loadingText, fontSize);
        int sh2 = GetScreenHeight(), sw2 = GetScreenWidth();
        DrawRectangle(0, sh2 - 60, sw2, 60, {0, 0, 0, 160});
        DrawText(loadingText, sw2 / 2 - tw / 2, sh2 - 40, fontSize, {200, 200, 200, 255});
        EndDrawing();
        if (splash.id > 0) UnloadTexture(splash);
    }
    // ────────────────────────────────────────────────────────────────────────
    
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
    victoryMusic   = LoadMusicStream("assets/sounds/Punic wars_ Castra Victory Rome.mp3");
    
    // Завантаження фону меню
    menuBackground = LoadTexture("assets/Background.png");
    
    // Завантаження фону вибору фракції
    factionBackground = LoadTexture("assets/Background2.png");
    
    // Завантаження фону налаштувань
    settingsBackground = LoadTexture("assets/Settings_background.png");
    
    // Завантаження фону меню паузи
    pauseBackground = LoadTexture("assets/Pause_background.png");
    if (pauseBackground.id > 0) {
        LOG_TEXTURE("[TEXTURE] Pause background loaded: %dx%d\n", pauseBackground.width, pauseBackground.height);
    } else {
        LOG_TEXTURE("[TEXTURE] Warning: Pause background not loaded!\n");
    }
    
    // Завантаження фону перемоги та поразки
    victoryBackground = LoadTexture("assets/Victory_background.png");
    defeatBackground  = LoadTexture("assets/Defeat_background.png");
    
    // Завантаження загальної текстури фону (SVG не підтримується, використаємо PNG якщо є)
    backgroundTexture = LoadTexture("assets/background_texture.png");
    if (backgroundTexture.id == 0) {
        // Якщо PNG немає, створюємо fallback текстуру
        Image fallbackImg = GenImageColor(256, 256, {40, 35, 30, 255});
        backgroundTexture = LoadTextureFromImage(fallbackImg);
        UnloadImage(fallbackImg);
        LOG_TEXTURE("[TEXTURE] Using fallback background texture\n");
    } else {
        LOG_TEXTURE("[TEXTURE] Background texture loaded: %dx%d\n", backgroundTexture.width, backgroundTexture.height);
    }
    
    // Завантаження логотипу
    gameLogo = LoadTexture("assets/sprites/Logo.png");
    if (gameLogo.id > 0) {
        logoLoaded = true;
        LOG_TEXTURE("[TEXTURE] Logo loaded: %dx%d\n", gameLogo.width, gameLogo.height);
    } else {
        LOG_TEXTURE("[TEXTURE] Warning: Logo not loaded!\n");
    }
    
    // Завантаження кастомного шрифту
    customFont = LoadFontEx("assets/fonts/GameFont.ttf", 48, 0, 0);
    if (customFont.texture.id > 0) {
        fontLoaded = true;
        LOG_TEXTURE("[FONT] Custom font loaded successfully\n");
    } else {
        LOG_TEXTURE("[FONT] Warning: Custom font not loaded, using default\n");
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
    LOG_TEXTURE("[TEXTURE] Loading building textures...\n");
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
        LOG_BUILDING("[TILEMAP] Tileset loaded successfully\n");
    } else {
        LOG_BUILDING("[TILEMAP] Warning: Tileset not found, using debug rendering\n");
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
    
    LOG_BUILDING("[TILEMAP] Map generated: %dx%d, %.1f%% passable\n", 
           gameMap->getWidth(), gameMap->getHeight(), 
           gameMap->getPassablePercentage() * 100.0f);
    
    // Ініціалізація UI систем
    unitOrderPanel = new UnitOrderPanel();
    unitOrderPanel->init(&buildings, playerFaction);
    
    slaveBuildPanel = new SlaveBuildPanel();
    slaveBuildPanel->init(&units, &buildings, playerFaction);
    
    resourceDisplay = new ResourceDisplay();
    resourceDisplay->init(resourcePanel, playerFaction);
    
    LOG_BUILDING("[UI] UI systems initialized\n");
    
    // Створюємо RenderTexture для знімку гри
    gameSnapshot = LoadRenderTexture(screenWidth, screenHeight);
    LOG_BUILDING("[UI] Game snapshot texture created: %dx%d\n", screenWidth, screenHeight);
    
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
            case VICTORY_SCREEN:
                DrawVictoryScreen();
                break;
            case DEFEAT_SCREEN:
                DrawDefeatScreen();
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
        UnloadMusicStream(victoryMusic);
    }
    UnloadTexture(menuBackground);
    UnloadTexture(factionBackground);
    UnloadTexture(settingsBackground);
    UnloadTexture(pauseBackground);
    UnloadTexture(victoryBackground);
    UnloadTexture(defeatBackground);
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
    if (slaveBuildPanel) delete slaveBuildPanel;
    if (resourceDisplay) delete resourceDisplay;
    
    CloseAudioDevice();
    
    // Закриття
    CloseWindow();
    return 0;
}
