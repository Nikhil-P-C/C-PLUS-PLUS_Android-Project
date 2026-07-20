//
// Created by LENOVO on 04-07-2026.
//
#pragma once
#include "State/State.h"
#include "SDL3/SDL.h"
#include "utils/utils.h"
#include "SDL3_ttf/SDL_ttf.h"
enum ScaleType{
    MASTER =0,
    MUSIC =1,
    SFX =2,
};
class AudioMenuState:public State{
public:
    void render(SDL_Renderer* renderer)override;
    void update(float dt)override;
    bool handleEvents(SDL_Event& event)override;
    AudioMenuState(SDL_Renderer* renderer);
    ~AudioMenuState()override;
private:
    ScaleType m_scaleType =MASTER;
    float m_minimumSliderX =930.00f,m_maximumSliderX =1357.50f;
    Button m_masterSlider{m_minimumSliderX + GameData::getInstance().getAudioScale() *(m_maximumSliderX-m_minimumSliderX),
                    175.00f,25.00f,40.00f};
    Button m_masterSlidebar{930.00f,182.50f,450.00f,25.00f};

    Button m_musicSlider{m_minimumSliderX + GameData::getInstance().getMusicAudioScale() *(m_maximumSliderX-m_minimumSliderX),
                        175.00f+150.00f+20.00f,25.00f,40.00f};
    Button m_musicSlidebar{930.00f, m_masterSlidebar.y+150.00f+20.00f, 450.00f, 25.00f};

    Button m_sfxSlider{m_minimumSliderX + GameData::getInstance().getSFXAudioScale() *(m_maximumSliderX-m_minimumSliderX),
                       175.00f+(150.00f+20.00f)*2,25.00f,40.00f};
    Button m_sfxSlidebar{930.00f, m_masterSlidebar.y+(150.00f+20.00f)*2, 450.00f, 25.00f};

    SDL_FingerID m_sliderFingerID =0;
    SDL_IOStream* m_menuSpriteFile = SDL_IOFromFile("menu/optionMenu.png","rb");
    SDL_IOStream* m_optionBlockFile= SDL_IOFromFile("menu/menutile.png","rb");
    SDL_IOStream* m_slidebarFile   = SDL_IOFromFile("menu/slidebar.png","rb");
    SDL_IOStream* m_sliderFile     = SDL_IOFromFile("menu/slider.png","rb");
    SDL_IOStream* m_fontfile       = SDL_IOFromFile("Fonts/Pixel Game.otf","rb");

    TTF_Font* m_font               = nullptr;

    SDL_Texture* m_masterFontTexture  = nullptr;
    SDL_Texture* m_musicFontTexture   = nullptr;
    SDL_Texture* m_sfxFontTexture     = nullptr;
    SDL_Texture* m_menuTexture        = nullptr;
    SDL_Texture* m_optionBlockTexture = nullptr;
    SDL_Texture* m_slidebarTexture    = nullptr;
    SDL_Texture* m_sliderTexture      = nullptr;

    SDL_Renderer* m_renderer = nullptr;
};
