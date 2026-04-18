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
    void render(){
        LOGI("render");
    }
    void update(float dt){
        LOGI("update");
    }
    void handleEvents(){
        LOGI("handleEvents");
    }
    virtual ~PauseState(){
        LOGI("destructor");
    }
};
