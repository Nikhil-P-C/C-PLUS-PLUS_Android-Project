//
// Created by LENOVO on 21-11-2025.
//
#ifndef MY_APP_PLAYER_H
#define MY_APP_PLAYER_H

class Player{
public:
    float x =0.00f;
    float y =0.00f;
    float w =0.00f;
    float h =0.00f;
    Player() = default;

    void setSize(float width, float height) {
        w = width;
        h = height;
    }

    void setPosition(float px, float py, int windowH,int SPRITE_HEIGHT, int P_scale) {
        x = (float) 100.00f;
        y = (float) (windowH - SPRITE_HEIGHT * P_scale);
    }
};

#endif //MY_APP_PLAYER_H
