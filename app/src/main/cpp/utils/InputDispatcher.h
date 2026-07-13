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

public:
    void inputLogClear(){
        movingLeft  =false;
        movingRight =false;
        jump =false;
        released =true;
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
