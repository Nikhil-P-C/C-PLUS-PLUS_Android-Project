//
// Created by LENOVO on 04-05-2026.
//

#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL.h>
#include "MenuState.h"
#include "Engine/engine.h"
#include "States/GameState/GameState.h"
#include "States/DebugState/DebugState.h"
#include "States/InputOverlayState/JoystickOverlay.h"
#include "States/InputOverlayState/ButtonOverlay.h"
#include "States/InputOverlayState/SepJoysticknButton.h"
#include "States/MenuState/OptionMenuState/OptionMenuState.h"
#include "States/MenuState/EditMenuState/EditMenuState.h"

MenuState::MenuState(SDL_Renderer *renderer) {
    LOGI("menu construct:%p",this);
    if(!m_fontfile)return;
    if(!m_fontShadowfile)return;



    m_font =TTF_OpenFontIO(m_fontfile,false,32);
    m_fontShadow =TTF_OpenFontIO(m_fontShadowfile,false,32);

    if(!m_font)return;
    if(!m_fontShadow)return;


    m_renderer = renderer;
    m_background = Engine::Get().getAssetManager().getTexture(TextureType::BG_CONFETTI);
    m_playButtonTexture = Engine::Get().getAssetManager().getTexture(TextureType::MENU_BUTTONS);

    //text texture
    //play text
    SDL_Surface *playFontSurface = TTF_RenderText_Solid(m_font, " Play",
                                                        5, SDL_Color{255, 255, 255, 255});
    SDL_Surface *playFontShadowSurface = TTF_RenderText_Solid(m_fontShadow, " Play",
                                                              5, SDL_Color{0, 0, 0, 255});
    m_playText = SDL_CreateTextureFromSurface(renderer, playFontSurface);
    m_playTextShadow = SDL_CreateTextureFromSurface(renderer, playFontShadowSurface);
    SDL_DestroySurface(playFontSurface);
    SDL_DestroySurface(playFontShadowSurface);

    //option text
    SDL_Surface *optionFontSurface = TTF_RenderText_Solid(m_font, " option",
                                                        7, SDL_Color{255, 255, 255, 255});
    SDL_Surface *optionFontShadowSurface = TTF_RenderText_Solid(m_fontShadow, " option",
                                                              7, SDL_Color{0, 0, 0, 255});
    m_optionText = SDL_CreateTextureFromSurface(renderer, optionFontSurface);
    m_optionTextShadow = SDL_CreateTextureFromSurface(renderer, optionFontShadowSurface);
    SDL_DestroySurface(optionFontSurface);
    SDL_DestroySurface(optionFontShadowSurface);

    //quit text
    SDL_Surface *quitFontSurface = TTF_RenderText_Solid(m_font, " Quit",
                                                    5, SDL_Color{255, 255, 255, 255});
    SDL_Surface *quitFontShadowSurface = TTF_RenderText_Solid(m_fontShadow, " Quit",
                                                          5, SDL_Color{0, 0, 0, 255});
    m_quitText = SDL_CreateTextureFromSurface(renderer, quitFontSurface);
    m_quitTextShadow = SDL_CreateTextureFromSurface(renderer, quitFontShadowSurface);
    SDL_DestroySurface(quitFontSurface);
    SDL_DestroySurface(quitFontShadowSurface);

    //edit text
    SDL_Surface *editFontSurface = TTF_RenderText_Solid(m_font, " Edit",
                                                    5, SDL_Color{255, 255, 255, 255});
    SDL_Surface *editFontShadowSurface = TTF_RenderText_Solid(m_fontShadow, " Edit",
                                                          5, SDL_Color{0, 0, 0, 255});
    m_editText = SDL_CreateTextureFromSurface(renderer, editFontSurface);
    m_editTextShadow = SDL_CreateTextureFromSurface(renderer, editFontShadowSurface);
    SDL_DestroySurface(editFontSurface);
    SDL_DestroySurface(editFontShadowSurface);

    SDL_SetTextureScaleMode(m_playText,SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_playTextShadow,SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_optionText,SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_optionTextShadow,SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_quitText,SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_quitTextShadow,SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_editText,SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_editTextShadow,SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_playButtonTexture,SDL_SCALEMODE_NEAREST);

}

MenuState::~MenuState() {

    LOGI("menu destructor:%p",this);

    SDL_DestroyTexture(m_playText);
    SDL_DestroyTexture(m_playTextShadow);
    SDL_DestroyTexture(m_optionText);
    SDL_DestroyTexture(m_optionTextShadow);
    SDL_DestroyTexture(m_quitText);
    SDL_DestroyTexture(m_quitTextShadow);
    SDL_DestroyTexture(m_editText);
    SDL_DestroyTexture(m_editTextShadow);

    TTF_CloseFont(m_font);
    TTF_CloseFont(m_fontShadow);

    SDL_CloseIO(m_fontfile);
    SDL_CloseIO(m_fontShadowfile);
}

