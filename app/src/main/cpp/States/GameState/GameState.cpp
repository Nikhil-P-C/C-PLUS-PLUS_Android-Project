#include "GameState.h"

//
// Created by LENOVO on 27-04-2026.
//
void GameState::render(SDL_Renderer* renderer)  {
    SDL_RenderClear(renderer);
    SDL_FRect backgroundDst{0, 0, 1600, 720};
    SDL_RenderTexture(renderer, backGround, nullptr, &backgroundDst);


    SDL_FRect dst = {100,200,120, 120};
  //  SDL_FRect src = {(float) (0 + (SPRITE_WIDTH * current_Frame)), 0, SPRITE_WIDTH, SPRITE_HEIGHT};
    SDL_FRect src = {24, 0, SPRITE_WIDTH, SPRITE_HEIGHT};
    SDL_RenderTexture(renderer, playerTexture, &src, &dst);
    SDL_RenderPresent(renderer);
}
void GameState::update(float dt){
}
void GameState::handleEvents(SDL_Event& event) {
    while(SDL_PollEvent(&event)){
        if(event.type == SDL_EVENT_FINGER_DOWN){
            LOGI("touch down");
        }
    }
}




