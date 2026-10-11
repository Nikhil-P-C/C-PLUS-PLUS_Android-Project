//
// Created by LENOVO on 10-08-2026.
//
// Advance page of the option screen: debug overlay on / off.
//
#pragma once
#include "State/State.h"
#include "utils/utils.h"
#include <SDL3/SDL.h>
#include "States/MenuState/MenuKit.h"

class AdvanceMenuState:public State{
public:
    void render(SDL_Renderer* renderer)override;
    void update(float dt)override;
    bool handleEvents(SDL_Event& event)override;

    explicit AdvanceMenuState(SDL_Renderer* renderer);
    ~AdvanceMenuState()override;

private:
    void setDebug(bool enabled);
    void perform(InputUtils::MenuAction action);

    SDL_Renderer* m_renderer = nullptr;
    MenuPainter   m_painter;
    MenuFont      m_font;
    MenuLabel     m_debugLabel, m_onLabel, m_offLabel;
    MenuRows      m_rows;
    float         m_on = 0.0f;                 // eased 0 = off, 1 = on
    InputUtils::MenuNavRepeater m_nav;
};
