//
// Created by LENOVO on 24-07-2026.
//
#include "Traps.h"
#include "States/GameState/GameState.h"
namespace {
    inline uint32_t trapKey(TrapType type,TrapStatus status)
    {
        return (static_cast<uint32_t>(type) <<8 |static_cast<uint32_t>(status));
    }
}
std::vector<Trap> &TrapBuilder::getTraps()
{
    return m_traps;
}

void TrapBuilder::init(const std::vector<Trap> &traps)

{
    m_traps =traps;
    unsigned int now = SDL_GetTicks();
    for(auto& trap : m_traps){
        trap.lastSwitchTime = now;
        trap.lastTime = now;
        if(const auto* info = getTrapFrameInfo(trap.type, trap.status)){
            trap.aniEndFrame = info->frameCount - 1;
        }
    }
    for(auto& trap:m_traps){
        if(const auto* info = getTrapFrameInfo(trap.type,trap.status)){
            trap.aniEndFrame = info->frameCount-1;
        }
    }
}

void TrapBuilder::render(SDL_Renderer *renderer)
{
    int camX = (int)std::round(Camera::getInstance().getCamera().x);
    int camY = (int)std::round(Camera::getInstance().getCamera().y);
    for(const auto& trap:m_traps)
    {
        if(trap.showChain && trapHasPath(trap.type))
            renderChain(renderer, trap, camX, camY);
    }
    for(const auto& trap:m_traps)
    {

        auto* info = getTrapFrameInfo(trap.type,trap.status);

        if(!info)//this only fails if key combination is wrong, it is fatal crash deferencing nullptr,
        {
            LOGI("failed to load info type :%d ,status:%d", trap.type, trap.status);
            continue;
        }

        SDL_Texture* texture = Engine::Get().getAssetManager().getTexture(info->texture);

        SDL_FRect Dst{trap.x - camX, trap.y - camY, info->frameW* SCALE, info->frameH * SCALE};
        SDL_FRect Src{0.0f+info->frameW*trap.aniStartFrame,0.0f,
                      static_cast<float>(info->frameW), static_cast<float>(info->frameH)};

        SDL_RenderTexture(renderer,texture,&Src,&Dst);
    }
}

void TrapBuilder::resetPlatforms(int trapIndex){
    for(auto &trap:m_traps)
    {
        if (trap.type == TrapType::MOVING_PLATFORM_BROWN) {
            trap.isActivated = false;
        }
    }
}

void TrapBuilder::update(float dt)
{

    for(auto &trap:m_traps)
    {


        //fire
        if(trap.type == TrapType::FIRE)
        {
            if (trap.status == TrapStatus::ON) {
                unsigned int nowSwitchTime = SDL_GetTicks();
                if (nowSwitchTime - trap.lastSwitchTime > m_fireTimer) {
                    trap.lastSwitchTime = nowSwitchTime;
                    trap.status = TrapStatus::OFF;
                    trap.aniStartFrame = 0;
                    trap.aniDone = false;
                }
            }
        }
        //fan
        if(trap.type == TrapType::FAN)
        {
            if (trap.status == TrapStatus::ON) {
                unsigned int nowSwitchTime = SDL_GetTicks();
                if (nowSwitchTime - trap.lastSwitchTime > m_fanTimer) {
                    trap.lastSwitchTime = nowSwitchTime;
                    trap.status = TrapStatus::OFF;
                    trap.aniStartFrame = 0;
                    trap.aniDone = false;
                }
            }
            if (trap.status == TrapStatus::OFF) {
                unsigned int nowSwitchTime = SDL_GetTicks();
                if (nowSwitchTime - trap.lastSwitchTime > m_fanTimer) {
                    trap.lastSwitchTime = nowSwitchTime;
                    trap.status = TrapStatus::ON;
                    trap.aniStartFrame = 0;
                    trap.aniDone = false;
                }
            }
        }
        //falling platform

        if(trap.aniDone && trap.type ==TrapType::FIRE)
        {
            if(trap.status == TrapStatus::HIT)
            {
                trap.lastSwitchTime =SDL_GetTicks();
                trap.status = TrapStatus::ON;
                trap.aniStartFrame = 0;
                trap.aniDone = false;
            }

        }
        if(trap.aniDone && (trap.type == TrapType::ROCK_HEAD||trap.type == TrapType::SPIKE_HEAD) && !trap.hasHitEnd)
        {
            if(trap.status == TrapStatus::HIT)
            {
                trap.status = TrapStatus::IDLE;
                trap.aniDone=false;
                trap.aniStartFrame=0;
            }
        }
        if(trap.type ==TrapType::MOVING_PLATFORM_BROWN)
        {
            TrapStatus newStatus = trap.isActivated ? TrapStatus::ON : TrapStatus::OFF;
            if(newStatus != trap.status)
            {
                trap.status = newStatus;
                trap.aniStartFrame = 0;
                trap.aniDone = false;
            }
        }
        if(trap.aniDone)continue;
        const auto* info = getTrapFrameInfo(trap.type,trap.status);
        if(!info) {
            LOGI("failed to load info type :%d ,status:%d", trap.type, trap.status);
            continue;
        }
        if(!info || info->frameCount<=1) continue;

        unsigned int now = SDL_GetTicks();
        if(now - trap.lastTime > info->frameDelay)
        {
            trap.lastTime =now;
            if(trap.aniStartFrame < info->frameCount-1){
                trap.aniStartFrame += 1;
            }
            else if(info->loop) trap.aniStartFrame=0;
            else {
                trap.aniDone=true;
            }
        }

    }
}

