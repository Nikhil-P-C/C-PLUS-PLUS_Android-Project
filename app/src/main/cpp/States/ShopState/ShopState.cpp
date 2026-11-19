//
// Created by LENOVO on 19-11-2026.
//
//
// The merchant's shop. See ShopState.h.
//
#include <SDL3/SDL.h>
#include <cmath>
#include <cstdio>
#include <string>

#include "ShopState.h"
#include "engine/Engine.h"
#include "States/GameState/GameState.h"
#include "States/InputOverlayState/ButtonOverlay.h"
#include "States/InputOverlayState/SepJoysticknButton.h"
#include "States/InputOverlayState/KeyboardOverlay.h"
#include "States/InputOverlayState/GamepadOverlay.h"
#include "States/DebugState/DebugState.h"
#include "States/HUDOverlayState/HUDOverlayState.h"

using namespace MenuKit;

namespace {
    constexpr float CORE_X = 800.0f, CORE_Y = 340.0f;
    constexpr float TITLE_Y = 58.0f, SUB_Y = 104.0f, DIVIDER_Y = 132.0f;
    constexpr float LIST_TOP = 196.0f, LIST_STEP = 76.0f;
    constexpr float PIPS_X = 930.0f, PIP_STEP = 34.0f;
    constexpr float COST_RIGHT = Layout::CONTROL_RIGHT;
    constexpr int   TITLE_FONT = 64, NAME_FONT = 34, EFFECT_FONT = 22, COST_FONT = 34, ENTRY_FONT = 38, HINT_FONT = 22;
    constexpr float IGNORE_INPUT_S = 0.30f;                  // a press that was meant for the game cannot buy anything
    const Rgb BAD_RED{0.95f, 0.36f, 0.30f};
    const Rgb OK_GREEN{0.52f, 0.92f, 0.62f};
}

ShopState::ShopState(SDL_Renderer* renderer, GameState* gameState)
        : m_renderer(renderer),
          m_gameState(gameState),
          m_backdrop(renderer, CORE_X, CORE_Y, 0.3f, true),
          m_font(ENTRY_FONT) {
    this->Name = "ShopState";
    LOGI("Shop construct:%p", this);

    m_title.set(renderer, m_font, "MERCHANT", TITLE_FONT);
    m_subtitle.set(renderer, m_font, "Upgrades are paid in fruit. Magic comes only from hitting enemies.", EFFECT_FONT);
    m_leaveLabel.set(renderer, m_font, "LEAVE", ENTRY_FONT);
    m_hint.set(renderer, m_font, "ENTER / tap again: buy        ESC / back: leave", HINT_FONT);
    for(int i = 0; i < Upgrades::COUNT; ++i)
        m_name[i].set(renderer, m_font, Upgrades::NAME[i], NAME_FONT);
    rebuildLabels();

    // start on the first thing that can be bought, otherwise on the first row
    int start = 0;
    for(int i = 0; i < Upgrades::COUNT; ++i){
        if(PlayerDetail::getInstance().canBuyUpgrade((Upgrades::Kind)i)){ start = i; break; }
    }
    m_rows.setup(ROW_COUNT, LIST_TOP, LIST_STEP, start, 0.15f, 0.08f);
}

ShopState::~ShopState(){
    LOGI("Shop destructor:%p", this);
}

