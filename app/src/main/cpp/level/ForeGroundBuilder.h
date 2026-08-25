//
// Created by LENOVO on 24-08-2026.
//

#pragma once
#include <SDL3/SDL.h>
#include <vector>
// index 1 is base , index > 1 is ForeGround
struct ForeGroundElement{
    ForeGroundElement(float x,float y,float w,float h,float parallaxFactor,bool isAnimated):
            parallaxFloat(parallaxFactor),isAnimated(isAnimated){
        rect.x = x;
        rect.y = y;
        rect.w = w;
        rect.h = h;
    }
    SDL_FRect rect{0};
    float parallaxFloat= 1.25;//default render on right below main ground
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