//
// Created by LENOVO on 08-08-2026.
//
#pragma once

#ifdef NDEBUG

#define LOGI(...)
#define LOGE(...)

#elif defined(__ANDROID__)

#include <android/log.h>

#define LOG_TAG "LOGGER"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#else

#include <cstdio>

#define LOGI(...) std::printf("\n[INFO] " __VA_ARGS__ ,"\n")
#define LOGE(...) std::printf("\n[ERROR] " __VA_ARGS__ ,"\n")

#endif