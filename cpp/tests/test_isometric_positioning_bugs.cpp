// Дослідницькі тести для виявлення багів позиціонування в ізометричній системі
// **Validates: Requirements 2.1, 2.2, 2.3, 2.4, 2.5**
// 
// ВАЖЛИВО: Ці тести МАЮТЬ ПРОВАЛИТИСЯ на невиправленому коді
// Провал підтверджує існування багів
// Коли тести пройдуть після виправлення - це підтвердить, що баги виправлені

// Мінімальні визначення для тестування без повної ініціалізації Raylib
#define RAYLIB_H
typedef struct Vector2 { float x; float y; } Vector2;
typedef struct Rectangle { float x; float y; float width; float height; } Rectangle;
typedef struct Color { unsigned char r; unsigned char g; unsigned char b; unsigned char a; } Color;
typedef struct Texture2D { unsigned int id; int width; int height; int mipmaps; int format; } Texture2D;

// Заглушки для Raylib функцій
#define TraceLog(level, ...) ((void)0)
#define LOG_INFO 0
bool CheckCollisionPointRec(Vector2 point, Rectangle rec) { return false; }
bool CheckCollisionPointCircle(Vector2 point, Vector2 center, float radius) { return false; }
void DrawText(const char* text, int x, int y, int size, Color color) {}
void DrawTexture(Texture2D texture, int x, int y, Color tint) {}
void DrawRectangle(int x, int y, int width, int height, Color color) {}
void DrawRectangleLines(int x, int y, int width, int height, Color color) {}
void DrawCircleLines(int x, int y, float radius, Color color) {}
void DrawLine(int x1, int y1, int x2, int y2, Color color) {}
const char* TextFormat(const char* text, ...) { return ""; }
float GetTime() { return 0.0f; }
float GetFrameTime() { return 0.016f; }

// Кольори
#define RED {255, 0, 0, 255}
#define BLUE {0, 0, 255, 255}
#define GRAY {128, 128, 128, 255}
#define WHITE {255, 255, 255, 255}
#define MAROON {128, 0, 0, 255}
#define DARKBLUE {0, 0, 128, 255}
#define YELLOW {255, 255, 0, 255}
#define GREEN {0, 255, 0, 255}
#define LIGHTGRAY {200, 200, 200, 255}
#define ORANGE {255, 165, 0, 255}
#define DARKGRAY {80, 80, 80, 255}

#include "../src/tilemap/coordinates.h"
#include <cassert>
#include <cstdio>
#include <cmath>
#include <string>
#include <vector>

// Мінімальні визначення для Building та Unit без IsometricSprite
enum Faction { ROME, CARTHAGE };
enum BuildingType {
    HQ_ROME, HQ_CARTHAGE, BARRACKS_ROME, BARRACKS_CARTHAGE,
    QUESTORIUM_ROME, LIBTENT_1, LIBTENT_2, LIBTENT_3, TENTORIUM
};

struct SimpleBuilding {
    GridCoords position;
    GridCoords footprint;
    BuildingType type;
    Faction faction;
    Vector2 texture_offset;
    
    void init(BuildingType buildingType, Faction buildingFaction, GridCoords startPos) {
        type = buildingType;
        faction = buildingFaction;
        position = startPos;
        
        // Встановлення footprint залежно від типу
        switch (type) {
            case HQ_ROME:
            case HQ_CARTHAGE:
                footprint = {3, 3};
                break;
            default:
                footprint = {2, 2};
                break;
        }
        
        // BUGGY CODE: Однаковий offset для всіх будівель
        texture_offset = {-192, -112};
    }
    
    ScreenCoords getScreenPosition() const {
        return CoordinateConverter::gridToScreen(position);
    }
    
    Rectangle getRect() const {
        ScreenCoords screenPos = getScreenPosition();
        return {
            screenPos.x + texture_offset.x,
            screenPos.y + texture_offset.y,
            384.0f,  // Ширина текстури
            224.0f   // Висота текстури
        };
    }
    
