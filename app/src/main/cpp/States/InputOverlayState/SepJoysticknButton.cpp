//
// Created by LENOVO on 13-07-2026.
//
#include <math.h>
#include "SepJoysticknButton.h"
#include "SDL3_image/SDL_image.h"
#include "engine/Engine.h"

void SepJoysticknButton::render(SDL_Renderer *renderer) {
    SDL_FRect joystick{m_joystick.x,m_joystick.y,m_joystick.w,m_joystick.h};
    SDL_FRect attackButtonDst{m_AttackButton.x,m_AttackButton.y,m_AttackButton.w,m_AttackButton.h};
    SDL_FRect joystickHandle{ m_joystickHandle.x-m_joystickHandle.h/2,
                              m_joystickHandle.y-m_joystickHandle.h/2,
                              m_joystickHandle.w,m_joystickHandle.h};
    SDL_FRect jumpButtonDst{m_JumpButton.x, m_JumpButton.y, m_JumpButton.w, m_JumpButton.h};
    SDL_FRect magicButtonDst{m_MagicButton.x, m_MagicButton.y, m_MagicButton.w, m_MagicButton.h};

    SDL_RenderTexture(renderer,m_joystickTexture,NULL,&joystick);
    SDL_RenderTexture(renderer,m_joystickHandleTexture,NULL,&joystickHandle);
    SDL_RenderTexture(renderer,m_jumpButtonTexture,NULL, &jumpButtonDst);
    SDL_RenderTexture(renderer, m_slashButtonTexture, nullptr, &attackButtonDst);
    SDL_RenderTexture(renderer, m_magicButtonTexture, nullptr,&magicButtonDst);

}

void SepJoysticknButton::update(float dt) {
    if(m_attackFingerActive){
        InputDispatcher::getInstance().triggerAttack();
        m_attackFingerActive =false;
    }
    if(m_magicFingerActive){
        if(!m_magicBeamStarted && (SDL_GetTicks() - m_magicPressStartTime) >= MAGIC_HOLD_THRESHOLD_MS){
            m_magicBeamStarted = true;
        }
        if(m_magicBeamStarted){
            InputDispatcher::getInstance().setBeaming(true);
        }
    }
    else{
        InputDispatcher::getInstance().setBeaming(false);
    }
    if(m_jumpFingerActive){
        InputDispatcher::getInstance().setJump(true);
    }

    float centerX = (m_joystick.x + m_joystick.w / 2);
    float centerY = (m_joystick.y + m_joystick.h / 2);
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
    if(m_joystickFingerActive){

        const float deadZone = 45.00f;
        if(len <deadZone){
            InputDispatcher::getInstance().setMovingLeft(false);
            InputDispatcher::getInstance().setMovingRight(false);
        }

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

    if(!m_joystickFingerActive){
        gameMath::interpolate(m_joystickHandle.x,m_joystickHandle.y,centerX,centerY,0.5f);
    }
    if(!m_joystickFingerActive){
        InputDispatcher::getInstance().setMovingLeft(false);
        InputDispatcher::getInstance().setMovingRight(false);
    }
    if(!m_jumpFingerActive){
        InputDispatcher::getInstance().setJump(false);
    }

}

bool SepJoysticknButton::handleEvents(SDL_Event &event) {
    if(event.type == SDL_EVENT_FINGER_DOWN ){
        InputDispatcher::getInstance().setInputReleased(false);
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


        if((touchX > m_JumpButton.x && touchX < m_JumpButton.x + m_JumpButton.w&&
            touchY > m_JumpButton.y && touchY < m_JumpButton.y + m_JumpButton.h )&& !m_jumpFingerActive){
            m_jumpFingerID = event.tfinger.fingerID;
            m_jumpFingerActive=true;
        }

        if((touchX > m_AttackButton.x && touchX < m_AttackButton.x + m_AttackButton.w&&
            touchY > m_AttackButton.y && touchY < m_AttackButton.y + m_AttackButton.h )&& !m_attackFingerActive){

            m_attackFingerID = event.tfinger.fingerID;
            m_attackFingerActive =true;
        }

        if((touchX > m_MagicButton.x && touchX < m_MagicButton.x + m_MagicButton.w&&
            touchY > m_MagicButton.y && touchY < m_MagicButton.y + m_MagicButton.h )&& !m_magicFingerActive){

            m_magicFingerID = event.tfinger.fingerID;
            m_magicFingerActive =true;
            m_magicPressStartTime = SDL_GetTicks();
            m_magicBeamStarted = false;
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
        }

        if(event.tfinger.fingerID == m_jumpFingerID){
            m_jumpFingerActive =false;

        }

        if(m_magicFingerActive && event.tfinger.fingerID == m_magicFingerID){
            m_magicFingerActive =false;
            if(!m_magicBeamStarted){
                InputDispatcher::getInstance().triggerHeal();   // released before the hold threshold -> tap -> heal
            }
            InputDispatcher::getInstance().setBeaming(false);
            m_magicBeamStarted = false;
        }

        return true;
    }
    return false;
}

SepJoysticknButton::SepJoysticknButton(SDL_Renderer *renderer) {
    this->Name = "JnBState";

    m_joystickTexture =Engine::Get().getAssetManager().getTexture(TextureType::JOYSTICK_JOYSTICK_OUTERRING);

    m_joystickHandleTexture =Engine::Get().getAssetManager().getTexture(TextureType::JOYSTICK_JOYSTICK_HANDLE);

    m_jumpButtonTexture =Engine::Get().getAssetManager().getTexture(TextureType::BUTTON_JUMP_BUTTON);

    m_slashButtonTexture =Engine::Get().getAssetManager().getTexture(TextureType::BUTTON_SLASH_BUTTON);

    m_magicButtonTexture =Engine::Get().getAssetManager().getTexture(TextureType::BUTTON_MAGIC_BUTTON);

    LOGI("SepJoysticknButton overlay constructor:%p",this);
}
SepJoysticknButton::~SepJoysticknButton() {

    LOGI("SepJoysticknButton overlay destructor:%p",this);
}
