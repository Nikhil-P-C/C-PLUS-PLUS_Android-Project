//
// Created by LENOVO on 17-04-2026.
//

#include "State/State.h"

#include <SDL3/SDL.h>
#include <android/log.h>
#include <vector>
#include <memory>
#define LOG_TAG "Engine"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#pragma once//MY_APP_ENGINE_H
struct Event{
    bool handled =false;
    SDL_Event event;
};
class Engine{

public:
    Engine(){
        LOGI("engine constructor");
        initEngine();
    }
    ~Engine(){
        LOGI("engine destructor");
        if(m_window)SDL_DestroyWindow(m_window);
        if(m_renderer)SDL_DestroyRenderer(m_renderer);
        SDL_Quit();
    }
    static Engine& Get(){
        static Engine instance;
        return instance;
    }
    void pushState(std::unique_ptr<State> state){
        m_States.push_back(std::move(state));
    }
    void popState(){
        m_States.pop_back();
    }
    void changeState(std::unique_ptr<State> state){
        popState();
        pushState(std::move(state));
    }
    void initEngine();

    void run(){
        unsigned int lastTime = SDL_GetTicks();
        unsigned int currentTime;
        int framedelay = 1000/60;
        while(true){
            currentTime = SDL_GetTicks();
            float deltaTime =(float) (currentTime - lastTime) / 1000.0f;
            lastTime = currentTime;

            while(SDL_PollEvent(&m_event)) {
                for (auto it = m_States.rbegin(); it != m_States.rend(); ++it)
                    (*it)->handleEvents(m_event);
            }


            for(const auto &state : m_States)
                state->update(0.0f);


            SDL_RenderClear(m_renderer);
            for(const auto &state : m_States)
                state->render(m_renderer);
            SDL_RenderPresent(m_renderer);
//            m_CurrentState->handleEvents(m_event);
//
//            m_CurrentState->update(0.0f);
//
//            m_CurrentState->render(m_renderer);
            unsigned int frametime = SDL_GetTicks() - currentTime;
            if(frametime < framedelay){
                SDL_Delay(framedelay - frametime);
            }
        }
    }

private:
    State* m_CurrentState= nullptr;
    std::vector<std::unique_ptr<State>> m_States;
    SDL_Window*   m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    SDL_Event     m_event;
};
