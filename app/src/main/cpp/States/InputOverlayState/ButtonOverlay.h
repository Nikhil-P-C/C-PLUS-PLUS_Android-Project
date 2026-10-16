//
// Created by LENOVO on 12-05-2026.
//
#pragma once
#include <vector>
#include "Engine/Engine.h"
#include "State/State.h"
#include "utils/button.h"

class ButtonOverlay: public State {
public:
    void render(SDL_Renderer* renderer) override;

    void update(float dt) override;

    bool handleEvents(SDL_Event& event) override;
    ButtonOverlay(SDL_Renderer* renderer);
    ~ButtonOverlay()override;
private:
    static const unsigned int MAGIC_HOLD_THRESHOLD_MS = 180; // hold past this -> beam, release before -> heal

    Button m_JumpButton{1400,400,100,100};
    Button m_LeftButton{200 ,400,100,100};
    Button m_RightButton{300,400,100,100};
    Button m_UpButton{250,290,100,100};      // above left/right. Tap = interact, held = aim up
    Button m_DownButton{250,510,100,100};    // below left/right. Held = aim down
    Button m_DashButton{1280,420,100,100};   // placeholder rect (no art yet): tap = dash
    Button m_CrouchButton{1400,600,100,100};
    Button m_AttackButton{1450,300,100,100};
    Button m_MagicButton{1300,300,100,100};

    SDL_Texture* m_jumpButtonTexture = nullptr;
    SDL_Texture* m_leftButtonTexture = nullptr;
    SDL_Texture* m_rightButtonTexture = nullptr;
    SDL_Texture* m_upButtonTexture = nullptr;
    SDL_Texture* m_downButtonTexture = nullptr;
    SDL_Texture* m_slashButtonTexture = nullptr;
    SDL_Texture* m_magicButtonTexture = nullptr;

    SDL_FingerID m_jumpFingerID=0;
    SDL_FingerID m_dFingerID=0;
    SDL_FingerID m_attackFingerID=0;
    SDL_FingerID m_magicFingerID=0;
    SDL_FingerID m_upFingerID=0;
    SDL_FingerID m_downFingerID=0;

    float m_TouchX =0;
    float m_TouchY =0;
    bool m_jumpFingerActive = false;
    bool m_dFingerActive = false;
    bool m_attackFingerActive = false;
    bool m_upPressed = false;                // up button tapped: fires one interact on the next update
    bool m_upFingerActive = false;           // up button held: aim up (upward slash)
    bool m_downFingerActive = false;         // down button held: aim down (downward slash / pogo)
    bool m_magicFingerActive = false;
    bool m_magicBeamStarted = false;       // true once the hold has crossed the threshold
    uint32_t m_magicPressStartTime = 0;

};

