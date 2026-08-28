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

void Camera::cameraClamp(SDL_FRect rect) {
    LOGI("wall col x:%f , y:%f , w:%f , h:%f",rect.x,rect.y,rect.w,rect.h);

    if(m_camera.x > rect.x+rect.w-1600)m_camera.x= rect.x+rect.w-1600;
    if(m_camera.x < rect.x)m_camera.x = rect.x;
    if(m_camera.y > rect.y+rect.h-720)m_camera.y= rect.y+rect.h-720;
    if(m_camera.y < rect.y)m_camera.y = rect.y;
}
