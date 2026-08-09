//
// Created by LENOVO on 08-08-2026.
//
#pragma once
#include "State/State.h"
class KeyboardOverlay:public State{
public:
    void render(SDL_Renderer* renderer)override;
};
