//
// Created by LENOVO on 09-10-2025.
//
#include <android/log.h>

#define LOG_TAG "INPUT"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#include <cmath>
#include "input.h"
#include "gameUtils.h"
#include "platform.h"

void Input::eventhandler(bool& running,int windowW,int windowH,float& x,float& y,float& deltaTime,Platform platforms[]) {

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
    velocityY += gravity *deltaTime;
    acceleration =( velocityY *jumpForce) * deltaTime;
    y+=acceleration;

    if(y >=groundlevel){
        y =groundlevel;
        velocityY =0.00f;
        isgrounded =true;
        jumping = false;
        P_action =IDLE;
    }

    prevJumpPressed =jumpPressed;
    jumpPressed =false;
    if(hold){
        if(dY < 0 && dX < 0.9 && dX > -0.9){
            jumpPressed=true;
            jumping=true;
            P_action = CROUCHING;
        }
    }
    jumpReleased = (!jumpPressed && prevJumpPressed);
    if(jumpReleased) { LOGI("jump released");
        jumping = false;
    }

    if(hold) {
        centerX = joystick.x + joystick.w / 2;
        centerY = joystick.y + joystick.h / 2;
        dY = TouchY - centerY;
        dX = TouchX - centerX;
        len = sqrtf(dX * dX + dY * dY);
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

//        LOGI("jumping:%d , y:%f,velocityY:%f,gravity:%f,ground:%f",
//             jumping,y,velocityY,(gravity * deltaTime),groundlevel);
        if(jumpReleased && velocityY < 0){
              // or 0.4–0.6 depending on feel
              LOGI("jump cancel");
        }
        if(jumping && isgrounded){
            velocityY = -250.0f;
            isgrounded = false;
//               LOGI("jumping:%d , y:%f,velocityY:%f,gravity:%f,ground:%f",
//                    jumping,y,velocityY,(gravity * deltaTime),groundlevel);
        }
        if(!jumping){
            if(dX>0)x += 250 * deltaTime;
            else if(dX<0)x -= 250*deltaTime;
        }
        if(jumping){
            if(dX>0)x += 50 * deltaTime;
            else if(dX<0)x -= 50*deltaTime;
        }

    }

    if(velocityY >=2000)velocityY =2000;


    LOGI("acceleration :%f",acceleration);

}


//bool Input::shouldJump(float& dX, float& dY) {
//
//    if (dY < 0 && isgrounded && dX < 0.9 && dX > -0.9){
//        isgrounded =false;
//        jumping =true;
//        P_action = CROUCHING;
//        velocityY = -250.00f;
////        LOGI("should jump?true");
//        return true;
//
//    }
////    LOGI("should jump?false");
//    return false;
//}
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


