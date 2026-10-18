//
// Created by LENOVO on 06-08-2026.
//
#pragma once
#include "SDL3/SDL.h"
#include "utils/utils.h"
#include <vector>

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
// One drawn piece of a block: where it goes in the world and which part of the tileset it samples.
struct BlockTile{
    SDL_FRect dst;
    SDL_FRect src;
};
class BlockBuilder{
public:
    void render(SDL_Renderer* renderer);

    void init(std::vector<Block>& blocks,int tileSize,float scale);


private:
    // HORIZONTAL / VERTICAL blocks are auto-tiled (caps + repeated middle) instead of stretched
    void buildStrip(const Block& block,float tilePx,int tileSize);

    std::vector<Block> m_blocks;
    std::vector<BlockTile> m_tiles;
};
