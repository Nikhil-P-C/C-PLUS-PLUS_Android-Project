//
// Created by LENOVO on 21-07-2026.
//
#pragma once
#include "State/State.h"
#include "utils/utils.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#define LOG_TAG "EditMenuState"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

class EditMenuState :public State{
public:
    EditMenuState(SDL_Renderer* renderer);
    ~EditMenuState()override;
    void render(SDL_Renderer* renderer)override;
    void update(float dt)override;
    bool handleEvents(SDL_Event &event)override;
private:
    Button m_backButton{100,550.00f,100.00f,100.00f};
    SDL_IOStream* m_backgroundFile = SDL_IOFromFile("misc/bg.png","rb");
    SDL_IOStream* m_editMenuFile = SDL_IOFromFile("menu/editMenu.png","rb");
    SDL_IOStream* m_nameTileFile = SDL_IOFromFile("menu/nameTile.png","rb");
    SDL_IOStream* m_previewTileFile = SDL_IOFromFile("menu/previewTile.png","rb");
    SDL_IOStream* m_skinTileFile = SDL_IOFromFile("menu/skinTile.png","rb");
    SDL_IOStream* m_fontfile = SDL_IOFromFile("Fonts/Pixel Game.otf","rb");

    SDL_Texture* m_backgroundTexture = nullptr;
    SDL_Texture* m_editMenuTexture = nullptr;
    SDL_Texture* m_nameTileTexture = nullptr;
    SDL_Texture* m_skinTileTexture = nullptr;
    SDL_Texture* m_previewTileTexture = nullptr;

    TTF_Font* m_font = nullptr;

    bool m_saveButtonActive =true;

    SDL_Renderer* m_renderer= nullptr;
};