void TrapBuilder::renderChain(SDL_Renderer* renderer, const Trap& trap, int camX, int camY) {
    TextureType chainTex = getChainTexture(trap.type);
    if(chainTex == TextureType::COUNT) return;
    SDL_Texture* tex = Engine::Get().getAssetManager().getTexture(chainTex);
    auto* info = getTrapFrameInfo(trap.type,trap.status);
    if(!info) {
        LOGI("failed to load info type :%d ,status:%d", trap.type, trap.status);
        return;
    }
        const float LINK_SPACING = 40.0f; // px between links, tune to your sprite size

    if(trap.pathShape == PathShape::LINE) {
        SDL_FPoint a = (trap.axis == PathAxis::HORIZONTAL) ? SDL_FPoint{trap.startPath+(info->frameW*SCALE/2)-((8*SCALE)/2),
                                                                        trap.baseY+(info->frameH*SCALE/2)-((8*SCALE)/2)}
                                                           : SDL_FPoint{trap.baseX+(info->frameW*SCALE/2)-((8*SCALE)/2),
                                                                        trap.startPath+(info->frameH*SCALE/2)-((8*SCALE)/2)};

        SDL_FPoint b = (trap.axis == PathAxis::HORIZONTAL) ? SDL_FPoint{trap.endPath+(info->frameW*SCALE/2)-((8*SCALE)/2),
                                                                        trap.baseY+(info->frameH*SCALE/2)-((8*SCALE)/2)}
                                                           : SDL_FPoint{trap.baseX+(info->frameW*SCALE/2)-((8*SCALE)/2),
                                                                        trap.endPath+(info->frameH*SCALE/2)-((8*SCALE)/2)};
        float len = SDL_sqrtf((b.x-a.x)*(b.x-a.x) + (b.y-a.y)*(b.y-a.y));
        int count = (int)(len / LINK_SPACING);
        for(int i = 0; i <= count; i++) {
            float t = (float)i / count;
            SDL_FRect dst{a.x + (b.x-a.x)*t - camX, a.y + (b.y-a.y)*t - camY, 8*SCALE, 8*SCALE};
            SDL_RenderTexture(renderer, tex, nullptr, &dst);
        }
    }
    else if(trap.pathShape == PathShape::RECT) {
        if(trap.type==TrapType::ROCK_HEAD||trap.type ==TrapType::SPIKE_HEAD)
            return;
        SDL_FPoint corners[4] = {
                {trap.baseX+(info->frameW*SCALE/2)-((8*SCALE)/2), trap.baseY+(info->frameH*SCALE/2)-((8*SCALE)/2)},
                {trap.startPath+(info->frameW*SCALE/2)-((8*SCALE)/2), trap.baseY+(info->frameH*SCALE/2)-((8*SCALE)/2)},
                {trap.startPath+(info->frameW*SCALE/2)-((8*SCALE)/2), trap.endPath+(info->frameH*SCALE/2)-((8*SCALE)/2)},
                {trap.baseX+(info->frameW*SCALE/2)-((8*SCALE)/2), trap.endPath+(info->frameH*SCALE/2)-((8*SCALE)/2)}
        };
        for(int c = 0; c < 4; c++) {
            SDL_FPoint a = corners[c], b = corners[(c+1)%4];
            float len = SDL_sqrtf((b.x-a.x)*(b.x-a.x) + (b.y-a.y)*(b.y-a.y));
            int count = (int)(len / LINK_SPACING);
            for(int i = 0; i <= count; i++) {
                float t = (float)i / count;
                SDL_FRect dst{a.x + (b.x-a.x)*t - camX, a.y + (b.y-a.y)*t - camY, 8*SCALE, 8*SCALE};
                SDL_RenderTexture(renderer, tex, nullptr, &dst);
            }
        }
    }
    else if(trap.pathShape == PathShape::CIRCLE) {
        SDL_FPoint a {trap.baseX+(info->frameW*SCALE/2)-((8*SCALE)/2),trap.baseY+(info->frameH*SCALE/2)-((8*SCALE)/2)};
        SDL_FPoint b {trap.x+(info->frameW*SCALE/2)-((8*SCALE)/2),trap.y+(info->frameH*SCALE/2)-((8*SCALE)/2)};
        float len = SDL_sqrtf((b.x-a.x)*(b.x-a.x) + (b.y-a.y)*(b.y-a.y));
        int count = (int)(len / LINK_SPACING);
        for(int i = 0; i <= count; i++) {
            float t = (float)i / count;
            SDL_FRect dst{a.x + (b.x-a.x)*t - camX, a.y + (b.y-a.y)*t - camY, 8*SCALE, 8*SCALE};
            SDL_RenderTexture(renderer, tex, nullptr, &dst);
        }
    }
    else if(trap.pathShape == PathShape::ARC) {
        SDL_FPoint a {trap.baseX+(info->frameW*SCALE/2)-((8*SCALE)/2),trap.baseY+(info->frameH*SCALE/2)-((8*SCALE)/2)};
        SDL_FPoint b {trap.x+(info->frameW*SCALE/2)-((8*SCALE)/2),trap.y+(info->frameH*SCALE/2)-((8*SCALE)/2)};
        float len = SDL_sqrtf((b.x-a.x)*(b.x-a.x) + (b.y-a.y)*(b.y-a.y));
        int count = (int)(len / LINK_SPACING);
        for(int i = 0; i <= count; i++) {
            float t = (float)i / count;
            SDL_FRect dst{a.x + (b.x-a.x)*t - camX, a.y + (b.y-a.y)*t - camY, 8*SCALE, 8*SCALE};
            SDL_RenderTexture(renderer, tex, nullptr, &dst);
        }
    }
}

