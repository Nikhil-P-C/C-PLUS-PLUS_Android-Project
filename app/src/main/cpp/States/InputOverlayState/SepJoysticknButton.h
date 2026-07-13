//
// Created by LENOVO on 13-07-2026.
//
#pragma once
#include <SDL3/SDL.h>
#include "State/state.h"
#include "utils/utils.h"

class SepJoysticknButton :public State {
public:
    SepJoysticknButton(SDL_Renderer* renderer);
    ~SepJoysticknButton() override;
    void render(SDL_Renderer* renderer) override;

    void update(float dt) override;

    bool handleEvents(SDL_Event& event) override;

private:
    Button m_JumpButton{1400,400,100,100};
    Button m_joystick{0+ 200, 720 -400,300,300};
    Button m_joystickHandle{ m_touchX -150.00f/ 2.00f,
                             m_touchY - 150.00f / 2.00f,
                             150,150};

    float m_touchX =m_joystick.x + m_joystick.w/2,m_touchY =m_joystick.y + m_joystick.h/2;

    bool m_joystickFingerActive =false;
    bool m_jumpFingerActive = false;

    SDL_FingerID m_joystickFingerID =0;
    SDL_FingerID m_jumpFingerID =0;

    SDL_IOStream* m_jumpButtonFile = SDL_IOFromFile("button/jump_button.png","rb");
    SDL_IOStream *m_joystickFile = SDL_IOFromFile("joystick/Joystick.png","rb");
    SDL_IOStream *m_joystickHandleFile =SDL_IOFromFile("joystick/SmallHandleFilledGrey.png","rb");

    SDL_Texture* m_jumpButtonTexture = nullptr;
    SDL_Texture *m_joystickTexture = nullptr;
    SDL_Texture *m_joystickHandleTexture = nullptr;
};


