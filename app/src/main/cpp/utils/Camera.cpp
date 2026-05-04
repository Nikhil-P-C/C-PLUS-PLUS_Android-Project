//
// Created by LENOVO on 03-05-2026.
//
#include "Camera.h"
#include "gameMath.h"

void Camera::lockCameraOn(float& targetX,float& targetY,float targetH,float targetW) {
    float centerX = targetX + (targetW / 2) -m_camera.w/2;
    float centerY = targetY + (targetH / 2) -m_camera.h/2;
//        LOGI("centerX:%f,centerY:%f",centerX,centerY);

    gameMath::interpolate(m_camera.x,m_camera.y,centerX,centerY,0.2f);
//        LOGI("cX:%f,c:%f",camera.x,camera.y);
}