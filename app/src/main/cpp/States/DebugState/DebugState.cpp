//
// Created by LENOVO on 03-05-2026.
//
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include "DebugState.h"
#include "engine/Engine.h"
#include "utils/InputUtils.h"
#include "level/LevelLoader/LevelLoader.h"


void DebugState::render(SDL_Renderer *renderer) {
    renderJumpStrip(renderer);
    m_player =m_gameState->getPlayer();

    if (m_frames != m_lastRenderedFrames || !m_fpsTexture) {
        if (m_fpsTexture) {
            SDL_DestroyTexture(m_fpsTexture);
            m_fpsTexture = nullptr;
        }
        std::string fps = "FPS:" + std::to_string(m_frames);
        SDL_Surface *fpsSurface = TTF_RenderText_Solid(m_font, fps.c_str(), fps.length(),
                                                       {255, 255, 255});
        if (fpsSurface) {
            m_fpsTexture = SDL_CreateTextureFromSurface(renderer, fpsSurface);
            SDL_DestroySurface(fpsSurface);
        }
        m_lastRenderedFrames = m_frames;
    }
    SDL_FRect fpsrect{0, 0, 100, 100};
    SDL_RenderTexture(renderer, m_fpsTexture, nullptr, &fpsrect);

    int camX = (int) std::round(Camera::getInstance().getCamera().x);
    int camY = (int) std::round(Camera::getInstance().getCamera().y);
    //wall borders
    SDL_SetRenderDrawColor(renderer,255,0,0,255);
    SDL_FRect wallBorder{m_walls.x-camX,m_walls.y-camY,m_walls.w,m_walls.h};
    SDL_FRect innerWallBorder{(m_walls.x+(TILE_SIZE*SCALE))-camX,(m_walls.y+(TILE_SIZE*SCALE))-camY,m_walls.w-(TILE_SIZE*SCALE)*2,m_walls.h-(TILE_SIZE*SCALE)*2};
    SDL_RenderRect(renderer,&wallBorder);
    SDL_RenderRect(renderer,&innerWallBorder);
    //platform
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    for(const auto& platform : m_platforms){
        SDL_FRect dst{platform.x-camX,platform.y-camY,platform.w*SCALE,platform.h};
        SDL_RenderRect(renderer,&dst);
    }
    //ground
    SDL_SetRenderDrawColor(renderer,255,128,0,255);
    for(const auto& ground : m_grounds){
        SDL_FRect dst{ground.x-camX,ground.y-camY,ground.w*SCALE,ground.h*SCALE};
        SDL_RenderRect(renderer,&dst);
    }
    //fruit
    for(const auto& fruit:m_fruits){
        SDL_FRect border{(fruit.x+35)-camX,(fruit.y+35)-camY,(32*SCALE)-70,(32*SCALE)-70};
        SDL_RenderRect(renderer,&border);
    }
    //player
    SDL_FRect playerBorder{m_player.x - camX, m_player.y - camY, m_player.w, m_player.h};
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_RenderRect(renderer, &playerBorder);

    //player hitbox
    const SDL_FRect& playerHitBox = m_gameState->getPlayerHitBox();
    SDL_FRect playerHitBoxBorder ={playerHitBox.x -camX,playerHitBox.y - camY,
                                   playerHitBox.w,playerHitBox.h};
    SDL_SetRenderDrawColor(renderer,0,0,255,255);
    SDL_RenderRect(renderer,&playerHitBoxBorder);

    //beam hitbox
    const SDL_FRect& beamHitBox = m_gameState->getBeamHitBox();
    SDL_FRect beamHitBoxBorder ={beamHitBox.x -camX,beamHitBox.y - camY,
                                 beamHitBox.w,beamHitBox.h};
    SDL_SetRenderDrawColor(renderer,255,0,255,255);
    SDL_RenderRect(renderer,&beamHitBoxBorder);
    //traps
    const std::vector<Trap>& m_traps = m_gameState->getTraps();

    SDL_SetRenderDrawColor(renderer,255,0,0,255);
    for(const auto& trap:m_traps)
    {
        if(m_trapBuilder.trapHasHazard(trap.type,trap.status))
        {
            SDL_FRect trapCollRect =m_trapBuilder.getHazardHitBox(trap);
            trapCollRect.x =trapCollRect.x-camX;
            trapCollRect.y =trapCollRect.y-camY;
            SDL_RenderRect(renderer,&trapCollRect);
        }
    }
    SDL_SetRenderDrawColor(renderer,0,0,255,255);
    for(const auto& trap:m_traps)
    {
        SDL_FRect trapCollRect =m_trapBuilder.getTrapCollisionBox(trap);
        if(m_trapBuilder.trapHasHazard(trap.type,trap.status))
            continue;
        trapCollRect.x =trapCollRect.x-camX;
        trapCollRect.y =trapCollRect.y-camY;
        if(trap.type == TrapType::FAN){
            trapCollRect.y -=700;
            trapCollRect.h +=700+trapCollRect.h;
        }
        SDL_RenderRect(renderer,&trapCollRect);
    }
    //enemies: body (yellow), sprite rect (grey - should sit centred on the body, feet on its bottom),
    //attack reach zone (dim orange, where the player must stand to trigger an attack),
    //live attack hitbox (red, only while it can actually damage the player)
    for(const auto& enemy : m_gameState->getEnemies()){
        if(enemy.isDead) continue;

        SDL_FRect sprite = m_gameState->getEnemyBuilder().getSpriteRect(enemy);
        SDL_FRect spriteBorder{sprite.x-camX,sprite.y-camY,sprite.w,sprite.h};
        SDL_SetRenderDrawColor(renderer,120,120,120,255);
        SDL_RenderRect(renderer,&spriteBorder);

        SDL_FRect body{enemy.x-camX,enemy.y-camY,enemy.w,enemy.h};
        SDL_SetRenderDrawColor(renderer,255,255,0,255);
        SDL_RenderRect(renderer,&body);

        const EnemyAIConfig* cfg = getEnemyAIConfig(enemy.type);
        if(cfg){
            // reach of both of this enemy's attacks (shorter = inner box)
            for(int i = 0; i < 2; ++i){
                SDL_FRect reach = EnemiesBuilder::getAttackBox(enemy, cfg->attackRanges[i]);
                SDL_FRect reachBorder{reach.x-camX,reach.y-camY,reach.w,reach.h};
                SDL_SetRenderDrawColor(renderer,255,140,0,255);
                SDL_RenderRect(renderer,&reachBorder);
            }
        }

        if(enemy.isAttackBoxActive){
            SDL_FRect atk{enemy.attackHitBox.x-camX,enemy.attackHitBox.y-camY,
                          enemy.attackHitBox.w,enemy.attackHitBox.h};
            SDL_SetRenderDrawColor(renderer,255,0,0,255);
            SDL_RenderFillRect(renderer,&atk);
        }
    }

    // bosses: body (cyan), live attack box (red), the fight arena the camera/player are locked to (magenta)
    for(const auto& boss : m_gameState->getBossBuilder().getBosses()){
        if(boss.isDead()) continue;
        SDL_FRect body{boss.x-camX,boss.y-camY,boss.w,boss.h};
        SDL_SetRenderDrawColor(renderer,0,255,255,255);
        SDL_RenderRect(renderer,&body);
        if(boss.isAttackBoxActive){
            SDL_FRect atk{boss.attackHitBox.x-camX,boss.attackHitBox.y-camY,boss.attackHitBox.w,boss.attackHitBox.h};
            SDL_SetRenderDrawColor(renderer,255,0,0,255);
            SDL_RenderFillRect(renderer,&atk);
        }
        if(boss.arenaReady){
            SDL_FRect arena{boss.arena.x-camX,boss.arena.y-camY,boss.arena.w,boss.arena.h};
            SDL_SetRenderDrawColor(renderer,255,0,255,255);
            SDL_RenderRect(renderer,&arena);
        }
    }

    SDL_SetRenderDrawColor(renderer,0,255,0,255);
    SDL_FRect checkPointBorder{m_checkPoint.x-camX,m_checkPoint.y-camY,m_checkPoint.w,m_checkPoint.h};
    SDL_RenderRect(renderer,&checkPointBorder);

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

// ---- DEBUG level jump: "<  LVL n  >  GO" at the bottom centre (touch / mouse), or [ ] to pick and J to go ----
void DebugState::stepJump(int dir){
    if(m_jumpLevel < 0) m_jumpLevel = m_gameState->getLevel();
    m_jumpLevel = (m_jumpLevel + dir + LAST_LEVEL + 1) % (LAST_LEVEL + 1);   // plain numerical order 0..LAST_LEVEL
}

void DebugState::doJump(){
    if(m_jumpLevel < 0) m_jumpLevel = m_gameState->getLevel();
    m_gameState->debugJumpToLevel(m_jumpLevel);
}

void DebugState::renderJumpStrip(SDL_Renderer* renderer){
    if(!m_jumpEnabled || !m_font) return;
    if(m_jumpLevel < 0) m_jumpLevel = m_gameState->getLevel();
    if(m_jumpTextureLevel != m_jumpLevel || !m_jumpTexture){
        if(m_jumpTexture){ SDL_DestroyTexture(m_jumpTexture); m_jumpTexture = nullptr; }
        const std::string text = "LVL " + std::to_string(m_jumpLevel);
        SDL_Surface* s = TTF_RenderText_Solid(m_font, text.c_str(), text.length(), SDL_Color{255, 255, 255, 255});
        if(s){
            m_jumpTexture = SDL_CreateTextureFromSurface(renderer, s);
            m_jumpTexW = (float)s->w; m_jumpTexH = (float)s->h;
            SDL_DestroySurface(s);
        }
        m_jumpTextureLevel = m_jumpLevel;
    }
    SDL_BlendMode prev = SDL_BLENDMODE_NONE;
    SDL_GetRenderDrawBlendMode(renderer, &prev);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    const SDL_FRect bg{JUMP_X, JUMP_Y, 500.0f, JUMP_H};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
    SDL_RenderFillRect(renderer, &bg);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 220);
    SDL_RenderRect(renderer, &bg);
    const SDL_FRect leftB{JUMP_X, JUMP_Y, 56.0f, JUMP_H}, rightB{JUMP_X + 344.0f, JUMP_Y, 56.0f, JUMP_H}, goB{JUMP_X + 410.0f, JUMP_Y, 90.0f, JUMP_H};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 220);
    SDL_RenderRect(renderer, &leftB); SDL_RenderRect(renderer, &rightB);
    SDL_SetRenderDrawColor(renderer, 60, 200, 90, 200);
    SDL_RenderFillRect(renderer, &goB);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderRect(renderer, &goB);
    // arrows (triangles drawn with lines) and a GO chevron
    const float cy = JUMP_Y + JUMP_H * 0.5f;
    for(int i = 0; i < 14; ++i){
        SDL_RenderLine(renderer, leftB.x + 36.0f - (float)i * 0.0f, cy - (float)i, leftB.x + 20.0f, cy);
        SDL_RenderLine(renderer, leftB.x + 36.0f, cy + (float)i, leftB.x + 20.0f, cy);
        SDL_RenderLine(renderer, rightB.x + 20.0f, cy - (float)i, rightB.x + 36.0f, cy);
        SDL_RenderLine(renderer, rightB.x + 20.0f, cy + (float)i, rightB.x + 36.0f, cy);
    }
    if(m_jumpTexture){
        const SDL_FRect t{JUMP_X + 56.0f + (288.0f - m_jumpTexW) * 0.5f, cy - m_jumpTexH * 0.5f, m_jumpTexW, m_jumpTexH};
        SDL_RenderTexture(renderer, m_jumpTexture, nullptr, &t);
    }
    // "GO" as two block letters
    SDL_FRect g1{goB.x + 22.0f, cy - 12.0f, 16.0f, 24.0f}, g2{goB.x + 52.0f, cy - 12.0f, 16.0f, 24.0f};
    SDL_RenderRect(renderer, &g1); SDL_RenderRect(renderer, &g2);
    SDL_SetRenderDrawBlendMode(renderer, prev);
}

