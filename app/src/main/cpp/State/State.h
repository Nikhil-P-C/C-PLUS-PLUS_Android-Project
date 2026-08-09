//
// Created by LENOVO on 17-04-2026.
//
#include <SDL3/SDL.h>
#pragma once
#define LOG_TAG "State"

class State{
public:
    State() =default;
    virtual void render(SDL_Renderer* renderer){};
    virtual void update(float dt){};
    virtual bool handleEvents(SDL_Event& event){};
    virtual ~State() =default;

};
 //MY_APP_STATE_H
