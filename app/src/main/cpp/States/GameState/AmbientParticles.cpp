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
                        1.0f, 0.6f, 1.2f, 1.0f, 1.1f, 1.0f};
            case AmbientTheme::STONE_CAVE:
                return {{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, 4,
                        0.9f, 0.6f, 0.7f, 1.0f, 0.8f, 1.0f};
            case AmbientTheme::BRIGHT_CAVE:
                return {{{90, 170, 255}, {120, 210, 255}, {170, 230, 255}, {70, 140, 255}}, 4,
                        0.9f, 0.6f, 0.8f, 1.0f, 0.9f, 1.0f};
            case AmbientTheme::MENU:
                // white: the menu recolours the motes every frame with setTint(); smaller than in-game,
                // and ~2.5x the back-layer count because the screen is otherwise just black
                return {{{255, 255, 255}, {235, 245, 255}, {255, 250, 235}, {255, 255, 255}}, 4,
                        1.0f, 0.5f, 1.2f, 0.9f, 1.0f, 2.5f};
            default:
                return {{{255, 255, 255}}, 1, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
        }
    }

    // Base counts, kept low on purpose (the C30s is the validation device).
    constexpr int BACK_COUNT  = 36;   // MENU base (x2.5 via backCount)
    constexpr int FRONT_COUNT = 12;
    // In-game: many small motes instead of a few big blobs (menu look). Still cheap: one draw each.
    constexpr int GAME_BACK_COUNT  = 70;
    constexpr int GAME_FRONT_COUNT = 14;
    // The halo sprite is drawn this many times larger than mote.size (size = visible core).
    constexpr float GAME_HALO_SCALE = 2.2f;
    constexpr float BACK_MARGIN  = 40.0f;
    constexpr float FRONT_MARGIN = 80.0f;
}

void AmbientParticles::destroySprites() {
    if (m_sharpSprite) { SDL_DestroyTexture(m_sharpSprite); m_sharpSprite = nullptr; }
    if (m_softSprite)  { SDL_DestroyTexture(m_softSprite);  m_softSprite  = nullptr; }
}

AmbientParticles::~AmbientParticles() {
    destroySprites();
}

// Gaussian hot core + wide soft halo. White RGB, per-mote colour via ColorMod (same as makeGlowSprite).
SDL_Texture* AmbientParticles::makeHaloSprite(SDL_Renderer* renderer, int size, float coreSigma,
                                              float haloGain, float haloFalloff) {
    SDL_Surface* surf = SDL_CreateSurface(size, size, SDL_PIXELFORMAT_RGBA32);
    if (!surf) return nullptr;
    auto* px = static_cast<Uint8*>(surf->pixels);
    const float c = (size - 1) * 0.5f;
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            float dx = (x - c) / c, dy = (y - c) / c;
            float d = std::sqrt(dx * dx + dy * dy);
            float core = std::exp(-(d * d) / (2.0f * coreSigma * coreSigma));
            float halo = d >= 1.0f ? 0.0f : haloGain * std::pow(1.0f - d, haloFalloff);
            float a = std::clamp(core + halo, 0.0f, 1.0f);
            Uint8* p = px + y * surf->pitch + x * 4;
            p[0] = p[1] = p[2] = 255;
            p[3] = static_cast<Uint8>(a * 255.0f);
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

    const bool menu = (theme == AmbientTheme::MENU);
    if ((m_sharpSprite || m_softSprite) && m_spritesForMenu != menu) destroySprites();
    m_spritesForMenu = menu;
    if (menu) {   // unchanged: the menu look the game is being matched to
        if (!m_sharpSprite) m_sharpSprite = makeGlowSprite(renderer, 32, 2.0f, 0.75f);
        if (!m_softSprite)  m_softSprite  = makeGlowSprite(renderer, 64, 1.4f, 0.75f);
    } else {
        // back: tight bright core + faint halo. front: big dim bokeh, almost no core
        if (!m_sharpSprite) m_sharpSprite = makeHaloSprite(renderer, 48, 0.16f, 0.30f, 2.6f);
        if (!m_softSprite)  m_softSprite  = makeHaloSprite(renderer, 64, 0.30f, 0.22f, 1.8f);
    }

    // In-game the motes are drawn into the (transparent) bloom target, so they must be plain alpha
    // blended. The menus draw straight onto the black screen, where additive gives the glowing look.
    const SDL_BlendMode mode = (theme == AmbientTheme::MENU) ? SDL_BLENDMODE_ADD : SDL_BLENDMODE_BLEND;
    if (m_sharpSprite) SDL_SetTextureBlendMode(m_sharpSprite, mode);
    if (m_softSprite)  SDL_SetTextureBlendMode(m_softSprite, mode);

    ThemeStyle s = styleFor(theme);
    if (menu) {
        buildLayer(m_back,  static_cast<int>(BACK_COUNT * s.backCount), false);
        buildLayer(m_front, FRONT_COUNT, true);
    } else {
        buildLayer(m_back,  static_cast<int>(GAME_BACK_COUNT * s.backCount), false);
        buildLayer(m_front, GAME_FRONT_COUNT, true);
    }
}

