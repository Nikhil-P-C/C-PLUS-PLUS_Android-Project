//
// Created by LENOVO on 12-05-2026.
//

#include <cmath>
#include <algorithm>
#include "JoystickOverlay.h"
#include "utils/utils.h"
#include "SDL3_image/SDL_image.h"
#include "utils/gameMath.h"

void JoystickOverlay::render(SDL_Renderer *renderer) {
    SDL_FRect joystick{m_joystick.x,m_joystick.y,m_joystick.w,m_joystick.h};
    SDL_FRect attackButtonDst{m_AttackButton.x,m_AttackButton.y,m_AttackButton.w,m_AttackButton.h};
    SDL_FRect joystickHandle{ m_joystickHandle.x-m_joystickHandle.h/2,
                              m_joystickHandle.y-m_joystickHandle.h/2,
                              m_joystickHandle.w,m_joystickHandle.h};

    SDL_RenderTexture(renderer,m_joystickTexture, nullptr,&joystick);
    SDL_RenderTexture(renderer,m_joystickHandleTexture, nullptr,&joystickHandle);
    SDL_RenderTexture(renderer, m_slashButtonTexture, nullptr, &attackButtonDst);

}

void JoystickOverlay::update(float dt) {

    if(m_attackFingerActive){
        InputDispatcher::getInstance().triggerAttack();
        m_attackFingerActive =false;
    }

    float centerX = (m_joystick.x + m_joystick.w / 2);
    float centerY = (m_joystick.y + m_joystick.h / 2);

    if(m_joystickFingerActive){
        float dY = m_touchY - centerY;
        float dX = m_touchX - centerX;
        float ndY =dY;
        float ndX =dX;
        float radius =m_joystick.w/2;
        float len = sqrtf(dX * dX + dY * dY);

        if(len > radius){
            dX = (dX / len) * radius ;
            dY = (dY / len) * radius;
        }


        if (len > 0) {
            ndX /= len;
            ndY /= len;
        }

        const float deadZone = 45.00f;
        if(len <deadZone)
            InputDispatcher::getInstance().inputLogClear();
        if(dY <-deadZone && dX < 106.0f && dX > -106.0f){

            InputDispatcher::getInstance().setJump(true);
        }
        else InputDispatcher::getInstance().setJump(false);

        if (dX > deadZone) {
            InputDispatcher::getInstance().setMovingRight(true);
            InputDispatcher::getInstance().setMovingLeft(false);
        }
        else if(dX < -deadZone) {
            InputDispatcher::getInstance().setMovingLeft(true);
            InputDispatcher::getInstance().setMovingRight(false);
        }



        m_joystickHandle.x = (centerX + dX);
        m_joystickHandle.y = (centerY + dY);

    }
    else{
        InputDispatcher::getInstance().setMovingLeft(false);
        InputDispatcher::getInstance().setMovingRight(false);
        InputDispatcher::getInstance().setInputReleased(true);
    }
    
    if(!m_joystickFingerActive){
        gameMath::interpolate(m_joystickHandle.x,m_joystickHandle.y,centerX,centerY,0.5f);
    }
}

bool JoystickOverlay::handleEvents(SDL_Event &event) {
    if(event.type == SDL_EVENT_FINGER_DOWN){
        float touchX =event.tfinger.x * (float)GameData::getInstance().getWinWidth();
        float touchY = event.tfinger.y * (float)GameData::getInstance().getWinHeight();

        if ((touchX >= m_joystick.x &&
            touchX <= m_joystick.x + m_joystick.w &&
            touchY >= m_joystick.y &&
            touchY <= m_joystick.y + m_joystick.h )&& !m_joystickFingerActive)
        {
            m_joystickFingerID = event.tfinger.fingerID;
            m_joystickFingerActive = true;
            InputDispatcher::getInstance().setInputReleased(false);
        }
        if((touchX > m_AttackButton.x && touchX < m_AttackButton.x + m_AttackButton.w&&
            touchY > m_AttackButton.y && touchY < m_AttackButton.y + m_AttackButton.h )&& !m_attackFingerActive){

            m_attackFingerID = event.tfinger.fingerID;
            m_attackFingerActive =true;
        }
        return true;
    }
    if(event.type == SDL_EVENT_FINGER_MOTION){
        float touchX =event.tfinger.x * (float)GameData::getInstance().getWinWidth();
        float touchY = event.tfinger.y * (float)GameData::getInstance().getWinHeight();
        if(event.tfinger.fingerID == m_joystickFingerID){

            m_touchX = touchX;
            m_touchY = touchY;
        }
    }
    if(event.type == SDL_EVENT_FINGER_UP){
        if(event.tfinger.fingerID == m_joystickFingerID){
            m_joystickFingerActive =false;
            InputDispatcher::getInstance().inputLogClear();
        }
        return true;
    }
    return false;
}

JoystickOverlay::JoystickOverlay(SDL_Renderer *renderer) {
    this->Name = "JoystickState";

    m_joystickTexture =Engine::Get().getAssetManager().getTexture(TextureType::JOYSTICK_JOYSTICK_OUTERRING);
    m_joystickHandleTexture =Engine::Get().getAssetManager().getTexture(TextureType::JOYSTICK_JOYSTICK_HANDLE);
    m_slashButtonTexture =Engine::Get().getAssetManager().getTexture(TextureType::BUTTON_SLASH_BUTTON);

    LOGI("joystick overlay constructor:%p",this);
}
JoystickOverlay::~JoystickOverlay() {
    LOGI("joystick overlay destructor:%p",this);
}