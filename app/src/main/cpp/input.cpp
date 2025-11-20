//
// Created by LENOVO on 09-10-2025.
//
#include <android/log.h>

#define LOG_TAG "INPUT"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#include <cmath>
#include "input.h"

void Input::eventhandler(bool& running,int windowW,int windowH,float& x,float& y,float& deltaTime) {
    float groundlevel =(float)windowH-24*5;
    while(SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                running = false;
                break;
            case SDL_EVENT_FINGER_DOWN:
            case SDL_EVENT_FINGER_MOTION:
                TouchX = event.tfinger.x * (float)windowW;
                TouchY = event.tfinger.y * (float)windowH;

                if (TouchX >= joystick.x &&
                    TouchX <= joystick.x + joystick.w &&
                    TouchY >= joystick.y &&
                    TouchY <= joystick.y + joystick.h)hold = true;
                break;
            case SDL_EVENT_FINGER_UP:
                jumping =false;
                hold = false;
                P_action =IDLE;
                break;
            default:
                break;
        }
    }
    if(y >groundlevel){
        y =groundlevel;
        isgrounded =true;
        jumping = false;
        P_action =IDLE;
    }

    if(!jumping)y +=  (gravity * 9.0f) * deltaTime;



    if(hold) {
        float centerX = joystick.x + joystick.w / 2;
        float centerY = joystick.y + joystick.h / 2;
        float dY = TouchY - centerY;
        float dX = TouchX - centerX;
        float len = sqrtf(dX * dX + dY * dY);
        if (len > 0) {
            dX /= len;
            dY /= len;
        }
        if (dX > 0 && isgrounded) {
            P_action = MOVINGRIGHT;
            isFacingRight = true;
        } else if(isgrounded) {
            P_action = MOVINGLEFT;
            isFacingRight = false;
        }
        if (dY < 0 && isgrounded && dX < 0.9 && dX > -0.9){
            isgrounded =false;
            P_action = CROUCHING;
            velocityY = -35.00f;
            jumping = true;
        }

        if (jumping) {
            y+= velocityY;
            velocityY += gravity  * deltaTime;
            if (y >= groundlevel) {
                y = groundlevel;
                isgrounded = true;
                jumping = false;
                velocityY = 0;
                P_action = IDLE;
            }
        }


        if(jumping){
            if(dX>0)x += 250 * deltaTime;
            else if(dX<0)x -= 250*deltaTime;
        }
        else{
            if(dX>0)x += 250*deltaTime;
            else if(dX<0)x -= 250*deltaTime;
        }
    }

}
//gets player animation indices for sprite rendering
void Input::getAnimationindexes(Animation& animation) {
        switch(P_action){
            case IDLE:
                animation.startIndex=0;
                animation.lastIndex =3;
                break;
            case MOVINGLEFT:
            case MOVINGRIGHT:
                animation.startIndex=4;
                animation.lastIndex =10;
                break;
            case HURT:
                animation.startIndex=14;
                animation.lastIndex =16;
                break;
            case CROUCHING:
                animation.startIndex=17;
                animation.lastIndex =23;
                break;
        }


}


