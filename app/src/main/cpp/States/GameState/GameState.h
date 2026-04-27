//
// Created by LENOVO on 17-04-2026.
//

#pragma once
#include <SDL3/SDL.h>
#include <android/log.h>
#include "State/State.h"
#include "Player.h"
#include "SDL3_ttf/SDL_ttf.h"
#include "platform.h"


#define LOG_TAG "GameState"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

struct Animation{
    int startIndex=0;
    int lastIndex=0;
};

class GameState : public State{
public:
    GameState(){
        LOGI("game state constructor");
    }
    void render(SDL_Renderer* renderer) override;

    void update(float dt) override;

    void handleEvents() override;

    ~GameState() override{
        LOGI("destructor");
    }
private:
    //mechanics
    bool isCompleted=false;
    //animation and texture
    Animation Animation;
    SDL_IOStream* playerSprite     = SDL_IOFromFile("sheets/DinoSprites - vita.png", "rb");
    SDL_IOStream* tilesetSprite    = SDL_IOFromFile("Platforms/Terrain.png", "rb");
    SDL_IOStream* backGroundSprite = SDL_IOFromFile("Background/orig.png", "rb");
    SDL_IOStream* fontFile         = SDL_IOFromFile("Fonts/PlayfulTime.ttf","rb");
    TTF_Font*    font       = nullptr;
    SDL_Texture* texture    = nullptr;
    SDL_Texture* tileset    = nullptr;
    SDL_Texture* backGround = nullptr;
    SDL_Texture* Etexture = nullptr;
    SDL_Texture* fpstexture = nullptr;
    //sprites
    static const int SPRITE_HEIGHT =24;
    static const int SPRITE_WIDTH =24;
    static const int TILE_HEIGHT =16;
    static const int TILE_WIDTH =16;
    //player and platform
    Platform platforms[100];
    Player player;
    //scaling and window and input
    constexpr static const float P_scale =5.0f;
    int windowH =0,windowW=0;
    bool running =true;
    //player init pos
    float x = 0,y=0;
};

