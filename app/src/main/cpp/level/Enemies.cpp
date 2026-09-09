//
// Created by LENOVO on 08-09-2026.
//
#include "Enemies.h"


namespace {
    inline uint32_t enemyActionKey(EnemyType type, EnemyAction action)
    {
        return (static_cast<uint32_t>(type) << 8 | static_cast<uint32_t>(action));
    }
    inline uint32_t enemyAttackKey(EnemyType type, AttackType attack)
    {
        return (static_cast<uint32_t>(type) << 8 | static_cast<uint32_t>(attack));
    }
}

void EnemiesBuilder::init(const std::vector<Enemy> enemies) {
    m_enemies.emplace_back(700.0f,600.0f,EnemyType::SINNERS_CHILD,EnemyAction::PATROL,AttackType::SLASH,
                           0.00f,600.00f,250.00f,PathAxis::HORIZONTAL,PathShape::LINE);
}

void EnemiesBuilder::render(SDL_Renderer *renderer) {
    int camX = (int)std::round(Camera::getInstance().getCamera().x);
    int camY = (int)std::round(Camera::getInstance().getCamera().y);
    for(const auto& enemy: m_enemies){
        auto info = getEnemyFrameInfo(enemy.type,enemy.action,enemy.attackType);
        if(info == nullptr) {
            LOGI("enemy info null");
            continue;
        }
        SDL_FRect dst{enemy.x-camX,enemy.y-camY,static_cast<float>(info->frameW),static_cast<float>(info->frameH)};
        SDL_SetRenderDrawColor(renderer,255,0,0,255);
        SDL_RenderFillRect(renderer,&dst);
    }

}

void EnemiesBuilder::update(float dt,float playerX,float playerY) {
    for(auto& enemy:m_enemies){
        updateAI(enemy,playerX,playerY,dt);
        if(enemy.action == EnemyAction::ATTACK){
            LOGI("enemy is Attacking");
        }
    }
}
void EnemiesBuilder::updateAI(Enemy& enemy, float playerX, float playerY, float dt) {
    float dx = playerX - enemy.x, dy = playerY - enemy.y;
    float distToPlayer = std::sqrt(dx*dx + dy*dy);
    const auto cfg = getEnemyAIConfig(enemy.type);

    switch (enemy.action) {
        case EnemyAction::PATROL:
            updatePath(enemy, dt);                       // existing trap-style movement
            if (distToPlayer <= cfg->detectRadius)
                enemy.action = EnemyAction::SEEK;
            break;

        case EnemyAction::SEEK: {
            moveToward(enemy, playerX, playerY, enemy.movingSpeed, dt);
            enemy.isFacingRight = (dx > 0);

            float homeDx = enemy.x - enemy.baseX, homeDy = enemy.y - enemy.baseY;
            float distFromHome = std::sqrt(homeDx*homeDx + homeDy*homeDy);

            if (distToPlayer <= cfg->attackRange)
                enemy.action = EnemyAction::ATTACK;
            else if (distToPlayer > cfg->loseRadius)
                enemy.action = EnemyAction::RETURN;
            break;
        }

        case EnemyAction::ATTACK:
            if (distToPlayer > cfg->attackRange * 1.2f)      // player escaped mid-attack
                enemy.action = EnemyAction::SEEK;
            else if (enemy.aniDone)                              // let animation drive the timing
                enemy.action = EnemyAction::SEEK;                 // reposition, re-enter ATTACK if still in range
            break;

        case EnemyAction::RETURN: {
            moveToward(enemy, enemy.baseX, enemy.baseY, enemy.movingSpeed, dt);
            float rdx = enemy.baseX - enemy.x, rdy = enemy.baseY - enemy.y;
            if (std::sqrt(rdx*rdx + rdy*rdy) < 4.0f) {
                enemy.x = enemy.baseX; enemy.y = enemy.baseY;
                enemy.action = EnemyAction::PATROL;   // pathIndex/hasHitEnd were untouched during the chase, so it resumes cleanly
            } else if (distToPlayer <= cfg->detectRadius) {
                enemy.action = EnemyAction::SEEK;     // player wandered back in range while it was heading home
            }
            break;
        }
        default: break;
    }
}

