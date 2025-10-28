//
// Created by LENOVO on 03-10-2025.
//
#include "SDL3/SDL.h"
#include "enet/enet.h"
#include "input.h"
#include "SDL3_image/SDL_image.h"

#define LOG_TAG "GAME"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

class Game{
private:
    SDL_IOStream* rw = SDL_IOFromFile("sheets/DinoSprites - doux.png", "rb");
    SDL_Texture* texture = nullptr;
    static const int SPRITE_HEIGHT =24;
    static const int SPRITE_WIDTH =24;
    const float W_scale =10.0f;
    int windowH =0,windowW=0;
    bool running =true;
    SDL_Event event;
    SDL_Window* window = SDL_CreateWindow("SDL3 Android App", 0, 0, SDL_WINDOW_FULLSCREEN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    float x = 100.00f , y =100.00f ;
public:
    void run(){
        int i =1;
        if(!rw)LOGE("cannot load");
        else
            LOGE("loaded");
        Input input(event);

        SDL_Surface* surface = IMG_Load_IO(rw,1);
        if(!surface)return;
        texture = SDL_CreateTextureFromSurface(renderer,surface);
        if(!texture)return;
        SDL_DestroySurface(surface);
        SDL_SetTextureScaleMode(texture,SDL_SCALEMODE_NEAREST);

        while(running){
            if(!window)return;
            if(!renderer)return;
            SDL_GetWindowSize(window,&windowW,&windowH);
            input.eventhandler(running,windowW,windowH,x,y);
            i =(i+1) % 24;
            SDL_SetRenderDrawColor(renderer, 0, 128, 255, 255);
            SDL_RenderClear(renderer);

            SDL_FRect dst = {x, y, SPRITE_WIDTH*W_scale, SPRITE_HEIGHT*W_scale};
            SDL_FRect src ={(float)(0+(SPRITE_WIDTH*i)),0,SPRITE_WIDTH,SPRITE_HEIGHT};
            SDL_RenderTexture(renderer, texture, &src, &dst);

            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            SDL_FRect Joystick=input.getJoystick();
            SDL_RenderFillRect(renderer, &Joystick);

            SDL_RenderPresent(renderer);
            SDL_Delay(16);
            i++;
        }
    }
    ~Game(){

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);

    }
};
