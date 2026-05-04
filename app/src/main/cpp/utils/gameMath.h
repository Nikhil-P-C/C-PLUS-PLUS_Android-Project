//
// Created by LENOVO on 03-05-2026.
//

#pragma once
#include "cmath"
namespace gameMath{
    bool checkcollision(float& x1, float& y1, float x2, float y2, float h1, float w1, float h2, float w2) ;
    void interpolate(float& cameraX,float& cameraY,float targetX, float targetY ,float factor);
}