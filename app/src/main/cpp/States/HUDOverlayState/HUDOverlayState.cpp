//
// Created by LENOVO on 03-08-2026.
//
#include "HUDOverlayState.h"
void HUDOverlayState::render(SDL_Renderer *renderer) {
    m_renderer =renderer;
    for(size_t i = 0; i < m_hearts.size(); i++){
        SDL_FRect dst = m_hearts[i].heartRect;
        SDL_FRect src{0.00f + (float)m_hearts[i].currentFrame * m_spriteWidth,0.0f,
                      (float)m_spriteWidth,(float)m_spriteHeight};
        SDL_RenderTexture(renderer,Engine::Get().getAssetManager().getTexture(TextureType::HUD_HEALTH_HEART),
                          &src,&dst);
    }
    renderMagicBar(renderer);
    renderFruitCounter(renderer);
}

void HUDOverlayState::renderMagicBar(SDL_Renderer* renderer) const {
    const PlayerDetail& pd = PlayerDetail::getInstance();
    const float frac = std::clamp(m_shownMagic / pd.getMaxMagic(), 0.0f, 1.0f);
    const SDL_FRect box{30.0f, 82.0f, 300.0f, 14.0f};
    SDL_BlendMode prev = SDL_BLENDMODE_NONE;
    SDL_GetRenderDrawBlendMode(renderer, &prev);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 10, 8, 24, 170);
    SDL_RenderFillRect(renderer, &box);
    const SDL_FRect fill{box.x + 2.0f, box.y + 2.0f, (box.w - 4.0f) * frac, box.h - 4.0f};
    SDL_SetRenderDrawColor(renderer, 120, 110, 255, 235);
    SDL_RenderFillRect(renderer, &fill);
    const SDL_FRect shine{fill.x, fill.y, fill.w, (box.h - 4.0f) * 0.4f};
    SDL_SetRenderDrawColor(renderer, 190, 185, 255, 120);
    SDL_RenderFillRect(renderer, &shine);
    SDL_SetRenderDrawColor(renderer, 190, 185, 255, 220);
    SDL_RenderRect(renderer, &box);
    SDL_SetRenderDrawBlendMode(renderer, prev);
}

void HUDOverlayState::renderFruitCounter(SDL_Renderer* renderer) const {
    SDL_Texture* apple = Engine::Get().getAssetManager().getTexture(TextureType::FRUIT_APPLE);
    if(apple){
        const int frame = (int)(SDL_GetTicks() / 55) % 17;                    // the same spin the pickup has
        const SDL_FRect src{(float)frame * 32.0f, 0.0f, 32.0f, 32.0f};
        const SDL_FRect dst{28.0f, 104.0f, 40.0f, 40.0f};
        SDL_RenderTexture(renderer, apple, &src, &dst);
    }
    if(m_fruitText){
        const SDL_FRect dst{76.0f, 108.0f + (40.0f - m_fruitTextH) * 0.5f - 4.0f, m_fruitTextW, m_fruitTextH};
        SDL_RenderTexture(renderer, m_fruitText, nullptr, &dst);
    }
}

void HUDOverlayState::refreshFruitText(){
    const int fruit = PlayerDetail::getInstance().getFruit();
    if(fruit == m_shownFruit || !m_font) return;
    m_shownFruit = fruit;
    if(m_fruitText){ SDL_DestroyTexture(m_fruitText); m_fruitText = nullptr; }
    const std::string text = "x " + std::to_string(fruit);
    if(SDL_Surface* s = TTF_RenderText_Solid(m_font, text.c_str(), 0, SDL_Color{255, 255, 255, 255})){
        m_fruitText = SDL_CreateTextureFromSurface(m_renderer, s);
        m_fruitTextW = (float)s->w; m_fruitTextH = (float)s->h;
        SDL_DestroySurface(s);
    }
}

// hearts follow the max HP: 5 at the start, up to 8 with the shop's heart upgrades
void HUDOverlayState::ensureHearts(){
    const int wanted = PlayerDetail::getInstance().getMaxHP();
    while((int)m_hearts.size() < wanted){
        const float x = 25.0f + 70.0f * (float)m_hearts.size();
        m_hearts.emplace_back(SDL_FRect{x, 0.0f, 75.0f, 75.0f});
    }
}

void HUDOverlayState::update(float dt) {
    ensureHearts();
    refreshFruitText();
    {   // ease the bar towards the real magic value
        const float target = PlayerDetail::getInstance().getMagic();
        m_shownMagic += (target - m_shownMagic) * std::min(1.0f, dt * 10.0f);
        if(std::fabs(target - m_shownMagic) < 0.2f) m_shownMagic = target;
    }
    updateAnimation();

    int currentHP = PlayerDetail::getInstance().getPlayerHP();
    if (currentHP < 0) currentHP = 0;
    if (currentHP > (int)m_hearts.size()) currentHP = (int)m_hearts.size();

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
    m_hearts.reserve(Upgrades::BASE_HEARTS + Upgrades::MAX_LEVEL[Upgrades::HEART]);
    ensureHearts();

    m_lastHeart=PlayerDetail::getInstance().getPlayerHP();
    m_prevHealth=m_lastHeart;
    for (int i = 0; i < (int)m_hearts.size(); ++i) {
        if(i >= m_lastHeart)
            m_hearts[i].heartAniType = HeartAniType::LOST;
    }
    m_shownMagic = PlayerDetail::getInstance().getMagic();
    m_fontFile = SDL_IOFromFile("Fonts/Pixel Game.otf", "rb");
    if(m_fontFile) m_font = TTF_OpenFontIO(m_fontFile, false, 30);
    refreshFruitText();
    LOGI("HUD overlay state constructor:%p",this);

}
HUDOverlayState::~HUDOverlayState() {
    if(m_fruitText) SDL_DestroyTexture(m_fruitText);
    if(m_font) TTF_CloseFont(m_font);
    if(m_fontFile) SDL_CloseIO(m_fontFile);
    LOGI("HUD overlay state destructor:%p",this);
}