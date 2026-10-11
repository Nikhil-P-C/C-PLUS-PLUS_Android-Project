//
// Created by LENOVO on 11-10-2026.
//

#include <SDL3/SDL.h>
#include <cmath>
#include <algorithm>
#include "MenuState.h"
#include "engine/Engine.h"
#include "States/MenuState/OptionMenuState/OptionMenuState.h"
#include "States/MenuState/EditMenuState/EditMenuState.h"
#include "States/TransitionState/TransitionState.h"

using namespace MenuKit;

// ---------------------------------------------------------------------------------------------
// Tunables
// ---------------------------------------------------------------------------------------------
namespace {
    // Swap for a serif / caps display face (Cinzel, Trajan-like) to get closer to Silksong.
    constexpr const char* TITLE_TEXT = "";           // "" = no title, only the divider ornament
    constexpr int   ITEM_FONT_SIZE  = 38;
    constexpr int   TITLE_FONT_SIZE = 78;

    constexpr float CORE_X = 800.0f, CORE_Y = 290.0f; // where the light sits
    constexpr float LIST_TOP = 360.0f, LIST_STEP = 68.0f;
    constexpr float ITEM_HIT_W = 460.0f, ITEM_HIT_H = 64.0f;
}

// ---------------------------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------------------------
MenuState::MenuState(SDL_Renderer *renderer, bool intro)
        : m_renderer(renderer),
          m_backdrop(renderer, CORE_X, CORE_Y, intro ? 1.2f : 0.4f),
          m_font(ITEM_FONT_SIZE),
          m_intro(intro) {
    this->Name = "NewMenuState";
    LOGI("menu construct:%p",this);

    static const char* labels[4] = {"PLAY","OPTIONS","EDIT","QUIT"};
    for(int i=0;i<4;i++){
        m_items[i].cx = SCREEN_W * 0.5f;
        m_items[i].cy = LIST_TOP + LIST_STEP * (float)i;
        m_items[i].label.set(renderer,m_font,labels[i],ITEM_FONT_SIZE);
    }
    m_items[0].anim = 1.0f;

    if(TITLE_TEXT[0]) m_title.set(renderer,m_font,TITLE_TEXT,TITLE_FONT_SIZE);

    if(m_intro) Engine::Get().getAssetManager().playSound(SoundType::SFX_UI_INTRO);
}

MenuState::~MenuState() {
    LOGI("menu destructor:%p",this);
}

// ---------------------------------------------------------------------------------------------
// Update
// ---------------------------------------------------------------------------------------------
float MenuState::revealOf(int index) const {
    // intro: entries fade in one after another; coming back from a sub menu: nearly instant
    const float delay = m_intro ? 0.45f : 0.05f;
    const float step  = m_intro ? 0.12f : 0.05f;
    const float span  = m_intro ? 0.55f : 0.30f;
    return smooth((m_backdrop.time() - delay - step * (float)index) / span);
}

void MenuState::update(float dt) {
    m_backdrop.update(dt);

    for(int i=0;i<4;i++)
        m_items[i].anim = approach(m_items[i].anim,(i == m_selected) ? 1.0f : 0.0f,12.0f,dt);

    // held dpad / left stick with auto-repeat
    const InputUtils::MenuAction held = m_nav.update(dt);
    if(!m_transitioning){
        if(held == InputUtils::MenuAction::UP)   moveSelection(-1);
        if(held == InputUtils::MenuAction::DOWN) moveSelection(1);
    }

    // quit: let the sound + fade play, then close
    if(m_quitting){
        m_quitTimer += dt;
        if(m_quitTimer >= 1.2f){
            m_quitting = false;
            Engine::Get().exitEngine();
        }
    }
}

