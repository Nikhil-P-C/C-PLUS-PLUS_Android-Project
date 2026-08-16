//
// Created by LENOVO on 17-04-2026.
//
#include <SDL3/SDL.h>
#include <string>
#pragma once
#define LOG_TAG "State"

class State{
public:
    State() =default;
    virtual void render(SDL_Renderer* renderer){};
    virtual void update(float dt){};
    virtual bool handleEvents(SDL_Event& event){return false;};
    virtual ~State() =default;
public:
    std::string Name = "state";
};
 //MY_APP_STATE_H
