//
// Created by LENOVO on 06-08-2026.
//
#include <algorithm>
#include <cmath>
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
    m_tiles.clear();
    const float tilePx = (float)tileSize * scale;   // one tile on screen / in the world
    for(auto& block : m_blocks)
    {
        block.src =
                SpriteCollection::getSrcRect(toSpriteType(block.type), toSpriteVariant(block.variant),
                                             tileSize,scale);
        if(block.variant == BlockVariant::HORIZONTAL || block.variant == BlockVariant::VERTICAL)
            buildStrip(block, tilePx, tileSize);
        else
            m_tiles.push_back({block.rect, block.src});   // SINGLE / BLOCK: drawn as one piece, as before
    }
}

// HORIZONTAL: [left cap][middle][middle]...[right cap]     VERTICAL: [top cap][middle]...[bottom cap]
// Every piece is drawn at its native tile size (no stretching along the strip). Only the middle pieces repeat;
// the caps are pinned to the two ends. If the length is not a whole number of tiles, the last middle piece
// is clipped (source and destination together, so it is cut, not squashed).
void BlockBuilder::buildStrip(const Block& block, float tilePx, int tileSize) {
    const bool horizontal = block.variant == BlockVariant::HORIZONTAL;
    const SpriteType sprite = toSpriteType(block.type);
    const SpriteVariant variant = toSpriteVariant(block.variant);
    const float length = horizontal ? block.rect.w : block.rect.h;
    // block thickness (the other axis) is used as given in the level file - level files use 1 tile
    const float thickness = horizontal ? block.rect.h : block.rect.w;

    auto place = [&](float along, float alongLen, const SDL_FRect& src){
        SDL_FRect dst = horizontal
                        ? SDL_FRect{block.rect.x + along, block.rect.y, alongLen, thickness}
                        : SDL_FRect{block.rect.x, block.rect.y + along, thickness, alongLen};
        m_tiles.push_back({dst, src});
    };

    const int count = std::max(1, (int)std::ceil(length / tilePx - 0.001f));
    if(count == 1){
        // shorter than (about) one tile: the single-tile sprite, fitted to the rect
        m_tiles.push_back({block.rect, SpriteCollection::getSrcRect(sprite, SpriteVariant::SINGLE, (float)tileSize, 1.0f)});
        return;
    }

    place(0.0f, tilePx, SpriteCollection::getBlockPartSrc(sprite, variant, 0, (float)tileSize));

    const float lastCapStart = length - tilePx;     // last cap sits flush with the far end
    for(int i = 1; i < count - 1; i++){
        const float at = (float)i * tilePx;
        const float room = lastCapStart - at;
        if(room <= 0.0f) break;
        const float len = std::min(tilePx, room);
        SDL_FRect src = SpriteCollection::getBlockPartSrc(sprite, variant, 1, (float)tileSize);
        if(len < tilePx){                           // clip the source in proportion
            if(horizontal) src.w *= len / tilePx; else src.h *= len / tilePx;
        }
        place(at, len, src);
    }

    place(lastCapStart, tilePx, SpriteCollection::getBlockPartSrc(sprite, variant, 2, (float)tileSize));
}

void BlockBuilder::render(SDL_Renderer *renderer) {
    int camX = (int)std::round(Camera::getInstance().getCamera().x);
    int camY = (int)std::round(Camera::getInstance().getCamera().y);
    SDL_Texture* texture =Engine::Get().getAssetManager().getTexture(TextureType::TILESET_SPRITE);
    if(!texture)
        return;
    for(const auto& tile : m_tiles)
    {
        SDL_FRect dst{tile.dst.x-camX,tile.dst.y-camY,tile.dst.w,tile.dst.h};
        SDL_RenderTexture(renderer,texture,&tile.src,&dst);
    }
}
