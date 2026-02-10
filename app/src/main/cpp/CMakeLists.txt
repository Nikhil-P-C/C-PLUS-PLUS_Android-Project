cmake_minimum_required(VERSION 3.22.1)
project(MyApp LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)


# --- Android build hints ---
set(ANDROID TRUE CACHE BOOL "Building for Android" FORCE)
set(SDL_USE_ANDROID_TOOLCHAIN TRUE CACHE BOOL "" FORCE)
# --- Enable Android platform in SDL ---
set(SDL_SYSTEM_ANDROID ON CACHE BOOL "" FORCE)
set(SDL_PLATFORM "android" CACHE STRING "SDL platform" FORCE)

# --- Disable host-only SDL3 features ---
set(SDL_HIDAPI OFF CACHE BOOL "Disable HIDAPI" FORCE)
set(SDL_ALSA OFF CACHE BOOL "Disable ALSA" FORCE)
set(SDL_ASAN OFF CACHE BOOL "Disable ASAN" FORCE)
set(SDL_AVX OFF CACHE BOOL "Disable AVX" FORCE)
set(SDL_AVX2 OFF CACHE BOOL "Disable AVX2" FORCE)
set(SDL_AVX512F OFF CACHE BOOL "Disable AVX512F" FORCE)
set(SDL_TEST OFF CACHE BOOL "Disable SDL test" FORCE)

# --- SDL build type ---
set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libs" FORCE)
set(SDL_STATIC OFF CACHE BOOL "Disable static SDL" FORCE)
set(SDL_SHARED ON CACHE BOOL "Enable shared SDL" FORCE)

# --- Add SDL3 ---
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/SDL3 EXCLUDE_FROM_ALL)
message(WARNING "SDL3 shared: ${SDL_SHARED}, static: ${SDL_STATIC}, build shared: ${BUILD_SHARED_LIBS}")
# ---- SDL_image ----
# Enable only what we need (optional)
set(SDLIMAGE_AVIF OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_BMP  ON  CACHE BOOL "" FORCE)
set(SDLIMAGE_GIF  OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_JPG  ON  CACHE BOOL "" FORCE)
set(SDLIMAGE_PNG  ON  CACHE BOOL "" FORCE)
set(SDLIMAGE_TIF  OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_WEBP OFF CACHE BOOL "" FORCE)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/SDL3_image EXCLUDE_FROM_ALL)
# --- Your main native library ---
add_library(main SHARED
        main.cpp
        game.cpp
        input.h
        input.cpp
        Player.h
        platform.h
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
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/SDL3_image/include
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/SDL3/include
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/enet-1.3.18/include
)

# --- Link libraries ---
target_link_libraries(main
        SDL3_image::SDL3_image
        SDL3::SDL3
        enet
        android
        log
)
