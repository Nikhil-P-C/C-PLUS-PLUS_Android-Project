//
// Created by LENOVO on 30-04-2026.
//
#include "SDL3_ttf/SDL_ttf.h"
#include "utils/utils.h"
#include "State/State.h"
#include "engine/Engine.h"
#include "States/GameState/GameState.h"
#include "States/MenuState/TitleScreenState.h"

void Engine::run(){
    m_assetManager.init(m_renderer);

    pushState(std::make_unique<TitleScreenState>(m_renderer));

    unsigned int lastTime = SDL_GetTicks();
    unsigned int currentTime;
    int framedelay = 1000/60;

    GameData::getInstance().setSize(1600,720);
    LOGI("height:%d,width:%d",GameData::getInstance().getWinHeight(),GameData::getInstance().getWinWidth());


    float lastMasterAudioScale = GameData::getInstance().getAudioScale();
    float nowMasterAudioScale =0.0f;

    float nowMusicAudioScale =0.0f;
    float lastMusicAudioScale = GameData::getInstance().getMusicAudioScale();
    while(m_running){
        nowMasterAudioScale = GameData::getInstance().getAudioScale();
        if(nowMasterAudioScale != lastMasterAudioScale)
            MIX_SetMixerGain(m_mixer,GameData::getInstance().getAudioScale());

        nowMusicAudioScale = GameData::getInstance().getMusicAudioScale();
        if(nowMusicAudioScale != lastMusicAudioScale)
            MIX_SetTrackGain(m_track,GameData::getInstance().getMusicAudioScale());
        lastMasterAudioScale = nowMasterAudioScale;
        lastMusicAudioScale = nowMusicAudioScale;

        currentTime = SDL_GetTicks();
        float deltaTime =(float) (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;
        //RENDER
        for (auto& cmd : m_CommandQueue) {
            if (cmd.type == commandType::PUSH)
                m_States.push_back(std::move(cmd.state));

            else if (cmd.type == commandType::POP)
                if (!m_States.empty())
                    m_States.pop_back();

        }
        m_CommandQueue.clear();


        SDL_RenderClear(m_renderer);
        for(const auto &state : m_States)
            state->render(m_renderer);



        for (auto& cmd : m_CommandOverlayQueue) {
            if (cmd.type == commandType::PUSH)
                m_OverlayStates.push_back(std::move(cmd.state));

            else if (cmd.type == commandType::POP)
                if (!m_OverlayStates.empty())
                    m_OverlayStates.pop_back();

        }
        m_CommandOverlayQueue.clear();
        for(const auto &state : m_OverlayStates)
            state->render(m_renderer);


        SDL_RenderPresent(m_renderer);

        //polling events from states

        while(SDL_PollEvent(&m_event)) {

            for (auto it = m_OverlayStates.rbegin(); it != m_OverlayStates.rend() ; ++it)
                if((*it)->handleEvents(m_event))break;
            for (auto it = m_States.rbegin(); it != m_States.rend(); ++it)
                if((*it)->handleEvents(m_event))break;
        }

        //updating states


        for(const auto &state : m_OverlayStates){
            state->update(deltaTime);
        }

        if (!m_States.empty())
            m_States.back()->update(deltaTime);


        unsigned int frametime = SDL_GetTicks() - currentTime;
        if(frametime < framedelay){
            SDL_Delay(framedelay - frametime);
        }
    }
}
void Engine::exitEngine(){
    m_running =false;
    if(m_mixer)MIX_DestroyMixer(m_mixer);
    if(m_audio)MIX_DestroyAudio(m_audio);
    if(m_musicfile)SDL_CloseIO(m_musicfile);
}
Engine::Engine(){
    LOGI("engine constructor");

    if(!MIX_Init()){
        LOGI("failed SDL_mixer:%s", SDL_GetError());
    }
    if (!SDL_Init(SDL_INIT_VIDEO))
        LOGI("failed SDL:%s", SDL_GetError());
    if (!TTF_Init())
        LOGI("failed ttf:%s", SDL_GetError());
    m_window = SDL_CreateWindow("Dino", 0, 0, SDL_WINDOW_FULLSCREEN);
    if (!m_window) {
        LOGE("Window creation failed: %s", SDL_GetError());
    }

    m_renderer = SDL_CreateRenderer(m_window, "opengles2");
    if (!m_renderer) {
        LOGE("Renderer creation failed: %s", SDL_GetError());
    }
    SDL_SetRenderLogicalPresentation(m_renderer,1600,720,SDL_LOGICAL_PRESENTATION_LETTERBOX);
    m_mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (!m_mixer) {
        LOGE("Mixer creation failed: %s", SDL_GetError());
    }

    m_audio = MIX_LoadAudio_IO(m_mixer,m_musicfile,false,false);
    m_track = MIX_CreateTrack(m_mixer);
    MIX_SetTrackAudio(m_track, m_audio);
    MIX_PlayTrack(m_track,true);
    MIX_SetTrackLoops(m_track,-1);

    const char* name = SDL_GetRendererName(m_renderer);
    LOGI("Renderer: %s", name);
}
Engine::~Engine(){
    LOGI("engine destructor");
    if(m_window)SDL_DestroyWindow(m_window);
    if(m_renderer)SDL_DestroyRenderer(m_renderer);
    SDL_Quit();
}



