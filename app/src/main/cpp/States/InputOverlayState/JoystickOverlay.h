//
// Created by LENOVO on 12-05-2026.
//
#pragma once
#include <SDL3/SDL.h>
#include "State/State.h"
#include "utils/button.h"
#include "engine/Engine.h"
class JoystickOverlay: public State {
public:
    JoystickOverlay(SDL_Renderer* renderer);
    ~JoystickOverlay() override;
    void render(SDL_Renderer* renderer) override;

    void update(float dt) override;

    bool handleEvents(SDL_Event& event) override;

private:
    Button m_joystick{0+ 200, 720 -400,300,300};
    Button m_joystickHandle{ 350.00f,
                             470.00f,
                             150,150};
    float m_touchX =m_joystick.x + m_joystick.w/2,m_touchY =m_joystick.y + m_joystick.h/2;
    bool m_joystickFingerActive =false;

    SDL_FingerID m_fingerID =0;

    SDL_Texture *m_joystickTexture = nullptr;
    SDL_Texture *m_joystickHandleTexture = nullptr;

};

