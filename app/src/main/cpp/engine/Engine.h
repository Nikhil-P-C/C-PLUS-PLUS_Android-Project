//
// Created by LENOVO on 17-04-2026.
//
#include "State/state.h"
#include "States/MenuState.h"
#include "States/GameState/GameState.h"
#include <SDL3/SDL.h>
#include <android/log.h>

#define LOG_TAG "Engine"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#pragma once//MY_APP_ENGINE_H
class Engine{

public:
    Engine(){
        LOGI("engine constructor");
        initEngine();
    }
    ~Engine(){
        delete CurrentState;
    }
    void initEngine(){
        LOGI("initEngine");
        if (SDL_Init(SDL_INIT_VIDEO) != 0)
            LOGI("failed SDL:%s", SDL_GetError());
        if (TTF_Init() != 0)
            LOGI("failed ttf:%s", SDL_GetError());
        if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG)
            LOGI("failed img:%s", SDL_GetError());

        window = SDL_CreateWindow("SDL3 Android App", 0, 0, SDL_WINDOW_FULLSCREEN);
        if (!window) {
            LOGE("Window creation failed: %s", SDL_GetError());
        }

        renderer = SDL_CreateRenderer(window, "opengles2");
        if (!renderer) {
            LOGE("Renderer creation failed: %s", SDL_GetError());
        }

        const char* name = SDL_GetRendererName(renderer);
        LOGI("Renderer: %s", name);
        CurrentState = new MenuState();
    }
    void run(){

        while(true){

        CurrentState->handleEvents();

        CurrentState->update(0.0f);

        CurrentState->render(renderer);
        }
    }

private:
    State* CurrentState= nullptr;
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Event event;
};
