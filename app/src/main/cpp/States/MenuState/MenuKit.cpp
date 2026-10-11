//
// MenuKit implementation, see MenuKit.h
//
#include "MenuKit.h"
#include <algorithm>
#include <cmath>
#include <vector>

using namespace MenuKit;

namespace {
    constexpr float COLOR_HOLD  = 8.0f;     // seconds per core colour
    constexpr float COLOR_FADE  = 3.0f;     // cross-fade between colours
    constexpr float BEAT_PERIOD = 1.15f;    // seconds per "lub-dub"

    inline float bump(float t, float c, float s) { float d = (t - c) / s; return std::exp(-0.5f * d * d); }

    // white RGB, alpha = radial falloff (1 in the centre -> 0 at the rim)
    SDL_Texture* makeSoftTexture(SDL_Renderer* renderer, int size, SDL_BlendMode mode, float power) {
        std::vector<Uint32> px(static_cast<size_t>(size) * size);
        const float half = static_cast<float>(size) * 0.5f;
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                const float dx = (static_cast<float>(x) + 0.5f - half) / half;
                const float dy = (static_cast<float>(y) + 0.5f - half) / half;
                const float r = std::sqrt(dx * dx + dy * dy);
                const float v = r >= 1.0f ? 0.0f : std::pow(1.0f - r, power);
                Uint8* p = reinterpret_cast<Uint8*>(&px[static_cast<size_t>(y) * size + x]);   // RGBA32 = bytes R,G,B,A
                p[0] = 255; p[1] = 255; p[2] = 255; p[3] = to8(v);
            }
        }
        SDL_Texture* t = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, size, size);
        if (!t) return nullptr;
        SDL_UpdateTexture(t, nullptr, px.data(), size * static_cast<int>(sizeof(Uint32)));
        SDL_SetTextureBlendMode(t, mode);
        SDL_SetTextureScaleMode(t, SDL_SCALEMODE_LINEAR);
        return t;
    }
}

// =============================================================================================
// MenuTheme
// =============================================================================================
MenuTheme& MenuTheme::get() {
    static MenuTheme instance;
    return instance;
}

void MenuTheme::update(float dt) {
    m_time += dt;
    updatePalette();
    updateHeartbeat(dt);
}

// Everything (text, ornaments, particles) is derived from the current core colour
void MenuTheme::updatePalette() {
    static const Rgb cores[4] = {
            {0.22f, 0.52f, 1.00f},   // blue
            {1.00f, 0.18f, 0.16f},   // red
            {0.20f, 1.00f, 0.42f},   // green
            {0.96f, 0.97f, 1.00f},   // white
    };
    const float seg = COLOR_HOLD + COLOR_FADE;
    const float t = std::fmod(m_time, seg * 4.0f);
    const int idx = static_cast<int>(t / seg) % 4;
    const float local = t - seg * static_cast<float>(idx);
    const float k = local > COLOR_HOLD ? smooth((local - COLOR_HOLD) / COLOR_FADE) : 0.0f;
    m_core = mix(cores[idx], cores[(idx + 1) % 4], k);

    const Rgb white{1, 1, 1};
    m_accent = mix(m_core, white, 0.78f);                        // selected: nearly white, tinted
    const Rgb d = mix(m_core, white, 0.30f);
    m_dim = {d.r * 0.55f, d.g * 0.55f, d.b * 0.55f};             // idle: dim, same hue
    m_mote = mix(m_core, white, 0.45f);
}

// Two soft beats ("lub-dub"). The core shrinks quickly on the beat (the black edges rush in)
// and slowly relaxes back out.
void MenuTheme::updateHeartbeat(float dt) {
    m_beatClock += dt;
    if (m_beatClock >= BEAT_PERIOD) m_beatClock -= BEAT_PERIOD;
    const float t = m_beatClock;
    float env = bump(t, 0.00f, 0.055f) + 0.72f * bump(t, 0.27f, 0.065f);
    env += bump(t - BEAT_PERIOD, 0.00f, 0.055f);                 // wrap-around tail of the next lub
    env = clamp01(env);
    const float rate = env > m_pulse ? 26.0f : 3.2f;
    m_pulse += (env - m_pulse) * std::min(1.0f, rate * dt);
}

// =============================================================================================
// MenuAssets
// =============================================================================================
MenuAssets::~MenuAssets() {
    if (glow)  SDL_DestroyTexture(glow);
    if (shade) SDL_DestroyTexture(shade);
}

std::shared_ptr<MenuAssets> MenuAssets::acquire(SDL_Renderer* renderer) {
    static std::weak_ptr<MenuAssets> cache;
    if (auto existing = cache.lock()) return existing;
    auto assets = std::make_shared<MenuAssets>();
    assets->glow  = makeSoftTexture(renderer, 256, SDL_BLENDMODE_ADD, 2.2f);
    assets->shade = makeSoftTexture(renderer, 256, SDL_BLENDMODE_BLEND, 1.4f);
    cache = assets;
    return assets;
}

