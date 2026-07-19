//
// Created by LENOVO on 03-05-2026.
//
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <strings.h>
#include <string>
#include "DebugState.h"
#include "engine/Engine.h"


void DebugState::render(SDL_Renderer *renderer) {
    m_player =m_gameState->getPlayer();
    std::string fps = "FPS:" + std::to_string(m_frames);
    SDL_Surface *fpsSurface = TTF_RenderText_Solid(m_font, fps.c_str(), fps.length(),
                                                   {255, 255, 255});
    m_fpsTexture = SDL_CreateTextureFromSurface(renderer, fpsSurface);
    SDL_Color fpsColor = {255, 255, 255, 255};
    SDL_FRect fpsrect{0, 0, 100, 100};
    SDL_RenderTexture(renderer, m_fpsTexture, nullptr, &fpsrect);
    SDL_DestroyTexture(m_fpsTexture);
    SDL_DestroySurface(fpsSurface);
    int camX = (int) std::round(Camera::getInstance().getCamera().x);
    int camY = (int) std::round(Camera::getInstance().getCamera().y);
    //wall borders
    SDL_SetRenderDrawColor(renderer,255,0,0,255);
    SDL_FRect wallBorder{m_walls.x-camX,m_walls.y-camY,m_walls.w,m_walls.h};
    SDL_FRect innerWallBorder{(m_walls.x+(TILE_SIZE*SCALE))-camX,(m_walls.y+(TILE_SIZE*SCALE))-camY,m_walls.w-(TILE_SIZE*SCALE)*2,m_walls.h-(TILE_SIZE*SCALE)*2};
    SDL_RenderRect(renderer,&wallBorder);
    SDL_RenderRect(renderer,&innerWallBorder);
    //platform

    for (int i = 0; i < 10; i++) {
        int tileSize = TILE_SIZE;
        const int platformWidth = (int) m_platforms[i].w;
        const int platformHeight = (int) m_platforms[i].h;
        int widthTiles = platformWidth / tileSize;
        int heightTiles = platformHeight / tileSize;

        for (int y = 0; y < heightTiles; y++) {
            for (int x = 0; x < widthTiles; x++) {

//          SDL_FRect dst = {m_platforms[j].x + j * (48*SCALE), m_platforms[j].y, 48*SCALE, 48*SCALE};
                SDL_FRect dst = {(m_platforms[i].x + x * (TILE_SIZE * SCALE)) - camX,
                                 (m_platforms[i].y + y * (TILE_SIZE * SCALE)) - camY,
                                 TILE_SIZE * SCALE, TILE_SIZE * SCALE};
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                SDL_RenderRect(renderer, &dst);
            }

        }
    }
    //ground
    SDL_SetRenderDrawColor(renderer,255,128,0,255);
    for(const auto& ground : m_grounds){
        SDL_FRect dst{ground.x-camX,ground.y-camY,ground.w*SCALE,ground.h*SCALE};
        SDL_RenderRect(renderer,&dst);
    }

    SDL_FRect playerBorder{m_player.x - camX, m_player.y - camY, m_player.w, m_player.h};
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_RenderRect(renderer, &playerBorder);
    SDL_SetRenderDrawColor(renderer, 33, 31, 48, 255);

}

void DebugState::update(float dt) {
    m_fps++;
    unsigned int current = SDL_GetTicks();
    if(current - m_last >= 1000){

        m_frames =m_fps;
        m_fps =0;
        m_last = current;
    }
}

bool DebugState::handleEvents(SDL_Event &event) {

    return false;
}

DebugState::DebugState(SDL_Renderer *renderer, GameState *gameState) {
    m_gameState =gameState;
    m_grounds=m_gameState->getLevelGrounds();
    m_walls=m_gameState->getLevelWalls();
    m_platforms=m_gameState->getPlatforms();

    LOGI("Debug state constructor :%p",this);
    m_font = TTF_OpenFontIO(m_fontFile,false, 24);
    LOGI("font:%d",(bool)m_font);
    if(!m_font)LOGE("failed to load font");
}
DebugState::~DebugState(){
    SDL_CloseIO(m_fontFile);
    TTF_CloseFont(m_font);
    SDL_DestroyTexture(m_fpsTexture);
    LOGI("debug state destructor :%p",this);
}

