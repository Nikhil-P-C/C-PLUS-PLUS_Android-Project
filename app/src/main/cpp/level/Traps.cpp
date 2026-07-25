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

int TrapBuilder::onCollision(float x,float y,float w,float h)
{

}
constexpr bool TrapBuilder::trapHasPath(TrapType type)
{
    return false;
}
constexpr bool TrapBuilder::trapHasHit(TrapType type)
{
    return false;
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
                    {TextureType::TRAP_FIRE_ON,16,32,3,50,true}},
            {trapKey(TrapType::FIRE,TrapStatus::HIT),
                    {TextureType::TRAP_FIRE_HIT,16,32,4,50,false}},


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
                    {TextureType::TRAP_TRAMPOLINE_TRIGGER, 28, 28, 8,50, true}}

    };

    auto it = table.find(trapKey(type, status));
    return it != table.end() ? &it->second : nullptr;
}