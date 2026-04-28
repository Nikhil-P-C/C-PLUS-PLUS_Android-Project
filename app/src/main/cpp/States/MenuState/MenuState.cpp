//
// Created by LENOVO on 28-04-2026.
//
#include "MenuState.h"
void MenuState::render(SDL_Renderer *renderer) {


    SDL_FRect backgroundDst = {0, 0, 1600, 720};
    SDL_RenderTexture(renderer, backGround, nullptr, &backgroundDst);
    SDL_RenderPresent(renderer);

}
void MenuState::update(float dt) {
}
void MenuState::handleEvents(SDL_Event &event){
    while(SDL_PollEvent(&event)){
        if(event.type == SDL_EVENT_FINGER_DOWN){
            LOGI("touch down");
        }
    }
}

