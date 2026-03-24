//
// Created by LENOVO on 24-03-2026.
//
#pragma once

#include "gameUtils.h"
#include "SDL3/SDL.h"
#include <android/log.h>
#define LOG_TAG "CAMERA"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
class Camera{
private:
    Camera() = default;
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;
public:

    void setSize(float width, float height) {
        camera.w = width;
        camera.h = height;

    }
    void lockCameraOn(float& targetX,float& targetY,float targetH,float targetW) {
        float centerX = targetX + (targetW / 2) -camera.w/2;
        float centerY = targetY + (targetH / 2) -camera.h/2;
//        LOGI("centerX:%f,centerY:%f",centerX,centerY);

        gameUtilities::interpolate(camera,centerX,centerY,0.2f);
//        LOGI("cX:%f,c:%f",camera.x,camera.y);
    }
    static Camera& getInstance()
    {
        static Camera instance;   // created only once
        return instance;
    }
    SDL_FRect getCamera() const{
        return camera;
    }
private:
    SDL_FRect camera{0.0f,0.0f,0.0f,0.0f};
};

