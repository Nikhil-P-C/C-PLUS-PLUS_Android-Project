//
// Created by LENOVO on 12-05-2026.
//
#pragma once
#include <vector>
#include "Engine/Engine.h"
#include "State/State.h"
#include "utils/button.h"

class ButtonOverlay: public State {
public:
    void render(SDL_Renderer* renderer) override;

    void update(float dt) override;

    bool handleEvents(SDL_Event& event) override;
    ButtonOverlay(SDL_Renderer* renderer);
    ~ButtonOverlay()override;
private:
    Button m_JumpButton{1400,400,100,100};
    Button m_LeftButton{200 ,400,100,100};
    Button m_RightButton{300,400,100,100};
    Button m_CrouchButton{1400,600,100,100};

    SDL_Texture* m_jumpButtonTexture = nullptr;
    SDL_Texture* m_leftButtonTexture = nullptr;
    SDL_Texture* m_rightButtonTexture = nullptr;

    SDL_FingerID m_jumpFingerID=0;
    SDL_FingerID m_dFingerID=0;
    bool m_jumpFingerActive = false;
    bool m_dFingerActive = false;
    float m_TouchX =0;
    float m_TouchY =0;
};

