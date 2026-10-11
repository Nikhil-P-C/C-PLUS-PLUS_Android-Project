//
// Created by LENOVO on 01-07-2026.
//

#include "OptionMenuState.h"
#include "SDL3/SDL.h"
#include "engine/Engine.h"
#include "States/MenuState/MenuState.h"
#include "States/MenuState/OptionMenuState/ControlMenuState.h"
#include "States/MenuState/OptionMenuState/AudioMenuState.h"
#include "States/MenuState/OptionMenuState/AdvanceMenuState.h"

using namespace MenuKit;

namespace {
    constexpr float CORE_X = 800.0f, CORE_Y = 300.0f;
    constexpr float TAB_Y = 95.0f, TAB_DX = 280.0f;            // tabs at x = 800 - DX, 800, 800 + DX
    constexpr float TAB_HIT_W = 250.0f, TAB_HIT_H = 70.0f;
    constexpr float BACK_X = 800.0f, BACK_Y = 660.0f;
    constexpr float BACK_HIT_W = 240.0f, BACK_HIT_H = 70.0f;
    constexpr int   TAB_FONT = 34, BACK_FONT = 34;

    inline float tabX(int i) { return CORE_X + TAB_DX * (float)(i - 1); }
}

OptionMenuState::OptionMenuState(SDL_Renderer *renderer)
        : m_renderer(renderer),
          m_backdrop(renderer,CORE_X,CORE_Y,0.4f),
          m_font(TAB_FONT) {
    this->Name = "OptionMenuState";
    LOGI("Option Menu State Constructor:%p",this);

    m_tabLabel[TAB_AUDIO].set(renderer,m_font,"AUDIO",TAB_FONT);
    m_tabLabel[TAB_CONTROL].set(renderer,m_font,"CONTROL",TAB_FONT);
    m_tabLabel[TAB_ADVANCE].set(renderer,m_font,"ADVANCE",TAB_FONT);
    m_backLabel.set(renderer,m_font,"BACK",BACK_FONT);

    // default page
    Engine::Get().pushOverlayState(makePage(m_tab));
}

OptionMenuState::~OptionMenuState() {
    LOGI("Option Menu State Destructor:%p",this);
}

std::unique_ptr<State> OptionMenuState::makePage(int tab) const {
    switch(tab){
        case TAB_CONTROL: return std::make_unique<ControlMenuState>(m_renderer);
        case TAB_ADVANCE: return std::make_unique<AdvanceMenuState>(m_renderer);
        default:          return std::make_unique<AudioMenuState>(m_renderer);
    }
}

void OptionMenuState::selectTab(int tab) {
    tab = (tab + TAB_COUNT) % TAB_COUNT;
    if(tab == m_tab) return;
    m_tab = tab;
    Engine::Get().getAssetManager().playSound(SoundType::SFX_UI_MOVE);
    m_backdrop.spark(tabX(tab),TAB_Y,8);
    Engine::Get().changeOverlayState(makePage(tab));
}

void OptionMenuState::leave() {
    if(m_leaving) return;
    m_leaving = true;
    Engine::Get().getAssetManager().playSound(SoundType::SFX_UI_ENTER);
    int count = (int)Engine::Get().getOverlayStateCount();
    while(count){
        count--;
        Engine::Get().popOverlayState();
    }
    Engine::Get().changeState(std::make_unique<MenuState>(m_renderer, false));
}

int OptionMenuState::tabAt(float x,float y) const {
    for(int i=0;i<TAB_COUNT;i++)
        if(InputUtils::PointInRect(x,y,tabX(i) - TAB_HIT_W*0.5f,TAB_Y - TAB_HIT_H*0.5f,TAB_HIT_W,TAB_HIT_H)) return i;
    return -1;
}

bool OptionMenuState::overBack(float x,float y) const {
    return InputUtils::PointInRect(x,y,BACK_X - BACK_HIT_W*0.5f,BACK_Y - BACK_HIT_H*0.5f,BACK_HIT_W,BACK_HIT_H);
}

