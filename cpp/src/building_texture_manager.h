#pragma once
#include "raylib.h"
#include "building.h"
#include <map>

// Singleton клас для управління текстурами будівель
class BuildingTextureManager {
private:
    static BuildingTextureManager* instance;
    std::map<BuildingType, Texture2D> textures;
    bool initialized;
    
    // Приватний конструктор для singleton
    BuildingTextureManager();
    
    // Заборонити копіювання
    BuildingTextureManager(const BuildingTextureManager&) = delete;
    BuildingTextureManager& operator=(const BuildingTextureManager&) = delete;
    
public:
    ~BuildingTextureManager();
    
    // Отримати єдиний екземпляр
    static BuildingTextureManager& getInstance();
    
    // Завантаження всіх текстур
    void loadAllTextures();
    
    // Отримання текстури за типом
    Texture2D getTexture(BuildingType type) const;
    
    // Перевірка чи текстура завантажена
    bool hasTexture(BuildingType type) const;
    
    // Звільнення всіх ресурсів
    void unloadAll();
    
    // Перевірка чи менеджер ініціалізований
    bool isInitialized() const { return initialized; }
    
private:
    // Завантаження окремої текстури
    bool loadTexture(BuildingType type, const char* filepath);
    
    // Створення fallback текстури
    Texture2D createFallbackTexture(BuildingType type);
    
    // Отримання шляху до текстури за типом
    const char* getTexturePath(BuildingType type) const;
};
