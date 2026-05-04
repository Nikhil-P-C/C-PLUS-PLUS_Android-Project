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
    static Camera& getInstance()
    {
        static Camera instance;   // created only once
        return instance;
    }
    SDL_FRect getCamera() const{
        return m_camera;
    }
private:
    SDL_FRect m_camera{0.0f,0.0f,0.0f,0.0f};
};
