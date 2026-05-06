//
// Created by LENOVO on 30-04-2026.
//
#include "SDL3_ttf/SDL_ttf.h"

#include "State/State.h"
#include "engine/Engine.h"
#include "States/GameState/GameState.h"
#include "States/MenuState/TitleScreenState.h"
void Engine::initEngine(){
    LOGI("initEngine");
    if(MIX_Init()){
        LOGI("failed SDL_mixer:%s", SDL_GetError());
    }
    if (SDL_Init(SDL_INIT_VIDEO))
        LOGI("failed SDL:%s", SDL_GetError());
    if (TTF_Init())
        LOGI("failed ttf:%s", SDL_GetError());
    m_window = SDL_CreateWindow("Dino", 0, 0, SDL_WINDOW_FULLSCREEN);
    if (!m_window) {
        LOGE("Window creation failed: %s", SDL_GetError());
    }

    m_renderer = SDL_CreateRenderer(m_window, "opengles2");
    if (!m_renderer) {
        LOGE("Renderer creation failed: %s", SDL_GetError());
    }
    m_mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (!m_mixer) {
        LOGE("Mixer creation failed: %s", SDL_GetError());
    }
    m_audio = MIX_LoadAudio_IO(m_mixer,m_musicfile,false,false);
    m_track = MIX_CreateTrack(m_mixer);
    MIX_SetTrackAudio(m_track, m_audio);
    MIX_PlayTrack(m_track,false);

    const char* name = SDL_GetRendererName(m_renderer);
    LOGI("Renderer: %s", name);
    pushState(std::make_unique<TitleScreenState>(m_renderer));
}
