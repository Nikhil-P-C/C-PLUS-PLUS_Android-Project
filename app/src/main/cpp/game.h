//
// Created by LENOVO on 29-10-2025.
//
#include "SDL3_image/SDL_image.h"
#include "SDL3/SDL.h"
#ifndef MY_APP_GAME_H
#define MY_APP_GAME_H
struct Animation{
    int startIndex=0;
    int lastIndex=0;
};
class Game{
private:

    Animation Animation;
    SDL_IOStream* playerSprite = SDL_IOFromFile("sheets/DinoSprites - mort.png", "rb");
    SDL_IOStream* tilesetSprite = SDL_IOFromFile("Platforms/Terrain.png", "rb");
    SDL_Texture* texture = nullptr;
    SDL_Texture* tileset = nullptr;
    static const int SPRITE_HEIGHT =24;
    static const int SPRITE_WIDTH =24;
    const float W_scale =5.0f;
    int windowH =0,windowW=0;
    bool running =true;
    SDL_Event event;
    SDL_Window* window = SDL_CreateWindow("SDL3 Android App", 0, 0, SDL_WINDOW_FULLSCREEN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    float x = 100.00f , y =720.00f - SPRITE_HEIGHT * W_scale ;
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
    void run();
    ~Game(){

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        if (texture) SDL_DestroyTexture(texture);
        if (tileset) SDL_DestroyTexture(tileset);
        if (playerSprite) SDL_CloseIO(playerSprite);
        if (tilesetSprite) SDL_CloseIO(tilesetSprite);

    }
};

#endif //MY_APP_GAME_H
