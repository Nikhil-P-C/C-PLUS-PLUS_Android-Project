//
// Biome-driven ambience + weather, layered on top of AmbientParticles (the glowing motes):
//
//   FIREFLIES  blinking, wandering glow points            (dusk/night forest + meadow, blue in bright caves)
//              (the old FLIES / gnats are gone: "Z FLIES" is accepted but now means FIREFLIES)
//   PETALS     petals that flutter down and sway in the air (meadows)
//   LEAVES     tumbling leaves                              (forest)
//   WIND       gusts: faint streaks + everything airborne leans/lifts with the wind
//   RAIN       streaks that fall at an angle: the slant follows the wind (+ an optional fixed ANGLE)
//   SNOW       soft volumetric flakes (tiny dots, fluffy flakes, big out-of-focus bokeh) blown by the wind
//   THUNDER    lightning bolt behind the terrain + screen flash + delayed thunder clap
//   SUNFLARE   a sun that travels across the sky as the player progresses through the level, god rays
//              behind the terrain and a lens flare (ghosts, halo, starburst, streak, lens dirt) in front
//
// Every effect has a BACK layer (small, slow, dim, drawn inside the bloom group behind the terrain) and a
// FRONT layer (bigger, faster, softer, drawn over everything), same depth idea as AmbientParticles.
// Particles live in a wrapping parallax domain, so there is no spawn logic: whatever leaves one edge
// re-enters the opposite one while the camera scrolls.
//
// The default set comes from the biome (+ the level's darkness, "D" line). A level overrides it with a
// 'Z' line (see Levels/readme.txt for every option):
//     Z SNOW WIND                        only snow + wind
//     Z AUTO RAIN                        biome defaults + rain
//     Z AUTO -SUNFLARE                   biome defaults without the flare
//     Z STORM                            rain + thunder + strong wind
//     Z RAIN=1.5 WIND=-2 ANGLE=-25       heavy rain, strong wind blowing LEFT, extra slant
//     Z SNOW=1.4 WIND=3 GUST=2 FOG=0.4   blizzard
//     Z NONE                             nothing
//
// Sound: this class only DECIDES (soundLevels(), consumeThunder(), consumeGust()); GameState
// plays the actual clips through AssetManager.
//
// Cost: everything is batched per texture into SDL_RenderGeometry calls (a handful of draws per layer),
// no per-particle ColorMod/AlphaMod state changes. Use setDensity() / DENSITY= to thin it out.
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
    // bit 1 used to be AMB_FLIES (removed on purpose, only fireflies now)
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

// What the ground sounds like under the feet (STEPS= option, AUTO picks it from the weather + biome)
enum class StepSurface {
    AUTO = 0,
    CONCRETE,
    GRASS,
    LEAVES,
    SNOW,
};

class AmbienceFx {
public:
    // Everything a level can tune from its 'Z' line (KEY=VALUE tokens). Defaults = the old look.
    struct Options {
        float rain      = 1.0f;   // RAIN=      density / opacity            0.2 .. 3
        float snow      = 1.0f;   // SNOW=      density                      0.2 .. 3
        float snowSize  = 1.0f;   // SNOWSIZE=  flake size                   0.5 .. 2.5
        float wind      = 1.0f;   // WIND=      strength (sign = prevailing direction, - = blows left)
        float windDir   = 0.0f;   //            -1 left, +1 right, 0 = gusts pick a side
        float gust      = 1.0f;   // GUST=      gust strength + frequency    0 .. 3
        float angle     = 0.0f;   // ANGLE=     constant rain/snow slant in degrees (+ = leans right), on top of the wind
        float fog       = -1.0f;  // FOG=       haze 0..1 (-1 = automatic: a little in snow / rain)
        float thunder   = 1.0f;   // THUNDER=   lightning frequency          0.2 .. 5
        float fireflies = 1.0f;   // FIREFLIES= density                      0 .. 3
        float leaves    = 1.0f;   // LEAVES=    density
        float petals    = 1.0f;   // PETALS=    density
        float flare     = 1.0f;   // SUNFLARE= / FLARE=  strength            0 .. 2
        float density   = 1.0f;   // DENSITY=   scales every particle count  0.25 .. 1.5
        float sound     = 1.0f;   // SOUND=     scales the ambience sounds   0 .. 2 (0 = silent level)
        // SUNPATH=x0,y0,x1,y1  where the sun is on screen at the START and at the END of the level
        // (fractions of the screen: 0.9,0.16 = top right). It slides between them as the camera travels.
        float sunPath[4] = {0.92f, 0.17f, 0.30f, 0.08f};
        uint8_t sunRGB[3]  = {255, 222, 165};    // SUNCOLOR=r,g,b
        bool    hasGlow    = false;
        uint8_t glowRGB[3] = {200, 255, 90};     // GLOW=r,g,b  firefly colour
        StepSurface steps = StepSurface::AUTO;   // STEPS=GRASS|LEAVES|SNOW|CONCRETE|AUTO
    };

