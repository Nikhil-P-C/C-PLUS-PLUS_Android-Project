//
// Created by LENOVO on 24-08-2026.
//
#include "BackGroundBuilder.h"
#include "utils/utils.h"
#include <SDL3/SDL.h>
namespace {
    inline TextureType toTextureType(BackGroundType type){
        switch (type) {
            case BackGroundType::GREENERY_SKY:
                return TextureType::BG_GREENERY_SKY;
            case BackGroundType::GREENERY_MOUNTAINS:
                return TextureType::BG_GREENERY_MOUNTAINS;
            case BackGroundType::GREENERY_TREES:
                return TextureType::BG_GREENERY_TREES;
            case BackGroundType::MEADOWS_SKY:
                return TextureType::BG_MEADOWS_SKY;
            case BackGroundType::MEADOWS_CLOUD:
                return TextureType::BG_MEADOWS_CLOUDS;
            case BackGroundType::MEADOWS_FAR_PLAIN:
                return TextureType::BG_MEADOWS_FAR_PLAIN;
            case BackGroundType::MEADOWS_FLOWER_FIELD:
                return TextureType::BG_MEADOWS_FLOWER_FIELD;
        }
        return TextureType::COUNT;
    }
}
void BackGroundBuilder::render(SDL_Renderer *renderer) {
    float camX = Camera::getInstance().getCamera().x;
    float camY = Camera::getInstance().getCamera().y;
    for(const auto element : m_elements){
        SDL_FRect dst{element.rect.x - camX * element.parallaxFloatX,
                      element.rect.y - camY * element.parallaxFloatY,
                      element.rect.w, element.rect.h};
//        SDL_FRect src{0.00f,0.00f,};
        SDL_Texture* texture = Engine::Get().getAssetManager().getTexture(toTextureType(element.type));
        SDL_RenderTexture(renderer,texture, nullptr,&dst);
        if(element.type == BackGroundType::GREENERY_MOUNTAINS){//try fill gap ,manually
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
    //comment this when creating objects here
    m_elements = elements;

    //test view here by creating elements directly if perfect then put the same in level file

}

