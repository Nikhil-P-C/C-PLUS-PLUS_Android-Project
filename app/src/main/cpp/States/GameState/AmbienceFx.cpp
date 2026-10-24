#include "AmbienceFx.h"
#include "utils/Camera.h"
#include <random>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <cctype>

namespace {
    std::mt19937 g_rng(std::random_device{}());

    float rnd(float a, float b) {
        std::uniform_real_distribution<float> d(a, b);
        return d(g_rng);
    }
    constexpr float PI = 3.14159265f;
    constexpr float TWO_PI = 6.2831853f;

    float lerp(float a, float b, float t) { return a + (b - a) * t; }
    float clamp01(float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }
    float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
    float wrap(float v, float size) { return v - std::floor(v / size) * size; }
    float smoothstep(float e0, float e1, float x) {
        float t = clamp01((x - e0) / (e1 - e0));
        return t * t * (3.0f - 2.0f * t);
    }
    float fract(float v) { return v - std::floor(v); }
    float hash1(float x) { return fract(std::sin(x * 12.9898f) * 43758.5453f); }
    SDL_FColor col(uint8_t r, uint8_t g, uint8_t b, float a) {
        return SDL_FColor{r / 255.0f, g / 255.0f, b / 255.0f, clamp01(a)};
    }
    SDL_FColor colf(float r, float g, float b, float a) {
        return SDL_FColor{clamp01(r), clamp01(g), clamp01(b), clamp01(a)};
    }

    // wrap margins (bigger in front because the sprites and rain streaks are bigger)
    constexpr float BACK_MARGIN  = 60.0f;
    constexpr float FRONT_MARGIN = 140.0f;

    // ---- batched geometry: one SDL_RenderGeometry call per texture instead of one draw per particle ----
    struct Batch {
        std::vector<SDL_Vertex> v;
        std::vector<int> idx;

        void clear() { v.clear(); idx.clear(); }

        void tri6(int b) {
            idx.push_back(b);     idx.push_back(b + 1); idx.push_back(b + 2);
            idx.push_back(b);     idx.push_back(b + 2); idx.push_back(b + 3);
        }
        // rotated textured quad, hw/hh = half size
        void quad(float cx, float cy, float hw, float hh, float rot, SDL_FColor c) {
            const float cs = std::cos(rot), sn = std::sin(rot);
            const float px[4] = {-hw, hw, hw, -hw};
            const float py[4] = {-hh, -hh, hh, hh};
            const float us[4] = {0, 1, 1, 0};
            const float vs[4] = {0, 0, 1, 1};
            int b = (int)v.size();
            for (int i = 0; i < 4; ++i) {
                SDL_Vertex vt;
                vt.position = SDL_FPoint{cx + px[i] * cs - py[i] * sn, cy + px[i] * sn + py[i] * cs};
                vt.color = c;
                vt.tex_coord = SDL_FPoint{us[i], vs[i]};
                v.push_back(vt);
            }
            tri6(b);
        }
        // untextured line with a colour at each end (alpha fades along it)
        void line(float x0, float y0, float x1, float y1, float w, SDL_FColor c0, SDL_FColor c1) {
            const float dx = x1 - x0, dy = y1 - y0;
            const float l = std::sqrt(dx * dx + dy * dy);
            if (l < 0.001f) return;
            const float nx = -dy / l * w * 0.5f, ny = dx / l * w * 0.5f;
            int b = (int)v.size();
            const SDL_FPoint uv{0, 0};
            v.push_back(SDL_Vertex{SDL_FPoint{x0 + nx, y0 + ny}, c0, uv});
            v.push_back(SDL_Vertex{SDL_FPoint{x0 - nx, y0 - ny}, c0, uv});
            v.push_back(SDL_Vertex{SDL_FPoint{x1 - nx, y1 - ny}, c1, uv});
            v.push_back(SDL_Vertex{SDL_FPoint{x1 + nx, y1 + ny}, c1, uv});
            tri6(b);
        }
        // untextured triangle, one colour per corner (god rays: bright apex, transparent far corners)
        void tri(float x0, float y0, SDL_FColor c0, float x1, float y1, SDL_FColor c1, float x2, float y2, SDL_FColor c2) {
            const SDL_FPoint uv{0, 0};
            int b = (int)v.size();
            v.push_back(SDL_Vertex{SDL_FPoint{x0, y0}, c0, uv});
            v.push_back(SDL_Vertex{SDL_FPoint{x1, y1}, c1, uv});
            v.push_back(SDL_Vertex{SDL_FPoint{x2, y2}, c2, uv});
            idx.push_back(b); idx.push_back(b + 1); idx.push_back(b + 2);
        }
        // untextured rectangle with a vertical colour gradient (haze)
        void rectGrad(float x, float y, float w, float h, SDL_FColor top, SDL_FColor bottom) {
            const SDL_FPoint uv{0, 0};
            int b = (int)v.size();
            v.push_back(SDL_Vertex{SDL_FPoint{x, y}, top, uv});
            v.push_back(SDL_Vertex{SDL_FPoint{x + w, y}, top, uv});
            v.push_back(SDL_Vertex{SDL_FPoint{x + w, y + h}, bottom, uv});
            v.push_back(SDL_Vertex{SDL_FPoint{x, y + h}, bottom, uv});
            tri6(b);
        }
        void flush(SDL_Renderer* r, SDL_Texture* tex) {
            if (!v.empty()) {
                SDL_RenderGeometry(r, tex, v.data(), (int)v.size(), idx.data(), (int)idx.size());
            }
            clear();
        }
    };
    Batch g_batch;

    // ---- procedural sprites: fn(x, y in -1..1, rgba out) ----
    template <class F>
    SDL_Texture* makeSprite(SDL_Renderer* renderer, int size, F fn) {
        SDL_Surface* surf = SDL_CreateSurface(size, size, SDL_PIXELFORMAT_RGBA32);
        if (!surf) return nullptr;
        auto* px = static_cast<Uint8*>(surf->pixels);
        const float c = (size - 1) * 0.5f;
        for (int y = 0; y < size; ++y) {
            for (int x = 0; x < size; ++x) {
                Uint8* p = px + y * surf->pitch + x * 4;
                fn((x - c) / c, (y - c) / c, p);
            }
        }
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_DestroySurface(surf);
        if (tex) {
            SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
            SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_LINEAR);
        }
        return tex;
    }
    void put(Uint8* p, float shade, float a) {
        Uint8 s = (Uint8)(clamp01(shade) * 255.0f);
        p[0] = p[1] = p[2] = s;
        p[3] = (Uint8)(clamp01(a) * 255.0f);
    }
    void putRGB(Uint8* p, float r, float g, float b, float a) {
        p[0] = (Uint8)(clamp01(r) * 255.0f);
        p[1] = (Uint8)(clamp01(g) * 255.0f);
        p[2] = (Uint8)(clamp01(b) * 255.0f);
        p[3] = (Uint8)(clamp01(a) * 255.0f);
    }
    // red -> yellow -> green -> cyan -> blue -> violet, h = 0..1
    void spectrum(float h, float& r, float& g, float& b) {
        h = clamp01(h) * 5.0f / 6.0f;
        r = clamp01(std::fabs(h * 6.0f - 3.0f) - 1.0f);
        g = clamp01(2.0f - std::fabs(h * 6.0f - 2.0f));
        b = clamp01(2.0f - std::fabs(h * 6.0f - 4.0f));
    }
    float gauss(float x, float y, float cx, float cy, float sigma) {
        const float dx = x - cx, dy = y - cy;
        return std::exp(-(dx * dx + dy * dy) / (2.0f * sigma * sigma));
    }

    // lightning brightness over time since the strike began: ramp, dip, second hit, long decay
    float flashAt(float t) {
        if (t < 0.0f) return 0.0f;
        if (t < 0.05f) return t / 0.05f;
        if (t < 0.11f) return 1.0f - (t - 0.05f) / 0.06f * 0.75f;
        if (t < 0.16f) return 0.25f + (t - 0.11f) / 0.05f * 0.70f;
        return 0.95f * std::exp(-(t - 0.16f) * 5.5f);
    }

    // one short bright pulse centred at c (0..1 of the blink cycle)
    float pulse(float ph, float c, float w) {
        float d = (ph - c) / w;
        return std::exp(-d * d);
    }

    // FLIES / FLY are still accepted in old level files, they just mean FIREFLIES now
    uint32_t flagFor(const std::string& n) {
        if (n == "FIREFLIES" || n == "FIREFLY" || n == "FLIES" || n == "FLY") return AMB_FIREFLIES;
        if (n == "PETALS" || n == "PETAL")      return AMB_PETALS;
        if (n == "LEAVES" || n == "LEAF")       return AMB_LEAVES;
        if (n == "WIND")                        return AMB_WIND;
        if (n == "RAIN")                        return AMB_RAIN;
        if (n == "SNOW")                        return AMB_SNOW;
        if (n == "THUNDER" || n == "LIGHTNING") return AMB_THUNDER;
        if (n == "STORM")                       return AMB_STORM;
        if (n == "SUNFLARE" || n == "SUN" || n == "LENSFLARE" || n == "FLARE") return AMB_SUNFLARE;
        return 0;
    }

    bool parseFloat(const std::string& s, float& out) {
        if (s.empty()) return false;
        char* end = nullptr;
        const float v = std::strtof(s.c_str(), &end);
        if (end == s.c_str() || *end != '\0') return false;
        out = v;
        return true;
    }
    // "a,b,c" -> up to n floats; returns how many were read
    int parseList(const std::string& s, float* out, int n) {
        int got = 0;
        size_t pos = 0;
        while (got < n && pos <= s.size()) {
            size_t comma = s.find(',', pos);
            if (comma == std::string::npos) comma = s.size();
            float v;
            if (!parseFloat(s.substr(pos, comma - pos), v)) break;
            out[got++] = v;
            pos = comma + 1;
        }
        return got;
    }
}

