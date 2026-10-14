//
// Created by LENOVO on 08-08-2026.
//
#include "KeyboardOverlay.h"
#include "utils/utils.h"
void KeyboardOverlay::render(SDL_Renderer *renderer) {
}

void KeyboardOverlay::update(float dt) {
    if(m_magicKeyActive){
        if (!m_magicBeamStarted &&
            (SDL_GetTicks() - m_magicPressStartTime) >= MAGIC_HOLD_THRESHOLD_MS) {
            m_magicBeamStarted = true;
        }
        if (m_magicBeamStarted) {
            InputDispatcher::getInstance().setBeaming(true);
        }
    }
}

bool KeyboardOverlay::handleEvents(SDL_Event &event) {
    if(event.type == SDL_EVENT_KEY_DOWN){
        if(event.key.key == SDLK_A){
            InputDispatcher::getInstance().setMovingLeft(true);
            return true;
        }
        if(event.key.key == SDLK_D){
            InputDispatcher::getInstance().setMovingRight(true);
            return true;
        }
        if(event.key.key == SDLK_SPACE){
            InputDispatcher::getInstance().setJump(true);
            return true;
        }
        if(event.key.key == SDLK_E || event.key.key == SDLK_W){   // W = "up" = interact (E still works too)
            if(!event.key.repeat) InputDispatcher::getInstance().triggerInteract();
            return true;
        }
        if(event.key.key == SDLK_K){
            InputDispatcher::getInstance().triggerAttack();
            return true;
        }
        if(event.key.key == SDLK_J){
            // Holding a key makes the OS send repeated KEY_DOWN events. Without this guard every
            // repeat re-armed the "tap" state (beamStarted=false, new press time), so releasing
            // shortly after a repeat looked like a tap and triggered a heal right after the beam.
            if(event.key.repeat) return true;
            m_magicKeyActive =true;
            m_magicBeamStarted = false;
            m_magicPressStartTime = SDL_GetTicks();
            return true;
        }
    }
    else if(event.type == SDL_EVENT_KEY_UP){
        if(event.key.key == SDLK_A){
            InputDispatcher::getInstance().setMovingLeft(false);
            return true;
        }
        if(event.key.key == SDLK_D){
            InputDispatcher::getInstance().setMovingRight(false);
            return true;
        }
        if(event.key.key == SDLK_SPACE){
            InputDispatcher::getInstance().setJump(false);
            return true;
        }
        if(event.key.key == SDLK_J){
            m_magicKeyActive =false;
            if(!m_magicBeamStarted){
                InputDispatcher::getInstance().triggerHeal();
            }
            InputDispatcher::getInstance().setBeaming(false);
            m_magicBeamStarted = false;
        }
    }

    return false;
}

KeyboardOverlay::KeyboardOverlay(SDL_Renderer *renderer) {
    this->Name = "KeyboardState";

    m_renderer=renderer;
    LOGI("keyboard overlay constructor:%p",this);

}

KeyboardOverlay::~KeyboardOverlay() {
    LOGI("keyboard overlay destructor:%p",this);
}