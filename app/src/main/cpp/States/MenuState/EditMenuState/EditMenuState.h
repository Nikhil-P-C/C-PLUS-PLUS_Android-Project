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
    Button m_backButton{75.00f,25.00f,100.00f,50.00f};

    Button m_yellowSkinButton{100.00f,100.00f,200.00f,300.00f};
    Button m_blueSkinButton{325.00f,100.00f,200.00f,300.00f};
    Button m_redSkinButton{550.00f,100.00f,200.00f,300.00f};
    Button m_greenSkinButton{775.00f,100.00f,200.00f,300.00f};

    SDL_FRect m_textbox{1057.00f,440.00f,462.00f,143.00f};
    SDL_FRect m_saveNameButton{1180.00f,600.00f,216.00f,77.00f};

    SDL_IOStream* m_fontfile = SDL_IOFromFile("Fonts/Pixel Game.otf","rb");

    SDL_Texture* m_yellowSkinTexture =nullptr;
    SDL_Texture* m_blueSkinTexture =nullptr;
    SDL_Texture* m_redSkinTexture =nullptr;
    SDL_Texture* m_greenSkinTexture =nullptr;
    SDL_Texture* m_backgroundTexture = nullptr;
    SDL_Texture* m_editMenuTexture = nullptr;
    SDL_Texture* m_nameTileTexture = nullptr;
    SDL_Texture* m_skinTileTexture = nullptr;
    SDL_Texture* m_previewTileTexture = nullptr;
    SDL_Texture* m_backButtonTexture = nullptr;
    SDL_Texture* m_leftChevronTexture = nullptr;
    SDL_Texture* m_previewTexture = nullptr;

    PlayerSkin m_playerSkin =PlayerDetail::getInstance().getPlayerSkin();
    TTF_Font* m_font = nullptr;

    bool m_saveButtonActive =true;
    bool m_textboxActive = false;
    std::string m_editName = PlayerDetail::getInstance().getPlayerName();
    unsigned int m_currentFrame=0;
    unsigned int m_startFrame =0;
    unsigned int m_endFrame = 4;
    unsigned int m_lastTime =0;
    unsigned int m_nowTime =0;
    int m_animationDelay =100;
    SDL_Renderer* m_renderer= nullptr;
};
