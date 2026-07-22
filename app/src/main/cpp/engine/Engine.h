//
// Created by LENOVO on 17-04-2026.
//

#include "State/State.h"
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3/SDL.h>
#include <android/log.h>
#include <vector>
#include <memory>

#define LOG_TAG "Engine"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#pragma once//MY_APP_ENGINE_H
enum class commandType{
    PUSH,
    POP,
};
struct Command{
    commandType type;
    std::unique_ptr<State> state;
};
struct Event{
    bool handled =false;
    SDL_Event event;
};
class Engine{

public:
    Engine();
    ~Engine();
    void run();
    void exitEngine();

    static Engine& Get(){
        static Engine instance;
        return instance;
    }
    void pushState(std::unique_ptr<State> state){
        m_CommandQueue.push_back({commandType::PUSH, std::move(state)});
    }

    void popState(){
        m_CommandQueue.push_back({commandType::POP, nullptr});
    }
    void changeState(std::unique_ptr<State> state){
        popState();
        pushState(std::move(state));
    }
    void pushOverlayState(std::unique_ptr<State> state){
        m_CommandOverlayQueue.push_back({commandType::PUSH, std::move(state)});
    }

    void popOverlayState(){
        m_CommandOverlayQueue.push_back({commandType::POP, nullptr});
    }
    void changeOverlayState(std::unique_ptr<State> state){
        popOverlayState();
        pushOverlayState(std::move(state));
    }
    bool isOverlayStatesEmpty(){
        return m_OverlayStates.empty();
    }
    bool isStatesEmpty(){
        return m_States.empty();
    }
    size_t getOverlayStateCount(){
        return m_OverlayStates.size();
    }
    size_t getStateCount(){
        return m_States.size();
    }
    SDL_Window* getWindow(){
        return m_window;
    }
private:
    std::vector<Command> m_CommandQueue;
    std::vector<std::unique_ptr<State>> m_States;

    std::vector<Command> m_CommandOverlayQueue;
    std::vector<std::unique_ptr<State>> m_OverlayStates;

    SDL_Window*   m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    SDL_Event     m_event;
    SDL_IOStream* m_musicfile = SDL_IOFromFile("music.mp3", "rb");


    MIX_Audio* m_audio = nullptr;
    MIX_Mixer* m_mixer =nullptr;
    MIX_Track* m_track = nullptr;

    bool m_running =true;


};
