#include "ParryFx.h"
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

void ParryFx::update(float realDt) {
    for (auto& f : m_flashes) f.age += realDt;
    m_flashes.erase(std::remove_if(m_flashes.begin(), m_flashes.end(),
                                   [](const Flash& f) { return f.age >= f.life; }), m_flashes.end());
}

void ParryFx::clear() {
    m_flashes.clear();
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

    if (!m_verts.empty()) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
        SDL_RenderGeometry(renderer, nullptr, m_verts.data(), (int)m_verts.size(), m_idx.data(), (int)m_idx.size());
    }

    SDL_SetRenderDrawBlendMode(renderer, prev);
}
