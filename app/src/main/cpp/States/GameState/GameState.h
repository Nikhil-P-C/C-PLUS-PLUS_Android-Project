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
#include "SDL3_image/SDL_image.h"


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
        LOGI("game state constructor");
        //font loading
        font = TTF_OpenFontIO(fontFile,false,36);

//    if(!font)return;
        LOGI("font:%d",(bool)font);
        //background sprite loading;
        SDL_Surface* Backgroundsurface = IMG_Load_IO(backGroundSprite,true);
        if(!Backgroundsurface)return;
        backGround = SDL_CreateTextureFromSurface(renderer,Backgroundsurface);
        if(!backGround)return;
        SDL_DestroySurface(Backgroundsurface);

        //player sprite loading
        SDL_Surface* playerSurface = IMG_Load_IO(playerSprite,true);
        if(!playerSurface)return;
        playerTexture = SDL_CreateTextureFromSurface(renderer,playerSurface);
        if(!playerTexture)return;
        SDL_DestroySurface(playerSurface);


        // tileset loading
        SDL_Surface* Tilesurface = IMG_Load_IO(tilesetSprite,true);
        if(!Tilesurface)return;
        tileset = SDL_CreateTextureFromSurface(renderer,Tilesurface);
        if(!tileset)return;
        SDL_DestroySurface(Tilesurface);

        SDL_SetTextureScaleMode(playerTexture,SDL_SCALEMODE_NEAREST);
        SDL_SetTextureScaleMode(tileset,SDL_SCALEMODE_NEAREST);
        SDL_SetTextureScaleMode(fpsTexture,SDL_SCALEMODE_NEAREST);
        if(!playerSprite)LOGE("cannot load sprite");

    }
    ~GameState() override{
        LOGI("destructor");
        if (playerTexture) SDL_DestroyTexture(playerTexture);
        if (tileset) SDL_DestroyTexture(tileset);
        if (fpsTexture)SDL_DestroyTexture(fpsTexture);
        if (endTexture)SDL_DestroyTexture(endTexture);
        if (playerSprite) SDL_CloseIO(playerSprite);
        if (tilesetSprite) SDL_CloseIO(tilesetSprite);
        if (backGroundSprite)SDL_CloseIO(backGroundSprite);
    }

    void render(SDL_Renderer* renderer) override;

    void update(float dt) override;

    void handleEvents(SDL_Event& event) override;


private:
    //mechanics
    bool isCompleted=false;
    //animation and texture
    Animation Animation;
    int current_Frame =0;
    int Aniframe_delay =50;//delayed by 50ms

    SDL_IOStream* playerSprite     = SDL_IOFromFile("sheets/DinoSprites - vita.png", "rb");
    SDL_IOStream* tilesetSprite    = SDL_IOFromFile("Platforms/Terrain.png", "rb");
    SDL_IOStream* backGroundSprite = SDL_IOFromFile("Background/orig.png", "rb");
    SDL_IOStream* fontFile         = SDL_IOFromFile("Fonts/PlayfulTime.ttf","rb");
    TTF_Font*    font       = nullptr;
    SDL_Texture* playerTexture    = nullptr;
    SDL_Texture* tileset    = nullptr;
    SDL_Texture* backGround = nullptr;
    SDL_Texture* endTexture = nullptr;
    SDL_Texture* fpsTexture = nullptr;
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

