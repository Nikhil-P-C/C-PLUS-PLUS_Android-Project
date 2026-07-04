//
// Created by LENOVO on 04-07-2026.
//
#include "AudioMenuState.h"
#include "SDL3/SDL.h"



void AudioMenuState::render(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer,255,0,0,255);
    SDL_FRect square{400.00f,50.00f,1000.00f,600.00f};
    SDL_RenderFillRect(renderer,&square);
}

void AudioMenuState::update(float dt) {

}

void AudioMenuState::handleEvents(SDL_Event &event) {

}
AudioMenuState::AudioMenuState(SDL_Renderer *renderer) {

}