void EnemiesBuilder::moveToward(Enemy& enemy, float targetX, float targetY, float speed, float dt) {
    float dx = targetX - enemy.x, dy = targetY - enemy.y;
    float dist = std::sqrt(dx*dx + dy*dy);
    if (dist < 0.01f) return;
    enemy.x += (dx/dist) * speed * dt;
    enemy.y += (dy/dist) * speed * dt;
}

void EnemiesBuilder::updatePath(Enemy& enemy,float dt)
{

    enemy.previousX=enemy.x;
    enemy.previousY=enemy.y;

    if(enemy.pathShape == PathShape::RECT)
    {
        SDL_FPoint paths[4]={
                {enemy.baseX,enemy.baseY},
                {enemy.startPath,enemy.baseY},
                {enemy.startPath,enemy.endPath},
                {enemy.baseX,enemy.endPath}
        };
        SDL_FPoint target = paths[enemy.pathIndex];

        float stepDist = enemy.movingSpeed*dt;
        if(enemy.x != target.x)
        {
            float dir = (target.x > enemy.x) ? 1.0f : -1.0f;
            if(SDL_fabsf(target.x - enemy.x) <= stepDist)
            {
                enemy.x = target.x;
            }
            else
            {
                enemy.x += stepDist*dir;
            }
        }
        if(enemy.y != target.y)
        {
            float dir = (target.y > enemy.y) ? 1.0f : -1.0f;
            if(SDL_fabsf(target.y - enemy.y) <= stepDist)
            {
                enemy.y = target.y;
            }
            else
            {
                enemy.y += stepDist*dir;
            }
        }
        if(target.x == enemy.x && target.y == enemy.y)
        {
            unsigned int now = SDL_GetTicks();
            if(!enemy.hasHitEnd)
            {
                enemy.hasHitEnd =true;
                enemy.lastSwitchTime=now;

                enemy.aniStartFrame = 0;
                enemy.aniDone = false;
            }
            else
                enemy.pathIndex = (enemy.pathIndex + 1) % 4;

        }
        else{
            enemy.hasHitEnd =false;
        }
        return;
    }

    if(enemy.pathShape == PathShape::LINE)
    {

        float& coord = (enemy.axis == PathAxis::HORIZONTAL)?enemy.x:enemy.y;
        float target = (enemy.isMovingForward)?enemy.endPath:enemy.startPath;
        float dir = (coord < target) ? 1.00f : -1.00f;

        float stepDist = enemy.movingSpeed*dt;

        if(SDL_fabsf(target - coord) <= stepDist)
        {
            unsigned int now = SDL_GetTicks();
            coord = target;
            if(!enemy.hasHitEnd)
            {
                enemy.hasHitEnd =true;
                enemy.lastSwitchTime=now;

                enemy.aniStartFrame = 0;
                enemy.aniDone = false;

                enemy.isMovingForward = !enemy.isMovingForward;
            }

        }
        else
        {
            enemy.hasHitEnd =false;

            coord += stepDist*dir;

        }
    }
    if(enemy.pathShape == PathShape::CIRCLE)
    {// no target we just move endlessly
        enemy.pathAngle += enemy.movingSpeed/enemy.radius *dt;
        enemy.x =enemy.baseX +enemy.radius *cosf(enemy.pathAngle);
        enemy.y =enemy.baseY +enemy.radius * sinf(enemy.pathAngle);
    }
    if(enemy.pathShape == PathShape::ARC)
    {
        // endPath/startPath as angle bounds (radians)
        float target = enemy.isMovingForward ? enemy.endPath : enemy.startPath;

        float dir = (enemy.pathAngle < target) ? 1.0f : -1.0f;
        float angularSpeed = enemy.movingSpeed / enemy.radius;
        float step = angularSpeed * dt;

        if(SDL_fabsf(target - enemy.pathAngle) <= step) {
            enemy.pathAngle = target;
            enemy.isMovingForward = !enemy.isMovingForward;
        } else {
            enemy.pathAngle += step * dir;
        }
        enemy.x = enemy.baseX + enemy.radius * cosf(enemy.pathAngle);
        enemy.y = enemy.baseY + enemy.radius * sinf(enemy.pathAngle);
    }
//    for(auto& enemy: m_enemies)
//    {
//        if(enemy.action != EnemyAction::PATROL)
//            continue;
//
//        enemy.previousX=enemy.x;
//        enemy.previousY=enemy.y;
//
//        if(enemy.pathShape == PathShape::RECT)
//        {
//            SDL_FPoint paths[4]={
//                    {enemy.baseX,enemy.baseY},
//                    {enemy.startPath,enemy.baseY},
//                    {enemy.startPath,enemy.endPath},
//                    {enemy.baseX,enemy.endPath}
//            };
//            SDL_FPoint target = paths[enemy.pathIndex];
//
//            float stepDist = enemy.movingSpeed*dt;
//            if(enemy.x != target.x)
//            {
//                float dir = (target.x > enemy.x) ? 1.0f : -1.0f;
//                if(SDL_fabsf(target.x - enemy.x) <= stepDist)
//                {
//                    enemy.x = target.x;
//                }
//                else
//                {
//                    enemy.x += stepDist*dir;
//                }
//            }
//            if(enemy.y != target.y)
//            {
//                float dir = (target.y > enemy.y) ? 1.0f : -1.0f;
//                if(SDL_fabsf(target.y - enemy.y) <= stepDist)
//                {
//                    enemy.y = target.y;
//                }
//                else
//                {
//                    enemy.y += stepDist*dir;
//                }
//            }
//            if(target.x == enemy.x && target.y == enemy.y)
//            {
//                unsigned int now = SDL_GetTicks();
//                if(!enemy.hasHitEnd)
//                {
//                    enemy.hasHitEnd =true;
//                    enemy.lastSwitchTime=now;
//
//                    enemy.aniStartFrame = 0;
//                    enemy.aniDone = false;
//                }
//                else
//                    enemy.pathIndex = (enemy.pathIndex + 1) % 4;
//
//            }
//            else{
//                enemy.hasHitEnd =false;
//            }
//            continue;
//        }
//
//        if(enemy.pathShape == PathShape::LINE)
//        {
//
//            float& coord = (enemy.axis == PathAxis::HORIZONTAL)?enemy.x:enemy.y;
//            float target = (enemy.isMovingForward)?enemy.endPath:enemy.startPath;
//            float dir = (coord < target) ? 1.00f : -1.00f;
//
//            float stepDist = enemy.movingSpeed*dt;
//
//            if(SDL_fabsf(target - coord) <= stepDist)
//            {
//                unsigned int now = SDL_GetTicks();
//                coord = target;
//                if(!enemy.hasHitEnd)
//                {
//                    enemy.hasHitEnd =true;
//                    enemy.lastSwitchTime=now;
//
//                    enemy.aniStartFrame = 0;
//                    enemy.aniDone = false;
//
//                    enemy.isMovingForward = !enemy.isMovingForward;
//                }
//
//            }
//            else
//            {
//                enemy.hasHitEnd =false;
//
//                coord += stepDist*dir;
//
//            }
//        }
//        if(enemy.pathShape == PathShape::CIRCLE)
//        {// no target we just move endlessly
//            enemy.pathAngle += enemy.movingSpeed/enemy.radius *dt;
//            enemy.x =enemy.baseX +enemy.radius *cosf(enemy.pathAngle);
//            enemy.y =enemy.baseY +enemy.radius * sinf(enemy.pathAngle);
//        }
//        if(enemy.pathShape == PathShape::ARC)
//        {
//            // endPath/startPath as angle bounds (radians)
//            float target = enemy.isMovingForward ? enemy.endPath : enemy.startPath;
//
//            float dir = (enemy.pathAngle < target) ? 1.0f : -1.0f;
//            float angularSpeed = enemy.movingSpeed / enemy.radius;
//            float step = angularSpeed * dt;
//
//            if(SDL_fabsf(target - enemy.pathAngle) <= step) {
//                enemy.pathAngle = target;
//                enemy.isMovingForward = !enemy.isMovingForward;
//            } else {
//                enemy.pathAngle += step * dir;
//            }
//            enemy.x = enemy.baseX + enemy.radius * cosf(enemy.pathAngle);
//            enemy.y = enemy.baseY + enemy.radius * sinf(enemy.pathAngle);
//        }
//    }
}



