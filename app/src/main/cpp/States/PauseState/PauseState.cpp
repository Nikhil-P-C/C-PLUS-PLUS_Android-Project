//
// Created by LENOVO on 01-05-2026.
//
#include <android/log.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>

#include "PauseState.h"
#include "engine/Engine.h"
#include "States/GameState/GameState.h"
#include "States/MenuState/TitleScreenState.h"
#include "States/MenuState/MenuState.h"
#include "States/InputOverlayState/JoystickOverlay.h"
#include "States/InputOverlayState/ButtonOverlay.h"
#include "States/InputOverlayState/SepJoysticknButton.h"
#include "States/DebugState/DebugState.h"

PauseState::PauseState(SDL_Renderer *renderer, GameState* gameState) {
    LOGI("Pause construct:%p",this);
    m_gameState = gameState;
    init(renderer);
}

void PauseState::init(SDL_Renderer *renderer) {
    m_font = TTF_OpenFontIO(m_fontfile,false,24);
    LOGI("font:%d",(bool)m_font);
    m_fontShadow = TTF_OpenFontIO(m_fontShadowfile,false,24);


    m_renderer = renderer;
    SDL_Surface* fontSurface = TTF_RenderText_Solid(m_font,"paused ",
                                                    7,SDL_Color{255,255,255,255});
    SDL_Surface* fontShadowSurface = TTF_RenderText_Solid(m_fontShadow,"paused ",
                                                          7,SDL_Color{0,0,0,255});
    m_pauseTexture = SDL_CreateTextureFromSurface(renderer,fontSurface);
    m_pauseShadowTexture = SDL_CreateTextureFromSurface(renderer,fontShadowSurface);
    SDL_DestroySurface(fontSurface);
    SDL_DestroySurface(fontShadowSurface);

    SDL_Surface* exitSurface = TTF_RenderText_Solid(m_font,"exit",
                                                    4,SDL_Color{255,255,255,255});
    SDL_Surface* exitShadowSurface = TTF_RenderText_Solid(m_fontShadow,"exit",
                                                          4,SDL_Color{0,0,0,255});
    m_exitTexture = SDL_CreateTextureFromSurface(renderer,exitSurface);
    m_exitShadowTexture = SDL_CreateTextureFromSurface(renderer,exitShadowSurface);
    SDL_DestroySurface(exitSurface);
    SDL_DestroySurface(exitShadowSurface);

    SDL_SetTextureScaleMode(m_pauseTexture,SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_pauseShadowTexture,SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_exitTexture,SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_exitShadowTexture,SDL_SCALEMODE_NEAREST);
    LOGI("Pause init completed");
}

void PauseState::render(SDL_Renderer *renderer) {



    SDL_FRect pauseDst = {800 - 150, 320 - 50, 300, 100};
    SDL_RenderTexture(renderer, m_pauseShadowTexture, nullptr, &pauseDst);
    SDL_RenderTexture(renderer, m_pauseTexture, nullptr, &pauseDst);




    SDL_FRect exitDst = {800-100, 600 - 50, 200, 100};
    SDL_RenderTexture(renderer, m_exitShadowTexture, nullptr, &exitDst);
    SDL_RenderTexture(renderer, m_exitTexture, nullptr, &exitDst);

}

void PauseState::update(float dt) {

}
bool PauseState::shouldClose(float x ,float y) const{
    if(x >= m_exit.x && x <= m_exit.x + m_exit.w && y >= m_exit.y && y <= m_exit.y + m_exit.h)return true;
    return false;
}
bool PauseState::handleEvents(SDL_Event &event) {
    if(m_transitioning)return true;
    if(event.type == SDL_EVENT_FINGER_DOWN){
        if (shouldClose(event.tfinger.x * 1600, event.tfinger.y * 720)) {
            PlayerDetail::getInstance().setScore(0);
            GameData::getInstance().setPaused(false);
            m_transitioning = true;
            LOGI("should close");
            Engine::Get().popState();
            if(GameData::getInstance().isDebugEnabled())
                Engine::Get().popOverlayState();
            Engine::Get().changeState(std::make_unique<MenuState>(m_renderer));
            return true;
        }
    }
    if(event.type == SDL_EVENT_FINGER_DOWN &&!shouldClose(event.tfinger.x * 1600, event.tfinger.y * 720)){
        GameData::getInstance().setPaused(false);
        m_transitioning =true;
        LOGI("back to game state");
        Engine::Get().popState();
        if(GameData::getInstance().isDebugEnabled())
            Engine::Get().pushOverlayState(std::make_unique<DebugState>(m_renderer ,m_gameState));
        if(GameData::getInstance().getControlType() ==ControlType::JOYSTICK)
            Engine::Get().pushOverlayState(std::make_unique<JoystickOverlay>(m_renderer));
        if(GameData::getInstance().getControlType() ==ControlType::BUTTONS)
            Engine::Get().pushOverlayState(std::make_unique<ButtonOverlay>(m_renderer));
        if(GameData::getInstance().getControlType() ==ControlType::SEP_JUMP_W_JOYSTICK)
            Engine::Get().pushOverlayState(std::make_unique<SepJoysticknButton>(m_renderer));
        return true;
    }
    if(event.type == SDL_EVENT_KEY_DOWN){
        if(event.key.key == SDLK_AC_BACK){
            return true;
        }
    }

    return false;
}

PauseState::~PauseState() {
    LOGI("Pause destructor:%p",this);
    if(m_font)      TTF_CloseFont(m_font);
    if(m_fontShadow)TTF_CloseFont(m_fontShadow);

    if(m_fontfile)      SDL_CloseIO(m_fontfile);
    if(m_fontShadowfile)SDL_CloseIO(m_fontShadowfile);

    if(m_pauseTexture)      SDL_DestroyTexture(m_pauseTexture);
    if(m_pauseShadowTexture)SDL_DestroyTexture(m_pauseShadowTexture);
    if(m_exitTexture)       SDL_DestroyTexture(m_exitTexture);
    if(m_exitShadowTexture) SDL_DestroyTexture(m_exitShadowTexture);
}
