//
// Created by LENOVO on 29-10-2025.
//
#include "SDL3_image/SDL_image.h"
#include "SDL3/SDL.h"
#include "platform.h"
#include "Player.h"
#ifndef MY_APP_GAME_H
#define MY_APP_GAME_H
#define MAX_PLATFORM 100

struct Animation{
    int startIndex=0;
    int lastIndex=0;
};

class Game{
public:
    Game(){
        //player sprite loading
        SDL_Surface* Playersurface = IMG_Load_IO(playerSprite,true);
        if(!Playersurface)return;
        texture = SDL_CreateTextureFromSurface(renderer,Playersurface);
        if(!texture)return;
        SDL_DestroySurface(Playersurface);

        // tileset loading
        SDL_Surface* Tilesurface = IMG_Load_IO(tilesetSprite,true);
        if(!Tilesurface)return;
        tileset = SDL_CreateTextureFromSurface(renderer,Tilesurface);
        if(!tileset)return;

        SDL_DestroySurface(Tilesurface);
        SDL_SetTextureScaleMode(texture,SDL_SCALEMODE_NEAREST);
        SDL_SetTextureScaleMode(tileset,SDL_SCALEMODE_NEAREST);
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

    void checkCollisions();
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
    static float getWscale(){
        return P_scale;
    }
private:
    //animation and texture
    Animation Animation;
    SDL_IOStream* playerSprite = SDL_IOFromFile("sheets/DinoSprites - mort.png", "rb");
    SDL_IOStream* tilesetSprite = SDL_IOFromFile("Platforms/Terrain.png", "rb");
    SDL_Texture* texture = nullptr;
    SDL_Texture* tileset = nullptr;
    static const int SPRITE_HEIGHT =24;
    static const int SPRITE_WIDTH =24;
    static const int TILE_HEIGHT =16;
    static const int TILE_WIDTH =16;
    //player and platform
    Platform platforms[MAX_PLATFORM];
    Player player;
    //scaling and window and input
    constexpr static const float P_scale =1.0f;
    int windowH =0,windowW=0;
    bool running =true;
    SDL_Event event;
    SDL_Window* window = SDL_CreateWindow("SDL3 Android App", 0, 0, SDL_WINDOW_FULLSCREEN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    //player init pos
    float x = 100.00f , y =720.00f - SPRITE_HEIGHT * P_scale ;
};

#endif //MY_APP_GAME_H
