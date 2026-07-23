//
// Created by LENOVO on 23-07-2026.
//
#include "ParticleSystem.h"
#include "engine/Engine.h"
#include <SDL3/SDL.h>
#include "utils/utils.h"
#include <random>

static std::mt19937 rng(std::random_device{}());
void ParticleSystem::render(SDL_Renderer* renderer)
{

    int camX = (int)std::round(Camera::getInstance().getCamera().x);
    int camY = (int)std::round(Camera::getInstance().getCamera().y);
    SDL_FRect src {0,0,16,16};
    for(const auto& particle:m_particles)
    {
        if (!particle.isAlive)
            continue;
        if(particle.isAlive){
            LOGI("particle is alive");
        }
        if (!particle.isAlive)
            continue;
        float sizefactor = particle.life /particle.maxLife;
        float alpha = particle.life / particle.maxLife;
        float size = 70.00f*sizefactor;
        SDL_SetTextureAlphaMod(
                m_particleTexture,
                static_cast<Uint8>(alpha * 255)
        );
        SDL_FRect dustDst{particle.x-camX,particle.y-camY,size,size};
        SDL_RenderTexture(renderer,m_particleTexture, &src,&dustDst);
    }
}

void ParticleSystem::update(float dt)
{
    for(auto& particle:m_particles)
    {
        if(!particle.isAlive)
            continue;
        particle.life -= dt;
        if(particle.life <=0)
        {
            particle.isAlive = false;
            continue;
        }
        particle.x += particle.vX*dt;
        particle.y += particle.vY*dt;
    }
}

void ParticleSystem::emitOneJumpParticle(float feetX, float feetY)
{
    for(auto& particle:m_particles){
        if(!particle.isAlive){
            particle.isAlive = true;
            particle.x =feetX;
            particle.y =feetY;

            particle.vX =randomFloat(-100,100);
            particle.vY =randomFloat(0,-300);

            particle.life = 0.4f;
            particle.maxLife=0.4f;
            return;
        }
    }

}

void ParticleSystem::emitJumpDust(float feetX, float feetY)
{
    int burst =randomInt(2,4);
    for(int i = 0; i< burst;i++)
    {
        emitOneJumpParticle(feetX, feetY);
    }
}
void ParticleSystem::emitLeftDust(float feetX, float feetY)
{
    for(auto& particle:m_particles){
        if(!particle.isAlive){
            particle.isAlive = true;
            particle.x =feetX;
            particle.y =feetY;

            particle.vX =randomFloat(0,200);
            particle.vY =randomFloat(-100,100);

            particle.life = 0.4f;
            particle.maxLife=0.4f;
            return;
        }
    }
}

void ParticleSystem::emitRightDust(float feetX, float feetY) {
    for(auto& particle:m_particles){
        if(!particle.isAlive){
            particle.isAlive = true;
            particle.x =feetX;
            particle.y =feetY;

            particle.vX =randomFloat(-200,0);
            particle.vY =randomFloat(-100,100);

            particle.life = 0.4f;
            particle.maxLife=0.4f;
            return;
        }
    }
}

void ParticleSystem::emitLandDust(float leftFeetX, float leftFeetY,float rightFeetX,float rightFeetY)
{
    emitLeftDust(leftFeetX,leftFeetY);
    emitRightDust(rightFeetX,rightFeetY);
}

ParticleSystem::ParticleSystem()
{
    m_particleTexture = Engine::Get().getAssetManager().getTexture(TextureType::DUST_PARTICLE);

}



float ParticleSystem::randomFloat(float min, float max)
{
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}

int ParticleSystem::randomInt(int min, int max)
{
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}

