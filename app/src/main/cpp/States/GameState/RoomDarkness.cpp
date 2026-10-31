#include "RoomDarkness.h"
#include <algorithm>
#include <cmath>

namespace {
    float clamp01(float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }
    float smooth(float t) { t = clamp01(t); return t * t * (3.0f - 2.0f * t); }

    constexpr float HIDE_SLOWER = 1.4f;    // the darkness comes back a little slower than it melts
}

void RoomDarkness::init(const std::vector<DarkRoom>& rooms, AmbienceBiome biome) {
    m_rooms.clear();
    m_first = true;
    const bool closed = (biome == AmbienceBiome::STONE_CAVE || biome == AmbienceBiome::BRIGHT_CAVE);
    for (const DarkRoom& r : rooms) {
        if (r.w <= 0.0f || r.h <= 0.0f) continue;
        if (!closed && !r.force) continue;     // forest / meadows / peaks: open air, no rooms
        Room room;
        room.def = r;
        m_rooms.push_back(room);
    }
}

void RoomDarkness::update(float dt, float px, float py) {
    dt = std::min(dt, 0.05f);
    for (Room& r : m_rooms) {
        const DarkRoom& d = r.def;
        const bool inside = px >= d.x && px <= d.x + d.w && py >= d.y && py <= d.y + d.h;
        if (inside) r.visited = true;
        const float target = (inside || (d.keep && r.visited)) ? 1.0f : 0.0f;
        const float fade = std::max(0.05f, d.fade);

        if (m_first) { r.reveal = target; continue; }
        if (r.reveal < target)      r.reveal = std::min(target, r.reveal + dt / fade);
        else if (r.reveal > target) r.reveal = std::max(target, r.reveal - dt / (fade * HIDE_SLOWER));
    }
    m_first = false;
}

float RoomDarkness::coverAt(float wx, float wy) const {
    float cover = 0.0f;
    for (const Room& r : m_rooms) {
        const DarkRoom& d = r.def;
        if (wx < d.x || wx > d.x + d.w || wy < d.y || wy > d.y + d.h) continue;
        cover = std::max(cover, (1.0f - smooth(r.reveal)) * d.opacity);
    }
    return cover;
}

void RoomDarkness::render(SDL_Renderer* renderer, float camX, float camY) const {
    if (!renderer || m_rooms.empty()) return;
    constexpr float SCREEN_W = 1600.0f, SCREEN_H = 720.0f;
    constexpr int STEPS = 6;                 // gradient slices per side (eased, so the grey looks smooth)
    constexpr int N = 2 * (STEPS + 1);       // grid points per axis

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    for (const Room& r : m_rooms) {
        const float a = (1.0f - smooth(r.reveal)) * clamp01(r.def.opacity);
        if (a < 0.004f) continue;

        const float x = r.def.x - camX, y = r.def.y - camY, w = r.def.w, h = r.def.h;
        // the gradient is centred on the room border: half of it inside the room, half outside
        const float f = std::max(2.0f, std::min(r.def.soft, std::min(w, h) * 0.45f));
        if (x + w + f < 0.0f || x - f > SCREEN_W || y + h + f < 0.0f || y - f > SCREEN_H) continue;

        // per axis: positions + 0..1 coverage. Left ramp (clear -> dark), then the right ramp (dark -> clear).
        float xs[N], ys[N], cx[N], cy[N];
        for (int k = 0; k <= STEPS; ++k) {
            const float t = (float)k / (float)STEPS;       // 0..1 across a ramp (2f wide)
            const float e = smooth(t);
            xs[k]            = x - f + 2.0f * f * t;      cx[k]            = e;
            xs[N - 1 - k]    = x + w + f - 2.0f * f * t;  cx[N - 1 - k]    = e;
            ys[k]            = y - f + 2.0f * f * t;      cy[k]            = e;
            ys[N - 1 - k]    = y + h + f - 2.0f * f * t;  cy[N - 1 - k]    = e;
        }
        SDL_Vertex v[N * N];
        for (int j = 0; j < N; ++j) {
            for (int i = 0; i < N; ++i) {
                SDL_Vertex& vt = v[j * N + i];
                vt.position = SDL_FPoint{xs[i], ys[j]};
                vt.color = SDL_FColor{0.0f, 0.0f, 0.0f, a * cx[i] * cy[j]};
                vt.tex_coord = SDL_FPoint{0.0f, 0.0f};
            }
        }
        int idx[(N - 1) * (N - 1) * 6];
        int n = 0;
        for (int j = 0; j < N - 1; ++j) {
            for (int i = 0; i < N - 1; ++i) {
                const int a0 = j * N + i, b0 = a0 + 1, c0 = a0 + N + 1, d0 = a0 + N;
                idx[n++] = a0; idx[n++] = b0; idx[n++] = c0;
                idx[n++] = a0; idx[n++] = c0; idx[n++] = d0;
            }
        }
        SDL_RenderGeometry(renderer, nullptr, v, N * N, idx, n);
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}
