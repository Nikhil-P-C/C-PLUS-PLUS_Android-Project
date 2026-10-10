//
// Created by LENOVO on 03-05-2026.
//
#include "Camera.h"
#include "gameMath.h"
#include <cmath>
#include <cstdlib>

void Camera::lockCameraOn(float& targetX,float& targetY,float targetH,float targetW) {
    float centerX = targetX + (targetW / 2) -m_camera.w/2;
    float centerY = targetY + (targetH / 2) -m_camera.h/2;
//        LOGI("centerX:%f,centerY:%f",centerX,centerY);

    gameMath::interpolate(m_camera.x,m_camera.y,centerX,centerY,0.2f);
//        LOGI("cX:%f,c:%f",camera.x,camera.y);
}

void Camera::cameraClamp(SDL_FRect rect) {

    if(m_camera.x > rect.x+rect.w-1600)m_camera.x= rect.x+rect.w-1600;
    if(m_camera.x < rect.x)m_camera.x = rect.x;
    if(m_camera.y > rect.y+rect.h-720)m_camera.y= rect.y+rect.h-720;
    if(m_camera.y < rect.y)m_camera.y = rect.y;
}

void Camera::updateShake(float dt) {
    m_trauma -= 1.8f * dt;                       // ~0.55s from full trauma to calm
    if(m_trauma < 0.0f) m_trauma = 0.0f;
    m_kickX *= std::exp(-18.0f * dt);            // kick dies out in ~0.15s
    if(std::fabs(m_kickX) < 0.5f) m_kickX = 0.0f;

    const float s = m_trauma * m_trauma;
    const float rx = (std::rand() / (float)RAND_MAX) * 2.0f - 1.0f;
    const float ry = (std::rand() / (float)RAND_MAX) * 2.0f - 1.0f;
    // whole pixels: matches the std::round(camX) the renderers do, so tiles don't shimmer
    m_shakeX = std::round((rx * s * 22.0f + m_kickX) * m_shakeScale);   // 22px max on the 1600-wide view
    m_shakeY = std::round((ry * s * 14.0f) * m_shakeScale);
}
