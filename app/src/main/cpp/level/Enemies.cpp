//
// Created by LENOVO on 08-09-2026.
//
#include "Enemies.h"
#include "States/GameState/GameState.h"
#include <algorithm>


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
                           64.00f,600.00f,250.00f,PathAxis::HORIZONTAL,PathShape::LINE);

}

void EnemiesBuilder::render(SDL_Renderer *renderer) {
    int camX = (int)std::round(Camera::getInstance().getCamera().x);
    int camY = (int)std::round(Camera::getInstance().getCamera().y);
    for(const auto& enemy: m_enemies){
        auto info = getEnemyFrameInfo(enemy.type,enemy.action,enemy.attackType);
        auto renderedRect = getEnemyRenderedRect(enemy);
        if(info == nullptr) {
            LOGI("enemy info null");
            continue;
        }
        SDL_FRect src{0.00f+info->frameW *enemy.aniStartFrame,0.00f,static_cast<float>(info->frameW),static_cast<float>(info->frameH)};
        SDL_FRect dst{renderedRect.x-camX,renderedRect.y-camY,static_cast<float>(info->frameW)*5,static_cast<float>(info->frameH)*5};
//        if(enemy.isHurt)
//            SDL_SetRenderDrawColor(renderer,255,255,255,255); // flash white for ENEMY_HURT_FLASH_MS after being hit
//        else
//            SDL_SetRenderDrawColor(renderer,255,0,0,255);
//        SDL_RenderFillRect(renderer,&dst);
        auto texture = Engine::Get().getAssetManager().getTexture(info->texture);
        if(!texture){
            LOGI("null texture");
            continue;
        }
        if(enemy.isFacingRight | enemy.isMovingForward)
            SDL_RenderTexture(renderer,texture,&src,&dst);
        else
            SDL_RenderTextureRotated(renderer,texture,&src,&dst,0.0f, nullptr,SDL_FLIP_HORIZONTAL);
    }

}

SDL_FRect EnemiesBuilder::getEnemyRenderedRect(Enemy enemy){
    auto info = getEnemyFrameInfo(enemy.type,enemy.action,enemy.attackType);
    switch (enemy.type) {
        case EnemyType::SINNERS_CHILD:
            if(enemy.action == EnemyAction::ATTACK){
                return {0};
            }
            else if(enemy.action == EnemyAction::SEEK){
                return{enemy.x - 40,enemy.y-20,static_cast<float>(info->frameW*5),static_cast<float>(info->frameH*5) };
            }
            else if(enemy.action == EnemyAction::PATROL){
                return{enemy.x - 40,enemy.y-20,static_cast<float>(info->frameW*5),static_cast<float>(info->frameH*5) };
            }
            else if(enemy.action == EnemyAction::RETURN){
                return{enemy.x - 40,enemy.y-20,static_cast<float>(info->frameW*5),static_cast<float>(info->frameH*5) };
            }
            else{
                return{enemy.x - 40,enemy.y-20,static_cast<float>(info->frameW*5),static_cast<float>(info->frameH*5) };
            }
    }
}

void EnemiesBuilder::update(float dt,float playerX,float playerY,
                            const SDL_FRect& wallCollisionRect,
                            const std::vector<LevelGround>& grounds,
                            const std::vector<Platform>& platforms,
                            const std::vector<Block>& blocks) {


    unsigned int now = SDL_GetTicks();
    for(auto& enemy:m_enemies){
        if(enemy.previousX < enemy.x){
            enemy.isFacingRight =false;
        }
        if(enemy.isDead) continue;

        if(enemy.isHurt && now >= enemy.hurtFlashEndTime) enemy.isHurt = false;


        bool inKnockback = now < enemy.knockbackEndTime;
        if(inKnockback){
            // knockback overrides normal movement for its short duration
            enemy.x += enemy.knockbackVelocityX * dt;
        } else {
            enemy.knockbackVelocityX = 0.0f;
            updateAI(enemy,playerX,playerY,dt,wallCollisionRect,blocks);
        }
        applyGravityAndCollision(enemy,dt,wallCollisionRect,grounds,platforms,blocks);

        if(enemy.aniDone)continue;
        const auto* info = getEnemyFrameInfo(enemy.type,enemy.action,enemy.attackType);
        if(!info) {
            continue;
        }
        if(!info || info->frameCount<=1) continue;

        unsigned int now = SDL_GetTicks();
        if(now - enemy.lastTime > info->frameDelay)
        {
            enemy.lastTime =now;
            if(enemy.aniStartFrame < info->frameCount-1){
                enemy.aniStartFrame += 1;
            }
            else if(info->loop) enemy.aniStartFrame=0;
            else {
                enemy.aniDone=true;
            }
        }
    }
}

