//
// Shown after the player's death animation. Same menu look as the pause screen (MenuKit): the frozen
// game stays visible underneath, dimmed, with a YOU DIED title and RESPAWN / EXIT entries.
//   keyboard / gamepad: up-down + confirm        mouse / touch: tap EXIT to leave, tap anywhere else to respawn
// Presses in the first moments are ignored so a frantic jump/attack tap can't skip the screen by accident.
//
#pragma once
#include "utils/utils.h"
#include "SDL3/SDL.h"
#include "State/State.h"
#include "States/GameState/GameState.h"
#include "States/MenuState/MenuKit.h"

#define LOG_TAG "RespawnState"

class RespawnState : public State{
public:
    // atCheckpoint: only changes the little caption under the title (where the player will come back)
    RespawnState(SDL_Renderer* renderer, GameState* gameState, bool atCheckpoint);
    void render(SDL_Renderer* renderer)override;
    void update(float dt)override;
    bool handleEvents(SDL_Event& event)override;
    ~RespawnState() override;
private:
    enum Entry { ENTRY_RESPAWN = 0, ENTRY_EXIT = 1, ENTRY_COUNT = 2 };

    void respawn();
    void exitToMenu();
    void activate(int entry);

    SDL_Renderer* m_renderer = nullptr;
    GameState* m_gameState = nullptr;

    MenuBackdrop m_backdrop;
    MenuFont     m_font;
    MenuLabel    m_title;
    MenuLabel    m_caption;
    MenuLabel    m_entry[ENTRY_COUNT];
    MenuRows     m_rows;
    InputUtils::MenuNavRepeater m_nav;

    bool m_transitioning = false;
};
