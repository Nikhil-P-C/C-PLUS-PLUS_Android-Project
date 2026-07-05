//
// Created by LENOVO on 01-07-2026.
//
#pragma once

#include "State/State.h"
#include "utils/utils.h"
class ControlMenuState :public State{
public:
    void render(SDL_Renderer* renderer)override;
    void update(float dt)override;
    bool handleEvents(SDL_Event& event)override;

    ControlMenuState(SDL_Renderer* renderer);
private:
    SDL_IOStream* m_menuSpriteFile = SDL_IOFromFile("optionMenu.png","rb");
    SDL_Texture* m_menuTexture = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    ControlType m_controlType = ControlType::JOYSTICK;
    Button m_joystickButton{400.00f,360.00f,200.00f,100.00f};
    Button m_buttonButton{600.00f,360.00f,200.00f,100.00f};
};
