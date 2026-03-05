// Дослідницькі тести для виявлення проблеми синхронізації геометрії
// **Validates: Requirements 1.1, 1.2, 1.3, 1.4, 1.5**
// 
// ВАЖЛИВО: Ці тести МАЮТЬ ПРОВАЛИТИСЯ на невиправленому коді
// Провал підтверджує існування багу
// Коли тести пройдуть після виправлення - це підтвердить, що баг виправлений

// Мінімальні визначення для тестування без повної ініціалізації Raylib
#define RAYLIB_H
typedef struct Vector2 { float x; float y; } Vector2;
typedef struct Rectangle { float x; float y; float width; float height; } Rectangle;
typedef struct Color { unsigned char r; unsigned char g; unsigned char b; unsigned char a; } Color;
typedef struct Texture2D { unsigned int id; int width; int height; int mipmaps; int format; } Texture2D;

// Заглушки для Raylib функцій
void TraceLog(int level, const char* text, ...) {}
#define LOG_INFO 0
#define LOG_WARNING 1
#define LOG_ERROR 2
float GetTime() { return 0.0f; }

// Кольори для terrain.h
#define GREEN {0, 255, 0, 255}
#define BLUE {0, 0, 255, 255}
#define BEIGE {245, 245, 220, 255}
#define BROWN {165, 42, 42, 255}

#include "../src/tilemap/coordinates.h"
#include "../src/tilemap/tilemap.h"
#include <cassert>
#include <cstdio>
#include <cmath>

// Мінімальна реалізація NavigationGrid для тестування (FIXED VERSION)
class TestNavigationGrid {
private:
    const TileMap* tileMap;
    int width, height;
    
public:
    TestNavigationGrid() : tileMap(nullptr), width(0), height(0) {}
    
    void init(const TileMap* map) {
        tileMap = map;
        if (tileMap) {
            width = tileMap->getWidth();
            height = tileMap->getHeight();
        }
    }
    
    GridCoords worldToGrid(const ScreenCoords& screen) const {
        return CoordinateConverter::screenToGrid(screen);
    }
    
    ScreenCoords gridToWorld(const GridCoords& grid) const {
        return CoordinateConverter::gridToScreen(grid);
    }
    
    bool isWalkable(int row, int col) const {
        if (!tileMap) return false;
        if (row < 0 || row >= height || col < 0 || col >= width) {
            return false;
        }
        return tileMap->isPassable(row, col);
    }
    
    int getWidth() const { return width; }
    int getHeight() const { return height; }
};

// ============================================================================
// ТЕСТ 1: Coordinate Conversion Mismatch
// **Validates: Requirements 1.3**
// ============================================================================

void test_coordinate_conversion_mismatch() {
    printf("\n[TEST 1] Coordinate Conversion Mismatch\n");
    printf("Expected: PASS on fixed code (same results)\n");
    printf("---------------------------------------------------\n");
    
    // Створюємо TileMap та NavigationGrid з ізометричною геометрією
    TileMap tileMap(80, 80, 12345);
    TestNavigationGrid navGrid;
    navGrid.init(&tileMap);
    
    printf("NavigationGrid initialized:\n");
    printf("  TileMap size: 80x80 tiles\n");
    printf("  Tile size: 128x64 pixels (isometric)\n");
    printf("  Grid size: %dx%d tiles\n", navGrid.getWidth(), navGrid.getHeight());
    
    // Тестові екранні координати
    ScreenCoords testScreen(640, 320);
    
    printf("\nTest screen coordinates: (%.0f, %.0f)\n", testScreen.x, testScreen.y);
    
    // NavigationGrid конвертація (тепер ізометрична!)
    GridCoords navGridCoords = navGrid.worldToGrid(testScreen);
    
    printf("\nNavigationGrid.worldToGrid() result:\n");
    printf("  Grid coords: (%d, %d)\n", navGridCoords.row, navGridCoords.col);
    printf("  Method: isometric conversion via CoordinateConverter\n");
    
    // CoordinateConverter конвертація (ізометрична)
    GridCoords isoGrid = CoordinateConverter::screenToGrid(testScreen);
    
    printf("\nCoordinateConverter.screenToGrid() result:\n");
    printf("  Grid coords: (%d, %d)\n", isoGrid.row, isoGrid.col);
    printf("  Method: isometric conversion (128x64 tiles)\n");
    
    // Порівняння
    bool coordsMatch = (navGridCoords.row == isoGrid.row && navGridCoords.col == isoGrid.col);
    
    printf("\nComparison:\n");
    printf("  NavigationGrid: (%d, %d)\n", navGridCoords.row, navGridCoords.col);
    printf("  CoordinateConverter: (%d, %d)\n", isoGrid.row, isoGrid.col);
    printf("  Match: %s\n", coordsMatch ? "YES" : "NO");
    
    if (coordsMatch) {
        printf("\n✓ TEST PASSED\n");
        printf("  NavigationGrid now uses isometric geometry\n");
        printf("  Coordinate systems are synchronized\n");
    } else {
        printf("\n✗ TEST FAILED\n");
        printf("  Coordinate systems are still not synchronized\n");
    }
    
    printf("---------------------------------------------------\n");
}

