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
    ControlType getControlType()const{
        return m_controlType;
    }
    float getAudioScale()const{
        return m_audioScale;
    }
    void setControlType(ControlType type){
        m_controlType= type;
    }
    void setDebug(bool debug){
        m_debug=debug;
    }
    void setAudioScale(float scale) {
        if (scale > 1.0f) scale = 1.0f;
        if (scale < 0.0f) scale = 0.0f;
        m_audioScale =scale;
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
    float m_audioScale= 1.0f;

    ControlType m_controlType=ControlType::BUTTONS;
};
