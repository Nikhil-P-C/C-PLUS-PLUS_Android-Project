#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <enet/enet.h>
#include "utils/utils.h"
#include <cmath>
#include "engine/Engine.h"
#define LOG_TAG "MYSDLAPP"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
//TODO : Change all unsigned int to uint32_t for type size safety
extern "C" int SDL_main(int argc, char* argv[]) {
    Engine::Get().run();
    return 0;
}
