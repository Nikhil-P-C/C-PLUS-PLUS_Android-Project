//
// Created by LENOVO on 02-07-2026.
//
#include "ControlMenuState.h"
#include "SDL3/SDL.h"
#include "SDL3_image/SDL_image.h"

void ControlMenuState::render(SDL_Renderer *renderer) {

    SDL_FRect menuDst ={100.00f,25.00f,1400.00f,670.00f};
    SDL_FRect menuSrc ={402.00f,0.00f,402.00f,198.00f};
    SDL_RenderTexture(renderer,m_menuTexture,&menuSrc,&menuDst);

    SDL_FRect optionBlockDst = {330.f, 75.00f, 1100.00f, 350.00f};
    SDL_FRect optionBlockSrc = {0.00f, 0.00f, 185.00f, 34.00f};
    SDL_RenderTexture(renderer, m_optionBlockTexture, &optionBlockSrc, &optionBlockDst);
//
//    SDL_SetRenderDrawColor(renderer,0,0,255,255);
//    SDL_FRect joystickButton ={m_joystickButton.x,m_joystickButton.y,m_joystickButton.w,m_joystickButton.h};
//    SDL_RenderRect(renderer,&joystickButton);
//
//    SDL_SetRenderDrawColor(renderer,255,0,0,255);
//    SDL_FRect buttonButton ={m_buttonButton.x,m_buttonButton.y,m_buttonButton.w,m_buttonButton.h};
//    SDL_RenderRect(renderer,&buttonButton);
//
//    SDL_SetRenderDrawColor(renderer,0,255,0,255);
//    SDL_FRect joystickWButton ={m_joystickWButtons.x,m_joystickWButtons.y,m_joystickWButtons.w,m_joystickWButtons.h};
//    SDL_RenderRect(renderer,&joystickWButton);
    //text rendering
    //joystick text
    {
        SDL_Surface *fontSurface = TTF_RenderText_Solid(m_font, "Joystick",8,
                                                        SDL_Color{0, 0, 0, 255});
        m_fontTexture = SDL_CreateTextureFromSurface(renderer, fontSurface);
        SDL_DestroySurface(fontSurface);
        SDL_FRect fontDst = {400.00f, m_joystickButton.y+(50.00f-15.00f), 160.00f, 30.00f};
        SDL_RenderTexture(renderer, m_fontTexture, nullptr, &fontDst);
    }
    //button text
    {
        SDL_Surface *fontSurface = TTF_RenderText_Solid(m_font, "Buttons",7,
                                                        SDL_Color{0, 0, 0, 255});
        m_fontTexture = SDL_CreateTextureFromSurface(renderer, fontSurface);
        SDL_DestroySurface(fontSurface);
        SDL_FRect fontDst = {400.00f, m_buttonButton.y+(50.00f-15.00f), 140.00f, 30.00f};
        SDL_RenderTexture(renderer, m_fontTexture, nullptr, &fontDst);
    }
    {
        SDL_Surface *fontSurface = TTF_RenderText_Solid(m_font, "Joystick + Button",17,
                                                        SDL_Color{0, 0, 0, 255});
        m_fontTexture = SDL_CreateTextureFromSurface(renderer, fontSurface);
        SDL_DestroySurface(fontSurface);
        SDL_FRect fontDst = {400.00f, m_joystickWButtons.y+(50.00f-15.00f), 340.00f,30.00f};
        SDL_RenderTexture(renderer, m_fontTexture, nullptr, &fontDst);
    }



    //radio button rendering
    //joystick radio button
    {
        SDL_FRect radioButtonDst = {m_joystickButton.x + 912.50f, m_joystickButton.y + 50.00f-(45.00f/2.00f),
                                    9.00f * 5.00f, 9.00f * 5.00f};
        SDL_FRect radioButtonSrc = {0.00f, 0.00f, 9.00f, 9.00f};
        radioButtonSrc.x = m_controlType == ControlType::JOYSTICK ? 0.00f : 10.00f;
        SDL_RenderTexture(renderer, m_radioButtonTexture, &radioButtonSrc, &radioButtonDst);
    }
    //button radio button
    {
        SDL_FRect radioButtonDst = {m_buttonButton.x + 912.50f, m_buttonButton.y + 50.00f-(45.00f/2.00f),
                                    9.00f * 5.00f, 9.00f * 5.00f};
        SDL_FRect radioButtonSrc = {10.00f, 0.00f, 9.00f, 9.00f};
        radioButtonSrc.x = m_controlType == ControlType::BUTTONS ? 0.00f : 10.00f;
        SDL_RenderTexture(renderer, m_radioButtonTexture, &radioButtonSrc, &radioButtonDst);
    }
    //joystick w button radio button
    {
        SDL_FRect radioButtonDst = {m_joystickWButtons.x + 912.50f, m_joystickWButtons.y + 50.00f-(45.00f/2.00f),
                                    9.00f * 5.00f, 9.00f * 5.00f};
        SDL_FRect radioButtonSrc = {10.00f, 0.00f, 9.00f, 9.00f};
        radioButtonSrc.x = m_controlType == ControlType::SEP_JUMP_W_JOYSTICK ? 0.00f : 10.00f;
        SDL_RenderTexture(renderer, m_radioButtonTexture, &radioButtonSrc, &radioButtonDst);
    }
}

