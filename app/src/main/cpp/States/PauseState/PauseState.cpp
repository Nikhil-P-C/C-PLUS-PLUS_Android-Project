//
// Created by LENOVO on 01-05-2026.
//
#include <android/log.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>

#include "PauseState.h"
#include "engine/Engine.h"
#include "States/GameState/GameState.h"
#include "States/MenuState/TitleScreenState.h"
#include "States/MenuState/MenuState.h"

PauseState::PauseState(SDL_Renderer *renderer) {
    LOGI("Pause construct");
    init(renderer);
}

void PauseState::init(SDL_Renderer *renderer) {
    m_font = TTF_OpenFontIO(m_fontfile,false,24);
    LOGI("font:%d",(bool)m_font);
    m_fontShadow = TTF_OpenFontIO(m_fontShadowfile,false,24);

    SDL_SetTextureScaleMode(m_fontShadowTexture,SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_fontTexture,SDL_SCALEMODE_NEAREST);
    m_renderer = renderer;

    LOGI("Pause init completed");
}

void PauseState::render(SDL_Renderer *renderer) {
    {
        SDL_Surface* fontSurface = TTF_RenderText_Solid(m_font,"paused ",
                                                        7,SDL_Color{255,255,255,255});
        SDL_Surface* fontShadowSurface = TTF_RenderText_Solid(m_fontShadow,"paused ",
                                                              7,SDL_Color{0,0,0,255});
        m_fontTexture = SDL_CreateTextureFromSurface(renderer,fontSurface);
        m_fontShadowTexture = SDL_CreateTextureFromSurface(renderer,fontShadowSurface);
        SDL_DestroySurface(fontSurface);
        SDL_DestroySurface(fontShadowSurface);

        SDL_FRect dst = {800 - 150, 320 - 50, 300, 100};
        SDL_RenderTexture(renderer, m_fontShadowTexture, nullptr, &dst);
        SDL_RenderTexture(renderer, m_fontTexture, nullptr, &dst);
    }
    {
        SDL_Surface* fontShadowSurface = TTF_RenderText_Solid(m_fontShadow,"exit",
                                                        4,SDL_Color{0,0,0,255});
        SDL_Surface* fontSurface = TTF_RenderText_Solid(m_font,"exit",
                                                          4,SDL_Color{255,255,255,255});
        m_fontTexture = SDL_CreateTextureFromSurface(renderer,fontSurface);
        m_fontShadowTexture = SDL_CreateTextureFromSurface(renderer,fontShadowSurface);
        SDL_DestroySurface(fontSurface);
        SDL_DestroySurface(fontShadowSurface);
        SDL_FRect dst = {800-100, 600 - 50, 200, 100};
        SDL_RenderTexture(renderer, m_fontShadowTexture, nullptr, &dst);
        SDL_RenderTexture(renderer, m_fontTexture, nullptr, &dst);
    }
}

void PauseState::update(float dt) {

}
bool PauseState::shouldClose(float x ,float y) const{
    if(x >= m_exit.x && x <= m_exit.x + m_exit.w && y >= m_exit.y && y <= m_exit.y + m_exit.h)return true;
    return false;
}
void PauseState::handleEvents(SDL_Event &event) {
    if(shouldClose(event.tfinger.x * 1600,event.tfinger.y*720)){
        LOGI("should close");
        Engine::Get().popState();
        Engine::Get().changeState(std::make_unique<MenuState>(m_renderer));
        return;
    }
    if(event.type == SDL_EVENT_FINGER_DOWN){
        LOGI("back to game state");
        Engine::Get().popState();
    }

}

PauseState::~PauseState() {
    LOGI("Pause destructor");
    if(m_font)      TTF_CloseFont(m_font);
    if(m_fontShadow)TTF_CloseFont(m_fontShadow);

    if(m_fontfile)      SDL_CloseIO(m_fontfile);
    if(m_fontShadowfile)SDL_CloseIO(m_fontShadowfile);

    if(m_fontTexture)      SDL_DestroyTexture(m_fontTexture);
    if(m_fontShadowTexture)SDL_DestroyTexture(m_fontShadowTexture);

}
