//
// Created by LENOVO on 24-08-2026.
//
#include "BackGroundBuilder.h"
#include "utils/utils.h"
#include <SDL3/SDL.h>

void BackGroundBuilder::render(SDL_Renderer *renderer) {
    int camX = (int)std::round(Camera::getInstance().getCamera().x);
    int camY = (int)std::round(Camera::getInstance().getCamera().y);
    for(const auto element : m_elements){
        SDL_FRect dst{element.rect.x - camX * element.parallaxFloatX,
                      element.rect.y - camY * element.parallaxFloatY,
                      element.rect.w, element.rect.h};

        if(element.parallaxFloatX >= 0.0 && element.parallaxFloatX < 0.50){
            SDL_SetRenderDrawColor(renderer,200,200,200,255);

        }
        if(element.parallaxFloatX >= 0.50 && element.parallaxFloatX < 0.75){
            SDL_SetRenderDrawColor(renderer,0,255,0,255);

        }
        if(element.parallaxFloatX >= 0.75 && element.parallaxFloatX < 1.0){
            SDL_SetRenderDrawColor(renderer,36, 77, 27,255);
        }
        SDL_RenderFillRect(renderer, &dst);

    }
}

void BackGroundBuilder::update(float dt) {

}

void BackGroundBuilder::init(std::vector<BackGroundElement>& elements) {

//    m_elements = elements;
    m_elements.emplace_back(550,1600,500,500,0.10,0.05,false);

    m_elements.emplace_back(0,1300,50,100,0.50,0.25,false);
    m_elements.emplace_back(200,1300,50,100,0.50,0.25,false);
    m_elements.emplace_back(300,1300,50,100,0.50,0.25,false);
    m_elements.emplace_back(400,1300,50,100,0.50,0.25,false);
    m_elements.emplace_back(450,1300,50,100,0.50,0.25,false);
    m_elements.emplace_back(750,1300,50,100,0.50,0.25,false);
    m_elements.emplace_back(2200,1300,50,100,0.50,0.25,false);
    m_elements.emplace_back(2000,1300,50,100,0.50,0.25,false);
    m_elements.emplace_back(900,1300,50,100,0.50,0.25,false);
    m_elements.emplace_back(800,1300,50,100,0.50,0.25,false);
    m_elements.emplace_back(1300,1300,50,100,0.50,0.25,false);


    m_elements.emplace_back(880,800,50,200,0.75,0.50,false);
    m_elements.emplace_back(1000,800,50,200,0.75,0.50,false);
    m_elements.emplace_back(1200,800,50,200,0.75,0.50,false);
    m_elements.emplace_back(1400,800,50,200,0.75,0.50,false);
    m_elements.emplace_back(1600,800,50,200,0.75,0.50,false);
    m_elements.emplace_back(1800,800,50,200,0.75,0.50,false);
    m_elements.emplace_back(2000,800,50,200,0.75,0.50,false);
    m_elements.emplace_back(2200,800,50,200,0.75,0.50,false);
    m_elements.emplace_back(2400,800,50,200,0.75,0.50,false);
    m_elements.emplace_back(2600,800,50,200,0.75,0.50,false);
    m_elements.emplace_back(2800,800,50,200,0.75,0.50,false);
    m_elements.emplace_back(3000,800,50,200,0.75,0.50,false);
    m_elements.emplace_back(3200,800,50,200,0.75,0.50,false);
    m_elements.emplace_back(500,800,50,200,0.75,0.50,false);
    m_elements.emplace_back(700,800,50,200,0.75,0.50,false);
    m_elements.emplace_back(100,800,50,200,0.75,0.50,false);





}

