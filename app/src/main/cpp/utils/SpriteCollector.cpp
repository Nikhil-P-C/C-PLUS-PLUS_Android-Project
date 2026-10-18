//
// Created by LENOVO on 18-07-2026.
//
#include "SpriteCollector.h"

SDL_FRect SpriteCollection::getSrcRect(SpriteType type, SpriteVariant variant,float tileSize, float scale) {
    const auto& value = spriteValueTable[static_cast<size_t>(type)];
    SDL_FRect src{value.x,value.y,value.width,value.height};
    if(isSpriteTypeGroundAndWall(type)){
        if(variant == SpriteVariant::TOP_LEFT){
            src = {value.x, value.y, tileSize, tileSize};
        }
        if(variant == SpriteVariant::TOP){
            src = {value.x+tileSize, value.y, tileSize, tileSize};
        }
        if(variant == SpriteVariant::TOP_RIGHT){
            src = {value.x+(tileSize*2), value.y, tileSize, tileSize};
        }
        if(variant == SpriteVariant::LEFT){
            src = {value.x, value.y+tileSize, tileSize, tileSize};
        }
        if(variant == SpriteVariant::CENTER){
            src = {value.x+tileSize, value.y+tileSize, tileSize, tileSize};
        }
        if(variant == SpriteVariant::RIGHT){
            src = {value.x+(tileSize*2) , value.y+tileSize, tileSize, tileSize};
        }
        if(variant == SpriteVariant::BOTTOM_LEFT){
            src = {value.x, value.y+(tileSize*2), tileSize, tileSize};
        }
        if(variant == SpriteVariant::BOTTOM){
            src = {value.x + tileSize , value.y+(tileSize*2), tileSize, tileSize};
        }
        if(variant == SpriteVariant::BOTTOM_RIGHT){
            src = {value.x+(tileSize*2) , value.y+(tileSize*2), tileSize, tileSize};
        }
    }
    if(isSpriteTypeBlock(type))
    {
        if(variant == SpriteVariant::SINGLE){
            src = {value.x,value.y+tileSize,tileSize,tileSize};
        }
        if(variant == SpriteVariant::VERTICAL){
            src = {value.x+(tileSize*3),value.y,tileSize,tileSize*3};
        }
        if(variant == SpriteVariant::HORIZONTAL){
            src = {value.x,value.y,tileSize*3,tileSize};
        }
        if(variant == SpriteVariant::BLOCK){
            src = {value.x+tileSize,value.y+tileSize,tileSize*2,tileSize*2};
        }
    }
    return src;
}

SDL_FRect SpriteCollection::getBlockPartSrc(SpriteType type, SpriteVariant variant, int part, float tileSize) {
    const auto& value = spriteValueTable[static_cast<size_t>(type)];
    if(part < 0) part = 0;
    if(part > 2) part = 2;
    if(variant == SpriteVariant::VERTICAL)
        return {value.x + tileSize * 3, value.y + tileSize * (float)part, tileSize, tileSize};
    // HORIZONTAL (and anything else): the 3-tile strip on the first row
    return {value.x + tileSize * (float)part, value.y, tileSize, tileSize};
}

bool SpriteCollection::isSpriteTypeGroundAndWall(SpriteType type) {
    if(type == SpriteType::GREEN_GRASS_GROUND)return true;
    if(type == SpriteType::BRICK_GROUND)return true;
    if(type == SpriteType::PINK_GRASS_GROUND)return true;
    if(type == SpriteType::ORANGE_GRASS_GROUND)return true;

    if(type == SpriteType::MOSS_WALL)return true;
    if(type == SpriteType::WOODEN_SHAFT_WALL)return true;
    if(type == SpriteType::STONE_BRICK_WALL)return true;

    return false;
}
bool SpriteCollection::isSpriteTypeBlock(SpriteType type){
    if(type == SpriteType::CLAY_BLOCK) return true;
    if(type == SpriteType::STONE_BLOCK) return true;
    if(type == SpriteType::WAX_BLOCK) return true;
    if(type == SpriteType::GOLD_BLOCK) return true;
    return false;
}

