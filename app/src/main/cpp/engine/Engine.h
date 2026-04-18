//
// Created by LENOVO on 17-04-2026.
//
#include "State/State.h"
#pragma once//MY_APP_ENGINE_H
class Engine{

public:
Engine()=default;



    void run(){
    CurrentState = getStates();// to do- implement the Functions
    while(true){
        CurrentState->handleEvents();
        CurrentState->update(0.0f);
        CurrentState->render();
    }
    }

private:
    State* CurrentState= nullptr;
};
