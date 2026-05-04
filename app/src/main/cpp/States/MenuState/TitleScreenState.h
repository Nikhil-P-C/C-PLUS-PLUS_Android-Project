//
// Created by LENOVO on 17-04-2026.
//

#pragma once
#include <android/log.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include "State/State.h"
#include "engine/Engine.h"


#define LOG_TAG "TitleScreenState"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
class TitleScreenState : public State{
public:

    TitleScreenState(SDL_Renderer* renderer);
    ~TitleScreenState() override;

    void render(SDL_Renderer* renderer)override;
    void update(float dt)override;
    void handleEvents(SDL_Event& event)override;

private:
    SDL_IOStream* m_fontFile         = SDL_IOFromFile("Fonts/PlayfulTime.ttf","rb");
    SDL_IOStream* m_backgroundSprite = SDL_IOFromFile("menu-animate.png","rb");

    SDL_Texture* m_background = nullptr;

    int m_currentFrame =0;
    unsigned int m_last=SDL_GetTicks();
    unsigned int m_now = 0;
    int m_aniDelay=50;//50ms
    SDL_Renderer* m_renderer = nullptr;
};

