//
// Created by LENOVO on 19-11-2026.
//
//
// Merchant upgrades: every number the shop sells lives in this one file. Change a price or a stat here and nothing
// else needs to be touched (the shop screen, the HUD and the gameplay code all read these tables).
//
// Currency = fruit (PlayerDetail::getFruit()). Magic is no longer bought or regenerated: it is only gained by HITTING
// enemies and bosses with the slash.
//
// "level" = how many times the upgrade has been bought. Level 0 is what a new game starts with.
// There is deliberately NO upgrade for the double jump (the broken wings): it stays as it is.
//
#pragma once

namespace Upgrades {

    enum Kind : int {
        SLASH_RANGE = 0,   // 3 levels
        SLASH_DAMAGE,      // 3 levels
        BEAM,              // 3 levels: more damage AND a cheaper magic drain
        HEART,             // 3 levels: +1 max heart each
        DASH,              // 1 level : the dash passes through trap hazards
        COUNT
    };

    constexpr int MAX_LEVEL[COUNT] = { 3, 3, 3, 3, 1 };

    // Fruit price of the NEXT level: COST[kind][levelYouHaveNow]. (A maxed upgrade has no price.)
    constexpr int COST[COUNT][3] = {
            {  3,  6, 10 },    // slash range
            {  4,  7, 12 },    // slash damage
            {  4,  8, 12 },    // beam
            {  5,  8, 12 },    // heart
            { 10,  0,  0 },    // dash
    };

    // ---- slash range: multiplier on the slash reach (hitbox + the arc drawn on screen) -------------------------------
    // Level 2 is the reach the game had before the shop existed (1.00). A new game starts shorter (level 0) and the
    // last level reaches further than the old slash. To make the OLD reach the start instead, set { 1.00, 1.10, 1.20, 1.30 }.
    constexpr float SLASH_RANGE_MUL[4] = { 0.70f, 0.85f, 1.00f, 1.20f };

    // ---- slash damage: multiplier on ENEMY_MELEE_DAMAGE (25: two slashes kill a 50 hp enemy; level 3 = 50: one slash) ---
    constexpr float SLASH_DAMAGE_MUL[4] = { 1.00f, 1.20f, 1.50f, 2.00f };

    // ---- beam: more damage per second, cheaper per second ------------------------------------------------------------
    constexpr float BEAM_DAMAGE_MUL[4] = { 1.00f, 1.25f, 1.50f, 1.80f };   // x ENEMY_BEAM_DAMAGE_PER_SEC
    constexpr float BEAM_COST_MUL[4]   = { 1.00f, 0.85f, 0.70f, 0.55f };   // x BEAM_MAGIC_DRAIN_PER_SEC

    // ---- hearts ---------------------------------------------------------------------------------------------------------
    constexpr int BASE_HEARTS = 5;     // max hearts with no upgrade; each HEART level adds one (5 -> 8)

    // ---- dash ------------------------------------------------------------------------------------------------------------
    // With the upgrade, trap hazards cannot hurt the player while dashing, plus this long after the dash ends so a dash
    // that finishes inside a spike strip does not punish you the instant it stops.
    constexpr float DASH_HAZARD_GRACE_S = 0.10f;

    // ---- magic (earned only by landing slashes) ---------------------------------------------------------------------------
    constexpr float MAX_MAGIC            = 100.0f;
    constexpr float STARTING_MAGIC       = 100.0f;   // a new game / a fresh run starts with this much (set 0 for none)
    constexpr float MAGIC_PER_ENEMY_HIT  = 10.0f;    // each slash that lands on an enemy (two slashes kill a 50 hp enemy = 20)
    constexpr float MAGIC_PER_BOSS_HIT   = 5.0f;     // each slash that lands on a boss or a boss part (bosses take many hits)
    // the beam, parries and reflected projectiles give nothing: the beam costs magic, so paying it back would be a loop

    constexpr const char* NAME[COUNT] = { "SLASH RANGE", "SLASH DAMAGE", "BEAM", "HEARTS", "DASH" };
}
