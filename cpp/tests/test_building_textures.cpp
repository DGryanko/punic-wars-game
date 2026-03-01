// Unit тести для BuildingTextureManager
// TODO: Додати повноцінні тести з testing framework

#include "../src/building_texture_manager.h"
#include <cassert>
#include <cstdio>

void test_singleton() {
    BuildingTextureManager& mgr1 = BuildingTextureManager::getInstance();
    BuildingTextureManager& mgr2 = BuildingTextureManager::getInstance();
    
    // Перевіряємо що це той самий екземпляр
    assert(&mgr1 == &mgr2);
    printf("[TEST] Singleton test passed\n");
}

void test_load_textures() {
    BuildingTextureManager& mgr = BuildingTextureManager::getInstance();
    
    // Завантажуємо текстури
    mgr.loadAllTextures();
    
    // Перевіряємо що всі типи мають текстури
    assert(mgr.hasTexture(HQ_ROME));
    assert(mgr.hasTexture(HQ_CARTHAGE));
    assert(mgr.hasTexture(BARRACKS_ROME));
    assert(mgr.hasTexture(BARRACKS_CARTHAGE));
    assert(mgr.hasTexture(QUESTORIUM_ROME));
    
    printf("[TEST] Load textures test passed\n");
}

void test_texture_caching() {
    BuildingTextureManager& mgr = BuildingTextureManager::getInstance();
    
    // Отримуємо текстуру двічі
    Texture2D tex1 = mgr.getTexture(HQ_ROME);
    Texture2D tex2 = mgr.getTexture(HQ_ROME);
    
    // Перевіряємо що це той самий ID (кешування)
    assert(tex1.id == tex2.id);
    printf("[TEST] Texture caching test passed\n");
}

int main() {
    printf("[TEST] Running BuildingTextureManager tests...\n");
    
    // Ініціалізуємо Raylib для тестів
    InitWindow(800, 600, "Texture Tests");
    
    test_singleton();
    test_load_textures();
    test_texture_caching();
    
    // Cleanup
    BuildingTextureManager::getInstance().unloadAll();
    CloseWindow();
    
    printf("[TEST] All tests passed!\n");
    return 0;
}
