//
// Created by LENOVO on 09-10-2025.
//
#include "SDL3/SDL.h"
#include <android/log.h>
#include "game.h"
#include <cmath>


#ifndef MY_APP_INOUT_H
#define MY_APP_INOUT_H
enum Action{
    IDLE=0,
    MOVINGLEFT=1,
    MOVINGRIGHT=2,
    HURT=3,
    CROUCHING=5
};
class Input {
private:

    bool isFacingRight =true;
    Action P_action =IDLE;
    SDL_Event event;
    bool hold =false;
    float TouchX =0.00f;
    float TouchY =0.00f;
    float gravity =95.9f;
    float velocityY= 0.00f;
    float acceleration =0.00f;
    float jumpForce =10.00f;
    bool jumping =false;

    bool isgrounded =false;
    SDL_FRect joystick{200,500,200,200};

public:
    Input(SDL_Event& event){
        this->event =event;
    }
    void eventhandler(bool& running,int windowW,int windowH,float& x,float& y,float& deltaTime);//does the input handling

    SDL_FRect getJoystick(){
        return joystick;
        //returns the joystick pos
    }
    int getPlayerAction(){
        return P_action;
        //returns the player action
    }
    bool getPlayerFacingdir() const{
        return isFacingRight;
        //returns the player facing direction true if right false if left
    }
    void getAnimationindexes(Animation& animation);// gets player animation indices for sprite rendering

    bool shouldJump(float& dX,float& dY);
};


#endif //MY_APP_INOUT_H
