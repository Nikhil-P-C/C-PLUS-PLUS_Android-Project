//
// Created by LENOVO on 07-11-2025.
//
#pragma once

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
    ColliderType colliderType =ColliderType::SOLID;
public:
    Platform() =default;
    Platform(float x,float y,float w,float h,ColliderType type):x(x),y(y),w(w),h(h),colliderType(type){}
    Platform& operator=(const Platform& other) {
        this->x = other.x;
        this->y = other.y;
        this->w = other.w;
        this->h = other.h;
        this->colliderType = other.colliderType;
        return *this;
    }
};

