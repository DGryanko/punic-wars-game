#ifndef NOISE_H
#define NOISE_H

#include <vector>
#include <cmath>
#include <random>

class NoiseGenerator {
private:
    unsigned int seed;
    float scale;
    int octaves;
    float persistence;
    float lacunarity;
    
    // Permutation table для Perlin Noise
    std::vector<int> permutation;
    
    // Ініціалізація permutation table
    void initPermutation();
    
    // Fade function для згладжування
    float fade(float t) const;
    
    // Лінійна інтерполяція
    float lerp(float a, float b, float t) const;
    
    // Градієнтна функція
    float grad(int hash, float x, float y) const;
    
public:
    NoiseGenerator(unsigned int s, float sc = 0.1f);
    
    // Генерація базового Perlin Noise
    float noise2D(float x, float y);
    
    // Генерація багатооктавного (fractal) шуму
    float fractalNoise2D(float x, float y);
    
    // Налаштування параметрів
    void setScale(float s) { scale = s; }
    void setOctaves(int o) { octaves = o; }
    void setPersistence(float p) { persistence = p; }
    void setLacunarity(float l) { lacunarity = l; }
    
    float getScale() const { return scale; }
    int getOctaves() const { return octaves; }
};

#endif // NOISE_H
