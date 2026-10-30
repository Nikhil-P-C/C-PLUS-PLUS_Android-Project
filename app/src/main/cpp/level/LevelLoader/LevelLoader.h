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
#include "level/BackGroundBuilder.h"
#include "level/ForeGroundBuilder.h"
#include "level/Lights.h"
#include "level/Enemies.h"
#include "level/Bosses.h"
#include "level/Interactables.h"
#include <sstream>
#include <filesystem>


#define LOG_TAG "LevelLoader"

// highest level_N.txt that exists; TransitionState wraps back to level 0 after this one
constexpr int LAST_LEVEL = 23;

// Play order = plain numerical order (level_0, level_1, ... level_LAST_LEVEL). Boss levels are NOT interleaved:
// 0-10 are the normal levels, 11-23 are the 13 boss test levels:
//   11/12 Moss mini+main, 13/14 Stone, 15/16 Shaft, 17/18 Plains, 19/20 Woods, 21/22 Mountain, 23 Peak (the Preacher)
// the level that follows `level` (wraps to level 0 after LAST_LEVEL)
inline int nextLevelAfter(int level){
    return (level + 1 > LAST_LEVEL) ? 0 : level + 1;
}

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
    const std::vector<BackGroundElement>& getBackGroundElements(){
        return m_backGroundElements;
    }
    const std::vector<ForeGroundElement>& getForeGroundElements(){
        return m_foreGroundElements;
    }
    const std::vector<Interactable>& getInteractables(){
        return m_interactables;
    }
    const std::vector<Enemy>& getEnemies(){
        return m_enemies;
    }
    // boss fights: 'K' records (see parseBoss). Boss levels also have an 'A' arena rect (see parseBossArena).
    const std::vector<Boss>& getBosses() const { return m_bosses; }
    CheckPoint getCheckPoint(){
        return m_checkPoint;
    }
    SDL_FRect getWallCollisionRect(){
        return m_wallCollisionRect;
    }
    const std::vector<LightSource>& getLights(){
        return m_lights;
    }
    const LevelAmbient& getAmbient(){
        return m_ambient;
    }
    // 'H' records: rooms hidden behind darkness until the player enters (caves / closed buildings)
    const std::vector<DarkRoom>& getDarkRooms() const { return m_darkRooms; }
    // 'Z' record: ambience/weather overrides (upper-case tokens, see AmbienceFx.h). Empty = biome defaults.
    const std::vector<std::string>& getWeatherTokens() const { return m_weatherTokens; }
    // level music: from the level's 'M' record, else Sound/Music/level_<n>.ogg. Empty = silent level.
    const std::string& getMusicPath() const { return m_musicPath; }
    float getMusicGain() const { return m_musicGain; }
    // true if the level file has an 'S' line (otherwise getPlayer() is just 0,0)
    bool hasSpawn() const { return m_hasSpawn; }
private:
    void parseAmbient(std::istringstream& ss);

    void parseWeather(std::istringstream& ss);

    void parseDarkRoom(std::istringstream& ss);

    void parseMusic(std::istringstream& ss);

    void parseLight(std::istringstream& ss);

    void parseLine(const std::string& line);

    void parseSpawn(std::istringstream& ss);

    void parseWalls(std::istringstream& ss);

    void parsePlatforms(std::istringstream& ss);

    void parseGrounds(std::istringstream& ss);

    void parseBlocks(std::istringstream& ss);

    void parseFruits(std::istringstream& ss);

    void parseBackGround(std::istringstream& ss);

    void parseForeGround(std::istringstream& ss);

    void parseEnemy(std::istringstream& ss);
    void parseInteractable(std::istringstream& ss);
    void parseBoss(std::istringstream& ss);
    void parseBossArena(std::istringstream& ss);

    void parseTraps(std::istringstream& ss);//some traps have additional param trailing at end

    void parseSawTail(std::istringstream& ss,Trap& trap);

    void parseSpikeBallTail(std::istringstream& ss,Trap& trap);

    void parseGenericTail(std::istringstream& ss,Trap& trap);

    void parseCheckPoint(std::istringstream& ss);

    void parseWallCollisionRect(std::istringstream &ss);

    std::string loadFromFile(const std::string& filepath);

private:
    Player m_player; // use the player rect to store spawn for now; wasteful find a workaround
    std::vector<Platform> m_platforms;
    std::vector<LevelGround> m_grounds;
    std::vector<LevelGround> m_levelWalls;
    std::vector<Fruit> m_fruits;
    std::vector<Trap> m_traps;
    std::vector<Block> m_blocks;
    std::vector<Enemy> m_enemies;
    std::vector<Interactable> m_interactables;
    std::vector<Boss> m_bosses;
    bool m_hasBossArena = false;
    SDL_FRect m_bossArena{0.0f,0.0f,0.0f,0.0f};
    std::vector<BackGroundElement> m_backGroundElements;
    std::vector<ForeGroundElement> m_foreGroundElements;
    CheckPoint m_checkPoint;
    SDL_FRect m_wallCollisionRect;
    std::vector<LightSource> m_lights;
    std::vector<DarkRoom> m_darkRooms;
    LevelAmbient m_ambient;
    std::vector<std::string> m_weatherTokens;   // 'Z' line
    bool m_hasSpawn = false;
    std::string m_musicPath;
    float m_musicGain = 1.0f;
    bool m_musicExplicit = false;

};