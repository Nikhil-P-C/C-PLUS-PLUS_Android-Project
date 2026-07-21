//
// Created by LENOVO on 21-07-2026.
//
#include "EditMenuState.h"
#include "engine/Engine.h"
#include "States/MenuState/MenuState.h"


void EditMenuState::render(SDL_Renderer *renderer) {
    SDL_FRect backgroundDst {0.00f,0.00f,1600.00f,720.00f};
    SDL_RenderTexture(renderer,m_backgroundTexture, nullptr,&backgroundDst);

    SDL_FRect editMenuSrc {0,0,682,385};
    SDL_FRect editMenuDst {50,25,1500,675};
    SDL_RenderTexture(renderer,m_editMenuTexture,&editMenuSrc,&editMenuDst);

    SDL_FRect previewTileSrc {0,0,260,265};
    SDL_FRect previewTileDst {1038,35,500,390};
    SDL_RenderTexture(renderer,m_previewTileTexture,&previewTileSrc,&previewTileDst);

    SDL_FRect nameTileSrc{0,0,100,50};
    if(m_saveButtonActive)
        nameTileSrc = {101,0,100,50};
    SDL_FRect nameTileDst {1038,420,500,270};
    SDL_RenderTexture(renderer,m_nameTileTexture,&nameTileSrc,&nameTileDst);

    SDL_FRect backButton ={m_backButton.x,m_backButton.y,m_backButton.w,m_backButton.h};
    SDL_SetRenderDrawColor(renderer,0,0,255,255);
    SDL_RenderFillRect(renderer,&backButton);
}

void EditMenuState::update(float dt) {
    State::update(dt);
}

bool EditMenuState::handleEvents(SDL_Event &event) {
    if(event.type == SDL_EVENT_FINGER_DOWN){
        float TouchX = event.tfinger.x * (float)GameData::getInstance().getWinWidth();
        float TouchY = event.tfinger.y * (float)GameData::getInstance().getWinHeight();
        if(TouchX >= m_backButton.x && TouchX <= m_backButton.x + m_backButton.w &&
            TouchY >= m_backButton.y && TouchY <= m_backButton.y + m_backButton.h){
            int count = (int)Engine::Get().getOverlayStateCount();
            while(count){
                count--;
                LOGI("popping overlay state");
                Engine::Get().popOverlayState();
            }
            Engine::Get().changeState(std::make_unique<MenuState>(m_renderer));
        }
        return true;
    }
    return false;
}

EditMenuState::EditMenuState(SDL_Renderer *renderer) {
    m_renderer =renderer;
    if(!m_backgroundFile)
        LOGI("failed to load background file");
    SDL_Surface* backgroundSurface = IMG_Load_IO(m_backgroundFile, false);
    m_backgroundTexture = SDL_CreateTextureFromSurface(renderer,backgroundSurface);
    SDL_DestroySurface(backgroundSurface);
    SDL_CloseIO(m_backgroundFile);

    if(!m_fontfile)
        LOGI("failed to load font file");
        m_font = TTF_OpenFontIO(m_fontfile,false,18);
    if(!m_editMenuFile)
        LOGI("failed to load editMenu file");
    SDL_Surface* editMenuSurface = IMG_Load_IO(m_editMenuFile,false);
    m_editMenuTexture = SDL_CreateTextureFromSurface(renderer,editMenuSurface);
    SDL_DestroySurface(editMenuSurface);
    SDL_CloseIO(m_editMenuFile);
    if(!m_nameTileFile)
        LOGI("failed to load nameTile file");
    SDL_Surface* nameTileSurface = IMG_Load_IO(m_nameTileFile,false);
    m_nameTileTexture = SDL_CreateTextureFromSurface(renderer,nameTileSurface);
    SDL_DestroySurface(nameTileSurface);
    SDL_CloseIO(m_nameTileFile);
    if(!m_skinTileFile)
        LOGI("failed to load skinTile file");
    SDL_Surface* skinTileSurface = IMG_Load_IO(m_skinTileFile,false);
    m_skinTileTexture = SDL_CreateTextureFromSurface(renderer,skinTileSurface);
    SDL_DestroySurface(skinTileSurface);
    SDL_CloseIO(m_skinTileFile);
    if(!m_previewTileFile)
        LOGI("failed to load previewTile file");
    SDL_Surface* previewTileSurface = IMG_Load_IO(m_previewTileFile,false);
    m_previewTileTexture = SDL_CreateTextureFromSurface(renderer,previewTileSurface);
    SDL_DestroySurface(previewTileSurface);
    SDL_CloseIO(m_previewTileFile);
}

EditMenuState::~EditMenuState() {
    SDL_CloseIO(m_fontfile);
    TTF_CloseFont(m_font);
    SDL_DestroyTexture(m_backgroundTexture);
    SDL_DestroyTexture(m_editMenuTexture);
    SDL_DestroyTexture(m_nameTileTexture);
    SDL_DestroyTexture(m_skinTileTexture);
    SDL_DestroyTexture(m_previewTileTexture);
}
