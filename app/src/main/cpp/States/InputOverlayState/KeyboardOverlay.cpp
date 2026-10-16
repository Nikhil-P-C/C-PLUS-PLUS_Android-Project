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
    // window lost focus (alt-tab, notification shade...): the key-up never arrives, so release everything
    // here or the player keeps running / aiming on its own
    if(event.type == SDL_EVENT_WINDOW_FOCUS_LOST){
        InputDispatcher::getInstance().inputLogClear();
        m_magicKeyActive = false;
        m_magicBeamStarted = false;
        return false;
    }
    // mouse: left click = slash (real mouse only, touch-generated clicks are ignored). Not consumed, so clicking an
    // interactable still works.
    if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT &&
       event.button.which != SDL_TOUCH_MOUSEID && event.button.which != SDL_PEN_MOUSEID){
        InputDispatcher::getInstance().triggerAttack();
        return false;
    }
    // arrow keys work like WASD (A/D/W/S keep working)
    SDL_Keycode key = event.key.key;
    if(event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP){
        if(key == SDLK_LEFT)  key = SDLK_A;
        if(key == SDLK_RIGHT) key = SDLK_D;
        if(key == SDLK_UP)    key = SDLK_W;
        if(key == SDLK_DOWN)  key = SDLK_S;
    }

    if(event.type == SDL_EVENT_KEY_DOWN){
        if(key == SDLK_A){
            InputDispatcher::getInstance().setMovingLeft(true);
            return true;
        }
        if(key == SDLK_D){
            InputDispatcher::getInstance().setMovingRight(true);
            return true;
        }
        if(key == SDLK_SPACE){
            if(!event.key.repeat) InputDispatcher::getInstance().setJump(true);
            return true;
        }
        if(key == SDLK_E || key == SDLK_W){   // W = "up" = interact (E still works too)
            if(!event.key.repeat) InputDispatcher::getInstance().triggerInteract();
            // W is also held to aim up (K + W = upward slash)
            if(key == SDLK_W) InputDispatcher::getInstance().setLookUp(true);
            return true;
        }
        if(key == SDLK_S){                               // S = "down": K + S in the air = downward slash / pogo
            InputDispatcher::getInstance().setLookDown(true);
            return true;
        }
        if(key == SDLK_K){
            if(!event.key.repeat) InputDispatcher::getInstance().triggerAttack();   // holding K must not auto-swing (OS key repeat)
            return true;
        }
        if(key == SDLK_J){
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
        if(key == SDLK_A){
            InputDispatcher::getInstance().setMovingLeft(false);
            return true;
        }
        if(key == SDLK_D){
            InputDispatcher::getInstance().setMovingRight(false);
            return true;
        }
        if(key == SDLK_SPACE){
            InputDispatcher::getInstance().setJump(false);
            return true;
        }
        if(key == SDLK_W){
            InputDispatcher::getInstance().setLookUp(false);
            return true;
        }
        if(key == SDLK_S){
            InputDispatcher::getInstance().setLookDown(false);
            return true;
        }
        if(key == SDLK_J){
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