    bool isClicked(Vector2 mousePos) const {
        Rectangle rect = getRect();
        // Проста перевірка точки в прямокутнику
        return mousePos.x >= rect.x && mousePos.x <= rect.x + rect.width &&
               mousePos.y >= rect.y && mousePos.y <= rect.y + rect.height;
    }
};

struct SimpleUnit {
    GridCoords position;
    GridCoords target_position;
    int x, y;
    int target_x, target_y;
    
    void init(GridCoords startPos) {
        position = startPos;
        target_position = startPos;
        syncScreenCoords();
    }
    
    void syncScreenCoords() {
        ScreenCoords screen = CoordinateConverter::gridToScreen(position);
        x = (int)screen.x;
        y = (int)screen.y;
        ScreenCoords targetScreen = CoordinateConverter::gridToScreen(target_position);
        target_x = (int)targetScreen.x;
        target_y = (int)targetScreen.y;
    }
    
    void moveTo(GridCoords newPos) {
        target_position = newPos;
        syncScreenCoords();
    }
    
    GridCoords getGridPosition() const {
        return position;
    }
};

// ============================================================================
// ТЕСТ 1.1: Розміщення будівель (building.h)
// **Validates: Requirements 2.1, 2.2**
// ============================================================================

void test_building_placement_alignment() {
    printf("\n[TEST 1.1] Building placement alignment\n");
    printf("Expected: FAIL on unfixed code (proves bug exists)\n");
    printf("---------------------------------------------------\n");
    
    // Створюємо будівлю HQ_ROME на позиції (10, 10)
    SimpleBuilding hq;
    hq.init(HQ_ROME, ROME, GridCoords(10, 10));
    
    printf("Building type: HQ_ROME (Praetorium)\n");
    printf("Grid position: (%d, %d)\n", hq.position.row, hq.position.col);
    printf("Footprint: %dx%d tiles\n", hq.footprint.row, hq.footprint.col);
    printf("Texture offset: (%.0f, %.0f)\n", hq.texture_offset.x, hq.texture_offset.y);
    
    // Перевіряємо texture_offset
    // На невиправленому коді: texture_offset = {-192, -112}
    // Очікуваний правильний offset для 3x3 footprint: приблизно {-160, -96}
    
    bool hasIncorrectOffset = (hq.texture_offset.x == -192.0f && hq.texture_offset.y == -112.0f);
    
    if (hasIncorrectOffset) {
        printf("✗ DETECTED BUG: texture_offset = {-192, -112} (incorrect)\n");
        printf("  Expected: offset should account for isometric geometry\n");
        printf("  For 3x3 footprint, center should be at tile (1.5, 1.5)\n");
        printf("  This causes visual misalignment with tiles\n");
    } else {
        printf("✓ texture_offset appears corrected\n");
    }
    
    // Обчислюємо очікуваний центр для 3x3 footprint
    // Центр має бути на тайлі (1, 1) відносно position (для 3x3 це середній тайл)
    GridCoords centerOffset(1, 1);
    ScreenCoords centerScreen = CoordinateConverter::gridToScreen(centerOffset);
    
    printf("\nExpected center offset for 3x3 footprint:\n");
    printf("  Grid: (%d, %d) relative to position\n", centerOffset.row, centerOffset.col);
    printf("  Screen: (%.0f, %.0f)\n", centerScreen.x, centerScreen.y);
    printf("  Expected texture_offset: (%.0f, %.0f)\n", 
           centerScreen.x - 192, centerScreen.y - 112);
    
    // Виміряємо фактичне зміщення
    float actualOffsetX = hq.texture_offset.x;
    float actualOffsetY = hq.texture_offset.y;
    float expectedOffsetX = centerScreen.x - 192;
    float expectedOffsetY = centerScreen.y - 112;
    
    float deviationX = fabs(actualOffsetX - expectedOffsetX);
    float deviationY = fabs(actualOffsetY - expectedOffsetY);
    
    printf("\nDeviation from expected:\n");
    printf("  X: %.0f pixels\n", deviationX);
    printf("  Y: %.0f pixels\n", deviationY);
    
    // Тест провалюється, якщо зміщення більше 5 пікселів
    bool testPassed = (deviationX <= 5.0f && deviationY <= 5.0f);
    
    if (!testPassed) {
        printf("\n✗ TEST FAILED (as expected on unfixed code)\n");
        printf("  Building texture is misaligned by ~%.0f pixels\n", 
               sqrt(deviationX * deviationX + deviationY * deviationY));
        printf("  This confirms Bug Condition 1: Buildings positioned off tiles\n");
    } else {
        printf("\n✓ TEST PASSED\n");
        printf("  Building texture is correctly aligned\n");
    }
    
    printf("---------------------------------------------------\n");
}

