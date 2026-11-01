#include "ParryFx.h"
#include "utils/Parry.h"
#include <cmath>
#include <algorithm>

namespace {
    constexpr float PI_F = 3.14159265f;

    inline float easeOut(float t) { const float u = 1.0f - t; return 1.0f - u * u * u; }
    inline float frand(unsigned int& s) {            // tiny deterministic LCG: every flash has its own streak pattern
        s = s * 1664525u + 1013904223u;
        return (float)((s >> 8) & 0xFFFFu) / 65535.0f;
    }

    struct Geo {
        std::vector<SDL_Vertex>& v;
        std::vector<int>& i;
        void quad(SDL_FPoint a, SDL_FPoint b, SDL_FPoint c, SDL_FPoint d,
                  SDL_FColor ca, SDL_FColor cb, SDL_FColor cc, SDL_FColor cd) {
            const int base = (int)v.size();
            v.push_back({a, ca, {0, 0}});
            v.push_back({b, cb, {0, 0}});
            v.push_back({c, cc, {0, 0}});
            v.push_back({d, cd, {0, 0}});
            const int q[6] = {0, 1, 2, 0, 2, 3};
            for (int k : q) i.push_back(base + k);
        }
        // a tapered streak: wide at the base (r0), a point at the tip (r1), along `angle`
        void streak(float x, float y, float angle, float r0, float r1, float width, SDL_FColor c) {
            const float dx = std::cos(angle), dy = std::sin(angle);
            const float nx = -dy, ny = dx;
            const SDL_FColor tip{c.r, c.g, c.b, 0.0f};
            const SDL_FPoint p0a{x + dx * r0 + nx * width * 0.5f, y + dy * r0 + ny * width * 0.5f};
            const SDL_FPoint p0b{x + dx * r0 - nx * width * 0.5f, y + dy * r0 - ny * width * 0.5f};
            const SDL_FPoint p1{x + dx * r1, y + dy * r1};
            const int base = (int)v.size();
            v.push_back({p0a, c, {0, 0}});
            v.push_back({p0b, c, {0, 0}});
            v.push_back({p1, tip, {0, 0}});
            i.push_back(base); i.push_back(base + 1); i.push_back(base + 2);
        }
        // an arc band: centre (x, y), radius r, thickness th, from a0 to a1 (radians); soft (transparent) edges
        void arc(float x, float y, float r, float th, float a0, float a1, SDL_FColor core, int seg) {
            const SDL_FColor edge{core.r, core.g, core.b, 0.0f};
            for (int s = 0; s < seg; ++s) {
                const float t0 = (float)s / (float)seg, t1 = (float)(s + 1) / (float)seg;
                const float A0 = a0 + (a1 - a0) * t0, A1 = a0 + (a1 - a0) * t1;
                // taper the alpha toward both ends of the arc
                const float f0 = std::sin(t0 * PI_F), f1 = std::sin(t1 * PI_F);
                SDL_FColor c0 = core, c1 = core; c0.a *= f0; c1.a *= f1;
                SDL_FColor e0 = edge, e1 = edge;
                const float ri = r - th * 0.5f, ro = r + th * 0.5f;
                const SDL_FPoint i0{x + std::cos(A0) * ri, y + std::sin(A0) * ri};
                const SDL_FPoint i1{x + std::cos(A1) * ri, y + std::sin(A1) * ri};
                const SDL_FPoint o0{x + std::cos(A0) * ro, y + std::sin(A0) * ro};
                const SDL_FPoint o1{x + std::cos(A1) * ro, y + std::sin(A1) * ro};
                const SDL_FPoint m0{x + std::cos(A0) * r,  y + std::sin(A0) * r};
                const SDL_FPoint m1{x + std::cos(A1) * r,  y + std::sin(A1) * r};
                // inner half and outer half meet in a bright centre line
                quad(i0, i1, m1, m0, e0, e1, c1, c0);
                quad(m0, m1, o1, o0, c0, c1, e1, e0);
            }
        }
        // a full ring, soft on both sides
        void ring(float x, float y, float r, float th, SDL_FColor c) { arc(x, y, r, th, 0.0f, 2.0f * PI_F, c, 28); }
    };
}

