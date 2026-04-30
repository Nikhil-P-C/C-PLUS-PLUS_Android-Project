//
// Created by LENOVO on 17-04-2026.
//

#pragma once
#include <SDL3/SDL.h>
#include <android/log.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>

#include "State/State.h"
#include "Player.h"
#include "platform.h"
#include "engine/Engine.h"

#define LOG_TAG "GameState"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

struct Animation{
    int startIndex=0;
    int lastIndex=0;
};

class GameState : public State{
public:
    GameState(SDL_Renderer* renderer){
        this->m_renderer = renderer;
        LOGI("game state constructor");
        const SDL_DisplayMode* mode =
                SDL_GetCurrentDisplayMode(SDL_GetPrimaryDisplay());
        if(mode)
        {
            LOGI("Display native: %d x %d", mode->w, mode->h);
            m_windowW =mode->w;
            m_windowH = mode->h;
        }
        //init player attributes
        m_player.setSize(SPRITE_WIDTH * P_scale, SPRITE_HEIGHT * P_scale);
        m_player.setPosition(100,0,m_windowH,SPRITE_HEIGHT,P_scale);

        //font loading
        m_font = TTF_OpenFontIO(m_fontFile, false, 36);
        LOGI("font:%d",(bool)m_font);

        //background sprite loading;
        SDL_Surface* Backgroundsurface = IMG_Load_IO(m_backGroundSprite, false);
        if(!Backgroundsurface)return;
        m_backGround = SDL_CreateTextureFromSurface(renderer, Backgroundsurface);
        if(!m_backGround)return;
        SDL_DestroySurface(Backgroundsurface);

        //player sprite loading
        SDL_Surface* playerSurface = IMG_Load_IO(m_playerSprite, false);
        if(!playerSurface)return;
        m_playerTexture = SDL_CreateTextureFromSurface(renderer, playerSurface);
        if(!m_playerTexture)return;
        SDL_DestroySurface(playerSurface);

        // m_tileset loading
        SDL_Surface* Tilesurface = IMG_Load_IO(m_tilesetSprite, false);
        if(!Tilesurface)return;
        m_tileset = SDL_CreateTextureFromSurface(renderer, Tilesurface);
        if(!m_tileset)return;
        SDL_DestroySurface(Tilesurface);

        SDL_SetTextureScaleMode(m_playerTexture, SDL_SCALEMODE_NEAREST);
        SDL_SetTextureScaleMode(m_tileset, SDL_SCALEMODE_NEAREST);
        SDL_SetTextureScaleMode(m_fpsTexture, SDL_SCALEMODE_NEAREST);

        if(!m_playerSprite)LOGE("cannot load sprite");

    }
    ~GameState() override{
        LOGI("destructor");
        if (m_font)            TTF_CloseFont(m_font);
        if (m_backGround)      SDL_DestroyTexture(m_backGround);
        if (m_playerTexture)   SDL_DestroyTexture(m_playerTexture);
        LOGI("destructor end-1");

        if (m_tileset)         SDL_DestroyTexture(m_tileset);
        if (m_fpsTexture)      SDL_DestroyTexture(m_fpsTexture);
        if (m_endTexture)      SDL_DestroyTexture(m_endTexture);
        LOGI("destructor end 0");

        if (m_playerSprite)    SDL_CloseIO(m_playerSprite);
        LOGI("destructor end 1");
        if (m_tilesetSprite)   SDL_CloseIO(m_tilesetSprite);
        LOGI("destructor end 2");

        if (m_backGroundSprite)SDL_CloseIO(m_backGroundSprite);
        LOGI("destructor end");
    }

    void render(SDL_Renderer* renderer) override;

    void update(float dt) override;

    void handleEvents(SDL_Event& event) override;


private:
    //mechanics
    bool m_isCompleted=false;
    //animation and texture
    Animation m_Animation;
    int m_currentFrame =0;
    int m_aniframeDelay =50;//delayed by 50ms

    SDL_IOStream* m_playerSprite     = SDL_IOFromFile("sheets/DinoSprites - vita.png", "rb");
    SDL_IOStream* m_tilesetSprite    = SDL_IOFromFile("Platforms/Terrain.png", "rb");
    SDL_IOStream* m_backGroundSprite = SDL_IOFromFile("Background/orig.png", "rb");
    SDL_IOStream* m_fontFile         = SDL_IOFromFile("Fonts/PlayfulTime.ttf", "rb");

    TTF_Font*    m_font              = nullptr;
    SDL_Texture* m_playerTexture     = nullptr;
    SDL_Texture* m_tileset           = nullptr;
    SDL_Texture* m_backGround        = nullptr;
    SDL_Texture* m_endTexture        = nullptr;
    SDL_Texture* m_fpsTexture        = nullptr;

    //sprites
    static const int SPRITE_HEIGHT =24;
    static const int SPRITE_WIDTH  =24;
    static const int TILE_HEIGHT   =16;
    static const int TILE_WIDTH    =16;

    //player and platform
    Platform m_platforms[100];
    Player m_player;

    //scaling and window and input
    constexpr static const float P_scale =5.0f;
    int m_windowH =0,m_windowW=0;
    bool m_running =true;

    SDL_Renderer* m_renderer = nullptr;
};

