//
// Created by LENOVO on 17-04-2026.
//

#pragma once
#include "State/state.h"
#include <android/log.h>

#define LOG_TAG "MenuState"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
class MenuState : public State{
public:
    void render(SDL_Renderer* renderer)override{

    }
    void update(float dt)override{
    }
    void handleEvents()override{
    }
    ~MenuState(){
        LOGI("destructor");
    }
};

