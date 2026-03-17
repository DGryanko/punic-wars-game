#pragma once

// Константи гри - централізоване місце для всіх magic numbers

namespace GameConstants {
    // Розміри карти
    constexpr int MAP_WIDTH = 80;
    constexpr int MAP_HEIGHT = 80;
    constexpr int MAP_CENTER_ROW = 40;
    constexpr int MAP_CENTER_COL = 40;
    
    // Відстані та радіуси
    constexpr float RESOURCE_COLLECTION_RADIUS = 40.0f;  // Радіус збору ресурсів (пікселі)
    constexpr float DROPOFF_DISTANCE = 20.0f;            // Відстань здачі ресурсів (пікселі)
    constexpr float ENEMY_DETECTION_RADIUS = 200.0f;     // Радіус виявлення ворога (пікселі)
    constexpr float UNIT_COLLISION_RADIUS = 16.0f;       // Радіус колізії юніта (пікселі)
    
    // Час та затримки
    constexpr float DOUBLE_CLICK_TIME = 0.3f;            // Час для подвійного кліку (секунди)
    constexpr float BATTLE_MUSIC_DELAY = 3.0f;           // Затримка перед бойовою музикою (секунди)
    constexpr float PEACEFUL_MUSIC_DELAY = 10.0f;        // Затримка перед мирною музикою (секунди)
    constexpr float AMBIENT_MUSIC_DELAY = 2.0f;          // Затримка перед амбієнт музикою (секунди)
    
    // Початкові ресурси
    namespace StartingResources {
        constexpr int ROME_FOOD = 200;
        constexpr int ROME_MONEY = 100;
        constexpr int CARTHAGE_FOOD = 150;
        constexpr int CARTHAGE_MONEY = 200;
    }
    
    // Ресурсні точки
    namespace ResourcePoints {
        constexpr int FOOD_SOURCES_COUNT = 8;
        constexpr int GOLD_SOURCES_COUNT = 6;
        constexpr int FOOD_MIN_AMOUNT = 500;
        constexpr int FOOD_MAX_AMOUNT = 1000;
        constexpr int GOLD_MIN_AMOUNT = 300;
        constexpr int GOLD_MAX_AMOUNT = 700;
        constexpr int MIN_SPACING = 3;  // Мінімальна відстань між ресурсами (тайли)
    }
    
    // Spawn конфігурація
    namespace Spawn {
        constexpr int MIN_HQ_DISTANCE = 20;              // Мінімальна відстань між HQ (тайли)
        constexpr int MAX_SPAWN_ATTEMPTS = 100;          // Максимум спроб spawn
        constexpr int REDUCE_DISTANCE_AFTER = 50;        // Зменшити відстань після N спроб
    }
    
    // UI розміри
    namespace UI {
        constexpr int WINDOW_WIDTH = 1434;
        constexpr int WINDOW_HEIGHT = 1075;
        constexpr int RESOURCE_PANEL_WIDTH = 1027;
        constexpr int RESOURCE_PANEL_HEIGHT = 105;
        constexpr int UNIT_ORDER_PANEL_X = 10;
        constexpr int UNIT_ORDER_PANEL_Y = 950;
        constexpr int UNIT_ORDER_PANEL_WIDTH = 300;
        constexpr int UNIT_ORDER_PANEL_HEIGHT = 100;
    }
    
    // Debug
    namespace Debug {
        constexpr bool ENABLE_PATHFINDING_LOGS = false;
        constexpr bool ENABLE_CLICK_LOGS = false;
        constexpr bool ENABLE_SPAWN_LOGS = true;
        constexpr bool ENABLE_BUILDING_LOGS = true;
        constexpr int MAX_DEBUG_CLICKS = 10;  // Максимум debug кліків для візуалізації
    }
}
