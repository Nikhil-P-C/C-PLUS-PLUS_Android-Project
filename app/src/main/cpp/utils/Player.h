//
// Created by LENOVO on 03-05-2026.
//
#pragma once

class Player{
public:
    float x =0.00f;
    float y =0.00f;
    float w =0.00f;
    float h =0.00f;

    float spriteOffsetX;
    float spriteOffsetY;

    float spriteW;
    float spriteH;
    Player() = default;
    void setSpriteOffset(float x, float y) {
        spriteOffsetX = x;
        spriteOffsetY = y;
    }
    void setSpriteSize(float w, float h) {
        spriteW = w;
        spriteH = h;
    }
    void setSize(float width, float height) {
        w = width;
        h = height;
    }

    void setPosition(float px, float py, int windowH,int SPRITE_HEIGHT, int P_scale) {
        x = (float) px;
        y = (float) (windowH - SPRITE_HEIGHT * P_scale);
    }
};

