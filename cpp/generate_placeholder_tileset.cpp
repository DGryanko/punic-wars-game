#include "raylib.h"

// Програма для генерації placeholder тайлсету
int main() {
    InitWindow(256, 128, "Generating Tileset...");
    
    // Створюємо зображення 256x128
    Image tileset = GenImageColor(256, 128, BLACK);
    
    // Малюємо 4 ізометричні тайли
    
    // Трава (0, 0) - зелений
    ImageDrawRectangle(&tileset, 0, 0, 128, 64, GREEN);
    
    // Вода (128, 0) - синій
    ImageDrawRectangle(&tileset, 128, 0, 128, 64, BLUE);
    
    // Пісок (0, 64) - бежевий
    ImageDrawRectangle(&tileset, 0, 64, 128, 64, BEIGE);
    
    // Дорога (128, 64) - коричневий
    ImageDrawRectangle(&tileset, 128, 64, 128, 64, BROWN);
    
    // Малюємо сітку для візуалізації
    for (int x = 0; x <= 256; x += 128) {
        ImageDrawLine(&tileset, x, 0, x, 128, WHITE);
    }
    for (int y = 0; y <= 128; y += 64) {
        ImageDrawLine(&tileset, 0, y, 256, y, WHITE);
    }
    
    // Зберігаємо
    ExportImage(tileset, "assets/isometric_tileset_placeholder.png");
    UnloadImage(tileset);
    
    TraceLog(LOG_INFO, "Placeholder tileset generated: assets/isometric_tileset_placeholder.png");
    TraceLog(LOG_INFO, "This is a simple placeholder. Replace with proper isometric tiles!");
    
    CloseWindow();
    return 0;
}
