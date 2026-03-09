#pragma once
#include <cstdio>
#include "game_constants.h"

// Централізований debug logger з можливістю вимкнення категорій

namespace DebugLogger {
    enum class Category {
        PATHFINDING,
        CLICK,
        SPAWN,
        BUILDING,
        TEXTURE,
        FIND,
        GENERAL
    };
    
    // Перевірка чи категорія увімкнена
    inline bool isEnabled(Category category) {
        using namespace GameConstants::Debug;
        switch (category) {
            case Category::PATHFINDING: return ENABLE_PATHFINDING_LOGS;
            case Category::CLICK: return ENABLE_CLICK_LOGS;
            case Category::SPAWN: return ENABLE_SPAWN_LOGS;
            case Category::BUILDING: return ENABLE_BUILDING_LOGS;
            case Category::TEXTURE: return true;  // Завжди показувати помилки текстур
            case Category::FIND: return ENABLE_CLICK_LOGS;  // Прив'язано до кліків
            case Category::GENERAL: return true;
            default: return false;
        }
    }
    
    // Логування з категорією
    template<typename... Args>
    inline void log(Category category, const char* format, Args... args) {
        if (isEnabled(category)) {
            printf(format, args...);
        }
    }
    
    // Завжди логувати (для критичних повідомлень)
    template<typename... Args>
    inline void logAlways(const char* format, Args... args) {
        printf(format, args...);
    }
    
    // Логування помилок (завжди увімкнено)
    template<typename... Args>
    inline void logError(const char* format, Args... args) {
        printf("[ERROR] ");
        printf(format, args...);
    }
    
    // Логування попереджень (завжди увімкнено)
    template<typename... Args>
    inline void logWarning(const char* format, Args... args) {
        printf("[WARNING] ");
        printf(format, args...);
    }
}

// Макроси для зручності
#define LOG_PATHFINDING(...) DebugLogger::log(DebugLogger::Category::PATHFINDING, __VA_ARGS__)
#define LOG_CLICK(...) DebugLogger::log(DebugLogger::Category::CLICK, __VA_ARGS__)
#define LOG_SPAWN(...) DebugLogger::log(DebugLogger::Category::SPAWN, __VA_ARGS__)
#define LOG_BUILDING(...) DebugLogger::log(DebugLogger::Category::BUILDING, __VA_ARGS__)
#define LOG_TEXTURE(...) DebugLogger::log(DebugLogger::Category::TEXTURE, __VA_ARGS__)
#define LOG_FIND(...) DebugLogger::log(DebugLogger::Category::FIND, __VA_ARGS__)
#define LOG_ERROR(...) DebugLogger::logError(__VA_ARGS__)
#define LOG_WARNING(...) DebugLogger::logWarning(__VA_ARGS__)
