//
// Created by LENOVO on 17-04-2026.
//
// Title screen in the menu look (MenuKit): black screen, breathing light, floating motes, the game name
// and a pulsing prompt that matches the device:
//   gamepad connected -> PRESS ANY BUTTON,  Android / iOS -> TOUCH ANYWHERE,  otherwise -> PRESS ANY KEY
//
#pragma once
#include "utils/utils.h"
#include <SDL3/SDL.h>
#include "State/State.h"
#include "engine/Engine.h"
#include "States/MenuState/MenuKit.h"

#define LOG_TAG "TitleScreenState"

class TitleScreenState : public State{
public:

    explicit TitleScreenState(SDL_Renderer* renderer);
    ~TitleScreenState() override;

    void render(SDL_Renderer* renderer)override;
    void update(float dt)override;
    bool handleEvents(SDL_Event& event)override;

private:
    enum class PromptKind { KEY, BUTTON, TOUCH };

    PromptKind detectPrompt() const;
    void setPrompt(PromptKind kind);

    SDL_Renderer* m_renderer = nullptr;
    MenuBackdrop  m_backdrop;
    MenuFont      m_font;
    MenuLabel     m_title;
    MenuLabel     m_prompt;
    PromptKind    m_kind = PromptKind::KEY;

    float m_recheckTimer = 0.0f;   // the prompt follows a gamepad being plugged in / removed
    bool  m_leaving = false;
    float m_leaveTimer = 0.0f;
};
