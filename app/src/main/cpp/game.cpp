//
// Created by LENOVO on 03-10-2025.
//
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <enet/enet.h>
#include "input.h"
#include "game.h"
#include "Player.h"
#include "gameUtils.h"
#include "camera.h"
#include "string"


#define LOG_TAG "GAME"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

void Game::setPlatform() {

    platforms[0] = {200,350,48,16};
    platforms[1] = {1800,150,48,16};
    platforms[2] = {400,0,48,16};
    platforms[3] = {1600,-100,48,16};
    platforms[4] = {600,-200,48,16};
    platforms[5] = {1400,-300,48,16};
    platforms[6] = {100,-400,48,16};
    platforms[7] = {1700,-500,48,16};
    platforms[8] = {300,-600,48,16};
    platforms[9] = {1500,-700,48,16};

    platforms[10] = {500,-800,48,16};
    platforms[11] = {1300,-900,48,16};
    platforms[12] = {250,-1000,48,16};
    platforms[13] = {1750,-1100,48,16};
    platforms[14] = {450,-1200,48,16};
    platforms[15] = {1650,-1300,48,16};
    platforms[16] = {650,-1400,48,16};
    platforms[17] = {1200,-1500,48,16};
    platforms[18] = {150,-1600,48,16};
    platforms[19] = {1850,-1700,48,16};

    platforms[20] = {350,-1800,48,16};
    platforms[21] = {1550,-1900,48,16};
    platforms[22] = {750,-2000,48,16};
    platforms[23] = {1100,-2100,48,16};
    platforms[24] = {50,-2200,48,16};
    platforms[25] = {1950,-2300,48,16};
    platforms[26] = {550,-2400,48,16};
    platforms[27] = {1450,-2500,48,16};
    platforms[28] = {250,-2600,48,16};
    platforms[29] = {1750,-2700,48,16};

    platforms[30] = {450,-2800,48,16};
    platforms[31] = {1650,-2900,48,16};
    platforms[32] = {850,-3000,48,16};
    platforms[33] = {1050,-3100,48,16};
    platforms[34] = {150,-3200,48,16};
    platforms[35] = {1850,-3300,48,16};
    platforms[36] = {350,-3400,48,16};
    platforms[37] = {1550,-3500,48,16};
    platforms[38] = {950,-3600,48,16};
    platforms[39] = {1000,-3700,48,16};

    platforms[40] = {200,-3800,48,16};
    platforms[41] = {1800,-3900,48,16};
    platforms[42] = {400,-4000,48,16};
    platforms[43] = {1600,-4100,48,16};
    platforms[44] = {600,-4200,48,16};
    platforms[45] = {1400,-4300,48,16};
    platforms[46] = {100,-4400,48,16};
    platforms[47] = {1700,-4500,48,16};
    platforms[48] = {300,-4600,48,16};
    platforms[49] = {1500,-4700,48,16};

    platforms[50] = {500,-4800,48,16};
    platforms[51] = {1300,-4900,48,16};
    platforms[52] = {250,-5000,48,16};
    platforms[53] = {1750,-5100,48,16};
    platforms[54] = {450,-5200,48,16};
    platforms[55] = {1650,-5300,48,16};
    platforms[56] = {650,-5400,48,16};
    platforms[57] = {1200,-5500,48,16};
    platforms[58] = {150,-5600,48,16};
    platforms[59] = {1850,-5700,48,16};

    platforms[60] = {350,-5800,48,16};
    platforms[61] = {1550,-5900,48,16};
    platforms[62] = {750,-6000,48,16};
    platforms[63] = {1100,-6100,48,16};
    platforms[64] = {50,-6200,48,16};
    platforms[65] = {1950,-6300,48,16};
    platforms[66] = {550,-6400,48,16};
    platforms[67] = {1450,-6500,48,16};
    platforms[68] = {250,-6600,48,16};
    platforms[69] = {1750,-6700,48,16};

    platforms[70] = {450,-6800,48,16};
    platforms[71] = {1650,-6900,48,16};
    platforms[72] = {850,-7000,48,16};
    platforms[73] = {1050,-7100,48,16};
    platforms[74] = {150,-7200,48,16};
    platforms[75] = {1850,-7300,48,16};
    platforms[76] = {350,-7400,48,16};
    platforms[77] = {1550,-7500,48,16};
    platforms[78] = {950,-7600,48,16};
    platforms[79] = {1000,-7700,48,16};

    platforms[80] = {200,-7800,48,16};
    platforms[81] = {1800,-7900,48,16};
    platforms[82] = {400,-8000,48,16};
    platforms[83] = {1600,-8100,48,16};
    platforms[84] = {600,-8200,48,16};
    platforms[85] = {1400,-8300,48,16};
    platforms[86] = {100,-8400,48,16};
    platforms[87] = {1700,-8500,48,16};
    platforms[88] = {300,-8600,48,16};
    platforms[89] = {1500,-8700,48,16};

    platforms[90] = {500,-8800,48,16};
    platforms[91] = {1300,-8900,48,16};
    platforms[92] = {250,-9000,48,16};
    platforms[93] = {1750,-9100,48,16};
    platforms[94] = {450,-9200,48,16};
    platforms[95] = {1650,-9300,48,16};
    platforms[96] = {650,-9400,48,16};
    platforms[97] = {1200,-9500,48,16};
    platforms[98] = {150,-9600,48,16};
    platforms[99] = {1850,-9700,48,16};



}



