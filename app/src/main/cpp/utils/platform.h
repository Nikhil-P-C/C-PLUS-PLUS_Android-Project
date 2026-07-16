//
// Created by LENOVO on 07-11-2025.
//
#pragma once
#include "SpriteCollector.h"
enum class ColliderType{
    SOLID=0,
    ONE_WAY=1,
    TRIGGER=3
};
class Platform {
public:
    float x;
    float y;
    float w;
    float h;
    SpriteType platformType;
    ColliderType colliderType;
public:
    Platform() =default;
    Platform(float x,float y,float w,float h,ColliderType cType,SpriteType sType):x(x),y(y),w(w),h(h),colliderType(cType),platformType(sType){}
    Platform& operator=(const Platform& other) {
        this->x = other.x;
        this->y = other.y;
        this->w = other.w;
        this->h = other.h;
        this->colliderType = other.colliderType;
        this->platformType =other.platformType;
        return *this;
    }
};

