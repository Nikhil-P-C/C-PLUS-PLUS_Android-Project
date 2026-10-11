//
// Created by LENOVO on 23-07-2026.
//
#pragma once
#include "SDL3/SDL.h"
#include <array>
#include "engine/AssetManager.h"

// PUFF   = the original soft square (dust, heal sparkle, bursts) - top-left anchored, shrinks with life
// STREAK = hit spark: stretched along its velocity, additive, x/y is its CENTRE
// FLASH  = impact flash: grows while fading, additive, x/y is its CENTRE
enum class ParticleKind : uint8_t { PUFF, STREAK, FLASH, SCRATCH, RING };
// RING = expanding shock ring (vector, additive): size grows from 0 to baseSize while it fades
// SCRATCH = long thin sharp line at a fixed angle (the "claw mark" across a hit enemy)

struct Particle{
    bool isAlive =false;
    ParticleKind kind = ParticleKind::PUFF;   // reset to PUFF/0/0 when a particle dies (see update())
    float angle = 0.0f;                       // SCRATCH only (radians)
    float drag = 0.0f;                        // 1/s velocity decay (exp)
    float gravity = 0.0f;                     // px/s^2

    float x=0,y=0;
    float vX=0,vY=0;
    float size=70.00f;
    float baseSize =70.00f;
    float life=0.04f;
    float maxLife =0.4;
    uint8_t colorR=255,colorG=255,colorB=255; // white = existing dust look; tinted for effects like healing
};
// One-shot sprite flipbook (hit effects drawn as art). Frames are square, laid out horizontally.
struct SpriteFx{
    bool alive = false;
    SDL_Texture* tex = nullptr;
    int frames = 1;
    int frameW = 0, frameH = 0;   // source px; vertical strip: frame i is at y = i*frameH
    float x = 0, y = 0;       // world centre
    float angleDeg = 0;
    float scale = 5.0f;       // art px -> game px (matches P_scale)
    float age = 0, frameDur = 0.035f;
    bool flipX = false;
};

class ParticleSystem{
public:
    // Plays a flipbook from the AssetManager. Returns false if the asset isn't there (caller falls back to generated sparks).
    bool emitSpriteFx(TextureType type, float x, float y, float dirX, float angleDeg, float frameDur, float scale = 5.0f);
    void render(SDL_Renderer* renderer);
    void update(float dt);
    // true = x/y are screen coordinates and the game camera is ignored (menus). Default false (world space).
    void setScreenSpace(bool screenSpace) { m_screenSpace = screenSpace; }

    void emitJumpDust(float feetX,float feetY);
    void emitLeftDust(float feetX,float feetY);
    void emitRightDust(float feetX ,float feetY);
    void emitLandDust(float leftFeetX, float leftFeetY,float rightFeetX,float rightFeetY);
    void emitOneJumpParticle(float feetX, float feetY);
    void emitHealSparkle(float x, float y);
    // Spark burst centred on (x,y) flying mostly toward dirX (+1 right / -1 left), plus one impact flash.
    // spread = half-angle in radians; flashSize 0 = no flash.
    void emitSparkBurst(float x, float y, float dirX, int count, float spread,
                        float speedMin, float speedMax, float lengthMin, float lengthMax,
                        uint8_t r, uint8_t g, uint8_t b, float flashSize);
    // player sword hit on an enemy: bigger, brighter and warmer as the combo grows
    // scale > 1 = bigger version of the same effect (used for the player-hurt impact)
    void emitHitSpark(float x, float y, float dirX, int combo, float scale = 1.0f);
    // sword/beam scraping a wall: few, pale-blue, short
    // sword/beam hits a wall/ground/platform: warm yellow-orange sparks bouncing back off the surface
    void emitWallSpark(float x, float y, float dirX);
    // sword hits a block/trap/object: cold white-blue metallic sparks (distinct from wall + enemy)
    void emitObjectSpark(float x, float y, float dirX);
    // long thin white scratch line across the hit point (enemy hits)
    void emitScratch(float x, float y, float angle, float length);
    // expanding shock ring, `diameter` = final size
    void emitRing(float x, float y, float diameter, float life, uint8_t r, uint8_t g, uint8_t b);
    // everything for "the player got hit": two shock rings, fast radial streaks, falling debris shards, floating embers
    void emitDamageBurst(float x, float y, float dirX);
    // radial burst centred on (x,y): used for death puffs. Speeds in px/s, size in px.
    void emitBurst(float x, float y, int count, float speedMin, float speedMax,
                   float lifeMin, float lifeMax, float size,
                   uint8_t r, uint8_t g, uint8_t b);
    void emitParticleWProps(int count,int rangeX1,int rangeX2,int rangeY1, int rangeY2,
                            int velocityX1,int velocityX2 , int velocityY1,int velocityY2,
                            float minLife, float maxLife,int dirX,int dirY,float size);
    void emitDust(float pointX, float pointY, int velocityX, int velocityY,float life,float size);
    int randomInt(int min, int max);
    float randomFloat(float min, float max);

    ParticleSystem();
private:
    Particle* findFree();
    SDL_Texture* m_particleTexture=nullptr;
    std::array<Particle,256> m_particles;
    std::array<SpriteFx,32> m_spriteFx;
    bool m_screenSpace = false;


};