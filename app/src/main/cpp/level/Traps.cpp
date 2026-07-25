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
        auto* info = getTrapFrameInfo(trap.type,trap.status);
        SDL_Texture* texture = Engine::Get().getAssetManager().getTexture(info->texture);
        SDL_FRect Dst{trap.x - camX, trap.y - camY, info->frameW* SCALE, info->frameH * SCALE};
        SDL_FRect Src{0.0f+info->frameW*trap.aniStartFrame,0.0f,
                      static_cast<float>(info->frameW), static_cast<float>(info->frameH)};
        SDL_RenderTexture(renderer,texture,&Src,&Dst);
    }
}
void TrapBuilder::update(float dt)
{

    for(auto &trap:m_traps)
    {
        if(trap.aniDone)continue;
        const auto* info = getTrapFrameInfo(trap.type,trap.status);
        if(!info || info->frameCount<=1) continue;

        unsigned int now = SDL_GetTicks();
        LOGI("delay:%d",info->frameDelay);
        if(now - trap.lastTime > info->frameDelay)
        {
            trap.lastTime =now;
            if(trap.aniStartFrame < info->frameCount-1){
                trap.aniStartFrame += 1;
            }
            else if(info->loop) trap.aniStartFrame=0;
            else {
                trap.aniDone;
            }
        }

    }
}

bool TrapBuilder::isSolid(int trapIndex) {
    auto& trap = m_traps[trapIndex];
    if(trap.type == TrapType::FALLING_PLATFORM)return trap.status == TrapStatus::ON;
    return trapHasPath(trap.type)||trap.type==TrapType::FIRE;
}
gameMath::collisionSide TrapBuilder::resolveTrapCollision(int trapIndex,float &playerX, float &playerY, float playerW, float playerH){
    auto& trap = m_traps[trapIndex];
    SDL_FRect trapCollider= getTrapCollisionBox(trap);
    if(trap.type ==TrapType::FIRE){
        trapCollider.y+=trapCollider.h/2;
        trapCollider.h-=trapCollider.h/2;
    }
    gameMath::collisionSide collision = gameMath::checkcollisionXY(playerX,playerY,trapCollider.x,trapCollider.y,
                                              playerH,playerW,trapCollider.h,trapCollider.w);
    return collision;
}
SDL_FRect TrapBuilder::getTrapCollisionBox(const Trap& trap) {
    auto* info = getTrapFrameInfo(trap.type,trap.status);
    SDL_FRect rect{trap.x,trap.y,info->frameW*SCALE,info->frameH*SCALE};
    return rect;
}

void TrapBuilder::triggerFall(int trapIndex) {
    auto &trap = m_traps[trapIndex];
    if(trap.type != TrapType::FALLING_PLATFORM || trap.status == TrapStatus::ON) return;
    trap.status = TrapStatus::OFF;
    trap.aniStartFrame = 0;
    trap.aniDone = true;
    if(const auto* info = getTrapFrameInfo(trap.type, trap.status)) trap.aniEndFrame = info->frameCount-1;
}

SDL_FRect TrapBuilder::getHazardHitBox(const Trap &trap) {
    const auto* info = getTrapFrameInfo(trap.type,trap.status);
    switch(trap.type){
        case TrapType::SPIKES:
            return { trap.x, trap.y , 12*SCALE, 10*SCALE };
        case TrapType::SAW:
            return { trap.x , trap.y , (info->frameW-6)*SCALE, (info->frameH-6)*SCALE };
        case TrapType::FIRE:
            return { trap.x , trap.y, (info->frameW-6)*SCALE, info->frameH*SCALE };
        case TrapType::SPIKE_BALL:
            return { trap.x , trap.y, (info->frameW-4)*SCALE, (info->frameH-4)*SCALE };
        case TrapType::ROCK_HEAD:
        case TrapType::SPIKE_HEAD:
            // only dangerous while actively hitting, not during the idle blink loop
            return { trap.x, trap.y, (info->frameW-8)*SCALE, (info->frameH-8)*SCALE };
        default:
            return { trap.x, trap.y, info->frameW*SCALE, info->frameH*SCALE };
    }
}


