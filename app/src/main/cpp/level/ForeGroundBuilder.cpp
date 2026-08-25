//
// Created by LENOVO on 24-08-2026.
//

#include "ForeGroundBuilder.h"
#include "utils/utils.h"
#include <SDL3/SDL.h>

void ForeGroundBuilder::render(SDL_Renderer *renderer) {
    int camX = (int)std::round(Camera::getInstance().getCamera().x);
    int camY = (int)std::round(Camera::getInstance().getCamera().y);
    for(const auto element : m_elements){
        SDL_FRect dst{element.rect.x - camX * element.parallaxFloat,
                      element.rect.y - camY * element.parallaxFloat,
                      element.rect.w, element.rect.h};

        if(element.parallaxFloat > 1.0 && element.parallaxFloat <= 1.25){
            SDL_SetRenderDrawColor(renderer,216, 252, 96,255);

        }
        if(element.parallaxFloat > 1.25 && element.parallaxFloat < 1.50){
            SDL_SetRenderDrawColor(renderer,148, 252, 96,255);

        }
        if(element.parallaxFloat > 1.50 && element.parallaxFloat < 1.75){
            SDL_SetRenderDrawColor(renderer,36, 77, 27,255);
        }
        SDL_RenderFillRect(renderer, &dst);

    }
}

void ForeGroundBuilder::update(float dt) {

}

void ForeGroundBuilder::init(std::vector<ForeGroundElement>& elements) {

//    m_elements = elements;
    m_elements.emplace_back(0,1600,500,50,1.10,false);

    m_elements.emplace_back(700,1600,500,50,1.10,false);
    m_elements.emplace_back(1400,1600,500,50,1.10,false);
    m_elements.emplace_back(2100,1600,500,50,1.10,false);
    m_elements.emplace_back(2800,1600,500,50,1.10,false);
    m_elements.emplace_back(3500,1600,500,50,1.10,false);

    m_elements.emplace_back(700,2100,600,200,1.50,false);
    m_elements.emplace_back(1400,2100,600,200,1.50,false);
    m_elements.emplace_back(2100,2100,600,200,1.50,false);
    m_elements.emplace_back(2800,2100,600,200,1.50,false);
    m_elements.emplace_back(3500,2100,500,200,1.50,false);


}