bool TrapBuilder::isSolid(int trapIndex) {
    auto& trap = m_traps[trapIndex];

    return trap.type == TrapType::MOVING_PLATFORM_GREY||trap.type == TrapType::MOVING_PLATFORM_BROWN||(trap.type == TrapType::ROCK_HEAD && trap.status != TrapStatus::HIT)||
            trap.type==TrapType::FIRE ||trap.type == TrapType::FALLING_PLATFORM;
}

gameMath::collisionSide TrapBuilder::resolveTrapCollision(int trapIndex,float &playerX, float& playerY,
                                                          float playerW, float playerH,float previousY,float velocityY){
    auto& trap = m_traps[trapIndex];

    if(trap.colliderType == ColliderType::SOLID)
    {
        SDL_FRect trapCollider = getTrapCollisionBox(trap);
        gameMath::collisionSide collision = gameMath::checkcollisionXY(playerX, playerY,
                                                                       trapCollider.x,
                                                                       trapCollider.y,
                                                                       playerH, playerW,
                                                                       trapCollider.h,
                                                                       trapCollider.w);
        return collision;
    }
    if(trap.colliderType == ColliderType::ONE_WAY){

        SDL_FRect trapCollider = getTrapCollisionBox(trap);
        float previousBottom = previousY + playerH;
        float currentBottom = playerY + playerH;
        float platformTop = trapCollider.y;
        float platformTopPrev = platformTop - (trap.y - trap.previousY);
        float sweptTopMin = (platformTopPrev < platformTop) ? platformTopPrev : platformTop;
        float sweptTopMax = (platformTopPrev > platformTop) ? platformTopPrev : platformTop;

        if (velocityY > 0 && previousBottom <= sweptTopMax
            && currentBottom >= sweptTopMin
            && gameMath::checkcollisionX(playerX, playerY, trapCollider.x,
                                             trapCollider.y,
                                             playerH, playerW, trapCollider.h,
                                             trapCollider.w)) {

                playerY = platformTop - playerH;
                if(trap.type == TrapType::MOVING_PLATFORM_BROWN)
                    trap.isActivated =true;
                return gameMath::collisionSide::TOP;
        }
        else if(trap.type == TrapType::MOVING_PLATFORM_BROWN){
            trap.isActivated =false;
        }
    }
    return gameMath::collisionSide::NONE;
}
SDL_FRect TrapBuilder::getTrapCollisionBox(const Trap& trap) {
    auto* info = getTrapFrameInfo(trap.type,trap.status);
    if(!info) {
        LOGI("failed to load info type :%d ,status:%d", trap.type, trap.status);
        return{0.0f,0.0f,0.0f,0.0f};
    }
    SDL_FRect rect{trap.x,trap.y,info->frameW*SCALE,info->frameH*SCALE};
    switch(trap.type){
        case TrapType::MOVING_PLATFORM_BROWN:
        case TrapType::MOVING_PLATFORM_GREY:
            return { trap.x+3, trap.y+7 , ((info->frameW)*SCALE)-6,(info->frameH) *SCALE-10 };
        case TrapType::FAN:
            return { trap.x+3, trap.y , ((info->frameW)*SCALE)-3,(info->frameH) *SCALE };
        case TrapType::TRAMPOLINE:
            return { trap.x+10 , trap.y+70 , (info->frameW-6)*SCALE, (info->frameH-20)*SCALE };
        case TrapType::FALLING_PLATFORM:
            return { trap.x , trap.y, (info->frameW)*SCALE, info->frameH*SCALE };
        case TrapType::FIRE:
            return { trap.x , trap.y+64, (info->frameW)*SCALE, (info->frameH-16)*SCALE};
        case TrapType::ROCK_HEAD:
            // only dangerous while actively hitting, not during the idle blink loop
            return { trap.x+20, trap.y+20, (info->frameW-10)*SCALE, (info->frameH-10)*SCALE };
        default:
            return { trap.x, trap.y, info->frameW*SCALE, info->frameH*SCALE };
    }
    return rect;
}

