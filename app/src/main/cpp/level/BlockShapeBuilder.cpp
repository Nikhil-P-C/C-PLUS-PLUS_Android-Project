//
// Created by LENOVO on 06-08-2026.
//
#include "BlockShapeBuilder.h"
#include "engine/Engine.h"
namespace {
    inline SpriteType toSpriteType(BlockType type){
        if(type == BlockType::CLAY)
            return SpriteType::CLAY_BLOCK;
        else if(type == BlockType::STONE)
            return SpriteType::STONE_BLOCK;
        else if(type == BlockType::WAX)
            return SpriteType::WAX_BLOCK;
        else
            return SpriteType::GOLD_BLOCK;
    }
    inline SpriteVariant toSpriteVariant(BlockVariant variant){
        if(variant == BlockVariant::SINGLE)
            return SpriteVariant::SINGLE;
        else if(variant == BlockVariant::VERTICAL)
            return SpriteVariant::VERTICAL;
        else if(variant == BlockVariant::HORIZONTAL)
            return SpriteVariant::HORIZONTAL;
        else
            return SpriteVariant::BLOCK;
    }
}
void BlockBuilder::init(std::vector<Block>& blocks,int tileSize,float scale) {
    m_blocks =blocks;
    for(auto& block : m_blocks)
    {
        block.src =
                SpriteCollection::getSrcRect(toSpriteType(block.type), toSpriteVariant(block.variant),
                                             tileSize,scale);
    }
}

void BlockBuilder::render(SDL_Renderer *renderer) {
    int camX = (int)std::round(Camera::getInstance().getCamera().x);
    int camY = (int)std::round(Camera::getInstance().getCamera().y);
    for(const auto& block : m_blocks)
    {
        SDL_Texture* texture =Engine::Get().getAssetManager().getTexture(TextureType::TILESET_SPRITE);
        if(!texture)
            continue;
        SDL_FRect dst{block.rect.x-camX,block.rect.y-camY,block.rect.w,block.rect.h};
        SDL_RenderTexture(renderer,texture,&block.src,&dst);
    }
}
