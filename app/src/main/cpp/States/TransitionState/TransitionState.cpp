// TransitionState.cpp
#include "TransitionState.h"
#include "engine/Engine.h"
#include "States/GameState/GameState.h"
#include "States/HUDOverlayState/HUDOverlayState.h"
#include "States/InputOverlayState/JoystickOverlay.h"
#include "States/InputOverlayState/ButtonOverlay.h"
#include "States/InputOverlayState/SepJoysticknButton.h"
#include "States/InputOverlayState/KeyboardOverlay.h"
#include "States/InputOverlayState/GamepadOverlay.h"
#include "utils/GameData.h"
#include "States/DebugState/DebugState.h"
#include <cmath>


void TransitionState::buildGrid() {
    int winW = GameData::getInstance().getWinWidth();
    int winH = GameData::getInstance().getWinHeight();

    int cols = static_cast<int>(std::ceil((float)winW / m_cellSize)) + 1;
    int rows = static_cast<int>(std::ceil((float)winH / m_cellSize)) + 1;

    m_maxSize = m_cellSize * 1.5f; // overshoot so fully-grown diamonds overlap and leave no gaps

    float centerCol = (cols - 1) / 2.0f;
    float centerRow = (rows - 1) / 2.0f;
    float maxDist = std::sqrt(centerCol * centerCol + centerRow * centerRow);
    if (maxDist <= 0.0f) maxDist = 1.0f;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            Diamond diamond;
            diamond.cx = (float)c * m_cellSize;
            diamond.cy = (float)r * m_cellSize;
            float dist = std::sqrt((c - centerCol) * (c - centerCol) +
                                   (r - centerRow) * (r - centerRow));
            diamond.delayOffset = (dist / maxDist) * m_maxStagger; // ripples outward from center
            diamond.currentSize = 0.0f;
            m_diamonds.push_back(diamond);
        }
    }
}

float TransitionState::computeSize(float localT, float phaseDuration, bool growing) const {
    if (phaseDuration <= 0.0f) return growing ? m_maxSize : 0.0f;
    float progress = localT / phaseDuration;
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;
    return growing ? m_maxSize * progress : m_maxSize * (1.0f - progress);
}

void TransitionState::update(float dt) {
    uint32_t now = SDL_GetTicks();
    uint32_t elapsed = now - m_lastTime;

    for (auto& diamond : m_diamonds) {
        float local = (float)elapsed - diamond.delayOffset;
        if (local < 0.0f) local = 0.0f;

        if (local < (float)m_growDuration) {
            diamond.currentSize = computeSize(local, (float)m_growDuration, true);
        } else if (!m_hasSwapped) {
            diamond.currentSize = m_maxSize;
        } else {
            float shrinkT = local - (float)m_growDuration;
            diamond.currentSize = computeSize(shrinkT, (float)m_shrinkDuration, false);
        }
    }

    if (!m_hasSwapped && elapsed >= m_growDuration + (uint32_t)m_maxStagger) {
        m_hasSwapped = true;

        //risky code
        Engine::Get().popState();//old gamestate
        auto newGameState = std::make_unique<GameState>(m_renderer, m_level);
        m_gameState = newGameState.get();//capture the raw pointer before ownership moves into the command queue;
        //getCurrentState() would be stale here since PUSH/POP are queued, not applied until next frame
        Engine::Get().pushState(std::move(newGameState));


    }

    if (m_hasSwapped && elapsed >= m_growDuration + m_shrinkDuration + (uint32_t)m_maxStagger) {
        Engine::Get().popOverlayState();//pop this state


        LOGI("overlayState:%d",Engine::Get().getOverlayStateCount());
        Engine::Get().pushOverlayState(std::make_unique<HUDOverlayState>(m_renderer));
        if(GameData::getInstance().isDebugEnabled()){
            if(m_gameState)
                Engine::Get().pushOverlayState(std::make_unique<DebugState>(m_renderer,m_gameState));
        }
        if (GameData::getInstance().getControlType() == JOYSTICK)
            Engine::Get().pushOverlayState(std::make_unique<JoystickOverlay>(m_renderer));
        if (GameData::getInstance().getControlType() == BUTTONS)
            Engine::Get().pushOverlayState(std::make_unique<ButtonOverlay>(m_renderer));
        if (GameData::getInstance().getControlType() == SEP_JUMP_W_JOYSTICK)
            Engine::Get().pushOverlayState(std::make_unique<SepJoysticknButton>(m_renderer));
        if (GameData::getInstance().getControlType() == KEYBOARD)
            Engine::Get().pushOverlayState(std::make_unique<KeyboardOverlay>(m_renderer));
        if (GameData::getInstance().getControlType() == GAMEPAD)
            Engine::Get().pushOverlayState(std::make_unique<GamepadOverlay>(m_renderer));

    }
}

void TransitionState::render(SDL_Renderer *renderer) {
    if (!m_diamondTexture) return;

    for (const auto& diamond : m_diamonds) {
        if (diamond.currentSize <= 0.5f) continue;

        SDL_FRect dst{
                (diamond.cx - diamond.currentSize * 0.5f),
                (diamond.cy - diamond.currentSize * 0.5f),
                diamond.currentSize+100,
                diamond.currentSize+100
        };
        SDL_RenderTexture(renderer, m_diamondTexture, nullptr, &dst);
    }
}

bool TransitionState::handleEvents(SDL_Event &event) {
    return true; // swallow input while transitioning
}

TransitionState::TransitionState(SDL_Renderer *renderer, int level) {
    this->Name = "Transition";
    m_renderer = renderer;
    m_level = level;
    if(level > 1)
        m_level =0;
    m_lastTime = SDL_GetTicks();

    m_diamondTexture = Engine::Get().getAssetManager().getTexture(TextureType::TRANSITION);


    buildGrid();
}

TransitionState::~TransitionState() {}