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
        return m_masterAudioScale;
    }
    float getMusicAudioScale()const{
        return m_musicAudioScale;
    }
    float getSFXAudioScale()const{
        return m_sfxAudioScale;
    }
    bool isPaused() const{
        return m_paused;
    }
    void setControlType(ControlType type){
        m_controlType= type;
    }
    void setPaused(bool pause){
        m_paused =pause;
    }
    void toggleDebug(){
        m_debug=!m_debug;
    }
    //set audio scales
    //master
    void setAudioScale(float scale) {
        if (scale > 1.0f) scale = 1.0f;
        if (scale < 0.0f) scale = 0.0f;
        m_masterAudioScale =scale;
    }
    //music
    void setMusicAudioScale(float scale) {
        if (scale > 1.0f) scale = 1.0f;
        if (scale < 0.0f) scale = 0.0f;
        m_musicAudioScale =scale;
    }
    //sfx
    void setSFXAudioScale(float scale) {
        if (scale > 1.0f) scale = 1.0f;
        if (scale < 0.0f) scale = 0.0f;
        m_sfxAudioScale = scale;
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
    bool m_debug=true;
    bool m_paused=false;
    float m_masterAudioScale= 1.0f;
    float m_musicAudioScale= 1.0f;
    float m_sfxAudioScale= 1.0f;
    ControlType m_controlType=ControlType::BUTTONS;
};
