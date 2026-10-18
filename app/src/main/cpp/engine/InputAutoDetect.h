//
// Input device auto-detection: decides which control scheme matches the device the player is
// actually using (touch / keyboard + mouse / gamepad). Header-only, no build changes needed.
//
#pragma once
#include <SDL3/SDL.h>
#include <cstdlib>
#include "utils/GameData.h"

namespace InputAutoDetect {

    enum class Device { NONE, TOUCH, KEYBOARD_MOUSE, GAMEPAD };

    // remembers which touch scheme (BUTTONS or JOYSTICK + BUTTON) the player last used, so going
    // keyboard -> touch again brings back the one they picked instead of always BUTTONS
    inline ControlType& lastTouchScheme(){
        static ControlType t = ControlType::BUTTONS;
        return t;
    }
    inline bool isTouchScheme(ControlType t){
        return t == ControlType::BUTTONS || t == ControlType::SEP_JUMP_W_JOYSTICK;
    }

    // keys that don't mean "the player is on a keyboard" (Android system keys also arrive as key events)
    inline bool isSystemKey(SDL_Keycode k){
        return k == SDLK_AC_BACK || k == SDLK_MENU || k == SDLK_VOLUMEUP || k == SDLK_VOLUMEDOWN ||
               k == SDLK_MUTE || k == SDLK_POWER || k == SDLK_UNKNOWN;
    }

    // Which device produced this event? NONE for anything that isn't a clear, deliberate input.
    // SDL fakes mouse events for touches (and touch events for the mouse), those are filtered out so a
    // finger tap is never mistaken for a mouse click.
    inline Device detect(const SDL_Event& e){
        switch(e.type){
            case SDL_EVENT_FINGER_DOWN:
                if(e.tfinger.touchID == SDL_MOUSE_TOUCHID || e.tfinger.touchID == SDL_PEN_TOUCHID) return Device::NONE;
                return Device::TOUCH;

            case SDL_EVENT_KEY_DOWN:
                if(e.key.repeat || isSystemKey(e.key.key)) return Device::NONE;
                return Device::KEYBOARD_MOUSE;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if(e.button.which == SDL_TOUCH_MOUSEID || e.button.which == SDL_PEN_MOUSEID) return Device::NONE;
                return Device::KEYBOARD_MOUSE;
            case SDL_EVENT_MOUSE_MOTION:
                if(e.motion.which == SDL_TOUCH_MOUSEID || e.motion.which == SDL_PEN_MOUSEID) return Device::NONE;
                return Device::KEYBOARD_MOUSE;
            case SDL_EVENT_MOUSE_WHEEL:
                if(e.wheel.which == SDL_TOUCH_MOUSEID || e.wheel.which == SDL_PEN_MOUSEID) return Device::NONE;
                return Device::KEYBOARD_MOUSE;

            case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
                return Device::GAMEPAD;
            case SDL_EVENT_GAMEPAD_AXIS_MOTION:
                // big deflection only: resting sticks drift and would otherwise flip the scheme by themselves
                return (std::abs((int)e.gaxis.value) > 20000) ? Device::GAMEPAD : Device::NONE;

            default:
                return Device::NONE;
        }
    }

    // the control scheme to use once `dev` was seen while `current` was active
    inline ControlType controlFor(Device dev, ControlType current){
        if(isTouchScheme(current)) lastTouchScheme() = current;
        switch(dev){
            case Device::TOUCH:          return isTouchScheme(current) ? current : lastTouchScheme();
            case Device::KEYBOARD_MOUSE: return ControlType::KEYBOARD;
            case Device::GAMEPAD:        return ControlType::GAMEPAD;
            default:                     return current;
        }
    }

    // best guess before the player has touched anything (start-up, or the gamepad was just unplugged);
    // the first real input event then corrects it
    inline ControlType guessInitial(bool allowGamepad = true){
        if(allowGamepad && SDL_HasGamepad()) return ControlType::GAMEPAD;
#if defined(__ANDROID__)
        return lastTouchScheme();
#else
        return ControlType::KEYBOARD;
#endif
    }
}
