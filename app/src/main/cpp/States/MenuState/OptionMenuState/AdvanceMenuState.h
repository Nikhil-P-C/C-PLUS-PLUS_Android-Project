//
// Created by LENOVO on 10-08-2026.
//
#pragma once
#include "State/State.h"
#include "utils/utils.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
class AdvanceMenuState:public State{
public:
    void render(SDL_Renderer* renderer)override;
    void update(float dt)override;
    bool handleEvents(SDL_Event& event)override;

    AdvanceMenuState(SDL_Renderer* renderer);
    ~AdvanceMenuState()override;

private:
    SDL_Renderer* m_renderer = nullptr;

    SDL_IOStream* m_fontfile = SDL_IOFromFile("Fonts/Pixel Game.otf","rb");

    TTF_Font* m_font = nullptr;
    SDL_Texture* m_menuTexture =nullptr;
    SDL_Texture* m_optionBlockTexture = nullptr;
    SDL_Texture* m_radioButtonTexture = nullptr;
    SDL_Texture* m_debugFontTexture = nullptr;


    Button m_debugButton{400.00f,500.00f,1000.00f,100.00f};

};