void TrapBuilder::triggerFall(int trapIndex) {
    auto &trap = m_traps[trapIndex];
    if(trap.type != TrapType::FALLING_PLATFORM || trap.status == TrapStatus::OFF) return;
    trap.status = TrapStatus::OFF;
    trap.aniStartFrame = 0;
    trap.aniDone = true;
    if(const auto* info = getTrapFrameInfo(trap.type, trap.status))
        trap.aniEndFrame = info->frameCount-1;
}

SDL_FRect TrapBuilder::getHazardHitBox(const Trap &trap) {
    const auto* info = getTrapFrameInfo(trap.type,trap.status);
    if(!info) {
        LOGI("failed to load info type :%d ,status:%d", trap.type, trap.status);
        return{0.0f,0.0f,0.0f,0.0f};
    }
    switch(trap.type){
        case TrapType::SPIKES:
            return { trap.x, trap.y +36, (info->frameW-1)*SCALE, (info->frameH-9)*SCALE };
        case TrapType::SAW:
            return { trap.x+10 , trap.y +10, (info->frameW-6)*SCALE, (info->frameH-6)*SCALE };
        case TrapType::FIRE:
            return { trap.x+12, trap.y+5, (info->frameW-6)*SCALE, (info->frameH-20)*SCALE };
        case TrapType::SPIKE_BALL:
            return { trap.x+20 , trap.y+20, (info->frameW-9)*SCALE, (info->frameH-9)*SCALE };
        case TrapType::ROCK_HEAD:
            return { trap.x+20, trap.y+20, (info->frameW-10)*SCALE, (info->frameH-10)*SCALE };
        case TrapType::SPIKE_HEAD:
            return { trap.x+40, trap.y+40, (info->frameW-20)*SCALE, (info->frameH-20)*SCALE };
        default:
            return { trap.x, trap.y, info->frameW*SCALE, info->frameH*SCALE };
    }
}