// =============================================================================================
// MenuFont / MenuLabel
// =============================================================================================
MenuFont::MenuFont(int defaultSize) : m_size(defaultSize) {
    m_io = SDL_IOFromFile("Fonts/Pixel Game.otf", "rb");
    if (m_io) m_font = TTF_OpenFontIO(m_io, false, static_cast<float>(defaultSize));
    if (!m_font) SDL_Log("menu font failed to load: %s", SDL_GetError());
}

MenuFont::~MenuFont() {
    if (m_font) TTF_CloseFont(m_font);
    if (m_io) SDL_CloseIO(m_io);
}

SDL_Texture* MenuFont::render(SDL_Renderer* renderer, const char* text, int size) {
    if (!m_font || !renderer || !text) return nullptr;
    if (size > 0 && size != m_size) { TTF_SetFontSize(m_font, static_cast<float>(size)); m_size = size; }
    // Blended (anti-aliased) instead of Solid: the text is scaled and tinted every frame
    SDL_Surface* s = TTF_RenderText_Blended(m_font, text, 0, SDL_Color{255, 255, 255, 255});
    if (!s) return nullptr;
    SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
    SDL_DestroySurface(s);
    if (t) {
        SDL_SetTextureScaleMode(t, SDL_SCALEMODE_LINEAR);
        SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
    }
    return t;
}

void MenuLabel::set(SDL_Renderer* renderer, MenuFont& font, const char* text, int fontSize) {
    reset();
    tex = font.render(renderer, text, fontSize);
    if (tex) SDL_GetTextureSize(tex, &w, &h);
}

void MenuLabel::reset() {
    if (tex) SDL_DestroyTexture(tex);
    tex = nullptr;
    w = h = 0.0f;
}

// =============================================================================================
// MenuPainter
// =============================================================================================
MenuPainter::MenuPainter(SDL_Renderer* renderer) : m_renderer(renderer), m_assets(MenuAssets::acquire(renderer)) {}

void MenuPainter::glow(float cx, float cy, float w, float h, Rgb c, float alpha) const {
    if (!m_assets->glow) return;
    SDL_SetTextureColorMod(m_assets->glow, to8(c.r), to8(c.g), to8(c.b));
    SDL_SetTextureAlphaMod(m_assets->glow, to8(alpha));
    SDL_FRect dst{cx - w * 0.5f, cy - h * 0.5f, w, h};
    SDL_RenderTexture(m_renderer, m_assets->glow, nullptr, &dst);
}

void MenuPainter::shade(float cx, float cy, float w, float h, float alpha) const {
    if (!m_assets->shade) return;
    SDL_SetTextureColorMod(m_assets->shade, 0, 0, 0);
    SDL_SetTextureAlphaMod(m_assets->shade, to8(alpha));
    SDL_FRect dst{cx - w * 0.5f, cy - h * 0.5f, w, h};
    SDL_RenderTexture(m_renderer, m_assets->shade, nullptr, &dst);
}

void MenuPainter::fadeLine(float x0, float x1, float y, Rgb c, float a0, float a1, float halfThickness) const {
    const SDL_FColor c0{c.r, c.g, c.b, clamp01(a0)};
    const SDL_FColor c1{c.r, c.g, c.b, clamp01(a1)};
    const SDL_Vertex v[4] = {
            {{x0, y - halfThickness}, c0, {0, 0}}, {{x0, y + halfThickness}, c0, {0, 0}},
            {{x1, y - halfThickness}, c1, {0, 0}}, {{x1, y + halfThickness}, c1, {0, 0}},
    };
    const int idx[6] = {0, 1, 2, 2, 1, 3};
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderGeometry(m_renderer, nullptr, v, 4, idx, 6);
}

void MenuPainter::diamond(float cx, float cy, float size, Rgb c, float a) const {
    const SDL_FColor col{c.r, c.g, c.b, clamp01(a)};
    const SDL_Vertex v[4] = {
            {{cx, cy - size}, col, {0, 0}}, {{cx + size, cy}, col, {0, 0}},
            {{cx, cy + size}, col, {0, 0}}, {{cx - size, cy}, col, {0, 0}},
    };
    const int idx[6] = {0, 1, 2, 0, 2, 3};
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderGeometry(m_renderer, nullptr, v, 4, idx, 6);
}

