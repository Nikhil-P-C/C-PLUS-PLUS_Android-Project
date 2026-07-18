//
// Created by LENOVO on 15-07-2026.
//

#pragma once
#include <array>
#include <SDL3/SDL.h>
enum SpriteVariant{
    CENTER=0,
    TOP=1,
    BOTTOM=2,
    LEFT=3,
    RIGHT=4,
    TOP_LEFT=5,
    TOP_RIGHT=6,
    BOTTOM_LEFT=7,
    BOTTOM_RIGHT=8,
    NONE=9,
};
enum SpriteType{
    STONE_BRICK_WALL =0,
    WOODEN_SHAFT_WALL =1,
    MOSS_WALL =2,
    GREEN_GRASS_GROUND =3,
    ORANGE_GRASS_GROUND =4,
    PINK_GRASS_GROUND =5,
    BRICK_GROUND =6,
    CLAY_BLOCK =7,
    CLAY_BLOCK_HORIZONTAL =8,
    CLAY_BLOCK_VERTICAL =9,
    CLAY_BLOCK_SINGLE =10,
    STONE_BLOCK =11,
    STONE_BLOCK_HORIZONTAL =12,
    STONE_BLOCK_VERTICAL =13,
    STONE_BLOCK_SINGLE =14,
    WAX_BLOCK =15,
    WAX_BLOCK_HORIZONTAL =16,
    WAX_BLOCK_VERTICAL =17,
    WAX_BLOCK_SINGLE =18,
    GOLD_BLOCK =19,
    GOLD_BLOCK_HORIZONTAL =20,
    GOLD_BLOCK_VERTICAL =21,
    GOLD_BLOCK_SINGLE =22,
    GOLD_PLATFORM =23,
    WOODEN_PLATFORM =24,
    STONE_PLATFORM =25,
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

    static constexpr SpriteValue horizontalClayBlock{SpriteType::CLAY_BLOCK,
                                                     192.00f,0.00f,16.00f,48.00f};
    static constexpr SpriteValue verticalClayBlock{SpriteType::CLAY_BLOCK,
                                                   240.00f,0.00f,48.00f,16.00f};
    static constexpr SpriteValue singleClayBlock{SpriteType::CLAY_BLOCK,
                                                 192.00f,16.00f,16.00f,16.00f};
    static constexpr SpriteValue clayBlock{SpriteType::CLAY_BLOCK,
                                           208.00f,16.00f,32.00f,32.00f};

    static constexpr SpriteValue horizontalStoneBlock{SpriteType::STONE_BLOCK,
                                                      192.00f,64.00f,16.00f,48.00f};
    static constexpr SpriteValue verticalStoneBlock{SpriteType::STONE_BLOCK,
                                                    240.00f,64.00f,48.00f,16.00f};
    static constexpr SpriteValue singleStoneBlock{SpriteType::STONE_BLOCK,
                                                  192.00f,72.00f,16.00f,16.00f};
    static constexpr SpriteValue stoneBlock{SpriteType::STONE_BLOCK,
                                            208.00f,72.00f,32.00f,32.00f};

    static constexpr SpriteValue horizontalWaxBlock{SpriteType::WAX_BLOCK,
                                                    192.00f,128.00f,16.00f,48.00f};
    static constexpr SpriteValue verticalWaxBlock{SpriteType::WAX_BLOCK,
                                                  240.00f,128.00f,48.00f,16.00f};
    static constexpr SpriteValue singleWaxBlock{SpriteType::WAX_BLOCK,
                                                192.00f,128.00f,16.00f,16.00f};
    static constexpr SpriteValue waxBlock{SpriteType::WAX_BLOCK,
                                          208.00f,128.00f,32.00f,32.00f};

    static constexpr SpriteValue horizontalGoldBlock{SpriteType::GOLD_BLOCK,
                                                     288.00f,128.00f,16.00f,48.00f};
    static constexpr SpriteValue verticalGoldBlock{SpriteType::GOLD_BLOCK,
                                                   336.00f,128.00f,48.00f,16.00f};
    static constexpr SpriteValue singleGoldBlock{SpriteType::GOLD_BLOCK,
                                                 288.00f,128.00f,16.00f,16.00f};
    static constexpr SpriteValue goldBlock{SpriteType::GOLD_BLOCK,
                                           304.00f,128.00f,32.00f,32.00f};

    static constexpr SpriteValue goldPlatform{SpriteType::GOLD_PLATFORM,
                                              272.00f,0.0f,16.00f,48.00f};
    static constexpr SpriteValue woodenPlatform{SpriteType::WOODEN_PLATFORM,
                                                272.00f,16.00f,16.00f,48.00f};
    static constexpr SpriteValue stonePlatform{SpriteType::STONE_PLATFORM,
                                               272.00f,32.00f,16.00f,48.00f};
    static constexpr std::array<SpriteValue,26> spriteValueTable{
            stoneBrickWall,
            woodenShaftWall,
            mossWall,

            greenGrassGround,
            orangeGrassGround,
            pinkGrassGround,
            brickGround,

            clayBlock,
            horizontalClayBlock,
            verticalClayBlock,
            singleClayBlock,
            stoneBlock,
            horizontalStoneBlock,
             verticalStoneBlock,
            singleStoneBlock,
            waxBlock,
            horizontalWaxBlock,
            verticalWaxBlock,
            singleWaxBlock,
            goldBlock,
            horizontalGoldBlock,
            verticalGoldBlock,
            singleGoldBlock,
            goldPlatform,
            woodenPlatform,
            stonePlatform
    };
//    static std::unordered_map<SpriteType,SpriteValue> spriteValueTable={
//            {STONE_BRICK_WALL,stoneBrickWall},
//            {WOODEN_SHAFT_WALL,woodenShaftWall},
//            {MOSS_WALL,mossWall},
//
//            {GREEN_GRASS_GROUND,greenGrassGround},
//            {ORANGE_GRASS_GROUND,orangeGrassGround},
//            {PINK_GRASS_GROUND,pinkGrassGround},
//            {BRICK_GROUND,brickGround},
//
//            {CLAY_BLOCK,clayBlock},
//            {CLAY_BLOCK_HORIZONTAL,horizontalClayBlock},
//            {CLAY_BLOCK_VERTICAL,verticalClayBlock},
//            {CLAY_BLOCK_SINGLE,singleClayBlock},
//            {STONE_BLOCK,stoneBlock},
//            {STONE_BLOCK_HORIZONTAL,horizontalStoneBlock},
//            {STONE_BLOCK_VERTICAL, verticalStoneBlock},
//            {STONE_BLOCK_SINGLE, singleStoneBlock},
//            {WAX_BLOCK,waxBlock},
//            {WAX_BLOCK_HORIZONTAL,horizontalWaxBlock},
//            {WAX_BLOCK_VERTICAL,verticalWaxBlock},
//            {WAX_BLOCK_SINGLE,singleWaxBlock},
//            {GOLD_BLOCK,goldBlock},
//            {GOLD_BLOCK_HORIZONTAL,horizontalGoldBlock},
//            {GOLD_BLOCK_VERTICAL,verticalGoldBlock},
//            {GOLD_BLOCK_SINGLE,singleGoldBlock},
//            {GOLD_PLATFORM, goldPlatform},
//            {WOODEN_PLATFORM,woodenPlatform},
//            {STONE_PLATFORM, stonePlatform}
//    };
};
