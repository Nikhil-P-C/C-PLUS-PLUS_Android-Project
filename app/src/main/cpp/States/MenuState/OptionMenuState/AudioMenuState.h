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
    bool handleEvents(SDL_Event& event)override;
    AudioMenuState(SDL_Renderer* renderer);
private:
    SDL_IOStream* m_menuSpriteFile = SDL_IOFromFile("optionMenu.png","rb");
    SDL_Texture* m_menuTexture = nullptr;
    SDL_Renderer* m_renderer = nullptr;
};
