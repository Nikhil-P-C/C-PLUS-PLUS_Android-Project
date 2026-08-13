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
    TRAMPOLINE,
    NONE
};
enum class TrapStatus{
    IDLE=0,
    HIT,
    ON,
    OFF,
    TRIGGERED,
    NONE
};
enum class PathAxis{
    AUTO=0,
    VERTICAL,
    HORIZONTAL,
    CIRCLE,
    NONE
};
enum class PathShape{
    RECT=0,
    LINE,
    ARC,
    CIRCLE,
    NONE
};
struct TrapFrameInfo{
    TextureType texture;
    int frameW, frameH;
    int frameCount;
    int frameDelay;
    bool loop;
};
struct Trap{
    Trap()=default;
    Trap(float x, float y, TrapType type, TrapStatus status,float startPath,float endPath,
         float speed,PathAxis axis,PathShape shape,ColliderType colliderType,float radius =0,bool showChain =false);
    float x ,y;
    TrapType type;
    TrapStatus status;

    PathAxis axis = PathAxis::AUTO;
    PathShape pathShape = PathShape::LINE;

    float startPath=0.00f,endPath=0.00f;//for RECT path this is used a opposite corner of starting corner
    float baseX =0.00f,baseY =0.00f;// this is same as spawn for path RECT it use as the starting corner
    float movingSpeed = 150.00f;
    float previousX =0.00f, previousY = 0.00f;
    float pathAngle = 0.00f; //only used for trap that should move in circle or swing arcs
    float radius = 0.00f;
    int pathIndex = 1;
    bool showChain =false;
    bool isMovingForward =true;
    bool isActivated =false;
    bool hasHitEnd =false;
    unsigned int lastTime = 0.00f;
    unsigned int lastSwitchTime =0.00f;
    int aniStartFrame =0.00f;
    int aniEndFrame=0.00f;
    bool aniDone= false;
    ColliderType colliderType;
};
const TrapFrameInfo* getTrapFrameInfo(TrapType type,TrapStatus status);
const TextureType getChainTexture(TrapType type);
class TrapBuilder{
public:
    std::vector<Trap>& getTraps();

    void init(const std::vector<Trap>& traps);

    void render(SDL_Renderer* renderer);

    void update(float dt);

    void renderChain(SDL_Renderer *renderer, const Trap &trap, int camX, int camY);

    void triggerFall(int trapIndex);

    void updatePath(float dt);

    void resetPlatforms(int trapIndex);

    SDL_FRect getTrapCollisionBox(const Trap& trap);

    SDL_FRect getHazardHitBox(const Trap& trap);

    SDL_FPoint getTrapDelta(int trapIndex);

    float checkFanForce(float playerX,float playerY,float playerW,float playerH,ParticleSystem& particleSystem);

    bool isSolid(int trapIndex);

    bool checkFireCollision(int trapIndex, float playerX, float playerY, float playerW, float playerH);

    bool checkHazard(float playerX ,float playerY,float playerW,float playerH,
                     TrapType& outType,gameMath::collisionSide& outSide);

    bool checkTrampolineBounce(int trapIndex,float playerX,float playerY,
                               float playerW,float playerH,ParticleSystem& particleSystem);

    constexpr bool trapHasPath(TrapType type){
        return type == TrapType::ROCK_HEAD || type == TrapType::SPIKE_HEAD ||
               type == TrapType::MOVING_PLATFORM_GREY || type == TrapType::MOVING_PLATFORM_BROWN||
               type == TrapType::SAW||type ==TrapType::SPIKE_BALL;
    }

    constexpr bool trapHasHit(TrapType type){
        return type == TrapType::FIRE || type ==TrapType::ROCK_HEAD|| type ==TrapType::SPIKE_HEAD ||
               type == TrapType::SAW|| type ==TrapType::SPIKE_BALL|| type == TrapType::SPIKES;
    }

    constexpr bool trapHasHazard(TrapType type,TrapStatus status){
        return type ==TrapType::SPIKE_HEAD||type ==TrapType::SPIKES||
               (type == TrapType::FIRE&& status == TrapStatus::ON)||
               type == TrapType::SPIKE_BALL||type == TrapType::SAW||
               (type == TrapType::ROCK_HEAD && status == TrapStatus::HIT);
    }

    gameMath::collisionSide resolveTrapCollision(int trapIndex,float& playerX, float& playerY,
                                                 float playerW, float playerH,float previousY,float velocityY);



private:
    std::vector<Trap> m_traps;
    int m_fireTimer =3000;
    int m_fanTimer =3000;
    int m_rockHeadTimer =1000;

};