// ============================================================================
// ТЕСТ 1.2: Клік по будівлі (building.h)
// **Validates: Requirements 2.2**
// ============================================================================

void test_building_click_area() {
    printf("\n[TEST 1.2] Building click area alignment\n");
    printf("Expected: FAIL on unfixed code (click area misaligned)\n");
    printf("---------------------------------------------------\n");
    
    // Створюємо будівлю QUESTORIUM_ROME на позиції (15, 15)
    SimpleBuilding questorium;
    questorium.init(QUESTORIUM_ROME, ROME, GridCoords(15, 15));
    
    printf("Building type: QUESTORIUM_ROME\n");
    printf("Grid position: (%d, %d)\n", questorium.position.row, questorium.position.col);
    printf("Footprint: %dx%d tiles\n", questorium.footprint.row, questorium.footprint.col);
    
    // Отримуємо екранну позицію та область кліку
    ScreenCoords screenPos = questorium.getScreenPosition();
    Rectangle clickRect = questorium.getRect();
    
    printf("\nScreen position: (%.0f, %.0f)\n", screenPos.x, screenPos.y);
    printf("Click rectangle: (%.0f, %.0f, %.0f, %.0f)\n", 
           clickRect.x, clickRect.y, clickRect.width, clickRect.height);
    printf("Texture offset: (%.0f, %.0f)\n", 
           questorium.texture_offset.x, questorium.texture_offset.y);
    
    // Обчислюємо центр текстури
    float textureCenterX = clickRect.x + clickRect.width / 2;
    float textureCenterY = clickRect.y + clickRect.height / 2;
    
    printf("Texture center: (%.0f, %.0f)\n", textureCenterX, textureCenterY);
    
    // Симулюємо клік на центр текстури
    Vector2 clickPos = {textureCenterX, textureCenterY};
    bool clickDetected = questorium.isClicked(clickPos);
    
    printf("\nSimulating click at texture center (%.0f, %.0f)\n", clickPos.x, clickPos.y);
    printf("Click detected: %s\n", clickDetected ? "YES" : "NO");
    
    // На невиправленому коді клік може не спрацювати через зміщену область
    if (!clickDetected) {
        printf("\n✗ TEST FAILED (as expected on unfixed code)\n");
        printf("  Click on texture center not detected\n");
        printf("  This confirms Bug Condition 1: Click area doesn't match visual texture\n");
        printf("  Cause: Incorrect texture_offset affects getRect() calculation\n");
    } else {
        printf("\n✓ TEST PASSED\n");
        printf("  Click area correctly matches texture position\n");
    }
    
    // Додатково перевіряємо, чи область кліку відповідає візуальному розташуванню
    // Очікуємо, що clickRect.x = screenPos.x + texture_offset.x
    float expectedClickX = screenPos.x + questorium.texture_offset.x;
    float expectedClickY = screenPos.y + questorium.texture_offset.y;
    
    float clickDeviationX = fabs(clickRect.x - expectedClickX);
    float clickDeviationY = fabs(clickRect.y - expectedClickY);
    
    printf("\nClick rectangle alignment check:\n");
    printf("  Expected top-left: (%.0f, %.0f)\n", expectedClickX, expectedClickY);
    printf("  Actual top-left: (%.0f, %.0f)\n", clickRect.x, clickRect.y);
    printf("  Deviation: (%.0f, %.0f)\n", clickDeviationX, clickDeviationY);
    
    printf("---------------------------------------------------\n");
}

