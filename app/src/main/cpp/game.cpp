//
// Created by LENOVO on 03-10-2025.
//
#include <SDL3/SDL.h>
#include <enet/enet.h>
#include "input.h"
#include "game.h"
#include "Player.h"
#include "gameUtils.h"

#define LOG_TAG "GAME"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

void Game::setPlatform() {
    platforms[0] ={1200,200,48,16};
    platforms[1] ={1000,300,48,16};
    platforms[2] ={800,400,48,16};
    platforms[3] ={500,500,48,16};



}



void Game::run(){
    window = SDL_CreateWindow("SDL3 Android App", 0, 0, SDL_WINDOW_FULLSCREEN);

    renderer = SDL_CreateRenderer(window, "opengles2");
    if (!renderer) {
        LOGE("Renderer creation failed: %s", SDL_GetError());
    }
    const char* name = SDL_GetRendererName(renderer);
    LOGI("Renderer: %s", name);
    //background sprite loading;
    SDL_Surface* Backgroundsurface = IMG_Load_IO(backGroundSprite,true);
    if(!Backgroundsurface)return;
    backGround = SDL_CreateTextureFromSurface(renderer,Backgroundsurface);
    if(!backGround)return;
    SDL_DestroySurface(Backgroundsurface);
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

    SDL_SetTextureScaleMode(texture,SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(tileset,SDL_SCALEMODE_NEAREST);
    SDL_DestroySurface(Tilesurface);
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
    //display resolution
    const SDL_DisplayMode* mode =
            SDL_GetCurrentDisplayMode(SDL_GetPrimaryDisplay());
    x = 100.00f , y =getWindowHeight() - SPRITE_HEIGHT * P_scale;

    if(mode)
    {
        LOGI("Display native: %d x %d", mode->w, mode->h);
        windowW =mode->w;
        windowH = mode->h;
    }
    player.setSize(SPRITE_WIDTH * P_scale, SPRITE_HEIGHT * P_scale);
    player.setPosition(x,y,getWindowHeight());
    SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(backGround,SDL_BLENDMODE_NONE);
    while(running){
        if(!window)return;
        if(!renderer)return;

        input.eventhandler(running,windowW,windowH,player.x,player.y,player.h,player.w, deltaTime,platforms);
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
        //background rendering
        SDL_FRect backgroundDst = {0, 0, getWindowWidth(), getWindowHeight()};

        SDL_RenderTexture(renderer, backGround, NULL, &backgroundDst);


        //tile rendering
        for(int i = 0; i < 4; i++){

            gameUtilities::checkcollision(player.x,player.y,platforms[i].x,platforms[i].y,player.h,player.w,16.0f *P_scale,48.0f *P_scale);


            SDL_FRect tileDst = {platforms[i].x, platforms[i].y, platforms[i].w * P_scale, platforms[i].h * P_scale};
            SDL_FRect tileSrc = {0+96, 0, 48, 16};
            SDL_RenderTexture(renderer, tileset, &tileSrc, &tileDst);

            SDL_FRect border = {platforms[i].x, platforms[i].y, platforms[i].w * P_scale, platforms[i].h * P_scale};
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderRect(renderer,&border);
        }
        //player rendering

        SDL_FRect border = {player.x, player.y, player.w, player.h};
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderRect(renderer,&border);

        SDL_FRect dst = {player.x, player.y, player.w, player.h};
        SDL_FRect src = {(float) (0 + (SPRITE_WIDTH * current_Frame)), 0, SPRITE_WIDTH, SPRITE_HEIGHT};
        if(input.getPlayerFacingdir())SDL_RenderTexture(renderer, texture, &src, &dst);
        else SDL_RenderTextureRotated(renderer, texture, &src, &dst,0.0,NULL,SDL_FLIP_HORIZONTAL);

        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 90);
        SDL_FRect Joystick=input.getJoystick();
        SDL_RenderFillRect(renderer, &Joystick);

        SDL_RenderPresent(renderer);
        Uint32 frametime = SDL_GetTicks() - framestart;
        if (frametime < frameDelay) {
            SDL_Delay(frameDelay - frametime);
        }

    }
}