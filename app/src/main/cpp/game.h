//
// Created by LENOVO on 29-10-2025.
//
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
    SDL_IOStream* rw = SDL_IOFromFile("sheets/DinoSprites - mort.png", "rb");
    SDL_Texture* texture = nullptr;
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
    void run();
    ~Game(){

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);

    }
};

#endif //MY_APP_GAME_H
