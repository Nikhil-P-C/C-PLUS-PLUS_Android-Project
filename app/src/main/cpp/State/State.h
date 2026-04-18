//
// Created by LENOVO on 17-04-2026.
//
#pragma once
class State{
public:
    virtual void render() = 0;
    virtual void update(float dt) = 0;
    virtual void handleEvents() = 0;
};
 //MY_APP_STATE_H
