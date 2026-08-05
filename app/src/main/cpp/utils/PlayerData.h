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
public:
    PlayerSkin getPlayerSkin(){
        return m_playerSkin;
    }
    std::string& getPlayerName(){
        return m_name;
    }
    int getScore() const{
        return m_fruitCollectionScore;
    }
    int getPlayerHP() const{
        return m_hp;
    }
    unsigned int getLastHitTime() const{
        return m_lastHit;
    }
    bool isInvincible() const{
        return m_isInvincible;
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
    void setLastHitTime(unsigned int newTime){
        m_lastHit=newTime;
    }
    void addPlayerHP(int factorRaise){
        m_hp += factorRaise;
    }
    void subPlayerHP(int factorDeduct){
        m_hp -= factorDeduct;
    }
    void setPlayerHP(int value){
        m_hp = value;
    }
    void setInvincibility(bool invincibility){
        m_isInvincible = invincibility;
    }
    static PlayerDetail& getInstance(){
        static PlayerDetail instance;
        return instance;
    }
private:
    int m_fruitCollectionScore = 0;
    unsigned int m_lastHit =0;
    int m_hp =5;
    bool m_isInvincible=false;
    std::string m_name = "Name";
    PlayerSkin m_playerSkin = PlayerSkin::YELLOW; //default:0(yellow) , 1(blue),2(red),3(green)
};