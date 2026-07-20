//
// Created by LENOVO on 13-07-2026.
//
#include <math.h>
#include "SepJoysticknButton.h"
#include "SDL3_image/SDL_image.h"

void SepJoysticknButton::render(SDL_Renderer *renderer) {
    SDL_FRect joystick{m_joystick.x,m_joystick.y,m_joystick.w,m_joystick.h};
    SDL_RenderTexture(renderer,m_joystickTexture,NULL,&joystick);

    SDL_FRect joystickHandle{ m_joystickHandle.x-m_joystickHandle.h/2,
                              m_joystickHandle.y-m_joystickHandle.h/2,
                              m_joystickHandle.w,m_joystickHandle.h};

    SDL_RenderTexture(renderer,m_joystickHandleTexture,NULL,&joystickHandle);

    SDL_FRect jumpButtonDst{m_JumpButton.x, m_JumpButton.y, m_JumpButton.w, m_JumpButton.h};
    SDL_RenderTexture(renderer,m_jumpButtonTexture,NULL, &jumpButtonDst);

}

void SepJoysticknButton::update(float dt) {

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
        InputDispatcher::getInstance().setInputReleased(true);
    }
    if(!m_jumpFingerActive){
        InputDispatcher::getInstance().setJump(false);
    }
    if(!m_joystickFingerActive && !m_jumpFingerActive)
        InputDispatcher::getInstance().inputLogClear();
//    LOGI(" jump:%d left:%d right:%d",InputDispatcher::getInstance().getJump(),
//                InputDispatcher::getInstance().getMovingLeft(),
//                InputDispatcher::getInstance().getMovingRight());
}

bool SepJoysticknButton::handleEvents(SDL_Event &event) {
    if(event.type == SDL_EVENT_FINGER_DOWN || event.type == SDL_EVENT_FINGER_MOTION){
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
        if(event.tfinger.fingerID == m_joystickFingerID){
            m_touchX = touchX;
            m_touchY = touchY;
        }

        if((touchX > m_JumpButton.x && touchX < m_JumpButton.x + m_JumpButton.w&&
            touchY > m_JumpButton.y && touchY < m_JumpButton.y + m_JumpButton.h )&& !m_jumpFingerActive){
//            LOGI("jump button pressed");
            m_jumpFingerID = event.tfinger.fingerID;
            m_jumpFingerActive=true;
        }

        return true;
    }

    if(event.type == SDL_EVENT_FINGER_UP){
        if(event.tfinger.fingerID == m_joystickFingerID){
            m_joystickFingerActive =false;
        }
        if(event.tfinger.fingerID == m_jumpFingerID){
//            LOGI("jump finger up");
            m_jumpFingerActive =false;

        }
        return true;
    }
    return false;
}

SepJoysticknButton::SepJoysticknButton(SDL_Renderer *renderer) {
    SDL_Surface* joystickSurface = IMG_Load_IO(m_joystickFile, false);
    m_joystickTexture =SDL_CreateTextureFromSurface(renderer, joystickSurface);
    SDL_DestroySurface(joystickSurface);

    SDL_Surface* joystickHandleSurface = IMG_Load_IO(m_joystickHandleFile, false);
    m_joystickHandleTexture =SDL_CreateTextureFromSurface(renderer, joystickHandleSurface);
    SDL_DestroySurface(joystickHandleSurface);

    SDL_Surface* jumpButtonSurface = IMG_Load_IO(m_jumpButtonFile, false);
    m_jumpButtonTexture =SDL_CreateTextureFromSurface(renderer, jumpButtonSurface);
    SDL_DestroySurface(jumpButtonSurface);

    SDL_SetTextureScaleMode(m_joystickTexture, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_joystickHandleTexture, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_jumpButtonTexture, SDL_SCALEMODE_NEAREST);

    SDL_CloseIO(m_jumpButtonFile);
    SDL_CloseIO(m_joystickFile);
    SDL_CloseIO(m_joystickHandleFile);

    LOGI("SepJoysticknButton overlay constructor:%p",this);
}
SepJoysticknButton::~SepJoysticknButton() {
    SDL_DestroyTexture(m_joystickTexture);
    SDL_DestroyTexture(m_joystickHandleTexture);
    SDL_DestroyTexture(m_jumpButtonTexture);
    LOGI("SepJoysticknButton overlay destructor:%p",this);
}
