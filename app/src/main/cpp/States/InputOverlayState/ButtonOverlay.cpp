//
// Created by LENOVO on 12-05-2026.
//

#include "ButtonOverlay.h"
#include "utils/GameData.h"
#include "utils/InputDispatcher.h"
#include "SDL3_image/SDL_image.h"


void ButtonOverlay::render(SDL_Renderer *renderer) {
    SDL_FRect jumpButtonDst{m_JumpButton.x, m_JumpButton.y, m_JumpButton.w, m_JumpButton.h};
    SDL_FRect leftButtonDst{m_LeftButton.x, m_LeftButton.y, m_LeftButton.w, m_LeftButton.h};
    SDL_FRect rightButtonDst{m_RightButton.x, m_RightButton.y, m_RightButton.w, m_RightButton.h};

    SDL_RenderTexture(renderer, m_jumpButtonTexture, nullptr, &jumpButtonDst);
    SDL_RenderTexture(renderer, m_leftButtonTexture, nullptr, &leftButtonDst);
    SDL_RenderTexture(renderer, m_rightButtonTexture, nullptr, &rightButtonDst);


//    SDL_FRect crouchButtonDst{m_CrouchButton.x, m_CrouchButton.y, m_CrouchButton.w, m_CrouchButton.h};
//    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
//    SDL_RenderRect(renderer, &crouchButtonDst);
}

void ButtonOverlay::update(float dt) {
    if( m_jumpFingerActive){
        InputDispatcher::getInstance().setJump(true);
    }
    if(m_dFingerActive){
        LOGI("dfinger active");
         if(m_TouchX > m_LeftButton.x && m_TouchX < m_LeftButton.x + m_LeftButton.w &&
            m_TouchY > m_LeftButton.y && m_TouchY < m_LeftButton.y + m_LeftButton.h){

             InputDispatcher::getInstance().setMovingLeft(true);
             InputDispatcher::getInstance().setMovingRight(false);
             LOGI("left pressed");
             LOGI("TouchX:%f TouchY:%f",m_TouchX,m_TouchY);
         }
         if(m_TouchX > m_RightButton.x && m_TouchX < m_RightButton.x + m_RightButton.w &&
            m_TouchY > m_RightButton.y && m_TouchY < m_RightButton.y + m_RightButton.h){

             InputDispatcher::getInstance().setMovingRight(true);
             InputDispatcher::getInstance().setMovingLeft(false);
             LOGI("right pressed");
         }


    }
    else{
        InputDispatcher::getInstance().setMovingLeft(false);
        InputDispatcher::getInstance().setMovingRight(false);
    }
    if(!m_jumpFingerActive && !m_dFingerActive){
        InputDispatcher::getInstance().inputLogClear();
    }
    if(InputDispatcher::getInstance().released){
        InputDispatcher::getInstance().inputLogClear();
    }
    //uncomment for debuging
//    LOGI(" jump:%d left:%d right:%d",InputDispatcher::getInstance().getJump(),
//                InputDispatcher::getInstance().getMovingLeft(),
//                InputDispatcher::getInstance().getMovingRight());
}

bool ButtonOverlay::handleEvents(SDL_Event &event) {

    if(event.type == SDL_EVENT_FINGER_DOWN ){
        InputDispatcher::getInstance().setInputReleased(false);
        float touchX = event.tfinger.x * (float)GameData::getInstance().getWinWidth();
        float touchY = event.tfinger.y * (float)GameData::getInstance().getWinHeight();


        if((touchX > m_LeftButton.x && touchX < m_LeftButton.x + m_LeftButton.w&&
            touchY > m_LeftButton.y && touchY < m_LeftButton.y + m_LeftButton.h||
            touchX > m_RightButton.x && touchX < m_RightButton.x + m_RightButton.w&&
            touchY > m_RightButton.y && touchY < m_RightButton.y + m_RightButton.h )&& !m_dFingerActive){
            m_dFingerID = event.tfinger.fingerID;
            m_dFingerActive =true;
        }
        if(event.tfinger.fingerID == m_dFingerID){
            m_TouchX =touchX;
            m_TouchY = touchY;
        }
        if((touchX > m_JumpButton.x && touchX < m_JumpButton.x + m_JumpButton.w&&
            touchY > m_JumpButton.y && touchY < m_JumpButton.y + m_JumpButton.h )&& !m_jumpFingerActive){

            m_jumpFingerID = event.tfinger.fingerID;
            m_jumpFingerActive=true;
        }


    }
    if(event.type == SDL_EVENT_FINGER_MOTION){
        float touchX =event.tfinger.x * (float)GameData::getInstance().getWinWidth();
        float touchY = event.tfinger.y * (float)GameData::getInstance().getWinHeight();
        if(event.tfinger.fingerID == m_dFingerID){
            if ((touchX > m_LeftButton.x && touchX < m_LeftButton.x + m_LeftButton.w &&
                 touchY > m_LeftButton.y && touchY < m_LeftButton.y + m_LeftButton.h ||
                 touchX > m_RightButton.x && touchX < m_RightButton.x + m_RightButton.w &&
                 touchY > m_RightButton.y && touchY < m_RightButton.y + m_RightButton.h) &&
                !m_dFingerActive) {

                m_dFingerActive = true;
            }
            m_TouchX = touchX;
            m_TouchY = touchY;
        }
    }
    if(event.type == SDL_EVENT_FINGER_UP){
        if(event.tfinger.fingerID == m_jumpFingerID){
            m_jumpFingerActive =false;
        }
        if(event.tfinger.fingerID == m_dFingerID){
            LOGI("d finger up");
            m_dFingerActive =false;
        }
        return true;
    }
    return false;
}


ButtonOverlay::ButtonOverlay(SDL_Renderer *renderer) {
    SDL_Surface* jumpButtonSurface = IMG_Load_IO(m_jumpButtonFile,false);
    SDL_Surface* leftButtonSurface = IMG_Load_IO(m_leftButtonFile,false);
    SDL_Surface* rightButtonSurface = IMG_Load_IO(m_rightButtonFile,false);
    m_jumpButtonTexture = SDL_CreateTextureFromSurface(renderer,jumpButtonSurface);
    m_leftButtonTexture = SDL_CreateTextureFromSurface(renderer,leftButtonSurface);
    m_rightButtonTexture = SDL_CreateTextureFromSurface(renderer,rightButtonSurface);
    SDL_CloseIO(m_jumpButtonFile);
    SDL_CloseIO(m_leftButtonFile);
    SDL_CloseIO(m_rightButtonFile);
    SDL_DestroySurface(jumpButtonSurface);
    SDL_DestroySurface(leftButtonSurface);
    SDL_DestroySurface(rightButtonSurface);
}
