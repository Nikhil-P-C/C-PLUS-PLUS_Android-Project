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
#include "utils/PerfStats.h"
#include "utils/Cull.h"
#include <cstdio>


void DebugState::render(SDL_Renderer *renderer) {
    renderJumpStrip(renderer);
    m_player =m_gameState->getPlayer();

    renderPerfPanel(renderer);

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
        // blade slash: the blade itself in every phase (white = hurts, grey = wind-up / return, harmless). The red box above is its sweep.
        if(boss.bladePhase > 0 && boss.bladeNow.w > 0.0f){
            SDL_FRect blade{boss.bladeNow.x-camX,boss.bladeNow.y-camY,boss.bladeNow.w,boss.bladeNow.h};
            const bool hurts = boss.bladePhase == 2 || boss.bladePhase == 3;
            const Uint8 v = hurts ? 255 : 150;
            SDL_SetRenderDrawColor(renderer,v,v,v,255);
            SDL_RenderRect(renderer,&blade);
        }
        // beams: the aim line while it is only a tell (orange), centre line + the two edges of its width while it hurts (red)
        if(boss.beamAim || boss.beamOn){
            const float bx1 = boss.beamX1-camX, by1 = boss.beamY1-camY, bx2 = boss.beamX2-camX, by2 = boss.beamY2-camY;
            SDL_SetRenderDrawColor(renderer,255,boss.beamOn ? 0 : 140,0,255);
            SDL_RenderLine(renderer,bx1,by1,bx2,by2);
            if(boss.beamOn){
                const float dx = bx2-bx1, dy = by2-by1;
                const float len = std::max(1.0f, std::sqrt(dx*dx+dy*dy));
                const float nx = -dy/len*boss.beamThick*0.5f, ny = dx/len*boss.beamThick*0.5f;
                SDL_RenderLine(renderer,bx1+nx,by1+ny,bx2+nx,by2+ny);
                SDL_RenderLine(renderer,bx1-nx,by1-ny,bx2-nx,by2-ny);
            }
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

// ---- performance panel ------------------------------------------------------------------------------------------------
namespace {
    SDL_Texture* makeText(SDL_Renderer* r, TTF_Font* font, const char* text, SDL_Color c, bool wrapped, float& w, float& h){
        SDL_Surface* s = wrapped ? TTF_RenderText_Solid_Wrapped(font, text, 0, c, 0)
                                 : TTF_RenderText_Solid(font, text, 0, c);
        if(!s) return nullptr;
        SDL_Texture* t = SDL_CreateTextureFromSurface(r, s);
        w = (float)s->w; h = (float)s->h;
        SDL_DestroySurface(s);
        return t;
    }
}

// rebuilt 4x per second so the numbers are readable (and so we are not creating textures every frame)
void DebugState::refreshPerfText(SDL_Renderer* renderer){
    if(!m_perfFont) return;
    PerfStats& perf = PerfStats::get();
    const PerfStats::Summary sm = perf.summarize();

    // header colour: green = holding 60, yellow = dipping, red = struggling
    SDL_Color head{90, 230, 120, 255};
    if(sm.avgMs > 20.0f)      head = {255, 80, 70, 255};
    else if(sm.avgMs > 17.5f) head = {255, 210, 70, 255};
    char buf[768];
    std::snprintf(buf, sizeof(buf), "%.0f FPS   %.1f ms", sm.fps, sm.avgMs);
    if(m_perfHeadTex){ SDL_DestroyTexture(m_perfHeadTex); m_perfHeadTex = nullptr; }
    m_perfHeadTex = makeText(renderer, m_perfFont, buf, head, false, m_perfHeadW, m_perfHeadH);

    if(m_perfBodyTex){ SDL_DestroyTexture(m_perfBodyTex); m_perfBodyTex = nullptr; }
    if(m_perfCompact) return;

    int aliveEnemies = 0;
    for(const auto& e : m_gameState->getEnemies()) if(!e.isDead) ++aliveEnemies;
    const uint32_t tested = perf.cullTested(), rejected = perf.cullRejected();
    const float culledPct = tested ? 100.0f * (float)rejected / (float)tested : 0.0f;
    const float ram = PerfStats::residentMB();

    int n = 0;
    n += std::snprintf(buf + n, sizeof(buf) - n, "frame  min %.1f  max %.1f  p99 %.1f ms\n", sm.minMs, sm.maxMs, sm.p99Ms);
    n += std::snprintf(buf + n, sizeof(buf) - n, "1%% low %.0f FPS   hitches >%.0fms: %d\n", sm.lowFps, PerfStats::HITCH_MS, sm.hitches);
    n += std::snprintf(buf + n, sizeof(buf) - n, "work %.1f ms  (%.0f%% of %.1f budget)\n", perf.workMs(),
                       100.0f * perf.workMs() / PerfStats::BUDGET_MS, PerfStats::BUDGET_MS);
    n += std::snprintf(buf + n, sizeof(buf) - n, "update %.1f  events %.1f  present %.1f\n",
                       perf.section(PerfStats::UPDATE), perf.section(PerfStats::EVENTS), perf.section(PerfStats::PRESENT));
    n += std::snprintf(buf + n, sizeof(buf) - n, "render world %.1f  overlays %.1f\n",
                       perf.section(PerfStats::RENDER_STATES), perf.section(PerfStats::RENDER_OVERLAYS));
    n += std::snprintf(buf + n, sizeof(buf) - n, "culled %u / %u (%.0f%%)\n", rejected, tested, culledPct);
    n += std::snprintf(buf + n, sizeof(buf) - n, "enemies %d  traps %d  bosses %d\n", aliveEnemies,
                       (int)m_gameState->getTraps().size(), (int)m_gameState->getBossBuilder().getBosses().size());
    if(ram > 0.0f) n += std::snprintf(buf + n, sizeof(buf) - n, "RAM %.0f MB   ", ram);
    n += std::snprintf(buf + n, sizeof(buf) - n, "%s", m_rendererName.c_str());
    m_perfBodyTex = makeText(renderer, m_perfFont, buf, SDL_Color{235, 235, 235, 255}, true, m_perfBodyW, m_perfBodyH);
}

void DebugState::renderPerfPanel(SDL_Renderer* renderer){
    if(!m_perfVisible) return;
    const uint64_t now = SDL_GetTicks();
    if(now >= m_perfNextRefresh || !m_perfHeadTex){
        refreshPerfText(renderer);
        m_perfNextRefresh = now + 250;
    }

    constexpr float PAD = 8.0f, GRAPH_H = 56.0f, GRAPH_BARS = 120.0f;
    const SDL_FRect box = perfPanelRect();
    const float bodyH = m_perfCompact ? 0.0f : m_perfBodyH;
    const float graphH = m_perfCompact ? 0.0f : GRAPH_H + PAD;
    const SDL_FRect bg{box.x, box.y, box.w, PAD * 2 + m_perfHeadH + (bodyH > 0 ? bodyH + PAD * 0.5f : 0.0f) + graphH};

    SDL_BlendMode prev = SDL_BLENDMODE_NONE;
    SDL_GetRenderDrawBlendMode(renderer, &prev);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 170);
    SDL_RenderFillRect(renderer, &bg);

    float y = bg.y + PAD;
    if(m_perfHeadTex){
        const SDL_FRect d{bg.x + PAD, y, m_perfHeadW, m_perfHeadH};
        SDL_RenderTexture(renderer, m_perfHeadTex, nullptr, &d);
    }
    y += m_perfHeadH;
    if(!m_perfCompact){
        if(m_perfBodyTex){
            y += PAD * 0.5f;
            const SDL_FRect d{bg.x + PAD, y, m_perfBodyW, m_perfBodyH};
            SDL_RenderTexture(renderer, m_perfBodyTex, nullptr, &d);
            y += m_perfBodyH;
        }
        // frame-time graph: one bar per frame (newest on the right), 0..50 ms. Lines mark 60 fps and 30 fps.
        y += PAD;
        constexpr float MAX_MS = 50.0f;
        const float barW = (bg.w - PAD * 2) / GRAPH_BARS;
        float ms[(int)GRAPH_BARS];
        const int n = PerfStats::get().recent(ms, (int)GRAPH_BARS);
        for(int i = 0; i < n; ++i){
            const float h = std::min(ms[i], MAX_MS) / MAX_MS * GRAPH_H;
            if(ms[i] <= 17.5f)                 SDL_SetRenderDrawColor(renderer, 90, 230, 120, 230);
            else if(ms[i] < PerfStats::HITCH_MS) SDL_SetRenderDrawColor(renderer, 255, 210, 70, 230);
            else                               SDL_SetRenderDrawColor(renderer, 255, 80, 70, 230);
            const SDL_FRect bar{bg.x + PAD + (GRAPH_BARS - (float)n + (float)i) * barW, y + GRAPH_H - h, std::max(1.0f, barW - 1.0f), h};
            SDL_RenderFillRect(renderer, &bar);
        }
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 90);
        const float y60 = y + GRAPH_H - (1000.0f / 60.0f) / MAX_MS * GRAPH_H;
        const float y30 = y + GRAPH_H - (1000.0f / 30.0f) / MAX_MS * GRAPH_H;
        SDL_RenderLine(renderer, bg.x + PAD, y60, bg.x + bg.w - PAD, y60);
        SDL_RenderLine(renderer, bg.x + PAD, y30, bg.x + bg.w - PAD, y30);
    }
    SDL_SetRenderDrawBlendMode(renderer, prev);
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
    // performance panel: F3 hides / shows it, tapping (or clicking) it folds it to the header line and back
    if(event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat && event.key.key == SDLK_F3){
        m_perfVisible = !m_perfVisible;
        return true;
    }
    if(m_perfVisible && InputUtils::IsPointerDown(event)){
        float x, y;
        InputUtils::GetPointerPosition(event, m_renderer, x, y);
        const SDL_FRect r = perfPanelRect();
        if(x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + (m_perfCompact ? 40.0f : r.h)){
            m_perfCompact = !m_perfCompact;
            m_perfNextRefresh = 0;     // rebuild the text right away
            return true;
        }
    }
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
    m_perfFont = TTF_OpenFontIO(m_perfFontFile, false, 20);
    if(!m_perfFont) LOGE("failed to load perf font");
    if(const char* name = SDL_GetRendererName(renderer)) m_rendererName = name;
}
DebugState::~DebugState(){
    if(m_jumpTexture) SDL_DestroyTexture(m_jumpTexture);
    SDL_CloseIO(m_fontFile);
    TTF_CloseFont(m_font);
    if(m_perfHeadTex) SDL_DestroyTexture(m_perfHeadTex);
    if(m_perfBodyTex) SDL_DestroyTexture(m_perfBodyTex);
    if(m_perfFont)    TTF_CloseFont(m_perfFont);
    if(m_perfFontFile) SDL_CloseIO(m_perfFontFile);
    LOGI("debug state destructor :%p",this);
}