    struct SoundLevels {            // 0..1 loop volumes, already scaled by SOUND=
        float rain = 0.0f;
        float wind = 0.0f;
    };

    AmbienceFx() = default;
    ~AmbienceFx();
    AmbienceFx(const AmbienceFx&) = delete;
    AmbienceFx& operator=(const AmbienceFx&) = delete;

    // Call from GameState::setLevel. darkness = the level's ambient darkness (0..1),
    // tokens = the level's 'Z' line (may be empty -> biome defaults), world = the level's wall-collision
    // rect (the sun travels across it; nullptr = the sun stays at the middle of its path).
    void init(SDL_Renderer* renderer, AmbienceBiome biome, float darkness, const std::vector<std::string>& tokens,
              const SDL_FRect* world = nullptr);
    void update(float dt);

    // renderBack / renderFront: inside the bloom group (next to AmbientParticles::renderBack/Front).
    // renderOverlay: after Lighting::render, before the HUD (lens flare, lightning flash, haze, rain tint).
    void renderBack(SDL_Renderer* renderer);
    void renderFront(SDL_Renderer* renderer);
    void renderOverlay(SDL_Renderer* renderer);

    // ---- sound hooks (all cheap, call every frame) ----
    // true once per thunder clap (it follows the lightning flash after a short delay).
    // strength 0.5..1, pan -1..1 = where the bolt was on screen.
    bool consumeThunder(float& strength);
    bool consumeThunder(float& strength, float& pan);
    // true once at the start of every wind gust. strength 0.3..1, dir -1 / +1 = blowing left / right.
    bool consumeGust(float& strength, float& dir);
    // target volumes of the looping beds (rain / wind)
    SoundLevels soundLevels() const;
    float soundScale() const { return m_opt.sound; }
    // footstep surface for this level (resolved: never AUTO)
    StepSurface stepSurface() const;

    // 0.25..1.5, multiplies every particle count (default 1). Lower it on slow devices.
    void setDensity(float density) { m_density = density < 0.25f ? 0.25f : (density > 1.5f ? 1.5f : density); }

    uint32_t effects() const { return m_mask; }
    float wind() const { return m_windVel; }   // px/s, + = blowing right (other systems may read it)
    const Options& options() const { return m_opt; }

    static AmbienceBiome biomeFromBackground(const std::vector<BackGroundElement>& elements);
    static uint32_t defaultsFor(AmbienceBiome biome, float darkness);
    static uint32_t resolve(AmbienceBiome biome, float darkness, const std::vector<std::string>& tokens);

private:
    struct P {
        float x = 0, y = 0;            // domain position (wrap space); for wanderers the "home" point
        float parallax = 1.0f, size = 10.0f, alpha = 1.0f;
        float fall = 0.0f;             // px/s downward (negative = rises)
        float vx = 0.0f, vy = 0.0f;    // current velocity (wind lag)
        float drift = 0.0f;            // constant sideways px/s
        float swayAmp = 0.0f, swayFreq = 1.0f, phase = 0.0f;
        float rot = 0.0f, spin = 0.0f, flipSpeed = 1.0f;   // flipSpeed: fireflies use it as "double flash" flag
        float ax = 0.0f, ay = 0.0f, fx = 1.0f, fy = 1.0f;  // lissajous wander (fireflies)
        float period = 2.0f;           // blink period (fireflies)
        float len = 0.0f;              // streak length
        float influence = 1.0f;        // how much wind moves it
        uint8_t kind = 0;              // snow: 0 soft dot, 1 fluffy flake, 2 bokeh
        uint8_t r = 255, g = 255, b = 255;
    };
    struct Group { std::vector<P> back, front; };
    struct View { float w, h, camX, camY; };

