#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <enet/enet.h>
#include "utils/utils.h"
#include "engine/Engine.h"

//TODO : Change all unsigned int to uint32_t for type size safety
extern "C" int SDL_main(int argc, char* argv[]) {
    Engine::Get().run();
    return 0;
}
