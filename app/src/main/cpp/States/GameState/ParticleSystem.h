//
// Created by LENOVO on 23-07-2026.
//
#pragma once
#include "SDL3/SDL.h"
#include <array>

struct Particle{
    bool isAlive =false;

    float x=0,y=0;
    float vX=0,vY=0;

    float life=0.04f;
    float maxLife =0.4;
};
class ParticleSystem{
public:
    void render(SDL_Renderer* renderer);
    void update(float dt);

    void emitJumpDust(float feetX,float feetY);
    void emitLeftDust(float feetX,float feetY);
    void emitRightDust(float feetX ,float feetY);
    void emitLandDust(float leftFeetX, float leftFeetY,float rightFeetX,float rightFeetY);
    void emitOneJumpParticle(float feetX, float feetY);
    void emitParticleWProps(int count,int rangeX1,int rangeX2,int rangeY1, int rangeY2,
                            int velocityX1,int velocityX2 , int velocityY1,int velocityY2,
                            int minParticleNum,int maxParticleNum,int dirX,int dirY);
    void emitDust(float pointX, float pointY, int velocityX, int velocityY);
    int randomInt(int min, int max);
    float randomFloat(float min, float max);

    ParticleSystem();
private:
    SDL_Texture* m_particleTexture=nullptr;
    std::array<Particle,50> m_particles;


};