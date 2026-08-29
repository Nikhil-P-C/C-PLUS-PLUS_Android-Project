//
// Created by LENOVO on 24-08-2026.
//

#pragma once
#include <SDL3/SDL.h>
#include <vector>
#include "engine/Engine.h"
// index 1 is base , index > 1 is ForeGround
enum class ForeGroundType{
    GREENERY_NEAR_OBJECT=0,
    GREENERY_NEAR_GRASS,
    MEADOWS_NEAR_FLOWERS,
    NONE
};
struct ForeGroundElement{
    ForeGroundElement(float x,float y,float w,float h,float parallaxFactorX,float parallaxFactorY,bool isAnimated,ForeGroundType type):
            parallaxFloatX(parallaxFactorX),parallaxFloatY(parallaxFactorY),isAnimated(isAnimated),type(type){
        rect.x = x;
        rect.y = y;
        rect.w = w;
        rect.h = h;
    }
    SDL_FRect rect{0};
    ForeGroundType type = ForeGroundType::MEADOWS_NEAR_FLOWERS;
    float parallaxFloatX= 1.25;//default render on right below main ground
    float parallaxFloatY= 1.50;
    bool isAnimated = false;
};

class ForeGroundBuilder{
public:

    void init(std::vector<ForeGroundElement>& elements);

    void render(SDL_Renderer* renderer);

    void update(float dt);

private:
    std::vector<ForeGroundElement> m_elements;

};