void MenuState::render(SDL_Renderer* renderer) {
    SDL_FRect backgroundDst{0, 0, 1600, 720};
    SDL_RenderTexture(renderer, m_background, nullptr, &backgroundDst);
    //button pixels 594 x 282
    SDL_FRect playButtonSrc{0+(1*594),0,594,282};
    SDL_FRect playButtonDst{m_playButton.x,m_playButton.y,m_playButton.w,m_playButton.h};
    SDL_RenderTexture(renderer, m_playButtonTexture, &playButtonSrc, &playButtonDst);

    SDL_FRect settingButtonSrc{0+(2*594),0,594,282};
    SDL_FRect settingButtonDst{m_optionButton.x,m_optionButton.y,m_optionButton.w,m_optionButton.h};
    SDL_RenderTexture(renderer, m_playButtonTexture, &settingButtonSrc, &settingButtonDst);

    SDL_FRect quitButtonSrc{0+(3*594),0,594,282};
    SDL_FRect quitButtonDst{m_quitButton.x,m_quitButton.y,m_quitButton.w,m_quitButton.h};
    SDL_RenderTexture(renderer, m_playButtonTexture, &quitButtonSrc, &quitButtonDst);

    SDL_FRect editButtonSrc{0+(0*594),0,594,282};
    SDL_FRect editButtonDst{m_editButton.x,m_editButton.y,m_editButton.w,m_editButton.h};
    SDL_RenderTexture(renderer, m_playButtonTexture, &editButtonSrc, &editButtonDst);


    SDL_FRect playTextDst = {220.00f,130.00f,207.00f,110.00f};
    SDL_RenderTexture(renderer, m_playTextShadow, nullptr, &playTextDst);
    SDL_RenderTexture(renderer, m_playText, nullptr, &playTextDst);


    SDL_FRect optionTextDst = {175.00f,300.00f,291.00f,110.00f};
    SDL_RenderTexture(renderer, m_optionTextShadow, nullptr, &optionTextDst);
    SDL_RenderTexture(renderer, m_optionText, nullptr, &optionTextDst);


    SDL_FRect quitTextDst = {220.00f,470.00f,207.00f,110.00f};
    SDL_RenderTexture(renderer, m_quitTextShadow, nullptr, &quitTextDst);
    SDL_RenderTexture(renderer, m_quitText, nullptr, &quitTextDst);



    SDL_FRect editTextDst = {1170.00f,380.00f,207.00f,110.00f};
    SDL_RenderTexture(renderer, m_editTextShadow, nullptr, &editTextDst);
    SDL_RenderTexture(renderer, m_editText, nullptr, &editTextDst);


}

void MenuState::update(float dt) {

}

bool MenuState::handleEvents(SDL_Event &event) {
    if(m_transitioning)return true;
    if(event.type == SDL_EVENT_FINGER_DOWN){
        float touchX =event.tfinger.x * 1600, touchY =event.tfinger.y*720;
        if(touchX > m_playButton.x && touchX < m_playButton.x + m_playButton.w){
            if(touchY > m_playButton.y && touchY < m_playButton.y + m_playButton.h){
                m_transitioning =true;
                LOGI("menu starts transitioning to game state");
                Engine::Get().changeState(std::make_unique<GameState>(m_renderer));

                if(GameData::getInstance().getControlType() == JOYSTICK) {
                    Engine::Get().pushOverlayState(std::make_unique<JoystickOverlay>(m_renderer));
                }
                if(GameData::getInstance().getControlType() == BUTTONS){
                    Engine::Get().pushOverlayState(std::make_unique<ButtonOverlay>(m_renderer));
                }
                if(GameData::getInstance().getControlType() == SEP_JUMP_W_JOYSTICK){
                    Engine::Get().pushOverlayState(std::make_unique<SepJoysticknButton>(m_renderer));
                }
                return true;

            }

        }
        if(touchX > m_optionButton.x && touchX < m_optionButton.x + m_optionButton.w){
            if(touchY > m_optionButton.y && touchY < m_optionButton.y + m_optionButton.h){
                m_transitioning =true;
                Engine::Get().changeState(std::make_unique<OptionMenuState>(m_renderer));
                return true;
            }
        }
        if(touchX >m_editButton.x && touchX < m_editButton.x+m_editButton.w){
            if(touchY>m_editButton.y&& touchY<m_editButton.y + m_editButton.h){
                m_transitioning =true;
                Engine::Get().changeState(std::make_unique<EditMenuState>(m_renderer));
                return true;
            }
        }
        if(touchX > m_quitButton.x && touchX < m_quitButton.x + m_quitButton.w){
            if(touchY > m_quitButton.y && touchY < m_quitButton.y + m_quitButton.h){
                LOGI("menu starts transitioning to game state");
                Engine::Get().exitEngine();
                return true;
            }

        }
        return true;
    }
    return false;
}