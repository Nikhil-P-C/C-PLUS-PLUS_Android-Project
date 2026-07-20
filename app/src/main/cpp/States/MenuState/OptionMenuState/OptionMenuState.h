//
// Created by LENOVO on 01-07-2026.
//

#pragma once

#include "SDL3/SDL.h"
#include "State/State.h"
#include "utils/utils.h"
#include "SDL3_ttf/SDL_ttf.h"

class OptionMenuState : public State {
public:
    OptionMenuState(SDL_Renderer* renderer);
    ~OptionMenuState()override;
    void render(SDL_Renderer* renderer)override;
    void update(float dt)override;
    bool handleEvents(SDL_Event& event)override;

private:
    Button m_audioButton{100.00f,120.00f,180.00f,155.00f};
    Button m_controlButton{100.00f,275.00f,180.00f,155.00f};
    Button m_backButton{50.00f,600.00f,200.00f,100.00f};

    SDL_IOStream* m_backGroundSprite = SDL_IOFromFile("misc/bg.png","rb");


    SDL_Texture* m_backGroundTexture = nullptr;

    SDL_Renderer* m_renderer= nullptr;
};