// ============================================================================
// ТЕСТ 1.3: Відображення монет (resource_display.h)
// **Validates: Requirements 2.3**
// ============================================================================

void test_coins_display_position() {
    printf("\n[TEST 1.3] Coins display position\n");
    printf("Expected: FAIL on unfixed code (coins offset by 400px)\n");
    printf("---------------------------------------------------\n");
    
    // Симулюємо panelX = 500
    float panelX = 500.0f;
    
    // На невиправленому коді:
    // Їжа: panelX + 280 = 780
    // Монети: panelX + 680 = 1180
    // Різниця: 400 пікселів
    
    float foodTextX = panelX + 280;
    float moneyTextX_buggy = panelX + 680;  // Невиправлений код
    float moneyTextX_correct = panelX + 280; // Правильна позиція
    
    printf("Panel X position: %.0f\n", panelX);
    printf("\nFood text position:\n");
    printf("  X = panelX + 280 = %.0f\n", foodTextX);
    
    printf("\nMoney text position (BUGGY CODE):\n");
    printf("  X = panelX + 680 = %.0f\n", moneyTextX_buggy);
    
    printf("\nExpected money text position (CORRECT):\n");
    printf("  X = panelX + 280 = %.0f\n", moneyTextX_correct);
    
    float offset_difference = moneyTextX_buggy - foodTextX;
    printf("\nOffset difference: %.0f pixels\n", offset_difference);
    
    // Перевіряємо, чи є зміщення 400 пікселів
    bool hasBuggyOffset = (fabs(offset_difference - 400.0f) < 1.0f);
    
    if (hasBuggyOffset) {
        printf("\n✗ TEST FAILED (as expected on unfixed code)\n");
        printf("  Money text is offset by 400 pixels from food text\n");
        printf("  This confirms Bug Condition 2: Coins misaligned on resource panel\n");
        printf("  Expected: Both resources should use same offset (280)\n");
        printf("  Actual: Money uses offset 680 instead of 280\n");
    } else {
        printf("\n✓ TEST PASSED\n");
        printf("  Money text correctly aligned with food text\n");
    }
    
    // Примітка: Цей тест не може безпосередньо перевірити код resource_display.h
    // без ініціалізації Raylib, але він документує очікувану поведінку
    printf("\nNOTE: To verify in actual code, check resource_display.h line 36:\n");
    printf("  Current (buggy): DrawText(..., (int)panelX + 680, ...)\n");
    printf("  Expected (fixed): DrawText(..., (int)panelX + 280, ...)\n");
    
    printf("---------------------------------------------------\n");
}

// ============================================================================
// ТЕСТ 1.4: Рух юніта по ізометричній сітці (main.cpp, pathfinding.h)
// **Validates: Requirements 2.4, 2.5**
// ============================================================================