// ---------------------------------------------------------------------------------------------
// biome + defaults
// ---------------------------------------------------------------------------------------------
AmbienceBiome AmbienceFx::biomeFromBackground(const std::vector<BackGroundElement>& elements) {
    bool cave = false, caveLight = false, meadows = false;
    for (const auto& e : elements) {
        switch (e.type) {
            case BackGroundType::CAVE_FAR_LIGHT1:
            case BackGroundType::CAVE_FAR_LIGHT2:
            case BackGroundType::CAVE_FAR_LIGHT3:
                caveLight = true; cave = true; break;
            case BackGroundType::CAVE_SKY:
            case BackGroundType::CAVE_FAR_OBJECT1:
            case BackGroundType::CAVE_FAR_OBJECT2:
            case BackGroundType::CAVE_FAR_OBJECT3:
            case BackGroundType::CAVE_FAR_OBJECT4:
            case BackGroundType::CAVE_FAR_OBJECT5:
                cave = true; break;
            case BackGroundType::MEADOWS_SKY:
            case BackGroundType::MEADOWS_CLOUD:
            case BackGroundType::MEADOWS_FAR_PLAIN:
            case BackGroundType::MEADOWS_FLOWER_FIELD:
                meadows = true; break;
            default: break;
        }
    }
    if (caveLight) return AmbienceBiome::BRIGHT_CAVE;
    if (cave)      return AmbienceBiome::STONE_CAVE;
    if (meadows)   return AmbienceBiome::MEADOWS;
    return AmbienceBiome::FOREST;
}

// What a biome gets when the level file has no 'Z' line. The level's darkness decides the time of day:
//   bright day (<= 0.15)   sun flare
//   dusk/night (>= 0.12)   fireflies (they fade in with the dark, a bright noon has none)
// There are no daytime flies / gnats any more: the only insect is the firefly.
uint32_t AmbienceFx::defaultsFor(AmbienceBiome biome, float darkness) {
    switch (biome) {
        case AmbienceBiome::FOREST:
        case AmbienceBiome::MEADOWS: {
            uint32_t m = AMB_WIND | (biome == AmbienceBiome::FOREST ? AMB_LEAVES : AMB_PETALS);
            if (darkness <= 0.15f) m |= AMB_SUNFLARE;
            if (darkness >= 0.12f) m |= AMB_FIREFLIES;
            return m;
        }
        case AmbienceBiome::STONE_CAVE:  return AMB_NONE;        // the dust motes already cover a stone cave
        case AmbienceBiome::BRIGHT_CAVE: return AMB_FIREFLIES;   // blue glow-worms
    }
    return AMB_NONE;
}

uint32_t AmbienceFx::resolve(AmbienceBiome biome, float darkness, const std::vector<std::string>& tokens) {
    const uint32_t defaults = defaultsFor(biome, darkness);
    if (tokens.empty()) return defaults;

    // "Z SNOW WIND" replaces the defaults; "Z AUTO RAIN" / "Z AUTO -SUNFLARE" start from them
    bool useDefaults = false;
    for (const auto& t : tokens) if (t == "AUTO" || (!t.empty() && t[0] == '-')) useDefaults = true;

    uint32_t m = useDefaults ? defaults : 0u;
    for (const auto& t : tokens) {
        if (t.empty() || t == "AUTO") continue;
        if (t == "NONE") { m = 0; continue; }

        // KEY=VALUE: a flag name switches that effect on (value 0 switches it off), the rest are plain options
        const size_t eq = t.find('=');
        if (eq != std::string::npos) {
            const uint32_t f = flagFor(t.substr(0, eq));
            if (f) {
                float v = 1.0f;
                if (parseFloat(t.substr(eq + 1), v) && std::fabs(v) < 0.0001f) m &= ~f; else m |= f;
            }
            continue;
        }
        const bool remove = (t[0] == '-');
        const uint32_t f = flagFor(remove ? t.substr(1) : t);
        if (remove) m &= ~f; else m |= f;
    }
    // no sun through rain/snow clouds
    if (m & (AMB_RAIN | AMB_SNOW)) m &= ~AMB_SUNFLARE;
    return m;
}

// Reads the KEY=VALUE tokens of the 'Z' line (see Levels/readme.txt). Unknown keys are ignored.
void AmbienceFx::parseOptions(const std::vector<std::string>& tokens) {
    m_opt = Options();
    for (const auto& t : tokens) {
        const size_t eq = t.find('=');
        if (eq == std::string::npos || eq == 0) continue;
        const std::string key = t.substr(0, eq);
        const std::string val = t.substr(eq + 1);
        float f = 0.0f;
        float list[4] = {0, 0, 0, 0};

        if (key == "STEPS" || key == "STEP" || key == "GROUND") {
            if (val == "CONCRETE" || val == "STONE" || val == "ROCK") m_opt.steps = StepSurface::CONCRETE;
            else if (val == "GRASS")                                  m_opt.steps = StepSurface::GRASS;
            else if (val == "LEAVES" || val == "LEAF")                m_opt.steps = StepSurface::LEAVES;
            else if (val == "SNOW")                                   m_opt.steps = StepSurface::SNOW;
            else                                                      m_opt.steps = StepSurface::AUTO;
            continue;
        }
        if (key == "SUNPATH") {
            if (parseList(val, list, 4) == 4) for (int i = 0; i < 4; ++i) m_opt.sunPath[i] = list[i];
            continue;
        }
        if (key == "SUNCOLOR" || key == "SUNCOLOUR") {
            if (parseList(val, list, 3) == 3) for (int i = 0; i < 3; ++i) m_opt.sunRGB[i] = (uint8_t)clampf(list[i], 0.0f, 255.0f);
            continue;
        }
        if (key == "GLOW" || key == "FIREFLYCOLOR" || key == "FIREFLYCOLOUR") {
            if (parseList(val, list, 3) == 3) {
                for (int i = 0; i < 3; ++i) m_opt.glowRGB[i] = (uint8_t)clampf(list[i], 0.0f, 255.0f);
                m_opt.hasGlow = true;
            }
            continue;
        }
        if (!parseFloat(val, f)) continue;

        if      (key == "RAIN")      m_opt.rain      = clampf(f, 0.2f, 3.0f);
        else if (key == "SNOW")      m_opt.snow      = clampf(f, 0.2f, 3.0f);
        else if (key == "SNOWSIZE")  m_opt.snowSize  = clampf(f, 0.5f, 2.5f);
        else if (key == "WIND") {            // WIND=2 strength 2; WIND=-2 strength 2 and the wind blows LEFT
            m_opt.wind = clampf(std::fabs(f), 0.0f, 4.0f);
            if (f < -0.0001f) m_opt.windDir = -1.0f;
        }
        else if (key == "WINDDIR")   m_opt.windDir   = f < 0.0f ? -1.0f : (f > 0.0f ? 1.0f : 0.0f);
        else if (key == "GUST" || key == "GUSTS") m_opt.gust = clampf(f, 0.0f, 3.0f);
        else if (key == "ANGLE" || key == "SLANT") m_opt.angle = clampf(f, -75.0f, 75.0f);
        else if (key == "FOG" || key == "HAZE")    m_opt.fog = clamp01(f);
        else if (key == "THUNDER" || key == "LIGHTNING") m_opt.thunder = clampf(f, 0.2f, 5.0f);
        else if (key == "FIREFLIES" || key == "FIREFLY" || key == "FLIES" || key == "FLY") m_opt.fireflies = clampf(f, 0.0f, 3.0f);
        else if (key == "LEAVES" || key == "LEAF") m_opt.leaves = clampf(f, 0.0f, 3.0f);
        else if (key == "PETALS" || key == "PETAL") m_opt.petals = clampf(f, 0.0f, 3.0f);
        else if (key == "SUNFLARE" || key == "FLARE" || key == "SUN" || key == "LENSFLARE") m_opt.flare = clampf(f, 0.0f, 2.0f);
        else if (key == "DENSITY")   m_opt.density   = clampf(f, 0.25f, 1.5f);
        else if (key == "SOUND" || key == "VOLUME") m_opt.sound = clampf(f, 0.0f, 2.0f);
    }
}

// ---------------------------------------------------------------------------------------------
// setup
// ---------------------------------------------------------------------------------------------
AmbienceFx::~AmbienceFx() { destroySprites(); }

void AmbienceFx::destroySprites() {
    SDL_Texture** all[] = {&m_glow, &m_soft, &m_dot, &m_pt, &m_flake, &m_bokeh, &m_petal, &m_leaf,
                           &m_ring, &m_hex, &m_star, &m_spec};
    for (auto** t : all) {
        if (*t) { SDL_DestroyTexture(*t); *t = nullptr; }
    }
}

