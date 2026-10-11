//
// Created by LENOVO on 01-07-2026.
//
// Option screen: black backdrop with the breathing light (MenuKit), a tab row (AUDIO / CONTROL / ADVANCE)
// and BACK. The page under the active tab is an overlay state (AudioMenuState / ControlMenuState /
// AdvanceMenuState) drawn on top, same as before.
//
#pragma once

#include <memory>
#include "SDL3/SDL.h"
#include "State/State.h"
#include "utils/utils.h"
#include "States/MenuState/MenuKit.h"

class OptionMenuState : public State {
public:
    explicit OptionMenuState(SDL_Renderer* renderer);
    ~OptionMenuState()override;
    void render(SDL_Renderer* renderer)override;
    void update(float dt)override;
    bool handleEvents(SDL_Event& event)override;

private:
    enum Tab { TAB_AUDIO = 0, TAB_CONTROL = 1, TAB_ADVANCE = 2, TAB_COUNT = 3 };

    std::unique_ptr<State> makePage(int tab) const;
    void selectTab(int tab);
    void leave();
    int  tabAt(float x,float y) const;
    bool overBack(float x,float y) const;

    SDL_Renderer* m_renderer = nullptr;
    MenuBackdrop  m_backdrop;
    MenuFont      m_font;
    MenuLabel     m_tabLabel[TAB_COUNT];
    MenuLabel     m_backLabel;

    int   m_tab = TAB_AUDIO;
    int   m_hoverTab = -1;
    bool  m_hoverBack = false;
    float m_tabAnim[TAB_COUNT] = {1.0f,0.0f,0.0f};
    float m_backAnim = 0.0f;
    bool  m_leaving = false;
};
