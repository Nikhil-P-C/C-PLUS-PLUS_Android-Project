//
// Created by LENOVO on 17-07-2026.
//

#include "WallShapeBuilder.h"



WallShape WallShapeBuilder::build(const std::vector<LevelWall> &walls, int tileSize, int scale) {
    for(const auto& wall : walls){
        const int platformWidth = (int)wall.w;
        const int platformHeight = (int)wall.h;
        int widthTiles = platformWidth / tileSize;
        int heightTiles = platformHeight / tileSize;
        for(int i = 0; i< heightTiles; i++){
            for(int j = 0; j< widthTiles;j++){

                float x =wall.x + j * (tileSize * scale);
                float y =wall.y + i* (tileSize * scale);
                float w =tileSize*scale;
                float h =tileSize*scale;
                SpriteVariant variant;
//                bool left = (j == 0);
//                bool right = (j == widthTiles - 1);
//                bool top = (i == 0);
//                bool bottom = (i == heightTiles - 1);
//                if (top) {
//                    if (left) variant = SpriteVariant::TOP_LEFT;
//                    else if (right) variant =SpriteVariant::TOP_RIGHT;
//                    else variant =SpriteVariant::TOP;
//                }
//                else if (bottom) {
//                    if (left) variant = SpriteVariant::BOTTOM_LEFT;
//                    else if (right) variant =SpriteVariant::BOTTOM_RIGHT;
//                    else variant = SpriteVariant::BOTTOM;
//                }
//                else {
//                    if (left) variant = SpriteVariant::LEFT;
//                    else if (right) variant =SpriteVariant::RIGHT;
//                    else variant =SpriteVariant::CENTER;
//                }

                m_wallShape.tiles.emplace_back(x,y,w,h,wall.spriteType,variant);
            }

        }
    }
    for (auto& tile : m_wallShape.tiles)
    {
        tile.variant = getVariant(tile);
    }

    return m_wallShape;
}

bool WallShapeBuilder::tileExists(float x, float y) {

    constexpr float epsilon = 0.01f;

    for(const auto& tile : m_wallShape.tiles)
    {
        if(std::abs(tile.x - x) < epsilon &&
           std::abs(tile.y - y) < epsilon)
        {
            return true;
        }
    }

    return false;
}

SpriteVariant WallShapeBuilder::getVariant(const WallTile & tile){

    SpriteVariant variant;
    bool hasTop    = tileExists(tile.x, tile.y -tile.h);
    bool hasBottom = tileExists(tile.x, tile.y + tile.h);
    bool hasLeft   = tileExists(tile.x - tile.w, tile.y);
    bool hasRight  = tileExists(tile.x + tile.w, tile.y);

    // Decide SpriteVariant
    if(!hasTop && hasBottom && hasLeft && !hasRight) variant = SpriteVariant::TOP_RIGHT;
    else if(!hasTop && hasBottom && hasLeft && hasRight)variant=SpriteVariant::TOP;
    else if(!hasTop && hasBottom && !hasLeft && hasRight)variant = SpriteVariant::TOP_LEFT;

    else if(hasTop && hasBottom && !hasLeft && hasRight) variant =SpriteVariant::LEFT;
    else if(hasTop && hasBottom && hasLeft && !hasRight) variant =SpriteVariant::RIGHT;
    else if(!hasTop && !hasBottom && !hasLeft && !hasRight) variant =SpriteVariant::CENTER;

    else if(hasTop && !hasBottom && !hasLeft && hasRight) variant = SpriteVariant::BOTTOM_LEFT;
    else if(hasTop && !hasBottom && hasLeft && hasRight) variant = SpriteVariant::BOTTOM;
    else if(hasTop && !hasBottom && hasLeft && !hasRight) variant = SpriteVariant::BOTTOM_RIGHT;
    else
    {
        LOGI("Unhandled wall tile neighbor combination");
        variant = SpriteVariant::CENTER;
    }
    return variant;
}