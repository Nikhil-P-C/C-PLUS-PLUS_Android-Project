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
// The freeze is an EXACT freeze-frame: the world, the spark, the particles, the weather and every wall-clock effect hold
// the pose they had on the clash (see utils/FrameClock.h) for this long, then everything carries on from that frame.
constexpr float PARRY_HITSTOP_S         = 0.25f;            // full parry freeze-frame (player AND enemy)
constexpr float PARRY_HITSTOP_BIG_S     = 0.25f;            // boss parries / QTE strikes
constexpr float PARRY_HITSTOP_PARTIAL_S = 0.25f;            // a deflected (not last) strike of a combo: lower this if three freezes in a row is too much
constexpr float PARRY_INVULN_S          = 2.5f;            // invulnerability after a full parry (2-3s). Does NOT make the player transparent / blink
constexpr int   PARRY_MAX_PER_ATTACK    = 3;               // one attack can be parried at most this many times (one per strike)
constexpr float PARRY_MAGIC_REWARD      = 35.0f;           // a big chunk of the 100 magic bar: parrying is the best way to fill it

// ---- multi-hit attacks (a combo / double strike): every strike is parried on its own, one by one ----
// A parry that is NOT the last strike of the attack only deflects that strike: the freeze, the spark and the clink all
// play, but the attacker is not staggered yet - it carries on into its next strike, which needs a fresh slash.
// The LAST strike's parry is the full one (stagger, knockback, riposte bonus).
constexpr float PARRY_SLOWMO_S          = 0.35f;           // after a full parry's freeze the world eases back in from slow motion over this long
constexpr float PARRY_SLOWMO_MIN        = 0.30f;           // ...starting at this speed (not used for deflected strikes of a combo: that would break the rhythm)
constexpr float PARRY_INVULN_PARTIAL_S  = 0.10f;           // i-frames after a deflected (not last) strike: short, so the next strike still has to be parried
constexpr float PARRY_PARTIAL_PUSH_MUL  = 0.35f;           // the player is shoved back less, so the next strike is still in reach
constexpr float PARRY_PARTIAL_MAGIC_MUL = 0.5f;            // magic reward of a deflected strike (the last one pays in full)

// ---- the tip spark: a white blade-flare at the exact point where the tip of the slash meets the enemy's attack box ----
constexpr float PARRY_SPARK_WIDTH       = 800.0f;          // half of the 1600px logical screen, centred on the contact point
constexpr float PARRY_SPARK_LIFE_S      = 0.26f;           // real-time seconds (it keeps animating through the hit-stop)

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