void ParryFx::spawn(float wx, float wy, float dirX, bool big) {
    Flash f;
    f.x = wx; f.y = wy;
    f.dir = dirX >= 0.0f ? 1.0f : -1.0f;
    f.scale = big ? 1.6f : 1.0f;
    f.life  = big ? 0.32f : 0.24f;
    f.seed  = (unsigned int)(wx * 31.0f + wy * 17.0f) + (unsigned int)m_flashes.size() * 977u + 13u;
    m_flashes.push_back(f);
    if (m_flashes.size() > 6) m_flashes.erase(m_flashes.begin());   // never pile up
}

void ParryFx::spawnTipSpark(float wx, float wy) {
    TipSpark s;
    s.x = wx; s.y = wy;
    s.seed = (unsigned int)(wx * 13.0f + wy * 29.0f) + (unsigned int)m_tipSparks.size() * 631u + 7u;
    m_tipSparks.push_back(s);
    if (m_tipSparks.size() > 4) m_tipSparks.erase(m_tipSparks.begin());   // never pile up (a combo parries one strike at a time)
}

void ParryFx::holdAtPeak() {
    for (auto& f : m_flashes)    f.age = std::max(f.age, f.life * 0.12f);
    for (auto& s : m_tipSparks)  s.age = std::max(s.age, PARRY_SPARK_LIFE_S * 0.18f);   // the spark is at full width from here
    m_held = true;
}

void ParryFx::update(float realDt) {
    if (m_held) return;     // the parry freeze-frame: hold the pose
    for (auto& f : m_flashes) f.age += realDt;
    m_flashes.erase(std::remove_if(m_flashes.begin(), m_flashes.end(),
                                   [](const Flash& f) { return f.age >= f.life; }), m_flashes.end());
    for (auto& s : m_tipSparks) s.age += realDt;
    m_tipSparks.erase(std::remove_if(m_tipSparks.begin(), m_tipSparks.end(),
                                     [](const TipSpark& s) { return s.age >= PARRY_SPARK_LIFE_S; }), m_tipSparks.end());
}

void ParryFx::clear() {
    m_held = false;
    m_flashes.clear();
    m_tipSparks.clear();
}

