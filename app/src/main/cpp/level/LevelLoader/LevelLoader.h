//
// Created by LENOVO on 06-08-2026.
//
#pragma once
#include <vector>
#include "level/Traps.h"
#include "level/Fruits.h"
#include "level/BlockShapeBuilder.h"
#include "level/CheckPoint.h"
#include "utils/utils.h"
#include <sstream>
#include <filesystem>


#define LOG_TAG "LevelLoader"

class LevelLoader{
public:

    LevelLoader();
    ~LevelLoader();
    // load level clears previous level objects , and loads new ones
    void loadLevel(int level);
    Player getPlayer(){
        return m_player;
    }
    const std::vector<Platform>& getPlatforms(){
        return m_platforms;
    }
    const std::vector<LevelGround>& getLevelWalls(){
        return m_levelWalls;
    }
    const std::vector<LevelGround>& getLevelGrounds(){
        return m_grounds;
    }
    const std::vector<Fruit>& getFruits(){
        return m_fruits;
    }
    const std::vector<Trap>& getTraps(){
        return m_traps;
    }
    const std::vector<Block>& getBlocks(){
        return m_blocks;
    }
    CheckPoint getCheckPoint(){
        return m_checkPoint;
    }
private:
    void parseLine(const std::string& line);

    void parseSpawn(std::istringstream& ss);

    void parseWalls(std::istringstream& ss);

    void parsePlatforms(std::istringstream& ss);

    void parseGrounds(std::istringstream& ss);

    void parseBlocks(std::istringstream& ss);

    void parseFruits(std::istringstream& ss);

    void parseTraps(std::istringstream& ss);//some traps have additional param trailing at end

    void parseSawTail(std::istringstream& ss,Trap& trap);

    void parseSpikeBallTail(std::istringstream& ss,Trap& trap);

    void parseGenericTail(std::istringstream& ss,Trap& trap);

    void parseCheckPoint(std::istringstream& ss);

    std::string loadFromFile(const std::string& filepath);

private:
    Player m_player; // use the player rect to store spawn for now; wasteful find a workaround
    std::vector<Platform> m_platforms;
    std::vector<LevelGround> m_grounds;
    std::vector<LevelGround> m_levelWalls;
    std::vector<Fruit> m_fruits;
    std::vector<Trap> m_traps;
    std::vector<Block> m_blocks;
    CheckPoint m_checkPoint;
};