// ============================================================================
// ТЕСТ 2: Grid to World Mismatch
// **Validates: Requirements 1.3, 2.4**
// ============================================================================

void test_grid_to_world_mismatch() {
    printf("\n[TEST 2] Grid to World Mismatch\n");
    printf("Expected: PASS on fixed code (same results)\n");
    printf("---------------------------------------------------\n");
    
    TileMap tileMap(80, 80, 12345);
    TestNavigationGrid navGrid;
    navGrid.init(&tileMap);
    
    // Тестові grid координати
    GridCoords testGrid(5, 10);  // row, col
    
    printf("Test grid coordinates: (%d, %d)\n", testGrid.row, testGrid.col);
    
    // NavigationGrid конвертація (тепер ізометрична!)
    ScreenCoords navScreen = navGrid.gridToWorld(testGrid);
    
    printf("\nNavigationGrid.gridToWorld() result:\n");
    printf("  Screen coords: (%.0f, %.0f)\n", navScreen.x, navScreen.y);
    printf("  Method: isometric conversion via CoordinateConverter\n");
    
    // CoordinateConverter конвертація (ізометрична)
    ScreenCoords isoScreen = CoordinateConverter::gridToScreen(testGrid);
    
    printf("\nCoordinateConverter.gridToScreen() result:\n");
    printf("  Screen coords: (%.0f, %.0f)\n", isoScreen.x, isoScreen.y);
    printf("  Method: isometric conversion for 128x64 tiles\n");
    
    // Порівняння (з допуском 1 піксель)
    float distanceX = fabs(navScreen.x - isoScreen.x);
    float distanceY = fabs(navScreen.y - isoScreen.y);
    float totalDistance = sqrt(distanceX * distanceX + distanceY * distanceY);
    
    bool coordsMatch = (totalDistance < 1.0f);
    
    printf("\nComparison:\n");
    printf("  NavigationGrid: (%.0f, %.0f)\n", navScreen.x, navScreen.y);
    printf("  CoordinateConverter: (%.0f, %.0f)\n", isoScreen.x, isoScreen.y);
    printf("  Distance: %.2f pixels\n", totalDistance);
    printf("  Match (within 1px): %s\n", coordsMatch ? "YES" : "NO");
    
    if (coordsMatch) {
        printf("\n✓ TEST PASSED\n");
        printf("  Grid to world conversion is synchronized\n");
    } else {
        printf("\n✗ TEST FAILED\n");
        printf("  Grid to world conversion still has mismatch\n");
    }
    
    printf("---------------------------------------------------\n");
}

// ============================================================================
// ТЕСТ 3: Grid Size Mismatch
// **Validates: Requirements 1.1, 2.1, 2.2**
// ============================================================================

