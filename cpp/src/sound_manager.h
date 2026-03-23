#pragma once
#include "raylib.h"
#include <cmath>
#include <vector>

// ─── Sound Manager ────────────────────────────────────────────────────────────
struct SoundManager {
    Sound hq      = {0};
    Sound walking = {0};
    Sound hammer  = {0};
    Sound battle  = {0};
    Sound die     = {0};
    Sound kntb    = {0}; // Contubernium click
    Sound qstr    = {0}; // Questorium click
    Sound coin    = {0}; // Tentorium / trade

    bool loaded = false;

    float dieVolume  = 0.80f;
    float hqVolume   = 0.90f;
    float kntbVolume = 0.85f;
    float qstrVolume = 0.85f;
    float coinVolume = 0.90f;

    void load() {
        auto tryLoad = [](const char* path, Sound& s) {
            if (FileExists(path)) {
                s = LoadSound(path);
            } else {
                TraceLog(LOG_WARNING, "[SFX] Missing: %s", path);
            }
        };
        tryLoad("assets/sounds/hq.mp3",      hq);
        tryLoad("assets/sounds/walking.mp3", walking);
        tryLoad("assets/sounds/hammer.mp3",  hammer);
        tryLoad("assets/sounds/battle.mp3",  battle);
        tryLoad("assets/sounds/die.mp3",     die);
        tryLoad("assets/sounds/kntb.mp3",    kntb);
        tryLoad("assets/sounds/qstr.mp3",    qstr);
        tryLoad("assets/sounds/coin.mp3",    coin);

        SetSoundVolume(hq,   hqVolume);
        SetSoundVolume(die,  dieVolume);
        SetSoundVolume(kntb, kntbVolume);
        SetSoundVolume(qstr, qstrVolume);
        SetSoundVolume(coin, coinVolume);
        // walking, hammer, battle — гучність задається динамічно

        loaded = true;
    }

    void unload() {
        if (!loaded) return;
        auto tryUnload = [](Sound& s) { if (s.stream.buffer) UnloadSound(s); };
        tryUnload(hq); tryUnload(walking); tryUnload(hammer);
        tryUnload(battle); tryUnload(die);
        tryUnload(kntb); tryUnload(qstr); tryUnload(coin);
        loaded = false;
    }

    // Чи видима world-позиція на екрані
    bool isOnScreen(Vector2 worldPos, const Camera2D& camera, float margin = 80.0f) const {
        Vector2 sc = GetWorldToScreen2D(worldPos, camera);
        float sw = (float)GetScreenWidth();
        float sh = (float)GetScreenHeight();
        return sc.x >= -margin && sc.x <= sw + margin &&
               sc.y >= -margin && sc.y <= sh + margin;
    }

    // Гучність залежно від відстані до джерела (0..1)
    // Відстань рахується в screen pixels — автоматично враховує zoom
    float distanceVolume(Vector2 worldPos, const Camera2D& camera,
                         float maxScreenDist = 500.0f, float baseVolume = 1.0f) const {
        Vector2 sc = GetWorldToScreen2D(worldPos, camera);
        float sw = (float)GetScreenWidth();
        float sh = (float)GetScreenHeight();
        float cx = sw * 0.5f;
        float cy = sh * 0.5f;
        float dx = sc.x - cx;
        float dy = sc.y - cy;
        float dist = sqrtf(dx*dx + dy*dy);
        float vol = 1.0f - (dist / maxScreenDist);
        vol = (vol < 0.0f) ? 0.0f : (vol > 1.0f ? 1.0f : vol);
        return vol * baseVolume;
    }

    // Відтворити one-shot якщо на екрані
    void playOnceIfVisible(Sound& s, Vector2 worldPos, const Camera2D& camera) const {
        if (!loaded || !s.stream.buffer) return;
        if (!isOnScreen(worldPos, camera)) return;
        PlaySound(s);
    }

    // Зупинити якщо не потрібен
    void stopIfNotNeeded(Sound& s, bool anyActive) const {
        if (!anyActive && IsSoundPlaying(s)) StopSound(s);
    }

    // Оновити позиційний looped звук: знаходимо найближче джерело, виставляємо гучність
    // sources — список world positions активних джерел
    // effectsVolume — з налаштувань гравця [0..1]
    void updatePositional(Sound& s, const std::vector<Vector2>& sources,
                          const Camera2D& camera, float effectsVolume,
                          float maxScreenDist = 700.0f, float volumeBoost = 1.0f) const {
        if (!loaded || !s.stream.buffer) return;
        if (sources.empty()) {
            if (IsSoundPlaying(s)) StopSound(s);
            return;
        }
        // Знаходимо найближче до центру екрану джерело (в screen coords)
        float bestVol = 0.0f;
        for (const auto& pos : sources) {
            float v = distanceVolume(pos, camera, maxScreenDist, effectsVolume * volumeBoost);
            if (v > bestVol) bestVol = v;
        }
        // Мінімальна гучність якщо джерело на екрані — щоб звук не зникав
        if (bestVol < 0.05f) bestVol = 0.05f * effectsVolume * volumeBoost;
        if (bestVol > 1.0f) bestVol = 1.0f;

        SetSoundVolume(s, bestVol);
        if (!IsSoundPlaying(s)) PlaySound(s);
    }
};

inline SoundManager sfx;
