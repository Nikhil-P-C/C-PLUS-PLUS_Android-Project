//
// Created by LENOVO on 15-07-2026.
//

#pragma once
#include <array>
#include <SDL3/SDL.h>
enum class SpriteVariant{
//for ground and wall
    CENTER=0,
    TOP,
    BOTTOM,
    LEFT,
    RIGHT,
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT,
//for blocks
    VERTICAL,
    HORIZONTAL,
    SINGLE,
    BLOCK,
    NONE,
};
enum class SpriteType{
    STONE_BRICK_WALL =0,
    WOODEN_SHAFT_WALL,
    MOSS_WALL,

    GREEN_GRASS_GROUND ,
    ORANGE_GRASS_GROUND,
    PINK_GRASS_GROUND,
    BRICK_GROUND,

    CLAY_BLOCK,
    STONE_BLOCK,
    WAX_BLOCK,
    GOLD_BLOCK,

    GOLD_PLATFORM,
    WOODEN_PLATFORM,
    STONE_PLATFORM,
    COUNT
};
struct Animation{
    int startIndex=0;
    int lastIndex=0;
};

struct SpriteValue{
    SpriteType type;
    float x=0.0f;
    float y=0.0f;
    float height =0.0f;
    float width =0.0f;
    constexpr SpriteValue(SpriteType t,float x,float y,float w,float h):
                          type(t), x(x), y(y), width(w), height(h){}
    SpriteValue() =default;
};
struct SpriteCollection{
public:
    static SDL_FRect getSrcRect(SpriteType type, SpriteVariant variant, float tileSize, float scale);
private:
    static bool isSpriteTypeGroundAndWall(SpriteType type);
    static bool isSpriteTypeBlock(SpriteType type);
public:
    static constexpr SpriteValue stoneBrickWall{SpriteType::STONE_BRICK_WALL,
                                            0.0f,0.0f,48.00f,48.00f};
    static constexpr SpriteValue stoneBrickWallEdges{SpriteType::STONE_BRICK_WALL,
                                                 48.00,0.0f,32.00f,32.00f};

    static constexpr SpriteValue woodenShaftWall{SpriteType::WOODEN_SHAFT_WALL,
                                             0.00f,64.0f,48.00f,48.00f};
    static constexpr SpriteValue woodenShaftWallEdges{SpriteType::WOODEN_SHAFT_WALL,
                                                  48.00f,64.0f,32.00f,32.00f};

    static constexpr SpriteValue mossWall{SpriteType::MOSS_WALL,
                                           0.00f,128.0f,48.00f,48.00f};
    static constexpr SpriteValue mossWallEdges{SpriteType::MOSS_WALL,
                                                48.00f,128.0f,32.00f,32.00f};

    static constexpr SpriteValue greenGrassGround{SpriteType::GREEN_GRASS_GROUND,
                                                  96.00f,0.00f,48.00f,48.00f};
    static constexpr SpriteValue greenGrassGroundEdges{SpriteType::GREEN_GRASS_GROUND,
                                                       144.00f,0.00f,32.00f,32.00f};

    static constexpr SpriteValue orangeGrassGround{SpriteType::ORANGE_GRASS_GROUND,
                                                   96.00f,64.00f,48.00f,48.00f};
    static constexpr SpriteValue orangeGrassGroundEdges{SpriteType::ORANGE_GRASS_GROUND,
                                                        144.00f,64.00f,32.00f,32.00f};

    static constexpr SpriteValue pinkGrassGround{SpriteType::PINK_GRASS_GROUND,
                                                 96.00f,128.00f,48.00f,48.00f};
    static constexpr SpriteValue pinkGrassGroundEdges{SpriteType::PINK_GRASS_GROUND,
                                                      144.00f,128.00f,32.00f,32.00f};

    static constexpr SpriteValue brickGround{SpriteType::BRICK_GROUND,
                                             288.00f,64.00f,48.00f,48.00f};
    static constexpr SpriteValue brickGroundEdges{SpriteType::BRICK_GROUND,
                                                  336.00f,64.00f,32.00f,32.00f};

    static constexpr SpriteValue clayBlock{SpriteType::CLAY_BLOCK,
                                           192.00f,0.00f,64.00f,48.00f};

    static constexpr SpriteValue stoneBlock{SpriteType::STONE_BLOCK,
                                            192.00f,64.00f,64.00f,48.00f};

    static constexpr SpriteValue waxBlock{SpriteType::WAX_BLOCK,
                                          192.00f,128.00f,64.00f,48.00f};


    static constexpr SpriteValue goldBlock{SpriteType::GOLD_BLOCK,
                                           272.00f,128.00f,64.00f,48.00f};

    static constexpr SpriteValue goldPlatform{SpriteType::GOLD_PLATFORM,
                                              272.00f,0.0f,16.00f,48.00f};
    static constexpr SpriteValue woodenPlatform{SpriteType::WOODEN_PLATFORM,
                                                272.00f,16.00f,16.00f,48.00f};
    static constexpr SpriteValue stonePlatform{SpriteType::STONE_PLATFORM,
                                               272.00f,32.00f,16.00f,48.00f};
    static constexpr std::array<SpriteValue,(unsigned int)SpriteType::COUNT> spriteValueTable{
            stoneBrickWall,
            woodenShaftWall,
            mossWall,

            greenGrassGround,
            orangeGrassGround,
            pinkGrassGround,
            brickGround,

            clayBlock,
            stoneBlock,
            waxBlock,
            goldBlock,

            goldPlatform,
            woodenPlatform,
            stonePlatform,
    };
};