// hollow diamond (a ring of 4 quads) - the "off" state of radio buttons
void MenuPainter::diamondOutline(float cx, float cy, float size, float thickness, Rgb c, float a) const {
    const SDL_FColor col{c.r, c.g, c.b, clamp01(a)};
    const float in = std::max(0.0f, size - thickness * 1.4142f);
    const float ox[4] = {0, size, 0, -size}, oy[4] = {-size, 0, size, 0};
    const float ix[4] = {0, in, 0, -in},     iy[4] = {-in, 0, in, 0};
    SDL_Vertex v[8];
    for (int i = 0; i < 4; i++) {
        v[i]     = {{cx + ox[i], cy + oy[i]}, col, {0, 0}};
        v[i + 4] = {{cx + ix[i], cy + iy[i]}, col, {0, 0}};
    }
    int idx[24];
    for (int i = 0; i < 4; i++) {
        const int n = (i + 1) % 4;
        idx[i * 6 + 0] = i;     idx[i * 6 + 1] = n;     idx[i * 6 + 2] = i + 4;
        idx[i * 6 + 3] = n;     idx[i * 6 + 4] = n + 4; idx[i * 6 + 5] = i + 4;
    }
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderGeometry(m_renderer, nullptr, v, 8, idx, 24);
}

void MenuPainter::label(const MenuLabel& l, float x, float y, float scale, Rgb c, float alpha, Align align) const {
    if (!l.tex) return;
    const float w = l.w * scale, h = l.h * scale;
    float left = x - w * 0.5f;
    if (align == Align::LEFT) left = x;
    else if (align == Align::RIGHT) left = x - w;
    const SDL_FRect dst{left, y - h * 0.5f, w, h};
    // soft shadow keeps the text readable when the bright core is behind it
    SDL_SetTextureColorMod(l.tex, 0, 0, 0);
    SDL_SetTextureAlphaMod(l.tex, to8(alpha * 0.75f));
    const SDL_FRect sh{dst.x + 2.0f, dst.y + 3.0f, dst.w, dst.h};
    SDL_RenderTexture(m_renderer, l.tex, nullptr, &sh);
    SDL_SetTextureColorMod(l.tex, to8(c.r), to8(c.g), to8(c.b));
    SDL_SetTextureAlphaMod(l.tex, to8(alpha));
    SDL_RenderTexture(m_renderer, l.tex, nullptr, &dst);
}

void MenuPainter::flank(const MenuLabel& l, float cx, float y, float scale, float focus, float reveal) const {
    if (focus <= 0.02f) return;
    const MenuTheme& t = MenuTheme::get();
    const float a = focus * reveal;
    const float half = l.w * scale * 0.5f;
    const float gap = 34.0f;
    const float ext = 60.0f + 70.0f * focus;
    glow(cx, y, l.w * scale + 220.0f, l.h * scale + 90.0f, t.core(), 0.22f * a);
    diamond(cx - half - gap, y, 5.0f, t.accent(), a);
    diamond(cx + half + gap, y, 5.0f, t.accent(), a);
    fadeLine(cx - half - gap - 12.0f - ext, cx - half - gap - 12.0f, y, t.accent(), 0.0f, a * 0.85f);
    fadeLine(cx + half + gap + 12.0f, cx + half + gap + 12.0f + ext, y, t.accent(), a * 0.85f, 0.0f);
}

void MenuPainter::rowFrame(float left, float right, float y, float focus, float reveal) const {
    if (focus <= 0.02f) return;
    const MenuTheme& t = MenuTheme::get();
    const float a = focus * reveal;
    const float ext = 60.0f + 70.0f * focus;
    glow((left + right) * 0.5f, y, (right - left) + 220.0f, 120.0f, t.core(), 0.22f * a);
    diamond(left, y, 5.0f, t.accent(), a);
    diamond(right, y, 5.0f, t.accent(), a);
    fadeLine(left - 12.0f - ext, left - 12.0f, y, t.accent(), 0.0f, a * 0.85f);
    fadeLine(right + 12.0f, right + 12.0f + ext, y, t.accent(), a * 0.85f, 0.0f);
}

// =============================================================================================
// MenuRows
// =============================================================================================
void MenuRows::setup(int count, float firstY, float step, int initialFocus, float revealDelay, float revealStep) {
    m_count = std::min(count, MAX_ROWS);
    m_firstY = firstY; m_step = step;
    m_revealDelay = revealDelay; m_revealStep = revealStep;
    m_time = 0.0f;
    m_focus = std::clamp(initialFocus, 0, std::max(0, m_count - 1));
    for (int i = 0; i < MAX_ROWS; i++) m_anim[i] = (i == m_focus) ? 1.0f : 0.0f;
}

void MenuRows::update(float dt) {
    m_time += dt;
    for (int i = 0; i < m_count; i++)
        m_anim[i] = approach(m_anim[i], i == m_focus ? 1.0f : 0.0f, 12.0f, dt);
}

bool MenuRows::setFocus(int index) {
    if (index < 0 || index >= m_count || index == m_focus) return false;
    m_focus = index;
    return true;
}

