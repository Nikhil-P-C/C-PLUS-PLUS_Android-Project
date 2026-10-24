//
// Biome-driven ambience + weather, layered on top of AmbientParticles (the glowing motes):
//
//   FIREFLIES  blinking, wandering glow points            (night-ish forest/meadow, blue in bright caves)
//   FLIES      tiny buzzing insects with a wing blur       (daytime forest/meadow, sparse gnats in caves)
//   PETALS     petals that flutter down and sway in the air (meadows)
//   LEAVES     tumbling leaves                              (forest)
//   WIND       gusts: faint streaks + everything airborne leans/lifts with the wind
//   RAIN       slanted streaks, the slant follows the wind
//   SNOW       flakes that sway, bokeh flakes in front
//   THUNDER    lightning bolt behind the terrain + screen flash + delayed rumble (camera trauma hook)
//   SUNFLARE   screen-space sun with lens-flare ghosts along the axis (bright days only)
//
// Every effect has a BACK layer (small, slow, dim, drawn inside the bloom group behind the terrain) and a
// FRONT layer (bigger, faster, softer, drawn over everything), same depth idea as AmbientParticles.
// Particles live in a wrapping parallax domain, so there is no spawn logic: whatever leaves one edge
// re-enters the opposite one while the camera scrolls.
//
// The default set comes from the biome (+ the level's darkness, "D" line). A level can override it with a
// 'Z' line, see Levels/readme.txt:
//     Z SNOW WIND            only snow + wind
//     Z AUTO RAIN            biome defaults + rain
//     Z AUTO -SUNFLARE       biome defaults without the flare
//     Z STORM                rain + thunder + strong wind
//     Z NONE                 nothing
//
// Cost: everything is batched per texture into SDL_RenderGeometry calls (a handful of draws per layer),
// no per-particle ColorMod/AlphaMod state changes. Use setDensity() to thin it out on slow devices.
//
#pragma once
#include <SDL3/SDL.h>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>
#include "level/BackGroundBuilder.h"

enum AmbienceFlag : uint32_t {
    AMB_NONE      = 0,
    AMB_FIREFLIES = 1u << 0,
    AMB_FLIES     = 1u << 1,
    AMB_PETALS    = 1u << 2,
    AMB_LEAVES    = 1u << 3,
    AMB_WIND      = 1u << 4,
    AMB_RAIN      = 1u << 5,
    AMB_SNOW      = 1u << 6,
    AMB_THUNDER   = 1u << 7,
    AMB_SUNFLARE  = 1u << 8,
    AMB_STORM     = AMB_RAIN | AMB_THUNDER | AMB_WIND,   // token only (Z STORM)
};

enum class AmbienceBiome {
    FOREST = 0,     // greenery
    MEADOWS,
    STONE_CAVE,
    BRIGHT_CAVE,
};

class AmbienceFx {
public:
    AmbienceFx() = default;
    ~AmbienceFx();
    AmbienceFx(const AmbienceFx&) = delete;
    AmbienceFx& operator=(const AmbienceFx&) = delete;

    // Call from GameState::setLevel. darkness = the level's ambient darkness (0..1),
    // tokens = the level's 'Z' line (may be empty -> biome defaults).
    void init(SDL_Renderer* renderer, AmbienceBiome biome, float darkness, const std::vector<std::string>& tokens);
    void update(float dt);

    // renderBack / renderFront: inside the bloom group (next to AmbientParticles::renderBack/Front).
    // renderOverlay: after Lighting::render, before the HUD (lens flare, lightning flash, rain tint).
    void renderBack(SDL_Renderer* renderer);
    void renderFront(SDL_Renderer* renderer);
    void renderOverlay(SDL_Renderer* renderer);

    // true once per thunder clap (it follows the lightning flash after a short delay).
    // strength 0.5..1. GameState uses it for camera rumble; hook a sound here when you have one.
    bool consumeThunder(float& strength);

    // 0.25..1.5, multiplies every particle count (default 1). Lower it on slow devices.
    void setDensity(float density) { m_density = density < 0.25f ? 0.25f : (density > 1.5f ? 1.5f : density); }

