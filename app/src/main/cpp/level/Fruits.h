//
// Created by LENOVO on 24-07-2026.
//
#pragma once
#include <vector>
#include <SDL3/SDL.h>
#include "engine/Engine.h"
enum class FruitType{
    BANANA,
    APPLE,
    ORANGE,
    STRAWBERRY,
    COLLECTED
};
struct Fruit{
    Fruit(float x, float y, FruitType type);

    float x=0;
    float y=0;
    FruitType type =FruitType::BANANA;
};

class FruitBuilder{
public:
    std::vector<Fruit>& getFruits();
    void init(const std::vector<Fruit>& fruits);
    void render(SDL_Renderer* renderer);
    void update(float dt);

private:
    std::vector<Fruit> m_fruits;
    SDL_Texture* m_orangeTexture= nullptr;
    SDL_Texture* m_bananaTexture= nullptr;
    SDL_Texture* m_appleTexture = nullptr;
    SDL_Texture* m_strawberryTexture = nullptr;
    SDL_Texture* m_collectedTexture = nullptr;

    float m_spriteWidth=32,m_spriteHeight =32;
    int m_currentFrame=0;
    int m_endFrame=16;
    int m_animationDelay =50;//50ms
    unsigned int m_lasTime =0;
};