// ---------------------------------------------------------------------------------------------
// Render
// ---------------------------------------------------------------------------------------------
void MenuState::render(SDL_Renderer* renderer) {
    const MenuTheme& theme = MenuTheme::get();
    const MenuPainter& p = m_backdrop.painter();

    // 1) black base + core + motes + sparks
    m_backdrop.render(renderer);

    // 2) calm the area behind the list so text stays readable over the light
    p.shade(CORE_X,LIST_TOP + LIST_STEP * 1.5f,1040.0f,520.0f,150.0f / 255.0f);

    // 3) title + divider ornament
    const float intro = m_intro ? smooth(m_backdrop.time() / 1.0f) : 1.0f;
    if(m_title.tex) p.label(m_title,CORE_X,175.0f,1.0f,theme.accent(),intro);
    {
        const float y = 262.0f;
        const float a = 0.8f * intro;
        p.fadeLine(CORE_X - 260.0f,CORE_X - 18.0f,y,theme.accent(),0.0f,a);
        p.fadeLine(CORE_X + 18.0f,CORE_X + 260.0f,y,theme.accent(),a,0.0f);
        p.diamond(CORE_X,y,7.0f + 2.0f * theme.pulse(),theme.accent(),a);
        p.diamond(CORE_X - 30.0f,y,2.5f,theme.accent(),a * 0.8f);
        p.diamond(CORE_X + 30.0f,y,2.5f,theme.accent(),a * 0.8f);
    }

    // 4) entries: plain text, selected one grows, brightens and gets a diamond + line ornament
    for(int i=0;i<4;i++){
        const MenuItem& it = m_items[i];
        const float s  = it.anim;
        const float rv = revealOf(i);
        const float y  = it.cy + (1.0f - rv) * 14.0f;
        const Rgb col  = mix(theme.dim(),theme.accent(),s);
        const float alpha = lerp(0.70f,1.0f,s) * rv;
        const float scale = 1.0f + 0.10f * s + 0.02f * s * theme.pulse();

        p.flank(it.label,it.cx,y,scale,s,rv);
        p.label(it.label,it.cx,y,scale,col,alpha);
    }

    // 5) soft foreground motes, then the fades (intro from black, outro on quit)
    m_backdrop.renderFront(renderer);
    m_backdrop.renderFade(renderer);
}

// ---------------------------------------------------------------------------------------------
// Input
// ---------------------------------------------------------------------------------------------
int MenuState::hitTest(float x, float y) const {
    for(int i=0;i<4;i++){
        const MenuItem& it = m_items[i];
        if(InputUtils::PointInRect(x,y,it.cx - ITEM_HIT_W*0.5f,it.cy - ITEM_HIT_H*0.5f,ITEM_HIT_W,ITEM_HIT_H)) return i;
    }
    return -1;
}

void MenuState::setSelected(int index, bool playSound) {
    if(index < 0 || index > 3 || index == m_selected) return;
    m_selected = index;
    m_backdrop.spark(m_items[index].cx,m_items[index].cy,10);
    if(playSound) Engine::Get().getAssetManager().playSound(SoundType::SFX_UI_MOVE);
}

void MenuState::moveSelection(int dir) {
    setSelected((m_selected + dir + 4) % 4,true);
}

bool MenuState::handleEvents(SDL_Event &event) {
    if(m_transitioning) return true;

    // dpad / left stick: stored by the repeater, acted on in update()
    if(m_nav.handleEvent(event)) return true;

    // ---- mouse hover / touch drag ----
    if(InputUtils::IsPointerMotion(event)){
        float x,y;
        InputUtils::GetPointerPosition(event,m_renderer,x,y);
        const int idx = hitTest(x,y);
        if(idx >= 0) setSelected(idx,true);
        return true;
    }

    // ---- mouse click / touch ----
    if(InputUtils::IsPointerDown(event)){
        float x,y;
        InputUtils::GetPointerPosition(event,m_renderer,x,y);
        const int idx = hitTest(x,y);
        if(idx >= 0){
            setSelected(idx,false);
            activateSelection();
        }
        return true;
    }

    // ---- keyboard / gamepad buttons ----
    if(InputUtils::IsGamepadStart(event)){      // Start = go, here (it is "back" in the sub menus)
        activateSelection();
        return true;
    }
    switch(InputUtils::GetMenuAction(event)){
        case InputUtils::MenuAction::UP:      moveSelection(-1);   return true;   // key repeat = hold to scroll
        case InputUtils::MenuAction::DOWN:    moveSelection(1);    return true;
        case InputUtils::MenuAction::CONFIRM: activateSelection(); return true;
        default:                              return false;
    }
}

void MenuState::activateSelection() {
    if(m_transitioning) return;
    AssetManager& audio = Engine::Get().getAssetManager();
    m_backdrop.burst(m_items[m_selected].cx,m_items[m_selected].cy);

    switch (static_cast<NewMenuButton>(m_selected)) {
        case NewMenuButton::PLAY: {
            audio.playSound(SoundType::SFX_UI_PLAY);
            m_transitioning = true;
            Engine::Get().pushOverlayState(std::make_unique<TransitionState>(m_renderer,0));
            break;
        }
        case NewMenuButton::OPTION:
            audio.playSound(SoundType::SFX_UI_OPEN);
            m_transitioning = true;
            Engine::Get().changeState(std::make_unique<OptionMenuState>(m_renderer));
            break;
        case NewMenuButton::EDIT:
            audio.playSound(SoundType::SFX_UI_OPEN);
            m_transitioning = true;
            Engine::Get().changeState(std::make_unique<EditMenuState>(m_renderer));
            break;
        case NewMenuButton::QUIT:
            audio.playSound(SoundType::SFX_UI_QUIT);
            m_transitioning = true;
            m_quitting = true;       // update() calls exitEngine() after the fade
            m_quitTimer = 0.0f;
            m_backdrop.beginFadeOut(1.1f);
            break;
    }
}