bool MenuRows::move(int dir) {
    if (m_count <= 1) return false;
    m_focus = (m_focus + dir + m_count) % m_count;
    return true;
}

float MenuRows::reveal(int i) const {
    return smooth((m_time - m_revealDelay - m_revealStep * static_cast<float>(i)) / 0.45f);
}

int MenuRows::hit(float px, float py, float x0, float x1, float halfH) const {
    if (px < x0 || px > x1) return -1;
    for (int i = 0; i < m_count; i++)
        if (std::fabs(py - rowY(i)) <= halfH) return i;
    return -1;
}

// =============================================================================================
// MenuBackdrop
// =============================================================================================
MenuBackdrop::MenuBackdrop(SDL_Renderer* renderer, float coreX, float coreY, float introTime, bool overlay)
        : m_painter(renderer), m_coreX(coreX), m_coreY(coreY), m_introTime(introTime), m_overlay(overlay) {
    m_motes.init(renderer, AmbientTheme::MENU);
    m_sparks.setScreenSpace(true);
}

void MenuBackdrop::update(float dt) {
    m_time += dt;
    if (m_fadeDuration > 0.0f) m_fadeTimer += dt;

    MenuTheme& theme = MenuTheme::get();
    theme.update(dt);

    const Rgb& m = theme.mote();
    m_motes.setTint(to8(m.r), to8(m.g), to8(m.b), 1.0f);       // motes follow the core colour
    const float ease = (m_overlay && m_introTime > 0.0f) ? smooth(m_time / m_introTime) : 1.0f;
    m_motes.setAlphaScale((0.85f + 0.25f * theme.pulse()) * ease);   // and breathe with the heartbeat
    m_motes.update(dt);
    m_sparks.update(dt);
}

void MenuBackdrop::render(SDL_Renderer* renderer) {
    // 1) black base (overlay: only dim what is already on screen)
    const float ease = (m_overlay && m_introTime > 0.0f) ? smooth(m_time / m_introTime) : 1.0f;
    const SDL_FRect full{0, 0, SCREEN_W, SCREEN_H};
    if (m_overlay) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, to8(0.78f * ease));
    } else {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    }
    SDL_RenderFillRect(renderer, &full);

    // 2) the core: black edges rush toward it on each beat, then drift back
    const MenuTheme& t = MenuTheme::get();
    const float glowMul = (m_overlay ? 0.45f : 1.0f) * ease;
    const float shrink  = 1.0f - 0.20f * t.pulse();
    const float flicker = 0.88f + 0.22f * t.pulse();
    const Rgb hot = mix(t.core(), Rgb{1, 1, 1}, 0.55f);
    m_painter.glow(m_coreX, m_coreY, 2100.0f * shrink, 1250.0f * shrink, t.core(), 0.80f * flicker * glowMul);
    m_painter.glow(m_coreX, m_coreY, 1050.0f * shrink,  640.0f * shrink, t.core(), 0.85f * flicker * glowMul);
    m_painter.glow(m_coreX, m_coreY,  430.0f * shrink,  300.0f * shrink, hot,      0.95f * flicker * glowMul);

    // 3) floating motes (back layer) + selection sparks
    m_motes.renderBack(renderer);
    m_sparks.render(renderer);
}

void MenuBackdrop::renderFront(SDL_Renderer* renderer) {
    m_motes.renderFront(renderer);
}

void MenuBackdrop::renderFade(SDL_Renderer* renderer) {
    float black = (!m_overlay && m_introTime > 0.0f) ? 1.0f - smooth(m_time / m_introTime) : 0.0f;
    if (m_fadeDuration > 0.0f) black = std::max(black, smooth(m_fadeTimer / m_fadeDuration));
    if (black <= 0.001f) return;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, to8(black));
    const SDL_FRect full{0, 0, SCREEN_W, SCREEN_H};
    SDL_RenderFillRect(renderer, &full);
}

void MenuBackdrop::spark(float x, float y, int count) {
    const Rgb& a = MenuTheme::get().accent();
    m_sparks.emitBurst(x, y, count, 40.0f, 140.0f, 0.35f, 0.80f, 12.0f, to8(a.r), to8(a.g), to8(a.b));
}

void MenuBackdrop::burst(float x, float y) {
    const Rgb& a = MenuTheme::get().accent();
    m_sparks.emitBurst(x, y, 22, 60.0f, 220.0f, 0.45f, 1.00f, 14.0f, to8(a.r), to8(a.g), to8(a.b));
    m_sparks.emitRing(x, y, 260.0f, 0.55f, to8(a.r), to8(a.g), to8(a.b));
}

void MenuBackdrop::beginFadeOut(float duration) {
    m_fadeDuration = duration;
    m_fadeTimer = 0.0f;
}
