//
// Created by LENOVO on 23-07-2026.
//
#include "AssetManager.h"
#include <SDL3_image/SDL_image.h>
SDL_Texture* AssetManager::getTexture(TextureType type)
{
    if(m_textureTable[(unsigned int)type])
        return m_textureTable[(unsigned int)type];

    return nullptr;
}

MIX_Audio *AssetManager::getSound(std::string soundName)
{
    return nullptr;
}

TTF_Font *AssetManager::getFont(std::string fontName)
{
    return nullptr;
}

void AssetManager::loadTexture(TextureType type, const std::string &filepath)
{
    LOGI("yo :%s",filepath.c_str());
    if(type >TextureType::COUNT)
    {
        LOGI("out of bound");
    }
    SDL_IOStream* ioStream =SDL_IOFromFile(filepath.c_str(),"rb");
    SDL_Surface* surface = IMG_Load_IO(ioStream, false);
    m_textureTable[(unsigned int)type] = SDL_CreateTextureFromSurface(m_renderer,surface);
    if(m_textureTable[(unsigned int)type] == nullptr)
    {
        LOGI("failed to load texture:%s",filepath.c_str());
    }

}
AssetManager::AssetManager(SDL_Renderer *renderer)
{
    if(!renderer) {
        LOGI("render is null");
        return;
    }
    m_renderer =renderer;
    loadTexture(TextureType::YELLOW_SKIN,"sheets/DinoSprites - tard.png");
    loadTexture(TextureType::BLUE_SKIN,"sheets/DinoSprites - doux.png");
    loadTexture(TextureType::RED_SKIN,"sheets/DinoSprites - mort.png");
    loadTexture(TextureType::GREEN_SKIN,"sheets/DinoSprites - vita.png");
    loadTexture(TextureType::TILESET_SPRITE,"Platforms/Terrain.png");
    loadTexture(TextureType::BG_SKY,"Background/orig.png");
    loadTexture(TextureType::BG_CONFETTI,"misc/bg.png");
    loadTexture(TextureType::BG_CONFETTI_ANIMATED,"menu-animate.png");
    loadTexture(TextureType::EDIT_MENU_TILE,"menu/editMenu.png");
    loadTexture(TextureType::EDIT_NAME_TILE,"menu/nameTile.png");
    loadTexture(TextureType::EDIT_PREVIEW_TILE,"menu/previewTile.png");
    loadTexture(TextureType::EDIT_SKIN_TILE,"menu/skinTile.png");
    loadTexture(TextureType::EDIT_LEFT_CHEVRON,"menu/Chevron-Arrow-Left.png");
    loadTexture(TextureType::BUTTON_JUMP_BUTTON,"button/jump_button.png");
    loadTexture(TextureType::BUTTON_LEFT_BUTTON,"button/left_button.png");
    loadTexture(TextureType::BUTTON_RIGHT_BUTTON,"button/right_button.png");
    loadTexture(TextureType::JOYSTICK_JOYSTICK_OUTERRING,"joystick/Joystick.png");
    loadTexture(TextureType::JOYSTICK_JOYSTICK_HANDLE,"joystick/SmallHandleFilledGrey.png");
    loadTexture(TextureType::MENU_BUTTONS,"button/buttons.png");
    loadTexture(TextureType::OPTION_MENU_TILE, "menu/optionMenu.png");
    loadTexture(TextureType::MENU_OPTION_BLOCK_TILE, "menu/menutile.png");
    loadTexture(TextureType::MENU_SLIDEBAR,"menu/slidebar.png");
    loadTexture(TextureType::MENU_SLIDER,"menu/slider.png");
    loadTexture(TextureType::MENU_RADIO_BUTTON,"menu/radioButton.png");




}

AssetManager::~AssetManager()
{

}