std::string ShopState::effectText(Upgrades::Kind kind) const{
    const PlayerDetail& pd = PlayerDetail::getInstance();
    const int lv = pd.getUpgradeLevel(kind);
    const bool maxed = pd.isUpgradeMaxed(kind);
    const int next = maxed ? lv : lv + 1;
    char buf[96];
    switch(kind){
        case Upgrades::SLASH_RANGE:
            if(maxed) std::snprintf(buf, sizeof(buf), "Reach %d%%", (int)std::lround(Upgrades::SLASH_RANGE_MUL[lv] * 100.0f));
            else      std::snprintf(buf, sizeof(buf), "Reach %d%% -> %d%%", (int)std::lround(Upgrades::SLASH_RANGE_MUL[lv] * 100.0f),
                                    (int)std::lround(Upgrades::SLASH_RANGE_MUL[next] * 100.0f));
            break;
        case Upgrades::SLASH_DAMAGE:{
            const int cur = (int)std::lround(ENEMY_MELEE_DAMAGE * Upgrades::SLASH_DAMAGE_MUL[lv]);
            const int nxt = (int)std::lround(ENEMY_MELEE_DAMAGE * Upgrades::SLASH_DAMAGE_MUL[next]);
            if(maxed) std::snprintf(buf, sizeof(buf), "Damage %d", cur);
            else      std::snprintf(buf, sizeof(buf), "Damage %d -> %d", cur, nxt);
            break;
        }
        case Upgrades::BEAM:{
            const int d0 = (int)std::lround(ENEMY_BEAM_DAMAGE_PER_SEC * Upgrades::BEAM_DAMAGE_MUL[lv]);
            const int d1 = (int)std::lround(ENEMY_BEAM_DAMAGE_PER_SEC * Upgrades::BEAM_DAMAGE_MUL[next]);
            const int c0 = (int)std::lround(BEAM_MAGIC_DRAIN_PER_SEC * Upgrades::BEAM_COST_MUL[lv]);
            const int c1 = (int)std::lround(BEAM_MAGIC_DRAIN_PER_SEC * Upgrades::BEAM_COST_MUL[next]);
            if(maxed) std::snprintf(buf, sizeof(buf), "Damage %d/s   Cost %d/s", d0, c0);
            else      std::snprintf(buf, sizeof(buf), "Dmg %d -> %d/s   Cost %d -> %d/s", d0, d1, c0, c1);
            break;
        }
        case Upgrades::HEART:
            if(maxed) std::snprintf(buf, sizeof(buf), "%d hearts", Upgrades::BASE_HEARTS + lv);
            else      std::snprintf(buf, sizeof(buf), "Hearts %d -> %d", Upgrades::BASE_HEARTS + lv, Upgrades::BASE_HEARTS + next);
            break;
        case Upgrades::DASH:
        default:
            std::snprintf(buf, sizeof(buf), "%s", maxed ? "Dash passes through trap hazards" : "Dash through trap hazards unharmed");
            break;
    }
    return buf;
}

void ShopState::rebuildLabels(){
    PlayerDetail& pd = PlayerDetail::getInstance();
    for(int i = 0; i < Upgrades::COUNT; ++i){
        const Upgrades::Kind kind = (Upgrades::Kind)i;
        const std::string effect = effectText(kind);
        m_effect[i].set(m_renderer, m_font, effect.c_str(), EFFECT_FONT);
        if(pd.isUpgradeMaxed(kind)){
            m_cost[i].set(m_renderer, m_font, kind == Upgrades::DASH ? "OWNED" : "MAX", COST_FONT);
            m_costIcon[i] = false;
        } else {
            const std::string c = std::to_string(pd.nextUpgradeCost(kind));
            m_cost[i].set(m_renderer, m_font, c.c_str(), COST_FONT);
            m_costIcon[i] = true;
        }
    }
    const std::string w = "x " + std::to_string(pd.getFruit());
    m_wallet.set(m_renderer, m_font, w.c_str(), 40);
}

void ShopState::drawFruitIcon(SDL_Renderer* renderer, float x, float y, float size) const{
    SDL_Texture* tex = Engine::Get().getAssetManager().getTexture(TextureType::FRUIT_APPLE);
    if(!tex) return;
    const int frame = (int)(m_backdrop.time() * 18.0f) % 17;          // the 17-frame apple spin of the pickup
    const SDL_FRect src{(float)frame * 32.0f, 0.0f, 32.0f, 32.0f};
    const SDL_FRect dst{x - size * 0.5f, y - size * 0.5f, size, size};
    SDL_RenderTexture(renderer, tex, &src, &dst);
}

