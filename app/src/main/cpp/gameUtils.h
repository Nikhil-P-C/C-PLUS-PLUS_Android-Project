//
// Created by LENOVO on 17-03-2026.
//
#pragma once
#include <cmath>



namespace gameUtilities
{
    void interpolate(SDL_FRect& src, float targetX, float targetY ,float&& factor);
    bool checkcollision(float& x1, float& y1, float x2, float y2, float h1, float w1, float h2, float w2) ;
}

