//
// Created by LENOVO on 11-08-2026.
//
#include "utils/utils.h"
#include "GamepadOverlay.h"
void GamepadOverlay::render(SDL_Renderer *renderer) {
    SDL_FRect dst{0.00f,200.00f,100.00f,100.00f};
    SDL_SetRenderDrawColor(renderer,0,255,0,255);
    SDL_RenderFillRect(renderer,&dst);
}

void GamepadOverlay::update(float dt) {

    float deadzone = 0.25f;

    if(m_leftStickX>deadzone){
        InputDispatcher::getInstance().setMovingRight(true);
        InputDispatcher::getInstance().setMovingLeft(false);
    }
    else if(m_leftStickX<-deadzone){
        InputDispatcher::getInstance().setMovingRight(false);
        InputDispatcher::getInstance().setMovingLeft(true);
    }
    else{
        InputDispatcher::getInstance().setMovingRight(false);
        InputDispatcher::getInstance().setMovingLeft(false);
    }

}

bool GamepadOverlay::handleEvents(SDL_Event &event) {

    if(event.type == SDL_EVENT_GAMEPAD_ADDED){
        m_gamepad = SDL_OpenGamepad(event.gdevice.which);
        if(!m_gamepad)
            LOGE("gamepad failed");
    }
    if(event.type ==SDL_EVENT_GAMEPAD_REMOVED){
        if(m_gamepad && SDL_GetGamepadID(m_gamepad) == event.gdevice.which)
            SDL_CloseGamepad(m_gamepad);
        m_gamepad = nullptr;
    }

    switch(event.type){
        case SDL_EVENT_GAMEPAD_AXIS_MOTION:
            if(event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTX){
                 float x  =event.gaxis.value / 32767;
                 m_leftStickX =x;
            }
            if(event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTY){

                float y  = event.gaxis.value / 32767;
                m_leftStickY =y;
            }
            break;
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
            if(event.gbutton.button == SDL_GAMEPAD_BUTTON_SOUTH){
                InputDispatcher::getInstance().setJump(true);
            }
            break;

        case SDL_EVENT_GAMEPAD_BUTTON_UP:

            if(event.gbutton.button == SDL_GAMEPAD_BUTTON_SOUTH){
                InputDispatcher::getInstance().setJump(false);
            }
    }
    return false;
}

GamepadOverlay::GamepadOverlay(SDL_Renderer *renderer) {

    m_renderer=renderer;
    LOGI("Gamepad overlay constructor:%p",this);

}

GamepadOverlay::~GamepadOverlay() {
    LOGI("Gamepad overlay destructor:%p",this);
}

