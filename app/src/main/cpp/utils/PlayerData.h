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
        return playerSkin;
    }
    void setPlayerSkin(PlayerSkin skin){
        playerSkin = skin;
    }
    std::string_view getPlayerName(){
        return name;
    }
//    void setPlayerName(const char* n){
//        strcpy(name,n);
//    }
    static PlayerDetail& getInstance(){
        static PlayerDetail instance;
        return instance;

    }
private:
    std::string name="";
    PlayerSkin playerSkin = YELLOW; //default:0(yellow) , 1(blue),2(red),3(green)
};