#include <SDL3/SDL.h>
#include <android/log.h>
#include <enet/enet.h>
#include <cmath>
#include "game.cpp"
#define LOG_TAG "MYSDLAPP"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

extern "C" int SDL_main(int argc, char* argv[]) {
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");

    Game game;
    game.run();
    return 0;
}
