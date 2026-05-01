
#include "States/MenuState/MenuState.h"
#include "GameState.h"
#include "States/PauseState/PauseState.h"

//
// Created by LENOVO on 27-04-2026.
//
GameState::GameState(SDL_Renderer *renderer) {
    m_renderer = renderer;
    LOGI("game state constructor");
    const SDL_DisplayMode* mode =
            SDL_GetCurrentDisplayMode(SDL_GetPrimaryDisplay());
    if(mode)
    {
        LOGI("Display native: %d x %d", mode->w, mode->h);
        m_windowW =mode->w;
        m_windowH = mode->h;
    }
    //init player attributes
    m_player.setSize(SPRITE_WIDTH * P_scale, SPRITE_HEIGHT * P_scale);
    m_player.setPosition(100,0,m_windowH,SPRITE_HEIGHT,P_scale);

    //font loading
    m_font = TTF_OpenFontIO(m_fontFile, false, 36);
    LOGI("font:%d",(bool)m_font);

    //background sprite loading;
    SDL_Surface* Backgroundsurface = IMG_Load_IO(m_backGroundSprite, false);
    if(!Backgroundsurface)return;
    m_backGround = SDL_CreateTextureFromSurface(renderer, Backgroundsurface);
    if(!m_backGround)return;
    SDL_DestroySurface(Backgroundsurface);

    //player sprite loading
    SDL_Surface* playerSurface = IMG_Load_IO(m_playerSprite, false);
    if(!playerSurface)return;
    m_playerTexture = SDL_CreateTextureFromSurface(renderer, playerSurface);
    if(!m_playerTexture)return;
    SDL_DestroySurface(playerSurface);

    // m_tileset loading
    SDL_Surface* Tilesurface = IMG_Load_IO(m_tilesetSprite, false);
    if(!Tilesurface)return;
    m_tileset = SDL_CreateTextureFromSurface(renderer, Tilesurface);
    if(!m_tileset)return;
    SDL_DestroySurface(Tilesurface);

    SDL_SetTextureScaleMode(m_playerTexture, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_tileset, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_fpsTexture, SDL_SCALEMODE_NEAREST);

    if(!m_playerSprite)LOGE("cannot load sprite");


}

void GameState::render(SDL_Renderer* renderer)  {
    SDL_FRect backgroundDst{0, 0, 1600, 720};
    SDL_RenderTexture(renderer, m_backGround, nullptr, &backgroundDst);


    SDL_FRect dst = {m_player.x,m_player.y,m_player.w,m_player.h};

    SDL_FRect src = {(float) (0 + (SPRITE_WIDTH * m_currentFrame)), 0, SPRITE_WIDTH, SPRITE_HEIGHT};
//    SDL_FRect src = {24, 0, SPRITE_WIDTH, SPRITE_HEIGHT};
    SDL_RenderTexture(renderer, m_playerTexture, &src, &dst);
}

void GameState::update(float dt){
    m_currentFrame = m_currentFrame%16;
    m_currentFrame++;
}

void GameState::handleEvents(SDL_Event& event) {

    if(event.key.key == SDLK_AC_BACK){
        LOGI("game state transitions to menu state");
        Engine::Get().pushState(std::make_unique<PauseState>(m_renderer));
    }
}






