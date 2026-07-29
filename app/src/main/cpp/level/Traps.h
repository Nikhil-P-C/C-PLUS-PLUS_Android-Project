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
enum class PathAxis{
    AUTO=0,
    VERTICAL,
    HORIZONTAL
};
enum class PathShape{
    RECT,
    LINE
};
struct TrapFrameInfo{
    TextureType texture;
    int frameW, frameH;
    int frameCount;
    int frameDelay;
    bool loop;
};
struct Trap{
    Trap(float x, float y, TrapType type, TrapStatus status,float startPath,float endPath,
         float speed,PathAxis axis,PathShape shape,ColliderType colliderType);
    float x ,y;
    TrapType type;
    TrapStatus status;

    PathAxis axis = PathAxis::AUTO;
    PathShape pathShape = PathShape::LINE;

    float startPath=0.00f,endPath=0.00f;
    float baseX =0.00f,baseY =0.00f;
    float movingSpeed = 150.00f;
    float previousX =0.00f, previousY = 0.00f;
    int pathIndex = 1;
    bool isMovingForward =true;
    bool hasHitEnd =false;
    unsigned int lastTime = 0.00f;
    unsigned int lastSwitchTime =0.00f;
    int aniStartFrame =0.00f;
     int aniEndFrame=0.00f;
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
    constexpr bool trapHasHazard(TrapType type,TrapStatus status){
        return type ==TrapType::SPIKE_HEAD||type ==TrapType::SPIKES||
        type == TrapType::SPIKE_BALL||type == TrapType::SAW||
                (type == TrapType::ROCK_HEAD && status == TrapStatus::HIT);
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
    void updatePath(float dt);
    SDL_FPoint getTrapDelta(int trapIndex);
private:
    std::vector<Trap> m_traps;
    int m_fireTimer =3000;
    int m_fanTimer =3000;
    int m_rockHeadTimer =2000;
};
