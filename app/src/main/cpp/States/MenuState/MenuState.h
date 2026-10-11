//
// Created by LENOVO on 11-10-2026.
//
// Silksong-style main menu: black screen, a breathing coloured core of light, floating motes,
// plain text entries with a thin ornament on the selected one.
//
// The shared look (core, motes, sparks, text drawing) lives in MenuKit; motes are the in-game
// AmbientParticles system, selection sparks are the in-game ParticleSystem.
//
#pragma once
#include <array>
#include "State/State.h"
#include "utils/utils.h"
#include "States/MenuState/MenuKit.h"

// Order = order on screen (top -> bottom)
enum class NewMenuButton{
    PLAY   = 0,
    OPTION = 1,
    EDIT   = 2,
    QUIT   = 3
};

class MenuState: public State{
public:
    // intro=false when coming back from a sub menu: no intro jingle, short fade-in, quick reveal
    explicit MenuState(SDL_Renderer* renderer, bool intro = true);
    ~MenuState()override;
    void render(SDL_Renderer* renderer)override;
    void update(float dt)override;
    bool handleEvents(SDL_Event& event)override;
private:
    struct MenuItem{
        MenuLabel label;
        float cx = 800, cy = 0;  // centre on the 1600x720 logical screen
        float anim = 0;          // 0 = idle, 1 = selected (eased)
    };

    void activateSelection();
    void setSelected(int index,bool playSound);
    void moveSelection(int dir);
    int  hitTest(float x,float y) const;
    float revealOf(int index) const;

    SDL_Renderer* m_renderer = nullptr;
    MenuBackdrop  m_backdrop;
    MenuFont      m_font;
    MenuLabel     m_title;

    std::array<MenuItem,4> m_items;
    int  m_selected = 0;

    // flow state
    bool  m_intro         = true;
    bool  m_transitioning = false;
    bool  m_quitting      = false;
    float m_quitTimer     = 0.0f;

    // held dpad / left stick -> repeated up/down (InputUtils)
    InputUtils::MenuNavRepeater m_nav;
};
