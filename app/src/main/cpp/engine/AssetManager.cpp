//
// Created by LENOVO on 23-07-2026.
//
#include "AssetManager.h"
#include <SDL3_image/SDL_image.h>
SDL_Texture *AssetManager::getTexture(std::string textureName) {
    if(m_textureTable["yellowSkin"])
        return m_textureTable["yellowSkin"];

    return nullptr;
}

MIX_Audio *AssetManager::getSound(std::string soundName) {
    return nullptr;
}

TTF_Font *AssetManager::getFont(std::string fontName) {
    return nullptr;
}

AssetManager::AssetManager(SDL_Renderer *renderer) {
    if(!renderer)
        LOGI("render is null");
    SDL_Surface* yellowSkinSurface = IMG_Load_IO(m_yellowSkinFile,false);
    m_textureTable["yellowSkin"]= SDL_CreateTextureFromSurface(renderer, yellowSkinSurface);
    SDL_CloseIO(m_yellowSkinFile);
    SDL_DestroySurface(yellowSkinSurface);
}

AssetManager::~AssetManager() {

}

