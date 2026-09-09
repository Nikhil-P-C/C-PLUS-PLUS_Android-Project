//
// Created by LENOVO on 08-09-2026.
//
#pragma once
#include <vector>
#include "Traps.h"
#include "engine/Engine.h"
#include <SDL3/SDL.h>

enum class EnemyType{
    //Moss cave enemies
    SPIDER_CHILD,
    MOSS_GLOW_WORM,
    MOSS_BAT,

    //Stone cave enemies
    SINNERS_CHILD,
    CAVE_PREDATOR,
    CAVE_RAT,

    //Wooden Shaft mine enemies
    SHAFTS_RAPTOR,
    GIANT_BATS,
    SHAFTS_WORKER,

    //PLAINS enemies
    ANKYLOSAURUS_CHILD,
    HUNTER_RAPTOR,
    HUNTER_EAGLE,

    //WOODS enemies,
    PREDATOR_THEROPOD,
    THEROPOD_CHILD,
    PREDATOR_CAT,

    //MOUNTAIN enemies,
    MOUNTAIN_CAT,
    CROCODYLOMORPH,
    TARBOSAURUS_CHILD,

    //PEAK enemies,
    TYRANNOSAURUS_REX_CHILD,
    ALBERTOSAURUS,
    DASPELTOSAURUS,

    NONE
};
enum class combatType{
    AERIAL=0,
    GROUND,
    NONE
};
enum class EnemyAction{
    PATROL=0,
    SEEK,
    ATTACK,
    RETURN,
    NONE
};
enum class AttackType{
    SLASH=0,
    BITE,
    DASH,
    BEAM,
    PROJECTILE,
    SHOOT,
    NONE,
};
struct EnemyFrameInfo{
    TextureType texture;
    int frameW, frameH;
    int frameCount;
    int frameDelay;
    bool loop;
};

struct EnemyAIConfig {
    float detectRadius;
    float loseRadius;
    float leashRadius;
    float attackRange;
    unsigned int attackCooldownMs;

    AttackType attacks[2];        // e.g. {BITE, DASH} for MOSS_BAT
    float attackRanges[2];        // range each attack triggers at
};

const EnemyAIConfig* getEnemyAIConfig(EnemyType type);
const EnemyFrameInfo* getEnemyFrameInfo(EnemyType type,EnemyAction Action,AttackType attackType);

struct Enemy{
    Enemy(float x,float y,EnemyType type,EnemyAction action,AttackType attackType,
          float startPath,float endPath,float speed,PathAxis axis,PathShape shape,float radius =0);


    float x=0.00f,y=0.00f;

    EnemyType type = EnemyType::NONE;
    EnemyAction action =EnemyAction::NONE;
    AttackType attackType =AttackType::NONE;

    int hp = 50;
    // copied from trap , check traps for usage
    float startPath=0.00f,endPath=0.00f;
    float baseX =0.00f,baseY =0.00f;
    float movingSpeed = 150.00f;
    float previousX = 0.00f, previousY = 0.00f;
    float pathAngle = 0.00f;
    float radius = 0.00f;

    int pathIndex = 1;
    bool isMovingForward =true;
    bool isActivated =false;
    bool isFacingRight =true;
    bool hasHitEnd =false;

    bool aniDone= false;

    PathAxis axis = PathAxis::AUTO;
    PathShape pathShape = PathShape::LINE;

    unsigned int lastTime = 0;
    unsigned int lastSwitchTime =0;
    int aniStartFrame =0;
    int aniEndFrame=0;

};
class EnemiesBuilder{
public:

    void init(const std::vector<Enemy> enemies);

    void render(SDL_Renderer* renderer);

    void update(float dt,float playerX,float playerY);

private:

    void moveToward(Enemy &e, float targetX, float targetY, float speed, float dt);

    void updateAI(Enemy &e, float playerX, float playerY, float dt);

    void updatePath(Enemy& enemy, float dt);

private:
    std::vector<Enemy> m_enemies;

};