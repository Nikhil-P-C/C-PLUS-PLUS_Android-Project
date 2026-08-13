//
// Created by LENOVO on 12-08-2026.
//
#pragma once
#include "utils/utils.h"
#include <stdint.h>

enum class CheckPointAni{
    NO_FLAG=0,
    FLAG_OUT,
    FLAG_IDLE,
    NONE,
};
struct CheckPoint{
    float x=0.00f;
    float y=0.00f;
    float w=0.00f;
    float h=0.00f;
    CheckPointAni aniType;
    int currentFrame=0;
    uint32_t aniLastTime =0;
    Animation ani;
};