//
// Created by LENOVO on 04-05-2026.
//
#pragma once
#include <SDL3_ttf/SDL_ttf.h>
#include "State/state.h"
#include "utils/utils.h"
class MenuState:public State{
public:
    MenuState(SDL_Renderer* renderer);
    ~MenuState()override;
    void render(SDL_Renderer* renderer)override;
    void update(float dt)override;
    void handleEvents(SDL_Event& event)override;
private:
    Button m_playButton{150,100,350,150};
    Button m_settingsButton{150,270,350,150};
    Button m_quitButton{150,440,350,150};
    Button m_customizeButton{1100,350,350,150};

    SDL_IOStream* m_backGroundSprite = SDL_IOFromFile("misc/bg.png","rb");
    SDL_IOStream* m_buttonSprite = SDL_IOFromFile("button/buttons.png","rb");
    SDL_IOStream* m_fontfile = SDL_IOFromFile("Fonts/Pixel Game.otf","rb");
    SDL_IOStream* m_fontShadowfile = SDL_IOFromFile("Fonts/Pixel Game Extrude.otf","rb");

    SDL_Texture* m_background = nullptr;
    SDL_Texture* m_playButtonTexture = nullptr;
    SDL_Texture* m_settingsButtonTexture = nullptr;
    SDL_Texture* m_quitButtonTexture = nullptr;
    SDL_Texture* m_customizeButtonTexture = nullptr;
    SDL_Texture* m_playText = nullptr;
    SDL_Texture* m_playTextShadow = nullptr;

    TTF_Font* m_font = nullptr;
    TTF_Font* m_fontShadow = nullptr;

    SDL_Renderer* m_renderer = nullptr;
};
