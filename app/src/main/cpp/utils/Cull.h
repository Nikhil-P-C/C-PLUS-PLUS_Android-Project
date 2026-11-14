//
// Camera culling helpers.
//
// Every world-space draw call asks "is this on screen?" before it reaches SDL. The view rectangle is built from the
// SAME rounded camera position the renderers subtract (camX / camY), so what is culled never disagrees with what
// would have been drawn (shake included, because Camera::getCamera() already adds it).
//
// Usage (once per render function, then per object):
//     const Cull::View view = Cull::world();                 // exact screen, in world px
//     if(!Cull::hit(view, obj.x, obj.y, obj.w, obj.h)) continue;
//     // margin: extra px around the screen, for things that draw bigger than their rect (glows, rotated sprites)
//     const Cull::View wide = Cull::world(128.0f);
//
// Cull::screen() is the same idea for things that are already in screen space (parallax layers, screen-space particles).
//
#pragma once
#include <cmath>
#include "utils/Camera.h"
#include "utils/PerfStats.h"

namespace Cull {

    struct View {
        float x, y, w, h;
    };

    // The visible part of the world, grown by `margin` px on every side.
    inline View world(float margin = 0.0f) {
        const SDL_FRect c = Camera::getInstance().getCamera();
        return { std::round(c.x) - margin, std::round(c.y) - margin, c.w + margin * 2.0f, c.h + margin * 2.0f };
    }

    // The screen rectangle (0,0,w,h), grown by `margin` px. For objects whose position is already camera-relative.
    inline View screen(float margin = 0.0f) {
        const SDL_FRect c = Camera::getInstance().getCamera();
        return { -margin, -margin, c.w + margin * 2.0f, c.h + margin * 2.0f };
    }

    // true if the rect touches the view (touching edges counts as visible, so nothing pops at the border)
    inline bool hit(const View& v, float x, float y, float w, float h) {
        const bool visible = x <= v.x + v.w && x + w >= v.x && y <= v.y + v.h && y + h >= v.y;
        ++PerfCounters::cullTested;                       // shown in the debug overlay ("culled n / m")
        if (!visible) ++PerfCounters::cullRejected;
        return visible;
    }

    inline bool hit(const View& v, const SDL_FRect& r) {
        return hit(v, r.x, r.y, r.w, r.h);
    }

    // a square centred on (cx, cy) with half-size r: particles, glows, anything radial
    inline bool hitCircle(const View& v, float cx, float cy, float r) {
        return hit(v, cx - r, cy - r, r * 2.0f, r * 2.0f);
    }
}
