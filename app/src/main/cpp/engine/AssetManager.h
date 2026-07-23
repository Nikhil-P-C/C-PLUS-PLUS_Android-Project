//
// Created by LENOVO on 23-07-2026.
//
#define pragma once
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <android/log.h>
#include <unordered_map>
#include <string>

#define LOG_TAG "AssetManager"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

class AssetManager{
public:
    SDL_Texture* getTexture(std::string textureName);
    MIX_Audio* getSound(std::string soundName);
    TTF_Font* getFont(std::string fontName);

    AssetManager(SDL_Renderer* renderer);
    ~AssetManager();
private:

    SDL_IOStream* m_yellowSkinFile   = SDL_IOFromFile("sheets/DinoSprites - tard.png","rb");
    SDL_IOStream* m_blueSkinFile     = SDL_IOFromFile("sheets/DinoSprites - doux.png","rb");
    SDL_IOStream* m_redSkinFile      = SDL_IOFromFile("sheets/DinoSprites - mort.png","rb");
    SDL_IOStream* m_greenSkinFile    = SDL_IOFromFile("sheets/DinoSprites - vita.png","rb");
    SDL_IOStream* m_tilesetSprite    = SDL_IOFromFile("Platforms/Terrain.png", "rb");
    SDL_IOStream* m_backGroundSprite = SDL_IOFromFile("Background/orig.png", "rb");
    SDL_IOStream* m_fontFile         = SDL_IOFromFile("Fonts/Pixel Game.otf", "rb");
    SDL_IOStream* m_backgroundFile = SDL_IOFromFile("misc/bg.png","rb");
    SDL_IOStream* m_editMenuFile = SDL_IOFromFile("menu/editMenu.png","rb");
    SDL_IOStream* m_nameTileFile = SDL_IOFromFile("menu/nameTile.png","rb");
    SDL_IOStream* m_previewTileFile = SDL_IOFromFile("menu/previewTile.png","rb");
    SDL_IOStream* m_skinTileFile = SDL_IOFromFile("menu/skinTile.png","rb");
    SDL_IOStream* m_leftChevronFile = SDL_IOFromFile("menu/Chevron-Arrow-Left.png","rb");
    std::unordered_map<std::string ,SDL_Texture*> m_textureTable;
    std::unordered_map<std::string ,MIX_Audio*> m_soundTable;
    std::unordered_map<std::string ,TTF_Font*> m_fontTable;
};