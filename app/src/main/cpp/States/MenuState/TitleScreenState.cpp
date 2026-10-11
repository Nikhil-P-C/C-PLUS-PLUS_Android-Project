//
// Created by LENOVO on 28-04-2026.
//
#include "TitleScreenState.h"
#include <cmath>
#include <cstring>
#include "MenuState.h"

using namespace MenuKit;

namespace {
    constexpr const char* GAME_TITLE = "DINO";      // the name shown above the divider
    constexpr int   TITLE_FONT = 110, PROMPT_FONT = 34;
    constexpr float CORE_X = 800.0f, CORE_Y = 300.0f;
    constexpr float TITLE_Y = 250.0f, DIVIDER_Y = 335.0f, PROMPT_Y = 520.0f;
    constexpr float LEAVE_TIME = 0.45f;
}

TitleScreenState::TitleScreenState(SDL_Renderer *renderer)
        : m_renderer(renderer),
          m_backdrop(renderer,CORE_X,CORE_Y,1.2f),
          m_font(TITLE_FONT) {
    this->Name = "TitleScreen";

    m_title.set(renderer,m_font,GAME_TITLE,TITLE_FONT);
    setPrompt(detectPrompt());
}

TitleScreenState::~TitleScreenState() {
    LOGI("TitleScreenState destructor:%p",this);
}

TitleScreenState::PromptKind TitleScreenState::detectPrompt() const {
    if(SDL_HasGamepad()) return PromptKind::BUTTON;
    const char* platform = SDL_GetPlatform();
    if(platform && (std::strcmp(platform,"Android") == 0 || std::strcmp(platform,"iOS") == 0))
        return PromptKind::TOUCH;
    return PromptKind::KEY;
}

void TitleScreenState::setPrompt(PromptKind kind) {
    m_kind = kind;
    const char* text = "PRESS ANY KEY";
    if(kind == PromptKind::BUTTON) text = "PRESS ANY BUTTON";
    else if(kind == PromptKind::TOUCH) text = "TOUCH ANYWHERE";
    m_prompt.set(m_renderer,m_font,text,PROMPT_FONT);
}

void TitleScreenState::update(float dt) {
    m_backdrop.update(dt);

    m_recheckTimer += dt;
    if(m_recheckTimer >= 0.5f){
        m_recheckTimer = 0.0f;
        const PromptKind kind = detectPrompt();
        if(kind != m_kind) setPrompt(kind);
    }

    if(m_leaving){
        m_leaveTimer += dt;
        if(m_leaveTimer >= LEAVE_TIME){
            m_leaving = false;
            m_leaveTimer = -1000.0f;      // never fire twice before this state is replaced
            Engine::Get().changeState(std::make_unique<MenuState>(m_renderer));
        }
    }
}

void TitleScreenState::render(SDL_Renderer *renderer) {
    const MenuTheme& theme = MenuTheme::get();
    const MenuPainter& p = m_backdrop.painter();
    const float t = m_backdrop.time();

    m_backdrop.render(renderer);

    // calm band behind the title block
    p.shade(CORE_X,400.0f,1100.0f,520.0f,150.0f / 255.0f);

    // game name + divider ornament (same as the main menu's)
    const float titleIn = smooth((t - 0.3f) / 0.9f);
    p.label(m_title,CORE_X,TITLE_Y + (1.0f - titleIn) * 16.0f,1.0f + 0.015f * theme.pulse(),theme.accent(),titleIn);
    {
        const float a = 0.8f * titleIn;
        p.fadeLine(CORE_X - 260.0f,CORE_X - 18.0f,DIVIDER_Y,theme.accent(),0.0f,a);
        p.fadeLine(CORE_X + 18.0f,CORE_X + 260.0f,DIVIDER_Y,theme.accent(),a,0.0f);
        p.diamond(CORE_X,DIVIDER_Y,7.0f + 2.0f * theme.pulse(),theme.accent(),a);
        p.diamond(CORE_X - 30.0f,DIVIDER_Y,2.5f,theme.accent(),a * 0.8f);
        p.diamond(CORE_X + 30.0f,DIVIDER_Y,2.5f,theme.accent(),a * 0.8f);
    }

    // prompt: slow breathing, ornament like a selected menu entry
    const float promptIn = smooth((t - 1.1f) / 0.6f);
    const float breathe  = 0.55f + 0.45f * (0.5f + 0.5f * std::sin(t * 2.4f));
    p.flank(m_prompt,CORE_X,PROMPT_Y,1.0f,breathe,promptIn);
    p.label(m_prompt,CORE_X,PROMPT_Y,1.0f,mix(theme.dim(),theme.accent(),breathe),breathe * promptIn);

    m_backdrop.renderFront(renderer);
    m_backdrop.renderFade(renderer);
}

bool TitleScreenState::handleEvents(SDL_Event &event){
    if(m_leaving || m_leaveTimer != 0.0f) return true;

    //tap / click / any key / any gamepad button moves on to the menu
    if(InputUtils::IsAnyPress(event)){
        m_leaving = true;
        m_backdrop.burst(CORE_X,PROMPT_Y);
        m_backdrop.beginFadeOut(LEAVE_TIME);
        Engine::Get().getAssetManager().playSound(SoundType::SFX_UI_ENTER);
        return true;
    }
    return false;
}
