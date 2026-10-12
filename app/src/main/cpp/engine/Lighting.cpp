#include "Lighting.h"
#include "engine/Engine.h"
#include <algorithm>
#include <cmath>

namespace {
    constexpr float SCREEN_W = 1600.0f, SCREEN_H = 720.0f;   // fixed logical presentation size
    constexpr float MAP_SCALE = 0.5f;                        // light map resolution relative to the screen
    constexpr float GLOW_STRENGTH = 0.40f;

    float clamp01(float v) { return std::clamp(v, 0.0f, 1.0f); }
    float smooth(float e0, float e1, float x) {
        float t = clamp01((x - e0) / (e1 - e0));
        return t * t * (3.0f - 2.0f * t);
    }

    // Torch wobble in [1 - flicker, 1]; phase comes from the position so neighbouring lights don't pulse together.
    float flickerFactor(const LightSource& l, float t) {
        if (l.flicker <= 0.0f) return 1.0f;
        const float phase = l.x * 0.013f + l.y * 0.007f;
        const float s = 0.55f * std::sin(t * 11.0f + phase) + 0.45f * std::sin(t * 23.7f + phase * 1.7f);   // -1..1
        return 1.0f - std::min(l.flicker, 1.0f) * (0.5f + 0.5f * s);
    }

    // white texture whose alpha is a radial profile. falloff(d) with d = 0 (centre) .. 1 (edge)
    template <class F>
    SDL_Texture* makeRadialSprite(SDL_Renderer* r, int size, F falloff) {
        SDL_Surface* surf = SDL_CreateSurface(size, size, SDL_PIXELFORMAT_RGBA32);
        if (!surf) return nullptr;
        auto* px = static_cast<Uint8*>(surf->pixels);
        const float c = (size - 1) * 0.5f;
        for (int y = 0; y < size; ++y) {
            for (int x = 0; x < size; ++x) {
                const float dx = (x - c) / c, dy = (y - c) / c;
                const float d = std::sqrt(dx * dx + dy * dy);
                Uint8* p = px + y * surf->pitch + x * 4;
                p[0] = p[1] = p[2] = 255;
                p[3] = static_cast<Uint8>(clamp01(d >= 1.0f ? 0.0f : falloff(d)) * 255.0f);
            }
        }
        SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
        SDL_DestroySurface(surf);
        if (tex) SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_LINEAR);
        return tex;
    }
}

Lighting::~Lighting() { destroy(); }

void Lighting::destroy() {
    if (m_lightmap) { SDL_DestroyTexture(m_lightmap); m_lightmap = nullptr; }
    if (m_halo)     { SDL_DestroyTexture(m_halo);     m_halo = nullptr; }
    if (m_core)     { SDL_DestroyTexture(m_core);     m_core = nullptr; }
    m_ok = false;
}

void Lighting::init(SDL_Renderer* r) {
    if (m_ok || !r) return;
    destroy();

    // full strength in the middle, smooth fade to nothing at the edge (matches the old player-light feel)
    m_halo = makeRadialSprite(r, 256, [](float d) { return 1.0f - smooth(0.30f, 1.0f, d); });
    m_core = makeRadialSprite(r, 64,  [](float d) { return std::exp(-(d * d) / (2.0f * 0.28f * 0.28f)); });

    m_lightmap = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET,
                                   (int)(SCREEN_W * MAP_SCALE), (int)(SCREEN_H * MAP_SCALE));
    if (m_lightmap) {
        SDL_SetTextureScaleMode(m_lightmap, SDL_SCALEMODE_LINEAR);
        SDL_SetTextureBlendMode(m_lightmap, SDL_BLENDMODE_BLEND);
    }

    // dst.rgb unchanged (the map is black), dst.a = dst.a * (1 - src.a)  ->  a light "cuts" the darkness
    m_cutBlend = SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_ZERO, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_ADD,
                                            SDL_BLENDFACTOR_ZERO, SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, SDL_BLENDOPERATION_ADD);
    m_ok = m_halo && m_core && m_lightmap && SDL_SetTextureBlendMode(m_halo, m_cutBlend);
    if (!m_ok) SDL_Log("Lighting: custom blend / light map unavailable, using single-light fallback");
    if (m_ok) SDL_SetTextureBlendMode(m_halo, SDL_BLENDMODE_BLEND);   // reset; the mode is chosen per pass
}

void Lighting::setLevel(const LevelAmbient& ambient, const std::vector<LightSource>& lights) {
    m_ambient = ambient;
    m_lights  = lights;
}