bool TrapBuilder::checkHazard(float playerX, float playerY, float playerW, float playerH,TrapType& outType) {
    for(const auto& trap:m_traps){
        if(!trapHasHit(trap.type))continue;
        bool live;
        switch(trap.type){
            case TrapType::FIRE:   live = trap.status == TrapStatus::ON; break;
            case TrapType::SAW:    live = trap.status == TrapStatus::ON; break;
            case TrapType::ROCK_HEAD:
            case TrapType::SPIKE_HEAD: live = trap.status == TrapStatus::HIT; break;
            default: live = true; break; // Spikes / Spike Ball are always hazardous
        }
        if(!live) continue;

        SDL_FRect box = getHazardHitBox(trap);
        if(gameMath::checkcollision(playerX, playerY, box.x, box.y, playerH, playerW, box.h, box.w)){
            outType = trap.type;
            return true;
        }

    }
    return false;
}


float TrapBuilder::checkFanForce(float playerX, float playerY, float playerW, float playerH) {
    const float FAN_FORCE = -500.0f; // px/s upward, tune against m_gravity/m_jumpVelocity
    for(const auto& trap : m_traps){
        if(trap.type != TrapType::FAN || trap.status != TrapStatus::ON) continue;
        SDL_FRect trapSize = getTrapCollisionBox(trap);
        float w=trapSize.w,h =700+trapSize.h;
        if(gameMath::checkcollision(playerX, playerY, trap.x, trap.y-700, playerH, playerW, h, w))
            return FAN_FORCE;
    }
    return 0.0f;
}
bool TrapBuilder::checkFireCollision(int trapIndex,float playerX, float playerY, float playerW, float playerH) {
    if(trapIndex < 0 || trapIndex >= (int)m_traps.size()) return false;
    Trap& trap = m_traps[trapIndex];
    if(trap.type != TrapType::FIRE) return false;
    SDL_FRect trapSize = getTrapCollisionBox(trap);
    float w=trapSize.w,h =trapSize.h;
    if(!gameMath::checkcollision(playerX, playerY, trap.x, trap.y, playerH, playerW, h, w)) return false;

    trap.status = TrapStatus::HIT;
    trap.aniStartFrame = 0;
    trap.aniDone = false;
    if(const auto* info = getTrapFrameInfo(trap.type, trap.status)) trap.aniEndFrame = info->frameCount-1;
    return true;
}
bool TrapBuilder::checkTrampolineBounce(int trapIndex,float playerX, float playerY, float playerW, float playerH) {
    if(trapIndex < 0 || trapIndex >= (int)m_traps.size()) return false;
    Trap& trap = m_traps[trapIndex];
    if(trap.type != TrapType::TRAMPOLINE) return false;
    SDL_FRect trapSize = getTrapCollisionBox(trap);
    float w=trapSize.w,h =trapSize.h;
    if(!gameMath::checkcollision(playerX, playerY, trap.x, trap.y, playerH, playerW, h, w)) return false;

    trap.status = TrapStatus::TRIGGERED;
    trap.aniStartFrame = 0;
    trap.aniDone = false;
    if(const auto* info = getTrapFrameInfo(trap.type, trap.status)) trap.aniEndFrame = info->frameCount-1;
    return true;
}


Trap::Trap(float x, float y, TrapType type, TrapStatus status, float startPath,float endPath)
        :x(x),y(y),type(type),status(status),startPath(startPath),endPath(endPath)
{

}