AmbienceFx::View AmbienceFx::getView() const {
    const SDL_FRect cam = Camera::getInstance().getCamera();
    return View{cam.w > 0 ? cam.w : 1600.0f, cam.h > 0 ? cam.h : 720.0f, cam.x, cam.y};
}

int AmbienceFx::count(int base) const {
    return std::max(1, (int)std::lround(base * m_density * m_opt.density));
}

void AmbienceFx::init(SDL_Renderer* renderer, AmbienceBiome biome, float darkness,
                      const std::vector<std::string>& tokens, const SDL_FRect* world) {
    m_biome = biome;
    m_darkness = darkness;
    parseOptions(tokens);
    m_mask = resolve(biome, darkness, tokens);
    m_ready = false;
    m_t = 0.0f;
    if (world) { m_worldX = world->x; m_worldY = world->y; m_worldW = world->w; }
    else       { m_worldX = 0.0f;     m_worldY = 0.0f;     m_worldW = 0.0f; }

    for (Group* g : {&m_fireflies, &m_petals, &m_leaves, &m_snow, &m_rain, &m_streaks}) {
        g->back.clear(); g->front.clear();
    }
    const bool cave = (biome == AmbienceBiome::BRIGHT_CAVE);
    m_fireNight = cave ? 1.0f : smoothstep(0.08f, 0.32f, darkness);

    // wind: storms blow harder, WIND= scales it, a negative WIND= (or WINDDIR=) fixes the side
    float baseScale = 1.0f;
    if      (m_mask & AMB_THUNDER) baseScale = 2.0f;
    else if (m_mask & AMB_RAIN)    baseScale = 1.4f;
    else if (m_mask & AMB_SNOW)    baseScale = 1.2f;
    m_windScale = baseScale * (m_opt.wind > 0.0001f ? m_opt.wind : 0.0001f);
    m_windVel = 0.0f; m_gust = 0.0f; m_gustDur = 0.0f; m_gustAge = 0.0f; m_gustReady = false;
    m_gustDir = m_opt.windDir != 0.0f ? m_opt.windDir : 1.0f;
    m_nextGust = rnd(2.0f, 5.0f) / std::max(0.4f, m_opt.gust);
    m_nextStrike = rnd(2.5f, 6.0f) / m_opt.thunder;
    m_flashT = 99.0f; m_thunderDelay = -1.0f; m_thunderReady = false;
    m_bolt.clear(); m_branches.clear();

    for (auto& d : m_dirt) {
        d.x = rnd(0.05f, 0.95f); d.y = rnd(0.08f, 0.92f);
        d.r = rnd(50.0f, 150.0f); d.a = rnd(0.5f, 1.0f);
    }

    if ((m_mask == 0 && m_opt.fog <= 0.0f) || !renderer) return;   // (FOG= alone is still a weather)

    // sprites are built once and kept across levels
    if (!m_glow) m_glow = makeSprite(renderer, 64, [](float x, float y, Uint8* p) {
            float d = std::sqrt(x * x + y * y);
            float core = std::exp(-(d * d) / (2.0f * 0.19f * 0.19f));
            float halo = d >= 1.0f ? 0.0f : 0.42f * std::pow(1.0f - d, 2.2f);
            put(p, 1.0f, core + halo);
        });
    if (!m_soft) m_soft = makeSprite(renderer, 48, [](float x, float y, Uint8* p) {
            float d = std::sqrt(x * x + y * y);
            put(p, 1.0f, d >= 1.0f ? 0.0f : 1.0f - smoothstep(0.0f, 1.0f, d));
        });
    if (!m_dot) m_dot = makeSprite(renderer, 16, [](float x, float y, Uint8* p) {
            float d = std::sqrt(x * x + y * y);
            put(p, 1.0f, smoothstep(1.0f, 0.65f, d));
        });
    // small snow: a gaussian point, no hard edge, so even the tiniest flake has a bit of volume
    if (!m_pt) m_pt = makeSprite(renderer, 32, [](float x, float y, Uint8* p) {
            float d = std::sqrt(x * x + y * y);
            float a = std::exp(-(d * d) / (2.0f * 0.30f * 0.30f)) * (1.0f - smoothstep(0.80f, 1.0f, d));
            put(p, 1.0f, a);
        });
    // fluffy flake: a bright core with a few soft lobes around it (a cluster of crystals seen slightly out of focus)
    if (!m_flake) m_flake = makeSprite(renderer, 64, [](float x, float y, Uint8* p) {
            float a = 0.95f * gauss(x, y, 0.0f, 0.0f, 0.22f);
            static const float lobe[6][2] = {{0.34f, 0.04f}, {-0.30f, 0.18f}, {0.10f, -0.36f}, {-0.12f, 0.38f}, {0.26f, 0.30f}, {-0.34f, -0.22f}};
            for (const auto& l : lobe) a += 0.42f * gauss(x, y, l[0], l[1], 0.15f);
            const float d = std::sqrt(x * x + y * y);
            a *= 1.0f - smoothstep(0.72f, 1.0f, d);
            put(p, 0.94f + 0.06f * clamp01(a), std::min(1.0f, a));
        });
    // bokeh: a big soft disc, a touch brighter at the rim like a real out-of-focus highlight
    if (!m_bokeh) m_bokeh = makeSprite(renderer, 64, [](float x, float y, Uint8* p) {
            float d = std::sqrt(x * x + y * y);
            float body = 0.55f + 0.45f * smoothstep(0.50f, 0.88f, d);
            float edge = 1.0f - smoothstep(0.82f, 1.0f, d);
            put(p, 1.0f, body * edge);
        });
    if (!m_petal) m_petal = makeSprite(renderer, 32, [](float x, float y, Uint8* p) {
            // egg shape along x: narrow tip on the left, round wide end on the right
            float t = (x + 1.0f) * 0.5f;
            float h = 0.62f * std::sin(PI * std::pow(clamp01(t), 0.8f));
            float edge = h - std::fabs(y);
            float shade = 0.72f + 0.28f * t + 0.10f * (h > 0.01f ? 1.0f - std::fabs(y) / h : 0.0f);
            put(p, shade, smoothstep(0.0f, 0.12f, edge));
        });
    if (!m_leaf) m_leaf = makeSprite(renderer, 32, [](float x, float y, Uint8* p) {
            // pointed at both ends, with a darker midrib
            float t = (x + 1.0f) * 0.5f;
            float h = 0.50f * std::pow(std::max(0.0f, std::sin(PI * clamp01(t))), 0.85f);
            float edge = h - std::fabs(y);
            float shade = 0.82f + 0.18f * (h > 0.01f ? y / h : 0.0f) * 0.6f;
            if (std::fabs(y) < 0.05f && h > 0.05f) shade *= 0.68f;
            put(p, shade, smoothstep(0.0f, 0.10f, edge));
        });
    if (!m_ring) {
        m_ring = makeSprite(renderer, 128, [](float x, float y, Uint8* p) {
            float d = std::sqrt(x * x + y * y);
            float k = (d - 0.82f) / 0.07f;
            float a = std::exp(-k * k) + 0.10f * (d >= 1.0f ? 0.0f : 1.0f - smoothstep(0.0f, 1.0f, d));
            put(p, 1.0f, d >= 1.0f ? 0.0f : a);
        });
        if (m_ring) SDL_SetTextureBlendMode(m_ring, SDL_BLENDMODE_ADD);
    }
    if (!m_hex) {
        m_hex = makeSprite(renderer, 64, [](float x, float y, Uint8* p) {
            float dh = std::max({std::fabs(y), std::fabs(0.866f * x + 0.5f * y), std::fabs(0.866f * x - 0.5f * y)}) / 0.9f;
            float fill = smoothstep(1.0f, 0.92f, dh);
            put(p, 1.0f, fill * (0.30f + 0.70f * smoothstep(0.55f, 0.97f, dh)));
        });
        if (m_hex) SDL_SetTextureBlendMode(m_hex, SDL_BLENDMODE_ADD);
    }
    // starburst: soft hot centre + six long thin spikes that fade out with distance
    if (!m_star) {
        m_star = makeSprite(renderer, 256, [](float x, float y, Uint8* p) {
            const float d = std::sqrt(x * x + y * y);
            if (d >= 1.0f) { put(p, 1.0f, 0.0f); return; }
            const float ang = std::atan2(y, x);
            const float spikes = std::pow(std::fabs(std::cos(ang * 3.0f)), 36.0f);   // 6 spikes
            const float fall = std::exp(-d * 4.2f);
            const float a = fall * (0.12f + 0.88f * spikes) + 0.35f * std::exp(-d * d * 60.0f);
            put(p, 1.0f, a * (1.0f - smoothstep(0.85f, 1.0f, d)));
        });
        if (m_star) SDL_SetTextureBlendMode(m_star, SDL_BLENDMODE_ADD);
    }
    // rainbow halo ring (chromatic dispersion): red on the outside, violet on the inside
    if (!m_spec) {
        m_spec = makeSprite(renderer, 256, [](float x, float y, Uint8* p) {
            const float d = std::sqrt(x * x + y * y);
            const float t = (d - 0.74f) / 0.24f;                       // 0..1 across the band
            if (t <= 0.0f || t >= 1.0f) { putRGB(p, 0, 0, 0, 0); return; }
            float r, g, b;
            spectrum(1.0f - t, r, g, b);
            const float a = std::sin(PI * t);
            putRGB(p, r, g, b, a * a);
        });
        if (m_spec) SDL_SetTextureBlendMode(m_spec, SDL_BLENDMODE_ADD);
    }

    buildAll();
    m_ready = true;
}

