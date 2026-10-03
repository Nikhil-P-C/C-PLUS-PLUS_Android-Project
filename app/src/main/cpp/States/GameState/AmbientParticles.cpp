#include "AmbientParticles.h"
#include "utils/Camera.h"
#include <random>
#include <cmath>
#include <algorithm>

namespace {
    std::mt19937 g_rng(std::random_device{}());

    float rnd(float a, float b) {
        std::uniform_real_distribution<float> d(a, b);
        return d(g_rng);
    }
    float lerp(float a, float b, float t) { return a + (b - a) * t; }

    // floor-based modulo so negative values wrap correctly
    float wrap(float v, float size) { return v - std::floor(v / size) * size; }

    struct ThemeStyle {
        uint8_t palette[4][3];
        int paletteCount;
        float speedMul;     // how fast motes rise
        float sizeMul;
        float swayMul;      // fireflies wander, dust barely moves
        float alphaMul;
        float twinkleMul;
        float backCount;    // multiplier on the base mote count
    };

    ThemeStyle styleFor(AmbientTheme t) {
        switch (t) {
            case AmbientTheme::GREENERY:
                return {{{150, 255, 90}, {225, 255, 110}, {255, 235, 110}, {120, 230, 100}}, 4,
                        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
            case AmbientTheme::STONE_CAVE:
                return {{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, 4,
                        0.9f, 1.0f, 0.7f, 1.0f, 0.8f, 1.0f};
            case AmbientTheme::BRIGHT_CAVE:
                return {{{90, 170, 255}, {120, 210, 255}, {170, 230, 255}, {70, 140, 255}}, 4,
                        0.9f, 1.0f, 0.7f, 1.0f, 0.8f, 1.0f};
            default:
                return {{{255, 255, 255}}, 1, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
        }
    }

    // Base counts, kept low on purpose (the C30s is the validation device).
    constexpr int BACK_COUNT  = 36;
    constexpr int FRONT_COUNT = 12;
    constexpr float BACK_MARGIN  = 40.0f;
    constexpr float FRONT_MARGIN = 80.0f;
}

AmbientParticles::~AmbientParticles() {
    if (m_sharpSprite) SDL_DestroyTexture(m_sharpSprite);
    if (m_softSprite)  SDL_DestroyTexture(m_softSprite);
}

// Procedural radial glow: white RGB, alpha falls off from the centre. The
// per-mote colour comes from SDL_SetTextureColorMod, so one sprite serves
// every theme and no asset file is needed.
SDL_Texture* AmbientParticles::makeGlowSprite(SDL_Renderer* renderer, int size, float falloff, float core) {
    SDL_Surface* surf = SDL_CreateSurface(size, size, SDL_PIXELFORMAT_RGBA32);
    if (!surf) return nullptr;
    auto* px = static_cast<Uint8*>(surf->pixels);
    const float c = (size - 1) * 0.5f;
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            float dx = (x - c) / c, dy = (y - c) / c;
            float d = std::sqrt(dx * dx + dy * dy);
            float a = d >= 1.0f ? 0.0f : std::pow(1.0f - d, falloff);
            if (d < core) a = std::min(1.0f, a + (core - d) / core); // hot centre so it passes the bloom threshold
            Uint8* p = px + y * surf->pitch + x * 4;
            p[0] = p[1] = p[2] = 255;
            p[3] = static_cast<Uint8>(std::clamp(a, 0.0f, 1.0f) * 255.0f);
        }
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_DestroySurface(surf);
    if (tex) {
        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND); // NOT additive: the bloom target is transparent, additive would leave alpha at 0
        SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_LINEAR);
    }
    return tex;
}

void AmbientParticles::init(SDL_Renderer* renderer, AmbientTheme theme) {
    m_theme = theme;
    m_back.clear();
    m_front.clear();
    m_time = 0.0f;
    if (theme == AmbientTheme::NONE) return;

    if (!m_sharpSprite) m_sharpSprite = makeGlowSprite(renderer, 32, 2.0f, 0.75f);
    if (!m_softSprite)  m_softSprite  = makeGlowSprite(renderer, 64, 1.4f, 0.75f);

    ThemeStyle s = styleFor(theme);
    buildLayer(m_back,  static_cast<int>(BACK_COUNT * s.backCount), false);
    buildLayer(m_front, FRONT_COUNT, true);
}

