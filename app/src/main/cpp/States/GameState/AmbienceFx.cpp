#include "AmbienceFx.h"
#include "utils/Camera.h"
#include <random>
#include <cmath>
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
    float wrap(float v, float size) { return v - std::floor(v / size) * size; }
    float smoothstep(float e0, float e1, float x) {
        float t = clamp01((x - e0) / (e1 - e0));
        return t * t * (3.0f - 2.0f * t);
    }
    SDL_FColor col(uint8_t r, uint8_t g, uint8_t b, float a) {
        return SDL_FColor{r / 255.0f, g / 255.0f, b / 255.0f, clamp01(a)};
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

    // lightning brightness over time since the strike began: ramp, dip, second hit, long decay
    float flashAt(float t) {
        if (t < 0.0f) return 0.0f;
        if (t < 0.05f) return t / 0.05f;
        if (t < 0.11f) return 1.0f - (t - 0.05f) / 0.06f * 0.75f;
        if (t < 0.16f) return 0.25f + (t - 0.11f) / 0.05f * 0.70f;
        return 0.95f * std::exp(-(t - 0.16f) * 5.5f);
    }

    uint32_t flagFor(const std::string& n) {
        if (n == "FIREFLIES" || n == "FIREFLY") return AMB_FIREFLIES;
        if (n == "FLIES" || n == "FLY")         return AMB_FLIES;
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

// What a biome gets when the level file has no 'Z' line. The level's darkness decides day vs. dusk:
//   bright day (<= 0.15)   sun flare
//   daylight   (<  0.30)   flies
//   dusk/night (>= 0.20)   fireflies   (0.20..0.30 has both)
uint32_t AmbienceFx::defaultsFor(AmbienceBiome biome, float darkness) {
    switch (biome) {
        case AmbienceBiome::FOREST:
        case AmbienceBiome::MEADOWS: {
            uint32_t m = AMB_WIND | (biome == AmbienceBiome::FOREST ? AMB_LEAVES : AMB_PETALS);
            if (darkness <= 0.15f) m |= AMB_SUNFLARE;
            if (darkness <  0.30f) m |= AMB_FLIES;
            if (darkness >= 0.20f) m |= AMB_FIREFLIES;
            return m;
        }
        case AmbienceBiome::STONE_CAVE:  return AMB_FLIES;       // sparse gnats, the motes already cover the dust
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
        const bool remove = (t[0] == '-');
        const uint32_t f = flagFor(remove ? t.substr(1) : t);
        if (remove) m &= ~f; else m |= f;
    }
    // no sun through rain/snow clouds, and no insects in the wet
    if (m & (AMB_RAIN | AMB_SNOW)) m &= ~(AMB_SUNFLARE | AMB_FLIES);
    return m;
}

// ---------------------------------------------------------------------------------------------
// setup
// ---------------------------------------------------------------------------------------------
AmbienceFx::~AmbienceFx() { destroySprites(); }

void AmbienceFx::destroySprites() {
    SDL_Texture** all[] = {&m_glow, &m_soft, &m_dot, &m_petal, &m_leaf, &m_ring, &m_hex};
    for (auto** t : all) {
        if (*t) { SDL_DestroyTexture(*t); *t = nullptr; }
    }
}

AmbienceFx::View AmbienceFx::getView() const {
    const SDL_FRect cam = Camera::getInstance().getCamera();
    return View{cam.w > 0 ? cam.w : 1600.0f, cam.h > 0 ? cam.h : 720.0f, cam.x, cam.y};
}

int AmbienceFx::count(int base) const {
    return std::max(1, (int)std::lround(base * m_density));
}

void AmbienceFx::init(SDL_Renderer* renderer, AmbienceBiome biome, float darkness,
                      const std::vector<std::string>& tokens) {
    m_biome = biome;
    m_darkness = darkness;
    m_mask = resolve(biome, darkness, tokens);
    m_ready = false;
    m_t = 0.0f;

    for (Group* g : {&m_fireflies, &m_flies, &m_petals, &m_leaves, &m_snow, &m_rain, &m_streaks}) {
        g->back.clear(); g->front.clear();
    }
    m_windVel = 0.0f; m_gust = 0.0f; m_gustDur = 0.0f; m_gustAge = 0.0f;
    m_nextGust = rnd(2.0f, 5.0f);
    m_windScale = (m_mask & AMB_THUNDER) ? 2.0f : ((m_mask & AMB_RAIN) ? 1.4f : 1.0f);
    m_nextStrike = rnd(2.5f, 6.0f);
    m_flashT = 99.0f; m_thunderDelay = -1.0f; m_thunderReady = false;
    m_bolt.clear(); m_branches.clear();

    if (m_mask == 0 || !renderer) return;

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

    buildAll();
    m_ready = true;
}

void AmbienceFx::buildAll() {
    if (m_mask & AMB_FIREFLIES) buildFireflies();
    if (m_mask & AMB_FLIES)     buildFlies();
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

void AmbienceFx::buildFireflies() {
    const View v = getView();
    const bool cave = (m_biome == AmbienceBiome::BRIGHT_CAVE);
    const float night = cave ? 1.0f : clamp01((m_darkness - 0.12f) / 0.35f);
    static const uint8_t forest[3][3] = {{190, 255, 70}, {225, 255, 110}, {255, 235, 120}};
    static const uint8_t blue[3][3]   = {{90, 180, 255}, {130, 220, 255}, {170, 235, 255}};
    const auto& pal = cave ? blue : forest;

    auto make = [&](std::vector<P>& layer, int n, bool front) {
        const float m = front ? FRONT_MARGIN : BACK_MARGIN;
        fillLayer(layer, n, v.w + m * 2, v.h + m * 2, [&](P& p, float t) {
            p.parallax = front ? lerp(1.15f, 1.6f, t) : lerp(0.40f, 0.85f, t);
            p.size     = front ? lerp(9.0f, 15.0f, t) : lerp(5.0f, 9.0f, t);
            p.alpha    = (front ? lerp(0.45f, 0.80f, t) : lerp(0.75f, 1.0f, t)) * (0.7f + 0.3f * night);
            p.ax = rnd(25.0f, 70.0f) * (front ? 1.6f : 1.0f);
            p.ay = p.ax * rnd(0.5f, 0.9f);
            p.fx = rnd(0.25f, 0.60f);
            p.fy = rnd(0.30f, 0.70f);
            p.period = rnd(1.8f, 4.2f);
            p.drift = rnd(-3.0f, 3.0f);
            p.fall = rnd(-5.0f, 2.0f);
            p.influence = 0.3f;
            const uint8_t* c = pal[std::uniform_int_distribution<int>(0, 2)(g_rng)];
            p.r = c[0]; p.g = c[1]; p.b = c[2];
        });
    };
    make(m_fireflies.back,  count(7 + (int)(11 * night)), false);
    make(m_fireflies.front, count(2 + (int)(3 * night)), true);
}

void AmbienceFx::buildFlies() {
    const View v = getView();
    const bool cave = (m_biome == AmbienceBiome::STONE_CAVE || m_biome == AmbienceBiome::BRIGHT_CAVE);
    const int backN  = cave ? 4 : (m_biome == AmbienceBiome::MEADOWS ? 8 : 6);
    const int frontN = cave ? 1 : 3;

    auto make = [&](std::vector<P>& layer, int n, bool front) {
        const float m = front ? FRONT_MARGIN : BACK_MARGIN;
        fillLayer(layer, n, v.w + m * 2, v.h + m * 2, [&](P& p, float t) {
            p.parallax = front ? lerp(1.2f, 1.6f, t) : lerp(0.5f, 0.9f, t);
            p.size  = front ? lerp(6.0f, 8.0f, t) : lerp(3.5f, 5.0f, t);
            p.alpha = front ? 0.95f : 0.85f;
            p.drift = rnd(-4.0f, 4.0f);
            p.influence = 0.25f;
            p.timer = rnd(0.0f, 0.4f);
            if (cave) { p.r = 165; p.g = 155; p.b = 138; p.alpha *= 0.55f; }   // pale gnats, dark ones vanish in a cave
            else      { p.r = 40;  p.g = 34;  p.b = 30; }
        });
    };
    make(m_flies.back,  count(backN), false);
    make(m_flies.front, count(frontN), true);
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
    make(g.back,  count(leaf ? 14 : 22), false);
    make(g.front, count(leaf ? 5 : 7), true);
}

void AmbienceFx::buildSnow() {
    const View v = getView();
    auto make = [&](std::vector<P>& layer, int n, bool front) {
        const float m = front ? FRONT_MARGIN : BACK_MARGIN;
        fillLayer(layer, n, v.w + m * 2, v.h + m * 2, [&](P& p, float t) {
            p.parallax = front ? lerp(1.15f, 1.70f, t) : lerp(0.35f, 0.85f, t);
            p.size  = front ? lerp(12.0f, 26.0f, t) : lerp(3.5f, 6.5f, t);     // front = out-of-focus bokeh
            p.alpha = front ? lerp(0.28f, 0.55f, t) : lerp(0.70f, 1.0f, t);
            p.fall  = front ? lerp(60.0f, 120.0f, t) : lerp(22.0f, 45.0f, t);
            p.swayAmp  = rnd(8.0f, 26.0f) * (front ? 1.6f : 1.0f);
            p.swayFreq = rnd(0.5f, 1.4f);
            p.drift = rnd(-4.0f, 4.0f);
            p.influence = front ? 1.0f : 0.7f;
            p.r = 232; p.g = 240; p.b = 255;
        });
    };
    make(m_snow.back,  count(80), false);
    make(m_snow.front, count(18), true);
}

void AmbienceFx::buildRain() {
    const View v = getView();
    const bool heavy = (m_mask & AMB_THUNDER) != 0;
    auto make = [&](std::vector<P>& layer, int n, bool front) {
        const float m = front ? FRONT_MARGIN : BACK_MARGIN;
        fillLayer(layer, n, v.w + m * 2, v.h + m * 2, [&](P& p, float t) {
            p.parallax = front ? lerp(1.2f, 1.7f, t) : lerp(0.5f, 0.9f, t);
            p.fall  = front ? lerp(1100.0f, 1500.0f, t) : lerp(700.0f, 950.0f, t);
            p.len   = front ? lerp(28.0f, 52.0f, t) : lerp(12.0f, 22.0f, t);
            p.size  = front ? lerp(1.6f, 2.4f, t) : lerp(1.0f, 1.4f, t);      // line width
            p.alpha = front ? lerp(0.40f, 0.65f, t) : lerp(0.30f, 0.50f, t);
            p.influence = 0.55f;
            p.r = 175; p.g = 195; p.b = 225;
        });
    };
    make(m_rain.back,  count(heavy ? 90 : 60), false);
    make(m_rain.front, count(heavy ? 30 : 18), true);
}

void AmbienceFx::buildStreaks() {
    const View v = getView();
    auto make = [&](std::vector<P>& layer, int n, bool front) {
        const float m = front ? FRONT_MARGIN : BACK_MARGIN;
        fillLayer(layer, n, v.w + m * 2, v.h + m * 2, [&](P& p, float t) {
            p.parallax = front ? lerp(1.2f, 1.7f, t) : lerp(0.5f, 0.9f, t);
            p.len   = front ? lerp(180.0f, 420.0f, t) : lerp(110.0f, 280.0f, t);
            p.size  = front ? lerp(2.0f, 3.5f, t) : lerp(1.2f, 2.0f, t);
            p.alpha = front ? lerp(0.10f, 0.18f, t) : lerp(0.12f, 0.22f, t);
            p.rot   = rnd(-0.05f, 0.08f);       // slight slope
            p.influence = 2.0f;                 // streaks outrun the wind a bit
            p.r = 235; p.g = 245; p.b = 255;
        });
    };
    make(m_streaks.back,  count(7), false);
    make(m_streaks.front, count(4), true);
}

// ---------------------------------------------------------------------------------------------
// simulation
// ---------------------------------------------------------------------------------------------
void AmbienceFx::updateWind(float dt) {
    const bool gusts = (m_mask & AMB_WIND) != 0;
    const float base = (std::sin(m_t * 0.17f + 1.0f) * 0.55f + std::sin(m_t * 0.43f + 2.1f) * 0.25f) * 40.0f * m_windScale;

    if (gusts) {
        if (m_gustDur > 0.0f) {
            m_gustAge += dt;
            if (m_gustAge >= m_gustDur) {
                m_gustDur = 0.0f; m_gust = 0.0f;
                m_nextGust = rnd(6.0f, 14.0f) / (m_windScale > 1.5f ? 1.8f : 1.0f);
            } else {
                float s = std::sin(PI * m_gustAge / m_gustDur);
                m_gust = m_gustPeak * s * s;   // smooth swell and fade
            }
        } else {
            m_nextGust -= dt;
            if (m_nextGust <= 0.0f) {
                m_gustDur = rnd(2.5f, 4.5f);
                m_gustAge = 0.0f;
                m_gustPeak = rnd(0.6f, 1.0f);
                m_gustDir = rnd(0.0f, 1.0f) < 0.65f ? 1.0f : -1.0f;
            }
        }
    } else {
        m_gust = 0.0f;
    }
    m_windVel = base * (gusts ? 1.0f : 0.5f) + m_gustDir * m_gust * 260.0f * m_windScale;
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
    m_boltParallaxX = rnd(0.12f, 0.88f) * v.w + v.camX * 0.5f;
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
        m_thunderDelay = rnd(0.35f, 1.5f);    // light first, sound after
        m_thunderStrength = rnd(0.5f, 1.0f);
        m_nextStrike = rnd(6.0f, 15.0f);
    }
    if (m_thunderDelay >= 0.0f) {
        m_thunderDelay -= dt;
        if (m_thunderDelay < 0.0f) { m_thunderReady = true; }
    }
}

bool AmbienceFx::consumeThunder(float& strength) {
    if (!m_thunderReady) return false;
    m_thunderReady = false;
    strength = m_thunderStrength;
    return true;
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

    for (int pass = 0; pass < 2; ++pass) {
        const bool front = (pass == 1);
        float dw, dh;
        dom(front, dw, dh);

        // fireflies: the home point drifts, the lissajous wander is added at render time
        for (auto& p : (front ? m_fireflies.front : m_fireflies.back)) {
            p.x = wrap(p.x + (p.drift + m_windVel * p.influence * p.parallax) * dt, dw);
            p.y = wrap(p.y + p.fall * dt, dh);
        }

        // flies: steer to a random velocity every ~0.1-0.45 s (sometimes hover), pulled back to the home point
        for (auto& p : (front ? m_flies.front : m_flies.back)) {
            p.timer -= dt;
            if (p.timer <= 0.0f) {
                if (rnd(0.0f, 1.0f) < 0.2f) { p.tvx = p.tvy = 0.0f; p.timer = rnd(0.3f, 0.9f); }
                else {
                    const float a = rnd(0.0f, TWO_PI), sp = rnd(70.0f, 170.0f) * (front ? 1.4f : 1.0f);
                    p.tvx = std::cos(a) * sp; p.tvy = std::sin(a) * sp;
                    p.timer = rnd(0.10f, 0.45f);
                }
            }
            const float tx = p.tvx - p.ox * 1.5f, ty = p.tvy - p.oy * 1.5f;
            const float s = std::min(1.0f, dt * 9.0f);
            p.vx += (tx - p.vx) * s;
            p.vy += (ty - p.vy) * s;
            p.ox += p.vx * dt;
            p.oy += p.vy * dt;
            p.x = wrap(p.x + (p.drift + m_windVel * p.influence * p.parallax) * dt, dw);
        }

        // petals / leaves / snow: fall, lean into the wind with a lag, lift in gusts
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
        drift(front ? m_snow.front   : m_snow.back,   25.0f, false);

        // rain: straight down with a wind slant
        for (auto& p : (front ? m_rain.front : m_rain.back)) {
            p.vx += (m_windVel * p.influence - p.vx) * std::min(1.0f, dt * 4.0f);
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
    // blink: short bright pulse, faint glow in between
    const float s = std::sin(m_t * TWO_PI / p.period + p.phase);
    a = p.alpha * (0.15f + 0.85f * std::pow(std::max(0.0f, s), 2.0f));
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

    // wind streaks (untextured, faded at both ends, a little bent)
    if (m_mask & AMB_WIND) {
        const float vis = clamp01(m_gust * 1.2f + std::min(0.25f, std::fabs(m_windVel) / 400.0f));
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

    // rain: slanted streak, bright head / transparent tail
    if (m_mask & AMB_RAIN) {
        for (const auto& p : (front ? m_rain.front : m_rain.back)) {
            float sx, sy; scr(p, 0, 0, sx, sy);
            if (!visible(sx, sy, p.len)) continue;
            const float l = std::sqrt(p.vx * p.vx + p.fall * p.fall);
            const float tx = sx - p.vx / l * p.len, ty = sy - p.fall / l * p.len;
            g_batch.line(tx, ty, sx, sy, p.size, col(p.r, p.g, p.b, 0.0f), col(p.r, p.g, p.b, p.alpha));
        }
        g_batch.flush(renderer, nullptr);
    }

    // snow: crisp dots behind, soft bokeh in front
    if (m_mask & AMB_SNOW) {
        SDL_Texture* tex = front ? m_soft : m_dot;
        for (const auto& p : (front ? m_snow.front : m_snow.back)) {
            const float sway = std::sin(m_t * p.swayFreq + p.phase) * p.swayAmp;
            float sx, sy; scr(p, sway, 0, sx, sy);
            if (!visible(sx, sy, p.size)) continue;
            g_batch.quad(sx, sy, p.size * 0.5f, p.size * 0.5f, 0.0f, col(p.r, p.g, p.b, p.alpha));
        }
        g_batch.flush(renderer, tex);
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

    // flies: a dark body plus a flickering wing blur
    if (m_mask & AMB_FLIES) {
        for (const auto& p : (front ? m_flies.front : m_flies.back)) {
            float sx, sy; scr(p, p.ox, p.oy, sx, sy);
            if (!visible(sx, sy, p.size * 3.0f)) continue;
            g_batch.quad(sx, sy, p.size * 0.5f, p.size * 0.5f, 0.0f, col(p.r, p.g, p.b, p.alpha));
            const float wing = 0.5f + 0.5f * std::sin(m_t * 90.0f + p.phase * 7.0f);
            g_batch.quad(sx, sy - p.size * 0.7f, p.size * 1.1f, p.size * 0.55f, 0.0f, col(225, 225, 235, p.alpha * (0.15f + 0.30f * wing)));
        }
        g_batch.flush(renderer, m_dot);
    }

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
    renderLayer(renderer, false);
}

void AmbienceFx::renderFront(SDL_Renderer* renderer) {
    if (!m_ready) return;
    renderLayer(renderer, true);
}

void AmbienceFx::renderLensFlare(SDL_Renderer* r, const View& v) {
    float vis = clamp01(1.0f - m_darkness * 3.0f);
    vis *= 0.90f + 0.10f * std::sin(m_t * 0.7f);
    if (vis < 0.02f || !m_glow || !m_ring || !m_hex) return;

    // the sun is nearly at infinity: it only creeps against the camera, which slides the ghosts along the axis
    const float sx = v.w * 0.80f - v.camX * 0.03f;
    const float sy = v.h * 0.15f - v.camY * 0.02f;
    const float dx = v.w * 0.5f - sx, dy = v.h * 0.5f - sy;   // sun -> screen centre axis
    auto at = [&](float t, float& x, float& y) { x = sx + dx * t; y = sy + dy * t; };

    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_ADD);
    SDL_SetTextureBlendMode(m_glow, SDL_BLENDMODE_ADD);

    float x, y;
    // glow batch: big halo, hot core, horizontal anamorphic streak, two soft ghosts
    g_batch.quad(sx, sy, 260, 260, 0, col(255, 225, 160, 0.32f * vis));
    g_batch.quad(sx, sy, 80, 80, 0, col(255, 248, 225, 0.75f * vis));
    g_batch.quad(sx, sy, 520, 14, 0, col(255, 235, 190, 0.28f * vis));
    at(0.55f, x, y); g_batch.quad(x, y, 30, 30, 0, col(255, 190, 120, 0.20f * vis));
    at(1.35f, x, y); g_batch.quad(x, y, 45, 45, 0, col(130, 170, 255, 0.12f * vis));
    g_batch.flush(r, m_glow);

    at(0.90f, x, y); g_batch.quad(x, y, 55, 55, 0, col(160, 255, 210, 0.22f * vis));
    at(1.70f, x, y); g_batch.quad(x, y, 100, 100, 0, col(255, 180, 130, 0.12f * vis));
    g_batch.flush(r, m_ring);

    at(0.30f, x, y); g_batch.quad(x, y, 23, 23, 0.3f, col(255, 140, 200, 0.22f * vis));
    at(1.10f, x, y); g_batch.quad(x, y, 35, 35, 0.0f, col(255, 240, 160, 0.16f * vis));
    at(2.00f, x, y); g_batch.quad(x, y, 60, 60, 0.5f, col(140, 190, 255, 0.10f * vis));
    g_batch.flush(r, m_hex);

    SDL_SetTextureBlendMode(m_glow, SDL_BLENDMODE_BLEND);
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
