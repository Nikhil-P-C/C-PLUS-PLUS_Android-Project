//
// Created by LENOVO on 28-04-2026.
//
#include "MenuState.h"
#include "States/GameState/GameState.h"

void MenuState::render(SDL_Renderer *renderer) {

    SDL_FRect backgroundDst = {0, 0, 1600, 720};
    SDL_RenderTexture(m_renderer, backGround, nullptr, &backgroundDst);
}
void MenuState::update(float dt) {
}
void MenuState::handleEvents(SDL_Event &event){

    if(event.type == SDL_EVENT_FINGER_DOWN){
        LOGI("menu starts transitioning to game state");
        Engine::Get().changeState(std::make_unique<GameState>(m_renderer));
        return;
        LOGI("menu state handleEvents end");
    }
}

