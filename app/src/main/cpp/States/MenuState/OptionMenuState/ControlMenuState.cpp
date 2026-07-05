//
// Created by LENOVO on 02-07-2026.
//
#include "ControlMenuState.h"
#include "SDL3/SDL.h"
#include "SDL3_image/SDL_image.h"

void ControlMenuState::render(SDL_Renderer *renderer) {
    SDL_SetTextureScaleMode(m_menuTexture,SDL_SCALEMODE_NEAREST);
    SDL_FRect menuDst ={100.00f,25.00f,1400.00f,670.00f};
    SDL_FRect menuSrc ={402.00f,0.00f,402.00f,198.00f};
    SDL_RenderTexture(renderer,m_menuTexture,&menuSrc,&menuDst);

    SDL_SetRenderDrawColor(renderer,0,0,255,255);
    SDL_FRect joystickButton ={m_joystickButton.x,m_joystickButton.y,m_joystickButton.w,m_joystickButton.h};
    SDL_RenderFillRect(renderer,&joystickButton);
    SDL_SetRenderDrawColor(renderer,255,0,0,255);
    SDL_FRect buttonButton ={m_buttonButton.x,m_buttonButton.y,m_buttonButton.w,m_buttonButton.h};
    SDL_RenderFillRect(renderer,&buttonButton);
}

void ControlMenuState::update(float dt) {
    GameData::getInstance().setControlType(m_controlType);
}

bool ControlMenuState::handleEvents(SDL_Event &event) {
    float touchX = event.tfinger.x * (float)GameData::getInstance().getWinWidth();
    float touchY = event.tfinger.y * (float)GameData::getInstance().getWinHeight();

    if(event.type == SDL_EVENT_FINGER_DOWN){
        if(touchX >= m_joystickButton.x && touchX <= m_joystickButton.x + m_joystickButton.w &&
            touchY >= m_joystickButton.y && touchY <= m_joystickButton.y + m_joystickButton.h){
            m_controlType = ControlType::JOYSTICK;
            return true;
        }
        if(touchX >= m_buttonButton.x && touchX <= m_buttonButton.x + m_buttonButton.w &&
            touchY >= m_buttonButton.y && touchY <= m_buttonButton.y + m_buttonButton.h){
            m_controlType = ControlType::BUTTONS;
            return true;
        }
    }
    return false;
}

ControlMenuState::ControlMenuState(SDL_Renderer *renderer) {
    SDL_Surface* menuSurface = IMG_Load_IO(m_menuSpriteFile,false);
    m_menuTexture = SDL_CreateTextureFromSurface(renderer,menuSurface);
    SDL_CloseIO(m_menuSpriteFile);
    SDL_DestroySurface(menuSurface);
}


