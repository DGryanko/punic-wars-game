#include "raylib.h"
#include "tilemap/tilemap_generator.h"
#include <ctime>

int main() {
    // Ініціалізація вікна
    const int screenWidth = 1024;
    const int screenHeight = 768;
    InitWindow(screenWidth, screenHeight, "Isometric Tilemap Generator Test");
    SetTargetFPS(60);
    
    // Генерація карти
    unsigned int seed = static_cast<unsigned int>(time(nullptr));
    MapGenerator generator(seed);
    
    // Налаштування параметрів генерації
    GenerationParams params;
    params.water_level = 0.3f;
    params.sand_level = 0.5f;
    params.grass_level = 0.9f;
    params.noise_scale = 0.1f;
    params.octaves = 4;
    
    TileMap map = generator.generate(50, 50, params);
    
    // Знаходимо стартові позиції
    auto start_positions = generator.findStartPositions(map);
    TraceLog(LOG_INFO, "Start position 1: (%d, %d)", start_positions.first.row, start_positions.first.col);
    TraceLog(LOG_INFO, "Start position 2: (%d, %d)", start_positions.second.row, start_positions.second.col);
    
    // Ініціалізація рендерера
    IsometricRenderer renderer;
    
    // Спроба завантажити тайлсет (якщо є)
    if (FileExists("assets/isometric_tileset.png")) {
        renderer.loadTileset("assets/isometric_tileset.png");
    } else {
        TraceLog(LOG_WARNING, "Tileset not found, using debug rendering");
    }
    
    // Налаштування камери
    Camera2D camera = {0};
    camera.target = {0, 0};
    camera.offset = {screenWidth / 2.0f, screenHeight / 2.0f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
    renderer.setCamera(camera);
    
    // Швидкість прокрутки
    float scroll_speed = 5.0f;
    
    // Головний цикл
    while (!WindowShouldClose()) {
        // Оновлення
        
        // Прокрутка камери стрілками
        if (IsKeyDown(KEY_RIGHT)) camera.target.x += scroll_speed;
        if (IsKeyDown(KEY_LEFT)) camera.target.x -= scroll_speed;
        if (IsKeyDown(KEY_DOWN)) camera.target.y += scroll_speed;
        if (IsKeyDown(KEY_UP)) camera.target.y -= scroll_speed;
        
        // Зум колесом миші
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            camera.zoom += wheel * 0.1f;
            if (camera.zoom < 0.5f) camera.zoom = 0.5f;
            if (camera.zoom > 2.0f) camera.zoom = 2.0f;
        }
        
        renderer.setCamera(camera);
        
        // Підсвічування тайлу під мишею
        GridCoords hovered = renderer.getHoveredTile();
        if (map.isValidCoord(hovered.row, hovered.col)) {
            renderer.highlightTile(hovered.row, hovered.col);
        }
        
        // Генерація нової карти на пробіл
        if (IsKeyPressed(KEY_SPACE)) {
            seed = static_cast<unsigned int>(time(nullptr));
            MapGenerator new_generator(seed);
            map = new_generator.generate(50, 50, params);
            start_positions = new_generator.findStartPositions(map);
            TraceLog(LOG_INFO, "Generated new map with seed: %u", seed);
        }
        
        // Збереження карти на S
        if (IsKeyPressed(KEY_S)) {
            if (MapSerializer::saveToFile(map, "saved_map.txt")) {
                TraceLog(LOG_INFO, "Map saved successfully");
            }
        }
        
        // Завантаження карти на L
        if (IsKeyPressed(KEY_L)) {
            if (MapSerializer::fileExists("saved_map.txt")) {
                map = MapSerializer::loadFromFile("saved_map.txt");
                TraceLog(LOG_INFO, "Map loaded successfully");
            }
        }
        
        // Рендеринг
        BeginDrawing();
        ClearBackground(BLACK);
        
        BeginMode2D(camera);
        
        // Рендеримо карту
        if (renderer.isTilesetLoaded()) {
            renderer.render(map);
        } else {
            renderer.renderDebug(map);
        }
        
        // Малюємо стартові позиції
        ScreenCoords start1_screen = CoordinateConverter::gridToScreen(start_positions.first);
        ScreenCoords start2_screen = CoordinateConverter::gridToScreen(start_positions.second);
        DrawCircle(start1_screen.x, start1_screen.y, 10, RED);
        DrawCircle(start2_screen.x, start2_screen.y, 10, BLUE);
        
        EndMode2D();
        
        // UI
        DrawFPS(10, 10);
        DrawText(TextFormat("Seed: %u", seed), 10, 30, 20, WHITE);
        DrawText(TextFormat("Passable: %.1f%%", map.getPassablePercentage() * 100.0f), 10, 50, 20, WHITE);
        DrawText(TextFormat("Zoom: %.1fx", camera.zoom), 10, 70, 20, WHITE);
        DrawText(TextFormat("Hovered: (%d, %d)", hovered.row, hovered.col), 10, 90, 20, WHITE);
        
        DrawText("Controls:", 10, 120, 20, YELLOW);
        DrawText("Arrows - Move camera", 10, 140, 16, WHITE);
        DrawText("Mouse Wheel - Zoom", 10, 160, 16, WHITE);
        DrawText("SPACE - Generate new map", 10, 180, 16, WHITE);
        DrawText("S - Save map", 10, 200, 16, WHITE);
        DrawText("L - Load map", 10, 220, 16, WHITE);
        
        EndDrawing();
    }
    
    // Очищення
    renderer.unloadTileset();
    CloseWindow();
    
    return 0;
}
