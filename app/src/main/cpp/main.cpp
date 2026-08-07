#include <SDL3/SDL.h>
#include <enet/enet.h>
#include <cmath>
#include "engine/Engine.h"
#define LOG_TAG "MYSDLAPP"
#define LOGI(...)
#define LOGE(...)

int main(int argc, char* argv[]) {
    Engine::Get().run();
    return 0;
}
