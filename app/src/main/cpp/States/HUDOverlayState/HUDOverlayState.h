//
// Created by LENOVO on 03-08-2026.
//
#pragma once
#include "State/State.h"
#include "States/GameState/GameState.h"
#include <SDL3/SDL.h>
enum class HeartAniType{
    IDLE =0,
    HURT,
    LOST
};

struct Heart {
    Heart(SDL_FRect rect):heartRect(rect){}
    SDL_FRect heartRect{0};
    HeartAniType heartAniType =HeartAniType::IDLE;
    Animation animation{0,0};
    int currentFrame =animation.startIndex;
    unsigned int aniLastTime =0;
    bool aniDone;
};
class HUDOverlayState:public State{
public:
    void render(SDL_Renderer* renderer) override;
    void update(float dt) override;
    bool handleEvents(SDL_Event& event) override;

    void updateAnimation();

    HUDOverlayState(SDL_Renderer* renderer);
    ~HUDOverlayState() override;

private:
    SDL_Renderer* m_renderer = nullptr;
    std::vector<Heart> m_hearts;
    int m_lastHeart =5;
    int m_prevHealth=5;
    int m_aniFrameDelay = 50;
    int m_spriteHeight =29, m_spriteWidth=30;

};