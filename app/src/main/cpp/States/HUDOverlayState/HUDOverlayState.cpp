//
// Created by LENOVO on 03-08-2026.
//
#include "HUDOverlayState.h"
void HUDOverlayState::render(SDL_Renderer *renderer) {
    m_renderer =renderer;
    for(int i = 0;i< 5; i++){
        SDL_FRect dst = m_hearts[i].heartRect;
        SDL_FRect src{0.00f + (float)m_hearts[i].currentFrame * m_spriteWidth,0.0f,
                      (float)m_spriteWidth,(float)m_spriteHeight};
        SDL_RenderTexture(renderer,Engine::Get().getAssetManager().getTexture(TextureType::HUD_HEALTH_HEART),
                          &src,&dst);
    }
}

void HUDOverlayState::update(float dt) {
    updateAnimation();
    if (PlayerDetail::getInstance().getPlayerHP() == 0) {
        m_lastHeart = 5;
        for (size_t i = 0; i < 5; i++) {

            m_hearts[i].heartAniType = HeartAniType::IDLE;
            m_hearts[i].currentFrame = 0;
        }
        return;
    }
    if (m_prevHealth > PlayerDetail::getInstance().getPlayerHP()) {
        m_hearts[m_lastHeart - 1].heartAniType = HeartAniType::HURT;
    } else if (m_prevHealth < PlayerDetail::getInstance().getPlayerHP()) {
        if (m_lastHeart > 5) m_lastHeart = 5;
        m_hearts[m_lastHeart - 1].heartAniType = HeartAniType::IDLE;
    }
    m_prevHealth = PlayerDetail::getInstance().getPlayerHP();
    m_lastHeart = m_prevHealth;
}

bool HUDOverlayState::handleEvents(SDL_Event &event) {

    return false;
}
void HUDOverlayState::updateAnimation() {
    for (auto& heart : m_hearts)
    {
        switch (heart.heartAniType) {
            case HeartAniType::IDLE:
                heart.animation.startIndex = 0;
                heart.animation.lastIndex = 3;
                break;
            case HeartAniType::HURT:
                heart.animation.startIndex = 4;
                heart.animation.lastIndex = 7;
                break;
            case HeartAniType::LOST:
                heart.animation.startIndex = 8;
                heart.animation.lastIndex = 8;
                break;

        }
    }
    unsigned int m_aniNowTime = SDL_GetTicks();
    for(auto& heart:m_hearts)
    {
        if((heart.currentFrame == heart.animation.lastIndex)&& heart.heartAniType == HeartAniType::HURT)
            heart.heartAniType =HeartAniType::LOST;
        if (m_aniNowTime - heart.aniLastTime > m_aniFrameDelay) {
            if (heart.currentFrame < heart.animation.startIndex)
                heart.currentFrame = heart.animation.startIndex;
            if (heart.currentFrame < heart.animation.lastIndex)
                heart.currentFrame++;
            else
                heart.currentFrame =heart.animation.startIndex;
            heart.aniLastTime = m_aniNowTime;
        }

    }
}

HUDOverlayState::HUDOverlayState(SDL_Renderer *renderer) {
    m_renderer =renderer;
    m_hearts.reserve(5);
    m_hearts.emplace_back(SDL_FRect{25.00f,0.00f,75.00f,75.00f});
    m_hearts.emplace_back(SDL_FRect{95.00f,0.00f,75.00f,75.00f});
    m_hearts.emplace_back(SDL_FRect{165.00f,0.00f,75.00f,75.00f});
    m_hearts.emplace_back(SDL_FRect{235.00f,0.00f,75.00f,75.00f});
    m_hearts.emplace_back(SDL_FRect{305.00f,0.00f,75.00f,75.00f});

    m_lastHeart=PlayerDetail::getInstance().getPlayerHP();
    m_prevHealth=m_lastHeart;
    for (int i = 0; i <5; ++i) {
        if(i >= m_lastHeart)
            m_hearts[i].heartAniType = HeartAniType::LOST;
    }
}
HUDOverlayState::~HUDOverlayState() {
    LOGI("HUD overlay state destructor:%p",this);
}


