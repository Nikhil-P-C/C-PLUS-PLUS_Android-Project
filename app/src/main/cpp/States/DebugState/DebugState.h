//
// Created by LENOVO on 03-05-2026.
//
#pragma once

#include "State/State.h"
#include "States/GameState/GameState.h"

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
    SDL_Texture*  m_fpsTexture       = nullptr;
    TTF_Font*     m_font             = nullptr;
    SDL_IOStream* m_fontFile         = SDL_IOFromFile("Fonts/Pixel Game.otf", "rb");
    int m_framedelay = 1000/60;
    unsigned int m_last = SDL_GetTicks();
    int m_fps =0;
    int m_frames =0;
    int m_lastRenderedFrames=-1;
    static constexpr float JUMP_X = 550.0f, JUMP_Y = 646.0f, JUMP_H = 58.0f;
};
