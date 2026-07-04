//
// Created by LENOVO on 29-06-2026.
//
#pragma once
enum ControlType{
    BUTTONS =0,
    JOYSTICK =1,
    SEP_JUMP_W_JOYSTICK =2
};
class GameData{
public:
    void setSize(int width,int height){
        m_winWidth=width;
        m_winHeight=height;
    }
    int getWinHeight() const {
        return m_winHeight;
    }
    int getWinWidth()const{
        return m_winWidth;
    }
    int getControlType()const{
        return m_controlType;
    }
    void setControlType(ControlType type){
        m_controlType= type;
    }
    void setDebug(bool debug){
        m_debug=debug;
    }
    bool isDebugEnabled() const{
        return m_debug;
    }
    static GameData& getInstance(){
        static GameData gameMeta;
        return gameMeta;
    }

private:
    GameData()= default;
    int m_winWidth;
    int m_winHeight;
    bool m_debug=false;
    ControlType m_controlType=ControlType::BUTTONS;
};
