//
// Created by LENOVO on 11-08-2026.
//
#include "utils/utils.h"
#include "GamepadOverlay.h"
#include <cmath>
void GamepadOverlay::render(SDL_Renderer *renderer) {

}

void GamepadOverlay::update(float dt) {
    float deadzone = 0.25f;
    if(m_magicButtonActive){
        if (!m_magicBeamStarted &&
            (SDL_GetTicks() - m_magicPressStartTime) >= MAGIC_HOLD_THRESHOLD_MS) {
            m_magicBeamStarted = true;
        }
        if (m_magicBeamStarted) {
            InputDispatcher::getInstance().setBeaming(true);
        }
    }
    // stick up (SDL: up is negative Y) = interact. Fires once per push; the stick has to return before it re-arms,
    // so holding it up doesn't skip through every page of a dialog.
    if(m_leftStickY < -0.6f && std::fabs(m_leftStickY) > std::fabs(m_leftStickX)){
        if(!m_stickUpLatched){
            InputDispatcher::getInstance().triggerInteract();
            m_stickUpLatched = true;
        }
    }
    else if(m_leftStickY > -0.3f){
        m_stickUpLatched = false;
    }
    // stick held up / down aims the slash (stick up = upward slash, stick down in the air = downward slash / pogo)
    {
        const bool vertical = std::fabs(m_leftStickY) > std::fabs(m_leftStickX);
        InputDispatcher::getInstance().setLookUp((m_leftStickY < -0.5f && vertical) || m_dpadUp);
        InputDispatcher::getInstance().setLookDown((m_leftStickY > 0.5f && vertical) || m_dpadDown);
    }
    // d-pad works exactly like the stick for left / right
    const bool goRight = m_leftStickX >  deadzone || m_dpadRight;
    const bool goLeft  = m_leftStickX < -deadzone || m_dpadLeft;
    if(goRight && !goLeft){
        InputDispatcher::getInstance().setMovingRight(true);
        InputDispatcher::getInstance().setMovingLeft(false);
    }
    else if(goLeft && !goRight){
        InputDispatcher::getInstance().setMovingRight(false);
        InputDispatcher::getInstance().setMovingLeft(true);
    }
    else{
        InputDispatcher::getInstance().setMovingRight(false);
        InputDispatcher::getInstance().setMovingLeft(false);
    }
}

bool GamepadOverlay::handleEvents(SDL_Event &event) {

    // (opening / closing gamepads is done centrally in Engine::handleGamepadHotplug, so it also works in the menus)
    if(event.type == SDL_EVENT_GAMEPAD_REMOVED){
        // unplugged mid-game: drop the held stick / d-pad so the player doesn't keep running
        m_leftStickX = m_leftStickY = 0.0f;
        m_dpadLeft = m_dpadRight = m_dpadUp = m_dpadDown = false;
        InputDispatcher::getInstance().inputLogClear();
    }

    switch(event.type){
        case SDL_EVENT_GAMEPAD_AXIS_MOTION:
            if(event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTX){
                float x  =event.gaxis.value / 32767.0f;
                m_leftStickX =x;
            }
            if(event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTY){

                float y  = event.gaxis.value / 32767.0f;   // float division: the old int division only ever gave -1/0/1
                m_leftStickY =y;
            }
            break;
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
            if(event.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_LEFT)  { m_dpadLeft  = true; return true; }
            if(event.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_RIGHT) { m_dpadRight = true; return true; }
            if(event.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_DOWN)  { m_dpadDown  = true; return true; }
            if(event.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_UP)    { m_dpadUp    = true; InputDispatcher::getInstance().triggerInteract(); return true; }
            if(event.gbutton.button == SDL_GAMEPAD_BUTTON_SOUTH){
                InputDispatcher::getInstance().setJump(true);
                return true;
            }
            if(event.gbutton.button == SDL_GAMEPAD_BUTTON_WEST){
                InputDispatcher::getInstance().triggerAttack();
            }
            if(event.gbutton.button == SDL_GAMEPAD_BUTTON_NORTH){
                InputDispatcher::getInstance().triggerInteract();
            }
            if(event.gbutton.button == SDL_GAMEPAD_BUTTON_EAST){
                m_magicButtonActive =true;
                m_magicPressStartTime = SDL_GetTicks();
                m_magicBeamStarted =false;
            }
            break;

        case SDL_EVENT_GAMEPAD_BUTTON_UP:

            if(event.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_LEFT)  { m_dpadLeft  = false; return true; }
            if(event.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_RIGHT) { m_dpadRight = false; return true; }
            if(event.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_DOWN)  { m_dpadDown  = false; return true; }
            if(event.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_UP)    { m_dpadUp    = false; return true; }
            if(event.gbutton.button == SDL_GAMEPAD_BUTTON_SOUTH){
                InputDispatcher::getInstance().setJump(false);
            }
            if(event.gbutton.button == SDL_GAMEPAD_BUTTON_EAST){
                m_magicButtonActive =false;
                if(!m_magicBeamStarted){
                    InputDispatcher::getInstance().triggerHeal();
                }
                InputDispatcher::getInstance().setBeaming(false);
                m_magicBeamStarted =false;
            }
    }
    return false;
}

GamepadOverlay::GamepadOverlay(SDL_Renderer *renderer) {
    this->Name = "GamepadState";

    m_renderer=renderer;
    LOGI("Gamepad overlay constructor:%p",this);

}

GamepadOverlay::~GamepadOverlay() {
    LOGI("Gamepad overlay destructor:%p",this);
}

