//
// Created by LENOVO on 03-05-2026.
//
#pragma once

#include "State/State.h"
#define LOG_TAG "DebugState"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

class DebugState : public State{
public:
    DebugState(SDL_Renderer* renderer);
    ~DebugState() override{
        if (m_font)            TTF_CloseFont(m_font);
        if (m_fpsTexture)      SDL_DestroyTexture(m_fpsTexture);
    }
    void render(SDL_Renderer* renderer) override;
    void update(float dt) override;
    void handleEvents(SDL_Event& event) override;
private:
    SDL_Texture*  m_fpsTexture       = nullptr;
    TTF_Font*     m_font             = nullptr;
    SDL_IOStream* m_fontFile         = SDL_IOFromFile("Fonts/Pixel Game.otf", "rb");
    int m_framedelay = 1000/60;
    unsigned int m_last = SDL_GetTicks();
    int m_fps =0;
    int m_frames =0;
};