bool TrapBuilder::checkHazard(float playerX, float playerY, float playerW, float playerH,
                              TrapType& outType,gameMath::collisionSide& outSide) {
    for(const auto& trap:m_traps){
        if(!trapHasHit(trap.type))continue;
        bool live;
        switch(trap.type){
            case TrapType::FIRE:   live = trap.status == TrapStatus::ON; break;
            case TrapType::SAW:    live = trap.status == TrapStatus::ON; break;
            case TrapType::ROCK_HEAD:live = trap.status == TrapStatus::HIT; break;
            default: live = true; break; // Spikes / Spike Ball are always hazardous
        }
        if(!live) continue;

        SDL_FRect box = getHazardHitBox(trap);
        gameMath::collisionSide side =gameMath::checkHazardCollision(playerX,playerY,playerW,playerH,box);
        if(side!=gameMath::collisionSide::NONE)
        {
            outType=trap.type;
            outSide=side;
            return true;
        }
    }
    return false;
}


float TrapBuilder::checkFanForce(float playerX, float playerY, float playerW, float playerH,ParticleSystem& particleSystem) {
    const float FAN_FORCE = -500.0f; // px/s upward, tune against m_gravity/m_jumpVelocity
    for(const auto& trap : m_traps){
        SDL_FRect trapCollRect =getTrapCollisionBox(trap);
        if(trap.type != TrapType::FAN)continue;
        SDL_FRect sensorRect{trapCollRect.x,trapCollRect.y -700,
                              trapCollRect.w,  trapCollRect.h +700+trapCollRect.h};
        if(trap.status == TrapStatus::ON){
            particleSystem.emitParticleWProps(1,(trapCollRect.x+trapCollRect.w/2)-40.00f,(trapCollRect.x+trapCollRect.w/2),
                                              trapCollRect.y-60,trapCollRect.y-30,-40,
                                              80,0,0,1.5f,1.5f,0,-500,40.00f);
        }

        if(trap.type != TrapType::FAN || trap.status != TrapStatus::ON) continue;
        if(gameMath::checkcollision(playerX, playerY, sensorRect.x,sensorRect.y, playerH, playerW, sensorRect.h, sensorRect.w)) {
            return FAN_FORCE;
        }
    }
    return 0.0f;
}
bool TrapBuilder::checkFireCollision(int trapIndex,float playerX, float playerY, float playerW, float playerH) {
    if(trapIndex < 0 || trapIndex >= (int)m_traps.size()) return false;
    Trap& trap = m_traps[trapIndex];
    if(trap.type != TrapType::FIRE) return false;
    SDL_FRect trapSize = getTrapCollisionBox(trap);
    float w=trapSize.w,h =trapSize.h;
    if(!gameMath::checkcollision(playerX, playerY, trapSize.x, trapSize.y,
                                 playerH, playerW, h, w)) return false;
    if(trap.status == TrapStatus::OFF)
    {
        trap.lastSwitchTime=SDL_GetTicks();
        trap.status = TrapStatus::HIT;
        trap.aniStartFrame = 0;
        trap.aniDone = false;
    }
    if(const auto* info = getTrapFrameInfo(trap.type, trap.status))
        trap.aniEndFrame = info->frameCount-1;
    return true;
}
bool TrapBuilder::checkTrampolineBounce(int trapIndex,float playerX, float playerY,
                                        float playerW, float playerH,ParticleSystem& particleSystem) {
    if(trapIndex < 0 || trapIndex >= (int)m_traps.size()) return false;
    Trap& trap = m_traps[trapIndex];
    if(trap.type != TrapType::TRAMPOLINE) return false;
    SDL_FRect trapSize = getTrapCollisionBox(trap);
    float w=trapSize.w,h =trapSize.h;
    if(!gameMath::checkcollision(playerX, playerY, trapSize.x, trapSize.y,
                                 playerH, playerW, h, w)) return false;
    particleSystem.emitJumpDust(playerX , playerY+playerH-40.00f);
    trap.status = TrapStatus::TRIGGERED;
    trap.aniStartFrame = 0;
    trap.aniDone = false;
    if(const auto* info = getTrapFrameInfo(trap.type, trap.status))
        trap.aniEndFrame = info->frameCount-1;
    return true;
}