void AmbienceFx::buildAll() {
    if (m_mask & AMB_FIREFLIES) buildFireflies();
    if (m_mask & AMB_PETALS)    buildDrifters(m_petals, false);
    if (m_mask & AMB_LEAVES)    buildDrifters(m_leaves, true);
    if (m_mask & AMB_SNOW)      buildSnow();
    if (m_mask & AMB_RAIN)      buildRain();
    if (m_mask & AMB_WIND)      buildStreaks();
}

// Fills one layer. `fill` sets everything type-specific; position + wrap domain are shared.
namespace {
    template <class P, class Fill>
    void fillLayer(std::vector<P>& out, int n, float domW, float domH, Fill fill) {
        out.resize(n);
        for (auto& p : out) {
            p.x = rnd(0.0f, domW);
            p.y = rnd(0.0f, domH);
            p.phase = rnd(0.0f, TWO_PI);
            fill(p, rnd(0.0f, 1.0f));   // t = depth inside the layer
        }
    }
}

// Fireflies are the only insects. Each one wanders on its own slow loop and blinks with a quick flash
// (some flash twice), a faint ember glow in between. In daylight they fade out, they belong to dusk.
void AmbienceFx::buildFireflies() {
    const View v = getView();
    const bool cave = (m_biome == AmbienceBiome::BRIGHT_CAVE);
    const float night = m_fireNight;
    static const uint8_t forest[3][3] = {{190, 255, 70}, {225, 255, 110}, {255, 235, 120}};
    static const uint8_t blue[3][3]   = {{90, 180, 255}, {130, 220, 255}, {170, 235, 255}};
    const auto& pal = cave ? blue : forest;
    const float vis = cave ? 1.0f : (0.40f + 0.60f * night);    // a bright noon dims them

    auto make = [&](std::vector<P>& layer, int n, bool front) {
        const float m = front ? FRONT_MARGIN : BACK_MARGIN;
        fillLayer(layer, n, v.w + m * 2, v.h + m * 2, [&](P& p, float t) {
            p.parallax = front ? lerp(1.15f, 1.6f, t) : lerp(0.40f, 0.85f, t);
            p.size     = front ? lerp(9.0f, 15.0f, t) : lerp(5.0f, 9.0f, t);
            p.alpha    = (front ? lerp(0.45f, 0.80f, t) : lerp(0.75f, 1.0f, t)) * vis;
            p.ax = rnd(25.0f, 70.0f) * (front ? 1.6f : 1.0f);
            p.ay = p.ax * rnd(0.5f, 0.9f);
            p.fx = rnd(0.25f, 0.60f);
            p.fy = rnd(0.30f, 0.70f);
            p.period = rnd(2.2f, 4.8f);
            p.flipSpeed = rnd(0.0f, 1.0f) < 0.35f ? 1.0f : 0.0f;   // double flash
            p.drift = rnd(-3.0f, 3.0f);
            p.fall = rnd(-5.0f, 2.0f);
            p.influence = 0.3f;
            if (m_opt.hasGlow) {
                p.r = m_opt.glowRGB[0]; p.g = m_opt.glowRGB[1]; p.b = m_opt.glowRGB[2];
            } else {
                const uint8_t* c = pal[std::uniform_int_distribution<int>(0, 2)(g_rng)];
                p.r = c[0]; p.g = c[1]; p.b = c[2];
            }
        });
    };
    const float k = std::max(0.0f, m_opt.fireflies);
    make(m_fireflies.back,  (k <= 0.0f) ? 0 : count((int)((7 + 11 * night) * k)), false);
    make(m_fireflies.front, (k <= 0.0f) ? 0 : count((int)((2 + 3 * night) * k)), true);
}

void AmbienceFx::buildDrifters(Group& g, bool leaf) {
    const View v = getView();
    static const uint8_t petalPal[5][3] = {{255, 170, 195}, {255, 235, 240}, {255, 214, 140}, {205, 170, 255}, {255, 140, 170}};
    static const uint8_t leafPal[5][3]  = {{120, 185, 70}, {165, 200, 70}, {215, 180, 60}, {205, 120, 45}, {95, 150, 60}};
    const auto& pal = leaf ? leafPal : petalPal;

    auto make = [&](std::vector<P>& layer, int n, bool front) {
        const float m = front ? FRONT_MARGIN : BACK_MARGIN;
        fillLayer(layer, n, v.w + m * 2, v.h + m * 2, [&](P& p, float t) {
            p.parallax = front ? lerp(1.15f, 1.70f, t) : lerp(0.40f, 0.90f, t);
            if (leaf) {
                p.size = front ? lerp(34.0f, 50.0f, t) : lerp(18.0f, 27.0f, t);
                p.fall = front ? lerp(55.0f, 95.0f, t) : lerp(28.0f, 50.0f, t);
                p.swayAmp = rnd(10.0f, 26.0f) * (front ? 1.5f : 1.0f);
                p.flipSpeed = rnd(0.8f, 2.0f);
                p.spin = rnd(-1.2f, 1.2f);
                p.influence = 0.8f;
            } else {
                p.size = front ? lerp(26.0f, 40.0f, t) : lerp(13.0f, 20.0f, t);
                p.fall = front ? lerp(48.0f, 85.0f, t) : lerp(20.0f, 38.0f, t);
                p.swayAmp = rnd(14.0f, 36.0f) * (front ? 1.5f : 1.0f);
                p.flipSpeed = rnd(1.5f, 3.2f);
                p.spin = rnd(-0.4f, 0.4f);
                p.influence = 0.95f;
            }
            p.alpha = front ? lerp(0.85f, 1.0f, t) : lerp(0.60f, 0.90f, t);
            p.swayFreq = rnd(0.6f, 1.4f);
            p.drift = rnd(-5.0f, 5.0f);
            p.rot = rnd(0.0f, TWO_PI);
            const uint8_t* c = pal[std::uniform_int_distribution<int>(0, 4)(g_rng)];
            p.r = c[0]; p.g = c[1]; p.b = c[2];
        });
    };
    const float k = leaf ? m_opt.leaves : m_opt.petals;
    if (k <= 0.0f) return;
    make(g.back,  count((int)((leaf ? 14 : 22) * k)), false);
    make(g.front, count((int)((leaf ? 5 : 7) * k)), true);
}

// Snow with volume: three kinds of flake instead of one hard dot.
//   kind 0  tiny soft point (far away)         kind 1  fluffy flake (mid distance)
//   kind 2  big out-of-focus bokeh (right in front of the lens, front layer only)
// Back layer = 0 + 1, front layer = 1 + 2, so there is always a gradient from sharp to blurry.
void AmbienceFx::buildSnow() {
    const View v = getView();
    const float sz = m_opt.snowSize;
    auto make = [&](std::vector<P>& layer, int n, bool front) {
        const float m = front ? FRONT_MARGIN : BACK_MARGIN;
        fillLayer(layer, n, v.w + m * 2, v.h + m * 2, [&](P& p, float t) {
            const float roll = rnd(0.0f, 1.0f);
            if (front) p.kind = roll < 0.42f ? 1 : 2;
            else       p.kind = roll < 0.62f ? 0 : 1;

            p.parallax = front ? lerp(1.15f, 1.70f, t) : lerp(0.35f, 0.85f, t);
            switch (p.kind) {
                case 0:  p.size = lerp(4.0f, 7.5f, t);   p.alpha = lerp(0.55f, 0.95f, t); p.fall = lerp(22.0f, 40.0f, t);  break;
                case 1:  p.size = front ? lerp(14.0f, 26.0f, t) : lerp(9.0f, 15.0f, t);
                    p.alpha = front ? lerp(0.40f, 0.65f, t) : lerp(0.60f, 0.90f, t);
                    p.fall  = front ? lerp(55.0f, 95.0f, t) : lerp(30.0f, 52.0f, t);                                   break;
                default: p.size = lerp(34.0f, 72.0f, t); p.alpha = lerp(0.10f, 0.26f, t); p.fall = lerp(80.0f, 140.0f, t); break;
            }
            p.size *= sz;
            p.swayAmp  = rnd(8.0f, 26.0f) * (front ? 1.6f : 1.0f);
            p.swayFreq = rnd(0.5f, 1.4f);
            p.drift = rnd(-4.0f, 4.0f);
            p.influence = front ? 1.0f : 0.7f;
            p.flipSpeed = rnd(0.6f, 2.0f);       // tumbling pulse
            p.rot = rnd(0.0f, TWO_PI);
            p.r = 234; p.g = 241; p.b = 255;
        });
    };
    make(m_snow.back,  count((int)(86 * m_opt.snow)), false);
    make(m_snow.front, count((int)(20 * m_opt.snow)), true);
}