Enemy::Enemy(float x, float y, EnemyType type, EnemyAction action, AttackType attackType,
             float startPath, float endPath, float speed, PathAxis axis, PathShape shape,
             float radius)
             :x(x),y(y),type(type),action(action),attackType(attackType),startPath(startPath),endPath(endPath),
             movingSpeed(speed),axis(axis),pathShape(shape),radius(radius),baseX(x),baseY(y),previousX(x),previousY(y){

}

const EnemyFrameInfo* getEnemyFrameInfo(EnemyType type, EnemyAction Action, AttackType attackType) {
    if(Action == EnemyAction::ATTACK){
        static const std::unordered_map<uint32_t, EnemyFrameInfo> attackTable{
                {enemyAttackKey(EnemyType::SPIDER_CHILD,AttackType::SHOOT), {TextureType::ENEMY_SPIDER_CHILD_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::SPIDER_CHILD,AttackType::BITE), {TextureType::ENEMY_SPIDER_CHILD_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::MOSS_GLOW_WORM,AttackType::DASH), {TextureType::ENEMY_MOSS_GLOW_WORM_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::MOSS_GLOW_WORM,AttackType::BITE), {TextureType::ENEMY_MOSS_GLOW_WORM_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::MOSS_BAT,AttackType::SLASH), {TextureType::ENEMY_MOSS_BAT_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::MOSS_BAT,AttackType::BITE), {TextureType::ENEMY_MOSS_BAT_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::SINNERS_CHILD,AttackType::SLASH), {TextureType::ENEMY_SINNERS_CHILD_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::SINNERS_CHILD,AttackType::BEAM), {TextureType::ENEMY_SINNERS_CHILD_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::CAVE_PREDATOR,AttackType::SLASH), {TextureType::ENEMY_CAVE_PREDATOR_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::CAVE_PREDATOR,AttackType::BITE), {TextureType::ENEMY_CAVE_PREDATOR_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::CAVE_RAT,AttackType::DASH), {TextureType::ENEMY_CAVE_RAT_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::CAVE_RAT,AttackType::BITE), {TextureType::ENEMY_CAVE_RAT_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::SHAFTS_RAPTOR,AttackType::SLASH), {TextureType::ENEMY_SHAFTS_RAPTOR_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::SHAFTS_RAPTOR,AttackType::BITE), {TextureType::ENEMY_SHAFTS_RAPTOR_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::GIANT_BATS,AttackType::SLASH), {TextureType::ENEMY_GIANT_BATS_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::GIANT_BATS,AttackType::BITE), {TextureType::ENEMY_GIANT_BATS_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::SHAFTS_WORKER,AttackType::SLASH), {TextureType::ENEMY_SHAFTS_WORKER_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::SHAFTS_WORKER,AttackType::PROJECTILE), {TextureType::ENEMY_SHAFTS_WORKER_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::HUNTER_RAPTOR,AttackType::BITE), {TextureType::ENEMY_HUNTER_RAPTOR_ATTACK2,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::HUNTER_RAPTOR,AttackType::SLASH), {TextureType::ENEMY_HUNTER_RAPTOR_ATTACK1,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::HUNTER_EAGLE,AttackType::SLASH), {TextureType::ENEMY_HUNTER_EAGLE_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::HUNTER_EAGLE,AttackType::DASH), {TextureType::ENEMY_HUNTER_EAGLE_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::PREDATOR_THEROPOD,AttackType::SLASH), {TextureType::ENEMY_PREDATOR_THEROPOD_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::PREDATOR_THEROPOD,AttackType::BITE), {TextureType::ENEMY_PREDATOR_THEROPOD_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::ANKYLOSAURUS_CHILD,AttackType::SLASH), {TextureType::ENEMY_ANKYLOSAURUS_CHILD_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::ANKYLOSAURUS_CHILD,AttackType::BITE), {TextureType::ENEMY_ANKYLOSAURUS_CHILD_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::THEROPOD_CHILD,AttackType::SLASH), {TextureType::ENEMY_THEROPOD_CHILD_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::THEROPOD_CHILD,AttackType::BITE), {TextureType::ENEMY_THEROPOD_CHILD_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::PREDATOR_CAT,AttackType::SLASH), {TextureType::ENEMY_PREDATOR_CAT_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::PREDATOR_CAT,AttackType::BITE), {TextureType::ENEMY_PREDATOR_CAT_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::MOUNTAIN_CAT,AttackType::DASH), {TextureType::ENEMY_MOUNTAIN_CAT_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::MOUNTAIN_CAT,AttackType::BITE), {TextureType::ENEMY_MOUNTAIN_CAT_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::CROCODYLOMORPH,AttackType::SLASH), {TextureType::ENEMY_CROCODYLOMORPH_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::CROCODYLOMORPH,AttackType::BITE), {TextureType::ENEMY_CROCODYLOMORPH_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::TARBOSAURUS_CHILD,AttackType::DASH), {TextureType::ENEMY_TARBOSAURUS_CHILD_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::TARBOSAURUS_CHILD,AttackType::BITE), {TextureType::ENEMY_TARBOSAURUS_CHILD_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::TYRANNOSAURUS_REX_CHILD,AttackType::BEAM), {TextureType::ENEMY_TYRANNOSAURUS_REX_CHILD_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::TYRANNOSAURUS_REX_CHILD,AttackType::BITE), {TextureType::ENEMY_TYRANNOSAURUS_REX_CHILD_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::ALBERTOSAURUS,AttackType::SLASH), {TextureType::ENEMY_ALBERTOSAURUS_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::ALBERTOSAURUS,AttackType::BITE), {TextureType::ENEMY_ALBERTOSAURUS_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::DASPELTOSAURUS,AttackType::SLASH), {TextureType::ENEMY_DASPELTOSAURUS_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::DASPELTOSAURUS,AttackType::BITE), {TextureType::ENEMY_DASPELTOSAURUS_ATTACK2,64,64,6,80,false}},
        };
        auto it = attackTable.find(enemyAttackKey(type, attackType));
        return it != attackTable.end() ? &it->second : nullptr;
    }

    static const std::unordered_map<uint32_t, EnemyFrameInfo> table{
            {enemyActionKey(EnemyType::SPIDER_CHILD,EnemyAction::PATROL), {TextureType::ENEMY_SPIDER_CHILD_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::SPIDER_CHILD,EnemyAction::SEEK), {TextureType::ENEMY_SPIDER_CHILD_SEEK,64,64,4,90,true}},
            {enemyActionKey(EnemyType::SPIDER_CHILD,EnemyAction::RETURN), {TextureType::ENEMY_SPIDER_CHILD_PATROL,64,64,4,90,true}},

            {enemyActionKey(EnemyType::MOSS_GLOW_WORM,EnemyAction::PATROL), {TextureType::ENEMY_MOSS_GLOW_WORM_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::MOSS_GLOW_WORM,EnemyAction::SEEK), {TextureType::ENEMY_MOSS_GLOW_WORM_SEEK,64,64,4,90,true}},
            {enemyActionKey(EnemyType::MOSS_GLOW_WORM,EnemyAction::RETURN), {TextureType::ENEMY_MOSS_GLOW_WORM_PATROL,64,64,4,120,true}},

            {enemyActionKey(EnemyType::MOSS_BAT,EnemyAction::PATROL), {TextureType::ENEMY_MOSS_BAT_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::MOSS_BAT,EnemyAction::SEEK), {TextureType::ENEMY_MOSS_BAT_SEEK,64,64,4,90,true}},
            {enemyActionKey(EnemyType::MOSS_BAT,EnemyAction::RETURN), {TextureType::ENEMY_MOSS_BAT_PATROL,64,64,4,120,true}},

            {enemyActionKey(EnemyType::SINNERS_CHILD,EnemyAction::PATROL), {TextureType::ENEMY_SINNERS_CHILD_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::SINNERS_CHILD,EnemyAction::SEEK), {TextureType::ENEMY_SINNERS_CHILD_SEEK,64,64,4,90,true}},
            {enemyActionKey(EnemyType::SINNERS_CHILD,EnemyAction::RETURN), {TextureType::ENEMY_SINNERS_CHILD_PATROL,64,64,4,120,true}},

            {enemyActionKey(EnemyType::CAVE_PREDATOR,EnemyAction::PATROL), {TextureType::ENEMY_CAVE_PREDATOR_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::CAVE_PREDATOR,EnemyAction::SEEK), {TextureType::ENEMY_CAVE_PREDATOR_SEEK,64,64,4,90,true}},
            {enemyActionKey(EnemyType::CAVE_PREDATOR,EnemyAction::RETURN), {TextureType::ENEMY_CAVE_PREDATOR_PATROL,64,64,4,120,true}},

            {enemyActionKey(EnemyType::CAVE_RAT,EnemyAction::PATROL), {TextureType::ENEMY_CAVE_RAT_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::CAVE_RAT,EnemyAction::SEEK), {TextureType::ENEMY_CAVE_RAT_SEEK,64,64,4,90,true}},
            {enemyActionKey(EnemyType::CAVE_RAT,EnemyAction::RETURN), {TextureType::ENEMY_CAVE_RAT_PATROL,64,64,4,120,true}},

            {enemyActionKey(EnemyType::SHAFTS_RAPTOR,EnemyAction::PATROL), {TextureType::ENEMY_SHAFTS_RAPTOR_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::SHAFTS_RAPTOR,EnemyAction::SEEK), {TextureType::ENEMY_SHAFTS_RAPTOR_SEEK,64,64,4,90,true}},
            {enemyActionKey(EnemyType::SHAFTS_RAPTOR,EnemyAction::RETURN), {TextureType::ENEMY_SHAFTS_RAPTOR_PATROL,64,64,4,120,true}},

            {enemyActionKey(EnemyType::GIANT_BATS,EnemyAction::PATROL), {TextureType::ENEMY_GIANT_BATS_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::GIANT_BATS,EnemyAction::SEEK), {TextureType::ENEMY_GIANT_BATS_SEEK,64,64,4,90,true}},
            {enemyActionKey(EnemyType::GIANT_BATS,EnemyAction::RETURN), {TextureType::ENEMY_GIANT_BATS_PATROL,64,64,4,120,true}},

            {enemyActionKey(EnemyType::SHAFTS_WORKER,EnemyAction::PATROL), {TextureType::ENEMY_SHAFTS_WORKER_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::SHAFTS_WORKER,EnemyAction::SEEK), {TextureType::ENEMY_SHAFTS_WORKER_SEEK,64,64,4,90,true}},
            {enemyActionKey(EnemyType::SHAFTS_WORKER,EnemyAction::RETURN), {TextureType::ENEMY_SHAFTS_WORKER_PATROL,64,64,4,120,true}},

            {enemyActionKey(EnemyType::ANKYLOSAURUS_CHILD,EnemyAction::PATROL), {TextureType::ENEMY_ANKYLOSAURUS_CHILD_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::ANKYLOSAURUS_CHILD,EnemyAction::SEEK), {TextureType::ENEMY_ANKYLOSAURUS_CHILD_SEEK,64,64,4,90,true}},
            {enemyActionKey(EnemyType::ANKYLOSAURUS_CHILD,EnemyAction::RETURN), {TextureType::ENEMY_ANKYLOSAURUS_CHILD_PATROL,64,64,4,120,true}},

            {enemyActionKey(EnemyType::HUNTER_RAPTOR,EnemyAction::PATROL), {TextureType::ENEMY_HUNTER_RAPTOR_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::HUNTER_RAPTOR,EnemyAction::SEEK), {TextureType::ENEMY_HUNTER_RAPTOR_SEEK,64,64,4,90,true}},
            {enemyActionKey(EnemyType::HUNTER_RAPTOR,EnemyAction::RETURN), {TextureType::ENEMY_HUNTER_RAPTOR_PATROL,64,64,4,120,true}},

            {enemyActionKey(EnemyType::HUNTER_EAGLE,EnemyAction::PATROL), {TextureType::ENEMY_HUNTER_EAGLE_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::HUNTER_EAGLE,EnemyAction::SEEK), {TextureType::ENEMY_HUNTER_EAGLE_SEEK,64,64,4,90,true}},
            {enemyActionKey(EnemyType::HUNTER_EAGLE,EnemyAction::RETURN), {TextureType::ENEMY_HUNTER_EAGLE_PATROL,64,64,4,120,true}},

            {enemyActionKey(EnemyType::PREDATOR_THEROPOD,EnemyAction::PATROL), {TextureType::ENEMY_PREDATOR_THEROPOD_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::PREDATOR_THEROPOD,EnemyAction::SEEK), {TextureType::ENEMY_PREDATOR_THEROPOD_SEEK,64,64,4,90,true}},
            {enemyActionKey(EnemyType::PREDATOR_THEROPOD,EnemyAction::RETURN), {TextureType::ENEMY_PREDATOR_THEROPOD_PATROL,64,64,4,120,true}},

            {enemyActionKey(EnemyType::THEROPOD_CHILD,EnemyAction::PATROL), {TextureType::ENEMY_THEROPOD_CHILD_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::THEROPOD_CHILD,EnemyAction::SEEK), {TextureType::ENEMY_THEROPOD_CHILD_SEEK,64,64,4,90,true}},
            {enemyActionKey(EnemyType::THEROPOD_CHILD,EnemyAction::RETURN), {TextureType::ENEMY_THEROPOD_CHILD_PATROL,64,64,4,120,true}},

            {enemyActionKey(EnemyType::PREDATOR_CAT,EnemyAction::PATROL), {TextureType::ENEMY_PREDATOR_CAT_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::PREDATOR_CAT,EnemyAction::SEEK), {TextureType::ENEMY_PREDATOR_CAT_SEEK,64,64,4,90,true}},
            {enemyActionKey(EnemyType::PREDATOR_CAT,EnemyAction::RETURN), {TextureType::ENEMY_PREDATOR_CAT_PATROL,64,64,4,120,true}},

            {enemyActionKey(EnemyType::MOUNTAIN_CAT,EnemyAction::PATROL), {TextureType::ENEMY_MOUNTAIN_CAT_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::MOUNTAIN_CAT,EnemyAction::SEEK), {TextureType::ENEMY_MOUNTAIN_CAT_SEEK,64,64,4,90,true}},
            {enemyActionKey(EnemyType::MOUNTAIN_CAT,EnemyAction::RETURN), {TextureType::ENEMY_MOUNTAIN_CAT_PATROL,64,64,4,120,true}},

            {enemyActionKey(EnemyType::CROCODYLOMORPH,EnemyAction::PATROL), {TextureType::ENEMY_CROCODYLOMORPH_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::CROCODYLOMORPH,EnemyAction::SEEK), {TextureType::ENEMY_CROCODYLOMORPH_SEEK,64,64,4,90,true}},
            {enemyActionKey(EnemyType::CROCODYLOMORPH,EnemyAction::RETURN), {TextureType::ENEMY_CROCODYLOMORPH_PATROL,64,64,4,120,true}},

            {enemyActionKey(EnemyType::TARBOSAURUS_CHILD,EnemyAction::PATROL), {TextureType::ENEMY_TARBOSAURUS_CHILD_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::TARBOSAURUS_CHILD,EnemyAction::SEEK), {TextureType::ENEMY_TARBOSAURUS_CHILD_SEEK,64,64,4,90,true}},
            {enemyActionKey(EnemyType::TARBOSAURUS_CHILD,EnemyAction::RETURN), {TextureType::ENEMY_TARBOSAURUS_CHILD_PATROL,64,64,4,120,true}},

            {enemyActionKey(EnemyType::TYRANNOSAURUS_REX_CHILD,EnemyAction::PATROL), {TextureType::ENEMY_TYRANNOSAURUS_REX_CHILD_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::TYRANNOSAURUS_REX_CHILD,EnemyAction::SEEK), {TextureType::ENEMY_TYRANNOSAURUS_REX_CHILD_SEEK,64,64,4,90,true}},
            {enemyActionKey(EnemyType::TYRANNOSAURUS_REX_CHILD,EnemyAction::RETURN), {TextureType::ENEMY_TYRANNOSAURUS_REX_CHILD_PATROL,64,64,4,120,true}},

            {enemyActionKey(EnemyType::ALBERTOSAURUS,EnemyAction::PATROL), {TextureType::ENEMY_ALBERTOSAURUS_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::ALBERTOSAURUS,EnemyAction::SEEK), {TextureType::ENEMY_ALBERTOSAURUS_SEEK,64,64,4,90,true}},
            {enemyActionKey(EnemyType::ALBERTOSAURUS,EnemyAction::RETURN), {TextureType::ENEMY_ALBERTOSAURUS_PATROL,64,64,4,120,true}},

            {enemyActionKey(EnemyType::DASPELTOSAURUS,EnemyAction::PATROL), {TextureType::ENEMY_DASPELTOSAURUS_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::DASPELTOSAURUS,EnemyAction::SEEK), {TextureType::ENEMY_DASPELTOSAURUS_SEEK,64,64,4,90,true}},
            {enemyActionKey(EnemyType::DASPELTOSAURUS,EnemyAction::RETURN), {TextureType::ENEMY_DASPELTOSAURUS_PATROL,64,64,4,120,true}},

    };
    auto it = table.find(enemyActionKey(type, Action));
    return it != table.end() ? &it->second : nullptr;
}

