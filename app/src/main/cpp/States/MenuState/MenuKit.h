//
// MenuKit: the shared look + helpers for every menu screen
// (main, options, audio, control, advance, edit).
//
//  MenuTheme     global (survives state changes) colour cycle + heartbeat, so the light keeps
//                breathing / changing colour while you move between menus
//  MenuBackdrop  black screen, breathing core of light, floating motes, selection sparks, fades.
//                Motes   = AmbientParticles (theme MENU)   <- the in-game ambient mote system
//                Sparks  = ParticleSystem (screen space)   <- the in-game particle system
//  MenuPainter   glow / fade line / diamond / text drawing shared by every screen
//  MenuFont/Label  pixel font + cached text textures
//  MenuRows      focus + reveal animation for a vertical list of rows
//
#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <memory>
#include "States/GameState/AmbientParticles.h"
#include "States/GameState/ParticleSystem.h"

namespace MenuKit {
    constexpr float SCREEN_W = 1600.0f;
    constexpr float SCREEN_H = 720.0f;

    struct Rgb { float r, g, b; };

    inline float clamp01(float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }
    inline float lerp(float a, float b, float t) { return a + (b - a) * t; }
    inline float smooth(float t) { t = clamp01(t); return t * t * (3.0f - 2.0f * t); }
    inline Uint8 to8(float v) { return static_cast<Uint8>(clamp01(v) * 255.0f + 0.5f); }
    inline Rgb mix(const Rgb& a, const Rgb& b, float t) { return {lerp(a.r, b.r, t), lerp(a.g, b.g, t), lerp(a.b, b.b, t)}; }
    // frame-rate independent ease toward a target
    inline float approach(float cur, float target, float rate, float dt) {
        const float k = rate * dt;
        return cur + (target - cur) * (k > 1.0f ? 1.0f : k);
    }

    enum class Align { LEFT, CENTER, RIGHT };

    // Shared geometry of the option pages (audio / control / advance): label on the left,
    // the row's control (slider value, radio, ON/OFF) on the right.
    namespace Layout {
        constexpr float ROW_LEFT  = 420.0f;     // focus ornament diamonds sit at ROW_LEFT / ROW_RIGHT
        constexpr float ROW_RIGHT = 1180.0f;
        constexpr float LABEL_X   = 470.0f;     // left edge of a row label
        constexpr float CONTROL_RIGHT = 1130.0f;// right edge of the control column
        constexpr float ROW_HALF_H = 40.0f;     // pointer hit half-height of a row
    }
}

// ---------------------------------------------------------------------------------------------
// Colour cycle + heartbeat. Advanced by whichever MenuBackdrop is alive (one at a time).
// ---------------------------------------------------------------------------------------------
class MenuTheme {
public:
    static MenuTheme& get();
    void update(float dt);

    const MenuKit::Rgb& core()   const { return m_core; }    // the light itself
    const MenuKit::Rgb& accent() const { return m_accent; }  // selected text / ornaments
    const MenuKit::Rgb& dim()    const { return m_dim; }     // idle text
    const MenuKit::Rgb& mote()   const { return m_mote; }    // floating particles
    float pulse() const { return m_pulse; }                  // 0..1, fast attack / slow release

private:
    MenuTheme() { updatePalette(); }
    void updatePalette();
    void updateHeartbeat(float dt);

    float m_time = 0.0f, m_beatClock = 0.0f, m_pulse = 0.0f;
    MenuKit::Rgb m_core{0.22f, 0.52f, 1.0f}, m_accent{1, 1, 1}, m_dim{0.5f, 0.5f, 0.5f}, m_mote{1, 1, 1};
};

// ---------------------------------------------------------------------------------------------
// Procedural textures shared by all menu states (created once, freed when the last menu state dies)
// ---------------------------------------------------------------------------------------------
struct MenuAssets {
    ~MenuAssets();
    static std::shared_ptr<MenuAssets> acquire(SDL_Renderer* renderer);
    SDL_Texture* glow  = nullptr;   // additive radial falloff
    SDL_Texture* shade = nullptr;   // same shape, alpha blended (drawn black to calm the area behind text)
};

// ---------------------------------------------------------------------------------------------
// Text
// ---------------------------------------------------------------------------------------------
class MenuFont {
public:
    explicit MenuFont(int defaultSize = 38);
    ~MenuFont();
    MenuFont(const MenuFont&) = delete;
    MenuFont& operator=(const MenuFont&) = delete;
    bool ok() const { return m_font != nullptr; }
    // anti-aliased white text, tint it with SDL_SetTextureColorMod. Caller owns the texture.
    SDL_Texture* render(SDL_Renderer* renderer, const char* text, int size = 0);
private:
    SDL_IOStream* m_io = nullptr;
    TTF_Font* m_font = nullptr;
    int m_size = 38;
};

