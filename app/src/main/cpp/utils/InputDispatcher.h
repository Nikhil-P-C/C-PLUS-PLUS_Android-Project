//
// Created by LENOVO on 29-06-2026.
//

#pragma once

class InputDispatcher{
public:
    bool movingLeft  =false;
    bool movingRight =false;
    bool jump =false;
    bool released =true;
    bool attack =false;
    bool heal =false;
    bool beaming =false;
public:
    void inputLogClear(){
        movingLeft  =false;
        movingRight =false;
        jump =false;
        released =true;
        attack =false;
        heal =false;
        beaming =false;
    }
    void setInputReleased(bool shouldRelease){
        released=shouldRelease;
    }
    void setJump(bool shouldJump){
        jump =shouldJump;
    }
    void setMovingLeft(bool shouldMoveLeft){
        movingLeft=shouldMoveLeft;
    }
    void setMovingRight(bool shouldMoveRight){
        movingRight=shouldMoveRight;
    }
    void triggerAttack(){
        attack=true;
    }
    bool consumeAttack(){
        bool prevAttack =attack;
        attack = false;
        return prevAttack;
    }
    void triggerHeal(){
        heal=true;
    }
    bool consumeHeal(){
        bool prevHeal =heal;
        heal = false;
        return prevHeal;
    }
    void setBeaming(bool shouldBeam){
        beaming=shouldBeam;
    }
    bool getJump(){
        return jump;
    }
    bool getMovingLeft(){
        return movingLeft;
    }
    bool getMovingRight(){
        return movingRight;
    }
    bool getReleased(){
        return released;
    }
    static InputDispatcher& getInstance(){
        static InputDispatcher inputDispatcher;
        return inputDispatcher;
    }
private:
    InputDispatcher()=default;
};
