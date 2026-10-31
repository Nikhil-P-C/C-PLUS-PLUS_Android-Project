//
// Parry: tuning + the player's parry state.
//
// The parry IS the player's slash, timed against an enemy attack (no separate button). Starting a slash opens a TIGHT
// window (about 7 frames at 60fps) at the start of the swing. If a parryable enemy attack reaches the player while that
// window is open, the attack is parried instead of hurting; a slash that starts too early or too late just swings.
//
//   slash starts --> [ window 7f ~0.117s ] --enemy attack arrives--> hit-stop 0.10s, spark, shake, magic, stagger, i-frames
//
// Everything here is in SECONDS on game time (dt): frame-rate independent, and it freezes with the hit-stop.
// The enemy side (what is parryable, stagger) lives in level/Enemies.* and level/Bosses.*; the one place that turns a
// parry into feedback for the player is GameState::onParrySuccess.
//
#pragma once
#include <algorithm>

// ---- timing (frames @ 60fps, written as frames/60 so the intent is readable) ----
constexpr float PARRY_WINDOW_S          = 7.0f  / 60.0f;   // ~0.117s: tight, opens the moment a slash starts
constexpr float PARRY_HITSTOP_S         = 0.10f;           // success freeze (player AND enemy): 80-120ms sells the parry
constexpr float PARRY_HITSTOP_BIG_S     = 0.12f;           // boss parries / QTE strikes
constexpr float PARRY_INVULN_S          = 0.45f;           // brief invulnerability after a successful parry
constexpr float PARRY_MAGIC_REWARD      = 35.0f;           // a big chunk of the 100 magic bar: parrying is the best way to fill it

// ---- the other side ----
constexpr unsigned int PARRY_STAGGER_MS = 800;             // enemy stunned this long (0.6 - 1s)
constexpr float PARRY_BONUS_DAMAGE_MUL  = 2.0f;            // the first slash that lands inside the stagger hits this much harder
constexpr float PARRY_REACH_PAD         = 28.0f;           // px: the attack only has to come this close to the player to be parried
constexpr float PARRY_PLAYER_PUSH       = 240.0f;          // px/s shove away from what was parried
constexpr float PARRY_PITCH_MIN         = 0.94f;           // each parry clink is played at a slightly different pitch
constexpr float PARRY_PITCH_MAX         = 1.08f;
constexpr unsigned int PARRY_CUE_LEAD_MS = 260;            // the "parry now" flash on a parryable wind-up starts this long before the hit

struct ParryState {
    float window = 0.0f;     // >0 while the parry window is open (seconds left)
    float invuln = 0.0f;     // >0 while invulnerable after a success
    bool  succeeded = false; // this window already parried something (one parry per slash)
    int   id = 0;            // bumped every time a window opens

    bool active() const { return window > 0.0f; }

    // a slash just started
    void open() { window = PARRY_WINDOW_S; succeeded = false; ++id; }

    void tick(float dt) {
        if (window > 0.0f) window = std::max(0.0f, window - dt);
        if (invuln > 0.0f) invuln = std::max(0.0f, invuln - dt);
    }

    // a parry landed: close the window, grant the i-frames
    void succeed() { window = 0.0f; invuln = PARRY_INVULN_S; succeeded = true; }

    void clear() { const int keep = id; *this = ParryState(); id = keep; }
};
