//
// Created by LENOVO on 04-07-2026.
//
#include "AudioMenuState.h"
#include "SDL3/SDL.h"
#include <cmath>
#include <cstdio>
#include "engine/Engine.h"

using namespace MenuKit;

namespace {
    constexpr float TRACK_L = 760.0f, TRACK_R = 1050.0f;   // slider track
    constexpr float ROW_TOP = 250.0f, ROW_STEP = 95.0f;
    constexpr int   LABEL_FONT = 36, VALUE_FONT = 30;
    constexpr float KEY_STEP = 0.05f;                       // 5% per key press / repeat
}

AudioMenuState::AudioMenuState(SDL_Renderer *renderer)
        : m_renderer(renderer), m_painter(renderer), m_font(LABEL_FONT) {
    this->Name = "AudioMenuState";
    LOGI("Audio menu state constructor:%p",this);

    static const char* names[ROW_COUNT] = {"MASTER","MUSIC","SFX"};
    for(int i=0;i<ROW_COUNT;i++){
        m_names[i].set(renderer,m_font,names[i],LABEL_FONT);
        refreshValueLabel(i);
    }
    m_rows.setup(ROW_COUNT,ROW_TOP,ROW_STEP);
}

AudioMenuState::~AudioMenuState(){
    LOGI("Audio menu state destructor:%p",this);
}

float AudioMenuState::value(int row) const {
    switch(row){
        case MASTER_ROW: return GameData::getInstance().getAudioScale();
        case MUSIC_ROW:  return GameData::getInstance().getMusicAudioScale();
        default:         return GameData::getInstance().getSFXAudioScale();
    }
}

void AudioMenuState::setValue(int row,float v) {
    v = clamp01(v);
    switch(row){
        case MASTER_ROW: GameData::getInstance().setAudioScale(v);      break;
        case MUSIC_ROW:  GameData::getInstance().setMusicAudioScale(v); break;
        default:         GameData::getInstance().setSFXAudioScale(v);   break;
    }
}

void AudioMenuState::refreshValueLabel(int row) {
    const int pct = (int)std::lround(value(row) * 100.0f);
    if(pct == m_shownPercent[row]) return;
    m_shownPercent[row] = pct;
    char buf[8];
    std::snprintf(buf,sizeof(buf),"%d",pct);
    m_values[row].set(m_renderer,m_font,buf,VALUE_FONT);
}

void AudioMenuState::step(int row,float delta) {
    const float before = value(row);
    setValue(row,before + delta);
    if(value(row) != before)
        Engine::Get().getAssetManager().playSound(SoundType::SFX_UI_MOVE);   // audible tick at the new level
}

void AudioMenuState::dragTo(float pointerX) {
    if(m_dragRow < 0) return;
    setValue(m_dragRow,(pointerX - TRACK_L) / (TRACK_R - TRACK_L));
}

void AudioMenuState::update(float dt) {
    m_rows.update(dt);
    for(int i=0;i<ROW_COUNT;i++) refreshValueLabel(i);     // the value can also change from outside

    perform(m_nav.update(dt));                               // held dpad / stick
}

void AudioMenuState::perform(InputUtils::MenuAction action) {
    switch(action){
        case InputUtils::MenuAction::UP:
            if(m_rows.move(-1)) Engine::Get().getAssetManager().playSound(SoundType::SFX_UI_MOVE);
            break;
        case InputUtils::MenuAction::DOWN:
            if(m_rows.move(1))  Engine::Get().getAssetManager().playSound(SoundType::SFX_UI_MOVE);
            break;
        case InputUtils::MenuAction::LEFT:  step(m_rows.focus(),-KEY_STEP); break;
        case InputUtils::MenuAction::RIGHT: step(m_rows.focus(), KEY_STEP); break;
        default: break;
    }
}

