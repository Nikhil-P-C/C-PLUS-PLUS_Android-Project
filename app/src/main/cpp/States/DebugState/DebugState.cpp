//
// Created by LENOVO on 03-05-2026.
//
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <strings.h>
#include <string>
#include "DebugState.h"
#include "engine/Engine.h"


void DebugState::render(SDL_Renderer *renderer) {
    std::string fps = "FPS:" + std::to_string(m_frames);
    SDL_Surface* fpsSurface = TTF_RenderText_Solid(m_font, fps.c_str(),fps.length(), {255,255,255});
    m_fpsTexture = SDL_CreateTextureFromSurface(renderer, fpsSurface);
    SDL_Color fpsColor = { 255, 255, 255, 255 };
    SDL_FRect fpsrect {0,0,100,100};
    SDL_RenderTexture(renderer, m_fpsTexture, nullptr, &fpsrect);
    SDL_DestroyTexture(m_fpsTexture);
    SDL_DestroySurface(fpsSurface);
}

void DebugState::update(float dt) {
    m_fps++;
    unsigned int current = SDL_GetTicks();
    if(current - m_last >= 1000){

        m_frames =m_fps;
        m_fps =0;
        m_last = current;
        LOGI("FPS: %d",m_frames);
    }
}

void DebugState::handleEvents(SDL_Event &event) {
    LOGI("debug state handle events");
}

DebugState::DebugState(SDL_Renderer *renderer) {

    m_font = TTF_OpenFontIO(m_fontFile,false, 24);
    LOGI("font:%d",(bool)m_font);
    if(!m_font)LOGE("failed to load font");
}