void TrapBuilder::updatePath(float dt)
{
    for(auto& trap: m_traps)
    {
        trap.previousX=trap.x;
        trap.previousY=trap.y;

        if(trap.type ==TrapType::FALLING_PLATFORM && trap.status ==TrapStatus::OFF){
            trap.y += 300.00f *dt;
        }
        if(!trapHasPath(trap.type))continue;

        if(trap.pathShape == PathShape::RECT)
        {
            SDL_FPoint paths[4]={
                    {trap.baseX,trap.baseY},
                    {trap.startPath,trap.baseY},
                    {trap.startPath,trap.endPath},
                    {trap.baseX,trap.endPath}
            };
            SDL_FPoint target = paths[trap.pathIndex];

            float stepDist = trap.movingSpeed*dt;
            if(trap.x != target.x)
            {
                float dir = (target.x > trap.x) ? 1.0f : -1.0f;
                if(SDL_fabsf(target.x - trap.x) <= stepDist)
                {
                    trap.x = target.x;
                }
                else
                {
                    trap.x += stepDist*dir;
                }
            }
            if(trap.y != target.y)
            {
                float dir = (target.y > trap.y) ? 1.0f : -1.0f;
                if(SDL_fabsf(target.y - trap.y) <= stepDist)
                {
                    trap.y = target.y;
                }
                else
                {
                    trap.y += stepDist*dir;
                }
            }
            if(target.x == trap.x && target.y == trap.y)
            {
                unsigned int now = SDL_GetTicks();
                if(!trap.hasHitEnd)
                {
                    trap.hasHitEnd =true;
                    trap.lastSwitchTime=now;
                    if(trap.type ==TrapType::ROCK_HEAD||trap.type == TrapType::SPIKE_HEAD)
                        trap.status = TrapStatus::HIT;
                    trap.aniStartFrame = 0;
                    trap.aniDone = false;
                }
                if(trap.type == TrapType::ROCK_HEAD || trap.type == TrapType::SPIKE_HEAD)
                {
                    if (now - trap.lastSwitchTime > m_rockHeadTimer) {
                        trap.lastSwitchTime = now;
                        trap.pathIndex = (trap.pathIndex + 1) % 4;
                    }
                }
                else
                    trap.pathIndex = (trap.pathIndex + 1) % 4;

            }
            else{
                trap.hasHitEnd =false;
            }
            continue;
        }

        if(trap.pathShape == PathShape::LINE)
        {

            if (trap.type == TrapType::MOVING_PLATFORM_BROWN)
            {
                if (!trap.isActivated)
                    trap.isMovingForward = trap.isActivated;
            }

            float& coord = (trap.axis == PathAxis::HORIZONTAL)?trap.x:trap.y;
            float target = (trap.isMovingForward)?trap.endPath:trap.startPath;
            float dir = (coord < target) ? 1.00f : -1.00f;

            float stepDist = trap.movingSpeed*dt;

            if(SDL_fabsf(target - coord) <= stepDist)
            {
                unsigned int now = SDL_GetTicks();
                coord = target;
                if(!trap.hasHitEnd)
                {
                    trap.hasHitEnd =true;
                    trap.lastSwitchTime=now;
                    if(trap.type ==TrapType::ROCK_HEAD||trap.type==TrapType::SPIKE_HEAD)
                        trap.status = TrapStatus::HIT;
                    trap.aniStartFrame = 0;
                    trap.aniDone = false;
                }
                if(trap.type ==TrapType::ROCK_HEAD||trap.type == TrapType::SPIKE_HEAD)
                {
                    if (now - trap.lastSwitchTime > m_rockHeadTimer) {
                        trap.lastSwitchTime = now;
                        trap.isMovingForward = !trap.isMovingForward;
                    }
                }
                else if(trap.type == TrapType::MOVING_PLATFORM_GREY||trap.type == TrapType::SAW)
                {
                    // always moving, immediate ping-pong, no timer
                    trap.isMovingForward = !trap.isMovingForward;
                }

                else if(trap.type == TrapType::MOVING_PLATFORM_BROWN)
                {
                    if(!trap.isActivated)
                        trap.isMovingForward = true;   // parked at start
                    else
                        trap.isMovingForward = !trap.isMovingForward;
                }
            }
            else
            {
                trap.hasHitEnd =false;

                coord += stepDist*dir;

            }
        }
        if(trap.pathShape == PathShape::CIRCLE)
        {// no target we just move endlessly
            trap.pathAngle += trap.movingSpeed/trap.radius *dt;
            trap.x =trap.baseX +trap.radius *cosf(trap.pathAngle);
            trap.y =trap.baseY +trap.radius * sinf(trap.pathAngle);
        }
        if(trap.pathShape == PathShape::ARC)
        {
            // endPath/startPath as angle bounds (radians)
            float target = trap.isMovingForward ? trap.endPath : trap.startPath;

            float dir = (trap.pathAngle < target) ? 1.0f : -1.0f;
            float angularSpeed = trap.movingSpeed / trap.radius;
            float step = angularSpeed * dt;

            if(SDL_fabsf(target - trap.pathAngle) <= step) {
                trap.pathAngle = target;
                trap.isMovingForward = !trap.isMovingForward;
            } else {
                trap.pathAngle += step * dir;
            }
            trap.x = trap.baseX + trap.radius * cosf(trap.pathAngle);
            trap.y = trap.baseY + trap.radius * sinf(trap.pathAngle);
        }
    }
}

