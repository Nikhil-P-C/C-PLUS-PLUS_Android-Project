//
// Created by LENOVO on 21-11-2025.
//

#ifndef MY_APP_PLAYER_H
#define MY_APP_PLAYER_H
class Player{
public:
    float x =100.00f;
    float y =720 - 120;
    float w =(float)(24*5);
    float h =(float)(24*5);
    Player() =default;
    Player(float x,float y,float w,float h):x(x),y(y),w(w),h(h){}
};
#endif //MY_APP_PLAYER_H
