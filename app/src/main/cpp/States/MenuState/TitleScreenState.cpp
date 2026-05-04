//
// Created by LENOVO on 28-04-2026.
//
#include "TitleScreenState.h"
#include "States/GameState/GameState.h"
#include "MenuState.h"

TitleScreenState::TitleScreenState(SDL_Renderer *renderer)
    {
        m_renderer = renderer;
        LOGI("menu state constructor");
//        if(backGroundSprite)LOGI("m_backGroundSprite:true");
//        Backgroundsurface = IMG_Load_IO(backGroundSprite,false);
//        if(!Backgroundsurface)return;

        SDL_Surface* Backgroundsurface = IMG_Load_IO(m_backgroundSprite,false);
        if(!Backgroundsurface)return;


        m_background = SDL_CreateTextureFromSurface(renderer,Backgroundsurface);
        SDL_DestroySurface(Backgroundsurface);
        if(!m_background)return;
        SDL_SetTextureScaleMode(m_background,SDL_SCALEMODE_NEAREST);
    }


void TitleScreenState::render(SDL_Renderer *renderer) {
    SDL_FRect backgroundSrc{};
    SDL_FRect backgroundDst = {0, 0, 1600, 720};


    backgroundSrc.x = (float)(0 + (m_currentFrame * 315));
    backgroundSrc.y = 0;
    backgroundSrc.w = 315.00f;
    backgroundSrc.h = 250.00f;

//  SDL_RenderTexture(m_renderer, backGround, nullptr, &backgroundDst);
    SDL_RenderTexture(m_renderer, m_background,&backgroundSrc, &backgroundDst);
}
void TitleScreenState::update(float dt) {
    m_now =SDL_GetTicks();
    if(m_now - m_last > m_aniDelay)
    {
        m_currentFrame = m_currentFrame % 5;
        m_currentFrame++;
        m_last = m_now;
    }
}
void TitleScreenState::handleEvents(SDL_Event &event){

    if(event.type == SDL_EVENT_FINGER_DOWN){
        LOGI("menu starts transitioning to game state");
        Engine::Get().changeState(std::make_unique<MenuState>(m_renderer));
        return;
        LOGI("menu state handleEvents end");
    }
}

TitleScreenState::~TitleScreenState() {
        LOGI("Menu destructor");
//        if (backGround)      SDL_DestroyTexture(backGround);
//        if (backGroundSprite)SDL_CloseIO(backGroundSprite);
}

