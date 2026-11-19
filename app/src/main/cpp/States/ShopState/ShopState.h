//
// Created by LENOVO on 19-11-2026.
//
#pragma once
//
// The merchant's shop: a pause-style screen (MenuKit look, the frozen game dimmed underneath) where fruit buys upgrades.
//
//   SLASH RANGE   3 levels     SLASH DAMAGE   3 levels     BEAM   3 levels (more damage, cheaper magic)
//   HEARTS        3 levels     DASH           1 level  (the dash passes through trap hazards)
//   (no upgrade for the broken wings / double jump)
//
// Every price and stat lives in utils/Upgrades.h. Opened by GameState::openShop() when the player interacts with a
// merchant (level line "V x y"), closed with LEAVE / back, which puts the gameplay overlays back like the pause screen does.
//
//   keyboard / gamepad: up-down select, confirm buys, back leaves      mouse: hover selects, click buys
//   touch: tap a row to select it, tap it again to buy                 (tap LEAVE to go back)
//
#pragma once
#include "utils/utils.h"
#include "SDL3/SDL.h"
#include "State/State.h"
#include "States/GameState/GameState.h"
#include "States/MenuState/MenuKit.h"
#include "utils/Upgrades.h"

#define LOG_TAG "ShopState"

class ShopState : public State{
public:
    ShopState(SDL_Renderer* renderer, GameState* gameState);
    void render(SDL_Renderer* renderer) override;
    void update(float dt) override;
    bool handleEvents(SDL_Event& event) override;
    ~ShopState() override;

private:
    static constexpr int ROW_LEAVE = Upgrades::COUNT;        // the last row
    static constexpr int ROW_COUNT = Upgrades::COUNT + 1;

    void rebuildLabels();                                    // after opening and after every purchase
    void activate(int row);
    void tryBuy(Upgrades::Kind kind);
    void leave();
    void drawFruitIcon(SDL_Renderer* renderer, float x, float y, float size) const;
    std::string effectText(Upgrades::Kind kind) const;

    SDL_Renderer* m_renderer = nullptr;
    GameState* m_gameState = nullptr;

    MenuBackdrop m_backdrop;
    MenuFont     m_font;
    MenuLabel    m_title, m_subtitle, m_wallet, m_hint, m_leaveLabel;
    MenuLabel    m_name[Upgrades::COUNT], m_effect[Upgrades::COUNT], m_cost[Upgrades::COUNT];
    bool         m_costIcon[Upgrades::COUNT] = {};           // true = the cost label is a number (draw the fruit icon)
    MenuRows     m_rows;
    InputUtils::MenuNavRepeater m_nav;

    float m_age = 0.0f;                                      // seconds open: early presses are ignored
    float m_denyTimer = 0.0f;  int m_denyRow = -1;           // "cannot afford": the row shakes
    float m_flash[ROW_COUNT] = {};                           // purchase flash per row
    float m_walletPop = 0.0f;                                // the fruit counter pulses when it changes
    bool  m_transitioning = false;
};

