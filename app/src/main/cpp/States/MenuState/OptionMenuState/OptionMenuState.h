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
    void render(SDL_Renderer* renderer)override;
    void update(float dt)override;
    void handleEvents(SDL_Event& event)override;

private:
    Button m_audioButton{200.00f,200.00f,200.00f,100.00f};
    Button m_controlButton{200.00f,300.00f,200.00f,100.00f};
    Button m_backButton{50.00f,600.00f,200.00f,100.00f};

    SDL_IOStream* m_fontFile = SDL_IOFromFile("Fonts/Pixel Game.otf","rb");
    SDL_IOStream* m_fontShadowFile = SDL_IOFromFile("Fonts/Pixel Game Extrude.otf","rb");
    TTF_Font* m_font = nullptr;
    TTF_Font* m_fontShadow = nullptr;
    SDL_Texture* m_fontTexture = nullptr;
    SDL_Texture* m_fontShadowTexture = nullptr;

    SDL_IOStream* m_backGroundSprite = SDL_IOFromFile("misc/bg.png","rb");
    SDL_Texture* m_backGroundTexture = nullptr;
    SDL_Renderer* m_renderer= nullptr;
};