void OptionMenuState::update(float dt) {
    m_backdrop.update(dt);
    for(int i=0;i<TAB_COUNT;i++){
        const float target = (i == m_tab) ? 1.0f : (i == m_hoverTab ? 0.45f : 0.0f);
        m_tabAnim[i] = approach(m_tabAnim[i],target,12.0f,dt);
    }
    m_backAnim = approach(m_backAnim,m_hoverBack ? 1.0f : 0.0f,12.0f,dt);
}

void OptionMenuState::render(SDL_Renderer *renderer) {
    const MenuTheme& theme = MenuTheme::get();
    const MenuPainter& p = m_backdrop.painter();
    const float intro = smooth(m_backdrop.time() / 0.4f);

    m_backdrop.render(renderer);

    // calm band behind the page
    p.shade(CORE_X,385.0f,1240.0f,600.0f,150.0f / 255.0f);

    // tabs
    for(int i=0;i<TAB_COUNT;i++){
        const float s = m_tabAnim[i];
        const float scale = 1.0f + 0.08f * s;
        const Rgb col = mix(theme.dim(),theme.accent(),s);
        const float alpha = lerp(0.70f,1.0f,s) * intro;
        const float x = tabX(i);
        if(s > 0.02f){
            p.glow(x,TAB_Y,m_tabLabel[i].w * scale + 160.0f,m_tabLabel[i].h * scale + 70.0f,theme.core(),0.22f * s * intro);
            const float half = m_tabLabel[i].w * scale * 0.5f + 20.0f;
            p.fadeLine(x - half,x - 6.0f,TAB_Y + 34.0f,theme.accent(),0.0f,0.8f * s * intro);
            p.fadeLine(x + 6.0f,x + half,TAB_Y + 34.0f,theme.accent(),0.8f * s * intro,0.0f);
            p.diamond(x,TAB_Y + 34.0f,4.0f,theme.accent(),s * intro);
        }
        p.label(m_tabLabel[i],x,TAB_Y,scale,col,alpha);
    }

    // divider under the tabs
    p.fadeLine(CORE_X - 560.0f,CORE_X,150.0f,theme.accent(),0.0f,0.28f * intro);
    p.fadeLine(CORE_X,CORE_X + 560.0f,150.0f,theme.accent(),0.28f * intro,0.0f);

    // back
    {
        const float s = m_backAnim;
        const float scale = 1.0f + 0.08f * s;
        p.flank(m_backLabel,BACK_X,BACK_Y,scale,s,intro);
        p.label(m_backLabel,BACK_X,BACK_Y,scale,mix(theme.dim(),theme.accent(),s),lerp(0.70f,1.0f,s) * intro);
    }

    m_backdrop.renderFront(renderer);
    m_backdrop.renderFade(renderer);     // the page (overlay) fades itself in
}

bool OptionMenuState::handleEvents(SDL_Event &event) {
    if(m_leaving) return true;

    if(InputUtils::IsPointerMotion(event)){
        float x,y;
        InputUtils::GetPointerPosition(event,m_renderer,x,y);
        m_hoverTab = tabAt(x,y);
        m_hoverBack = overBack(x,y);
        return false;
    }

    if(InputUtils::IsPointerDown(event)){
        float x,y;
        InputUtils::GetPointerPosition(event,m_renderer,x,y);
        if(overBack(x,y)){ leave(); return true; }
        const int tab = tabAt(x,y);
        if(tab >= 0) selectTab(tab);
        return true;
    }

    switch(InputUtils::GetMenuAction(event)){
        case InputUtils::MenuAction::BACK:     leave();                 return true;
        case InputUtils::MenuAction::TAB_PREV: selectTab(m_tab - 1);    return true;
        case InputUtils::MenuAction::TAB_NEXT: selectTab(m_tab + 1);    return true;
        default:                               return false;
    }
}
