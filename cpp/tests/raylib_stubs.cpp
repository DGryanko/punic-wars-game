// Заглушки для Raylib функцій для тестування
#include <cstdarg>

extern "C" {
    void TraceLog(int logLevel, const char* text, ...) {
        // Нічого не робимо - заглушка для тестів
    }
    
    float GetTime() {
        return 0.0f;
    }
}
