// Тест рефакторингу: перевірка констант та logger
#include <cassert>
#include <cstdio>
#include "../src/game_constants.h"
#include "../src/debug_logger.h"

void test_constants() {
    printf("[TEST] Testing game constants...\n");
    
    // Перевірка розмірів карти
    assert(GameConstants::MAP_WIDTH == 80);
    assert(GameConstants::MAP_HEIGHT == 80);
    
    // Перевірка початкових ресурсів
    assert(GameConstants::StartingResources::ROME_FOOD == 200);
    assert(GameConstants::StartingResources::ROME_MONEY == 100);
    assert(GameConstants::StartingResources::CARTHAGE_FOOD == 150);
    assert(GameConstants::StartingResources::CARTHAGE_MONEY == 200);
    
    // Перевірка UI розмірів
    assert(GameConstants::UI::WINDOW_WIDTH == 1434);
    assert(GameConstants::UI::WINDOW_HEIGHT == 1075);
    
    // Перевірка spawn констант
    assert(GameConstants::Spawn::MIN_HQ_DISTANCE == 20);
    assert(GameConstants::Spawn::MAX_SPAWN_ATTEMPTS == 100);
    assert(GameConstants::Spawn::REDUCE_DISTANCE_AFTER == 50);
    
    // Перевірка ресурсних точок
    assert(GameConstants::ResourcePoints::FOOD_SOURCES_COUNT == 8);
    assert(GameConstants::ResourcePoints::GOLD_SOURCES_COUNT == 6);
    
    printf("[TEST] Constants test PASSED\n");
}

void test_logger() {
    printf("[TEST] Testing debug logger...\n");
    
    // Тест різних категорій логування
    LOG_CLICK("[TEST] Click log test\n");
    LOG_SPAWN("[TEST] Spawn log test\n");
    LOG_BUILDING("[TEST] Building log test\n");
    LOG_TEXTURE("[TEST] Texture log test\n");
    LOG_ERROR("[TEST] Error log test\n");
    LOG_WARNING("[TEST] Warning log test\n");
    
    // Перевірка що категорії можна вимикати
    bool pathfindingEnabled = GameConstants::Debug::ENABLE_PATHFINDING_LOGS;
    bool clickEnabled = GameConstants::Debug::ENABLE_CLICK_LOGS;
    
    printf("[TEST] Pathfinding logs: %s\n", pathfindingEnabled ? "ENABLED" : "DISABLED");
    printf("[TEST] Click logs: %s\n", clickEnabled ? "ENABLED" : "DISABLED");
    
    printf("[TEST] Logger test PASSED\n");
}

void test_logger_categories() {
    printf("[TEST] Testing logger category control...\n");
    
    // Перевірка що категорії правильно налаштовані
    assert(DebugLogger::isEnabled(DebugLogger::Category::SPAWN) == GameConstants::Debug::ENABLE_SPAWN_LOGS);
    assert(DebugLogger::isEnabled(DebugLogger::Category::BUILDING) == GameConstants::Debug::ENABLE_BUILDING_LOGS);
    assert(DebugLogger::isEnabled(DebugLogger::Category::PATHFINDING) == GameConstants::Debug::ENABLE_PATHFINDING_LOGS);
    assert(DebugLogger::isEnabled(DebugLogger::Category::CLICK) == GameConstants::Debug::ENABLE_CLICK_LOGS);
    
    // Помилки та попередження завжди увімкнені
    assert(DebugLogger::isEnabled(DebugLogger::Category::TEXTURE) == true);
    assert(DebugLogger::isEnabled(DebugLogger::Category::GENERAL) == true);
    
    printf("[TEST] Logger categories test PASSED\n");
}

int main() {
    printf("\n=== REFACTORING TESTS ===\n\n");
    
    test_constants();
    test_logger();
    test_logger_categories();
    
    printf("\n=== ALL TESTS PASSED ===\n");
    return 0;
}
