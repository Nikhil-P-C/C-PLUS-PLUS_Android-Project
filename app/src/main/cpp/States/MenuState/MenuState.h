//
// Created by LENOVO on 17-04-2026.
//

#pragma once
#include "State/State.h"
#include <android/log.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#define LOG_TAG "MenuState"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
class MenuState : public State{
public:

    MenuState(SDL_Renderer* renderer){
        LOGI("menu state constructor");
        if(backGroundSprite)LOGI("backGroundSprite:true");
        Backgroundsurface = IMG_Load_IO(backGroundSprite,false);
        if(!Backgroundsurface)return;
        SDL_RenderClear(renderer);
        backGround = SDL_CreateTextureFromSurface(renderer,Backgroundsurface);
        SDL_DestroySurface(Backgroundsurface);
        if(!backGround)return;
        SDL_SetTextureScaleMode(backGround,SDL_SCALEMODE_NEAREST);
    }
    ~MenuState() override{
        LOGI("destructor");
        if (backGroundSprite)SDL_CloseIO(backGroundSprite);
    }
    void render(SDL_Renderer* renderer)override;
    void update(float dt)override;
    void handleEvents(SDL_Event& event)override;

private:
    SDL_IOStream* fontFile         = SDL_IOFromFile("Fonts/PlayfulTime.ttf","rb");
    SDL_IOStream* backGroundSprite = SDL_IOFromFile("dinoCharacters-display.gif", "rb");
    SDL_Texture* backGround = nullptr;
    SDL_Surface* Backgroundsurface= nullptr;
};

