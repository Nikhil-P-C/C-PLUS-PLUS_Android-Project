//
// Created by LENOVO on 17-07-2026.
//
#pragma once
#include <vector>
#include <android/log.h>
#include "utils/utils.h"
#define LOG_TAG "WallShapeBuilder"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

struct LevelWall{
    float x;
    float y;
    float w;
    float h;
    SpriteType spriteType;
    ColliderType colliderType;
    LevelWall(float x,float y,float w,float h,SpriteType spriteType,ColliderType colliderType):
    x(x),y(y),w(w),h(h),spriteType(spriteType),colliderType(colliderType){}
};
struct WallTile{
    float x;
    float y;
    float w;
    float h;
    SpriteType spriteType;
    SpriteVariant variant;
    WallTile(float x,float y,float w,float h,SpriteType sprite,SpriteVariant variant):
    x(x),y(y),w(w),h(h),spriteType(sprite),variant(variant){
    }
};
enum class EdgeType{
    Top,
    Bottom,
    Left,
    Right
};
struct CollisionEdge{
    SDL_FRect rect;
    EdgeType type;
};
struct CollisionShape{
    std::vector<CollisionEdge> edges;
};
struct WallShape{
    std::vector<WallTile> tiles;
    CollisionShape collision;
};
class WallShapeBuilder {
public:
    WallShape build(const std::vector<LevelWall>& walls,int tileSize,int scale);
    bool tileExists(float x, float y);
    SpriteVariant getVariant(const WallTile &tile);
    WallShapeBuilder(){

    }
private:
    WallShape m_wallShape;


};
