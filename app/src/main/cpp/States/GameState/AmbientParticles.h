//
// floating motes: a dim, slow BACK layer drawn
// behind the terrain and a larger, faster, softer FRONT layer drawn over
// everything. Both are drawn inside the existing bloom groups, so the glow
// comes from PostProcessor's bright-pass, no extra shader or blend mode.
//
#pragma once
#include <SDL3/SDL.h>
#include <vector>
#include "level/BackGroundBuilder.h"

enum class AmbientTheme {
    GREENERY=0,   // green + yellow, fireflies / pollen
    STONE_CAVE,   // dull grey dust
    BRIGHT_CAVE,  // blue glow motes
    NONE,
    MENU,         // menu screens: screen space (ignores the game camera), additive, recoloured with setTint()
};

class AmbientParticles {
public:
    AmbientParticles() = default;
    ~AmbientParticles();
    AmbientParticles(const AmbientParticles&) = delete;
    AmbientParticles& operator=(const AmbientParticles&) = delete;

    // Call from GameState::setLevel (after the background elements are loaded).
    void init(SDL_Renderer* renderer, AmbientTheme theme);
    void update(float dt);

    // Call inside the bloom groups (see GameState::render).
    void renderBack(SDL_Renderer* renderer);
    void renderFront(SDL_Renderer* renderer);

    // Menu helpers (work for any theme, the menus use AmbientTheme::MENU):
    //  setTint:       blend every mote's own colour towards (r,g,b); amount 0 = own palette, 1 = fully tinted
    //  setAlphaScale: global opacity multiplier (heartbeat pulse, fades)
    void setTint(uint8_t r, uint8_t g, uint8_t b, float amount = 1.0f) { m_tintR = r; m_tintG = g; m_tintB = b; m_tintAmount = amount; }
    void setAlphaScale(float scale) { m_alphaScale = scale; }

    // Derives the theme from the level's background layers:
    //   any CAVE_FAR_LIGHT*  -> BRIGHT_CAVE
    //   any other CAVE_*     -> STONE_CAVE
    //   any SNOWY_*          -> NONE (AmbienceFx does the snow)
    //   everything else      -> GREENERY (greenery + meadows + sunflower)
    static AmbientTheme themeFromBackground(const std::vector<BackGroundElement>& elements);

private:
    struct Mote {
        float x = 0, y = 0;        // position inside the wrap domain (layer space)
        float parallax = 1.0f;     // how much the camera moves this mote
        float size = 10.0f;        // sprite size in px
        float speedY = 20.0f;      // upward speed, px/s
        float driftX = 0.0f;       // slow sideways wind, px/s
        float swayAmp = 0.0f;      // px
        float swayFreq = 1.0f;     // rad/s
        float twinkleFreq = 1.0f;  // rad/s
        float phase = 0.0f;
        float alpha = 0.5f;        // 0..1 base opacity
        uint8_t r = 255, g = 255, b = 255;
    };

    // View size + camera offset the motes live in: the game camera, or a fixed 1600x720 screen for MENU.
    void getView(float& viewW, float& viewH, float& camX, float& camY) const;
    void buildLayer(std::vector<Mote>& layer, int count, bool front);
    void renderLayer(SDL_Renderer* renderer, const std::vector<Mote>& layer, SDL_Texture* sprite, float margin);
    SDL_Texture* makeGlowSprite(SDL_Renderer* renderer, int size, float falloff, float core);
    // In-game sprite: tiny hot core + smooth gaussian halo (reads as a glowing point, not a flat disc)
    SDL_Texture* makeHaloSprite(SDL_Renderer* renderer, int size, float coreSigma, float haloGain, float haloFalloff);
    void destroySprites();

    AmbientTheme m_theme = AmbientTheme::NONE;
    std::vector<Mote> m_back;
    std::vector<Mote> m_front;
    SDL_Texture* m_sharpSprite = nullptr; // small bright core, used by the back layer
    SDL_Texture* m_softSprite  = nullptr; // big soft bokeh, used by the front layer
    float m_time = 0.0f;
    uint8_t m_tintR = 255, m_tintG = 255, m_tintB = 255;
    float m_tintAmount = 0.0f;
    float m_alphaScale = 1.0f;
    bool  m_spritesForMenu = false;   // which sprite set is currently built
    float m_wind = 0.0f;              // shared slow gust (px/s), in-game only
};
