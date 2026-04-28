//
// Created by LENOVO on 17-04-2026.
//

#pragma once
#include "State/State.h"
#include <android/log.h>

#define LOG_TAG "PauseState"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
class PauseState : public State{
public:
    PauseState(){
        LOGI("pause state constructor");
    }
    void init(SDL_Renderer* renderer)override{
        LOGI("init");
    }
    void render(SDL_Renderer* renderer)override{
        LOGI("render");
    }
    void update(float dt)override{
        LOGI("update");
    }
    void handleEvents(SDL_Event& event)override{
        LOGI("handleEvents");
    }
    ~PauseState(){
        LOGI("destructor");
    }
};
