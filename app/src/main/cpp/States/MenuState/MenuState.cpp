//
// Created by LENOVO on 28-04-2026.
//
#include "MenuState.h"
#include "States/GameState/GameState.h"

MenuState::MenuState(SDL_Renderer *renderer)
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


void MenuState::render(SDL_Renderer *renderer) {
    SDL_FRect backgroundSrc{};
    SDL_FRect backgroundDst = {0, 0, 1600, 720};


    backgroundSrc.x = (float)(0 + (m_currentFrame * 315));
    backgroundSrc.y = 0;
    backgroundSrc.w = 315.00f;
    backgroundSrc.h = 250.00f;

//  SDL_RenderTexture(m_renderer, backGround, nullptr, &backgroundDst);
    SDL_RenderTexture(m_renderer, m_background,&backgroundSrc, &backgroundDst);
}
void MenuState::update(float dt) {
    m_now =SDL_GetTicks();
    if(m_now - m_last > m_aniDelay)
    {
        m_currentFrame = m_currentFrame % 5;
        m_currentFrame++;
        m_last = m_now;
    }
}
void MenuState::handleEvents(SDL_Event &event){

    if(event.type == SDL_EVENT_FINGER_DOWN){
        LOGI("menu starts transitioning to game state");
        Engine::Get().changeState(std::make_unique<GameState>(m_renderer));
        return;
        LOGI("menu state handleEvents end");
    }
}

MenuState::~MenuState() {
        LOGI("Menu destructor");
//        if (backGround)      SDL_DestroyTexture(backGround);
//        if (backGroundSprite)SDL_CloseIO(backGroundSprite);
}