int EnemiesBuilder::applyPlayerDamage(const SDL_FRect& meleeHitBox, bool meleeActive,
                                      const SDL_FRect& beamHitBox, bool beamActive, float dt) {
    unsigned int now = SDL_GetTicks();
    int kills = 0;

    for(auto& enemy : m_enemies){
        if(enemy.isDead) continue;

        bool hitThisFrame = false;
        float knockDirX = 1.0f; // which way to pop the enemy, decided per-hit below

        // melee: one hit per swing, gated by its own i-frame window so a single
        // slash that overlaps for several frames doesn't chew through hp instantly
        // (lastHitByPlayerTime==0 means "never hit yet" - don't let it gate the very first hit)
        if(meleeActive && (enemy.lastHitByPlayerTime == 0 || (now - enemy.lastHitByPlayerTime) > ENEMY_HIT_IFRAME_MS) &&
           gameMath::checkcollision(meleeHitBox.x, meleeHitBox.y, enemy.x, enemy.y,
                                    meleeHitBox.h, meleeHitBox.w, enemy.h, enemy.w)){
            enemy.hp -= ENEMY_MELEE_DAMAGE;
            enemy.lastHitByPlayerTime = now;
            hitThisFrame = true;
            float hitterCenterX = meleeHitBox.x + meleeHitBox.w * 0.5f;
            float enemyCenterX  = enemy.x + enemy.w * 0.5f;
            knockDirX = (enemyCenterX >= hitterCenterX) ? 1.0f : -1.0f;
        }
        // beam: continuous damage-per-second while standing in it, no i-frames needed
        else if(beamActive &&
                gameMath::checkcollision(beamHitBox.x, beamHitBox.y, enemy.x, enemy.y,
                                         beamHitBox.h, beamHitBox.w, enemy.h, enemy.w)){
            enemy.hp -= (int)(ENEMY_BEAM_DAMAGE_PER_SEC * dt);
            hitThisFrame = true;
            float hitterCenterX = beamHitBox.x + beamHitBox.w * 0.5f;
            float enemyCenterX  = enemy.x + enemy.w * 0.5f;
            knockDirX = (enemyCenterX >= hitterCenterX) ? 1.0f : -1.0f;
        }

        if(hitThisFrame){
            enemy.isHurt = true;
            enemy.hurtFlashEndTime = now + ENEMY_HURT_FLASH_MS;
            enemy.knockbackVelocityX = knockDirX * ENEMY_KNOCKBACK_SPEED_X;
            enemy.knockbackEndTime = now + ENEMY_KNOCKBACK_MS;
            if(enemy.hp <= 0){
                enemy.isDead = true;
                kills++;
            }
        }
    }

    if(kills > 0){
        m_enemies.erase(std::remove_if(m_enemies.begin(), m_enemies.end(),
                                       [](const Enemy& e){ return e.isDead; }),
                        m_enemies.end());
    }
    return kills;
}

bool EnemiesBuilder::checkAttackOnPlayer(float playerX, float playerY, float playerW, float playerH,
                                         unsigned int now, gameMath::collisionSide& outSide) {
    for(auto& enemy : m_enemies){
        if(enemy.isDead || !enemy.isAttackBoxActive) continue;

        const auto cfg = getEnemyAIConfig(enemy.type);
        unsigned int cooldown = cfg ? cfg->attackCooldownMs : 500u;
        // lastAttackHitTime==0 means "hasn't landed a hit yet" - don't gate the first one
        if(enemy.lastAttackHitTime != 0 && now - enemy.lastAttackHitTime < cooldown) continue;

        if(gameMath::checkcollision(enemy.attackHitBox.x, enemy.attackHitBox.y, playerX, playerY,
                                    enemy.attackHitBox.h, enemy.attackHitBox.w, playerH, playerW)){
            enemy.lastAttackHitTime = now;
            outSide = enemy.isFacingRight ? gameMath::collisionSide::RIGHT : gameMath::collisionSide::LEFT;
            return true;
        }
    }
    return false;
}

