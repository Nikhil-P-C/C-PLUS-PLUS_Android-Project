//
// Created by LENOVO on 29-10-2025.
//
#pragma once

#define MAX_PLATFORM 100

#include "SDL3_image/SDL_image.h"
#include "SDL3/SDL.h"
#include "platform.h"
#include "Player.h"


struct Animation{
    int startIndex=0;
    int lastIndex=0;
};

class Game{
public:
    Game(){
        player.setSize(SPRITE_WIDTH * P_scale, SPRITE_HEIGHT * P_scale);

    }
    ~Game(){

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        if (texture) SDL_DestroyTexture(texture);
        if (tileset) SDL_DestroyTexture(tileset);
        if (playerSprite) SDL_CloseIO(playerSprite);
        if (tilesetSprite) SDL_CloseIO(tilesetSprite);

    }
public:
    void run();


    void setPlatform();
    static int getSpriteHeight(){
        return SPRITE_HEIGHT;
    }
    static int getSpriteWidth(){
        return SPRITE_WIDTH;
    }
    static int getTileHeight(){
        return TILE_HEIGHT;
    }
    static int getTileWidth(){
        return TILE_WIDTH;
    }
    static float getWscale() {
        return P_scale;
    }
    float getWindowHeight() const{
        return (float)windowH;
    }
    float getWindowWidth() const{
        return (float)windowW;
    }
private:
    //animation and texture
    Animation Animation;
    SDL_IOStream* playerSprite = SDL_IOFromFile("sheets/DinoSprites - vita.png", "rb");
    SDL_IOStream* tilesetSprite = SDL_IOFromFile("Platforms/Terrain.png", "rb");
    SDL_IOStream* backGroundSprite = SDL_IOFromFile("Background/orig.png", "rb");
    SDL_Texture* texture = nullptr;
    SDL_Texture* tileset = nullptr;
    SDL_Texture* backGround = nullptr;
    static const int SPRITE_HEIGHT =24;
    static const int SPRITE_WIDTH =24;
    static const int TILE_HEIGHT =16;
    static const int TILE_WIDTH =16;
    //player and platform
    Platform platforms[MAX_PLATFORM];
    Player player;
    //scaling and window and input
    constexpr static const float P_scale =5.0f;
    int windowH =0,windowW=0;
    bool running =true;
    SDL_Event event;
    SDL_Window* window =nullptr;
    SDL_Renderer* renderer =nullptr;
    //player init pos
    float x = 0,y=0;
};


