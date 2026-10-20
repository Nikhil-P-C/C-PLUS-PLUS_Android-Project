//
// Created by LENOVO on 03-05-2026.
//
#pragma once

#include "SDL3/SDL_rect.h"

class Camera{
private:
    Camera() = default;
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;
public:

    void setSize(float width, float height) {
        m_camera.w = width;
        m_camera.h = height;

    }
    void lockCameraOn(float& targetX,float& targetY,float targetH,float targetW);

    void cameraClamp(SDL_FRect rect);

    static Camera& getInstance()
    {
        static Camera instance;   // created only once
        return instance;
    }
    // What everything draws with: the follow position plus the current shake offset.
    // lockCameraOn()/cameraClamp() keep working on the unshaken m_camera, so shake never
    // drifts the camera and the HUD (which doesn't use the camera) stays steady.
    SDL_FRect getCamera() const{
        SDL_FRect c = m_camera;
        c.x += m_shakeX;
        c.y += m_shakeY;
        return c;
    }

    // --- arena lock (boss fights) -------------------------------------------
    // lockTo(x,y): stop following the player; glide to this top-left position and hold it there.
    // updateLock(dt) does the gliding (call it instead of lockCameraOn()/cameraClamp() while locked).
    void lockTo(float x, float y){ m_locked = true; m_lockX = x; m_lockY = y; }
    void unlock(){ m_locked = false; }
    bool isLocked() const { return m_locked; }
    void updateLock(float dt);
    // instantly put the camera at x,y (level start, respawn after a boss fight)
    void snapTo(float x, float y){ m_camera.x = x; m_camera.y = y; }

    // --- screen shake -------------------------------------------------------
    // trauma 0..1 (offset scales with trauma^2). Stacks, then decays on its own.
    void addTrauma(float t){ m_trauma = m_trauma + t > 1.0f ? 1.0f : m_trauma + t; }
    // holds trauma at AT LEAST `level` (continuous rumble, e.g. while channelling the beam)
    void rumble(float level){ if(m_trauma < level) m_trauma = level; }
    // one-shot horizontal push in px (sign = direction), decays in ~0.15s
    void kick(float px){ m_kickX = px; }
    // call once per frame with REAL dt (also during hit-stop)
    void updateShake(float dt);
    void resetShake(){ m_trauma = m_kickX = m_shakeX = m_shakeY = 0.0f; }
    // 0 disables shake, 1 = full strength (hook this to an options-menu slider)
    void setShakeScale(float s){ m_shakeScale = s < 0.0f ? 0.0f : s; }
private:
    SDL_FRect m_camera{0.0f,0.0f,0.0f,0.0f};
    float m_trauma = 0.0f;
    float m_kickX = 0.0f;
    float m_shakeX = 0.0f, m_shakeY = 0.0f;
    float m_shakeScale = 1.0f;
    bool  m_locked = false;
    float m_lockX = 0.0f, m_lockY = 0.0f;
};