void EnemiesBuilder::applyGravityAndCollision(Enemy& enemy, float dt,
                                              const SDL_FRect& wallCollisionRect,
                                              const std::vector<LevelGround>& grounds,
                                              const std::vector<Platform>& platforms,
                                              const std::vector<Block>& blocks) {
    if(enemy.combat != combatType::GROUND) return; // aerial enemies fly freely, no gravity/ground collision

    const float previousY = enemy.y; // captured before gravity moves this frame, for the one-way sweep test below

    enemy.isGrounded = false;
    enemy.velocityY += ENEMY_GRAVITY * dt;
    enemy.y += enemy.velocityY * dt;

    const float renderedHeight = (std::ceil(wallCollisionRect.h / (SCALE*TILE_SIZE)) * (SCALE*TILE_SIZE));
    gameMath::collisionSide wallSide;

    wallSide = gameMath::checkcollisionXY(enemy.x, enemy.y, wallCollisionRect.x, wallCollisionRect.y,
                                          enemy.h, enemy.w, TILE_SIZE * SCALE, wallCollisionRect.w);
    if(wallSide == gameMath::collisionSide::BOTTOM)
        enemy.velocityY = 0.0f;

    wallSide = gameMath::checkcollisionXY(enemy.x, enemy.y, wallCollisionRect.x, wallCollisionRect.y,
                                          enemy.h, enemy.w, renderedHeight, TILE_SIZE * SCALE);
    if(wallSide == gameMath::collisionSide::BOTTOM)
        enemy.velocityY = 0.0f;

    wallSide = gameMath::checkcollisionXY(enemy.x, enemy.y,
                                          wallCollisionRect.x + wallCollisionRect.w - TILE_SIZE * SCALE,
                                          wallCollisionRect.y, enemy.h, enemy.w, renderedHeight, TILE_SIZE * SCALE);
    if(wallSide == gameMath::collisionSide::BOTTOM)
        enemy.velocityY = 0.0f;

    wallSide = gameMath::checkcollisionXY(enemy.x, enemy.y, wallCollisionRect.x,
                                          wallCollisionRect.y + renderedHeight - TILE_SIZE * SCALE,
                                          enemy.h, enemy.w, TILE_SIZE * SCALE, wallCollisionRect.w);
    if(wallSide == gameMath::collisionSide::BOTTOM)
        enemy.velocityY = 0.0f;
    if(wallSide == gameMath::collisionSide::TOP){
        enemy.isGrounded = true;
        enemy.velocityY = 0.0f;
    }

    //platforms
    for(const auto& platform : platforms){
        if(platform.colliderType == ColliderType::SOLID){
            gameMath::collisionSide side = gameMath::checkcollisionXY(enemy.x, enemy.y,
                                                                      platform.x, platform.y,
                                                                      enemy.h, enemy.w,
                                                                      platform.h * SCALE, platform.w * SCALE);
            if(side == gameMath::collisionSide::TOP){
                enemy.velocityY = 0.0f;
                enemy.isGrounded = true;
            }
            if(side == gameMath::collisionSide::BOTTOM){
                enemy.velocityY = 0.0f;
            }
        } else if(platform.colliderType == ColliderType::ONE_WAY){
            float previousBottom = previousY + enemy.h;
            float currentBottom = enemy.y + enemy.h;
            float platformTop = platform.y;

            if(enemy.velocityY > 0 && previousBottom <= platformTop
               && currentBottom >= platformTop
               && gameMath::checkcollisionX(enemy.x, enemy.y, platform.x, platform.y,
                                            enemy.h, enemy.w, platform.h * SCALE, platform.w * SCALE)){
                enemy.y = platformTop - enemy.h;
                enemy.velocityY = 0.0f;
                enemy.isGrounded = true;
            }
        }
    }

    //ground
    for(const auto& ground : grounds){
        gameMath::collisionSide side = gameMath::checkcollisionXY(enemy.x, enemy.y, ground.x, ground.y,
                                                                  enemy.h, enemy.w, ground.h*SCALE, ground.w*SCALE);
        if(side == gameMath::collisionSide::TOP){
            enemy.isGrounded = true;
            enemy.velocityY = 0.0f;
        }
        if(side == gameMath::collisionSide::BOTTOM){
            enemy.velocityY = 0.0f;
        }
    }

    //blocks
    for(const auto& block : blocks){
        gameMath::collisionSide side = gameMath::checkcollisionXY(enemy.x, enemy.y, block.rect.x, block.rect.y,
                                                                  enemy.h, enemy.w, block.rect.h, block.rect.w);
        if(side == gameMath::collisionSide::TOP){
            enemy.isGrounded = true;
            enemy.velocityY = 0.0f;
        }
        if(side == gameMath::collisionSide::BOTTOM){
            enemy.velocityY = 0.0f;
        }
    }
}
bool EnemiesBuilder::isHorizontalPathClear(const Enemy& e, float nextX,
                                           const SDL_FRect& wallCollisionRect,
                                           const std::vector<Block>& blocks) const {
    if(e.combat != combatType::GROUND) return true; // aerial enemies fly over/around obstacles

    // the solid wall is a one-tile-thick band right at the rect's edges (see
    // applyGravityAndCollision above / GameState::computeBeamLength), so the usable
    // interior is inset by one tile on each side - not the raw rect bounds.
    const float leftWall  = wallCollisionRect.x + TILE_SIZE * SCALE;
    const float rightWall = wallCollisionRect.x + wallCollisionRect.w - TILE_SIZE * SCALE;
    if(nextX < leftWall || nextX + e.w > rightWall) return false;

    for(const auto& block : blocks){
        if(gameMath::checkcollision(nextX, e.y, block.rect.x, block.rect.y,
                                    e.h, e.w, block.rect.h, block.rect.w))
            return false;
    }
    return true;
}

