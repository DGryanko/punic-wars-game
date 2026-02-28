#include "noise.h"
#include <algorithm>

NoiseGenerator::NoiseGenerator(unsigned int s, float sc)
    : seed(s), scale(sc), octaves(4), persistence(0.5f), lacunarity(2.0f) {
    initPermutation();
}

void NoiseGenerator::initPermutation() {
    // Створюємо permutation table розміром 512
    permutation.resize(512);
    
    // Заповнюємо першу половину числами 0-255
    std::vector<int> p(256);
    for (int i = 0; i < 256; i++) {
        p[i] = i;
    }
    
    // Перемішуємо з використанням seed
    std::mt19937 rng(seed);
    std::shuffle(p.begin(), p.end(), rng);
    
    // Дублюємо для уникнення overflow
    for (int i = 0; i < 256; i++) {
        permutation[i] = p[i];
        permutation[256 + i] = p[i];
    }
}

float NoiseGenerator::fade(float t) const {
    // Fade function: 6t^5 - 15t^4 + 10t^3
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float NoiseGenerator::lerp(float a, float b, float t) const {
    return a + t * (b - a);
}

float NoiseGenerator::grad(int hash, float x, float y) const {
    // Конвертуємо нижні 4 біти hash в один з 12 градієнтних векторів
    int h = hash & 15;
    float u = h < 8 ? x : y;
    float v = h < 4 ? y : (h == 12 || h == 14 ? x : 0);
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

float NoiseGenerator::noise2D(float x, float y) {
    // Знаходимо координати unit square
    int X = static_cast<int>(std::floor(x)) & 255;
    int Y = static_cast<int>(std::floor(y)) & 255;
    
    // Знаходимо відносні координати в square
    x -= std::floor(x);
    y -= std::floor(y);
    
    // Обчислюємо fade curves
    float u = fade(x);
    float v = fade(y);
    
    // Hash координат 4 кутів square
    int A = permutation[X] + Y;
    int AA = permutation[A];
    int AB = permutation[A + 1];
    int B = permutation[X + 1] + Y;
    int BA = permutation[B];
    int BB = permutation[B + 1];
    
    // Інтерполюємо результати від 4 кутів
    float res = lerp(
        lerp(grad(permutation[AA], x, y),
             grad(permutation[BA], x - 1, y), u),
        lerp(grad(permutation[AB], x, y - 1),
             grad(permutation[BB], x - 1, y - 1), u),
        v
    );
    
    // Нормалізуємо до діапазону [0, 1]
    return (res + 1.0f) / 2.0f;
}

float NoiseGenerator::fractalNoise2D(float x, float y) {
    float total = 0.0f;
    float frequency = scale;
    float amplitude = 1.0f;
    float maxValue = 0.0f;  // Для нормалізації
    
    for (int i = 0; i < octaves; i++) {
        total += noise2D(x * frequency, y * frequency) * amplitude;
        
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    
    // Нормалізуємо до діапазону [0, 1]
    return total / maxValue;
}