SDL_FPoint TrapBuilder::getTrapDelta(int trapIndex) {
    if(trapIndex < 0 || trapIndex >= (int)m_traps.size()) return {0.0f,0.0f};
    const Trap& trap = m_traps[trapIndex];
    return { trap.x - trap.previousX, trap.y - trap.previousY };
}


Trap::Trap(float x, float y, TrapType type, TrapStatus status, float startPath,float endPath,
           float speed,PathAxis axis,PathShape shape,ColliderType colliderType,float radius,
           bool showChain)
        :x(x),y(y),type(type),status(status),startPath(startPath),endPath(endPath),
        movingSpeed(speed),axis(axis),pathShape(shape),baseX(x),baseY(y),colliderType(colliderType),
        radius(radius),showChain(showChain)
{

}

const TextureType getChainTexture(TrapType type) {
    switch(type) {
        case TrapType::SAW: return TextureType::TRAP_SAW_CHAIN;
        case TrapType::MOVING_PLATFORM_BROWN:
        case TrapType::MOVING_PLATFORM_GREY: return TextureType::TRAP_PLATFORM_CHAIN;
        case TrapType::SPIKE_BALL: return TextureType::TRAP_SPIKE_BALL_CHAIN;
        default: return TextureType::COUNT; // no chain for anything else
    }
}