void Lighting::render(SDL_Renderer* r, float camX, float camY, float playerX, float playerY) {
    if (!r) return;
    const float t = SDL_GetTicks() * 0.001f;
    const float dark = clamp01(m_ambient.darkness);
    const float playerOuter = std::max(0.0f, m_ambient.playerRadius + m_ambient.playerSoftness);

    // ---- fallback: old single shader mask around the player only ----
    if (!m_ok) {
        if (playerOuter > 0.0f && dark > 0.001f)
            Engine::Get().getPostProcessor().applyPlayerLight(r, playerX, playerY,
                                                              m_ambient.playerRadius, m_ambient.playerSoftness, dark);
        return;
    }

    // lights that can be seen this frame (screen space)
    struct Visible { const LightSource* l; float sx, sy, flick; };
    std::vector<Visible> vis;
    vis.reserve(m_lights.size());
    for (const auto& l : m_lights) {
        const float sx = l.x - camX, sy = l.y - camY;
        if (sx < -l.radius || sx > SCREEN_W + l.radius || sy < -l.radius || sy > SCREEN_H + l.radius) continue;
        vis.push_back({&l, sx, sy, flickerFactor(l, t)});
    }

    // ---- 1+2: build the light map (darkness with the lights cut out) ----
    if (dark > 0.001f) {
        SDL_Texture* prevTarget = SDL_GetRenderTarget(r);
        SDL_SetRenderTarget(r, m_lightmap);
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
        SDL_SetRenderDrawColor(r, 0, 0, 0, (Uint8)(dark * 255.0f + 0.5f));
        SDL_RenderClear(r);

        SDL_SetTextureBlendMode(m_halo, m_cutBlend);
        SDL_SetTextureColorMod(m_halo, 255, 255, 255);

        auto cut = [&](float sx, float sy, float radius, float strength) {
            SDL_SetTextureAlphaMod(m_halo, (Uint8)(clamp01(strength) * 255.0f + 0.5f));
            const float d = radius * 2.0f * MAP_SCALE;
            SDL_FRect dst{sx * MAP_SCALE - d * 0.5f, sy * MAP_SCALE - d * 0.5f, d, d};
            SDL_RenderTexture(r, m_halo, nullptr, &dst);
        };
        if (playerOuter > 0.0f) cut(playerX, playerY, playerOuter, 1.0f);
        for (const auto& v : vis) cut(v.sx, v.sy, v.l->radius, v.l->intensity * v.flick);

        SDL_SetTextureAlphaMod(m_halo, 255);
        SDL_SetRenderTarget(r, prevTarget);

        // ---- 3: darken the finished frame ----
        SDL_FRect full{0.0f, 0.0f, SCREEN_W, SCREEN_H};
        SDL_RenderTexture(r, m_lightmap, nullptr, &full);
    }

    // ---- 4: coloured glow + visible core, added on top so lights still look lit inside the dark ----
    if (!vis.empty()) {
        SDL_SetTextureBlendMode(m_halo, SDL_BLENDMODE_ADD);
        SDL_SetTextureBlendMode(m_core, SDL_BLENDMODE_ADD);
        for (const auto& v : vis) {
            const LightSource& l = *v.l;
            const float glowA = clamp01(GLOW_STRENGTH * l.intensity * v.flick);
            SDL_SetTextureColorMod(m_halo, l.r, l.g, l.b);
            SDL_SetTextureAlphaMod(m_halo, (Uint8)(glowA * 255.0f + 0.5f));
            const float gd = l.radius * 1.6f;
            SDL_FRect gdst{v.sx - gd * 0.5f, v.sy - gd * 0.5f, gd, gd};
            SDL_RenderTexture(r, m_halo, nullptr, &gdst);

            if (l.visible) {
                // hot core: the light colour pushed towards white
                const Uint8 cr = (Uint8)(l.r + (255 - l.r) * 0.6f);
                const Uint8 cg = (Uint8)(l.g + (255 - l.g) * 0.6f);
                const Uint8 cb = (Uint8)(l.b + (255 - l.b) * 0.6f);
                SDL_SetTextureColorMod(m_core, cr, cg, cb);
                SDL_SetTextureAlphaMod(m_core, (Uint8)(clamp01(v.flick) * 255.0f));
                const float cd = std::clamp(l.radius * 0.14f, 18.0f, 44.0f);
                SDL_FRect cdst{v.sx - cd * 0.5f, v.sy - cd * 0.5f, cd, cd};
                SDL_RenderTexture(r, m_core, nullptr, &cdst);
            }
        }
        SDL_SetTextureColorMod(m_halo, 255, 255, 255);
        SDL_SetTextureAlphaMod(m_halo, 255);
        SDL_SetTextureColorMod(m_core, 255, 255, 255);
        SDL_SetTextureAlphaMod(m_core, 255);
    }
}
