//
// Created by LENOVO on 04-07-2026.
//
#include "AudioMenuState.h"
#include "SDL3/SDL.h"
#include "SDL3_image/SDL_image.h"
#include "SDL3_ttf/SDL_ttf.h"


void AudioMenuState::render(SDL_Renderer *renderer) {
    SDL_SetTextureScaleMode(m_menuTexture,SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_optionBlockTexture,SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_slidebarTexture,SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_sliderTexture,SDL_SCALEMODE_NEAREST);

    SDL_FRect menuDst ={100.00f,25.00f,1400.00f,670.00f};
    SDL_FRect menuSrc ={0.00f,0.00f,401.00f,198.00f};
    SDL_RenderTexture(renderer,m_menuTexture,&menuSrc,&menuDst);
    //MASTER option block Rendering
    {
        SDL_FRect optionBlockDst = {330.f, 120.00f, 1100.00f, 150.00f};

        SDL_FRect optionBlockSrc = {0.00f, 0.00f, 185.00f, 34.00f};
        SDL_RenderTexture(renderer, m_optionBlockTexture, &optionBlockSrc, &optionBlockDst);

        SDL_Surface *fontSurface = TTF_RenderText_Solid(m_font, "Master",
                                                        6, SDL_Color{0, 0, 0, 255});
        m_fontTexture = SDL_CreateTextureFromSurface(renderer, fontSurface);
        SDL_DestroySurface(fontSurface);
        SDL_FRect fontDst = {400.00f, m_masterSlidebar.y, 120.00f, 30.00f};
        SDL_RenderTexture(renderer, m_fontTexture, nullptr, &fontDst);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 0);
        SDL_FRect scaleValueRect = {m_minimumSliderX + 5, m_masterSlidebar.y + 4,
                                    m_masterSlider.x - m_minimumSliderX, m_masterSlidebar.h - 7};
        SDL_RenderFillRect(renderer, &scaleValueRect);

        SDL_FRect slidebarDst = {m_masterSlidebar.x, m_masterSlidebar.y, m_masterSlidebar.w, m_masterSlidebar.h};
        SDL_FRect slidebarSrc = {0.00f, 0.00f, 98.75f, 7.00f};
        SDL_RenderTexture(renderer, m_slidebarTexture, &slidebarSrc, &slidebarDst);
        SDL_FRect sliderDst = {m_masterSlider.x, m_masterSlider.y, m_masterSlider.w, m_masterSlider.h};
        SDL_FRect sliderSrc = {0.00f, 0.00f, 5.00f, 13.00f};
        SDL_RenderTexture(renderer, m_sliderTexture, &sliderSrc, &sliderDst);
    }
    //Music option block Rendering
    {
        SDL_FRect optionBlockDst = {330.f, 120.00f+150.00f+20.00f, 1100.00f, 150.00f};

        SDL_FRect optionBlockSrc = {0.00f, 0.00f, 185.00f, 34.00f};
        SDL_RenderTexture(renderer, m_optionBlockTexture, &optionBlockSrc, &optionBlockDst);

        SDL_Surface *fontSurface = TTF_RenderText_Solid(m_font, "Music",
                                                        5, SDL_Color{0, 0, 0, 255});
        m_fontTexture = SDL_CreateTextureFromSurface(renderer, fontSurface);
        SDL_DestroySurface(fontSurface);
        SDL_FRect fontDst = {400.00f, m_musicSlidebar.y, 100.00f, 30.00f};
        SDL_RenderTexture(renderer, m_fontTexture, nullptr, &fontDst);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 0);
        SDL_FRect scaleValueRect = {m_minimumSliderX + 5, m_musicSlidebar.y +4,
                                    m_musicSlider.x - m_minimumSliderX, m_musicSlidebar.h - 7};
        SDL_RenderFillRect(renderer, &scaleValueRect);

        SDL_FRect slidebarDst = {m_musicSlidebar.x, m_musicSlidebar.y, m_musicSlidebar.w, m_musicSlidebar.h};
        SDL_FRect slidebarSrc = {0.00f, 0.00f, 98.75f, 7.00f};
        SDL_RenderTexture(renderer, m_slidebarTexture, &slidebarSrc, &slidebarDst);
        SDL_FRect sliderDst = {m_musicSlider.x, m_musicSlider.y, m_musicSlider.w, m_musicSlider.h};
        SDL_FRect sliderSrc = {0.00f, 0.00f, 5.00f, 13.00f};
        SDL_RenderTexture(renderer, m_sliderTexture, &sliderSrc, &sliderDst);
    }
    // SFX option block Rendering
    {
        SDL_FRect optionBlockDst = {330.f, 120.00f+(150.00f+20.00f)*2, 1100.00f, 150.00f};

        SDL_FRect optionBlockSrc = {0.00f, 0.00f, 185.00f, 34.00f};
        SDL_RenderTexture(renderer, m_optionBlockTexture, &optionBlockSrc, &optionBlockDst);

        SDL_Surface *fontSurface = TTF_RenderText_Solid(m_font, "SFX",
                                                        3, SDL_Color{0, 0, 0, 255});
        m_fontTexture = SDL_CreateTextureFromSurface(renderer, fontSurface);
        SDL_DestroySurface(fontSurface);
        SDL_FRect fontDst = {400.00f, m_sfxSlidebar.y, 60.00f, 30.00f};
        SDL_RenderTexture(renderer, m_fontTexture, nullptr, &fontDst);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 0);
        SDL_FRect scaleValueRect = {m_minimumSliderX + 5, m_sfxSlidebar.y + 4,
                                    m_sfxSlider.x - m_minimumSliderX, m_sfxSlidebar.h - 7};
        SDL_RenderFillRect(renderer, &scaleValueRect);

        SDL_FRect slidebarDst = {m_sfxSlidebar.x, m_sfxSlidebar.y, m_sfxSlidebar.w, m_sfxSlidebar.h};
        SDL_FRect slidebarSrc = {0.00f, 0.00f, 98.75f, 7.00f};
        SDL_RenderTexture(renderer, m_slidebarTexture, &slidebarSrc, &slidebarDst);
        SDL_FRect sliderDst = {m_sfxSlider.x, m_sfxSlider.y, m_sfxSlider.w, m_sfxSlider.h};
        SDL_FRect sliderSrc = {0.00f, 0.00f, 5.00f, 13.00f};
        SDL_RenderTexture(renderer, m_sliderTexture, &sliderSrc, &sliderDst);
    }
}

