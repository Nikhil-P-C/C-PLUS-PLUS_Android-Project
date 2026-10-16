#include <SDL3/SDL.h>

#include "RespawnState.h"
#include "engine/Engine.h"

#include "States/GameState/GameState.h"
#include "States/MenuState/MenuState.h"

#include "States/InputOverlayState/ButtonOverlay.h"
#include "States/InputOverlayState/SepJoysticknButton.h"
#include "States/InputOverlayState/KeyboardOverlay.h"
#include "States/InputOverlayState/GamepadOverlay.h"

#include "States/DebugState/DebugState.h"
#include "States/HUDOverlayState/HUDOverlayState.h"

using namespace MenuKit;

namespace {
    constexpr float CORE_X = 800.0f, CORE_Y = 300.0f;
    constexpr float TITLE_Y = 230.0f, DIVIDER_Y = 300.0f, CAPTION_Y = 350.0f;
    constexpr float LIST_TOP = 430.0f, LIST_STEP = 76.0f;
    constexpr float HIT_HALF_W = 230.0f, HIT_HALF_H = 36.0f;
    constexpr int   TITLE_FONT = 72, ENTRY_FONT = 38, CAPTION_FONT = 28;
    constexpr float INPUT_LOCKOUT_S = 0.45f;   // ignore presses right after the screen appears
}

RespawnState::RespawnState(SDL_Renderer *renderer, GameState* gameState, bool atCheckpoint)
        : m_renderer(renderer),
          m_gameState(gameState),
          m_backdrop(renderer,CORE_X,CORE_Y,0.3f,true),
          m_font(ENTRY_FONT) {
    this->Name = "RespawnState";
    LOGI("Respawn construct:%p",this);

    m_title.set(renderer,m_font,"YOU DIED",TITLE_FONT);
    m_caption.set(renderer,m_font,atCheckpoint ? "BACK AT YOUR CHECKPOINT" : "BACK TO THE START",CAPTION_FONT);
    m_entry[ENTRY_RESPAWN].set(renderer,m_font,"RESPAWN",ENTRY_FONT);
    m_entry[ENTRY_EXIT].set(renderer,m_font,"EXIT",ENTRY_FONT);
    m_rows.setup(ENTRY_COUNT,LIST_TOP,LIST_STEP,ENTRY_RESPAWN,0.15f,0.10f);
}

RespawnState::~RespawnState() {
    LOGI("Respawn destructor:%p",this);
}

void RespawnState::update(float dt) {
    m_backdrop.update(dt);
    m_rows.update(dt);

    if(m_transitioning) return;
    switch(m_nav.update(dt)){                    // held dpad / stick
        case InputUtils::MenuAction::UP:
            if(m_rows.move(-1)) Engine::Get().getAssetManager().playSound(SoundType::SFX_UI_MOVE);
            break;
        case InputUtils::MenuAction::DOWN:
            if(m_rows.move(1))  Engine::Get().getAssetManager().playSound(SoundType::SFX_UI_MOVE);
            break;
        default: break;
    }
}

void RespawnState::render(SDL_Renderer *renderer) {
    const MenuTheme& theme = MenuTheme::get();
    const MenuPainter& p = m_backdrop.painter();
    const float t = smooth(m_backdrop.time() / 0.3f);

    m_backdrop.render(renderer);

    p.shade(CORE_X,LIST_TOP - 60.0f,1000.0f,600.0f,140.0f / 255.0f * t);

    p.label(m_title,CORE_X,TITLE_Y,1.0f,theme.accent(),t);
    {
        const float a = 0.8f * t;
        p.fadeLine(CORE_X - 260.0f,CORE_X - 18.0f,DIVIDER_Y,theme.accent(),0.0f,a);
        p.fadeLine(CORE_X + 18.0f,CORE_X + 260.0f,DIVIDER_Y,theme.accent(),a,0.0f);
        p.diamond(CORE_X,DIVIDER_Y,7.0f + 2.0f * theme.pulse(),theme.accent(),a);
        p.diamond(CORE_X - 30.0f,DIVIDER_Y,2.5f,theme.accent(),a * 0.8f);
        p.diamond(CORE_X + 30.0f,DIVIDER_Y,2.5f,theme.accent(),a * 0.8f);
    }
    p.label(m_caption,CORE_X,CAPTION_Y,1.0f,theme.dim(),0.85f * t);

    for(int i=0;i<ENTRY_COUNT;i++){
        const float s  = m_rows.focusAnim(i);
        const float rv = m_rows.reveal(i);
        const float y  = m_rows.rowY(i) + (1.0f - rv) * 14.0f;
        const float scale = 1.0f + 0.10f * s + 0.02f * s * theme.pulse();
        p.flank(m_entry[i],CORE_X,y,scale,s,rv);
        p.label(m_entry[i],CORE_X,y,scale,mix(theme.dim(),theme.accent(),s),lerp(0.70f,1.0f,s) * rv);
    }

    m_backdrop.renderFront(renderer);
    m_backdrop.renderFade(renderer);
}