void ParryFx::render(SDL_Renderer* renderer, float camX, float camY) const {
    if (!busy() || !renderer) return;

    SDL_BlendMode prev = SDL_BLENDMODE_NONE;
    SDL_GetRenderDrawBlendMode(renderer, &prev);

    // ---- additive pass: contact flashes ----
    m_verts.clear(); m_idx.clear();
    Geo g{m_verts, m_idx};

    for (const Flash& f : m_flashes) {
        const float t = std::clamp(f.age / f.life, 0.0f, 1.0f);
        const float e = easeOut(t);
        const float a = 1.0f - t;
        const float sx = f.x - camX, sy = f.y - camY;
        const float s = f.scale;

        // horizontal flare through the contact point (the "ting" line), then a vertical cross arm
        const float flare = (170.0f * s) * (1.0f - t * t) + 20.0f * s;
        g.streak(sx, sy, 0.0f,    0.0f, flare,        14.0f * s * a + 2.0f, SDL_FColor{1.0f, 1.0f, 1.0f, a});
        g.streak(sx, sy, PI_F,    0.0f, flare,        14.0f * s * a + 2.0f, SDL_FColor{1.0f, 1.0f, 1.0f, a});
        g.streak(sx, sy, -PI_F * 0.5f, 0.0f, flare * 0.45f, 9.0f * s * a + 1.0f, SDL_FColor{1.0f, 0.96f, 0.80f, a * 0.8f});
        g.streak(sx, sy,  PI_F * 0.5f, 0.0f, flare * 0.45f, 9.0f * s * a + 1.0f, SDL_FColor{1.0f, 0.96f, 0.80f, a * 0.8f});

        // expanding ring (warm), thinning as it grows
        g.ring(sx, sy, 16.0f * s + 86.0f * s * e, 12.0f * s * a + 2.0f, SDL_FColor{1.0f, 0.90f, 0.62f, 0.85f * a});

        // streaks thrown back toward the attacker, fanned around the contact direction
        unsigned int seed = f.seed;
        const float baseA = f.dir > 0.0f ? 0.0f : PI_F;
        for (int k = 0; k < 9; ++k) {
            const float ang = baseA + (frand(seed) - 0.5f) * 2.4f;
            const float len = (46.0f + frand(seed) * 90.0f) * s;
            const float r0 = (16.0f + 70.0f * e) * s * (0.6f + 0.4f * frand(seed));
            g.streak(sx, sy, ang, r0, r0 + len * (1.0f - t * 0.6f), 5.5f * s * a + 1.0f, SDL_FColor{0.80f, 0.93f, 1.0f, 0.9f * a});
        }

        // hot core
        const float cr = (30.0f * s) * (1.0f - t) + 6.0f;
        g.ring(sx, sy, cr * 0.5f, cr, SDL_FColor{1.0f, 1.0f, 1.0f, a});
    }

    // ---- the tip spark: white, PARRY_SPARK_WIDTH wide in total (half arm each way), snaps to full width then thins out ----
    for (const TipSpark& sp : m_tipSparks) {
        const float t    = std::clamp(sp.age / PARRY_SPARK_LIFE_S, 0.0f, 1.0f);
        const float grow = std::clamp(t / 0.18f, 0.0f, 1.0f);         // full width within the first ~18% of its life
        const float reach = (PARRY_SPARK_WIDTH * 0.5f) * easeOut(grow);
        const float fade = 1.0f - t;
        const float a    = fade * fade * (3.0f - 2.0f * fade);        // smooth fade-out
        const float sx = sp.x - camX, sy = sp.y - camY;
        const SDL_FColor white{1.0f, 1.0f, 1.0f, a};
        const SDL_FColor haze{1.0f, 1.0f, 1.0f, a * 0.45f};

        // soft wide haze under a thin hard blade: two layers per side read as a glowing line
        const float bladeW = 5.0f + 26.0f * fade * fade;
        for (int side = 0; side < 2; ++side) {
            const float ang = side == 0 ? 0.0f : PI_F;
            g.streak(sx, sy, ang, 0.0f, reach,        bladeW * 2.4f, haze);
            g.streak(sx, sy, ang, 0.0f, reach,        bladeW,        white);
            g.streak(sx, sy, ang, 0.0f, reach * 0.55f, bladeW * 0.45f, white);
        }

        // a short vertical arm and a few white splinters thrown off the contact point
        g.streak(sx, sy, -PI_F * 0.5f, 0.0f, reach * 0.16f, bladeW * 0.9f, white);
        g.streak(sx, sy,  PI_F * 0.5f, 0.0f, reach * 0.16f, bladeW * 0.9f, white);
        unsigned int seed = sp.seed;
        for (int k = 0; k < 6; ++k) {
            const float ang  = (frand(seed) - 0.5f) * 2.2f + (k & 1 ? PI_F : 0.0f);
            const float len  = (60.0f + frand(seed) * 150.0f) * (0.4f + 0.6f * grow);
            g.streak(sx, sy, ang, 6.0f, 6.0f + len, 3.0f + 4.0f * fade, SDL_FColor{1.0f, 1.0f, 1.0f, a * 0.85f});
        }

        // hot white core on the contact point itself
        const float cr = 54.0f * fade + 10.0f;
        g.ring(sx, sy, cr * 0.5f, cr, white);
    }

    if (!m_verts.empty()) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
        SDL_RenderGeometry(renderer, nullptr, m_verts.data(), (int)m_verts.size(), m_idx.data(), (int)m_idx.size());
    }

    SDL_SetRenderDrawBlendMode(renderer, prev);
}