    View getView() const;
    int  count(int base) const;
    void parseOptions(const std::vector<std::string>& tokens);
    void destroySprites();
    void buildAll();
    void buildFireflies();
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
    // sun on screen: false when it is not worth drawing. vis 0..~1.5 (fades off-screen), center 1 = sun in the middle of the screen
    bool sunState(const View& v, float& sx, float& sy, float& vis, float& center) const;
    void renderSunBack(SDL_Renderer* renderer, const View& v);
    void renderLensFlare(SDL_Renderer* renderer, const View& v);
    float fogAmount() const;

    uint32_t m_mask = 0;
    Options m_opt;
    AmbienceBiome m_biome = AmbienceBiome::FOREST;
    float m_darkness = 0.2f;
    float m_density = 1.0f;
    float m_t = 0.0f;
    bool  m_ready = false;
    float m_fireNight = 0.0f;         // 0..1 how dark enough it is for fireflies to shine (caves = 1)
    float m_worldX = 0.0f, m_worldY = 0.0f, m_worldW = 0.0f;   // level bounds, for the sun's travel

    Group m_fireflies, m_petals, m_leaves, m_snow, m_rain, m_streaks;

    // sprites (procedural, white; colour comes from vertex colours)
    SDL_Texture* m_glow  = nullptr;   // hot core + halo
    SDL_Texture* m_soft  = nullptr;   // smooth disc
    SDL_Texture* m_dot   = nullptr;   // small crisp dot
    SDL_Texture* m_pt    = nullptr;   // soft gaussian point (small snow)
    SDL_Texture* m_flake = nullptr;   // fluffy cluster (snow with volume)
    SDL_Texture* m_bokeh = nullptr;   // out-of-focus disc with a brighter rim
    SDL_Texture* m_petal = nullptr;
    SDL_Texture* m_leaf  = nullptr;
    SDL_Texture* m_ring  = nullptr;   // lens flare ghost
    SDL_Texture* m_hex   = nullptr;   // lens flare ghost
    SDL_Texture* m_star  = nullptr;   // lens flare starburst
    SDL_Texture* m_spec  = nullptr;   // lens flare rainbow halo ring

    // wind
    float m_windVel = 0.0f;           // px/s
    float m_gust = 0.0f;              // 0..1 envelope of the current gust
    float m_gustDir = 1.0f, m_gustPeak = 0.0f, m_gustAge = 0.0f, m_gustDur = 0.0f, m_nextGust = 4.0f;
    float m_windScale = 1.0f;         // storms blow harder
    bool  m_gustReady = false;
    float m_gustReadyStrength = 0.0f, m_gustReadyDir = 1.0f;

    // thunder
    float m_nextStrike = 6.0f;
    float m_flashT = 99.0f;           // seconds since the last strike started
    float m_thunderDelay = -1.0f;
    float m_thunderStrength = 0.0f;
    float m_thunderPan = 0.0f;
    bool  m_thunderReady = false;
    float m_boltParallaxX = 0.0f;     // bolt anchor in layer space
    std::vector<SDL_FPoint> m_bolt;
    std::vector<std::vector<SDL_FPoint>> m_branches;


    // lens dirt: fixed blobs on the "lens" that light up when the sun is near them
    struct Dirt { float x, y, r, a; };   // x,y in 0..1 of the screen
    Dirt m_dirt[8] = {};
};