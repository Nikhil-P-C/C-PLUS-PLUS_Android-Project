//
// Created by LENOVO on 02-07-2026.
//
#include "ControlMenuState.h"
#include "SDL3/SDL.h"
#include "SDL3_image/SDL_image.h"
#include "engine/Engine.h"

void ControlMenuState::render(SDL_Renderer *renderer) {

    SDL_FRect menuDst ={100.00f,25.00f,1400.00f,670.00f};
    SDL_FRect menuSrc ={402.00f,0.00f,402.00f,198.00f};
    SDL_RenderTexture(renderer,m_menuTexture,&menuSrc,&menuDst);
    //these three block draws three different part of block to avoid texture being stretched
    {
        SDL_FRect optionBlockDst = {330.f, 50.00f, 1100.00f, 75.00f};
        SDL_FRect optionBlockSrc = {0.00f, 0.00f, 185.00f, 11.00f};
        SDL_RenderTexture(renderer, m_optionBlockTexture, &optionBlockSrc, &optionBlockDst);
    }
    {
        SDL_FRect optionBlockDst = {330.f, 125.00f, 1100.00f, 450.00f};
        SDL_FRect optionBlockSrc = {0.00f, 11.00f, 185.00f, 11.00f};
        SDL_RenderTexture(renderer, m_optionBlockTexture, &optionBlockSrc, &optionBlockDst);
    }
    {
        SDL_FRect optionBlockDst = {330.f, 575.00f, 1100.00f, 75.00f};
        SDL_FRect optionBlockSrc = {0.00f, 22.00f, 185.00f, 11.00f};
        SDL_RenderTexture(renderer, m_optionBlockTexture, &optionBlockSrc, &optionBlockDst);
    }

    //text rendering
    //joystick text

    SDL_FRect joystickFontDst = {400.00f, m_joystickButton.y+(50.00f-15.00f), 160.00f, 30.00f};
    SDL_RenderTexture(renderer, m_joystickFontTexture, nullptr, &joystickFontDst);

    //button text


    SDL_FRect buttonsFontDst = {400.00f, m_buttonButton.y+(50.00f-15.00f), 140.00f, 30.00f};
    SDL_RenderTexture(renderer, m_buttonsFontTexture, nullptr, &buttonsFontDst);



    SDL_FRect joystickWButtonsFontDst = {400.00f, m_joystickWButtons.y+(50.00f-15.00f), 340.00f,30.00f};
    SDL_RenderTexture(renderer, m_joystickWButtonsFontTexture, nullptr, &joystickWButtonsFontDst);



    SDL_FRect keyboardFontDst = {400.00f, m_keyboardButton.y+(50.00f-15.00f), 160.00f,30.00f};
    SDL_RenderTexture(renderer, m_keyboardFontTexture, nullptr, &keyboardFontDst);



    SDL_FRect gamepadFontDst = {400.00f, m_gamepadButton.y+(50.00f-15.00f), 140.00f,30.00f};
    SDL_RenderTexture(renderer, m_gamepadFontTexture, nullptr, &gamepadFontDst);





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
    //Keyboard radio button
    {
        SDL_FRect radioButtonDst = {m_keyboardButton.x + 912.50f, m_keyboardButton.y + 50.00f-(45.00f/2.00f),
                                    9.00f * 5.00f, 9.00f * 5.00f};
        SDL_FRect radioButtonSrc = {10.00f, 0.00f, 9.00f, 9.00f};
        radioButtonSrc.x = m_controlType == ControlType::KEYBOARD ? 0.00f : 10.00f;
        SDL_RenderTexture(renderer, m_radioButtonTexture, &radioButtonSrc, &radioButtonDst);
    }
    //Gamepad radio button
    {
        SDL_FRect radioButtonDst = {m_gamepadButton.x + 912.50f,m_gamepadButton.y + 50.00f-(45.00f/2.00f),
                                    9.00f * 5.00f, 9.00f * 5.00f};
        SDL_FRect radioButtonSrc = {10.00f, 0.00f, 9.00f, 9.00f};
        radioButtonSrc.x = m_controlType == ControlType::GAMEPAD ? 0.00f : 10.00f;
        SDL_RenderTexture(renderer, m_radioButtonTexture, &radioButtonSrc, &radioButtonDst);
    }
}