void AmbienceFx::buildRain() {
    const View v = getView();
    const bool heavy = (m_mask & AMB_THUNDER) != 0 || m_opt.rain > 1.4f;
    const float opacity = clampf(0.75f + 0.25f * m_opt.rain, 0.5f, 1.3f);
    auto make = [&](std::vector<P>& layer, int n, bool front) {
        const float m = front ? FRONT_MARGIN : BACK_MARGIN;
        fillLayer(layer, n, v.w + m * 2, v.h + m * 2, [&](P& p, float t) {
            p.parallax = front ? lerp(1.2f, 1.7f, t) : lerp(0.5f, 0.9f, t);
            p.fall  = front ? lerp(1100.0f, 1500.0f, t) : lerp(700.0f, 950.0f, t);
            p.len   = front ? lerp(28.0f, 52.0f, t) : lerp(12.0f, 22.0f, t);
            p.size  = front ? lerp(1.6f, 2.4f, t) : lerp(1.0f, 1.4f, t);      // line width
            p.alpha = (front ? lerp(0.40f, 0.65f, t) : lerp(0.30f, 0.50f, t)) * opacity;
            p.influence = 1.0f;     // drops are carried by the wind (slower back drops lean more, like real rain)
            p.r = 175; p.g = 195; p.b = 225;
        });
    };
    make(m_rain.back,  count((int)((heavy ? 90 : 60) * m_opt.rain)), false);
    make(m_rain.front, count((int)((heavy ? 30 : 18) * m_opt.rain)), true);
}

void AmbienceFx::buildStreaks() {
    const View v = getView();
    const bool snowy = (m_mask & AMB_SNOW) != 0;      // snow-laden wind: more, whiter, thicker streaks
    auto make = [&](std::vector<P>& layer, int n, bool front) {
        const float m = front ? FRONT_MARGIN : BACK_MARGIN;
        fillLayer(layer, n, v.w + m * 2, v.h + m * 2, [&](P& p, float t) {
            p.parallax = front ? lerp(1.2f, 1.7f, t) : lerp(0.5f, 0.9f, t);
            p.len   = front ? lerp(180.0f, 420.0f, t) : lerp(110.0f, 280.0f, t);
            p.size  = (front ? lerp(2.0f, 3.5f, t) : lerp(1.2f, 2.0f, t)) * (snowy ? 1.5f : 1.0f);
            p.alpha = (front ? lerp(0.10f, 0.18f, t) : lerp(0.12f, 0.22f, t)) * (snowy ? 1.7f : 1.0f);
            p.rot   = rnd(-0.05f, 0.08f);       // slight slope
            p.influence = 2.0f;                 // streaks outrun the wind a bit
            if (snowy) { p.r = 246; p.g = 250; p.b = 255; }
            else       { p.r = 235; p.g = 245; p.b = 255; }
        });
    };
    make(m_streaks.back,  count(snowy ? 16 : 7), false);
    make(m_streaks.front, count(snowy ? 8 : 4), true);
}

// ---------------------------------------------------------------------------------------------
// simulation
// ---------------------------------------------------------------------------------------------
void AmbienceFx::updateWind(float dt) {
    const bool gusts = (m_mask & AMB_WIND) != 0 && m_opt.gust > 0.001f;
    const bool wet = (m_mask & (AMB_RAIN | AMB_SNOW)) != 0;
    // rain / snow always sway a little (slight slant) even without a WIND token
    const float baseAmp = wet ? 110.0f : 40.0f;
    float base = (std::sin(m_t * 0.17f + 1.0f) * 0.55f + std::sin(m_t * 0.43f + 2.1f) * 0.25f) * baseAmp * m_windScale;
    base += m_opt.windDir * 70.0f * m_windScale;       // prevailing wind (WIND=-n / WINDDIR=)

    if (gusts) {
        const float g = clampf(m_opt.gust, 0.3f, 3.0f);
        if (m_gustDur > 0.0f) {
            m_gustAge += dt;
            if (m_gustAge >= m_gustDur) {
                m_gustDur = 0.0f; m_gust = 0.0f;
                m_nextGust = rnd(6.0f, 14.0f) / (m_windScale > 1.5f ? 1.8f : 1.0f) / g;
            } else {
                float s = std::sin(PI * m_gustAge / m_gustDur);
                m_gust = m_gustPeak * s * s;   // smooth swell and fade
            }
        } else {
            m_nextGust -= dt;
            if (m_nextGust <= 0.0f) {
                m_gustDur = rnd(2.5f, 4.5f);
                m_gustAge = 0.0f;
                m_gustPeak = rnd(0.6f, 1.0f) * std::min(g, 2.0f);
                if (m_opt.windDir != 0.0f) m_gustDir = m_opt.windDir;
                else                       m_gustDir = rnd(0.0f, 1.0f) < 0.65f ? 1.0f : -1.0f;
                // tell the sound system a gust is starting
                m_gustReady = true;
                m_gustReadyStrength = clampf(0.3f + 0.7f * m_gustPeak / std::max(1.0f, std::min(g, 2.0f)), 0.3f, 1.0f);
                m_gustReadyDir = m_gustDir;
            }
        }
    } else {
        m_gust = 0.0f;
    }
    m_windVel = clampf(base * (gusts ? 1.0f : 0.5f) + m_gustDir * m_gust * 260.0f * m_windScale, -1500.0f, 1500.0f);
}

void AmbienceFx::makeBolt() {
    const View v = getView();
    auto subdivide = [](std::vector<SDL_FPoint> pts, float amp, int iters) {
        for (int it = 0; it < iters; ++it) {
            std::vector<SDL_FPoint> out;
            out.reserve(pts.size() * 2);
            for (size_t i = 0; i + 1 < pts.size(); ++i) {
                SDL_FPoint a = pts[i], b = pts[i + 1];
                out.push_back(a);
                out.push_back(SDL_FPoint{(a.x + b.x) * 0.5f + rnd(-amp, amp), (a.y + b.y) * 0.5f});
            }
            out.push_back(pts.back());
            pts.swap(out);
            amp *= 0.55f;
        }
        return pts;
    };

    // anchored in layer space with parallax 0.5, so it sits "far away" and slides slowly with the camera
    const float screenX = rnd(0.12f, 0.88f) * v.w;
    m_boltParallaxX = screenX + v.camX * 0.5f;
    m_thunderPan = clampf((screenX - v.w * 0.5f) / (v.w * 0.5f), -1.0f, 1.0f);   // the clap comes from the same side
    const float endY = v.h * rnd(0.55f, 0.85f);
    m_bolt = subdivide({SDL_FPoint{0.0f, -10.0f}, SDL_FPoint{rnd(-80.0f, 80.0f), endY}}, 60.0f, 5);

    m_branches.clear();
    const int nb = std::uniform_int_distribution<int>(1, 2)(g_rng);
    for (int i = 0; i < nb; ++i) {
        size_t k = (size_t)std::uniform_int_distribution<int>((int)m_bolt.size() / 6, (int)m_bolt.size() * 2 / 3)(g_rng);
        k = std::min(k, m_bolt.size() - 1);
        const SDL_FPoint s = m_bolt[k];
        m_branches.push_back(subdivide({s, SDL_FPoint{s.x + rnd(-170.0f, 170.0f), s.y + rnd(80.0f, 210.0f)}}, 26.0f, 3));
    }
}

void AmbienceFx::updateThunder(float dt) {
    if (!(m_mask & AMB_THUNDER)) return;
    m_flashT += dt;
    m_nextStrike -= dt;
    if (m_nextStrike <= 0.0f) {
        m_flashT = 0.0f;
        makeBolt();
        m_thunderDelay = rnd(0.02f, 0.12f);   // the clap comes with the strike
        m_thunderStrength = rnd(0.5f, 1.0f);
        m_nextStrike = rnd(6.0f, 15.0f) / m_opt.thunder;
    }
    if (m_thunderDelay >= 0.0f) {
        m_thunderDelay -= dt;
        if (m_thunderDelay < 0.0f) { m_thunderReady = true; }
    }
}

bool AmbienceFx::consumeThunder(float& strength) {
    float pan;
    return consumeThunder(strength, pan);
}

bool AmbienceFx::consumeThunder(float& strength, float& pan) {
    if (!m_thunderReady) return false;
    m_thunderReady = false;
    strength = m_thunderStrength;
    pan = m_thunderPan;
    return true;
}

bool AmbienceFx::consumeGust(float& strength, float& dir) {
    if (!m_gustReady) return false;
    m_gustReady = false;
    strength = m_gustReadyStrength;
    dir = m_gustReadyDir;
    return true;
}

AmbienceFx::SoundLevels AmbienceFx::soundLevels() const {
    SoundLevels s;
    if (!m_ready) return s;
    const float aw = clamp01(std::fabs(m_windVel) / 700.0f);
    if (m_mask & AMB_RAIN) {
        s.rain = clamp01(0.30f * m_opt.rain + ((m_mask & AMB_THUNDER) ? 0.22f : 0.0f) + 0.18f * aw);
    }
    if (m_mask & AMB_WIND)                       s.wind = clamp01(0.12f + 0.85f * aw);          // swells with every gust
    else if (m_mask & (AMB_RAIN | AMB_SNOW))     s.wind = clamp01(0.06f + 0.55f * aw);          // weather always has some air in it
    if ((m_mask & AMB_SNOW) && (m_mask & AMB_WIND)) s.wind = clamp01(s.wind * 1.15f);           // blizzard howl
    s.rain *= m_opt.sound; s.wind *= m_opt.sound;
    return s;
}

