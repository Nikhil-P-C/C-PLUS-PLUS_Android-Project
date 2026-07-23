
//
// Created by LENOVO on 24-07-2026.
//
#include <SDL3/SDL.h>
#include "Fruits.h"
#include "States/GameState/GameState.h"
#include "utils/utils.h"
std::vector<Fruit>& FruitBuilder::getFruits()
{
    return m_fruits;
}

void FruitBuilder::render(SDL_Renderer *renderer) {
    int camX = (int)std::round(Camera::getInstance().getCamera().x);
    int camY = (int)std::round(Camera::getInstance().getCamera().y);
    for(const auto& fruit : m_fruits){

        SDL_FRect fruitDst{fruit.x-camX,fruit.y-camY,m_spriteWidth*SCALE,m_spriteHeight*SCALE};
        LOGI("fruit dst: x:%f,y:%f,w:%f,h:%f",fruitDst.x,fruitDst.y,fruitDst.w,fruitDst.h);
        SDL_FRect fruitSrc{0.00f+m_spriteWidth*m_currentFrame,0.00f,m_spriteWidth,m_spriteHeight};
        LOGI("fruit src: x:%f,y:%f,w:%f,h:%f",fruitSrc.x,fruitSrc.y,fruitSrc.w,fruitSrc.h);
        if(fruit.type == FruitType::BANANA)
            SDL_RenderTexture(renderer,m_bananaTexture,&fruitSrc,&fruitDst);
        else if(fruit.type == FruitType::APPLE)
            SDL_RenderTexture(renderer,m_appleTexture,&fruitSrc,&fruitDst);
        else if(fruit.type == FruitType::ORANGE)
            SDL_RenderTexture(renderer,m_orangeTexture,&fruitSrc,&fruitDst);
        else if(fruit.type == FruitType::STRAWBERRY)
            SDL_RenderTexture(renderer,m_strawberryTexture,&fruitSrc,&fruitDst);
        else if(fruit.type == FruitType::COLLECTED)
        {
            fruitSrc={0.00f+m_spriteWidth*m_collectedStartFrame,0.00f,m_spriteWidth,m_spriteHeight};
            SDL_RenderTexture(renderer, m_collectedTexture,&fruitSrc,&fruitDst);
        }
//        SDL_FRect border{(fruit.x+35)-camX,(fruit.y+35)-camY,(m_spriteWidth*SCALE)-70,(m_spriteHeight*SCALE)-70};
//        SDL_RenderRect(renderer,&border);
//        LOGI("fruit rendered , frame:%d",m_currentFrame);
    }
}

void FruitBuilder::update(float dt) {
    unsigned int now = SDL_GetTicks();
    if(now -m_lasTime >m_animationDelay){
        m_currentFrame = (m_currentFrame +1) % m_endFrame;
        m_collectedStartFrame = (m_collectedStartFrame +1)%m_collectedEndFrame;
        m_lasTime =now;
    }

}
void FruitBuilder::onCollision(float x, float y, float w, float h) {
    for(auto& fruit:m_fruits){
        if(fruit.consumed)
            continue;
        SDL_FRect fruitcollider{(fruit.x+35),(fruit.y+35),(m_spriteWidth*SCALE)-70,(m_spriteHeight*SCALE)-70};
        bool collision = gameMath::checkcollision(x,y,fruitcollider.x,fruitcollider.y,
                                                  h,w,fruitcollider.h,fruitcollider.w);
        if(collision)
            fruit.type= FruitType::COLLECTED;
    }
}
void FruitBuilder::init(const std::vector<Fruit> &fruits) {

    m_fruits =fruits;

    m_bananaTexture =Engine::Get().getAssetManager().getTexture(TextureType::FRUIT_BANANA);
    m_appleTexture =Engine::Get().getAssetManager().getTexture(TextureType::FRUIT_APPLE);
    m_orangeTexture =Engine::Get().getAssetManager().getTexture(TextureType::FRUIT_ORANGE);
    m_strawberryTexture =Engine::Get().getAssetManager().getTexture(TextureType::FRUIT_STRAWBERRY);
    m_collectedTexture =Engine::Get().getAssetManager().getTexture(TextureType::FRUIT_COLLECTED);

}




Fruit::Fruit(float x, float y, FruitType type):x(x),y(y),type(type){
}
