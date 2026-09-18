//
// Created by LENOVO on 11-08-2026.
//
#pragma once
#include "State/State.h"
#include <SDL3/SDL.h>
class GamepadOverlay:public State{
public:
    void render(SDL_Renderer* renderer)override;
    void update(float dt)override;
    bool handleEvents(SDL_Event& event)override;

    GamepadOverlay(SDL_Renderer* renderer);
    ~GamepadOverlay()override;
private:
    static const unsigned int MAGIC_HOLD_THRESHOLD_MS = 180;

    SDL_Renderer* m_renderer =nullptr;

    SDL_Gamepad* m_gamepad = nullptr;

    float m_leftStickX =0.0f;
    float m_leftStickY =0.0f;

    uint32_t m_magicPressStartTime = 0;
    bool m_magicBeamStarted = false;
    bool m_magicButtonActive = false;
};
