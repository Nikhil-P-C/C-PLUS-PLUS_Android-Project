//
// 2D lighting: ambient darkness with soft lights cut out of it (player light + level light objects).
//
// How it works (plain SDL_Renderer, no extra shader):
//   1. A half-resolution "light map" texture is cleared to black with alpha = ambient darkness.
//   2. Every light is drawn into it with a blend mode that REMOVES alpha (dst.a *= 1 - src.a),
//      so overlapping lights combine smoothly instead of just stacking up.
//   3. The light map is stretched over the finished frame (black * alpha = darkened scene).
//   4. Coloured lights add a soft additive glow (+ a small hot core) on top, so they still read as light.
// If the renderer refuses the custom blend mode, it falls back to the old single-light shader mask.
//
#pragma once
#include <SDL3/SDL.h>
#include <vector>
#include "level/Lights.h"

class Lighting {
public:
    Lighting() = default;
    ~Lighting();
    Lighting(const Lighting&) = delete;
    Lighting& operator=(const Lighting&) = delete;

    // safe to call every level (textures are built once)
    void init(SDL_Renderer* renderer);
    void setLevel(const LevelAmbient& ambient, const std::vector<LightSource>& lights);

    // Call after the scene is drawn (before HUD / vignette). camX/camY = camera position,
    // playerScreenX/Y = centre of the player in screen pixels.
    void render(SDL_Renderer* renderer, float camX, float camY, float playerScreenX, float playerScreenY);

private:
    void destroy();

    SDL_Texture* m_lightmap = nullptr;   // half-res render target
    SDL_Texture* m_halo     = nullptr;   // soft radial gradient (bright centre, smooth edge)
    SDL_Texture* m_core     = nullptr;   // tight bright dot for the visible light core
    SDL_BlendMode m_cutBlend = SDL_BLENDMODE_NONE;
    bool m_ok = false;                   // custom blend supported -> multi-light path available

    LevelAmbient m_ambient;
    std::vector<LightSource> m_lights;
};
