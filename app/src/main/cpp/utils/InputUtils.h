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

    /*
        Point-in-rect for the pointer helpers above (x/y from GetPointerPosition).
    */
    inline bool PointInRect(float px, float py, float x, float y, float w, float h) {
        return px >= x && px <= x + w && py >= y && py <= y + h;
    }

    /*
        "Press any key / tap / gamepad button" (title screen style).
    */
    inline bool IsAnyPress(const SDL_Event &event) {
        if (IsPointerDown(event)) return true;
        if (event.type == SDL_EVENT_KEY_DOWN) return !event.key.repeat;
        return event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN;
    }

    inline bool IsGamepadStart(const SDL_Event &event) {
        return event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN && event.gbutton.button == SDL_GAMEPAD_BUTTON_START;
    }

    // ------------------------------------------------------------------------------------
    // Menu navigation: one keyboard / gamepad mapping shared by every menu screen.
    //
    //   UP/DOWN/LEFT/RIGHT  arrows (W A S D too)           - keyboard auto-repeat gives hold-to-scroll
    //   CONFIRM             Enter / Space / Z / pad A (south)
    //   BACK                Esc / Android back / pad B (east) / pad Start
    //   TAB_PREV/TAB_NEXT   Q E / PageUp PageDown / pad shoulder buttons
    //
    // Gamepad dpad + left stick are NOT returned here: they go through MenuNavRepeater so a
    // held direction repeats at the same rate as a held key.
    // ------------------------------------------------------------------------------------
    enum class MenuAction { NONE, UP, DOWN, LEFT, RIGHT, CONFIRM, BACK, TAB_PREV, TAB_NEXT };

    // allowLetterKeys=false keeps only arrows / Enter / Esc so a text field can use the other keys for typing.
    inline MenuAction GetMenuAction(const SDL_Event &event, bool allowLetterKeys = true) {
        if (event.type == SDL_EVENT_KEY_DOWN) {
            const bool repeat = event.key.repeat;
            switch (event.key.key) {
                case SDLK_UP:     return MenuAction::UP;
                case SDLK_DOWN:   return MenuAction::DOWN;
                case SDLK_LEFT:   return MenuAction::LEFT;
                case SDLK_RIGHT:  return MenuAction::RIGHT;
                case SDLK_RETURN:
                case SDLK_RETURN2:
                case SDLK_KP_ENTER:  return repeat ? MenuAction::NONE : MenuAction::CONFIRM;
                case SDLK_ESCAPE:
                case SDLK_AC_BACK:   return repeat ? MenuAction::NONE : MenuAction::BACK;
                default: break;
            }
            if (!allowLetterKeys) return MenuAction::NONE;
            switch (event.key.key) {
                case SDLK_W:      return MenuAction::UP;
                case SDLK_S:      return MenuAction::DOWN;
                case SDLK_A:      return MenuAction::LEFT;
                case SDLK_D:      return MenuAction::RIGHT;
                case SDLK_SPACE:
                case SDLK_Z:      return repeat ? MenuAction::NONE : MenuAction::CONFIRM;
                case SDLK_Q:
                case SDLK_PAGEUP:   return repeat ? MenuAction::NONE : MenuAction::TAB_PREV;
                case SDLK_E:
                case SDLK_PAGEDOWN: return repeat ? MenuAction::NONE : MenuAction::TAB_NEXT;
                default: return MenuAction::NONE;
            }
        }
        if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
            switch (event.gbutton.button) {
                case SDL_GAMEPAD_BUTTON_SOUTH:          return MenuAction::CONFIRM;
                case SDL_GAMEPAD_BUTTON_EAST:
                case SDL_GAMEPAD_BUTTON_START:          return MenuAction::BACK;
                case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:  return MenuAction::TAB_PREV;
                case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER: return MenuAction::TAB_NEXT;
                default: return MenuAction::NONE;
            }
        }
        return MenuAction::NONE;
    }

    /*
        Held gamepad direction (dpad or left stick) -> repeated UP/DOWN/LEFT/RIGHT.
        Usage: call handleEvent() for every event (returns true if it was a dpad/stick event),
        and update(dt) once per frame; update() returns the action to perform this frame
        (fires immediately on press, then after firstDelay, then every repeatDelay).
    */
    class MenuNavRepeater {
    public:
        bool handleEvent(const SDL_Event &event) {
            if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN || event.type == SDL_EVENT_GAMEPAD_BUTTON_UP) {
                const bool down = event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN;
                switch (event.gbutton.button) {
                    case SDL_GAMEPAD_BUTTON_DPAD_UP:    m_up = down;    return true;
                    case SDL_GAMEPAD_BUTTON_DPAD_DOWN:  m_down = down;  return true;
                    case SDL_GAMEPAD_BUTTON_DPAD_LEFT:  m_left = down;  return true;
                    case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: m_right = down; return true;
                    default: return false;
                }
            }
            if (event.type == SDL_EVENT_GAMEPAD_AXIS_MOTION) {
                if (event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTY) { m_stickY = (float) event.gaxis.value / 32767.0f; return true; }
                if (event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTX) { m_stickX = (float) event.gaxis.value / 32767.0f; return true; }
            }
            return false;
        }

        MenuAction update(float dt) {
            const MenuAction dir = heldDirection();
            if (dir != m_held) {
                m_held = dir;
                m_timer = m_firstDelay;
                return dir;                       // NONE when released
            }
            if (dir == MenuAction::NONE) return MenuAction::NONE;
            m_timer -= dt;
            if (m_timer > 0.0f) return MenuAction::NONE;
            m_timer = m_repeatDelay;
            return dir;
        }

        void reset() { m_up = m_down = m_left = m_right = false; m_stickX = m_stickY = 0.0f; m_held = MenuAction::NONE; m_timer = 0.0f; }

    private:
        MenuAction heldDirection() const {
            if (m_up && !m_down) return MenuAction::UP;
            if (m_down && !m_up) return MenuAction::DOWN;
            if (m_stickY < -kStick) return MenuAction::UP;
            if (m_stickY >  kStick) return MenuAction::DOWN;
            if (m_left && !m_right) return MenuAction::LEFT;
            if (m_right && !m_left) return MenuAction::RIGHT;
            if (m_stickX < -kStick) return MenuAction::LEFT;
            if (m_stickX >  kStick) return MenuAction::RIGHT;
            return MenuAction::NONE;
        }
        static constexpr float kStick = 0.55f;
        float m_firstDelay = 0.38f, m_repeatDelay = 0.14f;
        bool m_up = false, m_down = false, m_left = false, m_right = false;
        float m_stickX = 0.0f, m_stickY = 0.0f;
        MenuAction m_held = MenuAction::NONE;
        float m_timer = 0.0f;
    };
}
