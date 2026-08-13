//
// Created by LENOVO on 06-08-2026.
//
#pragma once
#include "SDL3/SDL.h"
#include "utils/utils.h"

enum class BlockType{
    CLAY =0,
    STONE,
    WAX,
    GOLD,
    NONE
};
enum class BlockVariant{
    SINGLE=0,
    VERTICAL,
    HORIZONTAL,
    BLOCK,
    NONE,
};
struct Block{
    Block(float x,float y,float w,float h,BlockVariant variant,BlockType type):
    variant(variant),type(type){
        rect.x = x;
        rect.y = y;
        rect.w = w;
        rect.h = h;
    }
    SDL_FRect rect{0.00f};
    SDL_FRect src{0.0};
    BlockVariant variant=BlockVariant::BLOCK;
    BlockType type=BlockType::NONE;
};
class BlockBuilder{
public:
    void render(SDL_Renderer* renderer);

    void init(std::vector<Block>& blocks,int tileSize,float scale);


private:
    std::vector<Block> m_blocks;
};
