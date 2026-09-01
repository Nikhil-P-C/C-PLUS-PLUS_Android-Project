//
// Created by LENOVO on 24-08-2026.
//

#include "ForeGroundBuilder.h"
#include "utils/utils.h"
#include <SDL3/SDL.h>
namespace {
    inline TextureType toTextureType(ForeGroundType type){
        switch (type) {
            case ForeGroundType::GREENERY_NEAR_GRASS :
                return TextureType::FG_GREENERY_NEAR_GRASS;
            case ForeGroundType::GREENERY_NEAR_OBJECT :
                return TextureType::FG_GREENERY_NEAR_OBJECT;
            case ForeGroundType::MEADOWS_NEAR_FLOWERS :
                return TextureType::FG_MEADOW_NEAR_FLOWERS;
            case ForeGroundType::CAVE_NEAR_OBJECT:
                return TextureType::FG_CAVE_NEAR_OBJECT;
        }
        return TextureType::COUNT;
    }
}
void ForeGroundBuilder::render(SDL_Renderer *renderer) {

    float camX = Camera::getInstance().getCamera().x;
    float camY = Camera::getInstance().getCamera().y;
    for(const auto element : m_elements){
        SDL_FRect dst{element.rect.x - camX * element.parallaxFloatX,
                      element.rect.y - camY * element.parallaxFloatY,
                      element.rect.w, element.rect.h};
//        SDL_FRect src{0.00f,0.00f,};
        SDL_Texture* texture = Engine::Get().getAssetManager().getTexture(toTextureType(element.type));
        SDL_RenderTexture(renderer,texture, nullptr,&dst);
    }

    SDL_SetGPURenderState(renderer, nullptr);
}

void ForeGroundBuilder::update(float dt) {

}

void ForeGroundBuilder::init(std::vector<ForeGroundElement>& elements) {

    m_elements = elements;



}