const TrapFrameInfo* getTrapFrameInfo(TrapType type,TrapStatus status){
    static std::unordered_map<uint32_t ,TrapFrameInfo> table{
            {trapKey(TrapType::FALLING_PLATFORM,TrapStatus::OFF),
                    {TextureType::TRAP_FALLING_PLATFORM_OFF,32,10,1,50,false}},
            {trapKey(TrapType::FALLING_PLATFORM,TrapStatus::ON),
                    {TextureType::TRAP_FALLING_PLATFORM_ON,32,10,4,50,true}},

            {trapKey(TrapType::FAN,TrapStatus::OFF),
                    {TextureType::TRAP_FAN_OFF,24,8,1,50,false}},
            {trapKey(TrapType::FAN,TrapStatus::ON),
                    {TextureType::TRAP_FAN_ON,24,8,4,50,true}},

            {trapKey(TrapType::FIRE,TrapStatus::OFF),
                    {TextureType::TRAP_FIRE_OFF,16,32,1,50,false}},
            {trapKey(TrapType::FIRE,TrapStatus::ON),
                    {TextureType::TRAP_FIRE_ON,16,32,3,50,false}},
            {trapKey(TrapType::FIRE,TrapStatus::HIT),
                    {TextureType::TRAP_FIRE_HIT,16,32,4,500,false}},


            {trapKey(TrapType::MOVING_PLATFORM_BROWN,TrapStatus::OFF),
                    {TextureType::TRAP_PLATFORM_BROWN_OFF,32,8,1,50,false}},
            {trapKey(TrapType::MOVING_PLATFORM_BROWN,TrapStatus::ON),
                    {TextureType::TRAP_PLATFORM_BROWN_ON,32,8,8,50,true}},

            {trapKey(TrapType::MOVING_PLATFORM_GREY,TrapStatus::OFF),
                    {TextureType::TRAP_PLATFORM_GREY_OFF,32,8,1,50,false}},
            {trapKey(TrapType::MOVING_PLATFORM_GREY,TrapStatus::ON),
                    {TextureType::TRAP_PLATFORM_GREY_ON,32,8,8,50,true}},

            {trapKey(TrapType::ROCK_HEAD,TrapStatus::IDLE),
                    {TextureType::TRAP_ROCK_HEAD_BLINK,42,42,4,200,true}},
            {trapKey(TrapType::ROCK_HEAD,TrapStatus::HIT),
                    {TextureType::TRAP_ROCK_HEAD_HIT_BOTTOM,42,42,4,200,true}},

            {trapKey(TrapType::SPIKE_HEAD,TrapStatus::IDLE),
                    {TextureType::TRAP_ROCK_HEAD_BLINK,54,52,4,200,true}},
            {trapKey(TrapType::SPIKE_HEAD,TrapStatus::HIT),
                    {TextureType::TRAP_ROCK_HEAD_HIT_BOTTOM,54,52,4,200,false}},


            {trapKey(TrapType::SAW,TrapStatus::OFF),
                    {TextureType::TRAP_SAW_OFF,38,38,1,50,false}},
            {trapKey(TrapType::SAW,TrapStatus::ON),
                    {TextureType::TRAP_SAW_ON,38,38,8,50,true}},

            {trapKey(TrapType::SPIKE_BALL,TrapStatus::IDLE),
                    {TextureType::TRAP_SPIKE_BALL,28,28,1,50,false}},


            {trapKey(TrapType::SPIKES,TrapStatus::IDLE),
                    {TextureType::TRAP_SPIKE, 16, 16, 1,50, false}},

            {trapKey(TrapType::TRAMPOLINE,TrapStatus::IDLE),
                    {TextureType::TRAP_TRAMPOLINE, 28, 28, 1,50, false}},
            {trapKey(TrapType::TRAMPOLINE,TrapStatus::TRIGGERED),
                    {TextureType::TRAP_TRAMPOLINE_TRIGGER, 28, 28, 8,50, false}}

    };

    auto it = table.find(trapKey(type, status));
    return it != table.end() ? &it->second : nullptr;
}