const EnemyAIConfig* getEnemyAIConfig(EnemyType type) {
    static const std::unordered_map<EnemyType, EnemyAIConfig> table{
            {EnemyType::SPIDER_CHILD, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::SHOOT},{60.f,40.f}}},
            {EnemyType::MOSS_GLOW_WORM, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::DASH},{60.f,40.f}}},
            {EnemyType::MOSS_BAT, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::SLASH},{60.f,40.f}}},
            {EnemyType::SINNERS_CHILD, {300.f,400.f,500.f,60.f,1500u,{AttackType::SLASH,AttackType::BEAM},{60.f,40.f}}},
            {EnemyType::CAVE_PREDATOR, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::SLASH},{60.f,40.f}}},
            {EnemyType::CAVE_RAT, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::DASH},{60.f,40.f}}},
            {EnemyType::SHAFTS_RAPTOR, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::SLASH},{60.f,40.f}}},
            {EnemyType::GIANT_BATS, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::SLASH},{60.f,40.f}}},
            {EnemyType::SHAFTS_WORKER, {300.f,400.f,500.f,60.f,1500u,{AttackType::SLASH,AttackType::PROJECTILE},{60.f,40.f}}},
            {EnemyType::ANKYLOSAURUS_CHILD, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::SLASH},{60.f,40.f}}},
            {EnemyType::HUNTER_RAPTOR, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::SLASH},{60.f,40.f}}},
            {EnemyType::HUNTER_EAGLE, {300.f,400.f,500.f,60.f,1500u,{AttackType::SLASH,AttackType::DASH},{60.f,40.f}}},
            {EnemyType::PREDATOR_THEROPOD, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::SLASH},{60.f,40.f}}},
            {EnemyType::THEROPOD_CHILD, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::SLASH},{60.f,40.f}}},
            {EnemyType::PREDATOR_CAT, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::SLASH},{60.f,40.f}}},
            {EnemyType::MOUNTAIN_CAT, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::DASH},{60.f,40.f}}},
            {EnemyType::CROCODYLOMORPH, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::SLASH},{60.f,40.f}}},
            {EnemyType::TARBOSAURUS_CHILD, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::DASH},{60.f,40.f}}},
            {EnemyType::TYRANNOSAURUS_REX_CHILD, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::BEAM},{60.f,40.f}}},
            {EnemyType::ALBERTOSAURUS, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::SLASH},{60.f,40.f}}},
            {EnemyType::DASPELTOSAURUS, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::SLASH},{60.f,40.f}}},
    };
    auto it = table.find(type);
    return it != table.end() ? &it->second : nullptr;
}
