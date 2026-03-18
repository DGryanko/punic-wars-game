#pragma once
#include "raylib.h"
#include "tilemap/coordinates.h"
#include <string>
#include <vector>

enum AnimState { ANIM_IDLE=0, ANIM_WALK=1, ANIM_ATTACK=2, ANIM_DEATH=3 };
enum AnimDir   { DIR_FRONT_LEFT=0, DIR_FRONT_RIGHT=1, DIR_BACK_LEFT=2, DIR_BACK_RIGHT=3 };

struct AnimTrack {
    Texture2D sheet = {0};
    std::vector<Texture2D> frames;
    int frameW = 64, frameH = 64;
    int frameCount = 0, cols = 1;
    float fps = 8.0f;
    bool loaded = false, isSheet = false;

    AnimTrack() = default;
    AnimTrack(const AnimTrack&) = delete;
    AnimTrack& operator=(const AnimTrack&) = delete;

    AnimTrack(AnimTrack&& o) noexcept
        : sheet(o.sheet), frames(std::move(o.frames)),
          frameW(o.frameW), frameH(o.frameH),
          frameCount(o.frameCount), cols(o.cols),
          fps(o.fps), loaded(o.loaded), isSheet(o.isSheet) {
        o.sheet = {0}; o.loaded = false; o.isSheet = false; o.frameCount = 0;
    }
    AnimTrack& operator=(AnimTrack&& o) noexcept {
        if (this != &o) {
            unload();
            sheet = o.sheet; frames = std::move(o.frames);
            frameW = o.frameW; frameH = o.frameH;
            frameCount = o.frameCount; cols = o.cols;
            fps = o.fps; loaded = o.loaded; isSheet = o.isSheet;
            o.sheet = {0}; o.loaded = false; o.isSheet = false; o.frameCount = 0;
        }
        return *this;
    }
    ~AnimTrack() { unload(); }

    void loadSheet(const std::string& path, int fw, int fh, int count = 0) {
        unload();
        if (!FileExists(path.c_str())) {
            TraceLog(LOG_DEBUG, "[ANIM] File not found: %s", path.c_str());
            return;
        }
        sheet = LoadTexture(path.c_str());
        if (sheet.id == 0) {
            TraceLog(LOG_WARNING, "[ANIM] LoadTexture failed: %s", path.c_str());
            return;
        }
        frameW = fw; frameH = fh;
        cols   = sheet.width / fw;
        int rows = sheet.height / fh;
        frameCount = (count > 0) ? count : cols * rows;
        isSheet = true; loaded = true;
        TraceLog(LOG_INFO, "[ANIM] Loaded sheet: %s (%dx%d, %d frames)", path.c_str(), sheet.width, sheet.height, frameCount);
    }

    void load(const std::string& folder, int maxFrames, int fw = 64, int fh = 64) {
        unload();
        frameW = fw; frameH = fh;
        for (int i = 0; i < maxFrames; i++) {
            std::string p = folder + "/" + std::to_string(i) + ".png";
            if (FileExists(p.c_str())) frames.push_back(LoadTexture(p.c_str()));
            else break;
        }
        if (!frames.empty()) { frameCount = (int)frames.size(); loaded = true; }
    }

    void unload() {
        if (isSheet && sheet.id > 0) { UnloadTexture(sheet); sheet.id = 0; }
        for (auto& t : frames) if (t.id > 0) UnloadTexture(t);
        frames.clear(); loaded = false; isSheet = false; frameCount = 0;
    }

    void draw(float timer, bool loop, ScreenCoords pos, Color tint = WHITE) const {
        if (!loaded || frameCount == 0) return;
        int idx = (int)(timer * fps);
        idx = loop ? (idx % frameCount) : (idx >= frameCount ? frameCount - 1 : idx);
        if (isSheet) {
            int col = idx % cols, row = idx / cols;
            Rectangle src = { (float)(col * frameW), (float)(row * frameH),
                               (float)frameW, (float)frameH };
            DrawTextureRec(sheet, src, { pos.x - frameW * 0.5f, pos.y - (float)frameH }, tint);
        } else if (idx < (int)frames.size() && frames[idx].id > 0) {
            DrawTextureV(frames[idx], { pos.x - frameW * 0.5f, pos.y - (float)frameH }, tint);
        }
    }

    bool isFinished(float timer) const {
        return !loaded || (timer * fps) >= (float)frameCount;
    }
};

struct UnitAnimator {
    AnimTrack tracks[4][4];
    AnimState currentState = ANIM_IDLE;
    AnimDir   currentDir   = DIR_FRONT_LEFT;
    float     timer        = 0.0f;
    bool      loaded       = false;

