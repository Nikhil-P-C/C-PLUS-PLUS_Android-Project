//
// Created by LENOVO on 17-07-2026.
//
#pragma once
#include <vector>
#include <android/log.h>
#include "utils/utils.h"
#define LOG_TAG "GroundShapeBuilder"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

struct LevelGround{
    float x;
    float y;
    float w;
    float h;
    SpriteType spriteType;
    ColliderType colliderType;
    LevelGround(float x, float y, float w, float h, SpriteType spriteType, ColliderType colliderType):
    x(x),y(y),w(w),h(h),spriteType(spriteType),colliderType(colliderType){}
};
struct GroundTile{
    float x;
    float y;
    float w;
    float h;
    SpriteType spriteType;
    SpriteVariant variant =SpriteVariant::NONE;
    SDL_FRect src;
    GroundTile(float x, float y, float w, float h, SpriteType sprite, SpriteVariant variant, SDL_FRect src):
    x(x),y(y),w(w),h(h),spriteType(sprite),variant(variant),src(src){
    }
};
enum class EdgeType{
    TOP,
    BOTTOM,
    LEFT,
    RIGHT
};
struct CollisionEdge{
    SDL_FRect rect;
    EdgeType type;
};
struct CollisionShape{
    std::vector<CollisionEdge> edges;
};
struct GroundShape{
    std::vector<GroundTile> tiles;
    CollisionShape collision;
};
class GroundShapeBuilder {
public:
    bool hasWallAbove(float x,float y,const std::vector<LevelGround>& grounds,int tileSize,int scale);
    bool hasWallBelow(float x,float y,const std::vector<LevelGround>& grounds,int tileSize,int scale);
    bool hasWallRight(float x,float y,const std::vector<LevelGround>& grounds,int tileSize,int scale);
    bool hasWallLeft(float x,float y,const std::vector<LevelGround>& grounds,int tileSize,int scale);
    GroundShape build(const std::vector<LevelGround>& walls, int tileSize, int scale);
    bool tileExists(float x, float y,const std::vector<LevelGround>& grounds);
    SpriteVariant getVariant(const GroundTile &tile, const std::vector<LevelGround>& grounds);
    GroundShapeBuilder()= default;
private:
    GroundShape m_groundShape;


};
