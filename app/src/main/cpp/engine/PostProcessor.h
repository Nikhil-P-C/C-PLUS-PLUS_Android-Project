//
// Created by LENOVO on 30-08-2026.
//
#pragma once

#include <SDL3/SDL.h>

struct ShaderBytes {
    void* data;
    size_t size;
};
ShaderBytes loadSpirv(const char* path);

class PostProcessor{
public:
    PostProcessor();

    void init(SDL_Renderer* renderer,SDL_GPUDevice* device);

    SDL_Texture* createTarget(SDL_Renderer* r, int w, int h);

    SDL_Texture* blurTexture(SDL_Renderer* r, SDL_Texture* src, float radiusTexels);

    //Layer-scoped bloom

    // Wrap a contiguous run of draw calls that should bloom as one unit —
    // e.g. background + walls + particles + traps + fruits, all drawn
    // before the player. Only genuinely BRIGHT pixels inside the group
    // actually glow (bright-pass threshold): a dark stone wall tile in the
    // same group as a lit torch won't bloom, the torch will. Composited
    // with a screen blend (lightens, never clips to white the way additive
    // does).
    //
    // Anything drawn OUTSIDE a group (player, HUD text, blocks/platforms
    // if you don't wrap them) stays completely untouched — sharp, direct
    // to the backbuffer, zero extra cost.
    //
    //   pp.beginBloomGroup(renderer);
    //       backgroundBuilder.render(renderer);
    //       ...wall tile loop...
    //       particleSystem.render(renderer);
    //       trapBuilder.render(renderer);
    //       fruitBuilder.render(renderer);
    //   pp.endBloomGroup(renderer); // composites onto the window
    //
    //   // player, HUD, blocks draw here — sharp, unaffected
    //
    //   pp.beginBloomGroup(renderer);
    //       foregroundBuilder.render(renderer);
    //   pp.endBloomGroup(renderer);
    //
    // Groups don't nest, and each is its own offscreen capture — every
    // group you add costs its own render-target switches, so keep the
    // number of groups per frame small (2 is the current setup: one for
    // everything before the player, one for foreground).
    void beginBloomGroup(SDL_Renderer* r); // target = m_bloomLayerTex, transparent clear
    void endBloomGroup(SDL_Renderer* r);   // bright-pass -> blur -> draw sharp layer + screen-blend the glow, onto the window

    // --- Player light / vignette ----------------------------------------
    // Darkens the whole frame except a soft circle around a light center
    // (typically the player) — the Hollow-Knight lantern look. Draws
    // straight onto the window, on top of whatever's already there, so
    // call it once everything for the frame is drawn and before any HUD
    // to stay fully bright.
    //
    //   pp.applyPlayerLight(renderer, playerScreenX, playerScreenY,
    //                        /*radiusPx=*/260.0f, /*softnessPx=*/180.0f,
    //                        /*darkness=*/0.85f);
    //
    // centerX/centerY are in screen (post-camera) pixels, same space as
    // your sprite dst rects. radiusPx is how far the fully-lit area
    // reaches; softnessPx is the extra distance the fade takes to reach
    // full darkness. darkness is the max alpha of the dark overlay
    // (1.0 = pitch black at the edges, lower = dim rather than black).
    void applyPlayerLight(SDL_Renderer* r, float centerX, float centerY,
                          float radiusPx, float softnessPx, float darkness);

private:
    SDL_Texture* m_bloomLayerTex = nullptr; // full-res: where a group's draws land
    SDL_Texture* m_brightTex     = nullptr; // bright-pass output, half-res (cheaper blur)
    SDL_Texture* m_pingPongA     = nullptr;
    SDL_Texture* m_pingPongB     = nullptr;
    SDL_Texture* m_whitePixel    = nullptr; // 1x1 texture stretched full-screen, just to get a 0..1 UV quad for the light-mask shader


    SDL_GPURenderState* m_brightPassState = nullptr;
    SDL_GPURenderState* m_blurState       = nullptr;
    SDL_GPURenderState* m_lightMaskState  = nullptr;
};
