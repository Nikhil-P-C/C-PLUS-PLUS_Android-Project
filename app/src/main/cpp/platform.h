//
// Created by LENOVO on 07-11-2025.
//

#ifndef MY_APP_PLATFORM_H
#define MY_APP_PLATFORM_H
enum type{GRASS=0,DIRT =1,STONE=2};

class platform {
public:
    float x;
    float y;
    float w;
    float h;
    platform() =default;
    platform(float x,float y,float w,float h):x(x),y(y),w(w),h(h){}
    platform& operator=(const platform& other) {
        this->x = other.x;
        this->y = other.y;
        this->w = other.w;
        this->h = other.h;
        return *this;
    }

};


#endif //MY_APP_PLATFORM_H