struct MenuLabel {
    MenuLabel() = default;
    ~MenuLabel() { reset(); }
    MenuLabel(const MenuLabel&) = delete;
    MenuLabel& operator=(const MenuLabel&) = delete;
    void set(SDL_Renderer* renderer, MenuFont& font, const char* text, int fontSize = 0);
    void reset();
    SDL_Texture* tex = nullptr;
    float w = 0.0f, h = 0.0f;
};

// ---------------------------------------------------------------------------------------------
// Drawing helpers. Cheap to construct, one per state.
// ---------------------------------------------------------------------------------------------
class MenuPainter {
public:
    explicit MenuPainter(SDL_Renderer* renderer);

    void glow(float cx, float cy, float w, float h, MenuKit::Rgb c, float alpha) const;
    void shade(float cx, float cy, float w, float h, float alpha) const;                  // soft black patch
    void fadeLine(float x0, float x1, float y, MenuKit::Rgb c, float a0, float a1, float halfThickness = 1.0f) const;
    void diamond(float cx, float cy, float size, MenuKit::Rgb c, float a) const;
    void diamondOutline(float cx, float cy, float size, float thickness, MenuKit::Rgb c, float a) const;
    void label(const MenuLabel& l, float x, float y, float scale, MenuKit::Rgb c, float alpha,
               MenuKit::Align align = MenuKit::Align::CENTER) const;
    // focus ornament around a centred label (main-menu style): glow, diamond each side, fading lines outward
    void flank(const MenuLabel& l, float cx, float y, float scale, float focus, float reveal) const;
    // focus ornament for a list row spanning [left,right]: soft glow, a diamond at each end, fading lines outward
    void rowFrame(float left, float right, float y, float focus, float reveal) const;

private:
    SDL_Renderer* m_renderer;
    std::shared_ptr<MenuAssets> m_assets;
};

// ---------------------------------------------------------------------------------------------
// Vertical list of rows: eased focus (0..1 per row), staggered reveal, hit testing
// ---------------------------------------------------------------------------------------------
class MenuRows {
public:
    static constexpr int MAX_ROWS = 8;
    void setup(int count, float firstY, float step, int initialFocus = 0,
               float revealDelay = 0.10f, float revealStep = 0.08f);
    void update(float dt);
    void restartReveal() { m_time = 0.0f; }

    int   count() const { return m_count; }
    int   focus() const { return m_focus; }
    bool  setFocus(int index);                 // true if it changed
    bool  move(int dir);                       // wraps around, true if it changed
    float rowY(int i) const { return m_firstY + m_step * static_cast<float>(i); }
    float focusAnim(int i) const { return m_anim[i]; }
    float reveal(int i) const;                 // smoothed 0..1
    // row under (px,py) inside [x0,x1] and +-halfH around the row centre, -1 if none
    int   hit(float px, float py, float x0, float x1, float halfH) const;

private:
    int m_count = 0, m_focus = 0;
    float m_firstY = 0.0f, m_step = 0.0f, m_revealDelay = 0.1f, m_revealStep = 0.08f, m_time = 0.0f;
    float m_anim[MAX_ROWS] = {};
};

// ---------------------------------------------------------------------------------------------
// The whole animated background
// ---------------------------------------------------------------------------------------------
class MenuBackdrop {
public:
    // introTime: seconds to fade in from black. coreX/coreY: where the light sits.
    // overlay=true (pause): instead of a black screen the game underneath is dimmed, the light is
    // softer, and the intro eases the dimming in rather than fading from black.
    MenuBackdrop(SDL_Renderer* renderer, float coreX = 800.0f, float coreY = 290.0f, float introTime = 1.2f,
                 bool overlay = false);

    void update(float dt);
    void render(SDL_Renderer* renderer);        // black, core, back motes, sparks   (draw first)
    void renderFront(SDL_Renderer* renderer);   // big soft front motes              (draw after the text)
    void renderFade(SDL_Renderer* renderer);    // intro / outro black overlay       (draw last)

    void spark(float x, float y, int count = 8);   // small burst: selection moved
    void burst(float x, float y);                  // big burst + shock ring: something was chosen
    void beginFadeOut(float duration = 1.1f);

    float time() const { return m_time; }
    const MenuPainter& painter() const { return m_painter; }

private:
    MenuPainter m_painter;
    AmbientParticles m_motes;
    ParticleSystem m_sparks;
    float m_coreX, m_coreY, m_introTime;
    bool  m_overlay = false;
    float m_time = 0.0f;
    float m_fadeTimer = 0.0f, m_fadeDuration = 0.0f;
};
