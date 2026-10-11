//
// Created by LENOVO on 21-07-2026.
//
// Edit screen: pick a skin, type a name, SAVE, BACK. Same look as the other menus (MenuKit).
//   keyboard / gamepad: up-down = row, left-right = skin, confirm = edit name / save / back
//   mouse / touch:      click a skin, the name field, SAVE or BACK
//
#pragma once
#include <string>
#include "State/State.h"
#include "utils/utils.h"
#include <SDL3/SDL.h>
#include "States/MenuState/MenuKit.h"

class EditMenuState :public State{
public:
    explicit EditMenuState(SDL_Renderer* renderer);
    ~EditMenuState()override;
    void render(SDL_Renderer* renderer)override;
    void update(float dt)override;
    bool handleEvents(SDL_Event &event)override;
private:
    enum Row { ROW_SKIN = 0, ROW_NAME = 1, ROW_SAVE = 2, ROW_BACK = 3, ROW_COUNT = 4 };
    static constexpr int SKIN_COUNT = 4;

    int  rowAt(float x,float y) const;                 // which row the pointer is over, -1 = none
    int  skinAt(float x,float y) const;
    void selectSkin(int skin,bool sound);
    void beginTextInput();
    void endTextInput();
    void saveName();
    void leave();
    void perform(InputUtils::MenuAction action);
    SDL_Texture* skinTexture(int skin) const;

    SDL_Renderer* m_renderer = nullptr;
    MenuBackdrop  m_backdrop;
    MenuFont      m_font;
    MenuLabel     m_nameTag, m_nameValue, m_saveLabel, m_savedLabel, m_backLabel;
    MenuRows      m_rows;
    InputUtils::MenuNavRepeater m_nav;

    SDL_Texture*  m_skinTex[SKIN_COUNT] = {};
    float         m_skinAnim[SKIN_COUNT] = {};         // eased "this is the chosen skin"
    int           m_skin = 0;

    std::string   m_editName;
    bool          m_textActive = false;
    float         m_savedTimer = 0.0f;                 // > 0: SAVE shows "SAVED"
    float         m_blink = 0.0f;
    float         m_animTimer = 0.0f;                  // sprite animation (4 frames, 0.1s each)
    int           m_frame = 0;
    float         m_saveAnim = 0.0f, m_backAnim = 0.0f;
    bool          m_leaving = false;
};
