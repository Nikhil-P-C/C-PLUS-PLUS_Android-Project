//
// Room darkness: in caves and closed buildings every room is hidden behind solid darkness until the player
// walks into it. From outside (the corridor, the next room, the other side of a wall or blockade) the player
// sees nothing of what is in there; the moment the player steps inside, the darkness fades away.
//
// Level file record (see Levels/readme.txt):
//     H x y w h [fadeSeconds] [KEEP] [FORCE] [OPACITY=0..1] [SOFT=px]
//     x y w h   the room, in world pixels. Cover the whole inside (up to the walls / the doorway).
//     fadeSeconds  how long the darkness takes to melt when the player enters (default 0.6)
//     KEEP      the room stays revealed after the first visit (default: the darkness returns when you leave)
//     FORCE     also works in biomes that are normally open air (a closed building drawn with forest art, ...)
//     OPACITY   how black the hidden room is (default 1 = nothing visible, 0.9 = a faint silhouette)
//     SOFT      half-width in px of the grey gradient between the dark room and the lit area (default 120,
//               so the blend is ~240 px wide, centred on the room border; 0 = nearly hard edge)
//
// Only caves (stone / bright) honour the H lines by themselves. Forest, meadows and peaks are open areas,
// so their H lines are ignored unless the room says FORCE.
//
// It is drawn in screen space right after Lighting::render (so it also hides lights, glows and particles
// inside the room) and before the HUD. The border is a wide eased gradient (black -> grey -> clear), not a cut-out.
//
#pragma once
#include <SDL3/SDL.h>
#include <vector>
#include "level/Lights.h"
#include "AmbienceFx.h"

class RoomDarkness {
public:
    // call from GameState::setLevel. Rooms are dropped in open-air biomes (unless FORCE).
    void init(const std::vector<DarkRoom>& rooms, AmbienceBiome biome);

    // px, py = the player's centre in world pixels. The first call after init() snaps (the room you spawn in
    // starts lit instead of fading in).
    void update(float dt, float px, float py);

    // screen space, after the lighting pass
    void render(SDL_Renderer* renderer, float camX, float camY) const;

    bool active() const { return !m_rooms.empty(); }

    // 0 = fully lit .. 1 = fully hidden, at a world point (for hiding map markers, enemy health bars, ...)
    float coverAt(float wx, float wy) const;

private:
    struct Room {
        DarkRoom def;
        float reveal = 0.0f;     // 0 hidden .. 1 revealed
        bool  visited = false;
    };
    std::vector<Room> m_rooms;
    bool m_first = true;
};