void ControlMenuState::update(float dt) {
    GameData::getInstance().setControlType(m_controlType);
}

bool ControlMenuState::handleEvents(SDL_Event &event) {
    float touchX = event.tfinger.x * (float)GameData::getInstance().getWinWidth();
    float touchY = event.tfinger.y * (float)GameData::getInstance().getWinHeight();

    if(event.type == SDL_EVENT_FINGER_DOWN){
        if(touchX >= m_joystickButton.x && touchX <= m_joystickButton.x + m_joystickButton.w &&
            touchY >= m_joystickButton.y && touchY <= m_joystickButton.y + m_joystickButton.h){
            m_controlType = ControlType::JOYSTICK;
            return true;
        }
        if(touchX >= m_buttonButton.x && touchX <= m_buttonButton.x + m_buttonButton.w &&
            touchY >= m_buttonButton.y && touchY <= m_buttonButton.y + m_buttonButton.h){
            m_controlType = ControlType::BUTTONS;
            return true;
        }
        if(touchX >= m_joystickWButtons.x && touchX <= m_joystickWButtons.x + m_joystickWButtons.w &&
            touchY >= m_joystickWButtons.y && touchY <= m_joystickWButtons.y + m_joystickWButtons.h){
            m_controlType = ControlType::SEP_JUMP_W_JOYSTICK;
        }
    }
    return false;
}

ControlMenuState::ControlMenuState(SDL_Renderer *renderer) {

    SDL_Surface* menuSurface = IMG_Load_IO(m_menuSpriteFile,false);
    m_menuTexture = SDL_CreateTextureFromSurface(renderer,menuSurface);
    SDL_CloseIO(m_menuSpriteFile);
    SDL_DestroySurface(menuSurface);

    SDL_Surface* optionBlockSurface = IMG_Load_IO(m_optionBlockFile,false);
    m_optionBlockTexture = SDL_CreateTextureFromSurface(renderer,optionBlockSurface);
    SDL_CloseIO(m_optionBlockFile);
    SDL_DestroySurface(optionBlockSurface);

    SDL_Surface* radioButtonSurface = IMG_Load_IO(m_radioButtonfile,false);
    m_radioButtonTexture = SDL_CreateTextureFromSurface(renderer,radioButtonSurface);
    SDL_CloseIO(m_radioButtonfile);
    SDL_DestroySurface(radioButtonSurface);

    m_font = TTF_OpenFontIO(m_fontfile,false,26);
    LOGI("Loaded font %p", m_font);
    if (!m_font)
        LOGI("%s", SDL_GetError());

    SDL_SetTextureScaleMode(m_fontTexture,SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_menuTexture,SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_radioButtonTexture,SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_optionBlockTexture,SDL_SCALEMODE_NEAREST);
}