// Depth drives everything: a mote that is "further" (small parallax) is small,
// slow and dim; a "nearer" one (big parallax, front layer) is large, fast and soft
void AmbientParticles::buildLayer(std::vector<Mote>& layer, int count, bool front) {
    ThemeStyle s = styleFor(m_theme);
    const SDL_FRect cam = Camera::getInstance().getCamera();
    const float viewW = cam.w > 0 ? cam.w : 1600.0f;
    const float viewH = cam.h > 0 ? cam.h : 720.0f;
    const float margin = front ? FRONT_MARGIN : BACK_MARGIN;
    const float domW = viewW + margin * 2, domH = viewH + margin * 2;

    layer.resize(count);
    for (auto& m : layer) {
        float t = rnd(0.0f, 1.0f); // depth within the layer
        if (!front) {
            m.parallax = lerp(0.35f, 0.85f, t);
            m.size     = lerp(16.0f, 36.0f, t) * s.sizeMul;
            m.speedY   = lerp(8.0f, 22.0f, t) * s.speedMul;
            m.alpha    = lerp(0.30f, 1.0f, t) * s.alphaMul;
        } else {
            m.parallax = lerp(1.15f, 1.70f, t);
            m.size     = lerp(20.0f, 40.0f, t) * s.sizeMul;
            m.speedY   = lerp(35.0f, 90.0f, t) * s.speedMul;
            m.alpha    = lerp(0.18f, 1.0f, t) * s.alphaMul;
        }
        m.x = rnd(0.0f, domW);
        m.y = rnd(0.0f, domH);
        m.driftX = rnd(-6.0f, 6.0f) * (front ? 2.0f : 1.0f);
        m.swayAmp = rnd(4.0f, 14.0f) * s.swayMul * (front ? 1.5f : 1.0f);
        m.swayFreq = rnd(0.5f, 1.6f);
        m.twinkleFreq = rnd(0.8f, 2.4f) * s.twinkleMul;
        m.phase = rnd(0.0f, 6.2831853f);
        const uint8_t* col = s.palette[std::uniform_int_distribution<int>(0, s.paletteCount - 1)(g_rng)];
        m.r = col[0]; m.g = col[1]; m.b = col[2];
    }
}

void AmbientParticles::update(float dt) {
    if (m_theme == AmbientTheme::NONE) return;
    m_time += dt;
    const SDL_FRect cam = Camera::getInstance().getCamera();
    const float viewW = cam.w > 0 ? cam.w : 1600.0f;
    const float viewH = cam.h > 0 ? cam.h : 720.0f;

    auto step = [&](std::vector<Mote>& layer, float margin) {
        const float domW = viewW + margin * 2, domH = viewH + margin * 2;
        for (auto& m : layer) {
            m.y -= m.speedY * dt;       // always rises
            m.x += m.driftX * dt;
            m.x = wrap(m.x, domW);      // keeps floats small, wrap again at render time
            m.y = wrap(m.y, domH);
        }
    };
    step(m_back, BACK_MARGIN);
    step(m_front, FRONT_MARGIN);
}

void AmbientParticles::renderLayer(SDL_Renderer* renderer, const std::vector<Mote>& layer,
                                   SDL_Texture* sprite, float margin) {
    if (!sprite) {
        LOGI("returning");
        return;
    }
    const SDL_FRect cam = Camera::getInstance().getCamera();
    const float viewW = cam.w > 0 ? cam.w : 1600.0f;
    const float viewH = cam.h > 0 ? cam.h : 720.0f;
    const float domW = viewW + margin * 2, domH = viewH + margin * 2;
    LOGI("we are rendering");
    for (const auto& m : layer) {
        // Wrap the parallax-shifted position into the on-screen domain: particles
        // leaving one edge re-enter the opposite one, so coverage is always uniform
        // while the camera scrolls, with no spawn logic.
        float sway = std::sin(m_time * m.swayFreq + m.phase) * m.swayAmp;
        float sx = wrap(m.x + sway - cam.x * m.parallax, domW) - margin;
        float sy = wrap(m.y - cam.y * m.parallax, domH) - margin;

        if (sx < -m.size || sx > viewW + m.size || sy < -m.size || sy > viewH + m.size) continue;

        float twinkle = 0.65f + 0.35f * std::sin(m_time * m.twinkleFreq + m.phase * 1.7f);
        float a = std::clamp(m.alpha * twinkle, 0.0f, 1.0f);

        SDL_SetTextureColorMod(sprite, m.r, m.g, m.b);
        SDL_SetTextureAlphaMod(sprite, static_cast<Uint8>(a * 255.0f));
        SDL_FRect dst{sx - m.size * 0.5f, sy - m.size * 0.5f, m.size, m.size};
        SDL_RenderTexture(renderer, sprite, nullptr, &dst);
        LOGI("particle x:%f ,y:%f ,w:%f ,h:%f",dst.x,dst.y,dst.w,dst.h);
    }
}

void AmbientParticles::renderBack(SDL_Renderer* renderer) {
    LOGI("right after call");

    if (m_theme == AmbientTheme::NONE) {
        LOGI("returning form renderback");
        return;
    }
    renderLayer(renderer, m_back, m_sharpSprite, BACK_MARGIN);
}

void AmbientParticles::renderFront(SDL_Renderer* renderer) {
    LOGI("right after call");

    if (m_theme == AmbientTheme::NONE) {
        LOGI("returning form renderFront");
        return;
    }
    renderLayer(renderer, m_front, m_softSprite, FRONT_MARGIN);
}

AmbientTheme AmbientParticles::themeFromBackground(const std::vector<BackGroundElement>& elements) {
    bool cave = false, caveLight = false;
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
            default: break;
        }
    }
    if (caveLight) return AmbientTheme::BRIGHT_CAVE;
    if (cave)      return AmbientTheme::STONE_CAVE;
    return AmbientTheme::GREENERY;
}
