//
// Created by LENOVO on 03-10-2025.
//
#include "SDL3/SDL.h"
#include "enet/enet.h"
#include "input.h"
#include "SDL3_image/SDL_image.h"
#include "game.h"
#define LOG_TAG "GAME"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

void Game::run(){

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
    int current_Frame =0;
    int frame_delay =50;//delayed by 50ms
    Uint32 Last_Frame_Time =SDL_GetTicks();
    while(running){
        if(!window)return;
        if(!renderer)return;
        SDL_GetWindowSize(window,&windowW,&windowH);
        input.eventhandler(running,windowW,windowH,x,y);
        //gets player animation indices for sprite rendering
        input.getAnimationindexes(this->Animation);
        Uint32 now =SDL_GetTicks();
        if(now-Last_Frame_Time>frame_delay){
            if(current_Frame<Animation.lastIndex)current_Frame++;
            else current_Frame=Animation.startIndex;
            Last_Frame_Time=now;

        }
        //sets color
        SDL_SetRenderDrawColor(renderer, 0, 128, 255, 255);
        SDL_RenderClear(renderer);

        SDL_FRect dst = {x, y, SPRITE_WIDTH * W_scale, SPRITE_HEIGHT * W_scale};
        SDL_FRect src = {(float) (0 + (SPRITE_WIDTH * current_Frame)), 0, SPRITE_WIDTH, SPRITE_HEIGHT};
        if(input.getPlayerFacingdir())SDL_RenderTexture(renderer, texture, &src, &dst);
        else SDL_RenderTextureRotated(renderer, texture, &src, &dst,0.0,NULL,SDL_FLIP_HORIZONTAL);
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_FRect Joystick=input.getJoystick();
        SDL_RenderFillRect(renderer, &Joystick);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);

    }
}