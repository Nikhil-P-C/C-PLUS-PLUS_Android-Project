//
// Created by LENOVO on 10-08-2026.
//
#include "AdvanceMenuState.h"
#include "engine/Engine.h"
void AdvanceMenuState::render(SDL_Renderer *renderer) {
    SDL_FRect menuDst ={100.00f,25.00f,1400.00f,670.00f};
    SDL_FRect menuSrc ={804.00f,0.00f,402.00f,198.00f};
    SDL_RenderTexture(renderer,m_menuTexture,&menuSrc,&menuDst);

    {
        SDL_FRect optionBlockDst = {330.f, 475.00f, 1100.00f, 150.00f};

        SDL_FRect optionBlockSrc = {0.00f, 0.00f, 185.00f, 34.00f};
        SDL_RenderTexture(renderer, m_optionBlockTexture, &optionBlockSrc, &optionBlockDst);
    }

    SDL_FRect fontDst = {400.00f, m_debugButton.y+(50.00f-15.00f), 100.00f,30.00f};
    SDL_RenderTexture(renderer, m_debugFontTexture, nullptr, &fontDst);

    //debug button
    {
        SDL_FRect radioButtonDst = {m_debugButton.x + 912.50f, m_debugButton.y+ 50.00f-(45.00f/2.00f),
                                    9.00f * 5.00f, 9.00f * 5.00f};
        SDL_FRect radioButtonSrc = {10.00f, 0.00f, 9.00f, 9.00f};
        radioButtonSrc.x = GameData::getInstance().isDebugEnabled() ? 0.00f : 10.00f;
        SDL_RenderTexture(renderer, m_radioButtonTexture, &radioButtonSrc, &radioButtonDst);
    }
}

void AdvanceMenuState::update(float dt) {

}

bool AdvanceMenuState::handleEvents(SDL_Event &event) {
    if(InputUtils::IsPointerDown(event)){
        float touchX, touchY;
        InputUtils::GetPointerPosition(event, m_renderer, touchX, touchY);

        if(touchX >= m_debugButton.x && touchX <= m_debugButton.x + m_debugButton.w &&
           touchY >= m_debugButton.y && touchY <= m_debugButton.y + m_debugButton.h){
            GameData::getInstance().toggleDebug();
            LOGI("toggle debug:%d",GameData::getInstance().isDebugEnabled());
            return true;
        }
    }

    return false;
}

AdvanceMenuState::AdvanceMenuState(SDL_Renderer *renderer) {
    this->Name = "AdvanceMenuState";

    m_renderer = renderer;
    m_menuTexture=Engine::Get().getAssetManager().getTexture(TextureType::OPTION_MENU_TILE);
    m_optionBlockTexture = Engine::Get().getAssetManager().getTexture(TextureType::MENU_OPTION_BLOCK_TILE);
    m_radioButtonTexture = Engine::Get().getAssetManager().getTexture(TextureType::MENU_RADIO_BUTTON);

    m_font = TTF_OpenFontIO(m_fontfile,false,26);
    LOGI("Loaded font %p", m_font);
    if (!m_font)
        LOGI("%s", SDL_GetError());

    SDL_Surface* debugFontSurface = TTF_RenderText_Solid(m_font, "Debug",5,
                                                         SDL_Color{0, 0, 0, 255});
    m_debugFontTexture = SDL_CreateTextureFromSurface(renderer, debugFontSurface);
    SDL_DestroySurface(debugFontSurface);

    SDL_SetTextureScaleMode(m_debugFontTexture,SDL_SCALEMODE_NEAREST);
}

AdvanceMenuState::~AdvanceMenuState() {

}
