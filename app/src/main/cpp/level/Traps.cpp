//
// Created by LENOVO on 24-07-2026.
//
#include "Traps.h"

std::vector<Trap> &TrapBuilder::getTraps()
{
    return m_traps;
}

void TrapBuilder::init(const std::vector<Trap> &traps) {
    m_traps =traps;
}

void TrapBuilder::render(SDL_Renderer *renderer) {

}
void TrapBuilder::update(float dt)
{

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