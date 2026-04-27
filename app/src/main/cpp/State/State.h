//
// Created by LENOVO on 17-04-2026.
//
#include <android/log.h>
#include <SDL3/SDL.h>
#pragma once
#define LOG_TAG "State"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

class State{
public:
    State(){
        LOGI("state constructor");
    }
    virtual void render(SDL_Renderer* renderer) = 0;
    virtual void update(float dt) = 0;
    virtual void handleEvents() = 0;
    virtual ~State() =default;
};
 //MY_APP_STATE_H
