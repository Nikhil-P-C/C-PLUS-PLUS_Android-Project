//
// Created by LENOVO on 24-07-2026.
//
#pragma once
#include <vector>
#include <unordered_map>
#include <SDL3/SDL.h>
#include "engine/Engine.h"
#include "utils/utils.h"
#include "States/GameState/ParticleSystem.h"

enum class TrapType{
    FALLING_PLATFORM=0,
    FAN,
    FIRE,
    MOVING_PLATFORM_BROWN,
    MOVING_PLATFORM_GREY,
    ROCK_HEAD,
    SAW,
    SPIKE_HEAD,
    SPIKE_BALL,
    SPIKES,
    TRAMPOLINE
};
enum class TrapStatus{
    IDLE=0,
    HIT,
    ON,
    OFF,
    TRIGGERED,
};
struct TrapFrameInfo{
    TextureType texture;
    int frameW, frameH;
    int frameCount;
    int frameDelay;
    bool loop;
};
struct Trap{
    Trap(float x, float y, TrapType type, TrapStatus status,float startPath,float endPath,ColliderType colliderType);
    float x ,y;
    TrapType type;
    TrapStatus status;
    float startPath=0,endPath=0;
    unsigned int lastTime = 0;
    unsigned int lastSwitchTime =0;
    int aniStartFrame =0;
     int aniEndFrame=0;
    bool aniDone= false;
    ColliderType colliderType;
};
const TrapFrameInfo* getTrapFrameInfo(TrapType type,TrapStatus status);
class TrapBuilder{
public:
    std::vector<Trap>& getTraps();
    void init(const std::vector<Trap>& traps);
    void render(SDL_Renderer* renderer);
    void update(float dt);
    constexpr bool trapHasPath(TrapType type){
        return type == TrapType::ROCK_HEAD || type == TrapType::SPIKE_HEAD ||
        type == TrapType::MOVING_PLATFORM_GREY || type == TrapType::MOVING_PLATFORM_BROWN;
    }
    constexpr bool trapHasHit(TrapType type){
        return type == TrapType::FIRE || type ==TrapType::ROCK_HEAD|| type ==TrapType::SPIKE_HEAD;
    }

    bool isSolid(int trapIndex);
    SDL_FRect getTrapCollisionBox(const Trap& trap);
    void triggerFall(int trapIndex);
    bool checkHazard(float playerX ,float playerY,float playerW,float playerH,TrapType& outType);
    SDL_FRect getHazardHitBox(const Trap& trap);
    float checkFanForce(float playerX,float playerY,float playerW,float playerH,ParticleSystem& particleSystem);
    bool checkTrampolineBounce(int trapIndex,float playerX,float playerY,
                               float playerW,float playerH,ParticleSystem& particleSystem);
    bool checkFireCollision(int trapIndex, float playerX, float playerY, float playerW, float playerH);
    gameMath::collisionSide resolveTrapCollision(int trapIndex,float& playerX, float& playerY,
                                                 float playerW, float playerH,float previousY,float velocityY);
private:
    std::vector<Trap> m_traps;
    int m_fireTimer =3000;
    int m_fanTimer =3000;
};
