//
// Created by LENOVO on 24-07-2026.
//
#pragma once
#include <vector>
#include <SDL3/SDL.h>
#include "engine/Engine.h"
enum class TrapType{
    FALLING_PLATFORM=0,
    FAN,
    FIRE,
    MOVING_PLATFORM_BROWN,
    MOVING_PLATFORM_GREY,
    ROCK_HEAD,
    SAW,
    SPIKE_HEAD,
    SPIKE_BALL,
    SPIKES,
    TRAMPOLINE
};
enum class TrapStatus{
    IDLE=0,
    HIT,
    ON,
    OFF,
    TRIGGERED,
};
struct Trap{
    TrapType type;
    TrapStatus status;
    float x ,y;
    float startPath,endPath;
    int aniStartFrame =0;
    int aniEndFrame;
    bool aniDone;
};

class TrapBuilder{
public:
    std::vector<Trap>& getTraps();
    void init(const std::vector<Trap>& traps);
    void render(SDL_Renderer* renderer);
    void update(float dt);
    int onCollision(float x,float y,float w,float h);
    constexpr bool trapHasPath(TrapType type);
    constexpr bool trapHasHit(TrapType type);

private:
    std::vector<Trap> m_traps;

    SDL_Texture* m_fallingPlatformOnTexture =nullptr;
    SDL_Texture* m_fallingPlatformOffTexture =nullptr;

    SDL_Texture* m_fanOnTexture =nullptr;
    SDL_Texture* m_fanOffTexture =nullptr;

    SDL_Texture* m_fireHitTexture =nullptr;
    SDL_Texture* m_fireOffTexture =nullptr;
    SDL_Texture* m_fireOnTexture =nullptr;

    SDL_Texture* m_platformBrownOffTexture = nullptr;
    SDL_Texture* m_platformBrownOnTexture = nullptr;
    SDL_Texture* m_platformGreyOffTexture = nullptr;
    SDL_Texture* m_platformGreyOnTexture = nullptr;
    SDL_Texture* m_platformChainTexture =nullptr;

    SDL_Texture* m_rockHeadBlinkTexture = nullptr;
    SDL_Texture* m_rockHeadHitBottomTexture = nullptr;
    SDL_Texture* m_rockHeadHitTopTexture = nullptr;
    SDL_Texture* m_rockHeadHitLeftTexture = nullptr;
    SDL_Texture* m_rockHeadHitRightTexture = nullptr;

    SDL_Texture* m_spikeHeadHitBottomTexture = nullptr;
    SDL_Texture* m_spikeHeadBlinkTexture = nullptr;
    SDL_Texture* m_spikeHeadHitTopTexture = nullptr;
    SDL_Texture* m_spikeHeadHitLeftTexture = nullptr;
    SDL_Texture* m_spikeHeadHitRightTexture = nullptr;

    SDL_Texture* m_spikeBallTexture =nullptr;
    SDL_Texture* m_spikeBallChainTexture = nullptr;

    SDL_Texture* m_sawOffTexture = nullptr;
    SDL_Texture* m_sawOnTexture = nullptr;
    SDL_Texture* m_sawChainTexture = nullptr;

    SDL_Texture* m_spikesTexture = nullptr;

    SDL_Texture* m_trampolineIdleTexture = nullptr;
    SDL_Texture* m_trampolineTriggerTexture = nullptr;

};