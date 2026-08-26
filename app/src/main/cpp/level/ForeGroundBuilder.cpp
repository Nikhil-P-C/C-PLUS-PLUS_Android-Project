//
// Created by LENOVO on 24-08-2026.
//

#include "ForeGroundBuilder.h"
#include "utils/utils.h"
#include <SDL3/SDL.h>

void ForeGroundBuilder::render(SDL_Renderer *renderer) {
    float camX = Camera::getInstance().getCamera().x;
    float camY = Camera::getInstance().getCamera().y;
    for(const auto element : m_elements){
        SDL_FRect dst{element.rect.x - camX * element.parallaxFloatX,
                      element.rect.y - camY * element.parallaxFloatY,
                      element.rect.w, element.rect.h};
//        SDL_FRect src{0.00f,0.00f,};
        LOGI("dst: x:%f,y:%f",dst.x,dst.y);
        SDL_Texture* texture = Engine::Get().getAssetManager().getTexture(element.type);
        SDL_RenderTexture(renderer,texture, nullptr,&dst);
    }
}

void ForeGroundBuilder::update(float dt) {

}

void ForeGroundBuilder::init(std::vector<ForeGroundElement>& elements) {

//    m_elements = elements;

    m_elements.emplace_back(0,4600,3200,1440,4.90,4.90,false,TextureType::FG_GREENERY_NEAR_OBJECT);
    m_elements.emplace_back(3200,4600,3200,1440,4.90,4.90,false,TextureType::FG_GREENERY_NEAR_OBJECT);
    m_elements.emplace_back(6400,4600,3200,1440,4.90,4.90,false,TextureType::FG_GREENERY_NEAR_OBJECT);
    m_elements.emplace_back(9600,4600,3200,1440,4.90,4.90,false,TextureType::FG_GREENERY_NEAR_OBJECT);



    //    m_elements.emplace_back(1400,1600,500,50,1.10,1.25,false);
//    m_elements.emplace_back(2100,1600,500,50,1.10,1.25,false);
//    m_elements.emplace_back(2800,1600,500,50,1.10,1.25,false);
//    m_elements.emplace_back(3500,1600,500,50,1.10,1.25,false);
//
//    m_elements.emplace_back(700,2100,600,200,1.10,1.75,false);
//    m_elements.emplace_back(1400,2100,600,200,1.10,1.75,false);
//    m_elements.emplace_back(2100,2100,600,200,1.10,1.75,false);
//    m_elements.emplace_back(2800,2100,600,200,1.10,1.75,false);
//    m_elements.emplace_back(3500,2100,500,200,1.10,1.75,false);


}