void AudioMenuState::update(float dt) {
    m_masterSlider.x = m_minimumSliderX + GameData::getInstance().getAudioScale() *(m_maximumSliderX-m_minimumSliderX);
    m_musicSlider.x = m_minimumSliderX + GameData::getInstance().getMusicAudioScale() *(m_maximumSliderX-m_minimumSliderX);
    m_sfxSlider.x = m_minimumSliderX + GameData::getInstance().getSFXAudioScale() *(m_maximumSliderX-m_minimumSliderX);
}

bool AudioMenuState::handleEvents(SDL_Event &event) {
    float startTouchX=-0.0f;
    float startTouchY=0.0f;
    if(event.type == SDL_EVENT_FINGER_DOWN){

        float touchX = event.tfinger.x * (float)GameData::getInstance().getWinWidth();
        float touchY = event.tfinger.y * (float)GameData::getInstance().getWinHeight();
//        LOGI("touch x:%f y:%f",touchX,touchY);
        if(touchX >= m_masterSlidebar.x && touchX <= m_masterSlidebar.x + m_masterSlidebar.w &&
            touchY >= m_masterSlidebar.y && touchY <= m_masterSlidebar.y + m_masterSlidebar.h){
            m_scaleType =ScaleType::MASTER;
            m_sliderFingerID =event.tfinger.fingerID;
            startTouchX=touchX;
            startTouchY=touchY;
        }
        else if(touchX >= m_musicSlidebar.x && touchX <= m_musicSlidebar.x + m_musicSlidebar.w &&
            touchY >= m_musicSlidebar.y && touchY <= m_musicSlidebar.y + m_musicSlidebar.h){
            m_scaleType =ScaleType::MUSIC;
            m_sliderFingerID =event.tfinger.fingerID;
            startTouchX=touchX;
            startTouchY=touchY;
        }
        else if(touchX >= m_sfxSlidebar.x && touchX <= m_sfxSlidebar.x + m_sfxSlidebar.w &&
            touchY >= m_sfxSlidebar.y && touchY <= m_sfxSlidebar.y + m_sfxSlidebar.h){
            m_scaleType =ScaleType::SFX;
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
            float len =(touchX-m_minimumSliderX);
            if(len > m_maximumSliderX)len = fullwidth;
            if(m_scaleType == ScaleType::MASTER)
                GameData::getInstance().setAudioScale(len/fullwidth);
            if(m_scaleType == ScaleType::MUSIC)
                GameData::getInstance().setMusicAudioScale(len/fullwidth);
            if(m_scaleType == ScaleType::SFX)
                GameData::getInstance().setSFXAudioScale(len/fullwidth);

        }
        return true;
    }
    return false;
}
AudioMenuState::AudioMenuState(SDL_Renderer *renderer) {
    LOGI("Audio menu state constructor:%p",this);
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

    m_font = TTF_OpenFontIO(m_fontfile,false,24);
    LOGI("Loaded font %p", m_font);
    if (!m_font)
        LOGI("%s", SDL_GetError());
}
AudioMenuState::~AudioMenuState(){
    LOGI("Audio menu state destructor:%p",this);
    SDL_DestroyTexture(m_menuTexture);
    SDL_DestroyTexture(m_optionBlockTexture);
    SDL_DestroyTexture(m_slidebarTexture);
    SDL_DestroyTexture(m_sliderTexture);
    SDL_DestroyTexture(m_fontTexture);
    TTF_CloseFont(m_font);
    SDL_CloseIO(m_fontfile);
}
