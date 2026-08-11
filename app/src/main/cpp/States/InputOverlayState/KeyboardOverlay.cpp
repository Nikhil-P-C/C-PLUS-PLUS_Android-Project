//
// Created by LENOVO on 08-08-2026.
//
#include "KeyboardOverlay.h"
#include "utils/utils.h"
void KeyboardOverlay::render(SDL_Renderer *renderer) {
    SDL_FRect dst{0.00f,200.00f,100.00f,100.00f};
    SDL_SetRenderDrawColor(renderer,255,0,0,255);
    SDL_RenderFillRect(renderer,&dst);
}

void KeyboardOverlay::update(float dt) {
}

bool KeyboardOverlay::handleEvents(SDL_Event &event) {
    if(event.type == SDL_EVENT_KEY_DOWN){
        if(event.key.key == SDLK_A){
            InputDispatcher::getInstance().setMovingLeft(true);
            return true;
        }
        if(event.key.key == SDLK_D){
            InputDispatcher::getInstance().setMovingRight(true);
            return true;
        }
        if(event.key.key == SDLK_SPACE){
            InputDispatcher::getInstance().setJump(true);
            return true;
        }
    }
    else if(event.type == SDL_EVENT_KEY_UP){
        if(event.key.key == SDLK_A){
            InputDispatcher::getInstance().setMovingLeft(false);
            return true;
        }
        if(event.key.key == SDLK_D){
            InputDispatcher::getInstance().setMovingRight(false);
            return true;
        }
        if(event.key.key == SDLK_SPACE){
            InputDispatcher::getInstance().setJump(false);
            return true;
        }
    }

    return false;
}

KeyboardOverlay::KeyboardOverlay(SDL_Renderer *renderer) {
    m_renderer=renderer;
    LOGI("keyboard overlay constructor:%p",this);

}

KeyboardOverlay::~KeyboardOverlay() {
    LOGI("keyboard overlay destructor:%p",this);
}

