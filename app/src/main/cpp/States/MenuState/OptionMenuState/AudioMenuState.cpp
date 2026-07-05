//
// Created by LENOVO on 04-07-2026.
//
#include "AudioMenuState.h"
#include "SDL3/SDL.h"
#include "SDL3_image/SDL_image.h"


void AudioMenuState::render(SDL_Renderer *renderer) {
    SDL_SetTextureScaleMode(m_menuTexture,SDL_SCALEMODE_NEAREST);

    SDL_FRect menuDst ={100.00f,25.00f,1400.00f,670.00f};
    SDL_FRect menuSrc ={0.00f,0.00f,401.00f,198.00f};
    SDL_RenderTexture(renderer,m_menuTexture,&menuSrc,&menuDst);
}

void AudioMenuState::update(float dt) {

}

bool AudioMenuState::handleEvents(SDL_Event &event) {
    return false;
}
AudioMenuState::AudioMenuState(SDL_Renderer *renderer) {
    SDL_Surface* menuSurface = IMG_Load_IO(m_menuSpriteFile,false);
    m_menuTexture =SDL_CreateTextureFromSurface(renderer,menuSurface);
    SDL_CloseIO(m_menuSpriteFile);
    SDL_DestroySurface(menuSurface);
}
