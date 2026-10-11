//
// Created by LENOVO on 10-08-2026.
//
#include "AdvanceMenuState.h"
#include "engine/Engine.h"

using namespace MenuKit;

namespace {
    constexpr float ROW_TOP = 250.0f;
    constexpr int   LABEL_FONT = 36, VALUE_FONT = 32;
}

AdvanceMenuState::AdvanceMenuState(SDL_Renderer *renderer)
        : m_renderer(renderer), m_painter(renderer), m_font(LABEL_FONT) {
    this->Name = "AdvanceMenuState";

    m_debugLabel.set(renderer,m_font,"DEBUG",LABEL_FONT);
    m_onLabel.set(renderer,m_font,"ON",VALUE_FONT);
    m_offLabel.set(renderer,m_font,"OFF",VALUE_FONT);

    m_rows.setup(1,ROW_TOP,95.0f);
    m_on = GameData::getInstance().isDebugEnabled() ? 1.0f : 0.0f;
}

AdvanceMenuState::~AdvanceMenuState() {
}

void AdvanceMenuState::setDebug(bool enabled) {
    if(GameData::getInstance().isDebugEnabled() == enabled) return;
    GameData::getInstance().toggleDebug();
    LOGI("toggle debug:%d",GameData::getInstance().isDebugEnabled());
    Engine::Get().getAssetManager().playSound(SoundType::SFX_UI_CONFIRM);
}

void AdvanceMenuState::update(float dt) {
    m_rows.update(dt);
    m_on = approach(m_on,GameData::getInstance().isDebugEnabled() ? 1.0f : 0.0f,14.0f,dt);
    perform(m_nav.update(dt));                               // held dpad / stick
}

void AdvanceMenuState::perform(InputUtils::MenuAction action) {
    switch(action){
        case InputUtils::MenuAction::CONFIRM: setDebug(!GameData::getInstance().isDebugEnabled()); break;
        case InputUtils::MenuAction::LEFT:    setDebug(false); break;
        case InputUtils::MenuAction::RIGHT:   setDebug(true);  break;
        default: break;
    }
}

void AdvanceMenuState::render(SDL_Renderer *renderer) {
    const MenuTheme& theme = MenuTheme::get();
    const MenuPainter& p = m_painter;

    const float y  = m_rows.rowY(0);
    const float f  = m_rows.focusAnim(0);
    const float rv = m_rows.reveal(0);
    const float a  = lerp(0.70f,1.0f,f) * rv;
    const Rgb col  = mix(theme.dim(),theme.accent(),f);

    p.rowFrame(Layout::ROW_LEFT,Layout::ROW_RIGHT,y,f,rv);
    p.label(m_debugLabel,Layout::LABEL_X,y,1.0f + 0.06f * f,col,a,Align::LEFT);

    // ON / OFF: the word crossfades, the lit one is bright and glows
    p.glow(Layout::CONTROL_RIGHT - 25.0f,y,120.0f,80.0f,theme.core(),0.35f * m_on * rv);
    p.label(m_offLabel,Layout::CONTROL_RIGHT,y,1.0f,theme.dim(),0.9f * (1.0f - m_on) * rv,Align::RIGHT);
    p.label(m_onLabel,Layout::CONTROL_RIGHT,y,1.0f,theme.accent(),m_on * rv,Align::RIGHT);
}

bool AdvanceMenuState::handleEvents(SDL_Event &event) {
    if(m_nav.handleEvent(event)) return true;

    if(InputUtils::IsPointerDown(event)){
        float x,y;
        InputUtils::GetPointerPosition(event,m_renderer,x,y);
        if(m_rows.hit(x,y,Layout::ROW_LEFT - 40.0f,Layout::ROW_RIGHT + 40.0f,Layout::ROW_HALF_H) < 0)
            return false;                                 // tabs / back belong to OptionMenuState
        setDebug(!GameData::getInstance().isDebugEnabled());
        return true;
    }

    const InputUtils::MenuAction action = InputUtils::GetMenuAction(event);
    switch(action){
        case InputUtils::MenuAction::CONFIRM:
        case InputUtils::MenuAction::LEFT:
        case InputUtils::MenuAction::RIGHT:
            perform(action);
            return true;
        default:
            return false;
    }
}
