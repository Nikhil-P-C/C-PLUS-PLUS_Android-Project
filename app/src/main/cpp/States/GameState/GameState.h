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
#include "utils/utils.h"

#define LOG_TAG "GameState"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

typedef struct Animation{
    int startIndex=0;
    int lastIndex=0;
}Animation;
enum PlayerAction{
    IDLE =0,
    MOVINGLEFT=1,
    MOVINGRIGHT=2,
    CROUCHING=3,
    HURT=4,
    JUMP=5,

};
class GameState : public State{
public:
    GameState(SDL_Renderer* renderer);
    ~GameState() override{
        if (m_font)            TTF_CloseFont(m_font);
        if (m_backGround)      SDL_DestroyTexture(m_backGround);
        if (m_playerTexture)   SDL_DestroyTexture(m_playerTexture);
        if (m_tileset)         SDL_DestroyTexture(m_tileset);
        if (m_endTexture)      SDL_DestroyTexture(m_endTexture);

        if (m_fontFile)        SDL_CloseIO(m_fontFile);
        if (m_playerSprite)    SDL_CloseIO(m_playerSprite);
        if (m_tilesetSprite)   SDL_CloseIO(m_tilesetSprite);
        if (m_backGroundSprite)SDL_CloseIO(m_backGroundSprite);
    }


    void render(SDL_Renderer* renderer) override;

    void update(float dt) override;

    void handleEvents(SDL_Event& event) override;


private:

    //animation and texture
    Animation m_Animation;
    PlayerAction m_playerAction = IDLE;
    int m_currentFrame=0;
    int m_aniframeDelay =50;//delayed by 50ms
    unsigned int m_aniNowTime =0;
    unsigned int m_aniLastTime =SDL_GetTicks();

    SDL_IOStream* m_playerSprite     = SDL_IOFromFile("sheets/DinoSprites - vita.png", "rb");
    SDL_IOStream* m_tilesetSprite    = SDL_IOFromFile("Platforms/Terrain.png", "rb");
    SDL_IOStream* m_backGroundSprite = SDL_IOFromFile("Background/orig.png", "rb");
    SDL_IOStream* m_fontFile         = SDL_IOFromFile("Fonts/PlayfulTime.ttf", "rb");


    SDL_Texture* m_playerTexture     = nullptr;
    SDL_Texture* m_tileset           = nullptr;
    SDL_Texture* m_backGround        = nullptr;
    SDL_Texture* m_endTexture        = nullptr;
    TTF_Font*    m_font              = nullptr;

    //sprites
    static const int SPRITE_HEIGHT =24;
    static const int SPRITE_WIDTH  =24;
    static const int TILE_HEIGHT   =16;
    static const int TILE_WIDTH    =16;
\
    //player and platform
    Platform m_platforms[100];
    Player   m_player;

    //scaling and window
    constexpr static const float P_scale =5.0f;
    int m_windowH =0,m_windowW=0;


    SDL_Renderer* m_renderer = nullptr;

    //mechanics
    bool m_running =true;
    bool m_isCompleted=false;

    //input
    Button m_JumpButton;
    Button m_LeftButton;
    Button m_RightButton;
    Button m_CrouchButton;

};

