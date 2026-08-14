//
// Created by LENOVO on 23-07-2026.
//
#pragma once
#include <SDL3/SDL.h>
#include "utils/utils.h"
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <array>
#include <string>

#define LOG_TAG "AssetManager"

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
    DUST_PARTICLE,
    FRUIT_BANANA,
    FRUIT_APPLE,
    FRUIT_ORANGE,
    FRUIT_STRAWBERRY,
    FRUIT_COLLECTED,
    TRAP_FALLING_PLATFORM_ON,
    TRAP_FALLING_PLATFORM_OFF,
    TRAP_FAN_ON,
    TRAP_FAN_OFF,
    TRAP_FIRE_HIT,
    TRAP_FIRE_ON,
    TRAP_FIRE_OFF,
    TRAP_PLATFORM_BROWN_OFF,
    TRAP_PLATFORM_BROWN_ON,
    TRAP_PLATFORM_GREY_OFF,
    TRAP_PLATFORM_GREY_ON,
    TRAP_PLATFORM_CHAIN,
    TRAP_ROCK_HEAD_BLINK,
    TRAP_ROCK_HEAD_HIT_BOTTOM,
    TRAP_ROCK_HEAD_HIT_TOP,
    TRAP_ROCK_HEAD_HIT_RIGHT,
    TRAP_ROCK_HEAD_HIT_LEFT,
    TRAP_SPIKE_HEAD_BLINK,
    TRAP_SPIKE_HEAD_HIT_BOTTOM,
    TRAP_SPIKE_HEAD_HIT_TOP,
    TRAP_SPIKE_HEAD_HIT_RIGHT,
    TRAP_SPIKE_HEAD_HIT_LEFT,
    TRAP_SPIKE_BALL,
    TRAP_SPIKE_BALL_CHAIN,
    TRAP_SAW_OFF,
    TRAP_SAW_ON,
    TRAP_SAW_CHAIN,
    TRAP_SPIKE,
    TRAP_TRAMPOLINE,
    TRAP_TRAMPOLINE_TRIGGER,
    HUD_HEALTH_HEART,
    CHECKPOINT_FLAG_NO,
    CHECKPOINT_FLAG_OUT,
    CHECKPOINT_FLAG_IDLE,
    TRANSITION,
    COUNT
};
class AssetManager{
public:
    SDL_Texture* getTexture(TextureType type);
    MIX_Audio* getSound(std::string soundName);
    TTF_Font* getFont(std::string fontName);
    void loadTexture(TextureType type,const std::string_view& filepath);
    void init(SDL_Renderer* renderer);
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;
    AssetManager()= default;
    ~AssetManager();
private:
    SDL_IOStream* m_fontFile         = SDL_IOFromFile("Fonts/Pixel Game.otf", "rb");

    std::array<SDL_Texture*,(unsigned int)TextureType::COUNT> m_textureTable{nullptr};
    std::array<MIX_Audio*,5> m_soundTable{nullptr};
    std::array<TTF_Font*,5> m_fontTable{nullptr};

    SDL_Renderer* m_renderer= nullptr;
};