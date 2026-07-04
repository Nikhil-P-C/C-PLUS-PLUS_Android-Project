//
// Created by LENOVO on 02-07-2026.
//
#include "ControlMenuState.h"
#include "SDL3/SDL.h"

void ControlMenuState::render(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer,0,255,0,255);
    SDL_FRect square{400.00f,50.00f,1000.00f,600.00f};
    SDL_RenderFillRect(renderer,&square);
}

void ControlMenuState::update(float dt) {

}

void ControlMenuState::handleEvents(SDL_Event &event) {

}

ControlMenuState::ControlMenuState(SDL_Renderer *renderer) {

}