// STEPS= wins; otherwise the weather and the biome decide what the ground sounds like
StepSurface AmbienceFx::stepSurface() const {
    if (m_opt.steps != StepSurface::AUTO) return m_opt.steps;
    if (m_mask & AMB_SNOW)   return StepSurface::SNOW;
    if (m_mask & AMB_LEAVES) return StepSurface::LEAVES;
    if (m_biome == AmbienceBiome::FOREST || m_biome == AmbienceBiome::MEADOWS) return StepSurface::GRASS;
    return StepSurface::CONCRETE;
}

void AmbienceFx::update(float dt) {
    if (!m_ready) return;
    dt = std::min(dt, 0.05f);   // a long frame (level load, resume) must not teleport the particles
    m_t += dt;
    updateWind(dt);
    updateThunder(dt);

    const View v = getView();
    auto dom = [&](bool front, float& w, float& h) {
        const float m = front ? FRONT_MARGIN : BACK_MARGIN;
        w = v.w + m * 2; h = v.h + m * 2;
    };
    const float k = std::min(1.0f, dt * 1.3f);   // wind response lag
    const float tanA = std::tan(m_opt.angle * PI / 180.0f);   // constant slant of rain / snow (ANGLE=)
    const float snowTurb = clamp01((std::fabs(m_windVel) - 120.0f) / 500.0f);   // swirl when it really blows

    for (int pass = 0; pass < 2; ++pass) {
        const bool front = (pass == 1);
        float dw, dh;
        dom(front, dw, dh);

        // fireflies: the home point drifts, the lissajous wander is added at render time
        for (auto& p : (front ? m_fireflies.front : m_fireflies.back)) {
            p.x = wrap(p.x + (p.drift + m_windVel * p.influence * p.parallax) * dt, dw);
            p.y = wrap(p.y + p.fall * dt, dh);
        }

        // petals / leaves: fall, lean into the wind with a lag, lift in gusts
        auto drift = [&](std::vector<P>& layer, float lift, bool spin) {
            for (auto& p : layer) {
                p.vx += (m_windVel * p.influence - p.vx) * k;
                p.x = wrap(p.x + (p.vx + p.drift) * dt, dw);
                const float fall = p.fall * (0.85f + 0.15f * std::sin(m_t * 0.9f + p.phase));
                p.y = wrap(p.y + (fall - m_gust * lift) * dt, dh);
                if (spin) p.rot += p.spin * dt;
            }
        };
        drift(front ? m_petals.front : m_petals.back, 55.0f, false);
        drift(front ? m_leaves.front : m_leaves.back, 35.0f, true);

        // snow: same as above + the constant slant + a swirl that grows with the wind (blizzard)
        for (auto& p : (front ? m_snow.front : m_snow.back)) {
            const float swirl = std::sin(m_t * 2.3f + p.phase * 1.7f) * std::fabs(m_windVel) * 0.18f * snowTurb * p.influence;
            const float target = m_windVel * p.influence + tanA * p.fall + swirl;
            p.vx += (target - p.vx) * k;
            p.x = wrap(p.x + (p.vx + p.drift) * dt, dw);
            const float fall = p.fall * (0.85f + 0.15f * std::sin(m_t * 0.9f + p.phase));
            p.y = wrap(p.y + (fall - m_gust * 25.0f) * dt, dh);
        }

        // rain: the wind carries the drops sideways, ANGLE= adds a fixed slant on top
        // (angle from the vertical = atan(vx / fall), so even a gentle breeze tilts the slow back drops more)
        for (auto& p : (front ? m_rain.front : m_rain.back)) {
            float target = m_windVel * p.influence + tanA * p.fall;
            target = clampf(target, -p.fall * 3.7f, p.fall * 3.7f);      // never past ~75 degrees
            p.vx += (target - p.vx) * std::min(1.0f, dt * 5.0f);
            p.x = wrap(p.x + p.vx * dt, dw);
            p.y = wrap(p.y + p.fall * dt, dh);
        }

        // wind streaks: ride the wind
        for (auto& p : (front ? m_streaks.front : m_streaks.back)) {
            p.x = wrap(p.x + m_windVel * p.influence * p.parallax * dt, dw);
        }
    }
}

// ---------------------------------------------------------------------------------------------
// rendering
// ---------------------------------------------------------------------------------------------
void AmbienceFx::fireflyScreen(const P& p, bool front, const View& v, float& sx, float& sy, float& a) const {
    const float margin = front ? FRONT_MARGIN : BACK_MARGIN;
    const float domW = v.w + margin * 2, domH = v.h + margin * 2;
    const float ox = p.ax * std::sin(m_t * p.fx + p.phase) + p.ax * 0.35f * std::sin(m_t * p.fx * 2.3f + p.phase * 1.7f);
    const float oy = p.ay * std::sin(m_t * p.fy + p.phase * 0.6f) + p.ay * 0.35f * std::cos(m_t * p.fy * 1.9f + p.phase);
    sx = wrap(p.x + ox - v.camX * p.parallax, domW) - margin;
    sy = wrap(p.y + oy - v.camY * p.parallax, domH) - margin;
    // blink: a quick flash (some flash twice), only a faint ember glow in between
    const float ph = fract(m_t / p.period + p.phase / TWO_PI);
    float b = pulse(ph, 0.18f, 0.075f);
    if (p.flipSpeed > 0.5f) b += 0.75f * pulse(ph, 0.36f, 0.05f);
    a = p.alpha * (0.07f + 0.93f * std::min(1.0f, b));
}

