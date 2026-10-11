//
// Created by LENOVO on 01-07-2026.
//
// Control page of the option screen: pick the on-screen / physical control scheme (radio list).
//
#pragma once

#include "State/State.h"
#include "utils/utils.h"
#include "States/MenuState/MenuKit.h"

class ControlMenuState :public State{
public:
    void render(SDL_Renderer* renderer)override;
    void update(float dt)override;
    bool handleEvents(SDL_Event& event)override;

    explicit ControlMenuState(SDL_Renderer* renderer);
    ~ControlMenuState()override;
private:
    static constexpr int OPTION_COUNT = 5;

    void choose(int index);
    void perform(InputUtils::MenuAction action);
    int  currentIndex() const;

    SDL_Renderer* m_renderer = nullptr;
    MenuPainter   m_painter;
    MenuFont      m_font;
    MenuLabel     m_labels[OPTION_COUNT];
    MenuRows      m_rows;
    float         m_pick[OPTION_COUNT] = {};   // 0..1 eased "this one is the active scheme"
    float         m_pop[OPTION_COUNT]  = {};   // 1 -> 0 flash right after choosing
    InputUtils::MenuNavRepeater m_nav;
};