    uint32_t effects() const { return m_mask; }
    float wind() const { return m_windVel; }   // px/s, + = blowing right (other systems may read it)

    static AmbienceBiome biomeFromBackground(const std::vector<BackGroundElement>& elements);
    static uint32_t defaultsFor(AmbienceBiome biome, float darkness);
    static uint32_t resolve(AmbienceBiome biome, float darkness, const std::vector<std::string>& tokens);

private:
    struct P {
        float x = 0, y = 0;            // domain position (wrap space); for wanderers the "home" point
        float parallax = 1.0f, size = 10.0f, alpha = 1.0f;
        float fall = 0.0f;             // px/s downward (negative = rises)
        float vx = 0.0f, vy = 0.0f;    // current velocity (wind lag / fly steering)
        float drift = 0.0f;            // constant sideways px/s
        float swayAmp = 0.0f, swayFreq = 1.0f, phase = 0.0f;
        float rot = 0.0f, spin = 0.0f, flipSpeed = 1.0f;
        float ox = 0.0f, oy = 0.0f;    // wander offset from home (flies)
        float tvx = 0.0f, tvy = 0.0f, timer = 0.0f;
        float ax = 0.0f, ay = 0.0f, fx = 1.0f, fy = 1.0f;   // lissajous wander (fireflies)
        float period = 2.0f;           // blink period (fireflies)
        float len = 0.0f;              // streak length
        float influence = 1.0f;        // how much wind moves it
        uint8_t r = 255, g = 255, b = 255;
    };
    struct Group { std::vector<P> back, front; };
    struct View { float w, h, camX, camY; };

    View getView() const;
    int  count(int base) const;
    void destroySprites();
    void buildAll();
    void buildFireflies();
    void buildFlies();
    void buildDrifters(Group& g, bool leaf);
    void buildSnow();
    void buildRain();
    void buildStreaks();
    void updateWind(float dt);
    void updateThunder(float dt);
    void makeBolt();
    void renderLayer(SDL_Renderer* renderer, bool front);
    // screen position + current brightness of a firefly (shared by the layer pass and the overlay glow)
    void fireflyScreen(const P& p, bool front, const View& v, float& sx, float& sy, float& a) const;
    void renderLensFlare(SDL_Renderer* renderer, const View& v);

    uint32_t m_mask = 0;
    AmbienceBiome m_biome = AmbienceBiome::FOREST;
    float m_darkness = 0.2f;
    float m_density = 1.0f;
    float m_t = 0.0f;
    bool  m_ready = false;

    Group m_fireflies, m_flies, m_petals, m_leaves, m_snow, m_rain, m_streaks;

    // sprites (procedural, white; colour comes from vertex colours)
    SDL_Texture* m_glow  = nullptr;   // hot core + halo
    SDL_Texture* m_soft  = nullptr;   // smooth disc (bokeh)
    SDL_Texture* m_dot   = nullptr;   // small crisp dot
    SDL_Texture* m_petal = nullptr;
    SDL_Texture* m_leaf  = nullptr;
    SDL_Texture* m_ring  = nullptr;   // lens flare ghost
    SDL_Texture* m_hex   = nullptr;   // lens flare ghost

    // wind
    float m_windVel = 0.0f;           // px/s
    float m_gust = 0.0f;              // 0..1 envelope of the current gust
    float m_gustDir = 1.0f, m_gustPeak = 0.0f, m_gustAge = 0.0f, m_gustDur = 0.0f, m_nextGust = 4.0f;
    float m_windScale = 1.0f;         // storms blow harder

    // thunder
    float m_nextStrike = 6.0f;
    float m_flashT = 99.0f;           // seconds since the last strike started
    float m_thunderDelay = -1.0f;
    float m_thunderStrength = 0.0f;
    bool  m_thunderReady = false;
    float m_boltParallaxX = 0.0f;     // bolt anchor in layer space
    std::vector<SDL_FPoint> m_bolt;
    std::vector<std::vector<SDL_FPoint>> m_branches;
};
