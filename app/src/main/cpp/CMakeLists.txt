cmake_minimum_required(VERSION 3.22.1)
project(MyApp LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(ANDROID)
    set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
endif()


# --- Disable host-only SDL3 features ---
set(SDL_HIDAPI OFF CACHE BOOL "Disable HIDAPI" FORCE)
set(SDL_ALSA OFF CACHE BOOL "Disable ALSA" FORCE)
set(SDL_ASAN OFF CACHE BOOL "Disable ASAN" FORCE)
set(SDL_AVX OFF CACHE BOOL "Disable AVX" FORCE)
set(SDL_AVX2 OFF CACHE BOOL "Disable AVX2" FORCE)
set(SDL_AVX512F OFF CACHE BOOL "Disable AVX512F" FORCE)
set(SDL_TEST OFF CACHE BOOL "Disable SDL test" FORCE)

# --- SDL build type ---


# --- Add SDL3 ---
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/SDL3 EXCLUDE_FROM_ALL)
message(WARNING "SDL3 shared: ${SDL_SHARED}, static: ${SDL_STATIC}, build shared: ${BUILD_SHARED_LIBS}")#used for debugging :To do- can be removed
# ---- SDL_image ----
# Enable only what is needed
set(SDLIMAGE_AVIF OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_BMP  ON  CACHE BOOL "" FORCE)
set(SDLIMAGE_GIF  ON CACHE BOOL "" FORCE)
set(SDLIMAGE_JPG  ON  CACHE BOOL "" FORCE)
set(SDLIMAGE_PNG  ON  CACHE BOOL "" FORCE)
set(SDLIMAGE_TIF  OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_WEBP OFF CACHE BOOL "" FORCE)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/SDL3_image EXCLUDE_FROM_ALL)
# ---- SDL_ttf ----
set(SDL_DISABLE_VERSIONING ON)
set(SDLTTF_VENDORED ON)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/SDL_ttf EXCLUDE_FROM_ALL)

#---- SDL_mixer ----
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

# --- Your main native library ---
add_executable(main 
        main.cpp
        States/inputOverlayState/JoystickOverlay.cpp
        States/inputOverlayState/ButtonOverlay.cpp
        States/inputOverlayState/SepJoysticknButton.cpp
        States/DebugState/DebugState.cpp
        States/HUDOverlayState/HUDOverlayState.cpp
        States/GameState/GameState.cpp
        States/GameState/ParticleSystem.cpp
        States/MenuState/TitleScreenState.cpp
        States/MenuState/MenuState.cpp
        States/MenuState/OptionMenuState/OptionMenuState.cpp
        States/MenuState/OptionMenuState/ControlMenuState.cpp
        States/MenuState/OptionMenuState/AudioMenuState.cpp
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
