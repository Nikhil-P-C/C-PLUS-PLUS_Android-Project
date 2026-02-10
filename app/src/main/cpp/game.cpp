//
// Created by LENOVO on 03-10-2025.
//
#include <SDL3/SDL.h>
#include <enet/enet.h>
#include "input.h"
#include "game.h"

#define LOG_TAG "GAME"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

void Game::setPlatform() {
    platforms[0] ={0,0,48,16};




}
void Game::checkCollisions() {

}


void Game::run(){

    if(!playerSprite)LOGE("cannot load");

    int current_Frame =0;
    int Aniframe_delay =50;//delayed by 50ms
    Uint32 Last_Frame_Time =SDL_GetTicks();
    Uint32 last =SDL_GetTicks();
    Uint32 lasttime =SDL_GetTicks();
    const int FPS =60;

    const int frameDelay = 1000/FPS;
    int fps =0;
    int frames =0;
    float deltaTime =0.0f;
    Input input(event);
    setPlatform();

    SDL_GetWindowSize(window,&windowW,&windowH);
    SDL_GetWindowSizeInPixels(window,&windowW,&windowH);
    const SDL_DisplayMode* mode =
            SDL_GetCurrentDisplayMode(SDL_GetPrimaryDisplay());

    if(mode)
    {
        LOGI("Display native: %d x %d", mode->w, mode->h);
        windowW =mode->w;
        windowH = mode->h;
    }


    while(running){
        if(!window)return;
        if(!renderer)return;

        input.eventhandler(running,windowW,windowH,player.x,player.y, deltaTime);
        //gets player animation indices for sprite rendering
        input.getAnimationindexes(this->Animation);
        Uint32 now =SDL_GetTicks();
        if(now-Last_Frame_Time>Aniframe_delay){
            if(current_Frame<Animation.startIndex)current_Frame=Animation.startIndex;
            if(current_Frame<Animation.lastIndex)current_Frame++;
            else current_Frame=Animation.startIndex;
            Last_Frame_Time=now;
        }
        fps++;
        Uint32 current =SDL_GetTicks();
        if(current - last >= 1000){

            frames =fps;
            fps =0;
            last = current;
            LOGI("FPS: %d",frames);
        }

        //delta time
        Uint32 framestart = SDL_GetTicks();
        deltaTime = (float)(framestart - lasttime) / 1000.0f;
        lasttime = framestart;

        //game starts

        SDL_SetRenderDrawColor(renderer, 0, 128, 255, 255);
        SDL_RenderClear(renderer);
        //tile rendering
        for(int i = 0; i < 100; i++){
            SDL_FRect tileDst = {platforms[i].x, platforms[i].y, platforms[i].w * P_scale, platforms[i].h * P_scale};
            SDL_FRect tileSrc = {0+96, 0, 48, 16};
            SDL_RenderTexture(renderer, tileset, &tileSrc, &tileDst);
        }
        //player rendering
        SDL_FRect dst = {player.x, player.y, player.w, player.h};
        SDL_FRect src = {(float) (0 + (SPRITE_WIDTH * current_Frame)), 0, SPRITE_WIDTH, SPRITE_HEIGHT};
        if(input.getPlayerFacingdir())SDL_RenderTexture(renderer, texture, &src, &dst);
        else SDL_RenderTextureRotated(renderer, texture, &src, &dst,0.0,NULL,SDL_FLIP_HORIZONTAL);

        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_FRect Joystick=input.getJoystick();
        SDL_RenderFillRect(renderer, &Joystick);

        SDL_RenderPresent(renderer);
        Uint32 frametime = SDL_GetTicks() - framestart;
        if (frametime < frameDelay) {
            SDL_Delay(frameDelay - frametime);
        }

    }
}