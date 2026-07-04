//
// Created by LENOVO on 04-07-2026.
//
#pragma once
#include "State/State.h"
#include "SDL3/SDL.h"
class AudioMenuState:public State{
public:
    void render(SDL_Renderer* renderer)override;
    void update(float dt)override;
    void handleEvents(SDL_Event& event)override;
    AudioMenuState(SDL_Renderer* renderer);
private:
    SDL_Renderer* m_renderer = nullptr;
};