void Game::run(){
    if(!TTF_Init())LOGI("failed tff:%s",SDL_GetError());
    if(!fontFile)LOGI("cant find font");

    window = SDL_CreateWindow("SDL3 Android App", 0, 0, SDL_WINDOW_FULLSCREEN);

    renderer = SDL_CreateRenderer(window, "opengles2");
    if (!renderer) {
        LOGE("Renderer creation failed: %s", SDL_GetError());
    }
    const char* name = SDL_GetRendererName(renderer);
    LOGI("Renderer: %s", name);

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
    SDL_SetTextureScaleMode(fpstexture,SDL_SCALEMODE_NEAREST);
    if(!playerSprite)LOGE("cannot load sprite");

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
    Camera::getInstance().setSize(getWindowWidth(),getWindowHeight());

    SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(backGround,SDL_BLENDMODE_NONE);
    while(running){
        if(!window)return;
        if(!renderer)return;

        input.eventhandler(running,windowW,windowH,player.x,player.y,player.h,player.w, deltaTime,platforms, isCompleted);
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
        Camera::getInstance().lockCameraOn(player.x,player.y,player.h,player.w);
        SDL_SetRenderDrawColor(renderer, 0, 128, 255, 255);
        SDL_RenderClear(renderer);
        //background rendering
        SDL_FRect backgroundDst = {0, 0, getWindowWidth(), getWindowHeight()};

        SDL_RenderTexture(renderer, backGround, NULL, &backgroundDst);


        //tile rendering
        for(int i = 0; i < 100; i++){

            SDL_FRect tileDst = {platforms[i].x-Camera::getInstance().getCamera().x,
                                 platforms[i].y-Camera::getInstance().getCamera().y,
                                 platforms[i].w * P_scale, platforms[i].h * P_scale};
            SDL_FRect tileSrc = {0+96, 0, 48, 16};
            SDL_RenderTexture(renderer, tileset, &tileSrc, &tileDst);
            //tile border
            SDL_FRect border = {platforms[i].x-Camera::getInstance().getCamera().x,
                                platforms[i].y-Camera::getInstance().getCamera().y,
                                platforms[i].w * P_scale, platforms[i].h * P_scale};
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderRect(renderer,&border);
            //ground border
            SDL_FRect groundborder = {platforms[i].x-Camera::getInstance().getCamera().x,
                                platforms[i].y-Camera::getInstance().getCamera().y,
                                platforms[i].w * P_scale, 1 * P_scale};
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            SDL_RenderRect(renderer,&groundborder);
        }
        //player rendering

        SDL_FRect border = {(player.x)-Camera::getInstance().getCamera().x,
                            (player.y)-Camera::getInstance().getCamera().y, player.w-26, player.h-20};
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderRect(renderer,&border);
//        LOGI("camera X: %f, camera Y: %f",Camera::getInstance().getCamera().x,Camera::getInstance().getCamera().y);
        SDL_FRect dst = {(player.x-16)-Camera::getInstance().getCamera().x,
                         (player.y-9)-Camera::getInstance().getCamera().y,
                         player.w, player.h};
        SDL_FRect src = {(float) (0 + (SPRITE_WIDTH * current_Frame)), 0, SPRITE_WIDTH, SPRITE_HEIGHT};
        if(input.getPlayerFacingdir())SDL_RenderTexture(renderer, texture, &src, &dst);
        else SDL_RenderTextureRotated(renderer, texture, &src, &dst,0.0,NULL,SDL_FLIP_HORIZONTAL);

        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 90);
        SDL_FRect Joystick=input.getJoystick();
        SDL_RenderFillRect(renderer, &Joystick);

        //mechanics rendering
        if(isCompleted)
        {
            SDL_Color EColor = {2, 255, 25, 255};
            SDL_Surface *ESurface = TTF_RenderText_Blended(font, "YOU WON", 7, EColor);
            Etexture = SDL_CreateTextureFromSurface(renderer, ESurface);
            SDL_FRect Etextrect = {((player.x)-Camera::getInstance().getCamera().x)-((static_cast<float>(ESurface->w) * 2)/2)/2,
                                   (player.y)-Camera::getInstance().getCamera().y, static_cast<float>(ESurface->w) * 2,
                                   static_cast<float>(ESurface->h) * 2};
            SDL_DestroySurface(ESurface);
            SDL_RenderTexture(renderer, Etexture, nullptr, &Etextrect);
        }

        //fps counter rendering
        SDL_Color fpsColor = { 255, 255, 255, 255 };
        SDL_Surface* fpsSurface = TTF_RenderText_Blended(font, "FPS:", 4,fpsColor);
        fpstexture = SDL_CreateTextureFromSurface(renderer, fpsSurface);
        SDL_FRect fpstextrect = {0, 0, static_cast<float>(fpsSurface->w)*2,static_cast<float>(fpsSurface->h)*2 };
        SDL_DestroySurface(fpsSurface);
        SDL_RenderTexture(renderer,fpstexture,nullptr,&fpstextrect);
        SDL_RenderPresent(renderer);
        Uint32 frametime = SDL_GetTicks() - framestart;
        if (frametime < frameDelay) {
            SDL_Delay(frameDelay - frametime);
        }

    }
}