//
// Created by LENOVO on 24-08-2026.
//
#pragma once
#include <SDL3/SDL.h>
#include <vector>
#include "engine/Engine.h"
// index 1 is base , index < 1 is background
enum class BackGroundType{
    GREENERY_SKY=0,
    GREENERY_MOUNTAINS,
    GREENERY_TREES,
    MEADOWS_SKY,
    MEADOWS_CLOUD,
    MEADOWS_FAR_PLAIN,
    MEADOWS_FLOWER_FIELD,

    CAVE_SKY,
    CAVE_FAR_OBJECT1,
    CAVE_FAR_OBJECT2,
    CAVE_FAR_OBJECT3,
    CAVE_FAR_OBJECT4,
    CAVE_FAR_OBJECT5,
    CAVE_FAR_LIGHT1,
    CAVE_FAR_LIGHT2,
    CAVE_FAR_LIGHT3,
    NONE
};
struct BackGroundElement{
    BackGroundElement(float x,float y,float w,float h,float parallaxFactorX,float parallaxFactorY,bool isAnimated,BackGroundType type):
    parallaxFloatX(parallaxFactorX),parallaxFloatY(parallaxFactorY),isAnimated(isAnimated),type(type){
        rect.x = x;
        rect.y = y;
        rect.w = w;
        rect.h = h;
    }
    BackGroundType type;
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