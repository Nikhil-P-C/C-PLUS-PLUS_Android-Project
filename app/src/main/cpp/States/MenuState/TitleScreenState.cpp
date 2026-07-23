//
// Created by LENOVO on 28-04-2026.
//
#include "TitleScreenState.h"
#include "States/GameState/GameState.h"
#include "MenuState.h"

void TitleScreenState::render(SDL_Renderer *renderer) {
    LOGI("inside title render");
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
    LOGI("inside title update");

    m_now =SDL_GetTicks();
    if(m_now - m_last > m_aniDelay)
    {
        m_currentFrame = m_currentFrame % 5;
        m_currentFrame++;
        m_last = m_now;
    }
}
bool TitleScreenState::handleEvents(SDL_Event &event){
    LOGI("inside title event handler");

    if(event.type == SDL_EVENT_FINGER_DOWN){

        Engine::Get().changeState(std::make_unique<MenuState>(m_renderer));
        return true;

    }
    return false;
}
TitleScreenState::TitleScreenState(SDL_Renderer *renderer){
    m_renderer = renderer;
    LOGI("TitleScreenState constructor:%p",this);
    LOGI("inside title render");

    m_background = Engine::Get().getAssetManager().getTexture(TextureType::BG_CONFETTI_ANIMATED);
    LOGI("inside title render");

    if(m_background){
        LOGI("bg loaded");
    }
    else
        LOGI("failed to load bg");
    SDL_SetTextureScaleMode(m_background,SDL_SCALEMODE_NEAREST);

}
TitleScreenState::~TitleScreenState() {
        LOGI("TitleScreenState destructor:%p",this);
}

