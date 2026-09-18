//
// Created by LENOVO on 08-08-2026.
//
#pragma once
#include "State/State.h"
#include <SDL3/SDL.h>
class KeyboardOverlay:public State{
public:
    void render(SDL_Renderer* renderer)override;
    void update(float dt)override;
    bool handleEvents(SDL_Event& event)override;

    KeyboardOverlay(SDL_Renderer* renderer);
    ~KeyboardOverlay()override;
private:
    static const unsigned int MAGIC_HOLD_THRESHOLD_MS = 180;



    SDL_Renderer* m_renderer =nullptr;

    uint32_t m_magicPressStartTime = 0;
    bool m_magicBeamStarted = false;
    bool m_magicKeyActive = false;
};
