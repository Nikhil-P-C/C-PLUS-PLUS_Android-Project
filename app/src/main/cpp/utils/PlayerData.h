//
// Created by LENOVO on 05-05-2026.
//
#pragma once

#include <string>

enum PlayerSkin{
    YELLOW=0,
    BLUE=1,
    RED=2,
    GREEN=3
};
class PlayerDetail{
private:
    PlayerDetail() = default;
    PlayerDetail(const PlayerDetail&) = delete;
    PlayerDetail operator=(const PlayerDetail&) = delete;
public:
    PlayerSkin getPlayerSkin(){
        return m_playerSkin;
    }
    std::string& getPlayerName(){
        return m_name;
    }
    int getScore(){
        return m_fruitCollectionScore;
    }
    void setPlayerSkin(PlayerSkin skin){
        m_playerSkin = skin;
    }

    void setPlayerName(const std::string& newName){
        m_name = newName;
    }
    void addScore(int score){
        m_fruitCollectionScore += score;
    }
    void setScore(int score){
        m_fruitCollectionScore = score;
    }
    static PlayerDetail& getInstance(){
        static PlayerDetail instance;
        return instance;
    }
private:
    int m_fruitCollectionScore = 0;
    std::string m_name = "Name";
    PlayerSkin m_playerSkin = PlayerSkin::YELLOW; //default:0(yellow) , 1(blue),2(red),3(green)
};