bool DebugState::handleEvents(SDL_Event &event) {
    if(!m_jumpEnabled) return false;
    if(event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat){
        if(event.key.key == SDLK_LEFTBRACKET)  { stepJump(-1); return true; }
        if(event.key.key == SDLK_RIGHTBRACKET) { stepJump(1);  return true; }
        if(event.key.key == SDLK_P)            { doJump();     return true; }
    }
    if(InputUtils::IsPointerDown(event)){
        float x, y;
        InputUtils::GetPointerPosition(event, m_renderer, x, y);
        if(y < JUMP_Y || y > JUMP_Y + JUMP_H || x < JUMP_X || x > JUMP_X + 500.0f) return false;
        if(x < JUMP_X + 56.0f)              stepJump(-1);
        else if(x > JUMP_X + 410.0f)        doJump();
        else if(x > JUMP_X + 344.0f)        stepJump(1);
        return true;                                           // taps on the strip never reach the game
    }
    return false;
}

DebugState::DebugState(SDL_Renderer *renderer, GameState *gameState) {
    this->Name = "DebugState";
    m_renderer = renderer;

    LOGI("gameState ptr passed in debug:%p",gameState);

    m_gameState =gameState;
    m_grounds=m_gameState->getLevelGrounds();
    m_walls=m_gameState->getLevelWalls();
    m_platforms=m_gameState->getPlatforms();
    m_fruits=m_gameState->getFruits();
    m_trapBuilder =m_gameState->getTrapBuilder();
//    m_traps =m_trapBuilder.getTraps();
    m_checkPoint =gameState->getCheckPoint();
    LOGI("Debug state constructor :%p",this);
    m_font = TTF_OpenFontIO(m_fontFile,false, 24);
    LOGI("font:%d",(bool)m_font);
    if(!m_font)LOGE("failed to load font");
}
DebugState::~DebugState(){
    if(m_jumpTexture) SDL_DestroyTexture(m_jumpTexture);
    SDL_CloseIO(m_fontFile);
    TTF_CloseFont(m_font);
    if(m_fpsTexture)
        SDL_DestroyTexture(m_fpsTexture);
    LOGI("debug state destructor :%p",this);
}