//
// Created by LENOVO on 03-05-2026.
//
#pragma once

#include "State/State.h"
#include "States/GameState/GameState.h"
#include "utils/PerfStats.h"

#define LOG_TAG "DebugState"

class DebugState : public State{
public:
    DebugState(SDL_Renderer* renderer, GameState *gameState);
    ~DebugState() override;
    void render(SDL_Renderer* renderer) override;
    void update(float dt) override;
    bool handleEvents(SDL_Event& event) override;
    // the level-jump strip only works in the normal in-game overlay stack (TransitionState turns it on)
    void setJumpEnabled(bool enabled){ m_jumpEnabled = enabled; }
private:
    void stepJump(int dir);
    void doJump();
    void renderJumpStrip(SDL_Renderer* renderer);
    // performance overlay: FPS, frame time (avg / min / max / 1% low), where the time goes, frame-time graph, culling, RAM
    void refreshPerfText(SDL_Renderer* renderer);
    void renderPerfPanel(SDL_Renderer* renderer);
    static SDL_FRect perfPanelRect(){ return SDL_FRect{8.0f, 8.0f, 440.0f, 262.0f}; }
    SDL_Renderer* m_renderer = nullptr;
    bool m_jumpEnabled = false;
    int  m_jumpLevel = -1;                    // level shown in the strip (-1 = current level)
    SDL_Texture* m_jumpTexture = nullptr;
    int  m_jumpTextureLevel = -2;
    float m_jumpTexW = 0.0f, m_jumpTexH = 0.0f;
    GameState* m_gameState;
    std::vector<Platform> m_platforms;
    SDL_FRect m_walls;
    std::vector<LevelGround> m_grounds;
    std::vector<Fruit> m_fruits;
//    std::vector<Trap> m_traps;
    TrapBuilder m_trapBuilder;
    Player m_player;
    CheckPoint m_checkPoint;
    TTF_Font*     m_font             = nullptr;
    SDL_IOStream* m_fontFile         = SDL_IOFromFile("Fonts/Pixel Game.otf", "rb");
    // perf panel (own font stream: a TTF_Font owns the read position of the stream it was opened from)
    TTF_Font*     m_perfFont         = nullptr;
    SDL_IOStream* m_perfFontFile     = SDL_IOFromFile("Fonts/Pixel Game.otf", "rb");
    SDL_Texture*  m_perfHeadTex      = nullptr;   // "60 FPS  16.7 ms" coloured by how healthy it is
    SDL_Texture*  m_perfBodyTex      = nullptr;   // the detail lines
    float m_perfHeadW = 0.0f, m_perfHeadH = 0.0f, m_perfBodyW = 0.0f, m_perfBodyH = 0.0f;
    uint64_t m_perfNextRefresh = 0;               // SDL_GetTicks() of the next text rebuild (4x per second)
    bool m_perfVisible = true;                    // F3, or tap the panel to fold it to the header line
    bool m_perfCompact = false;
    std::string m_rendererName;
    int m_framedelay = 1000/60;
    unsigned int m_last = SDL_GetTicks();
    int m_fps =0;
    int m_frames =0;
    static constexpr float JUMP_X = 550.0f, JUMP_Y = 646.0f, JUMP_H = 58.0f;
};