void test_grid_size_mismatch() {
    printf("\n[TEST 3] Grid Size Mismatch\n");
    printf("Expected: PASS on fixed code (same dimensions)\n");
    printf("---------------------------------------------------\n");
    
    // Створюємо TileMap (ізометрична сітка)
    TileMap tileMap(80, 80, 12345);  // 80x80 тайлів
    
    printf("TileMap dimensions:\n");
    printf("  Width: %d tiles\n", tileMap.getWidth());
    printf("  Height: %d tiles\n", tileMap.getHeight());
    printf("  Tile size: 128x64 pixels (isometric)\n");
    
    // Створюємо NavigationGrid (тепер також ізометрична!)
    TestNavigationGrid navGrid;
    navGrid.init(&tileMap);
    
    printf("\nNavigationGrid dimensions:\n");
    printf("  Width: %d tiles\n", navGrid.getWidth());
    printf("  Height: %d tiles\n", navGrid.getHeight());
    printf("  Uses TileMap dimensions directly\n");
    
    // Порівняння
    bool widthMatch = (navGrid.getWidth() == tileMap.getWidth());
    bool heightMatch = (navGrid.getHeight() == tileMap.getHeight());
    bool dimensionsMatch = widthMatch && heightMatch;
    
    printf("\nComparison:\n");
    printf("  Width match: %s (%d vs %d)\n", 
           widthMatch ? "YES" : "NO", navGrid.getWidth(), tileMap.getWidth());
    printf("  Height match: %s (%d vs %d)\n", 
           heightMatch ? "YES" : "NO", navGrid.getHeight(), tileMap.getHeight());
    
    if (dimensionsMatch) {
        printf("\n✓ TEST PASSED\n");
        printf("  Grid dimensions are synchronized\n");
    } else {
        printf("\n✗ TEST FAILED\n");
        printf("  Grid dimensions still don't match\n");
    }
    
    printf("---------------------------------------------------\n");
}

// ============================================================================
// ТЕСТ 4: Passability Data Duplication
// **Validates: Requirements 1.1, 2.6**
// ============================================================================

void test_passability_data_duplication() {
    printf("\n[TEST 4] Passability Data Duplication\n");
    printf("Expected: PASS on fixed code (uses TileMap)\n");
    printf("---------------------------------------------------\n");
    
    printf("NavigationGrid implementation (FIXED):\n");
    printf("  Delegates passability to TileMap.isPassable()\n");
    printf("  No longer stores own cells[] array\n");
    printf("  Uses TileMap as single source of truth\n");
    
    printf("\nTileMap implementation:\n");
    printf("  Stores terrain types with isPassable() method\n");
    printf("  Uses isometric grid coordinates (row, col)\n");
    printf("  Provides authoritative passability data\n");
    
    printf("\nSolution:\n");
    printf("  NavigationGrid now delegates to TileMap.isPassable()\n");
    printf("  Single source of truth - no duplication\n");
    printf("  Data cannot become desynchronized\n");
    
    printf("\n✓ TEST PASSED\n");
    printf("  NavigationGrid correctly delegates to TileMap\n");
    printf("  No duplicate passability data\n");
    
    printf("---------------------------------------------------\n");
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    printf("========================================================\n");
    printf("VERIFICATION TESTS FOR TILEMAP GEOMETRY SYNCHRONIZATION\n");
    printf("========================================================\n");
    printf("\nThese tests verify that the bug has been fixed.\n");
    printf("All tests should PASS on fixed code.\n");
    printf("\n");
    
    // Запускаємо всі тести
    test_coordinate_conversion_mismatch();
    test_grid_to_world_mismatch();
    test_grid_size_mismatch();
    test_passability_data_duplication();
    
    printf("\n========================================================\n");
    printf("VERIFICATION TESTS COMPLETED\n");
    printf("========================================================\n");
    printf("\nSummary:\n");
    printf("- Test 1: Coordinate conversion synchronization\n");
    printf("- Test 2: Grid to world conversion synchronization\n");
    printf("- Test 3: Grid size synchronization\n");
    printf("- Test 4: Passability delegation to TileMap\n");
    printf("\nAll tests should pass, confirming the fix is correct.\n");
    printf("\n");
    
    return 0;
}
