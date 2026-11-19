//
// Created by LENOVO on 05-05-2026.
//
#pragma once

#include <string>
#include <array>
#include <algorithm>
#include "utils/Upgrades.h"

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
    float getMagic() const{
        return m_magic;
    }
    float getMaxMagic() const{
        return m_maxMagic;
    }
    unsigned int getLastHitTime() const{
        return m_lastHit;
    }
    bool isInvincible() const{
        return m_isInvincible;
    }
    // ---- fruit = the merchant's currency -------------------------------------------------------------------------------
    // (getScore() also counts kill score, so the wallet is separate: it goes UP when fruit is picked up and DOWN in the shop)
    int  getFruit() const{ return m_fruit; }
    void addFruit(int amount){ m_fruit += amount; }
    bool spendFruit(int amount){
        if(amount < 0 || m_fruit < amount) return false;
        m_fruit -= amount;
        return true;
    }

    // ---- merchant upgrades (tables in utils/Upgrades.h) -------------------------------------------------------------------
    int  getUpgradeLevel(Upgrades::Kind kind) const{ return m_upgrade[kind]; }
    bool isUpgradeMaxed(Upgrades::Kind kind) const{ return m_upgrade[kind] >= Upgrades::MAX_LEVEL[kind]; }
    // fruit price of the next level, -1 when the upgrade is already maxed
    int  nextUpgradeCost(Upgrades::Kind kind) const{
        return isUpgradeMaxed(kind) ? -1 : Upgrades::COST[kind][m_upgrade[kind]];
    }
    bool canBuyUpgrade(Upgrades::Kind kind) const{
        return !isUpgradeMaxed(kind) && m_fruit >= nextUpgradeCost(kind);
    }
    // pays and raises the level. A new heart also arrives full (+1 hp), like a fresh container. false if it cannot be bought.
    bool buyUpgrade(Upgrades::Kind kind){
        if(!canBuyUpgrade(kind)) return false;
        m_fruit -= nextUpgradeCost(kind);
        ++m_upgrade[kind];
        if(kind == Upgrades::HEART) ++m_hp;
        return true;
    }
    // the five levels as a plain array: that is all a save file has to store (fruit, hp and magic are the other three numbers)
    std::array<int, Upgrades::COUNT> getUpgradeLevels() const{
        std::array<int, Upgrades::COUNT> out{};
        for(int i = 0; i < Upgrades::COUNT; ++i) out[i] = m_upgrade[i];
        return out;
    }
    void setUpgradeLevels(const std::array<int, Upgrades::COUNT>& levels){
        for(int i = 0; i < Upgrades::COUNT; ++i) m_upgrade[i] = std::clamp(levels[i], 0, Upgrades::MAX_LEVEL[i]);
    }

    // what the upgrades mean for the game
    int   getMaxHP() const{ return Upgrades::BASE_HEARTS + m_upgrade[Upgrades::HEART]; }
    float slashRangeMul() const{ return Upgrades::SLASH_RANGE_MUL[m_upgrade[Upgrades::SLASH_RANGE]]; }
    float slashDamageMul() const{ return Upgrades::SLASH_DAMAGE_MUL[m_upgrade[Upgrades::SLASH_DAMAGE]]; }
    float beamDamageMul() const{ return Upgrades::BEAM_DAMAGE_MUL[m_upgrade[Upgrades::BEAM]]; }
    float beamCostMul() const{ return Upgrades::BEAM_COST_MUL[m_upgrade[Upgrades::BEAM]]; }
    bool  dashPhasesHazards() const{ return m_upgrade[Upgrades::DASH] > 0; }

    // a fresh run: new game / back to the title screen. Score, fruit, upgrades, hearts and magic all start over.
    void resetRun(){
        m_fruitCollectionScore = 0;
        m_fruit = 0;
        for(int& level : m_upgrade) level = 0;
        m_hp = getMaxHP();
        m_magic = Upgrades::STARTING_MAGIC;
        m_isInvincible = false;
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
    // magic comes ONLY from landing slashes on enemies / bosses (GameState::awardHitMagic): no regeneration, no other source
    void addMagic(float amount){
        m_magic += amount;
        if(m_magic > m_maxMagic) m_magic = m_maxMagic;
    }
    void subMagic(float amount){
        m_magic -= amount;
        if(m_magic < 0.0f) m_magic = 0.0f;
    }
    void setMagic(float value){
        m_magic = value;
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
    int m_fruit = 0;                                   // spendable fruit (the merchant's currency)
    int m_upgrade[Upgrades::COUNT] = {0,0,0,0,0};      // levels bought, see utils/Upgrades.h
    unsigned int m_lastHit =0;
    int m_hp = Upgrades::BASE_HEARTS;
    float m_magic = Upgrades::STARTING_MAGIC;
    float m_maxMagic = Upgrades::MAX_MAGIC;
    bool m_isInvincible=false;
    std::string m_name = "Name";
    PlayerSkin m_playerSkin = PlayerSkin::YELLOW; //default:0(yellow) , 1(blue),2(red),3(green)
};