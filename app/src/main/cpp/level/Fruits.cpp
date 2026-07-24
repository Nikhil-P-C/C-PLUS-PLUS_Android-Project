
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
        if(fruit.collectedAniDone) continue;
        SDL_FRect fruitDst{fruit.x-camX,fruit.y-camY,m_spriteWidth*SCALE,m_spriteHeight*SCALE};
        SDL_FRect fruitSrc{0.00f+m_spriteWidth*m_currentFrame,0.00f,m_spriteWidth,m_spriteHeight};
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
            fruitSrc={0.00f+m_spriteWidth*fruit.collectedAniFrame,0.00f,m_spriteWidth,m_spriteHeight};
            SDL_RenderTexture(renderer, m_collectedTexture,&fruitSrc,&fruitDst);
        }
//        SDL_FRect border{(fruit.x+35)-camX,(fruit.y+35)-camY,(m_spriteWidth*SCALE)-70,(m_spriteHeight*SCALE)-70};
//        SDL_RenderRect(renderer,&border);
//        LOGI("fruit rendered , frame:%d",m_currentFrame);
    }
}

void FruitBuilder::update(float dt)
{
    unsigned int now = SDL_GetTicks();
    if(now -m_lasTime >m_animationDelay)
    {
        m_currentFrame = (m_currentFrame +1) % m_endFrame;
        m_lasTime =now;
    }
    unsigned int collectedAniNowTime=SDL_GetTicks();
    for(auto& fruit:m_fruits)
    {
        if(fruit.consumed)
        {
            if (collectedAniNowTime - fruit.lastCollect > m_animationDelay)
            {
                if (fruit.collectedAniFrame < m_collectedAniEndFrame)
                {
                    fruit.collectedAniFrame++;
                    fruit.lastCollect =collectedAniNowTime;
                }
                else
                {
                    fruit.collectedAniDone = true;
                }
            }
        }
    }
}
int FruitBuilder::onCollision(float x, float y, float w, float h)
{
    int totalConsumedFruit =0;
    for(auto& fruit:m_fruits)
    {
        if(fruit.collectedAniDone)
            continue;
        SDL_FRect fruitcollider{(fruit.x+35),(fruit.y+35),(m_spriteWidth*SCALE)-70,(m_spriteHeight*SCALE)-70};
        bool collision = gameMath::checkcollision(x,y,fruitcollider.x,fruitcollider.y,
                                                  h,w,fruitcollider.h,fruitcollider.w);
        if(collision) {

            if(fruit.type != FruitType::COLLECTED)
                totalConsumedFruit++;
            fruit.consumed=true;
            fruit.type = FruitType::COLLECTED;
        }

    }
    LOGI("total fruit consumed:%d",totalConsumedFruit);

    return totalConsumedFruit;
}
void FruitBuilder::init(const std::vector<Fruit> &fruits)
{

    m_fruits =fruits;

    m_bananaTexture =Engine::Get().getAssetManager().getTexture(TextureType::FRUIT_BANANA);
    m_appleTexture =Engine::Get().getAssetManager().getTexture(TextureType::FRUIT_APPLE);
    m_orangeTexture =Engine::Get().getAssetManager().getTexture(TextureType::FRUIT_ORANGE);
    m_strawberryTexture =Engine::Get().getAssetManager().getTexture(TextureType::FRUIT_STRAWBERRY);
    m_collectedTexture =Engine::Get().getAssetManager().getTexture(TextureType::FRUIT_COLLECTED);

}




Fruit::Fruit(float x, float y, FruitType type):x(x),y(y),type(type){
}
