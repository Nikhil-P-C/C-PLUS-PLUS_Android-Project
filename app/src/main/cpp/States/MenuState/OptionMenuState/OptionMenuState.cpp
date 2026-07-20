//
// Created by LENOVO on 01-07-2026.
//

#include "OptionMenuState.h"
#include "SDL3/SDL.h"
#include "SDL3_image/SDL_image.h"
#include "Engine/Engine.h"
#include "States/MenuState/MenuState.h"
#include "States/MenuState/OptionMenuState/ControlMenuState.h"
#include "States/MenuState/OptionMenuState/AudioMenuState.h"

void OptionMenuState::render(SDL_Renderer *renderer) {
    SDL_FRect backgroundDst{0, 0, static_cast<float>(GameData::getInstance().getWinWidth()), static_cast<float>(GameData::getInstance().getWinHeight())};
    SDL_RenderTexture(renderer, m_backGroundTexture, nullptr, &backgroundDst);


    SDL_FRect backButton ={m_backButton.x,m_backButton.y,m_backButton.w,m_backButton.h};
    SDL_SetRenderDrawColor(renderer,0,0,255,255);
    SDL_RenderFillRect(renderer,&backButton);

}

void OptionMenuState::update(float dt) {

}

bool OptionMenuState::handleEvents(SDL_Event &event) {
    if(event.key.key == SDLK_AC_BACK) {
        int count = (int)Engine::Get().getOverlayStateCount();
        while(count){
            count--;
            LOGI("popping overlay state");
            Engine::Get().popOverlayState();
        }
        Engine::Get().changeState(std::make_unique<MenuState>(m_renderer));
        return true;
    }

    if(event.type == SDL_EVENT_FINGER_DOWN){
        float TouchX = event.tfinger.x * (float)GameData::getInstance().getWinWidth();
        float TouchY = event.tfinger.y * (float)GameData::getInstance().getWinHeight();
        if(TouchX >= m_backButton.x && TouchX <= m_backButton.x + m_backButton.w &&
            TouchY >= m_backButton.y && TouchY <= m_backButton.y + m_backButton.h){
            int count = (int)Engine::Get().getOverlayStateCount();
            while(count){
                count--;
                LOGI("popping overlay state");
                Engine::Get().popOverlayState();
            }
            Engine::Get().changeState(std::make_unique<MenuState>(m_renderer));
        }
        if(TouchX >= m_controlButton.x && TouchX <= m_controlButton.x + m_controlButton.w &&
            TouchY >= m_controlButton.y && TouchY <= m_controlButton.y + m_controlButton.h){
            if(Engine::Get().getOverlayStateCount()>0)
                Engine::Get().popOverlayState();
            Engine::Get().pushOverlayState(std::make_unique<ControlMenuState>(m_renderer));
            return true;
        }
        if(TouchX >= m_audioButton.x && TouchX <= m_audioButton.x + m_audioButton.w &&
            TouchY >= m_audioButton.y && TouchY <= m_audioButton.y + m_audioButton.h){
            if(Engine::Get().getOverlayStateCount()>0)
                Engine::Get().popOverlayState();
            Engine::Get().pushOverlayState(std::make_unique<AudioMenuState>(m_renderer));
            return true;
        }
        return true;
    }
    return false;

}

OptionMenuState::OptionMenuState(SDL_Renderer *renderer) {
    m_renderer =renderer;
    SDL_Surface* backgroundSurface = IMG_Load_IO(m_backGroundSprite,false);
    m_backGroundTexture = SDL_CreateTextureFromSurface(renderer,backgroundSurface);
    SDL_CloseIO(m_backGroundSprite);
    SDL_DestroySurface(backgroundSurface);
    //push default optionstate
    Engine::Get().pushOverlayState(std::make_unique<AudioMenuState>(m_renderer));

}

OptionMenuState::~OptionMenuState() {
    SDL_DestroyTexture(m_backGroundTexture);
}
