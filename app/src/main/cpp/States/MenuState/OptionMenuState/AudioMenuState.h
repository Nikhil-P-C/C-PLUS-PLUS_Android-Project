//
// Created by LENOVO on 04-07-2026.
//
// Audio page of the option screen: MASTER / MUSIC / SFX sliders.
//   keyboard / gamepad: up-down = pick a slider, left-right = change it
//   mouse / touch:      press and drag on a slider
//
#pragma once
#include "State/State.h"
#include "SDL3/SDL.h"
#include "utils/utils.h"
#include "States/MenuState/MenuKit.h"

class AudioMenuState:public State{
public:
    void render(SDL_Renderer* renderer)override;
    void update(float dt)override;
    bool handleEvents(SDL_Event& event)override;
    explicit AudioMenuState(SDL_Renderer* renderer);
    ~AudioMenuState()override;
private:
    enum Slider { MASTER_ROW = 0, MUSIC_ROW = 1, SFX_ROW = 2, ROW_COUNT = 3 };

    float value(int row) const;
    void  setValue(int row,float v);
    void  refreshValueLabel(int row);
    void  dragTo(float pointerX);
    void  step(int row,float delta);
    void  perform(InputUtils::MenuAction action);

    SDL_Renderer* m_renderer = nullptr;
    MenuPainter   m_painter;
    MenuFont      m_font;
    MenuLabel     m_names[ROW_COUNT];
    MenuLabel     m_values[ROW_COUNT];
    int           m_shownPercent[ROW_COUNT] = {-1,-1,-1};
    MenuRows      m_rows;
    InputUtils::MenuNavRepeater m_nav;

    // pointer drag
    bool        m_dragging = false;
    int         m_dragRow  = -1;
    bool        m_dragIsTouch = false;
    SDL_FingerID m_dragFinger = 0;
};
