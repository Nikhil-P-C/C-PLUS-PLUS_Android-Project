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
                SDL_FRect src;
                m_wallShape.tiles.emplace_back(x,y,w,h,wall.spriteType,variant,src);
            }

        }
    }
    for (auto& tile : m_wallShape.tiles)
    {
        tile.variant = getVariant(tile,walls);
        SDL_FRect src = SpriteCollection::getSrcRect(tile.spriteType,tile.variant,tileSize,scale);
        tile.src =src;
    }

    return m_wallShape;
}

bool WallShapeBuilder::tileExists(float x, float y,const std::vector<LevelWall>& walls) {

    constexpr float epsilon = 0.01f;

    for(const auto& wall : walls)
    {
        if(x >= wall.x &&
           x < wall.x + wall.w &&
           y >= wall.y &&
           y < wall.y + wall.h)
        {
            return true;
        }
    }

    return false;
}

SpriteVariant WallShapeBuilder::getVariant(const WallTile & tile,const std::vector<LevelWall>& walls){

    SpriteVariant variant;
    bool hasTop    = hasWallAbove(tile.x,tile.y,walls);
    bool hasBottom = hasWallBelow(tile.x,tile.y,walls);
    bool hasLeft   = hasWallLeft(tile.x,tile.y,walls);
    bool hasRight  = hasWallRight(tile.x,tile.y,walls);

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
        // Completely surrounded
    else if(hasTop && hasBottom && hasLeft && hasRight)variant = SpriteVariant::CENTER;
    else if(hasTop && hasBottom && !hasLeft && !hasRight)variant = SpriteVariant::CENTER;
    else if(!hasTop && !hasBottom && hasLeft && hasRight)variant = SpriteVariant::CENTER;
    else if(hasTop && !hasBottom && !hasLeft && !hasRight)variant = SpriteVariant::CENTER;
    else if(!hasTop && hasBottom && !hasLeft && !hasRight)variant = SpriteVariant::CENTER;
    else if(!hasTop && !hasBottom && hasLeft && !hasRight)variant = SpriteVariant::CENTER;
    else if(!hasTop && !hasBottom && !hasLeft && hasRight)variant = SpriteVariant::CENTER;
    else
    {
        LOGI("Unhandled wall tile neighbor combination");
        variant = SpriteVariant::CENTER;
    }
    if(hasTop)variant =SpriteVariant::NONE;
    if(hasBottom)variant =SpriteVariant::NONE;
    return variant;
}

bool WallShapeBuilder::hasWallAbove(float x, float y,const std::vector<LevelWall>& walls) {
    float checkX = x;
    float checkY = y - (16 * 4);
    for(const auto& wall : walls){
        if(checkX >= wall.x&& checkX < wall.x + wall.w &&
           checkY >=wall.y&& checkY<wall.y+wall.h)
            return true;
    }
    return false;
}

bool WallShapeBuilder::hasWallBelow(float x, float y,const std::vector<LevelWall>& walls) {
    float checkX = x;
    float checkY = y + (16 * 4);
    for(const auto& wall : walls){
        if(checkX >= wall.x&& checkX < wall.x + wall.w &&
           checkY >=wall.y&& checkY<wall.y+wall.h)
            return true;
    }
    return false;
}

bool WallShapeBuilder::hasWallRight(float x, float y,const std::vector<LevelWall>& walls) {
    float checkX = x+ (16 * 4);
    float checkY = y;
    for(const auto& wall : walls){
        if(checkX >= wall.x&& checkX < wall.x + wall.w &&
           checkY >=wall.y&& checkY<wall.y+wall.h)
            return true;
    }
    return false;
}

bool WallShapeBuilder::hasWallLeft(float x, float y,const std::vector<LevelWall>& walls) {
    float checkX = x- (16 * 4);
    float checkY = y;
    for(const auto& wall : walls){
        if(checkX >= wall.x&& checkX < wall.x + wall.w &&
           checkY >=wall.y&& checkY<wall.y+wall.h)
            return true;
    }
    return false;
}
