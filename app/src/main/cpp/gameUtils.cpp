//
// Created by LENOVO on 17-03-2026.
//
#include "SDL3/SDL.h"
#include "gameUtils.h"
namespace gameUtilities{
    bool checkcollision(float& x1, float& y1, float x2, float y2, float h1, float w1, float h2, float w2) {

        if (x1 < x2 + w2 &&
            x1 + w1 > x2 &&
            y1 < y2 + h2 &&
            y1 + h1 > y2) {
            float centerX1 = x1 + (w1 / 2);
            float centerY1 = y1 + (h1 / 2);
            float centerX2 = x2 + (w2 / 2);
            float centerY2 = y2 + (h2 / 2);

            float dx = centerX1 - centerX2;
            float dy = centerY1 - centerY2;

            float overlapX = ((w1 / 2) + (w2 / 2)) - fabs(dx);
            float overlapY = ((h1 / 2) + (h2 / 2)) - fabs(dy);
            if (overlapX > 0 && overlapY > 0) {
                if (overlapX < overlapY) {
                    // Push along X
                    if (dx > 0) x1 += overlapX;
                    else        x1 -= overlapX;
                }
                else {
                    // Push along Y
                    if (dy > 0) y1 += overlapY;
                    else        y1 -= overlapY;
                }
                return true;
            }
            return false;
        }
        return false;

    }
    void interpolate(SDL_FRect& src, float targetX, float targetY ,float&& factor){// try values between 0.05f and 0.2f

        // interpolate towards target
        src.x += (targetX - src.x) * factor;
        src.y += (targetY - src.y) * factor;
    }
}