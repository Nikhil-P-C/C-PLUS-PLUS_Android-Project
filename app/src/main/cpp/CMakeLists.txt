cmake_minimum_required(VERSION 3.22.1)
project(MyApp LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# CMake sets ANDROID automatically when configured with the NDK's
# android.toolchain.cmake -- no need to force it, and forcing it here would
# break desktop builds since this file is shared by both platforms.
if(ANDROID)
    set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
endif()

# --- Disable host-only SDL3 features (both platforms) ---
set(SDL_HIDAPI OFF CACHE BOOL "Disable HIDAPI" FORCE)
set(SDL_ALSA OFF CACHE BOOL "Disable ALSA" FORCE)
set(SDL_ASAN OFF CACHE BOOL "Disable ASAN" FORCE)
set(SDL_AVX OFF CACHE BOOL "Disable AVX" FORCE)
set(SDL_AVX2 OFF CACHE BOOL "Disable AVX2" FORCE)
set(SDL_AVX512F OFF CACHE BOOL "Disable AVX512F" FORCE)
set(SDL_TEST OFF CACHE BOOL "Disable SDL test" FORCE)

if(ANDROID)
    # --- Android build hints ---
    set(SDL_USE_ANDROID_TOOLCHAIN TRUE CACHE BOOL "" FORCE)
    set(SDL_SYSTEM_ANDROID ON CACHE BOOL "" FORCE)
    set(SDL_PLATFORM "android" CACHE STRING "SDL platform" FORCE)

    # Android needs SDL built shared, so its .so's can be packaged into the APK.
    set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libs" FORCE)
    set(SDL_STATIC OFF CACHE BOOL "Disable static SDL" FORCE)
    set(SDL_SHARED ON CACHE BOOL "Enable shared SDL" FORCE)
endif()
# Desktop leaves BUILD_SHARED_LIBS/SDL_STATIC/SDL_SHARED unset, same as your
# working desktop config -- SDL falls back to its own default there.

# --- Add SDL3 ---
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/SDL3 EXCLUDE_FROM_ALL)
message(WARNING "SDL3 shared: ${SDL_SHARED}, static: ${SDL_STATIC}, build shared: ${BUILD_SHARED_LIBS}") # debugging, can be removed

# ---- SDL_image ----
set(SDLIMAGE_AVIF OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_BMP  ON  CACHE BOOL "" FORCE)
set(SDLIMAGE_GIF  ON  CACHE BOOL "" FORCE)
set(SDLIMAGE_JPG  ON  CACHE BOOL "" FORCE)
set(SDLIMAGE_PNG  ON  CACHE BOOL "" FORCE)
set(SDLIMAGE_TIF  OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_WEBP OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/SDL3_image EXCLUDE_FROM_ALL)

# ---- SDL_ttf ----
set(SDL_DISABLE_VERSIONING ON)
set(SDLTTF_VENDORED ON)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/SDL_ttf EXCLUDE_FROM_ALL)

# ---- SDL_mixer ----
set(SDLMIXER_WAV ON CACHE BOOL "" FORCE)
set(WAVPACK_BUILD_ASM OFF CACHE BOOL "" FORCE)
set(SDLMIXER_FLAC OFF CACHE BOOL "" FORCE)
set(SDLMIXER_MP3 ON CACHE BOOL "" FORCE)
set(SDLMIXER_OGG OFF CACHE BOOL "" FORCE)
set(SDLMIXER_OPUS OFF CACHE BOOL "" FORCE)
set(BUILD_PROGRAMS OFF)
set(BUILD_EXAMPLES OFF)
set(BUILD_TESTING OFF)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/SDL_mixer EXCLUDE_FROM_ALL)

# --- Shared source list ---
set(MYAPP_SOURCES
        main.cpp
        States/inputOverlayState/JoystickOverlay.cpp
        States/inputOverlayState/ButtonOverlay.cpp
        States/inputOverlayState/SepJoysticknButton.cpp
        States/inputOverlayState/KeyboardOverlay.cpp
        States/DebugState/DebugState.cpp
        States/HUDOverlayState/HUDOverlayState.cpp
        States/GameState/GameState.cpp
        States/GameState/ParticleSystem.cpp
        States/MenuState/TitleScreenState.cpp
        States/MenuState/MenuState.cpp
        States/MenuState/OptionMenuState/OptionMenuState.cpp
        States/MenuState/OptionMenuState/ControlMenuState.cpp
        States/MenuState/OptionMenuState/AudioMenuState.cpp
        States/MenuState/OptionMenuState/AdvanceMenuState.cpp
        States/MenuState/EditMenuState/EditMenuState.cpp
        States/PauseState/PauseState.cpp

        engine/Engine.cpp
        engine/AssetManager.cpp

        utils/Camera.cpp
        utils/gameMath.cpp
        utils/SpriteCollector.cpp

        level/GroundShapeBuilder.cpp
        level/BlockShapeBuilder.cpp
        level/Fruits.cpp
        level/Traps.cpp
        level/LevelLoader/LevelLoader.cpp
)

# --- Your main native library / executable ---
# Android's Gradle/NDK build expects a shared library (loaded via JNI);
# desktop needs a normal executable with a real main().
if(ANDROID)
    add_library(main SHARED ${MYAPP_SOURCES})
else()
    add_executable(main ${MYAPP_SOURCES})
endif()

target_compile_definitions(main PRIVATE SDL_DISABLE_HIDAPI)

# --- ENet ---
file(GLOB ENET_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/enet-1.3.18/*.c
)
add_library(enet STATIC ${ENET_SRC})
target_include_directories(enet PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/enet-1.3.18/include
)

# --- Include directories ---
target_include_directories(main PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/SDL_ttf/include
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/SDL3_image/include
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/SDL3/include
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/SDL_mixer/include
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/enet-1.3.18/include
        ${CMAKE_CURRENT_SOURCE_DIR}/
)

# --- Link libraries ---
target_link_libraries(main
        SDL3_mixer::SDL3_mixer
        SDL3_ttf::SDL3_ttf
        SDL3_image::SDL3_image
        SDL3::SDL3
        enet
)

if(ANDROID)
    target_link_libraries(main
            android
            log
    )
endif()