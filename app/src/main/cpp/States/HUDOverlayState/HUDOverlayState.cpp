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

    int currentHP = PlayerDetail::getInstance().getPlayerHP();
    if (currentHP < 0) currentHP = 0;
    if (currentHP > 5) currentHP = 5;

    if (m_prevHealth > currentHP) {
        // lost one or more hearts - kick off the hurt animation on each newly-lost heart
        for (int i = currentHP; i < m_prevHealth; i++) {
            m_hearts[i].heartAniType = HeartAniType::HURT;
        }
    } else if (m_prevHealth < currentHP) {
        // gained one or more hearts (heal/respawn) - bring the newly-restored ones
        // straight back to IDLE. Previously this used the stale m_lastHeart index and
        // re-idled a heart that was already alive, leaving the actually-restored heart
        // stuck on its LOST frame - which is why the HUD could drift out of sync with HP.
        for (int i = m_prevHealth; i < currentHP; i++) {
            m_hearts[i].heartAniType = HeartAniType::IDLE;
            m_hearts[i].currentFrame = m_hearts[i].animation.startIndex;
        }
    }

    m_prevHealth = currentHP;
    m_lastHeart = currentHP;
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
    this->Name = "HUDState";

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
    LOGI("HUD overlay state constructor:%p",this);

}
HUDOverlayState::~HUDOverlayState() {
    LOGI("HUD overlay state destructor:%p",this);
}