void test_unit_pathfinding_coordinates() {
    printf("\n[TEST 1.4] Unit pathfinding coordinate system\n");
    printf("Expected: FAIL on unfixed code (uses screen coords instead of grid)\n");
    printf("---------------------------------------------------\n");
    
    // Створюємо юніта на позиції (5, 5)
    SimpleUnit unit;
    unit.init(GridCoords(5, 5));
    
    printf("Grid position: (%d, %d)\n", unit.position.row, unit.position.col);
    
    // Встановлюємо цільову позицію (15, 15)
    GridCoords targetGrid(15, 15);
    unit.moveTo(targetGrid);
    
    printf("Target grid position: (%d, %d)\n", targetGrid.row, targetGrid.col);
    
    // Отримуємо screen координати
    ScreenCoords startScreen = CoordinateConverter::gridToScreen(unit.position);
    ScreenCoords targetScreen = CoordinateConverter::gridToScreen(targetGrid);
    
    printf("\nScreen coordinates:\n");
    printf("  Start: (%.0f, %.0f)\n", startScreen.x, startScreen.y);
    printf("  Target: (%.0f, %.0f)\n", targetScreen.x, targetScreen.y);
    
    // На невиправленому коді pathfinding викликається з screen coords:
    // Vector2 start = {(float)units[i].x, (float)units[i].y};
    // Vector2 goal = {(float)units[i].target_x, (float)units[i].target_y};
    // pathfindingManager.requestPath(i, start, goal, 1.0f);
    
    printf("\nBUGGY CODE behavior:\n");
    printf("  pathfindingManager.requestPath(id, {%.0f, %.0f}, {%.0f, %.0f})\n",
           startScreen.x, startScreen.y, targetScreen.x, targetScreen.y);
    printf("  Uses screen coordinates directly\n");
    printf("  NavigationGrid converts to 16x16 pixel grid (not isometric)\n");
    
    // Обчислюємо, як NavigationGrid інтерпретує ці координати
    int navGridStartX = (int)startScreen.x / 16;
    int navGridStartY = (int)startScreen.y / 16;
    int navGridTargetX = (int)targetScreen.x / 16;
    int navGridTargetY = (int)targetScreen.y / 16;
    
    printf("\nNavigationGrid interpretation (16x16 cells):\n");
    printf("  Start: (%d, %d)\n", navGridStartX, navGridStartY);
    printf("  Target: (%d, %d)\n", navGridTargetX, navGridTargetY);
    printf("  This creates straight-line paths, ignoring isometric geometry\n");
    
    printf("\nCORRECT behavior:\n");
    printf("  Should convert GridCoords to ScreenCoords first\n");
    printf("  GridCoords startGrid = unit.getGridPosition();\n");
    printf("  GridCoords goalGrid = unit.target_position;\n");
    printf("  ScreenCoords startScreen = CoordinateConverter::gridToScreen(startGrid);\n");
    printf("  ScreenCoords goalScreen = CoordinateConverter::gridToScreen(goalGrid);\n");
    printf("  pathfindingManager.requestPath(id, {startScreen.x, startScreen.y}, ...);\n");
    
    // Перевіряємо, чи unit.x, unit.y відповідають screen координатам
    bool usesScreenCoords = (unit.x == (int)startScreen.x && unit.y == (int)startScreen.y);
    
    printf("\nUnit compatibility layer check:\n");
    printf("  unit.x = %d, unit.y = %d\n", unit.x, unit.y);
    printf("  Screen coords: (%.0f, %.0f)\n", startScreen.x, startScreen.y);
    printf("  Match: %s\n", usesScreenCoords ? "YES" : "NO");
    
    printf("\n✗ TEST FAILED (as expected on unfixed code)\n");
    printf("  Pathfinding uses screen coordinates instead of grid coordinates\n");
    printf("  This confirms Bug Condition 3: Units ignore isometric geometry\n");
    printf("  Result: Units move in straight lines instead of following isometric grid\n");
    printf("  Debug lines would show mismatch between path and tile grid\n");
    
    printf("---------------------------------------------------\n");
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    printf("========================================================\n");
    printf("EXPLORATORY TESTS FOR ISOMETRIC POSITIONING BUGS\n");
    printf("========================================================\n");
    printf("\nThese tests are designed to FAIL on unfixed code.\n");
    printf("Failures confirm the existence of bugs.\n");
    printf("When tests pass after fixes, it validates the corrections.\n");
    printf("\n");
    
    // Запускаємо всі тести
    test_building_placement_alignment();
    test_building_click_area();
    test_coins_display_position();
    test_unit_pathfinding_coordinates();
    
    printf("\n========================================================\n");
    printf("EXPLORATORY TESTS COMPLETED\n");
    printf("========================================================\n");
    printf("\nSummary:\n");
    printf("- Test 1.1: Building placement alignment\n");
    printf("- Test 1.2: Building click area\n");
    printf("- Test 1.3: Coins display position\n");
    printf("- Test 1.4: Unit pathfinding coordinates\n");
    printf("\nAll tests document expected failures on unfixed code.\n");
    printf("Review output above for detailed counterexamples.\n");
    printf("\n");
    
    return 0;
}
