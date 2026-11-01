//
// FrameClock: the clock that RENDER code reads.
//
// Normally it is just SDL_GetTicks(). During the parry freeze-frame it is stopped at the tick of the clash, so every
// wall-clock driven visual (white flashes, stun pulses, light flicker, the arena-gate pulse, the i-frame blink...)
// holds the exact pose it had on the parry frame instead of animating behind the freeze.
// Gameplay timers keep using SDL_GetTicks(); GameState pushes them forward by the frozen time when the freeze ends.
//
#pragma once
#include <SDL3/SDL.h>
#include <cstdint>

namespace FrameClock {
    inline bool&     frozenFlag() { static bool f = false; return f; }
    inline uint32_t& frozenAt()   { static uint32_t t = 0; return t; }

    inline uint32_t now()       { return frozenFlag() ? frozenAt() : (uint32_t)SDL_GetTicks(); }
    inline bool     isFrozen()  { return frozenFlag(); }
    inline void     freeze()    { if(!frozenFlag()){ frozenAt() = (uint32_t)SDL_GetTicks(); frozenFlag() = true; } }
    inline void     thaw()      { frozenFlag() = false; }
}