void AudioMenuState::render(SDL_Renderer *renderer) {
    const MenuTheme& theme = MenuTheme::get();
    const MenuPainter& p = m_painter;

    for(int i=0;i<ROW_COUNT;i++){
        const float y  = m_rows.rowY(i);
        const float f  = m_rows.focusAnim(i);
        const float rv = m_rows.reveal(i);
        const float a  = lerp(0.70f,1.0f,f) * rv;
        const Rgb col  = mix(theme.dim(),theme.accent(),f);

        p.rowFrame(Layout::ROW_LEFT,Layout::ROW_RIGHT,y,f,rv);
        p.label(m_names[i],Layout::LABEL_X,y,1.0f + 0.06f * f,col,a,Align::LEFT);

        // track, filled part, handle
        const float v  = value(i);
        const float hx = lerp(TRACK_L,TRACK_R,v);
        p.fadeLine(TRACK_L,TRACK_R,y,theme.dim(),0.55f * rv,0.55f * rv);
        if(hx > TRACK_L + 1.0f)
            p.fadeLine(TRACK_L,hx,y,theme.accent(),0.35f * rv,(0.55f + 0.4f * f) * rv,1.5f + 0.5f * f);
        p.glow(hx,y,60.0f + 40.0f * f,60.0f + 40.0f * f,theme.core(),(0.30f + 0.35f * f) * rv);
        p.diamond(hx,y,6.0f + 3.0f * f,theme.accent(),rv);

        p.label(m_values[i],Layout::CONTROL_RIGHT,y,1.0f,col,a,Align::RIGHT);
    }
}

bool AudioMenuState::handleEvents(SDL_Event &event) {
    if(m_nav.handleEvent(event)) return true;

    // ---- pointer: press on a row = focus it, press on its track = start dragging ----
    if(InputUtils::IsPointerDown(event)){
        float x,y;
        InputUtils::GetPointerPosition(event,m_renderer,x,y);
        const int row = m_rows.hit(x,y,Layout::ROW_LEFT - 40.0f,Layout::ROW_RIGHT + 40.0f,Layout::ROW_HALF_H);
        if(row < 0) return false;                         // tabs / back belong to OptionMenuState
        m_rows.setFocus(row);
        if(x >= TRACK_L - 24.0f && x <= TRACK_R + 24.0f){
            m_dragging = true;
            m_dragRow = row;
            m_dragIsTouch = event.type == SDL_EVENT_FINGER_DOWN;
            m_dragFinger = m_dragIsTouch ? event.tfinger.fingerID : 0;
            dragTo(x);
        }
        return true;
    }

    if(InputUtils::IsPointerUp(event) && m_dragging){
        // only the finger / mouse that started the drag ends it
        if(event.type == SDL_EVENT_FINGER_UP && (!m_dragIsTouch || event.tfinger.fingerID != m_dragFinger)) return false;
        if(event.type == SDL_EVENT_MOUSE_BUTTON_UP && m_dragIsTouch) return false;
        m_dragging = false;
        m_dragRow = -1;
        Engine::Get().getAssetManager().playSound(SoundType::SFX_UI_MOVE);   // lets you hear the new level
        return true;
    }

    if(InputUtils::IsPointerMotion(event)){
        const bool touchMotion = event.type == SDL_EVENT_FINGER_MOTION;
        float x,y;
        InputUtils::GetPointerPosition(event,m_renderer,x,y);
        if(m_dragging){
            // ignore other fingers, and ignore the mouse while a finger drags (and vice versa)
            if(touchMotion != m_dragIsTouch) return false;
            if(touchMotion && event.tfinger.fingerID != m_dragFinger) return false;
            dragTo(x);
            return true;
        }
        const int row = m_rows.hit(x,y,Layout::ROW_LEFT - 40.0f,Layout::ROW_RIGHT + 40.0f,Layout::ROW_HALF_H);
        if(row >= 0 && m_rows.setFocus(row)) Engine::Get().getAssetManager().playSound(SoundType::SFX_UI_MOVE);
        return false;                                     // tabs / back hover still needs it
    }

    // ---- keyboard / gamepad buttons ----
    const InputUtils::MenuAction action = InputUtils::GetMenuAction(event);
    switch(action){
        case InputUtils::MenuAction::UP:
        case InputUtils::MenuAction::DOWN:
        case InputUtils::MenuAction::LEFT:
        case InputUtils::MenuAction::RIGHT:
            perform(action);
            return true;
        default:
            return false;                                 // back / tab switching: OptionMenuState
    }
}
