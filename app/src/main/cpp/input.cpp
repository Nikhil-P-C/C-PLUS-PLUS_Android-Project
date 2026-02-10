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

     float groundlevel =600;

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
    if(velocityY <0){
        velocityY += gravity  * deltaTime;
    }
    else {
        velocityY += 1000.0f*deltaTime;
    }
    y+= velocityY * deltaTime;
    if(y >=groundlevel){
        y =groundlevel;
        isgrounded =true;
        jumping = false;
        P_action =IDLE;
    }



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

        LOGI("jumping:%d , y:%f,velocityY:%f,gravity:%f,ground:%f",
             jumping,y,velocityY,(gravity * deltaTime),groundlevel);

        if(shouldJump(dX,dY)) {
            LOGI("jumping:%d , y:%f,velocityY:%f,gravity:%f,ground:%f",
                 jumping,y,velocityY,(gravity * deltaTime),groundlevel);
            if (y > groundlevel) {
                jumping = false;
                velocityY =0.00f;
                P_action = IDLE;
            }
        }



        if(dX>0)x += 250 * deltaTime;
        else if(dX<0)x -= 250*deltaTime;


    }

}
bool Input::shouldJump(float& dX, float& dY) {

    if (dY < 0 && isgrounded && dX < 0.9 && dX > -0.9){
        isgrounded =false;
        P_action = CROUCHING;
        velocityY = -850.00f;
        LOGI("should jump?true");
        return true;

    }
    LOGI("should jump?false");
    return false;
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