    void loadSheets(const std::string& basePath, int fw = 64, int fh = 64,
                    float idleFps = 3.0f, float walkFps = 6.0f,
                    float attackFps = 12.0f, float deathFps = 6.0f) {
        const char* dn[4] = { "front_left","front_right","back_left","back_right" };
        TraceLog(LOG_INFO, "[ANIM] loadSheets: '%s' fw=%d fh=%d", basePath.c_str(), fw, fh);

        for (int d = 0; d < 4; d++) {
            tracks[ANIM_IDLE][d].fps = idleFps;
            tracks[ANIM_IDLE][d].loadSheet(basePath + "/idle.png", fw, fh);
        }
        for (int d = 0; d < 4; d++) {
            tracks[ANIM_WALK][d].fps = walkFps;
            tracks[ANIM_WALK][d].loadSheet(basePath + "/walk_" + dn[d] + ".png", fw, fh);
        }
        for (int d = 0; d < 4; d++) {
            tracks[ANIM_ATTACK][d].fps = attackFps;
            tracks[ANIM_ATTACK][d].loadSheet(basePath + "/attack_" + dn[d] + ".png", fw, fh);
        }
        for (int d = 0; d < 4; d++) {
            tracks[ANIM_DEATH][d].fps = deathFps;
            tracks[ANIM_DEATH][d].loadSheet(basePath + "/death.png", fw, fh);
        }

        loaded = false;
        for (int s = 0; s < 4 && !loaded; s++)
            for (int d = 0; d < 4 && !loaded; d++)
                if (tracks[s][d].loaded) loaded = true;

        TraceLog(LOG_INFO, "[ANIM] loadSheets done: loaded=%d  idle[0]=%d walk[0]=%d walk[1]=%d",
            loaded, tracks[ANIM_IDLE][0].loaded,
            tracks[ANIM_WALK][0].loaded, tracks[ANIM_WALK][1].loaded);
    }

    void load(const std::string& basePath, int fw = 64, int fh = 64) {
        const char* dn[4] = { "front_left","front_right","back_left","back_right" };
        TraceLog(LOG_INFO, "[ANIM] load (individual files): '%s'", basePath.c_str());
        for (int d=0;d<4;d++){tracks[ANIM_IDLE][d].fps=2.0f;    tracks[ANIM_IDLE][d].load(basePath+"/idle",4,fw,fh);}
        for (int d=0;d<4;d++){tracks[ANIM_WALK][d].fps=8.0f;    tracks[ANIM_WALK][d].load(basePath+"/walk_"+dn[d],8,fw,fh);}
        for (int d=0;d<4;d++){tracks[ANIM_ATTACK][d].fps=12.0f; tracks[ANIM_ATTACK][d].load(basePath+"/attack_"+dn[d],8,fw,fh);}
        for (int d=0;d<4;d++){tracks[ANIM_DEATH][d].fps=6.0f;   tracks[ANIM_DEATH][d].load(basePath+"/death",8,fw,fh);}
        loaded = false;
        for (int s=0;s<4&&!loaded;s++)
            for (int d=0;d<4&&!loaded;d++)
                if (tracks[s][d].loaded) loaded = true;
        TraceLog(LOG_INFO, "[ANIM] load done: loaded=%d", loaded);
    }

    void unload() {
        for (int s=0;s<4;s++) for (int d=0;d<4;d++) tracks[s][d].unload();
        loaded = false;
    }

    void setState(AnimState ns, AnimDir nd) {
        if (ns == currentState && nd == currentDir) return;
        if (currentState != ANIM_DEATH) timer = 0.0f;
        currentState = ns; currentDir = nd;
    }

    void update(float dt) {
        if (currentState == ANIM_DEATH) {
            if (!tracks[ANIM_DEATH][0].isFinished(timer)) timer += dt;
            return;
        }
        timer += dt;
    }

    bool isDeathFinished() const {
        return currentState == ANIM_DEATH && tracks[ANIM_DEATH][0].isFinished(timer);
    }

    void draw(ScreenCoords pos, Color tint = WHITE) const {
        if (!loaded) return;
        const AnimTrack& t = tracks[currentState][currentDir];
        bool loop = (currentState != ANIM_DEATH);
        if (t.loaded) { t.draw(timer, loop, pos, tint); return; }
        const AnimTrack& idle = tracks[ANIM_IDLE][0];
        if (idle.loaded) idle.draw(0.0f, true, pos, tint);
    }

    static AnimDir dirFromMovement(float dx, float dy) {
        bool down=(dy>=0), right=(dx>=0);
        if (down&&!right)  return DIR_FRONT_LEFT;
        if (down&&right)   return DIR_FRONT_RIGHT;
        if (!down&&!right) return DIR_BACK_LEFT;
        return DIR_BACK_RIGHT;
    }
};