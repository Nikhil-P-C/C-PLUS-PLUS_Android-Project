//
// Created by LENOVO on 23-07-2026.
//
#include "AssetManager.h"
#include <SDL3_image/SDL_image.h>
SDL_Texture* AssetManager::getTexture(TextureType type){

    if(m_textureTable[(unsigned int)type]) {
        return m_textureTable[(unsigned int) type];
    }
    LOGI("the type that failed : %d",(int)type);
    return nullptr;
}

MIX_Audio *AssetManager::getSound(std::string soundName){
    //TODO implement getSound
    return nullptr;
}

TTF_Font *AssetManager::getFont(std::string fontName){
    //TODO implement getFont
    return nullptr;
}

void AssetManager::loadTexture(TextureType type, const std::string_view& filepath){
    if(type >=TextureType::COUNT){
        LOGI("out of bound");
        return;
    }
    SDL_IOStream* ioStream =SDL_IOFromFile(filepath.data(),"rb");
    SDL_Surface* surface = IMG_Load_IO(ioStream, false);
    m_textureTable[(unsigned int)type] = SDL_CreateTextureFromSurface(m_renderer,surface);
    SDL_CloseIO(ioStream);
    SDL_DestroySurface(surface);
    if(m_textureTable[(unsigned int)type] == nullptr)
    {
        LOGI("failed to load texture:%s",filepath.data());
    }

}


AssetManager::~AssetManager()
{
    for(const auto& texture:m_textureTable){
        SDL_DestroyTexture(texture);
    }
    LOGI("all texture destroyed");

}

void AssetManager::init(SDL_Renderer *renderer) {

    if(!renderer) {
        LOGI("(AssetManger)render is null");
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
    loadTexture(TextureType::DUST_PARTICLE,"Particles/Dust Particle.png");
    loadTexture(TextureType::FRUIT_BANANA,"Fruits/Bananas.png");
    loadTexture(TextureType::FRUIT_APPLE,"Fruits/Apple.png");
    loadTexture(TextureType::FRUIT_ORANGE,"Fruits/Orange.png");
    loadTexture(TextureType::FRUIT_STRAWBERRY,"Fruits/Strawberry.png");
    loadTexture(TextureType::FRUIT_COLLECTED,"Fruits/Collected.png");
    loadTexture(TextureType::TRAP_FALLING_PLATFORM_OFF,"Traps/Falling Platforms/Off.png");
    loadTexture(TextureType::TRAP_FALLING_PLATFORM_ON,"Traps/Falling Platforms/On (32x10).png");
    loadTexture(TextureType::TRAP_FAN_OFF,"Traps/Fan/Off.png");
    loadTexture(TextureType::TRAP_FAN_ON,"Traps/Fan/On (24x8).png");
    loadTexture(TextureType::TRAP_FIRE_HIT,"Traps/Fire/Hit (16x32).png");
    loadTexture(TextureType::TRAP_FIRE_OFF,"Traps/Fire/Off.png");
    loadTexture(TextureType::TRAP_FIRE_ON,"Traps/Fire/On (16x32).png");
    loadTexture(TextureType::TRAP_PLATFORM_BROWN_OFF,"Traps/Platforms/Brown Off.png");
    loadTexture(TextureType::TRAP_PLATFORM_BROWN_ON,"Traps/Platforms/Brown On (32x8).png");
    loadTexture(TextureType::TRAP_PLATFORM_CHAIN,"Traps/Platforms/Chain.png");
    loadTexture(TextureType::TRAP_PLATFORM_GREY_OFF,"Traps/Platforms/Grey Off.png");
    loadTexture(TextureType::TRAP_PLATFORM_GREY_ON,"Traps/Platforms/Grey On (32x8).png");
    loadTexture(TextureType::TRAP_ROCK_HEAD_BLINK,"Traps/Rock Head/Blink (42x42).png");
    loadTexture(TextureType::TRAP_ROCK_HEAD_HIT_BOTTOM,"Traps/Rock Head/Bottom Hit (42x42).png");
    loadTexture(TextureType::TRAP_ROCK_HEAD_HIT_LEFT,"Traps/Rock Head/Left Hit (42x42).png");
    loadTexture(TextureType::TRAP_ROCK_HEAD_HIT_RIGHT,"Traps/Rock Head/Right Hit (42x42).png");
    loadTexture(TextureType::TRAP_ROCK_HEAD_HIT_TOP,"Traps/Rock Head/Top Hit (42x42).png");
    loadTexture(TextureType::TRAP_SPIKE_HEAD_BLINK,"Traps/Spike Head/Blink (54x52).png");
    loadTexture(TextureType::TRAP_SPIKE_HEAD_HIT_BOTTOM, "Traps/Spike Head/Bottom Hit (54x52).png");
    loadTexture(TextureType::TRAP_SPIKE_HEAD_HIT_LEFT,"Traps/Spike Head/Left Hit (54x52).png");
    loadTexture(TextureType::TRAP_SPIKE_HEAD_HIT_RIGHT,"Traps/Spike Head/Right Hit (54x52).png");
    loadTexture(TextureType::TRAP_SPIKE_HEAD_HIT_TOP,"Traps/Spike Head/Top Hit (54x52).png");
    loadTexture(TextureType::TRAP_SPIKE_BALL_CHAIN,"Traps/Spiked Ball/Chain.png");
    loadTexture(TextureType::TRAP_SPIKE_BALL,"Traps/Spiked Ball/Spiked Ball.png");
    loadTexture(TextureType::TRAP_SAW_CHAIN,"Traps/Saw/Chain.png");
    loadTexture(TextureType::TRAP_SAW_OFF,"Traps/Saw/Off.png");
    loadTexture(TextureType::TRAP_SAW_ON,"Traps/Saw/On (38x38).png");
    loadTexture(TextureType::TRAP_SPIKE,"Traps/Spikes/Idle.png");
    loadTexture(TextureType::TRAP_TRAMPOLINE,"Traps/Trampoline/Idle.png");
    loadTexture(TextureType::TRAP_TRAMPOLINE_TRIGGER,"Traps/Trampoline/Jump (28x28).png");
    loadTexture(TextureType::HUD_HEALTH_HEART,"HUD/heart-health.png");
    loadTexture(TextureType::CHECKPOINT_FLAG_IDLE,"CheckPoint/Checkpoint (Flag Idle)(64x64).png");
    loadTexture(TextureType::CHECKPOINT_FLAG_OUT,"CheckPoint/Checkpoint (Flag Out) (64x64).png");
    loadTexture(TextureType::CHECKPOINT_FLAG_NO,"CheckPoint/Checkpoint (No Flag).png");
    loadTexture(TextureType::TRANSITION,"misc/Transition.png");
    for(const auto & texture: m_textureTable){
        SDL_SetTextureScaleMode(texture,SDL_SCALEMODE_NEAREST);
    }
}


