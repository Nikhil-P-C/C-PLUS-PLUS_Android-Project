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
    void setPlayerSkin(PlayerSkin skin){
        m_playerSkin = skin;
    }
    std::string& getPlayerName(){
        return m_name;
    }
    void setPlayerName(const std::string& newName){
        m_name = newName;
    }
    static PlayerDetail& getInstance(){
        static PlayerDetail instance;
        return instance;

    }
private:

    std::string m_name = "Name";
    PlayerSkin m_playerSkin = PlayerSkin::YELLOW; //default:0(yellow) , 1(blue),2(red),3(green)
};