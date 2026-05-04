//
// Created by LENOVO on 17-04-2026.
//

#pragma once
#include <android/log.h>
#include "SDL3/SDL.h"
#include "State/State.h"

#define LOG_TAG "PauseState"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
class PauseState : public State{
public:
    PauseState(SDL_Renderer* renderer);
    void init(SDL_Renderer* renderer);
    bool shouldClose(float x ,float y) const;
    void render(SDL_Renderer* renderer)override;
    void update(float dt)override;
    void handleEvents(SDL_Event& event)override;
    ~PauseState() override;
private:
    SDL_FRect m_exit = {800-100, 600 - 50, 200, 100};
    SDL_IOStream* m_fontfile       = SDL_IOFromFile("Fonts/Pixel Game.otf","rb");
    SDL_IOStream* m_fontShadowfile = SDL_IOFromFile("Fonts/Pixel Game Extrude.otf","rb");

    TTF_Font* m_font       = nullptr;
    TTF_Font* m_fontShadow = nullptr;

    SDL_Texture* m_fontTexture       = nullptr;
    SDL_Texture* m_fontShadowTexture = nullptr;

    SDL_Renderer* m_renderer = nullptr;
};
