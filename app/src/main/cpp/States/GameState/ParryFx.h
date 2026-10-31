//
// Parry visuals that are drawn directly on the frame (no art needed):
//   the contact flash on a successful parry: bright flare, expanding ring and streaks
//
// It is its own small class so the parry never has to touch the ambience / lighting / darkness code:
//   * update() runs on REAL time (GameState::updateRealtimeFx), so the flash animates while the hit-stop freezes the world.
//   * render() is drawn AFTER Lighting + AmbienceFx::renderOverlay (a spark is its own light, a dark cave must not swallow
//     it, the weather tint must not colour it) and BEFORE RoomDarkness (a room that is still hidden stays hidden).
//
#pragma once
#include <SDL3/SDL.h>
#include <vector>

class ParryFx {
public:
    // contact flash. (wx, wy) = world point where the attack met the parry, dirX = +1/-1 toward the attacker
    void spawn(float wx, float wy, float dirX, bool big);

    void update(float realDt);
    void render(SDL_Renderer* renderer, float camX, float camY) const;
    void clear();

    bool busy() const { return !m_flashes.empty(); }

private:
    struct Flash {
        float x = 0.0f, y = 0.0f, dir = 1.0f;
        float age = 0.0f, life = 0.24f, scale = 1.0f;
        unsigned int seed = 1u;
    };
    std::vector<Flash> m_flashes;

    mutable std::vector<SDL_Vertex> m_verts;   // scratch buffers, reused every frame
    mutable std::vector<int>        m_idx;
};
