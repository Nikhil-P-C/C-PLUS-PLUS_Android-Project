//
// Created by LENOVO on 24-08-2026.
//
#pragma once
#include <SDL3/SDL.h>
#include <vector>
// index 1 is base , index < 1 is background
struct BackGroundElement{
    BackGroundElement(float x,float y,float w,float h,float parallaxFactorX,float parallaxFactorY,bool isAnimated):
    parallaxFloatX(parallaxFactorX),parallaxFloatY(parallaxFactorY),isAnimated(isAnimated){
        rect.x = x;
        rect.y = y;
        rect.w = w;
        rect.h = h;
    }
    SDL_FRect rect{0};
    float parallaxFloatX= 0.75f;//default render on right below main ground
    float parallaxFloatY = 0.50f;
    bool isAnimated = false;
};

class BackGroundBuilder{
public:

    void init(std::vector<BackGroundElement>& elements);

    void render(SDL_Renderer* renderer);

    void update(float dt);

private:
    std::vector<BackGroundElement> m_elements;

};