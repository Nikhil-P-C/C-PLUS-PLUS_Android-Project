//
// Created by LENOVO on 15-07-2026.
//
#pragma once
enum SpriteVariant{
    CENTER=0,
    TOP=1,
    BOTTOM=2,
    LEFT=3,
    RIGHT=4,
    TOP_LEFT=5,
    TOP_RIGHT=6,
    BOTTOM_LEFT=7,
    BOTTOM_RIGHT=8
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
    STONE_BLOCK =8,
    WAX_BLOCK =9,
    GOLD_BLOCK =10,
    GOLD_PLATFORM =11,
    WOODEN_PLATFORM =12,
    STONE_PLATFORM =13,
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

    static constexpr SpriteValue stoneBrickWall{SpriteType::STONE_BRICK_WALL,
                                            0.0f,0.0f,48.00f,48.00f};
    static constexpr SpriteValue stoneBrickWallEdges{SpriteType::STONE_BRICK_WALL,
                                                 48.00,0.0f,32.00f,32.00f};

    static constexpr SpriteValue woodenShaftWall{SpriteType::WOODEN_SHAFT_WALL,
                                             0.00f,56.0f,48.00f,48.00f};
    static constexpr SpriteValue woodenShaftWallEdges{SpriteType::WOODEN_SHAFT_WALL,
                                                  48.00f,56.0f,32.00f,32.00f};

    static constexpr SpriteValue mossWall{SpriteType::MOSS_WALL,
                                           0.00f,128.0f,48.00f,48.00f};
    static constexpr SpriteValue mossWallEdges{SpriteType::MOSS_WALL,
                                                48.00f,128.0f,32.00f,32.00f};

    static constexpr SpriteValue greenGrassGround{SpriteType::GREEN_GRASS_GROUND,
                                                  96.00f,0.00f,48.00f,48.00f};
    static constexpr SpriteValue greenGrassGroundEdges{SpriteType::GREEN_GRASS_GROUND,
                                                       144.00f,0.00f,32.00f,32.00f};

    static constexpr SpriteValue orangeGrassGround{SpriteType::ORANGE_GRASS_GROUND,
                                                   96.00f,56.00f,48.00f,48.00f};
    static constexpr SpriteValue orangeGrassGroundEdges{SpriteType::ORANGE_GRASS_GROUND,
                                                        144.00f,56.00f,32.00f,32.00f};

    static constexpr SpriteValue pinkGrassGround{SpriteType::PINK_GRASS_GROUND,
                                                 96.00f,112.00f,48.00f,48.00f};
    static constexpr SpriteValue pinkGrassGroundEdges{SpriteType::PINK_GRASS_GROUND,
                                                      144.00f,112.00f,32.00f,32.00f};

    static constexpr SpriteValue brickGround{SpriteType::BRICK_GROUND,
                                             288.00f,56.00f,48.00f,48.00f};
    static constexpr SpriteValue brickGroundEdges{SpriteType::BRICK_GROUND,
                                                  336.00f,56.00f,32.00f,32.00f};

    static constexpr SpriteValue horizontalClayBlock{SpriteType::CLAY_BLOCK,
                                                     192.00f,0.00f,16.00f,48.00f};
    static constexpr SpriteValue verticalClayBlock{SpriteType::CLAY_BLOCK,
                                                   240.00f,0.00f,48.00f,16.00f};
    static constexpr SpriteValue singleClayBlock{SpriteType::CLAY_BLOCK,
                                                 192.00f,16.00f,16.00f,16.00f};
    static constexpr SpriteValue clayBlock{SpriteType::CLAY_BLOCK,
                                           208.00f,16.00f,32.00f,32.00f};

    static constexpr SpriteValue horizontalStoneBlock{SpriteType::STONE_BLOCK,
                                                      192.00f,56.00f,16.00f,48.00f};
    static constexpr SpriteValue verticalStoneBlock{SpriteType::STONE_BLOCK,
                                                    240.00f,56.00f,48.00f,16.00f};
    static constexpr SpriteValue singleStoneBlock{SpriteType::STONE_BLOCK,
                                                  192.00f,72.00f,16.00f,16.00f};
    static constexpr SpriteValue stoneBlock{SpriteType::STONE_BLOCK,
                                            208.00f,72.00f,32.00f,32.00f};

    static constexpr SpriteValue horizontalWaxBlock{SpriteType::WAX_BLOCK,
                                                    192.00f,112.00f,16.00f,48.00f};
    static constexpr SpriteValue verticalWaxBlock{SpriteType::WAX_BLOCK,
                                                  240.00f,112.00f,48.00f,16.00f};
    static constexpr SpriteValue singleWaxBlock{SpriteType::WAX_BLOCK,
                                                192.00f,128.00f,16.00f,16.00f};
    static constexpr SpriteValue waxBlock{SpriteType::WAX_BLOCK,
                                          208.00f,128.00f,32.00f,32.00f};

    static constexpr SpriteValue horizontalGoldBlock{SpriteType::GOLD_BLOCK,
                                                     288.00f,112.00f,16.00f,48.00f};
    static constexpr SpriteValue verticalGoldBlock{SpriteType::GOLD_BLOCK,
                                                   336.00f,112.00f,48.00f,16.00f};
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

};
