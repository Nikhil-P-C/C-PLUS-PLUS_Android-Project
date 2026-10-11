//
// Created by LENOVO on 17-04-2026.
//
// Pause screen in the menu look (MenuKit): the game stays visible underneath, dimmed, with the
// breathing light, motes, a PAUSED title and RESUME / EXIT entries.
//   keyboard / gamepad: up-down + confirm, back = resume      mouse / touch: tap EXIT, tap anywhere else to resume
//
#pragma once
#include "utils/utils.h"
#include "SDL3/SDL.h"
#include "State/State.h"
#include "States/GameState/GameState.h"
#include "States/MenuState/MenuKit.h"

#define LOG_TAG "PauseState"

class PauseState : public State{
public:
    PauseState(SDL_Renderer* renderer, GameState* gameState);
    void render(SDL_Renderer* renderer)override;
    void update(float dt)override;
    bool handleEvents(SDL_Event& event)override;
    ~PauseState() override;
private:
    enum Entry { ENTRY_RESUME = 0, ENTRY_EXIT = 1, ENTRY_COUNT = 2 };

    void resume();
    void exitToMenu();
    void activate(int entry);

    SDL_Renderer* m_renderer = nullptr;
    GameState* m_gameState = nullptr;

    MenuBackdrop m_backdrop;
    MenuFont     m_font;
    MenuLabel    m_title;
    MenuLabel    m_entry[ENTRY_COUNT];
    MenuRows     m_rows;
    InputUtils::MenuNavRepeater m_nav;

    bool m_transitioning = false;
};
