//
// Created by LENOVO on 02-07-2026.
//
#include "ControlMenuState.h"
#include "SDL3/SDL.h"
#include "engine/Engine.h"

using namespace MenuKit;

namespace {
    constexpr float ROW_TOP = 230.0f, ROW_STEP = 85.0f;
    constexpr int   LABEL_FONT = 36;

    struct Option { const char* name; ControlType type; };
    // same order as the old screen
    const Option OPTIONS[5] = {
            {"JOYSTICK",          ControlType::JOYSTICK},
            {"BUTTONS",           ControlType::BUTTONS},
            {"JOYSTICK + BUTTON", ControlType::SEP_JUMP_W_JOYSTICK},
            {"KEYBOARD",          ControlType::KEYBOARD},
            {"GAMEPAD",           ControlType::GAMEPAD},
    };
}

ControlMenuState::ControlMenuState(SDL_Renderer *renderer)
        : m_renderer(renderer), m_painter(renderer), m_font(LABEL_FONT) {
    this->Name = "ControlMenu";

    for(int i=0;i<OPTION_COUNT;i++)
        m_labels[i].set(renderer,m_font,OPTIONS[i].name,LABEL_FONT);

    const int current = currentIndex();
    m_rows.setup(OPTION_COUNT,ROW_TOP,ROW_STEP,current);   // start on the active scheme
    for(int i=0;i<OPTION_COUNT;i++) m_pick[i] = (i == current) ? 1.0f : 0.0f;
}

ControlMenuState::~ControlMenuState() {
    LOGI("Control menu state destructor:%p",this);
}

int ControlMenuState::currentIndex() const {
    const ControlType type = GameData::getInstance().getControlType();
    for(int i=0;i<OPTION_COUNT;i++)
        if(OPTIONS[i].type == type) return i;
    return 0;
}

void ControlMenuState::choose(int index) {
    if(index < 0 || index >= OPTION_COUNT) return;
    m_rows.setFocus(index);
    if(OPTIONS[index].type == GameData::getInstance().getControlType()) return;
    GameData::getInstance().setControlType(OPTIONS[index].type);
    m_pop[index] = 1.0f;
    Engine::Get().getAssetManager().playSound(SoundType::SFX_UI_CONFIRM);
}

void ControlMenuState::update(float dt) {
    m_rows.update(dt);
    const int current = currentIndex();
    for(int i=0;i<OPTION_COUNT;i++){
        m_pick[i] = approach(m_pick[i],(i == current) ? 1.0f : 0.0f,14.0f,dt);
        m_pop[i]  = approach(m_pop[i],0.0f,5.0f,dt);
    }
    perform(m_nav.update(dt));                               // held dpad / stick
}

void ControlMenuState::perform(InputUtils::MenuAction action) {
    switch(action){
        case InputUtils::MenuAction::UP:
            if(m_rows.move(-1)) Engine::Get().getAssetManager().playSound(SoundType::SFX_UI_MOVE);
            break;
        case InputUtils::MenuAction::DOWN:
            if(m_rows.move(1))  Engine::Get().getAssetManager().playSound(SoundType::SFX_UI_MOVE);
            break;
        case InputUtils::MenuAction::CONFIRM:
            choose(m_rows.focus());
            break;
        default: break;
    }
}

void ControlMenuState::render(SDL_Renderer *renderer) {
    const MenuTheme& theme = MenuTheme::get();
    const MenuPainter& p = m_painter;

    for(int i=0;i<OPTION_COUNT;i++){
        const float y  = m_rows.rowY(i);
        const float f  = m_rows.focusAnim(i);
        const float rv = m_rows.reveal(i);
        const float a  = lerp(0.70f,1.0f,f) * rv;
        const Rgb col  = mix(theme.dim(),theme.accent(),f);

        p.rowFrame(Layout::ROW_LEFT,Layout::ROW_RIGHT,y,f,rv);
        p.label(m_labels[i],Layout::LABEL_X,y,1.0f + 0.06f * f,col,a,Align::LEFT);

        // radio: hollow diamond = off, lit solid diamond = the active scheme
        const float rx = Layout::CONTROL_RIGHT - 14.0f;
        const float on = m_pick[i];
        p.diamondOutline(rx,y,11.0f,2.0f,mix(theme.dim(),theme.accent(),f),(0.85f * (1.0f - on) + 0.35f * on) * rv);
        if(on > 0.02f){
            const float pop = m_pop[i];
            p.glow(rx,y,70.0f + 90.0f * pop,70.0f + 90.0f * pop,theme.core(),(0.45f + 0.4f * pop) * on * rv);
            p.diamond(rx,y,(7.5f + 3.0f * pop) * on,theme.accent(),on * rv);
        }
    }
}

bool ControlMenuState::handleEvents(SDL_Event &event) {
    if(m_nav.handleEvent(event)) return true;

    if(InputUtils::IsPointerDown(event)){
        float x,y;
        InputUtils::GetPointerPosition(event,m_renderer,x,y);
        const int row = m_rows.hit(x,y,Layout::ROW_LEFT - 40.0f,Layout::ROW_RIGHT + 40.0f,Layout::ROW_HALF_H);
        if(row < 0) return false;                         // tabs / back belong to OptionMenuState
        choose(row);
        return true;
    }

    if(InputUtils::IsPointerMotion(event)){
        float x,y;
        InputUtils::GetPointerPosition(event,m_renderer,x,y);
        const int row = m_rows.hit(x,y,Layout::ROW_LEFT - 40.0f,Layout::ROW_RIGHT + 40.0f,Layout::ROW_HALF_H);
        if(row >= 0 && m_rows.setFocus(row)) Engine::Get().getAssetManager().playSound(SoundType::SFX_UI_MOVE);
        return false;
    }

    const InputUtils::MenuAction action = InputUtils::GetMenuAction(event);
    switch(action){
        case InputUtils::MenuAction::UP:
        case InputUtils::MenuAction::DOWN:
        case InputUtils::MenuAction::CONFIRM:
            perform(action);
            return true;
        default:
            return false;
    }
}
