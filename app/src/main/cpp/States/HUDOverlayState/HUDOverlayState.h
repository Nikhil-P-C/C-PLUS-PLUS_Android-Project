//
// Created by LENOVO on 03-08-2026.
//
#pragma once
#include "State/State.h"
#include "States/GameState/GameState.h"
#include <SDL3/SDL.h>
#include <string>
enum class HeartAniType{
    IDLE =0,
    HURT,
    LOST
};

struct Heart {
    Heart(SDL_FRect rect):heartRect(rect){}
    SDL_FRect heartRect{0};
    HeartAniType heartAniType =HeartAniType::IDLE;
    Animation animation{0,0};
    int currentFrame =animation.startIndex;
    unsigned int aniLastTime =0;
};
class HUDOverlayState:public State{
public:
    void render(SDL_Renderer* renderer) override;
    void update(float dt) override;
    bool handleEvents(SDL_Event& event) override;

    void updateAnimation();
    void ensureHearts();                       // one heart per max HP (the shop can add more)
    void refreshFruitText();
    void renderMagicBar(SDL_Renderer* renderer) const;
    void renderFruitCounter(SDL_Renderer* renderer) const;

    HUDOverlayState(SDL_Renderer* renderer);
    ~HUDOverlayState() override;

private:
    SDL_Renderer* m_renderer = nullptr;
    std::vector<Heart> m_hearts;
    int m_lastHeart =0;
    int m_prevHealth=0;
    int m_aniFrameDelay = 50;
    int m_spriteHeight =29, m_spriteWidth=30;

    // magic bar + fruit (the merchant's currency) under the hearts
    TTF_Font*     m_font = nullptr;
    SDL_IOStream* m_fontFile = nullptr;
    SDL_Texture*  m_fruitText = nullptr;
    float m_fruitTextW = 0.0f, m_fruitTextH = 0.0f;
    int   m_shownFruit = -1;
    float m_shownMagic = 0.0f;                 // eased towards the real value so the bar slides instead of jumping

};