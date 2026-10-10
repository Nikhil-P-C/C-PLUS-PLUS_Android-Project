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

MIX_Audio *AssetManager::getSound(SoundType type){
    if(m_soundTable[(unsigned int)type]) {
        return m_soundTable[(unsigned int) type];
    }
    LOGI("the type that failed : %d",(int)type);
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

void AssetManager::loadSound(SoundType type, const std::string_view &filepath) {
    if(type >=SoundType::COUNT){
        LOGI("out of bound");
        return;
    }
    if(!m_mixer){
        LOGI("loadSound called before the mixer exists");
        return;
    }
    SDL_IOStream* ioStream =SDL_IOFromFile(filepath.data(),"rb");
    if(!ioStream){
        LOGI("failed to open sound:%s (%s)",filepath.data(),SDL_GetError());
        return;
    }
    // Short SFX are decoded once at load time so playing them costs nothing per call (the old
    // code decoded the file again on every play). Music stays compressed in RAM - it is big.
    // closeio=true: SDL_mixer closes the stream itself, on success and on failure.
    const bool predecode = (type != SoundType::MUSIC_BACKGROUND);
    m_soundTable[(unsigned int)type] = MIX_LoadAudio_IO(m_mixer,ioStream,predecode,true);
    if(!m_soundTable[(unsigned int)type]){
        LOGI("failed to load sound:%s (%s)",filepath.data(),SDL_GetError());
    }
}

// Listener is the player; SDL_mixer's listener is fixed at (0,0,0) and works in "units", not
// pixels - a source 800px away would otherwise be 800 units away and basically silent.
static constexpr float SFX_PIXELS_PER_UNIT = 200.0f;

void AssetManager::setTrackWorldPosition(MIX_Track* track,float listenerX,float listenerY,float srcX,float srcY){
    MIX_Point3D position{
            (srcX - listenerX) / SFX_PIXELS_PER_UNIT,
            -(srcY - listenerY) / SFX_PIXELS_PER_UNIT,   // screen Y grows down, mixer Y grows up
            0.0f
    };
    MIX_SetTrack3DPosition(track,&position);
}

MIX_Track* AssetManager::acquireSfxTrack() {
    if(!m_mixer) return nullptr;
    for(auto& track : m_sfxPool){
        if(!track){
            track = MIX_CreateTrack(m_mixer);   // created lazily, once, then reused forever
            if(!track){
                LOGI("MIX_CreateTrack failed:%s",SDL_GetError());
                return nullptr;
            }
            return track;
        }
        if(!MIX_TrackPlaying(track)) return track;
    }
    return nullptr;   // all voices busy - drop this sound
}

void AssetManager::playSound(SoundType type) {
    if(type >=SoundType::COUNT || !m_soundTable[(unsigned int)type]) return;
    MIX_Track* track = acquireSfxTrack();
    if(!track) return;
    MIX_SetTrackAudio(track, m_soundTable[(unsigned int)type]);
    MIX_SetTrack3DPosition(track,nullptr);   // pooled track may still be positional from last use
    MIX_SetTrackGain(track,1.0f);
    MIX_PlayTrack(track,0);                  // options is an SDL_PropertiesID: 0 = none (was `true` = bogus ID 1)
}

void AssetManager::playSoundAt(float listenerX, float listenerY, float srcX, float srcY, SoundType type, float gain) {
    if(type >=SoundType::COUNT || !m_soundTable[(unsigned int)type]) return;
    MIX_Track* track = acquireSfxTrack();
    if(!track) return;
    MIX_SetTrackAudio(track, m_soundTable[(unsigned int)type]);
    setTrackWorldPosition(track,listenerX,listenerY,srcX,srcY);
    MIX_SetTrackGain(track,gain);
    MIX_PlayTrack(track,0);
}

unsigned int AssetManager::getSoundDurationMs(SoundType type) {
    if(type >=SoundType::COUNT || !m_soundTable[(unsigned int)type]) return 0;
    MIX_Audio* audio = m_soundTable[(unsigned int)type];
    Sint64 frames = MIX_GetAudioDuration(audio);
    if(frames < 0) return 0;   // MIX_DURATION_UNKNOWN / infinite
    Sint64 ms = MIX_AudioFramesToMS(audio,frames);
    return ms > 0 ? (unsigned int)ms : 0;
}

bool AssetManager::playSoundIfIdle(SoundType type,float gain) {
    if(!m_mixer || type >=SoundType::COUNT) return false;
    MIX_Audio* audio = m_soundTable[(unsigned int)type];
    if(!audio) return false;

    MIX_Track*& track = m_idleTracks[(unsigned int)type];
    if(!track){
        track = MIX_CreateTrack(m_mixer);
        if(!track){
            LOGI("MIX_CreateTrack failed:%s",SDL_GetError());
            return false;
        }
    }
    else if(MIX_TrackPlaying(track)){
        return false;   // previous clip still playing - let it finish, don't restart/overlap
    }

    MIX_SetTrackAudio(track,audio);
    MIX_SetTrack3DPosition(track,nullptr);
    MIX_SetTrackGain(track,gain);
    return MIX_PlayTrack(track,0);
}

void AssetManager::playMusic(SoundType type) {
    if(type >=SoundType::COUNT || !m_soundTable[(unsigned int)type] || !m_mixer) return;
    if(!m_musicTrack){
        m_musicTrack = MIX_CreateTrack(m_mixer);
        if(!m_musicTrack){
            LOGI("MIX_CreateTrack failed:%s",SDL_GetError());
            return;
        }
    }
    MIX_StopTrack(m_musicTrack,0);   // replace whatever music was playing
    MIX_SetTrackAudio(m_musicTrack, m_soundTable[(unsigned int)type]);
    MIX_SetTrack3DPosition(m_musicTrack,nullptr);
    MIX_SetTrackGain(m_musicTrack,GameData::getInstance().getMusicAudioScale());
    MIX_SetTrackLoops(m_musicTrack,-1);
    MIX_PlayTrack(m_musicTrack,0);
}

void AssetManager::playMusicAt(float listenerX, float listenerY, float srcX, float srcY, SoundType type) {
    playMusic(type);
    if(m_musicTrack) setTrackWorldPosition(m_musicTrack,listenerX,listenerY,srcX,srcY);
}

AssetManager::~AssetManager()
{
    for(const auto& texture:m_textureTable){
        SDL_DestroyTexture(texture);
    }
    LOGI("all texture destroyed");
    for(auto& audio:m_soundTable){
        if(audio) MIX_DestroyAudio(audio);
        audio = nullptr;
    }

}

void AssetManager::init(SDL_Renderer *renderer,MIX_Mixer* mixer) {

    if(!renderer) {
        LOGI("(AssetManger)render is null");
        return;
    }
    if(!mixer){
        LOGI("(AssetManger)render is null");
        return;
    }

    m_mixer =mixer;
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
    loadTexture(TextureType::BUTTON_SLASH_BUTTON,"button/slash_button.png");
    loadTexture(TextureType::BUTTON_MAGIC_BUTTON,"button/magic_button.png");
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
    loadTexture(TextureType::BG_GREENERY_SKY,"Background/Greenery/greenery_sky.png");
    loadTexture(TextureType::BG_GREENERY_MOUNTAINS,"Background/Greenery/greenery_mountains.png");
    loadTexture(TextureType::BG_GREENERY_TREES,"Background/Greenery/greenery_trees.png");
    loadTexture(TextureType::FG_GREENERY_NEAR_GRASS,"ForeGround/Greenery/greenery_neargrass.png");
    loadTexture(TextureType::FG_GREENERY_NEAR_OBJECT,"ForeGround/Greenery/greenery_nearobjects.png");
    loadTexture(TextureType::BG_MEADOWS_SKY,"Background/Meadows/meadows_sky.png");
    loadTexture(TextureType::BG_MEADOWS_CLOUDS,"Background/Meadows/meadows_cloud.png");
    loadTexture(TextureType::BG_MEADOWS_FAR_PLAIN,"Background/Meadows/meadows_farplain.png");
    loadTexture(TextureType::BG_MEADOWS_FLOWER_FIELD,"Background/Meadows/meadows_flowerfield.png");
    loadTexture(TextureType::FG_MEADOW_NEAR_FLOWERS,"ForeGround/Meadows/meadows_foreground.png");
    loadTexture(TextureType::BG_CAVE_SKY,"Background/Cave/cave_sky.png");
    loadTexture(TextureType::BG_CAVE_FAR_LIGHT1,"Background/Cave/far_light1.png");
    loadTexture(TextureType::BG_CAVE_FAR_LIGHT2,"Background/Cave/far_light2.png");
    loadTexture(TextureType::BG_CAVE_FAR_LIGHT3,"Background/Cave/far_light3.png");
    loadTexture(TextureType::BG_CAVE_FAR_OBJECT1,"Background/Cave/far_object1.png");
    loadTexture(TextureType::BG_CAVE_FAR_OBJECT2,"Background/Cave/far_object2.png");
    loadTexture(TextureType::BG_CAVE_FAR_OBJECT3,"Background/Cave/far_object3.png");
    loadTexture(TextureType::BG_CAVE_FAR_OBJECT4,"Background/Cave/far_object4.png");
    loadTexture(TextureType::BG_CAVE_FAR_OBJECT5,"Background/Cave/far_object5.png");
    loadTexture(TextureType::FG_CAVE_NEAR_OBJECT,"ForeGround/Cave/near_object.png");

    loadTexture(TextureType::PLAYER_DOUBLE_JUMP, "Skill/broken_wings.png");
    loadTexture(TextureType::ATTACK_PLAYER_SLASH,"Attacks/slash.png");
    loadTexture(TextureType::ATTACK_PLAYER_BEAM,"Attacks/beam_sprite.png");

    loadTexture(TextureType::ENEMY_SINNERS_CHILD_PATROL,"Enemy/sinner_child/sinner_child_patrol.png");
    loadTexture(TextureType::ENEMY_SINNERS_CHILD_SEEK,"Enemy/sinner_child/sinner_child_seek.png");
    loadTexture(TextureType::ENEMY_SINNERS_CHILD_DEAD,"Enemy/sinner_child/sinner_child_death.png");

    loadSound(SoundType::SFX_FOOTSTEPS,"Sound/footstep/concrete_step.wav");

    for(const auto & texture: m_textureTable){
        SDL_SetTextureScaleMode(texture,SDL_SCALEMODE_NEAREST);
    }
}