void ShopState::update(float dt){
    m_age += dt;
    m_backdrop.update(dt);
    m_rows.update(dt);
    if(m_denyTimer > 0.0f) m_denyTimer = std::max(0.0f, m_denyTimer - dt);
    for(float& f : m_flash) if(f > 0.0f) f = std::max(0.0f, f - dt * 2.2f);
    if(m_walletPop > 0.0f) m_walletPop = std::max(0.0f, m_walletPop - dt * 3.0f);

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

void ShopState::render(SDL_Renderer* renderer){
    const MenuTheme& theme = MenuTheme::get();
    const MenuPainter& p = m_backdrop.painter();
    const PlayerDetail& pd = PlayerDetail::getInstance();
    const float t = smooth(m_backdrop.time() / 0.3f);

    m_backdrop.render(renderer);

    // calm band behind the whole list
    p.shade(CORE_X, 400.0f, 1100.0f, 620.0f, 140.0f / 255.0f * t);

    // title, subtitle, divider
    p.label(m_title, CORE_X, TITLE_Y, 1.0f, theme.accent(), t);
    p.label(m_subtitle, CORE_X, SUB_Y, 1.0f, theme.dim(), 0.85f * t);
    {
        const float a = 0.8f * t;
        p.fadeLine(CORE_X - 300.0f, CORE_X - 18.0f, DIVIDER_Y, theme.accent(), 0.0f, a);
        p.fadeLine(CORE_X + 18.0f, CORE_X + 300.0f, DIVIDER_Y, theme.accent(), a, 0.0f);
        p.diamond(CORE_X, DIVIDER_Y, 7.0f + 2.0f * theme.pulse(), theme.accent(), a);
    }

    // the fruit wallet, top right
    {
        const float pop = 1.0f + 0.25f * m_walletPop;
        const float wx = Layout::ROW_RIGHT - 10.0f;
        p.label(m_wallet, wx, TITLE_Y + 2.0f, pop, Rgb{1.0f, 1.0f, 1.0f}, t, Align::RIGHT);
        drawFruitIcon(renderer, wx - m_wallet.w * pop - 34.0f, TITLE_Y + 2.0f, 52.0f * pop);
    }

    // upgrade rows
    for(int i = 0; i < Upgrades::COUNT; ++i){
        const Upgrades::Kind kind = (Upgrades::Kind)i;
        const float s  = m_rows.focusAnim(i);
        const float rv = m_rows.reveal(i);
        float shake = 0.0f;
        if(m_denyRow == i && m_denyTimer > 0.0f) shake = std::sin(m_denyTimer * 90.0f) * 8.0f * (m_denyTimer / 0.28f);
        const float y = m_rows.rowY(i) + (1.0f - rv) * 14.0f;
        const float x0 = shake;

        const bool maxed = pd.isUpgradeMaxed(kind);
        const bool afford = pd.canBuyUpgrade(kind);

        p.rowFrame(Layout::ROW_LEFT + x0, Layout::ROW_RIGHT + x0, y, s, rv);
        if(m_flash[i] > 0.0f)
            p.glow(CORE_X + x0, y, 900.0f, 110.0f, theme.accent(), 0.7f * m_flash[i]);

        const Rgb nameCol = maxed ? mix(OK_GREEN, theme.accent(), s * 0.5f) : mix(theme.dim(), theme.accent(), s);
        p.label(m_name[i],   Layout::LABEL_X + x0, y - 12.0f, 1.0f, nameCol, lerp(0.70f, 1.0f, s) * rv, Align::LEFT);
        p.label(m_effect[i], Layout::LABEL_X + x0, y + 19.0f, 1.0f, theme.dim(), lerp(0.60f, 0.95f, s) * rv, Align::LEFT);

        // level pips: filled = bought, outline = not yet, the next one breathes while the row is focused
        const int maxLv = Upgrades::MAX_LEVEL[i];
        const int lv = pd.getUpgradeLevel(kind);
        const float firstX = PIPS_X - PIP_STEP * 0.5f * (float)(maxLv - 1) + x0;
        for(int j = 0; j < maxLv; ++j){
            const float px = firstX + PIP_STEP * (float)j;
            if(j < lv){
                p.diamond(px, y, 9.0f, maxed ? OK_GREEN : theme.accent(), 0.95f * rv);
            } else {
                const bool next = (j == lv);
                const float grow = next ? (1.0f + 0.25f * theme.pulse() * s) : 1.0f;
                p.diamondOutline(px, y, 9.0f * grow, 2.0f, next ? theme.accent() : theme.dim(), (next ? 0.9f : 0.45f) * rv);
            }
        }

        // price (fruit icon + number) or MAX / OWNED
        Rgb costCol = maxed ? OK_GREEN : (afford ? mix(theme.dim(), Rgb{1.0f, 1.0f, 1.0f}, 0.5f + 0.5f * s) : BAD_RED);
        p.label(m_cost[i], COST_RIGHT + x0, y, 1.0f, costCol, lerp(0.75f, 1.0f, s) * rv, Align::RIGHT);
        if(m_costIcon[i]) drawFruitIcon(renderer, COST_RIGHT + x0 - m_cost[i].w - 30.0f, y, 40.0f);
    }

    // LEAVE
    {
        const int i = ROW_LEAVE;
        const float s  = m_rows.focusAnim(i);
        const float rv = m_rows.reveal(i);
        const float y  = m_rows.rowY(i) + (1.0f - rv) * 14.0f + 6.0f;
        const float scale = 1.0f + 0.10f * s + 0.02f * s * theme.pulse();
        p.flank(m_leaveLabel, CORE_X, y, scale, s, rv);
        p.label(m_leaveLabel, CORE_X, y, scale, mix(theme.dim(), theme.accent(), s), lerp(0.70f, 1.0f, s) * rv);
    }

    p.label(m_hint, CORE_X, 690.0f, 1.0f, theme.dim(), 0.7f * t);

    m_backdrop.renderFront(renderer);
    m_backdrop.renderFade(renderer);
}

void ShopState::tryBuy(Upgrades::Kind kind){
    PlayerDetail& pd = PlayerDetail::getInstance();
    AssetManager& assets = Engine::Get().getAssetManager();
    if(pd.isUpgradeMaxed(kind)){
        assets.playSoundPitched(SoundType::SFX_UI_MOVE, 0.8f, 0.7f);
        return;                                                // nothing left to buy: no shake, it is not an error
    }
    if(!pd.canBuyUpgrade(kind)){                               // not enough fruit
        m_denyRow = (int)kind;
        m_denyTimer = 0.28f;
        assets.playSoundPitched(SoundType::SFX_UI_MOVE, 1.0f, 0.55f);
        return;
    }
    pd.buyUpgrade(kind);
    m_flash[kind] = 1.0f;
    m_walletPop = 1.0f;
    assets.playSound(SoundType::SFX_UI_CONFIRM);
    m_backdrop.burst(CORE_X, m_rows.rowY((int)kind));
    rebuildLabels();                                           // new level, new price, new wallet
}

void ShopState::activate(int row){
    if(m_transitioning) return;
    if(row == ROW_LEAVE){
        Engine::Get().getAssetManager().playSound(SoundType::SFX_UI_ENTER);
        leave();
    } else if(row >= 0 && row < Upgrades::COUNT){
        tryBuy((Upgrades::Kind)row);
    }
}

// back into the level: same hand-back as the pause screen (HUD, then the input overlay, the debug overlay on top)
void ShopState::leave(){
    GameData::getInstance().setPaused(false);
    m_transitioning = true;
    Engine::Get().popState();                                  // this screen
    Engine::Get().pushOverlayState(std::make_unique<HUDOverlayState>(m_renderer));   // built now, so new hearts show
    if(GameData::getInstance().getControlType() == ControlType::BUTTONS)
        Engine::Get().pushOverlayState(std::make_unique<ButtonOverlay>(m_renderer));
    if(GameData::getInstance().getControlType() == ControlType::SEP_JUMP_W_JOYSTICK)
        Engine::Get().pushOverlayState(std::make_unique<SepJoysticknButton>(m_renderer));
    if(GameData::getInstance().getControlType() == ControlType::KEYBOARD)
        Engine::Get().pushOverlayState(std::make_unique<KeyboardOverlay>(m_renderer));
    if(GameData::getInstance().getControlType() == ControlType::GAMEPAD)
        Engine::Get().pushOverlayState(std::make_unique<GamepadOverlay>(m_renderer));
    if(GameData::getInstance().isDebugEnabled() && m_gameState){
        auto dbg = std::make_unique<DebugState>(m_renderer, m_gameState);
        dbg->setJumpEnabled(true);
        Engine::Get().pushOverlayState(std::move(dbg));
    }
}

bool ShopState::handleEvents(SDL_Event& event){
    if(m_transitioning) return true;

    if(m_nav.handleEvent(event)) return true;

    const bool armed = m_age >= IGNORE_INPUT_S;

    if(InputUtils::IsPointerMotion(event)){
        float x, y;
        InputUtils::GetPointerPosition(event, m_renderer, x, y);
        const int row = m_rows.hit(x, y, Layout::ROW_LEFT - 40.0f, Layout::ROW_RIGHT + 40.0f, 36.0f);
        if(row >= 0 && m_rows.setFocus(row)) Engine::Get().getAssetManager().playSound(SoundType::SFX_UI_MOVE);
        return true;
    }

    if(InputUtils::IsPointerDown(event)){
        if(!armed) return true;
        float x, y;
        InputUtils::GetPointerPosition(event, m_renderer, x, y);
        const int row = m_rows.hit(x, y, Layout::ROW_LEFT - 40.0f, Layout::ROW_RIGHT + 40.0f, 36.0f);
        if(row < 0) return true;                               // a tap beside the list does nothing (LEAVE is a row)
        if(row == ROW_LEAVE || row == m_rows.focus()) activate(row);   // already selected (mouse hover / 2nd tap): act
        else if(m_rows.setFocus(row)) Engine::Get().getAssetManager().playSound(SoundType::SFX_UI_MOVE);   // 1st tap: select
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
            if(armed) activate(m_rows.focus());
            return true;
        case InputUtils::MenuAction::BACK:
            if(armed) activate(ROW_LEAVE);
            return true;
        default:
            return false;
    }
}
