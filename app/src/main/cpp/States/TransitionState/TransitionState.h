// TransitionState.h
#pragma once
#include "State/State.h"
#include <SDL3/SDL.h>
#include <vector>
class GameState;//forward declare;
class TransitionState : public State {
public:
    TransitionState(SDL_Renderer* renderer, int level);
    ~TransitionState() override;

    void render(SDL_Renderer* renderer) override;
    void update(float dt) override;
    bool handleEvents(SDL_Event& event) override;

private:
    struct Diamond {
        float cx, cy;
        float delayOffset;  // ms, staggers this diamond relative to the center
        float currentSize;  // current width/height, recomputed each update()
    };

    void buildGrid();
    float computeSize(float localT, float phaseDuration, bool growing) const;

    SDL_Renderer* m_renderer = nullptr;
    SDL_Texture*  m_diamondTexture = nullptr;
    int m_level = 0;

    uint32_t m_lastTime = 0;
    uint32_t m_growDuration = 400;
    uint32_t m_shrinkDuration = 400;
    float m_maxStagger = 250.0f;
    float m_cellSize = 110.0f;
    float m_maxSize = 0.0f;

    bool m_hasSwapped = false;
    std::vector<Diamond> m_diamonds;
    GameState* m_gameState= nullptr;
};