void AmbientParticles::getView(float& viewW, float& viewH, float& camX, float& camY) const {
    if (m_theme == AmbientTheme::MENU) {          // menus have no scrolling camera (and the game camera may be stale)
        viewW = 1600.0f; viewH = 720.0f; camX = 0.0f; camY = 0.0f;
        return;
    }
    const SDL_FRect cam = Camera::getInstance().getCamera();
    viewW = cam.w > 0 ? cam.w : 1600.0f;
    viewH = cam.h > 0 ? cam.h : 720.0f;
    camX = cam.x; camY = cam.y;
}

// Depth drives everything: a mote that is "further" (small parallax) is small,
// slow and dim; a "nearer" one (big parallax, front layer) is large, fast and soft
void AmbientParticles::buildLayer(std::vector<Mote>& layer, int count, bool front) {
    ThemeStyle s = styleFor(m_theme);
    float viewW, viewH, camX, camY;
    getView(viewW, viewH, camX, camY);
    const float margin = front ? FRONT_MARGIN : BACK_MARGIN;
    const float domW = viewW + margin * 2, domH = viewH + margin * 2;
    const bool game = (m_theme != AmbientTheme::MENU);

    layer.resize(count);
    for (auto& m : layer) {
        float t = rnd(0.0f, 1.0f); // depth within the layer
        if (!front) {
            m.parallax = lerp(0.35f, 0.85f, t);
            m.size     = lerp(16.0f, 36.0f, t) * s.sizeMul;
            m.speedY   = lerp(8.0f, 22.0f, t) * s.speedMul;
            m.alpha    = lerp(game ? 0.50f : 0.30f, 1.0f, t) * s.alphaMul;   // game: floor high enough to still bloom
        } else {
            m.parallax = lerp(1.15f, 1.70f, t);
            m.size     = lerp(20.0f, 40.0f, t) * s.sizeMul;
            m.speedY   = lerp(35.0f, 90.0f, t) * s.speedMul;
            m.alpha    = (game ? lerp(0.20f, 0.55f, t) : lerp(0.18f, 1.0f, t)) * s.alphaMul;   // game: foreground bokeh stays subtle
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
    float viewW, viewH, camX, camY;
    getView(viewW, viewH, camX, camY);

    // Slow shared gust so the whole field leans one way, then the other (nearer motes lean more).
    m_wind = (m_theme == AmbientTheme::MENU) ? 0.0f
                                             : std::sin(m_time * 0.23f) * 14.0f + std::sin(m_time * 0.61f + 1.3f) * 6.0f;

    auto step = [&](std::vector<Mote>& layer, float margin) {
        const float domW = viewW + margin * 2, domH = viewH + margin * 2;
        for (auto& m : layer) {
            m.y -= m.speedY * dt;       // always rises
            m.x += (m.driftX + m_wind * m.parallax) * dt;
            m.x = wrap(m.x, domW);      // keeps floats small, wrap again at render time
            m.y = wrap(m.y, domH);
        }
    };
    step(m_back, BACK_MARGIN);
    step(m_front, FRONT_MARGIN);
}

void AmbientParticles::renderLayer(SDL_Renderer* renderer, const std::vector<Mote>& layer,
                                   SDL_Texture* sprite, float margin) {
    if (!sprite) return;
    float viewW, viewH, camX, camY;
    getView(viewW, viewH, camX, camY);
    const float domW = viewW + margin * 2, domH = viewH + margin * 2;
    const float haloScale = (m_theme == AmbientTheme::MENU) ? 1.0f : GAME_HALO_SCALE;
    for (const auto& m : layer) {
        // Wrap the parallax-shifted position into the on-screen domain: particles
        // leaving one edge re-enter the opposite one, so coverage is always uniform
        // while the camera scrolls, with no spawn logic.
        float sway = std::sin(m_time * m.swayFreq + m.phase) * m.swayAmp;
        float sx = wrap(m.x + sway - camX * m.parallax, domW) - margin;
        float sy = wrap(m.y - camY * m.parallax, domH) - margin;

        const float drawSize = m.size * haloScale;
        if (sx < -drawSize || sx > viewW + drawSize || sy < -drawSize || sy > viewH + drawSize) continue;

        float twinkle = 0.65f + 0.35f * std::sin(m_time * m.twinkleFreq + m.phase * 1.7f);
        float a = std::clamp(m.alpha * twinkle * m_alphaScale, 0.0f, 1.0f);

        uint8_t cr = m.r, cg = m.g, cb = m.b;
        if (m_tintAmount > 0.0f) {
            cr = static_cast<uint8_t>(lerp(m.r, m_tintR, m_tintAmount));
            cg = static_cast<uint8_t>(lerp(m.g, m_tintG, m_tintAmount));
            cb = static_cast<uint8_t>(lerp(m.b, m_tintB, m_tintAmount));
        }
        SDL_SetTextureColorMod(sprite, cr, cg, cb);
        SDL_SetTextureAlphaMod(sprite, static_cast<Uint8>(a * 255.0f));
        SDL_FRect dst{sx - drawSize * 0.5f, sy - drawSize * 0.5f, drawSize, drawSize};
        SDL_RenderTexture(renderer, sprite, nullptr, &dst);
    }
}

void AmbientParticles::renderBack(SDL_Renderer* renderer) {
    if (m_theme == AmbientTheme::NONE) return;
    renderLayer(renderer, m_back, m_sharpSprite, BACK_MARGIN);
}

void AmbientParticles::renderFront(SDL_Renderer* renderer) {
    if (m_theme == AmbientTheme::NONE) return;
    renderLayer(renderer, m_front, m_softSprite, FRONT_MARGIN);
}

AmbientTheme AmbientParticles::themeFromBackground(const std::vector<BackGroundElement>& elements) {
    bool cave = false, caveLight = false, snowy = false;
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
            case BackGroundType::SNOWY_SKY:
            case BackGroundType::SNOWY_MOUNTAIN:
            case BackGroundType::SNOWY_FAR_RIDGE1:
            case BackGroundType::SNOWY_FAR_RIDGE2:
            case BackGroundType::SNOWY_FAR_RIDGE3:
            case BackGroundType::SNOWY_FAR_OBJECT1:
            case BackGroundType::SNOWY_FAR_OBJECT2:
            case BackGroundType::SNOWY_FAR_OBJECT3:
            case BackGroundType::SNOWY_FAR_OBJECT4:
                snowy = true; break;
            default: break;
        }
    }
    if (caveLight) return AmbientTheme::BRIGHT_CAVE;
    if (cave)      return AmbientTheme::STONE_CAVE;
    if (snowy)     return AmbientTheme::NONE;   // no green pollen over snow; the snowfall comes from AmbienceFx
    return AmbientTheme::GREENERY;
}