// put the gameplay overlays back (same set the pause screen restores)
void RespawnState::respawn() {
    GameData::getInstance().setPaused(false);
    m_transitioning = true;
    Engine::Get().popState();                 // remove this screen
    m_gameState->completeRespawn();           // player back on its feet at the checkpoint / level start
    Engine::Get().pushOverlayState(std::make_unique<HUDOverlayState>(m_renderer));
    if(GameData::getInstance().isDebugEnabled())
        Engine::Get().pushOverlayState(std::make_unique<DebugState>(m_renderer ,m_gameState));
    if(GameData::getInstance().getControlType() ==ControlType::BUTTONS)
        Engine::Get().pushOverlayState(std::make_unique<ButtonOverlay>(m_renderer));
    if(GameData::getInstance().getControlType() ==ControlType::SEP_JUMP_W_JOYSTICK)
        Engine::Get().pushOverlayState(std::make_unique<SepJoysticknButton>(m_renderer));
    if(GameData::getInstance().getControlType() ==ControlType::KEYBOARD)
        Engine::Get().pushOverlayState(std::make_unique<KeyboardOverlay>(m_renderer));
    if(GameData::getInstance().getControlType() ==ControlType::GAMEPAD)
        Engine::Get().pushOverlayState(std::make_unique<GamepadOverlay>(m_renderer));
}

// identical to PauseState::exitToMenu
void RespawnState::exitToMenu() {
    PlayerDetail::getInstance().setScore(0);
    PlayerDetail::getInstance().setPlayerHP(5);
    GameData::getInstance().setPaused(false);
    m_transitioning = true;
    Engine::Get().popState();
    if(GameData::getInstance().isDebugEnabled())
        Engine::Get().popOverlayState();
    Engine::Get().changeState(std::make_unique<MenuState>(m_renderer, false));
}

void RespawnState::activate(int entry) {
    if(m_transitioning) return;
    m_backdrop.burst(CORE_X,m_rows.rowY(entry));
    if(entry == ENTRY_EXIT){
        Engine::Get().getAssetManager().playSound(SoundType::SFX_UI_ENTER);
        exitToMenu();
    }else{
        Engine::Get().getAssetManager().playSound(SoundType::SFX_UI_CONFIRM);
        respawn();
    }
}

bool RespawnState::handleEvents(SDL_Event &event) {
    if(m_transitioning) return true;
    if(m_backdrop.time() < INPUT_LOCKOUT_S) return true;   // swallow everything for a moment

    if(m_nav.handleEvent(event)) return true;

    if(InputUtils::IsPointerMotion(event)){
        float x,y;
        InputUtils::GetPointerPosition(event,m_renderer,x,y);
        const int row = m_rows.hit(x,y,CORE_X - HIT_HALF_W,CORE_X + HIT_HALF_W,HIT_HALF_H);
        if(row >= 0 && m_rows.setFocus(row)) Engine::Get().getAssetManager().playSound(SoundType::SFX_UI_MOVE);
        return true;
    }

    // tap EXIT to leave, tap anywhere else to respawn
    if(InputUtils::IsPointerDown(event)){
        float x,y;
        InputUtils::GetPointerPosition(event,m_renderer,x,y);
        const int row = m_rows.hit(x,y,CORE_X - HIT_HALF_W,CORE_X + HIT_HALF_W,HIT_HALF_H);
        activate(row == ENTRY_EXIT ? ENTRY_EXIT : ENTRY_RESPAWN);
        return true;
    }

    switch(InputUtils::GetMenuAction(event)){
        case InputUtils::MenuAction::UP:
            if(m_rows.move(-1)) Engine::Get().getAssetManager().playSound(SoundType::SFX_UI_MOVE);
            return true;
        case InputUtils::MenuAction::DOWN:
            if(m_rows.move(1))  Engine::Get().getAssetManager().playSound(SoundType::SFX_UI_MOVE);
            return true;
        case InputUtils::MenuAction::CONFIRM:
            activate(m_rows.focus());
            return true;
        case InputUtils::MenuAction::BACK:       // Esc / Android back: swallowed so it can't open the pause menu underneath
            return true;
        default:
            return false;
    }
}
