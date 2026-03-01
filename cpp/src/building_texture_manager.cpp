#include "building_texture_manager.h"
#include <cstdio>

// Ініціалізація статичного члена
BuildingTextureManager* BuildingTextureManager::instance = nullptr;

BuildingTextureManager::BuildingTextureManager() : initialized(false) {
    // Порожній конструктор
}

BuildingTextureManager::~BuildingTextureManager() {
    unloadAll();
}

BuildingTextureManager& BuildingTextureManager::getInstance() {
    if (instance == nullptr) {
        instance = new BuildingTextureManager();
    }
    return *instance;
}

const char* BuildingTextureManager::getTexturePath(BuildingType type) const {
    switch (type) {
        case HQ_ROME:
            return "assets/sprites/Praetorium.png";
        case HQ_CARTHAGE:
            return "assets/sprites/MainTent.png";
        case BARRACKS_ROME:
            return "assets/sprites/Contubernium.png";
        case BARRACKS_CARTHAGE:
            return "assets/sprites/MercenaryCamp.png";
        case QUESTORIUM_ROME:
            return "assets/sprites/Questorium.png";
        case LIBTENT_1:
            return "assets/sprites/LibTent1.png";
        case LIBTENT_2:
            return "assets/sprites/LibTent2.png";
        case LIBTENT_3:
            return "assets/sprites/LibTent3.png";
        case TENTORIUM:
            return "assets/sprites/Tentorium.png";
        default:
            return nullptr;
    }
}

Texture2D BuildingTextureManager::createFallbackTexture(BuildingType type) {
    // Створюємо кольорову текстуру як fallback
    Color fallbackColor;
    
    // Визначаємо колір залежно від типу
    switch (type) {
        case HQ_ROME:
        case BARRACKS_ROME:
        case QUESTORIUM_ROME:
            fallbackColor = RED;
            break;
        case HQ_CARTHAGE:
        case BARRACKS_CARTHAGE:
            fallbackColor = BLUE;
            break;
        default:
            fallbackColor = GRAY;
            break;
    }
    
    // Створюємо просте зображення 80x60 пікселів
    Image fallbackImage = GenImageColor(80, 60, fallbackColor);
    Texture2D fallbackTexture = LoadTextureFromImage(fallbackImage);
    UnloadImage(fallbackImage);
    
    printf("[TEXTURE] Created fallback texture for building type %d\n", type);
    
    return fallbackTexture;
}

bool BuildingTextureManager::loadTexture(BuildingType type, const char* filepath) {
    if (filepath == nullptr) {
        printf("[TEXTURE] Error: No filepath for building type %d\n", type);
        textures[type] = createFallbackTexture(type);
        return false;
    }
    
    // Перевіряємо чи файл існує
    if (!FileExists(filepath)) {
        printf("[TEXTURE] Error: File not found: %s\n", filepath);
        textures[type] = createFallbackTexture(type);
        return false;
    }
    
    // Завантажуємо текстуру
    Texture2D texture = LoadTexture(filepath);
    
    if (texture.id == 0) {
        printf("[TEXTURE] Error: Failed to load texture: %s\n", filepath);
        textures[type] = createFallbackTexture(type);
        return false;
    }
    
    printf("[TEXTURE] Loaded texture: %s (%dx%d)\n", filepath, texture.width, texture.height);
    textures[type] = texture;
    return true;
}

void BuildingTextureManager::loadAllTextures() {
    if (initialized) {
        printf("[TEXTURE] Warning: Textures already loaded\n");
        return;
    }
    
    printf("[TEXTURE] Loading all building textures...\n");
    
    // Завантажуємо текстури для всіх типів будівель
    loadTexture(HQ_ROME, getTexturePath(HQ_ROME));
    loadTexture(HQ_CARTHAGE, getTexturePath(HQ_CARTHAGE));
    loadTexture(BARRACKS_ROME, getTexturePath(BARRACKS_ROME));
    loadTexture(BARRACKS_CARTHAGE, getTexturePath(BARRACKS_CARTHAGE));
    loadTexture(QUESTORIUM_ROME, getTexturePath(QUESTORIUM_ROME));
    loadTexture(LIBTENT_1, getTexturePath(LIBTENT_1));
    loadTexture(LIBTENT_2, getTexturePath(LIBTENT_2));
    loadTexture(LIBTENT_3, getTexturePath(LIBTENT_3));
    loadTexture(TENTORIUM, getTexturePath(TENTORIUM));
    
    initialized = true;
    printf("[TEXTURE] All building textures loaded (%d total)\n", (int)textures.size());
}

Texture2D BuildingTextureManager::getTexture(BuildingType type) const {
    auto it = textures.find(type);
    if (it != textures.end()) {
        return it->second;
    }
    
    // Повертаємо порожню текстуру якщо не знайдено
    printf("[TEXTURE] Warning: Texture not found for building type %d\n", type);
    return Texture2D{0};
}

bool BuildingTextureManager::hasTexture(BuildingType type) const {
    auto it = textures.find(type);
    return it != textures.end() && it->second.id != 0;
}

void BuildingTextureManager::unloadAll() {
    if (!initialized) {
        return;
    }
    
    printf("[TEXTURE] Unloading all building textures...\n");
    
    for (auto& pair : textures) {
        if (pair.second.id != 0) {
            UnloadTexture(pair.second);
        }
    }
    
    textures.clear();
    initialized = false;
    
    printf("[TEXTURE] All textures unloaded\n");
}
