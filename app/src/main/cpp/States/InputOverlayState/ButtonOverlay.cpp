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
    SDL_FRect attackButtonDst{m_AttackButton.x,m_AttackButton.y,m_AttackButton.w,m_AttackButton.h};
    SDL_FRect magicButtonDst{m_MagicButton.x, m_MagicButton.y, m_MagicButton.w, m_MagicButton.h};

    SDL_RenderTexture(renderer, m_jumpButtonTexture, nullptr, &jumpButtonDst);
    SDL_RenderTexture(renderer, m_leftButtonTexture, nullptr, &leftButtonDst);
    SDL_RenderTexture(renderer, m_rightButtonTexture, nullptr, &rightButtonDst);
    // up / down have their own art now
    SDL_FRect upButtonDst{m_UpButton.x, m_UpButton.y, m_UpButton.w, m_UpButton.h};
    SDL_RenderTexture(renderer, m_upButtonTexture, nullptr, &upButtonDst);
    SDL_FRect downButtonDst{m_DownButton.x, m_DownButton.y, m_DownButton.w, m_DownButton.h};
    SDL_RenderTexture(renderer, m_downButtonTexture, nullptr, &downButtonDst);
    SDL_RenderTexture(renderer, m_slashButtonTexture, nullptr, &attackButtonDst);
    SDL_RenderTexture(renderer, m_magicButtonTexture, nullptr,&magicButtonDst);

    // dash button: placeholder rect until the art exists
    {
        Uint8 r, g, b, a;
        SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
        SDL_FRect dashButtonDst{m_DashButton.x, m_DashButton.y, m_DashButton.w, m_DashButton.h};
        SDL_SetRenderDrawColor(renderer, 80, 200, 255, 255);
        SDL_RenderRect(renderer, &dashButtonDst);
        SDL_SetRenderDrawColor(renderer, r, g, b, a);
    }

//    SDL_FRect crouchButtonDst{m_CrouchButton.x, m_CrouchButton.y, m_CrouchButton.w, m_CrouchButton.h};
//    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
//    SDL_RenderRect(renderer, &crouchButtonDst);
}

