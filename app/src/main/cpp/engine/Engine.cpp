//
// Created by LENOVO on 30-04-2026.
//
#include "SDL3_ttf/SDL_ttf.h"


#include "State/State.h"
#include "engine/Engine.h"
#include "States/GameState/GameState.h"
#include "States/MenuState/MenuState.h"
void Engine::initEngine(){
    LOGI("initEngine");
    if (SDL_Init(SDL_INIT_VIDEO) == 0)
        LOGI("failed SDL:%s", SDL_GetError());
    if (TTF_Init() == 0)
        LOGI("failed ttf:%s", SDL_GetError());
    m_window = SDL_CreateWindow("Dino", 0, 0, SDL_WINDOW_FULLSCREEN);
    if (!m_window) {
        LOGE("Window creation failed: %s", SDL_GetError());
    }

    m_renderer = SDL_CreateRenderer(m_window, "opengles2");
    if (!m_renderer) {
        LOGE("Renderer creation failed: %s", SDL_GetError());
    }

    const char* name = SDL_GetRendererName(m_renderer);
    LOGI("Renderer: %s", name);
    pushState(std::make_unique<MenuState>(m_renderer));
}