bool EnemiesBuilder::updateStuckTimer(Enemy& e, unsigned int now) {
    float dx = e.x - e.stuckAnchorX, dy = e.y - e.stuckAnchorY;
    float moved = std::sqrt(dx*dx + dy*dy);

    if(!e.stuckTimerActive || moved > ENEMY_STUCK_MOVE_EPS){
        // either just started tracking, or it made real progress - reset the clock
        e.stuckTimerActive = true;
        e.stuckAnchorX = e.x;
        e.stuckAnchorY = e.y;
        e.stuckTimerStart = now;
        return false;
    }
    return (now - e.stuckTimerStart) > ENEMY_STUCK_TIME_MS;
}

void EnemiesBuilder::updateAI(Enemy& enemy, float playerX, float playerY, float dt,
                              const SDL_FRect& wallCollisionRect, const std::vector<Block>& blocks) {
    float dx = playerX - enemy.x, dy = playerY - enemy.y;
    float distToPlayer = std::sqrt(dx*dx + dy*dy);
    const auto cfg = getEnemyAIConfig(enemy.type);
    unsigned int now = SDL_GetTicks();

    if(enemy.action != EnemyAction::ATTACK) enemy.isAttackBoxActive = false;
    if(enemy.action != EnemyAction::PATROL) enemy.stuckTimerActive = false;

    switch (enemy.action) {
        case EnemyAction::PATROL:
            updatePath(enemy, dt);                       // existing trap-style movement

            // stuck-in-the-middle-of-the-path check: only while actually travelling
            // between waypoints (not while it's paused/switching direction at an end)
            if(!enemy.hasHitEnd){
                if(updateStuckTimer(enemy, now)){
                    enemy.stuckTimerActive = false;
                    enemy.action = EnemyAction::RETURN;   // give up on this leg, head back to where it started
                    break;
                }
            } else {
                enemy.stuckTimerActive = false;
            }

            if (distToPlayer <= cfg->detectRadius)
                enemy.action = EnemyAction::SEEK;
            break;

        case EnemyAction::SEEK: {
            moveToward(enemy, playerX, playerY, enemy.movingSpeed, dt);
            enemy.isFacingRight = (dx > 0);

            float homeDx = enemy.x - enemy.baseX, homeDy = enemy.y - enemy.baseY;
            if(enemy.combat == combatType::GROUND)
                homeDy = enemy.y;
            float distFromHome = std::sqrt(homeDx*homeDx + homeDy*homeDy);

            if (distToPlayer <= cfg->attackRange)
                enemy.action = EnemyAction::ATTACK;
            else if (distToPlayer > cfg->loseRadius)
                enemy.action = EnemyAction::RETURN;
            break;
        }

        case EnemyAction::ATTACK: {
            // attack hitbox: reaches attackRange out from the enemy, in the direction it's facing
            float boxW = cfg->attackRange;
            enemy.attackHitBox.w = boxW;
            enemy.attackHitBox.h = enemy.h;
            enemy.attackHitBox.y = enemy.y;
            enemy.attackHitBox.x = enemy.isFacingRight ? (enemy.x + enemy.w) : (enemy.x - boxW);
            enemy.isAttackBoxActive = true;

            if (distToPlayer > cfg->attackRange * 1.2f){      // player escaped mid-attack
                enemy.action = EnemyAction::SEEK;
                enemy.isAttackBoxActive = false;
            }
            else if (enemy.aniDone){                              // let animation drive the timing
                enemy.action = EnemyAction::SEEK;                 // reposition, re-enter ATTACK if still in range
                enemy.isAttackBoxActive = false;
            }
            break;
        }

        case EnemyAction::RETURN: {
            float targetX = enemy.baseX;
            float dir = (targetX > enemy.x) ? 1.0f : (targetX < enemy.x ? -1.0f : 0.0f);
            float nextX = enemy.x + dir * enemy.movingSpeed * dt;

            // don't let the trip home walk it straight into a wall/block - if the
            // very next step is obstructed, just hold position this frame instead
            bool clear = isHorizontalPathClear(enemy, nextX, wallCollisionRect, blocks);

            if(clear){
                if(enemy.combat != combatType::GROUND){
                    moveToward(enemy, enemy.baseX, enemy.baseY, enemy.movingSpeed, dt);
                }
                else{
                    moveToward(enemy, enemy.baseX, enemy.y, enemy.movingSpeed,dt);
                }
            }

            float rdx = enemy.baseX - enemy.x, rdy = enemy.baseY - enemy.y;
            if(enemy.combat ==combatType::GROUND){
                rdy = 0;
            }
            if (std::sqrt(rdx*rdx + rdy*rdy) < 4.0f) {
                enemy.x = enemy.baseX;
                if(enemy.combat != combatType::GROUND){
                    enemy.y = enemy.baseY;
                }
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
    if(enemy.combat != combatType::GROUND){
        enemy.y += (dy / dist) * speed * dt;
    }
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
        if(enemy.combat != combatType::GROUND){
            if (enemy.y != target.y) {
                float dir = (target.y > enemy.y) ? 1.0f : -1.0f;
                if (SDL_fabsf(target.y - enemy.y) <= stepDist) {
                    enemy.y = target.y;
                } else {
                    enemy.y += stepDist * dir;
                }
            }
        }
        if(enemy.combat == combatType::GROUND){
            if(target.x == enemy.x){
                unsigned int now = SDL_GetTicks();
                if(!enemy.hasHitEnd){
                    enemy.hasHitEnd =true;
                    enemy.lastSwitchTime=now;

                    enemy.aniStartFrame = 0;
                    enemy.aniDone = false;
                }
                else
                    enemy.pathIndex = (enemy.pathIndex + 1) % 4;

            }
        }
        else if(target.x == enemy.x && target.y == enemy.y){
            unsigned int now = SDL_GetTicks();
            if(!enemy.hasHitEnd){
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
}



Enemy::Enemy(float x, float y, EnemyType type, EnemyAction action, AttackType attackType,
             float startPath, float endPath, float speed, PathAxis axis, PathShape shape,
             float radius, combatType combat)
        :x(x),y(y),type(type),action(action),attackType(attackType),startPath(startPath),endPath(endPath),
         movingSpeed(speed),axis(axis),pathShape(shape),radius(radius),combat(combat),baseX(x),baseY(y),previousX(x),previousY(y){

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

            {enemyActionKey(EnemyType::SINNERS_CHILD,EnemyAction::PATROL), {TextureType::ENEMY_SINNERS_CHILD_PATROL,24,24,10,50,true}},
            {enemyActionKey(EnemyType::SINNERS_CHILD,EnemyAction::SEEK), {TextureType::ENEMY_SINNERS_CHILD_SEEK,24,24,7,50,true}},
            {enemyActionKey(EnemyType::SINNERS_CHILD,EnemyAction::RETURN), {TextureType::ENEMY_SINNERS_CHILD_PATROL,24,24,10,50,true}},

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