const TrapFrameInfo* getTrapFrameInfo(TrapType type,TrapStatus status){
    static std::unordered_map<uint32_t ,TrapFrameInfo> table{
            {trapKey(TrapType::FALLING_PLATFORM,TrapStatus::OFF),
                    {TextureType::TRAP_FALLING_PLATFORM_OFF,
                     32,10,1,50,false}},
            {trapKey(TrapType::FALLING_PLATFORM,TrapStatus::ON),
                    {TextureType::TRAP_FALLING_PLATFORM_ON,
                     32,10,4,50,true}},

            {trapKey(TrapType::FAN,TrapStatus::OFF),
                    {TextureType::TRAP_FAN_OFF,
                     24,8,1,50,false}},
            {trapKey(TrapType::FAN,TrapStatus::ON),
                    {TextureType::TRAP_FAN_ON,
                     24,8,4,50,true}},

            {trapKey(TrapType::FIRE,TrapStatus::OFF),
                    {TextureType::TRAP_FIRE_OFF,
                     16,32,1,50,false}},
            {trapKey(TrapType::FIRE,TrapStatus::ON),
                    {TextureType::TRAP_FIRE_ON,
                     16,32,3,50,true}},
            {trapKey(TrapType::FIRE,TrapStatus::HIT),
                    {TextureType::TRAP_FIRE_HIT,
                     16,32,4,50,false}},


            {trapKey(TrapType::MOVING_PLATFORM_BROWN,TrapStatus::OFF),
                    {TextureType::TRAP_PLATFORM_BROWN_OFF,
                     32,8,1,50,false}},
            {trapKey(TrapType::MOVING_PLATFORM_BROWN,TrapStatus::ON),
                    {TextureType::TRAP_PLATFORM_BROWN_ON,
                     32,8,8,50,true}},

            {trapKey(TrapType::MOVING_PLATFORM_GREY,TrapStatus::OFF),
                    {TextureType::TRAP_PLATFORM_GREY_OFF,
                     32,8,1,50,false}},
            {trapKey(TrapType::MOVING_PLATFORM_GREY,TrapStatus::ON),
                    {TextureType::TRAP_PLATFORM_GREY_ON,
                     32,8,8,50,true}},

            {trapKey(TrapType::ROCK_HEAD,TrapStatus::IDLE),
                    {TextureType::TRAP_ROCK_HEAD_BLINK,
                     42,42,4,200,true}},
            {trapKey(TrapType::ROCK_HEAD,TrapStatus::HIT),
                    {TextureType::TRAP_ROCK_HEAD_HIT_BOTTOM,
                     42,42,4,100,false}},

            {trapKey(TrapType::SPIKE_HEAD,TrapStatus::IDLE),
                    {TextureType::TRAP_SPIKE_HEAD_BLINK,
                     54,52,4,200,true}},
            {trapKey(TrapType::SPIKE_HEAD,TrapStatus::HIT),
                    {TextureType::TRAP_SPIKE_HEAD_HIT_BOTTOM,
                     54,52,4,100,false}},


            {trapKey(TrapType::SAW,TrapStatus::OFF),
                    {TextureType::TRAP_SAW_OFF,
                     38,38,1,50,false}},
            {trapKey(TrapType::SAW,TrapStatus::ON),
                    {TextureType::TRAP_SAW_ON,
                     38,38,8,50,true}},

            {trapKey(TrapType::SPIKE_BALL,TrapStatus::IDLE),
                    {TextureType::TRAP_SPIKE_BALL,
                     28,28,1,50,false}},


            {trapKey(TrapType::SPIKES,TrapStatus::IDLE),
                    {TextureType::TRAP_SPIKE,
                     16, 16, 1,50, false}},

            {trapKey(TrapType::TRAMPOLINE,TrapStatus::IDLE),
                    {TextureType::TRAP_TRAMPOLINE,
                     28, 28, 1,50, false}},
            {trapKey(TrapType::TRAMPOLINE,TrapStatus::TRIGGERED),
                    {TextureType::TRAP_TRAMPOLINE_TRIGGER,
                     28, 28, 8,50, false}}

    };

    auto it = table.find(trapKey(type, status));
    return it != table.end() ? &it->second : nullptr;
}