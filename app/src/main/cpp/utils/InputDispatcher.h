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
    bool dash =false;       // dash pressed (one-shot, consumed by GameState)
    bool interact =false;   // talk to / read the interactable in range (also advances an open dialog)
    // held aim directions, shared by every input overlay. Attack + lookUp = upward slash,
    // attack + lookDown (in the air) = downward slash / pogo, otherwise the slash goes sideways.
    bool lookUp =false;
    bool lookDown =false;

private:
    bool m_jumpPressed =false;

public:
    void inputLogClear(){
        movingLeft  =false;
        movingRight =false;
        jump =false;
        released =true;
        attack =false;
        heal =false;
        beaming =false;
        interact =false;
        dash =false;
        lookUp =false;
        lookDown =false;
        m_jumpPressed =false;
    }
    void setInputReleased(bool shouldRelease){
        released=shouldRelease;
    }
    void setJump(bool shouldJump){
        if(shouldJump && !jump) m_jumpPressed =true; // rising edge only, fires once per press
        jump =shouldJump;
    }
    bool consumeJumpPressed(){
        bool prevJumpPressed =m_jumpPressed;
        m_jumpPressed =false;
        return prevJumpPressed;
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
    void triggerDash(){
        dash=true;
    }
    bool consumeDash(){
        bool prevDash =dash;
        dash = false;
        return prevDash;
    }
    void triggerInteract(){
        interact=true;
    }
    bool consumeInteract(){
        bool prevInteract =interact;
        interact = false;
        return prevInteract;
    }
    void setLookUp(bool shouldLookUp){
        lookUp=shouldLookUp;
    }
    void setLookDown(bool shouldLookDown){
        lookDown=shouldLookDown;
    }
    bool getLookUp() const{
        return lookUp;
    }
    bool getLookDown() const{
        return lookDown;
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