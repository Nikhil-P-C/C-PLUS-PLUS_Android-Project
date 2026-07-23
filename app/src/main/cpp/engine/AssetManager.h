//
// Created by LENOVO on 23-07-2026.
//
#define pragma once
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <android/log.h>
#include <array>
#include <string>

#define LOG_TAG "AssetManager"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
enum class TextureType{
    YELLOW_SKIN=0,
    BLUE_SKIN,
    RED_SKIN,
    GREEN_SKIN,
    TILESET_SPRITE,
    BG_SKY,
    BG_CONFETTI,
    BG_CONFETTI_ANIMATED,
    EDIT_MENU_TILE,
    EDIT_NAME_TILE,
    EDIT_PREVIEW_TILE,
    EDIT_SKIN_TILE,
    EDIT_LEFT_CHEVRON,
    BUTTON_JUMP_BUTTON,
    BUTTON_LEFT_BUTTON,
    BUTTON_RIGHT_BUTTON,
    JOYSTICK_JOYSTICK_OUTERRING,
    JOYSTICK_JOYSTICK_HANDLE,
    MENU_BUTTONS,
    OPTION_MENU_TILE,
    MENU_OPTION_BLOCK_TILE,
    MENU_SLIDEBAR,
    MENU_SLIDER,
    MENU_RADIO_BUTTON,
    COUNT
};
class AssetManager{
public:
    SDL_Texture* getTexture(TextureType type);
    MIX_Audio* getSound(std::string soundName);
    TTF_Font* getFont(std::string fontName);
    void loadTexture(TextureType type,const std::string& filepath);
    AssetManager(SDL_Renderer* renderer);
    ~AssetManager();
private:
    SDL_IOStream* m_fontFile         = SDL_IOFromFile("Fonts/Pixel Game.otf", "rb");

    std::array<SDL_Texture*,(unsigned int)TextureType::COUNT> m_textureTable{nullptr};
    std::array<MIX_Audio*,5> m_soundTable{nullptr};
    std::array<TTF_Font*,5> m_fontTable{nullptr};

    SDL_Renderer* m_renderer= nullptr;
};