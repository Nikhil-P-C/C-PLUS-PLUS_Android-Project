
#include "States/MenuState/MenuState.h"
#include "GameState.h"

//
// Created by LENOVO on 27-04-2026.
//
void GameState::render(SDL_Renderer* renderer)  {
    SDL_FRect backgroundDst{0, 0, 1600, 720};
    SDL_RenderTexture(renderer, m_backGround, nullptr, &backgroundDst);


    SDL_FRect dst = {m_player.x,m_player.y,m_player.w,m_player.h};
    m_currentFrame = m_currentFrame%16;
    m_currentFrame++;
    SDL_FRect src = {(float) (0 + (SPRITE_WIDTH * m_currentFrame)), 0, SPRITE_WIDTH, SPRITE_HEIGHT};
//    SDL_FRect src = {24, 0, SPRITE_WIDTH, SPRITE_HEIGHT};
    SDL_RenderTexture(renderer, m_playerTexture, &src, &dst);
}
void GameState::update(float dt){
}
void GameState::handleEvents(SDL_Event& event) {

    if(event.key.key == SDLK_AC_BACK){
        LOGI("game state transitions to menu state");
        Engine::Get().changeState(std::make_unique<MenuState>(m_renderer));
    }
}




