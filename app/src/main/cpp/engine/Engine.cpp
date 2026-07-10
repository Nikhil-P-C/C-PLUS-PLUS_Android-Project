//
// Created by LENOVO on 30-04-2026.
//
#include "SDL3_ttf/SDL_ttf.h"
#include "utils/utils.h"
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
    SDL_SetRenderLogicalPresentation(m_renderer,1600,720,SDL_LOGICAL_PRESENTATION_LETTERBOX);

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
void Engine::run(){
    unsigned int lastTime = SDL_GetTicks();
    unsigned int currentTime;
    int framedelay = 1000/60;
    const SDL_DisplayMode* mode =SDL_GetCurrentDisplayMode(SDL_GetPrimaryDisplay());
    if(mode)
    {
        LOGI("Display native: %d x %d", mode->w, mode->h);
        int windowW =mode->w;
        int windowH = mode->h;
        GameData::getInstance().setSize(windowW,windowH);
        LOGI("height:%d,width:%d",GameData::getInstance().getWinHeight(),GameData::getInstance().getWinWidth());
    }


    while(m_running){

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


        if(!m_OverlayStates.empty())
            m_OverlayStates.back()->update(deltaTime);

        if (!m_States.empty())
            m_States.back()->update(deltaTime);


        unsigned int frametime = SDL_GetTicks() - currentTime;
        if(frametime < framedelay){
            SDL_Delay(framedelay - frametime);
        }
    }
    SDL_Quit();
}
void Engine::exitEngine(){
    m_running =false;
    if(m_mixer)MIX_DestroyMixer(m_mixer);
    if(m_audio)MIX_DestroyAudio(m_audio);
    if(m_musicfile)SDL_CloseIO(m_musicfile);
}