void ButtonOverlay::update(float dt) {
    if(m_jumpFingerActive){
        InputDispatcher::getInstance().setJump(true);
    }
    else
        InputDispatcher::getInstance().setJump(false);

    if(m_attackFingerActive){
        InputDispatcher::getInstance().triggerAttack();
        m_attackFingerActive =false;
    }
    if(m_upPressed){
        InputDispatcher::getInstance().triggerInteract();   // talk to / read the interactable, advance a dialog
        m_upPressed =false;
    }
    // held up / down buttons aim the slash (up: upward slash, down in the air: downward slash / pogo)
    InputDispatcher::getInstance().setLookUp(m_upFingerActive);
    InputDispatcher::getInstance().setLookDown(m_downFingerActive);

    if(m_magicFingerActive){
        if(!m_magicBeamStarted && (SDL_GetTicks() - m_magicPressStartTime) >= MAGIC_HOLD_THRESHOLD_MS){
            m_magicBeamStarted = true;   // held past the threshold -> it's a beam, not a tap
        }
        if(m_magicBeamStarted){
            InputDispatcher::getInstance().setBeaming(true);
        }
    }
    else{
        InputDispatcher::getInstance().setBeaming(false);
    }


    if(m_dFingerActive){

        if(m_TouchX > m_LeftButton.x && m_TouchX < m_LeftButton.x + m_LeftButton.w &&
           m_TouchY > m_LeftButton.y && m_TouchY < m_LeftButton.y + m_LeftButton.h){

            InputDispatcher::getInstance().setMovingLeft(true);
            InputDispatcher::getInstance().setMovingRight(false);

        }
        if(m_TouchX > m_RightButton.x && m_TouchX < m_RightButton.x + m_RightButton.w &&
           m_TouchY > m_RightButton.y && m_TouchY < m_RightButton.y + m_RightButton.h){

            InputDispatcher::getInstance().setMovingRight(true);
            InputDispatcher::getInstance().setMovingLeft(false);

        }


    }
    else{
        InputDispatcher::getInstance().setMovingLeft(false);
        InputDispatcher::getInstance().setMovingRight(false);
    }

    if(InputDispatcher::getInstance().released){
        InputDispatcher::getInstance().inputLogClear();
    }

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
            InputDispatcher::getInstance().setJump(true);   // latch the press now: a tap shorter than one frame must not be lost
        }
        if(touchX > m_UpButton.x && touchX < m_UpButton.x + m_UpButton.w &&
           touchY > m_UpButton.y && touchY < m_UpButton.y + m_UpButton.h && !m_upFingerActive){
            m_upFingerID = event.tfinger.fingerID;
            m_upFingerActive = true;
            m_upPressed = true;                  // the press itself still interacts; holding it aims up
        }
        if(touchX > m_DownButton.x && touchX < m_DownButton.x + m_DownButton.w &&
           touchY > m_DownButton.y && touchY < m_DownButton.y + m_DownButton.h && !m_downFingerActive){
            m_downFingerID = event.tfinger.fingerID;
            m_downFingerActive = true;
        }
        if(touchX > m_DashButton.x && touchX < m_DashButton.x + m_DashButton.w &&
           touchY > m_DashButton.y && touchY < m_DashButton.y + m_DashButton.h){
            InputDispatcher::getInstance().triggerDash();   // one-shot, fired from the event so a fast tap isn't lost
        }
        if((touchX > m_AttackButton.x && touchX < m_AttackButton.x + m_AttackButton.w&&
            touchY > m_AttackButton.y && touchY < m_AttackButton.y + m_AttackButton.h )&& !m_attackFingerActive){

            m_attackFingerID = event.tfinger.fingerID;
            InputDispatcher::getInstance().triggerAttack();   // fire from the event itself so a fast tap can't be missed by update()
        }
        if((touchX > m_MagicButton.x && touchX < m_MagicButton.x + m_MagicButton.w&&
            touchY > m_MagicButton.y && touchY < m_MagicButton.y + m_MagicButton.h )&& !m_magicFingerActive){

            m_magicFingerID = event.tfinger.fingerID;
            m_magicFingerActive =true;
            m_magicPressStartTime = SDL_GetTicks();
            m_magicBeamStarted = false;
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
        if(m_jumpFingerActive && event.tfinger.fingerID == m_jumpFingerID){
            m_jumpFingerActive =false;
            InputDispatcher::getInstance().setJump(false);
        }
        if(m_dFingerActive && event.tfinger.fingerID == m_dFingerID){
            m_dFingerActive =false;
        }
        if(m_upFingerActive && event.tfinger.fingerID == m_upFingerID){
            m_upFingerActive =false;
        }
        if(m_downFingerActive && event.tfinger.fingerID == m_downFingerID){
            m_downFingerActive =false;
        }
        if(m_attackFingerActive && event.tfinger.fingerID == m_attackFingerID){
            m_attackFingerActive =false;
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


ButtonOverlay::ButtonOverlay(SDL_Renderer *renderer) {
    this->Name = "ButtonState";

    LOGI("Button Overlay Constructor : %p",this);
    m_jumpButtonTexture =Engine::Get().getAssetManager().getTexture(TextureType::BUTTON_JUMP_BUTTON);
    m_leftButtonTexture =Engine::Get().getAssetManager().getTexture(TextureType::BUTTON_LEFT_BUTTON);
    m_rightButtonTexture =Engine::Get().getAssetManager().getTexture(TextureType::BUTTON_RIGHT_BUTTON);
    m_upButtonTexture =Engine::Get().getAssetManager().getTexture(TextureType::BUTTON_UP_BUTTON);
    m_downButtonTexture =Engine::Get().getAssetManager().getTexture(TextureType::BUTTON_DOWN_BUTTON);
    m_slashButtonTexture =Engine::Get().getAssetManager().getTexture(TextureType::BUTTON_SLASH_BUTTON);
    m_magicButtonTexture =Engine::Get().getAssetManager().getTexture(TextureType::BUTTON_MAGIC_BUTTON);
}

ButtonOverlay::~ButtonOverlay(){
    LOGI("Button Overlay Destructor : %p",this);

}