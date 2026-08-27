//
// Created by LENOVO on 24-08-2026.
//
#include "BackGroundBuilder.h"
#include "utils/utils.h"
#include <SDL3/SDL.h>

void BackGroundBuilder::render(SDL_Renderer *renderer) {
    float camX = Camera::getInstance().getCamera().x;
    float camY = Camera::getInstance().getCamera().y;
    for(const auto element : m_elements){
        SDL_FRect dst{element.rect.x - camX * element.parallaxFloatX,
                      element.rect.y - camY * element.parallaxFloatY,
                      element.rect.w, element.rect.h};
//        SDL_FRect src{0.00f,0.00f,};
        SDL_Texture* texture = Engine::Get().getAssetManager().getTexture(element.type);
        SDL_RenderTexture(renderer,texture, nullptr,&dst);
        if(element.type == TextureType::BG_GREENERY_MOUNTAINS){//try fill gap ,mannually
            SDL_FRect gapDst{(element.rect.x) - camX * element.parallaxFloatX,
                             (element.rect.y+element.rect.h-100)- camY * element.parallaxFloatY,
                             element.rect.w, element.rect.h};
            SDL_SetRenderDrawColor(renderer,65 ,85 ,34,255);
            SDL_RenderFillRect(renderer,&gapDst);
        }
    }
}

void BackGroundBuilder::update(float dt) {

}

void BackGroundBuilder::init(std::vector<BackGroundElement>& elements) {

//    m_elements = elements;
    m_elements.emplace_back(0,0,1600,720,0.00,0.00,false,TextureType::BG_GREENERY_SKY);

    m_elements.emplace_back(0,0,1822,820,0.03,0.03,false,TextureType::BG_GREENERY_MOUNTAINS);


//    m_elements.emplace_back(-1000,-600,2488,1120,0.40,0.40,false,TextureType::BG_GREENERY_TREES);
//
//    m_elements.emplace_back(1488,-600,2488,1120,0.40,0.40,false,TextureType::BG_GREENERY_TREES);
//
//    m_elements.emplace_back(3996,-600,2488,1120,0.40,0.40,false,TextureType::BG_GREENERY_TREES);

    m_elements.emplace_back(0,-400,2488,1120,0.50,0.50,false,TextureType::BG_GREENERY_TREES);

    m_elements.emplace_back(2488,-400,2488,1120,0.50,0.50,false,TextureType::BG_GREENERY_TREES);
}