void ControlMenuState::update(float dt) {
    GameData::getInstance().setControlType(m_controlType);
}

bool ControlMenuState::handleEvents(SDL_Event &event) {
    if(InputUtils::IsPointerDown(event)){
        float touchX, touchY;
        InputUtils::GetPointerPosition(event, m_renderer, touchX, touchY);
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
            return true;
        }
        if(touchX >= m_keyboardButton.x && touchX <= m_keyboardButton.x + m_keyboardButton.w &&
           touchY >= m_keyboardButton.y && touchY <= m_keyboardButton.y + m_keyboardButton.h){
            m_controlType = ControlType::KEYBOARD;
            return true;
        }
        if(touchX >= m_gamepadButton.x && touchX <= m_gamepadButton.x + m_gamepadButton.w &&
           touchY >= m_gamepadButton.y && touchY <= m_gamepadButton.y + m_gamepadButton.h){
            m_controlType = ControlType::GAMEPAD;
            return true;
        }
    }
    return false;
}

ControlMenuState::ControlMenuState(SDL_Renderer *renderer) {
    m_renderer =renderer;

    m_menuTexture =Engine::Get().getAssetManager().getTexture(TextureType::OPTION_MENU_TILE);
    m_optionBlockTexture = Engine::Get().getAssetManager().getTexture(TextureType::MENU_OPTION_BLOCK_TILE);
    m_radioButtonTexture = Engine::Get().getAssetManager().getTexture(TextureType::MENU_RADIO_BUTTON);
    m_font = TTF_OpenFontIO(m_fontfile,false,26);
    LOGI("Loaded font %p", m_font);
    if (!m_font)
        LOGI("%s", SDL_GetError());
    //text texture
    SDL_Surface* jotstickFontSurface = TTF_RenderText_Solid(m_font, "Joystick",8,
                                                    SDL_Color{0, 0, 0, 255});
    m_joystickFontTexture = SDL_CreateTextureFromSurface(renderer, jotstickFontSurface);
    SDL_DestroySurface(jotstickFontSurface);


    SDL_Surface *buttonsFontSurface = TTF_RenderText_Solid(m_font, "Buttons",7,
                                                    SDL_Color{0, 0, 0, 255});
    m_buttonsFontTexture = SDL_CreateTextureFromSurface(renderer, buttonsFontSurface);
    SDL_DestroySurface(buttonsFontSurface);


    SDL_Surface* joystickWButtonsFontSurface = TTF_RenderText_Solid(m_font, "Joystick + Button",17,
                                                    SDL_Color{0, 0, 0, 255});
    m_joystickWButtonsFontTexture = SDL_CreateTextureFromSurface(renderer, joystickWButtonsFontSurface);
    SDL_DestroySurface(joystickWButtonsFontSurface);


    SDL_Surface* keyboardFontSurface = TTF_RenderText_Solid(m_font, "Keyboard",8,
                                                                    SDL_Color{0, 0, 0, 255});
    m_keyboardFontTexture = SDL_CreateTextureFromSurface(renderer, keyboardFontSurface);
    SDL_DestroySurface(keyboardFontSurface);


    SDL_Surface* gamepadFontSurface = TTF_RenderText_Solid(m_font, "Gamepad",7,
                                                            SDL_Color{0, 0, 0, 255});
    m_gamepadFontTexture = SDL_CreateTextureFromSurface(renderer, gamepadFontSurface);
    SDL_DestroySurface(gamepadFontSurface);
}

ControlMenuState::~ControlMenuState() {
    SDL_DestroyTexture(m_joystickFontTexture);
    SDL_DestroyTexture(m_buttonsFontTexture);
    SDL_DestroyTexture(m_joystickWButtonsFontTexture);
    SDL_DestroyTexture(m_keyboardFontTexture);
    SDL_DestroyTexture(m_gamepadFontTexture);
    TTF_CloseFont(m_font);
    SDL_CloseIO(m_fontfile);
}


