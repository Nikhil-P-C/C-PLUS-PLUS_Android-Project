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

MenuState::MenuState(SDL_Renderer *renderer) {
    LOGI("menu construct:%p",this);
    if(!m_fontfile)return;
    if(!m_fontShadowfile)return;

    if(!m_backGroundSprite)return;
    if(!m_buttonSprite)return;

    m_font =TTF_OpenFontIO(m_fontfile,false,32);
    m_fontShadow =TTF_OpenFontIO(m_fontShadowfile,false,32);

    if(!m_font)return;
    if(!m_fontShadow)return;

    SDL_Surface* Backgroundsurface = IMG_Load_IO(m_backGroundSprite,false);
    SDL_Surface* playButtonSurface = IMG_Load_IO(m_buttonSprite,false);

    m_renderer = renderer;
    m_background = SDL_CreateTextureFromSurface(renderer,Backgroundsurface);
    m_playButtonTexture = SDL_CreateTextureFromSurface(renderer,playButtonSurface);
    SDL_SetTextureScaleMode(m_playText,SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_playTextShadow,SDL_SCALEMODE_NEAREST);
    SDL_DestroySurface(Backgroundsurface);
    SDL_DestroySurface(playButtonSurface);
    if(!m_background)return;
    if(!m_playButtonTexture)return;
}

MenuState::~MenuState() {
    SDL_CloseIO(m_backGroundSprite);
    SDL_CloseIO(m_buttonSprite);
    LOGI("menu destructor:%p",this);
    SDL_DestroyTexture(m_background);
    SDL_DestroyTexture(m_playButtonTexture);
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

    SDL_FRect customizeButtonSrc{0+(0*594),0,594,282};
    SDL_FRect customizeButtonDst{m_customizeButton.x,m_customizeButton.y,m_customizeButton.w,m_customizeButton.h};
    SDL_RenderTexture(renderer, m_playButtonTexture, &customizeButtonSrc, &customizeButtonDst);


    {
        SDL_Surface *fontSurface = TTF_RenderText_Solid(m_font, "Play",
                                                        4, SDL_Color{255, 255, 255, 255});
        SDL_Surface *fontShadowSurface = TTF_RenderText_Solid(m_fontShadow, "Play",
                                                              4, SDL_Color{0, 0, 0, 255});
        m_playText = SDL_CreateTextureFromSurface(renderer, fontSurface);
        m_playTextShadow = SDL_CreateTextureFromSurface(renderer, fontShadowSurface);
        SDL_DestroySurface(fontSurface);
        SDL_DestroySurface(fontShadowSurface);
        SDL_FRect dst = {m_playButton.x+90,m_playButton.y+30,m_playButton.w-180,m_playButton.h-40};
        SDL_RenderTexture(renderer, m_playTextShadow, nullptr, &dst);
        SDL_RenderTexture(renderer, m_playText, nullptr, &dst);
    }

    {
        SDL_Surface *fontSurface = TTF_RenderText_Solid(m_font, "option",
                                                        6, SDL_Color{255, 255, 255, 255});
        SDL_Surface *fontShadowSurface = TTF_RenderText_Solid(m_fontShadow, "option",
                                                              6, SDL_Color{0, 0, 0, 255});
        m_playText = SDL_CreateTextureFromSurface(renderer, fontSurface);
        m_playTextShadow = SDL_CreateTextureFromSurface(renderer, fontShadowSurface);
        SDL_DestroySurface(fontSurface);
        SDL_DestroySurface(fontShadowSurface);
        SDL_FRect dst = {m_optionButton.x+80,m_optionButton.y+30,m_optionButton.w-160,m_optionButton.h-40};
        SDL_RenderTexture(renderer, m_playTextShadow, nullptr, &dst);
        SDL_RenderTexture(renderer, m_playText, nullptr, &dst);
    }
    {
        SDL_Surface *fontSurface = TTF_RenderText_Solid(m_font, "Quit",
                                                        4, SDL_Color{255, 255, 255, 255});
        SDL_Surface *fontShadowSurface = TTF_RenderText_Solid(m_fontShadow, "Quit",
                                                              4, SDL_Color{0, 0, 0, 255});
        m_playText = SDL_CreateTextureFromSurface(renderer, fontSurface);
        m_playTextShadow = SDL_CreateTextureFromSurface(renderer, fontShadowSurface);
        SDL_DestroySurface(fontSurface);
        SDL_DestroySurface(fontShadowSurface);
        SDL_FRect dst = {m_quitButton.x+90,m_quitButton.y+30,m_quitButton.w-180,m_quitButton.h-40};
        SDL_RenderTexture(renderer, m_playTextShadow, nullptr, &dst);
        SDL_RenderTexture(renderer, m_playText, nullptr, &dst);

    }
    {
        SDL_Surface *fontSurface = TTF_RenderText_Solid(m_font, "Edit",
                                                        4, SDL_Color{255, 255, 255, 255});
        SDL_Surface *fontShadowSurface = TTF_RenderText_Solid(m_fontShadow, "Edit",
                                                              4, SDL_Color{0, 0, 0, 255});
        m_playText = SDL_CreateTextureFromSurface(renderer, fontSurface);
        m_playTextShadow = SDL_CreateTextureFromSurface(renderer, fontShadowSurface);
        SDL_DestroySurface(fontSurface);
        SDL_DestroySurface(fontShadowSurface);
        SDL_FRect dst = {m_customizeButton.x+90,m_customizeButton.y+30,m_customizeButton.w-180,m_customizeButton.h-40};
        SDL_RenderTexture(renderer, m_playTextShadow, nullptr, &dst);
        SDL_RenderTexture(renderer, m_playText, nullptr, &dst);
    }

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
