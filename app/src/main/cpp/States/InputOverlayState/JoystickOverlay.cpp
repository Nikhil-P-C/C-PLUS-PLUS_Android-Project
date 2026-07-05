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
    SDL_RenderTexture(renderer,m_joystickTexture,NULL,&joystick);

    SDL_FRect joystickHandle{ m_joystickHandle.x-m_joystickHandle.h/2,
                              m_joystickHandle.y-m_joystickHandle.h/2,
                              m_joystickHandle.w,m_joystickHandle.h};

    SDL_RenderTexture(renderer,m_joystickHandleTexture,NULL,&joystickHandle);
}

void JoystickOverlay::update(float dt) {

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

//        float centerX = m_joystick.x + m_joystick.w / 2;
//        float centerY = m_joystick.y + m_joystick.h / 2;
//        float dY = m_touchY - centerY;
//        float dX = m_touchX - centerX;
//        float len = sqrtf(dX * dX + dY * dY);
        const float ndeadZone = 30.0f; // pixels


        if (len > 0) {
            ndX /= len;
            ndY /= len;
        }

        const float deadZone = 45.00f;
        if(len <deadZone)
            InputDispatcher::getInstance().inputLogClear();
        if (dX > deadZone) {
            InputDispatcher::getInstance().setMovingRight(true);
            InputDispatcher::getInstance().setMovingLeft(false);
        }
        else if(dX < -deadZone) {
            InputDispatcher::getInstance().setMovingLeft(true);
            InputDispatcher::getInstance().setMovingRight(false);
        }
        if(dY <deadZone && dX < 0.9 && dX > -0.9){
            LOGI("JUmp pressed");
            InputDispatcher::getInstance().setJump(true);
        }
        else InputDispatcher::getInstance().setJump(false);

        m_joystickHandle.x = (centerX + dX);
        m_joystickHandle.y = (centerY + dY);

    }
//    LOGI("joystick handle x:%f y:%f w:%f h:%f",m_joystickHandle.x,m_joystickHandle.y,m_joystickHandle.w,m_joystickHandle.h);
//    LOGI("joystick x:%f y:%f w:%f h:%f",m_joystick.x,m_joystick.y,m_joystick.w,m_joystick.h);
//    LOGI("center x:%f y:%f",centerX,centerY);
    if(!m_joystickFingerActive){
        gameMath::interpolate(m_joystickHandle.x,m_joystickHandle.y,centerX,centerY,0.5f);
    }
}

bool JoystickOverlay::handleEvents(SDL_Event &event) {
    if(event.type == SDL_EVENT_FINGER_DOWN || event.type == SDL_EVENT_FINGER_MOTION){
        InputDispatcher::getInstance().setInputReleased(false);
        float touchX =event.tfinger.x * (float)GameData::getInstance().getWinWidth();
        float touchY = event.tfinger.y * (float)GameData::getInstance().getWinHeight();

        if ((touchX >= m_joystick.x &&
            touchX <= m_joystick.x + m_joystick.w &&
            touchY >= m_joystick.y &&
            touchY <= m_joystick.y + m_joystick.h )&& !m_joystickFingerActive)
        {
            m_fingerID = event.tfinger.fingerID;
            m_joystickFingerActive = true;
            InputDispatcher::getInstance().setInputReleased(false);
        }
        if(event.tfinger.fingerID == m_fingerID){
            m_touchX = touchX;
            m_touchY = touchY;
        }
        return true;
    }

    if(event.type == SDL_EVENT_FINGER_UP){
        if(event.tfinger.fingerID == m_fingerID){
            m_joystickFingerActive =false;
            InputDispatcher::getInstance().inputLogClear();
        }
        return true;
    }
    return false;
}

JoystickOverlay::JoystickOverlay(SDL_Renderer *renderer) {
    SDL_Surface* joystickSurface = IMG_Load_IO(m_joystickFile, false);
    m_joystickTexture =SDL_CreateTextureFromSurface(renderer, joystickSurface);
    SDL_DestroySurface(joystickSurface);

    SDL_Surface* joystickHandleSurface = IMG_Load_IO(m_joystickHandleFile, false);
    m_joystickHandleTexture =SDL_CreateTextureFromSurface(renderer, joystickHandleSurface);
    SDL_DestroySurface(joystickHandleSurface);
    LOGI("joystick overlay constructor:%p",this);
}
JoystickOverlay::~JoystickOverlay() {
    LOGI("joystick overlay destructor:%p",this);
}