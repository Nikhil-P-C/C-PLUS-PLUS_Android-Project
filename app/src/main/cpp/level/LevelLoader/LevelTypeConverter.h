//
// Created by LENOVO on 13-08-2026.
//

#pragma once
#include "utils/utils.h"
#include <unordered_map>
namespace {
    inline void logUnknownToken(const char* what, std::string_view token) {
        LOGE("could not find %s for: %.*s", what, (int)token.size(), token.data());
    }

    SpriteType toSpriteType(std::string_view stringSpriteType){
        static const std::unordered_map<std::string_view,SpriteType> map ={
                {"STONE_BRICK_WALL",SpriteType::STONE_BRICK_WALL},
                {"WOODEN_SHAFT_WALL",SpriteType::WOODEN_SHAFT_WALL},
                {"MOSS_WALL",SpriteType::MOSS_WALL},

                {"GREEN_GRASS_GROUND",SpriteType::GREEN_GRASS_GROUND},
                {"ORANGE_GRASS_GROUND",SpriteType::ORANGE_GRASS_GROUND},
                {"PINK_GRASS_GROUND",SpriteType::PINK_GRASS_GROUND},
                {"BRICK_GROUND",SpriteType::BRICK_GROUND},

                {"CLAY_BLOCK",SpriteType::CLAY_BLOCK},
                {"STONE_BLOCK",SpriteType::STONE_BLOCK},
                {"WAX_BLOCK",SpriteType::WAX_BLOCK},
                {"GOLD_BLOCK",SpriteType::GOLD_BLOCK},

                {"GOLD_PLATFORM",SpriteType::GOLD_PLATFORM},
                {"WOODEN_PLATFORM",SpriteType::WOODEN_PLATFORM},
                {"STONE_PLATFORM",SpriteType::STONE_PLATFORM},
        };
        const auto& it = map.find(stringSpriteType);
        if(it == map.end()){
            logUnknownToken("spriteType",stringSpriteType);
            return SpriteType::COUNT;
        }
        return it->second;
    }
    ColliderType toColliderType(std::string_view stringColliderType){
        static const std::unordered_map<std::string_view,ColliderType> map{
                {"SOLID", ColliderType::SOLID},
                {"ONE_WAY",ColliderType::ONE_WAY},
                {"TRIGGER",ColliderType::TRIGGER}
        };
        const auto& it = map.find(stringColliderType);
        if(it == map.end()){
            logUnknownToken("colliderType",stringColliderType);
            return ColliderType::NONE;
        }
        return it->second;
    }

    BlockType toBlockType(std::string_view stringBlockType){
        static const std::unordered_map<std::string_view ,BlockType> map{
                {"CLAY",BlockType::CLAY},
                {"STONE",BlockType::STONE},
                {"WAX",BlockType::WAX},
                {"GOLD",BlockType::GOLD}
        };
        const auto& it = map.find(stringBlockType);
        if(it == map.end()){
            logUnknownToken("blockType",stringBlockType);
            return BlockType::NONE;
        }
        return it->second;
    }

    BlockVariant toBlockVariant(std::string_view stringBlockVariant){
        static const std::unordered_map<std::string_view ,BlockVariant> map{
                {"SINGLE",BlockVariant::SINGLE},
                {"VERTICAL",BlockVariant::VERTICAL},
                {"HORIZONTAL",BlockVariant::HORIZONTAL},
                {"BLOCK",BlockVariant::BLOCK},
        };
        const auto& it = map.find(stringBlockVariant);
        if(it == map.end()){
            logUnknownToken("blockVariant",stringBlockVariant);
            return BlockVariant::NONE;
        }
        return it->second;
    }

    FruitType toFruitType(std::string_view stringFruitType){
        static const std::unordered_map<std::string_view ,FruitType> map{
                {"BANANA",FruitType::BANANA},
                {"APPLE",FruitType::APPLE},
                {"ORANGE",FruitType::ORANGE},
                {"STRAWBERRY",FruitType::STRAWBERRY},
        };
        const auto& it = map.find(stringFruitType);
        if(it == map.end()){
            logUnknownToken("fruitType",stringFruitType);
            return FruitType::NONE;
        }
        return it->second;
    }
    TrapType toTrapType(std::string_view stringTrapType){
        static const std::unordered_map<std::string_view ,TrapType> map{
                {"FALLING_PLATFORM",TrapType::FALLING_PLATFORM},
                {"FAN",TrapType::FAN},
                {"FIRE",TrapType::FIRE},
                {"MOVING_PLATFORM_BROWN",TrapType::MOVING_PLATFORM_BROWN},
                {"MOVING_PLATFORM_GREY",TrapType::MOVING_PLATFORM_GREY},
                {"ROCK_HEAD",TrapType::ROCK_HEAD},
                {"SAW",TrapType::SAW},
                {"SPIKE_HEAD",TrapType::SPIKE_HEAD},
                {"SPIKE_BALL",TrapType::SPIKE_BALL},
                {"SPIKES",TrapType::SPIKES},
                {"TRAMPOLINE",TrapType::TRAMPOLINE}
        };
        const auto& it = map.find(stringTrapType);
        if(it == map.end()){
            logUnknownToken("trapType",stringTrapType);
            return TrapType::NONE;
        }
        return it->second;
    }
    TrapStatus toTrapStatus(std::string_view stringTrapStatus){
        static const std::unordered_map<std::string_view ,TrapStatus> map{
                {"IDLE",TrapStatus::IDLE},
                {"HIT",TrapStatus::HIT},
                {"ON",TrapStatus::ON},
                {"OFF",TrapStatus::OFF},
                {"TRIGGERED",TrapStatus::TRIGGERED}
        };
        const auto& it = map.find(stringTrapStatus);
        if(it == map.end()){
            logUnknownToken("trapStatus",stringTrapStatus);
            return TrapStatus::NONE;
        }
        return it->second;
    }
    PathAxis toPathAxis(std::string_view stringPathAxis){
        static const std::unordered_map<std::string_view ,PathAxis> map{
                {"AUTO",PathAxis::AUTO},
                {"VERTICAL",PathAxis::VERTICAL},
                {"HORIZONTAL",PathAxis::HORIZONTAL},
                {"CIRCLE",PathAxis::CIRCLE}
        };
        const auto& it = map.find(stringPathAxis);
        if(it == map.end()){
            logUnknownToken("pathAxis",stringPathAxis);
            return PathAxis::NONE;
        }
        return it->second;
    }
    PathShape toPathShape(std::string_view stringPathShape){
        static const std::unordered_map<std::string_view ,PathShape> map{
                {"RECT",PathShape::RECT},
                {"LINE",PathShape::LINE},
                {"ARC",PathShape::ARC},
                {"CIRCLE",PathShape::CIRCLE}
        };
        const auto& it = map.find(stringPathShape);
        if(it == map.end()){
            logUnknownToken("pathShape",stringPathShape);
            return PathShape::NONE;
        }
        return it->second;
    }
    CheckPointAni toCheckPointAniType(std::string_view stringCheckPointAniType){
        static const std::unordered_map<std::string_view ,CheckPointAni> map{
                {"NO_FLAG",CheckPointAni::NO_FLAG},
                {"FLAG_OUT",CheckPointAni::FLAG_OUT},
                {"FLAG_IDLE",CheckPointAni::FLAG_IDLE},
        };
        const auto& it = map.find(stringCheckPointAniType);
        if(it == map.end()){
            logUnknownToken("checkPointAniType",stringCheckPointAniType);
            return CheckPointAni::NONE;
        }
        return it->second;
    }

}