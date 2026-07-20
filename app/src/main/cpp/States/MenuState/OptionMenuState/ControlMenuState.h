//
// Created by LENOVO on 01-07-2026.
//
#pragma once

#include "State/State.h"
#include "utils/utils.h"
#include "SDL3_ttf/SDL_ttf.h"

class ControlMenuState :public State{
public:
    void render(SDL_Renderer* renderer)override;
    void update(float dt)override;
    bool handleEvents(SDL_Event& event)override;

    ControlMenuState(SDL_Renderer* renderer);
    ~ControlMenuState()override;
private:
    SDL_IOStream* m_menuSpriteFile = SDL_IOFromFile("menu/optionMenu.png","rb");
    SDL_IOStream* m_optionBlockFile= SDL_IOFromFile("menu/menutile.png","rb");
    SDL_IOStream* m_radioButtonfile = SDL_IOFromFile("menu/radioButton.png","rb");
    SDL_IOStream* m_fontfile = SDL_IOFromFile("Fonts/Pixel Game.otf","rb");

    TTF_Font* m_font = nullptr;

    SDL_Texture* m_optionBlockTexture = nullptr;
    SDL_Texture* m_radioButtonTexture = nullptr;
    SDL_Texture* m_menuTexture = nullptr;
    SDL_Texture* m_joystickFontTexture = nullptr;
    SDL_Texture* m_buttonsFontTexture = nullptr;
    SDL_Texture* m_joystickWButtonsFontTexture = nullptr;
    SDL_Texture* m_debugFontTexture = nullptr;

    SDL_Renderer* m_renderer = nullptr;

    ControlType m_controlType =GameData::getInstance().getControlType();

    Button m_joystickButton{400.00f,100.00f,1000.00f,100.00f};
    Button m_buttonButton{400.00f,200.00f,1000.00f,100.00f};
    Button m_joystickWButtons{400.00f,300.00f,1000.00f,100.00f};
    Button m_debugButton{400.00f,500.00f,1000.00f,100.00f};
};
