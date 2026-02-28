#include "raylib.h"
#include "building.h"
#include "unit.h"
#include "resource.h"
#include <vector>

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
int carth_food = 150;
int carth_money = 200;

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
bool audioInitialized = false;

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
