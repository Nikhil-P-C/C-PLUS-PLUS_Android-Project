//
// Created by LENOVO on 08-08-2026.
//
//
// Cross-platform pointer helpers: treats a finger touch (Android) and a left
// mouse click (Windows/desktop) as the same logical "pointer" input so menu
// code only has to be written once.
//
#pragma once
#include <SDL3/SDL.h>
#include "GameData.h"

namespace InputUtils {

    // SDL synthesizes a mouse event for every real touch event (so mouse-only
    // code still works on touchscreens) and vice versa on some backends.
    // Since IsPointerDown/Up/Motion below treat touch and mouse as the same
    // "pointer", a real finger tap would otherwise also pass its synthetic
    // mouse twin, firing button logic twice for one physical tap.
    inline bool IsSyntheticMouseEvent(const SDL_Event &event) {
        return event.button.which == SDL_TOUCH_MOUSEID;
    }
    inline bool IsSyntheticTouchEvent(const SDL_Event &event) {
        return event.tfinger.touchID == SDL_MOUSE_TOUCHID;
    }

    inline bool IsPointerDown(const SDL_Event &event) {
        if (event.type == SDL_EVENT_FINGER_DOWN) return !IsSyntheticTouchEvent(event);
        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT)
            return !IsSyntheticMouseEvent(event);
        return false;
    }

    inline bool IsPointerUp(const SDL_Event &event) {
        if (event.type == SDL_EVENT_FINGER_UP) return !IsSyntheticTouchEvent(event);
        if (event.type == SDL_EVENT_MOUSE_BUTTON_UP && event.button.button == SDL_BUTTON_LEFT)
            return !IsSyntheticMouseEvent(event);
        return false;
    }

    inline bool IsPointerMotion(const SDL_Event &event) {
        if (event.type == SDL_EVENT_FINGER_MOTION) return !IsSyntheticTouchEvent(event);
        if (event.type == SDL_EVENT_MOUSE_MOTION) return !IsSyntheticMouseEvent(event);
        return false;
    }

    /*
        Is the "back"/"cancel" key on this platform? Android hardware back
        button on device, Escape on a desktop keyboard.
    */
    inline bool IsBackKey(const SDL_Event &event) {
//        if (event.type != SDL_EVENT_KEY_DOWN) return false;
        switch(event.type){
            case SDL_EVENT_KEY_DOWN:
                return event.key.key == SDLK_AC_BACK || event.key.key == SDLK_ESCAPE;
            case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
                return event.gbutton.button == SDL_GAMEPAD_BUTTON_START;
            default:
                return false;
        }
//        return event.key.key == SDLK_AC_BACK || event.key.key == SDLK_ESCAPE || event.gbutton.button == SDL_GAMEPAD_BUTTON_START;
    }
    /*
        Fills outX/outY with the pointer's position in the game's fixed
        logical coordinate space (1600x720), regardless of whether it came
        from a normalized finger touch or a mouse click in real window pixels.
        Only call this once IsPointerDown/IsPointerUp/IsPointerMotion returned
        true for the same event.
    */
    inline void GetPointerPosition(const SDL_Event &event, SDL_Renderer *renderer, float &outX, float &outY) {
        if (event.type == SDL_EVENT_FINGER_DOWN || event.type == SDL_EVENT_FINGER_UP || event.type == SDL_EVENT_FINGER_MOTION) {
            outX = event.tfinger.x * (float) GameData::getInstance().getWinWidth();
            outY = event.tfinger.y * (float) GameData::getInstance().getWinHeight();
            return;
        }
        /*
            Mouse events carry real window-pixel coordinates, so they have to be
            converted through the renderer's logical presentation (letterboxing,
            DPI/window scale) to land in the same 1600x720 space touch events use.
        */
        float winX = 0.0f, winY = 0.0f;
        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            winX = event.button.x;
            winY = event.button.y;
        } else if (event.type == SDL_EVENT_MOUSE_MOTION) {
            winX = event.motion.x;
            winY = event.motion.y;
        }
        if (renderer) {
            SDL_RenderCoordinatesFromWindow(renderer, winX, winY, &outX, &outY);
        } else {
            outX = winX;
            outY = winY;
        }
    }
}