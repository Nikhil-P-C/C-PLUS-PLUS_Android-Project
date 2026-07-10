//
// Created by LENOVO on 04-07-2026.
//
#include "AudioMenuState.h"
#include "SDL3/SDL.h"
#include "SDL3_image/SDL_image.h"
#include "SDL3_ttf/SDL_ttf.h"


void AudioMenuState::render(SDL_Renderer *renderer) {
    LOGI("audio menu state render");
    SDL_SetTextureScaleMode(m_menuTexture,SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_optionBlockTexture,SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_slidebarTexture,SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_sliderTexture,SDL_SCALEMODE_NEAREST);

    SDL_FRect menuDst ={100.00f,25.00f,1400.00f,670.00f};
    SDL_FRect menuSrc ={0.00f,0.00f,401.00f,198.00f};
    SDL_RenderTexture(renderer,m_menuTexture,&menuSrc,&menuDst);

    SDL_FRect optionBlockDst ={330.f,120.00f,1100.00f,150.00f};

    SDL_FRect optionBlockSrc ={0.00f,0.00f,185.00f,34.00f};
    SDL_RenderTexture(renderer,m_optionBlockTexture,&optionBlockSrc,&optionBlockDst);

    LOGI("audio menu state render");
    SDL_Surface* fontSurface = TTF_RenderText_Solid(m_font,"paused ",
                                                    7,SDL_Color{255,255,255,255});
    LOGI("audio menu state render");
    m_fontTexture = SDL_CreateTextureFromSurface(renderer,fontSurface);
    SDL_DestroySurface(fontSurface);
    LOGI("audio menu state render");
    SDL_FRect fontDst ={330.00f,120.00f,1100.00f,150.00f};
    SDL_RenderTexture(renderer,m_fontTexture,nullptr,&fontDst);

    LOGI("audio menu state render");
    SDL_SetRenderDrawColor(renderer,255,0,0,0);
    SDL_FRect scaleValueRect ={m_minimumSliderX+5,m_slidebar.y+4,m_slider.x-m_minimumSliderX,m_slidebar.h-7};
    SDL_RenderFillRect(renderer,&scaleValueRect);

    SDL_FRect slidebarDst ={m_slidebar.x,m_slidebar.y,m_slidebar.w,m_slidebar.h};
    SDL_FRect slidebarSrc ={0.00f,0.00f,98.75f,7.00f};
    SDL_RenderTexture(renderer,m_slidebarTexture,&slidebarSrc,&slidebarDst);
    LOGI("audio menu state render");
    SDL_FRect sliderDst ={m_slider.x,m_slider.y,m_slider.w,m_slider.h};
    SDL_FRect sliderSrc ={0.00f,0.00f,5.00f,13.00f};
    SDL_RenderTexture(renderer,m_sliderTexture,&sliderSrc,&sliderDst);


}

void AudioMenuState::update(float dt) {
    m_slider.x = m_minimumSliderX + GameData::getInstance().getAudioScale() *(m_maximumSliderX-m_minimumSliderX);
}

bool AudioMenuState::handleEvents(SDL_Event &event) {
    float startTouchX=-0.0f;
    float startTouchY=0.0f;
    if(event.type == SDL_EVENT_FINGER_DOWN){

        float touchX = event.tfinger.x * (float)GameData::getInstance().getWinWidth();
        float touchY = event.tfinger.y * (float)GameData::getInstance().getWinHeight();
        LOGI("touch x:%f y:%f",touchX,touchY);
        if(touchX >= m_slidebar.x && touchX <= m_slidebar.x + m_slidebar.w &&
            touchY >= m_slidebar.y && touchY <= m_slidebar.y + m_slidebar.h){
            m_sliderFingerID =event.tfinger.fingerID;
            startTouchX=touchX;
            startTouchY=touchY;

        }
        else{
            m_sliderFingerID =0;
            startTouchX=0.0f;
            startTouchY=0.0f;
        }
    }
    if(event.type == SDL_EVENT_FINGER_MOTION && m_sliderFingerID == event.tfinger.fingerID){

        float touchX = event.tfinger.x * (float)GameData::getInstance().getWinWidth();
        float touchY = event.tfinger.y * (float)GameData::getInstance().getWinHeight();

        float diffX = touchX - startTouchX;
        float diffY = touchY - startTouchY;
        if(diffX > 0){
            float fullwidth =m_maximumSliderX - m_minimumSliderX;
            LOGI("fullwidth:%f",fullwidth);
            float len =(touchX-m_minimumSliderX);
            LOGI("len:%f",len);
            if(len > m_maximumSliderX)len = fullwidth;
            GameData::getInstance().setAudioScale(len/fullwidth);
            LOGI("audio scale:%f",GameData::getInstance().getAudioScale());
        }
    }
    return false;
}
AudioMenuState::AudioMenuState(SDL_Renderer *renderer) {
    SDL_Surface* menuSurface = IMG_Load_IO(m_menuSpriteFile,false);
    m_menuTexture =SDL_CreateTextureFromSurface(renderer,menuSurface);
    SDL_CloseIO(m_menuSpriteFile);
    SDL_DestroySurface(menuSurface);

    SDL_Surface* optionBlockSurface = IMG_Load_IO(m_optionBlockFile,false);
    m_optionBlockTexture = SDL_CreateTextureFromSurface(renderer,optionBlockSurface);
    SDL_CloseIO(m_optionBlockFile);
    SDL_DestroySurface(optionBlockSurface);

    SDL_Surface* slidebarSurface = IMG_Load_IO(m_slidebarFile,false);
    m_slidebarTexture = SDL_CreateTextureFromSurface(renderer,slidebarSurface);
    SDL_CloseIO(m_slidebarFile);
    SDL_DestroySurface(slidebarSurface);

    SDL_Surface* sliderSurface = IMG_Load_IO(m_sliderFile,false);
    m_sliderTexture = SDL_CreateTextureFromSurface(renderer,sliderSurface);
    SDL_CloseIO(m_sliderFile);
    SDL_DestroySurface(sliderSurface);
    LOGI("audio menu state constructor");
    m_font = TTF_OpenFontIO(m_fontfile,false,24);

    m_fontShadow = TTF_OpenFontIO(m_fontShadowfile,false,24);
    SDL_SetTextureScaleMode(m_fontShadowTexture,SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_fontTexture,SDL_SCALEMODE_NEAREST);
    SDL_CloseIO(m_fontfile);
    SDL_CloseIO(m_fontShadowfile);
    LOGI("audio menu state constructor");

}
