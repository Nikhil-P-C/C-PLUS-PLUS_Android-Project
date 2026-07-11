//
// Created by LENOVO on 04-07-2026.
//
#pragma once
#include "State/State.h"
#include "SDL3/SDL.h"
#include "utils/utils.h"
#include "SDL3_ttf/SDL_ttf.h"

class AudioMenuState:public State{
public:
    void render(SDL_Renderer* renderer)override;
    void update(float dt)override;
    bool handleEvents(SDL_Event& event)override;
    AudioMenuState(SDL_Renderer* renderer);
    ~AudioMenuState()override;
private:
    float m_minimumSliderX =930.00f,m_maximumSliderX =1357.50f;
    Button m_slider{m_minimumSliderX + GameData::getInstance().getAudioScale() *(m_maximumSliderX-m_minimumSliderX),
                    175.00f,25.00f,40.00f};
    Button m_slidebar{930.00f,182.50f,450.00f,25.00f};
    SDL_FingerID m_sliderFingerID =0;
    SDL_IOStream* m_menuSpriteFile = SDL_IOFromFile("menu/optionMenu.png","rb");
    SDL_IOStream* m_optionBlockFile= SDL_IOFromFile("menu/menutile.png","rb");
    SDL_IOStream* m_slidebarFile   = SDL_IOFromFile("menu/slidebar.png","rb");
    SDL_IOStream* m_sliderFile     = SDL_IOFromFile("menu/slider.png","rb");

    SDL_IOStream* m_fontfile       = SDL_IOFromFile("Fonts/Pixel Game.otf","rb");
    SDL_IOStream* m_fontShadowfile = SDL_IOFromFile("Fonts/Pixel Game Extrude.otf","rb");

    TTF_Font* m_font               = nullptr;
    TTF_Font* m_fontShadow         = nullptr;

    SDL_Texture* m_fontTexture       = nullptr;
    SDL_Texture* m_fontShadowTexture = nullptr;
    SDL_Texture* m_menuTexture        = nullptr;
    SDL_Texture* m_optionBlockTexture = nullptr;
    SDL_Texture* m_slidebarTexture    = nullptr;
    SDL_Texture* m_sliderTexture      = nullptr;

    SDL_Renderer* m_renderer = nullptr;
};