void AmbienceFx::renderLayer(SDL_Renderer* renderer, bool front) {
    const View v = getView();
    const float margin = front ? FRONT_MARGIN : BACK_MARGIN;
    const float domW = v.w + margin * 2, domH = v.h + margin * 2;

    auto scr = [&](const P& p, float offX, float offY, float& sx, float& sy) {
        sx = wrap(p.x + offX - v.camX * p.parallax, domW) - margin;
        sy = wrap(p.y + offY - v.camY * p.parallax, domH) - margin;
    };
    auto visible = [&](float sx, float sy, float r) {
        return !(sx < -r || sx > v.w + r || sy < -r || sy > v.h + r);
    };
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // wind streaks (untextured, faded at both ends, a little bent); in a snowstorm they are white and heavier
    if (m_mask & AMB_WIND) {
        const bool snowy = (m_mask & AMB_SNOW) != 0;
        const float vis = clamp01(m_gust * 1.2f + std::min(snowy ? 0.45f : 0.25f, std::fabs(m_windVel) / (snowy ? 300.0f : 400.0f)));
        if (vis > 0.02f) {
            const float dir = (m_windVel >= 0.0f) ? 1.0f : -1.0f;
            for (const auto& p : (front ? m_streaks.front : m_streaks.back)) {
                float sx, sy; scr(p, 0, 0, sx, sy);
                if (!visible(sx, sy, p.len)) continue;
                const float h = p.len * 0.5f * dir;
                const float x0 = sx - h, x1 = sx + h;
                const float y0 = sy - h * p.rot, y1 = sy + h * p.rot;
                const float bend = std::sin(m_t * 0.9f + p.phase) * p.len * 0.06f;
                const float mx = sx, my = sy + bend;
                const SDL_FColor clear = col(p.r, p.g, p.b, 0.0f), mid = col(p.r, p.g, p.b, p.alpha * vis);
                g_batch.line(x0, y0, mx, my, p.size, clear, mid);
                g_batch.line(mx, my, x1, y1, p.size, mid, clear);
            }
            g_batch.flush(renderer, nullptr);
        }
    }

    // rain: streak along the real direction of fall (wind + ANGLE), bright head / transparent tail,
    // longer when the drop is slanted (it moves faster, so it smears more)
    if (m_mask & AMB_RAIN) {
        for (const auto& p : (front ? m_rain.front : m_rain.back)) {
            float sx, sy; scr(p, 0, 0, sx, sy);
            if (!visible(sx, sy, p.len * 2.5f)) continue;
            const float l = std::sqrt(p.vx * p.vx + p.fall * p.fall);
            const float slant = std::min(2.5f, std::fabs(p.vx) / std::max(1.0f, p.fall));
            const float len = p.len * (1.0f + 0.35f * slant);
            const float tx = sx - p.vx / l * len, ty = sy - p.fall / l * len;
            g_batch.line(tx, ty, sx, sy, p.size, col(p.r, p.g, p.b, 0.0f), col(p.r, p.g, p.b, p.alpha));
        }
        g_batch.flush(renderer, nullptr);
    }

    // snow: soft points + fluffy flakes + bokeh. Flakes smear along their motion when the wind picks up.
    if ((m_mask & AMB_SNOW) && m_pt && m_flake && m_bokeh) {
        const auto& layer = front ? m_snow.front : m_snow.back;
        SDL_Texture* tex[3] = {m_pt, m_flake, m_bokeh};
        for (int kind = 0; kind < 3; ++kind) {
            for (const auto& p : layer) {
                if (p.kind != kind) continue;
                const float sway = std::sin(m_t * p.swayFreq + p.phase) * p.swayAmp;
                float sx, sy; scr(p, sway, 0, sx, sy);
                if (!visible(sx, sy, p.size * 2.0f)) continue;
                const float tumble = 1.0f + 0.10f * std::sin(m_t * p.flipSpeed + p.phase);
                const float size = p.size * tumble;
                float stretch = 1.0f, rot = p.rot;
                if (kind != 2) {
                    const float vxn = p.vx + p.drift;
                    stretch = 1.0f + std::min(2.5f, std::fabs(vxn) / (p.fall + 40.0f)) * 0.9f;
                    rot = std::atan2(p.fall, vxn);
                }
                g_batch.quad(sx, sy, size * 0.5f * stretch, size * 0.5f, rot, col(p.r, p.g, p.b, p.alpha));
            }
            g_batch.flush(renderer, tex[kind]);
        }
    }

    // leaves + petals: tumble (width flips like a 3D turn), sway, lean into the wind
    auto drawDrifters = [&](const std::vector<P>& layer, SDL_Texture* tex, bool leaf) {
        for (const auto& p : layer) {
            const float sway = std::sin(m_t * p.swayFreq + p.phase) * p.swayAmp;
            float sx, sy; scr(p, sway, 0, sx, sy);
            if (!visible(sx, sy, p.size)) continue;
            const float flip = 0.20f + 0.80f * std::fabs(std::cos(m_t * p.flipSpeed + p.phase));
            const float lean = p.vx * 0.004f;
            const float rot = leaf ? (p.rot * 0.35f + lean) : (std::sin(m_t * p.swayFreq * 1.3f + p.phase) * 0.7f + lean);
            g_batch.quad(sx, sy, p.size * 0.5f * flip, p.size * 0.5f, rot, col(p.r, p.g, p.b, p.alpha));
        }
        g_batch.flush(renderer, tex);
    };
    if (m_mask & AMB_LEAVES)  drawDrifters(front ? m_leaves.front : m_leaves.back, m_leaf, true);
    if (m_mask & AMB_PETALS)  drawDrifters(front ? m_petals.front : m_petals.back, m_petal, false);

    // fireflies: glow sprite, blinking
    if (m_mask & AMB_FIREFLIES) {
        for (const auto& p : (front ? m_fireflies.front : m_fireflies.back)) {
            float sx, sy, a; fireflyScreen(p, front, v, sx, sy, a);
            const float d = p.size * 4.8f;
            if (!visible(sx, sy, d)) continue;
            g_batch.quad(sx, sy, d * 0.5f, d * 0.5f, 0.0f, col(p.r, p.g, p.b, a));
        }
        g_batch.flush(renderer, m_glow);
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void AmbienceFx::renderBack(SDL_Renderer* renderer) {
    if (!m_ready) return;

    // lightning bolt: far behind everything (drawn first), glow + white core
    if ((m_mask & AMB_THUNDER) && !m_bolt.empty() && m_flashT < 0.30f) {
        const float f = flashAt(m_flashT);
        if (f > 0.01f) {
            const View v = getView();
            const float ax = m_boltParallaxX - v.camX * 0.5f;
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            auto strokes = [&](const std::vector<SDL_FPoint>& pts, float wGlow, float wCore, float k) {
                for (size_t i = 0; i + 1 < pts.size(); ++i) {
                    const SDL_FColor g = col(150, 170, 255, 0.35f * f * k);
                    g_batch.line(ax + pts[i].x, pts[i].y, ax + pts[i + 1].x, pts[i + 1].y, wGlow, g, g);
                }
                g_batch.flush(renderer, nullptr);
                for (size_t i = 0; i + 1 < pts.size(); ++i) {
                    const SDL_FColor c = col(255, 255, 255, f * k);
                    g_batch.line(ax + pts[i].x, pts[i].y, ax + pts[i + 1].x, pts[i + 1].y, wCore, c, c);
                }
                g_batch.flush(renderer, nullptr);
            };
            strokes(m_bolt, 14.0f, 3.5f, 1.0f);
            for (const auto& b : m_branches) strokes(b, 8.0f, 2.0f, 0.7f);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        }
    }
    if (m_mask & AMB_SUNFLARE) renderSunBack(renderer, getView());
    renderLayer(renderer, false);
}

void AmbienceFx::renderFront(SDL_Renderer* renderer) {
    if (!m_ready) return;
    renderLayer(renderer, true);
}

// ---------------------------------------------------------------------------------------------
// the sun
// ---------------------------------------------------------------------------------------------
// The sun lives far away in the sky and slides across the screen while the player travels through the level:
// SUNPATH gives its screen position at the start and at the end of the level, the camera decides where it is
// in between. It fades out when it leaves the screen and its strength follows the level's darkness.
bool AmbienceFx::sunState(const View& v, float& sx, float& sy, float& vis, float& center) const {
    float vis0 = clamp01(1.0f - m_darkness * 1.5f) * m_opt.flare;
    if (vis0 < 0.02f) return false;

    float prog = 0.5f;
    const float span = m_worldW - v.w;
    if (span > 1.0f) prog = clamp01((v.camX - m_worldX) / span);
    sx = lerp(m_opt.sunPath[0], m_opt.sunPath[2], prog) * v.w;
    sy = lerp(m_opt.sunPath[1], m_opt.sunPath[3], prog) * v.h - (v.camY - m_worldY) * 0.04f;   // climbing / diving moves it a little

    // fade when it is off screen (a sun just outside the frame still throws a flare, 30% of a screen away it is gone)
    const float ox = std::max(0.0f, std::fabs(sx - v.w * 0.5f) - v.w * 0.5f) / (v.w * 0.30f);
    const float oy = std::max(0.0f, std::fabs(sy - v.h * 0.5f) - v.h * 0.5f) / (v.h * 0.30f);
    const float off = std::sqrt(ox * ox + oy * oy);
    vis = vis0 * (1.0f - smoothstep(0.0f, 1.0f, off));
    vis *= 0.93f + 0.045f * std::sin(m_t * 0.7f) + 0.025f * std::sin(m_t * 2.1f + 1.3f);   // thin cloud / haze breathing
    if (vis < 0.02f) return false;

    const float dcx = (sx - v.w * 0.5f) / v.w, dcy = (sy - v.h * 0.5f) / v.h;
    center = clamp01(1.0f - std::sqrt(dcx * dcx + dcy * dcy) / 0.75f);
    return true;
}

// Behind the terrain (inside the bloom group): a glow in the sky and god rays. Everything solid in front of
// them casts a shadow for free, so the light shafts come out between the trees and rocks.
void AmbienceFx::renderSunBack(SDL_Renderer* r, const View& v) {
    if (!m_glow) return;
    float sx, sy, vis, center;
    if (!sunState(v, sx, sy, vis, center)) return;
    const uint8_t R = m_opt.sunRGB[0], G = m_opt.sunRGB[1], B = m_opt.sunRGB[2];

    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_ADD);
    SDL_SetTextureBlendMode(m_glow, SDL_BLENDMODE_ADD);
    g_batch.quad(sx, sy, 440.0f, 440.0f, 0.0f, col(R, G, B, 0.24f * vis));
    g_batch.quad(sx, sy, 150.0f, 150.0f, 0.0f, col(255, 244, 220, 0.50f * vis));
    g_batch.flush(r, m_glow);
    SDL_SetTextureBlendMode(m_glow, SDL_BLENDMODE_BLEND);

    // god rays: thin wedges, bright at the sun, gone at the tip. They rotate slowly with the sun's travel.
    const int N = 16;
    const float base = sx * 0.0011f;
    for (int i = 0; i < N; ++i) {
        const float h = hash1(i * 3.7f + 1.0f), h2 = hash1(i * 5.3f + 2.0f), h3 = hash1(i * 7.1f + 3.0f);
        const float a = base + i * TWO_PI / N + (h - 0.5f) * 0.25f + 0.04f * std::sin(m_t * 0.3f + i * 1.9f);
        const float len = (650.0f + 700.0f * h2) * (0.80f + 0.30f * center);
        const float hw = 0.028f + 0.055f * h3;
        const float al = (0.045f + 0.075f * h) * vis * (0.75f + 0.25f * std::sin(m_t * 0.6f + i * 2.3f));
        const SDL_FColor c0 = col(R, G, B, al), c1 = col(R, G, B, 0.0f);
        g_batch.tri(sx, sy, c0,
                    sx + std::cos(a - hw) * len, sy + std::sin(a - hw) * len, c1,
                    sx + std::cos(a + hw) * len, sy + std::sin(a + hw) * len, c1);
    }
    g_batch.flush(r, nullptr);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
}

// In front of everything, after the darkness pass. Real lens flares are the sun's light bouncing between the
// lens elements, so the ghosts sit on the line from the sun THROUGH the middle of the screen. As the sun slides
// across the sky (player travels), that line sweeps, the ghosts glide along it and swap sides when the sun
// crosses the centre; the veil and the halo are strongest when the sun is near the middle of the view.
void AmbienceFx::renderLensFlare(SDL_Renderer* r, const View& v) {
    if (!m_glow || !m_ring || !m_hex || !m_star || !m_spec || !m_soft) return;
    float sx, sy, vis, center;
    if (!sunState(v, sx, sy, vis, center)) return;

    const uint8_t sR = m_opt.sunRGB[0], sG = m_opt.sunRGB[1], sB = m_opt.sunRGB[2];
    // tint a ghost colour towards the sun colour (a red sunset makes red ghosts)
    auto tone = [&](uint8_t cr, uint8_t cg, uint8_t cb, float a) {
        return colf((cr / 255.0f) * (0.45f + 0.55f * sR / 255.0f),
                    (cg / 255.0f) * (0.45f + 0.55f * sG / 255.0f),
                    (cb / 255.0f) * (0.45f + 0.55f * sB / 255.0f), a);
    };

    const float cx = v.w * 0.5f, cy = v.h * 0.5f;
    const float dx = cx - sx, dy = cy - sy;              // sun -> screen centre axis
    const float axisAng = std::atan2(dy, dx);
    const float nearK = 0.55f + 0.45f * center;            // veil / halo / core: strongest with the sun in view
    const float farK  = 0.55f + 0.45f * (1.0f - center);   // ghosts spread and brighten as the sun nears the edge

    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_ADD);
    SDL_SetTextureBlendMode(m_glow, SDL_BLENDMODE_ADD);
    SDL_SetTextureBlendMode(m_soft, SDL_BLENDMODE_ADD);

    // 1. veil over the whole view (light scattering in the lens) + lens dirt that lights up near the sun
    g_batch.quad(sx, sy, v.w * 1.25f, v.w * 1.25f, 0.0f, tone(255, 220, 160, 0.085f * vis * nearK));
    g_batch.quad(sx, sy, 300.0f, 300.0f, 0.0f, tone(255, 228, 180, 0.22f * vis * nearK));
    g_batch.quad(sx, sy, 100.0f, 100.0f, 0.0f, col(255, 248, 232, 0.62f * vis));
    g_batch.quad(sx, sy, 34.0f, 34.0f, 0.0f, col(255, 255, 255, 0.85f * vis));
    // anamorphic streak: a long cool line and a short warm one
    g_batch.quad(sx, sy, 640.0f, 11.0f, 0.0f, tone(140, 185, 255, 0.28f * vis));
    g_batch.quad(sx, sy, 430.0f, 5.0f, 0.0f, tone(255, 238, 205, 0.34f * vis));
    g_batch.flush(r, m_glow);

    for (const auto& d : m_dirt) {
        const float px = d.x * v.w, py = d.y * v.h;
        const float dist = std::sqrt((px - sx) * (px - sx) + (py - sy) * (py - sy));
        const float k = (1.0f - smoothstep(0.0f, v.w * 0.85f, dist)) * vis * d.a * 0.075f;
        if (k > 0.002f) g_batch.quad(px, py, d.r, d.r, 0.0f, tone(255, 230, 190, k));
    }
    g_batch.flush(r, m_soft);

    // 2. starburst: two crossed spike sets that turn as the sun slides
    const float pulseK = 0.9f + 0.1f * std::sin(m_t * 0.9f);
    g_batch.quad(sx, sy, 340.0f * pulseK, 340.0f * pulseK, sx * 0.0009f, tone(255, 240, 210, 0.42f * vis * nearK));
    g_batch.quad(sx, sy, 230.0f * pulseK, 230.0f * pulseK, sx * 0.0009f + 0.5236f, tone(255, 225, 190, 0.26f * vis * nearK));
    g_batch.flush(r, m_star);

    // 3. rainbow halo around the sun (it breathes a little)
    g_batch.quad(sx, sy, v.w * (0.17f + 0.006f * std::sin(m_t * 0.5f)), v.w * (0.17f + 0.006f * std::sin(m_t * 0.5f)),
                 0.0f, colf(1.0f, 1.0f, 1.0f, 0.10f * vis * nearK));
    // a second, fainter, bigger one centred on the screen (reflection of the first)
    g_batch.quad(cx, cy, 0.30f * v.w + 0.35f * std::sqrt(dx * dx + dy * dy), 0.30f * v.w + 0.35f * std::sqrt(dx * dx + dy * dy),
                 0.0f, colf(1.0f, 1.0f, 1.0f, 0.045f * vis * farK));
    g_batch.flush(r, m_spec);

    // 4. ghost chain along the axis: t = 0 sun, t = 1 screen centre, t > 1 the far side
    struct Ghost { float t, rad; int tex; uint8_t r, g, b; float a; };
    static const Ghost ghosts[] = {
            {-0.20f,  26.0f, 0, 255, 200, 140, 0.20f},
            { 0.18f,  30.0f, 2, 255, 150, 210, 0.20f},
            { 0.42f,  58.0f, 1, 160, 255, 210, 0.22f},
            { 0.60f,  22.0f, 0, 255, 190, 120, 0.24f},
            { 0.82f,  40.0f, 2, 255, 235, 150, 0.17f},
            { 1.08f,  78.0f, 1, 255, 170, 120, 0.14f},
            { 1.30f,  34.0f, 0, 140, 180, 255, 0.16f},
            { 1.62f,  52.0f, 2, 140, 200, 255, 0.12f},
            { 2.05f,  96.0f, 1, 255, 150, 200, 0.09f},
    };
    SDL_Texture* texOf[3] = {m_glow, m_ring, m_hex};
    for (int tx = 0; tx < 3; ++tx) {
        for (const auto& g : ghosts) {
            if (g.tex != tx) continue;
            const float x = sx + dx * g.t, y = sy + dy * g.t;
            const float rad = g.rad * (0.80f + 0.45f * (1.0f - center));
            const float rot = (tx == 2) ? (axisAng + g.t * 0.8f) : 0.0f;
            if (x < -rad || x > v.w + rad || y < -rad || y > v.h + rad) continue;
            g_batch.quad(x, y, rad, rad, rot, tone(g.r, g.g, g.b, g.a * vis * farK));
        }
        g_batch.flush(r, texOf[tx]);
    }

    SDL_SetTextureBlendMode(m_glow, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(m_soft, SDL_BLENDMODE_BLEND);
}

float AmbienceFx::fogAmount() const {
    if (m_opt.fog >= 0.0f) return m_opt.fog;
    float f = 0.0f;
    if (m_mask & AMB_SNOW) f = 0.07f + clamp01(std::fabs(m_windVel) / 1000.0f) * 0.22f;   // a blizzard whites the view out
    if (m_mask & AMB_RAIN) f = std::max(f, (m_mask & AMB_THUNDER) ? 0.10f : 0.05f);
    return f;
}

// After Lighting::render: the screen is opaque here, so additive blending works, and nothing below darkens it.
void AmbienceFx::renderOverlay(SDL_Renderer* renderer) {
    if (!m_ready) return;
    const View v = getView();
    const SDL_FRect full{0.0f, 0.0f, v.w, v.h};

    // grey-blue weather tint
    float tint = 0.0f;
    if (m_mask & AMB_RAIN) tint = (m_mask & AMB_THUNDER) ? 0.20f : 0.11f;
    if (m_mask & AMB_SNOW) tint = std::max(tint, 0.05f);
    if (tint > 0.0f) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 28, 38, 60, (Uint8)(tint * 255.0f));
        SDL_RenderFillRect(renderer, &full);
    }

    // haze / fog: heavier near the ground, white in snow, grey in rain
    const float fog = fogAmount();
    if (fog > 0.005f) {
        uint8_t fr = 205, fg = 212, fb = 222;
        if (m_mask & AMB_SNOW)      { fr = 230; fg = 237; fb = 249; }
        else if (m_mask & AMB_RAIN) { fr = 120; fg = 134; fb = 160; }
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        g_batch.rectGrad(0.0f, 0.0f, v.w, v.h, col(fr, fg, fb, fog * 0.45f), col(fr, fg, fb, fog));
        g_batch.flush(renderer, nullptr);
    }

    // front fireflies light themselves again after the darkness pass, so they glow through dark levels
    if ((m_mask & AMB_FIREFLIES) && m_darkness > 0.12f && m_glow) {
        const float strength = clamp01((m_darkness - 0.12f) * 2.0f) * 0.55f;
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
        SDL_SetTextureBlendMode(m_glow, SDL_BLENDMODE_ADD);
        for (const auto& p : m_fireflies.front) {
            float sx, sy, a; fireflyScreen(p, true, v, sx, sy, a);
            const float d = p.size * 6.5f;
            if (sx < -d || sx > v.w + d || sy < -d || sy > v.h + d) continue;
            g_batch.quad(sx, sy, d * 0.5f, d * 0.5f, 0.0f, col(p.r, p.g, p.b, a * strength));
        }
        g_batch.flush(renderer, m_glow);
        SDL_SetTextureBlendMode(m_glow, SDL_BLENDMODE_BLEND);
    }

    // lightning flash
    if ((m_mask & AMB_THUNDER) && m_flashT < 0.9f) {
        const float f = flashAt(m_flashT);
        if (f > 0.01f) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
            SDL_SetRenderDrawColor(renderer, 170, 190, 255, (Uint8)(clamp01(f * 0.45f) * 255.0f));
            SDL_RenderFillRect(renderer, &full);
        }
    }

    if (m_mask & AMB_SUNFLARE) renderLensFlare(renderer, v);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}