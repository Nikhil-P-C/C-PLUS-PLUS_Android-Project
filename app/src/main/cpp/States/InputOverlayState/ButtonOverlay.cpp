//
// Created by LENOVO on 12-05-2026.
//

#include "ButtonOverlay.h"
#include "utils/GameData.h"
#include "utils/InputDispatcher.h"


void ButtonOverlay::render(SDL_Renderer *renderer) {
    SDL_FRect jumpButton{m_JumpButton.x, m_JumpButton.y, m_JumpButton.w, m_JumpButton.h};
    SDL_FRect leftButton{m_LeftButton.x, m_LeftButton.y, m_LeftButton.w, m_LeftButton.h};
    SDL_FRect rightButton{m_RightButton.x, m_RightButton.y, m_RightButton.w, m_RightButton.h};
    SDL_FRect crouchButton{m_CrouchButton.x, m_CrouchButton.y, m_CrouchButton.w, m_CrouchButton.h};
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderRect(renderer, &jumpButton);
    SDL_RenderRect(renderer, &leftButton);
    SDL_RenderRect(renderer, &rightButton);
    SDL_RenderRect(renderer, &crouchButton);
}

void ButtonOverlay::update(float dt) {
    if(m_TouchX > m_JumpButton.x && m_TouchX < m_JumpButton.x + m_JumpButton.w){
        if(m_TouchY > m_JumpButton.y && m_TouchY < m_JumpButton.y + m_JumpButton.h){
            InputDispatcher::getInstance().setJump(true);
        }

    }
    if(m_TouchX > m_LeftButton.x && m_TouchX < m_LeftButton.x + m_LeftButton.w){
        if(m_TouchY > m_LeftButton.y && m_TouchY < m_LeftButton.y + m_LeftButton.h){
            InputDispatcher::getInstance().setMovingLeft(true);
            InputDispatcher::getInstance().setMovingRight(false);
        }

    }

    if(m_TouchX > m_RightButton.x && m_TouchX < m_RightButton.x + m_RightButton.w){
        if(m_TouchY > m_RightButton.y && m_TouchY < m_RightButton.y + m_RightButton.h){
            InputDispatcher::getInstance().setMovingRight(true);
            InputDispatcher::getInstance().setMovingLeft(false);
        }

    }
    if(InputDispatcher::getInstance().released){
        InputDispatcher::getInstance().inputLogClear();
    }
}

void ButtonOverlay::handleEvents(SDL_Event &event) {

    if(event.type == SDL_EVENT_FINGER_DOWN || event.type == SDL_EVENT_FINGER_MOTION){
        InputDispatcher::getInstance().setInputReleased(false);
        m_TouchX =event.tfinger.x * (float)GameData::getInstance().getWinWidth();
        m_TouchY = event.tfinger.y * (float)GameData::getInstance().getWinHeight();


    }
    if(event.type == SDL_EVENT_FINGER_UP){
        InputDispatcher::getInstance().inputLogClear();
    }
}

ButtonOverlay::ButtonOverlay(SDL_Renderer *renderer) {

}
