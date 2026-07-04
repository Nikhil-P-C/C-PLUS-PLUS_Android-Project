//
// Created by LENOVO on 01-07-2026.
//
#pragma once

#include "State/State.h"

class ControlMenuState :public State{
public:
    void render(SDL_Renderer* renderer)override;
    void update(float dt)override;
    void handleEvents(SDL_Event& event)override;
    ControlMenuState(SDL_Renderer* renderer);
private:
    SDL_Renderer* m_renderer = nullptr;
};
