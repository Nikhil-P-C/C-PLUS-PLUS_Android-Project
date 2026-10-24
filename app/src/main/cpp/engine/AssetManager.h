//
// Created by LENOVO on 23-07-2026.
//
#pragma once
#include <SDL3/SDL.h>
#include "utils/utils.h"
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <array>
#include <atomic>
#include "HearingFx.h"
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
    BUTTON_UP_BUTTON,
    BUTTON_DOWN_BUTTON,
    BUTTON_SLASH_BUTTON,
    BUTTON_MAGIC_BUTTON,
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
    BG_GREENERY_SKY,
    BG_GREENERY_MOUNTAINS,
    BG_GREENERY_TREES,
    BG_MEADOWS_SKY,
    BG_MEADOWS_CLOUDS,
    BG_MEADOWS_FAR_PLAIN,
    BG_MEADOWS_FLOWER_FIELD,
    BG_CAVE_SKY,
    BG_CAVE_FAR_OBJECT1,
    BG_CAVE_FAR_OBJECT2,
    BG_CAVE_FAR_OBJECT3,
    BG_CAVE_FAR_OBJECT4,
    BG_CAVE_FAR_OBJECT5,
    BG_CAVE_FAR_LIGHT1,
    BG_CAVE_FAR_LIGHT2,
    BG_CAVE_FAR_LIGHT3,
    FG_GREENERY_NEAR_GRASS,
    FG_GREENERY_NEAR_OBJECT,
    FG_MEADOW_NEAR_FLOWERS,
    FG_CAVE_NEAR_OBJECT,

    ATTACK_PLAYER_SLASH,
    ATTACK_PLAYER_BEAM,
    // hit effects: VERTICAL strips, every frame is (texture width) x 32 px, art points RIGHT (mirrored in code)
    FX_SPARK_ENEMY,      // burst on an enemy hit
    FX_SPARK_FINISHER,   // ring burst on every 5th combo hit
    FX_SPARK_WALL,       // wall / ground / platform
    FX_SPARK_OBJECT,     // block / trap
    FX_THRUST_1,         // long scratch through the enemy
    FX_THRUST_2,         // shorter crossing scratch
    PLAYER_DOUBLE_JUMP,

    ENEMY_SPIDER_CHILD_PATROL,
    ENEMY_SPIDER_CHILD_SEEK,
    ENEMY_SPIDER_CHILD_ATTACK1,
    ENEMY_SPIDER_CHILD_ATTACK2,
    ENEMY_MOSS_GLOW_WORM_PATROL,
    ENEMY_MOSS_GLOW_WORM_SEEK,
    ENEMY_MOSS_GLOW_WORM_ATTACK1,
    ENEMY_MOSS_GLOW_WORM_ATTACK2,
    ENEMY_MOSS_BAT_PATROL,
    ENEMY_MOSS_BAT_SEEK,
    ENEMY_MOSS_BAT_ATTACK1,
    ENEMY_MOSS_BAT_ATTACK2,
    ENEMY_SINNERS_CHILD_PATROL,
    ENEMY_SINNERS_CHILD_SEEK,
    ENEMY_SINNERS_CHILD_ATTACK1,
    ENEMY_SINNERS_CHILD_ATTACK2,
    ENEMY_SINNERS_CHILD_DEAD,
    ENEMY_CAVE_PREDATOR_PATROL,
    ENEMY_CAVE_PREDATOR_SEEK,
    ENEMY_CAVE_PREDATOR_ATTACK1,
    ENEMY_CAVE_PREDATOR_ATTACK2,
    ENEMY_CAVE_RAT_PATROL,
    ENEMY_CAVE_RAT_SEEK,
    ENEMY_CAVE_RAT_ATTACK1,
    ENEMY_CAVE_RAT_ATTACK2,
    ENEMY_SHAFTS_RAPTOR_PATROL,
    ENEMY_SHAFTS_RAPTOR_SEEK,
    ENEMY_SHAFTS_RAPTOR_ATTACK1,
    ENEMY_SHAFTS_RAPTOR_ATTACK2,
    ENEMY_GIANT_BATS_PATROL,
    ENEMY_GIANT_BATS_SEEK,
    ENEMY_GIANT_BATS_ATTACK1,
    ENEMY_GIANT_BATS_ATTACK2,
    ENEMY_SHAFTS_WORKER_PATROL,
    ENEMY_SHAFTS_WORKER_SEEK,
    ENEMY_SHAFTS_WORKER_ATTACK1,
    ENEMY_SHAFTS_WORKER_ATTACK2,
    ENEMY_ANKYLOSAURUS_CHILD_PATROL,
    ENEMY_ANKYLOSAURUS_CHILD_SEEK,
    ENEMY_ANKYLOSAURUS_CHILD_ATTACK1,
    ENEMY_ANKYLOSAURUS_CHILD_ATTACK2,
    ENEMY_HUNTER_RAPTOR_PATROL,
    ENEMY_HUNTER_RAPTOR_SEEK,
    ENEMY_HUNTER_RAPTOR_ATTACK1,
    ENEMY_HUNTER_RAPTOR_ATTACK2,
    ENEMY_HUNTER_EAGLE_PATROL,
    ENEMY_HUNTER_EAGLE_SEEK,
    ENEMY_HUNTER_EAGLE_ATTACK1,
    ENEMY_HUNTER_EAGLE_ATTACK2,
    ENEMY_PREDATOR_THEROPOD_PATROL,
    ENEMY_PREDATOR_THEROPOD_SEEK,
    ENEMY_PREDATOR_THEROPOD_ATTACK1,
    ENEMY_PREDATOR_THEROPOD_ATTACK2,
    ENEMY_THEROPOD_CHILD_PATROL,
    ENEMY_THEROPOD_CHILD_SEEK,
    ENEMY_THEROPOD_CHILD_ATTACK1,
    ENEMY_THEROPOD_CHILD_ATTACK2,
    ENEMY_PREDATOR_CAT_PATROL,
    ENEMY_PREDATOR_CAT_SEEK,
    ENEMY_PREDATOR_CAT_ATTACK1,
    ENEMY_PREDATOR_CAT_ATTACK2,
    ENEMY_MOUNTAIN_CAT_PATROL,
    ENEMY_MOUNTAIN_CAT_SEEK,
    ENEMY_MOUNTAIN_CAT_ATTACK1,
    ENEMY_MOUNTAIN_CAT_ATTACK2,
    ENEMY_CROCODYLOMORPH_PATROL,
    ENEMY_CROCODYLOMORPH_SEEK,
    ENEMY_CROCODYLOMORPH_ATTACK1,
    ENEMY_CROCODYLOMORPH_ATTACK2,
    ENEMY_TARBOSAURUS_CHILD_PATROL,
    ENEMY_TARBOSAURUS_CHILD_SEEK,
    ENEMY_TARBOSAURUS_CHILD_ATTACK1,
    ENEMY_TARBOSAURUS_CHILD_ATTACK2,
    ENEMY_TYRANNOSAURUS_REX_CHILD_PATROL,
    ENEMY_TYRANNOSAURUS_REX_CHILD_SEEK,
    ENEMY_TYRANNOSAURUS_REX_CHILD_ATTACK1,
    ENEMY_TYRANNOSAURUS_REX_CHILD_ATTACK2,
    ENEMY_ALBERTOSAURUS_PATROL,
    ENEMY_ALBERTOSAURUS_SEEK,
    ENEMY_ALBERTOSAURUS_ATTACK1,
    ENEMY_ALBERTOSAURUS_ATTACK2,
    ENEMY_DASPELTOSAURUS_PATROL,
    ENEMY_DASPELTOSAURUS_SEEK,
    ENEMY_DASPELTOSAURUS_ATTACK1,
    ENEMY_DASPELTOSAURUS_ATTACK2,

    COUNT
};
enum class SoundType{
    SFX_FOOTSTEPS=0,
    SFX_ITEM_COLLECTED,
    SFX_ATTACK_SLASH_HIT,
    SFX_ATTACK_SLASH_COLLISION,
    SFX_ATTACK_BEAM,
    SFX_JUMP,
    SFX_LAND,
    SFX_UI_MOVE,      // menu selection moved
    SFX_UI_CONFIRM,   // generic confirm (spare)
    SFX_UI_ENTER,     // title screen -> menu
    SFX_UI_PLAY,      // Play pressed
    SFX_UI_OPEN,      // Options / Edit opened
    SFX_UI_QUIT,      // Quit pressed
    SFX_UI_INTRO,     // menu appears
    // surfaces: which one is used comes from the level (Z ... STEPS= / weather / biome), see setFootstepSound()
    SFX_FOOTSTEP_GRASS,
    SFX_FOOTSTEP_LEAVES,
    SFX_FOOTSTEP_SNOW,
    // environment one-shots (Sound/Ambient_Environment/oneshot)
    SFX_THUNDER,      // thunder_close_01, on every lightning strike
    SFX_WIND_GUST,    // wind_gust_01, when a gust starts
    MUSIC_BACKGROUND,
    COUNT
};
// Looping environment beds (rain + wind, Sound/Ambient_Environment/*_loop_01.ogg), streamed, one dedicated track each.
enum class AmbientLoop{
    RAIN = 0,       // rain_light_loop_01
    WIND,           // wind_light_loop_01
    COUNT
};
class AssetManager{
public:
    SDL_Texture* getTexture(TextureType type);
    MIX_Audio* getSound(SoundType type);
    TTF_Font* getFont(std::string fontName);

    // One-shot SFX. Uses a fixed pool of reusable tracks (no track is created per call).
    // If every voice is busy the sound is dropped instead of piling up.
    void playSound(SoundType);
    // listener = whoever is hearing it (the player), src = where the sound comes from.
    // `gain` lets distant/minor sources (enemy steps) sit below the player's own sounds.
    void playSoundAt(float listenerX,float listenerY,float srcX ,float srcY,SoundType,float gain =1.0f);

    // length of a loaded sound in milliseconds (0 if not loaded) - used to pace repeating sounds
    unsigned int getSoundDurationMs(SoundType type);

    // Plays `type` only if the previous play of that same sound has finished. Call it every
    // frame while the action is happening (footsteps, beam hum...): clips play back to back,
    // each for its full length, and never overlap or restart. Returns true if it started.
    bool playSoundIfIdle(SoundType type,float gain =1.0f);

    // --- environment beds ------------------------------------------------------------------------------
    // Sets the volume (0..1, before the SFX slider) of one looping bed. The first call with gain > 0 loads the
    // file and starts it; a gain of 0 stops it again (nothing keeps streaming for silence). Call every frame
    // with an already smoothed value, it only touches the mixer when the value changes.
    void setAmbientLoop(AmbientLoop id,float gain);
    void stopAmbientLoops();

    // The sound the player's / enemies' footsteps use on the current level. Anything that is not a footstep
    // sound, or whose file is missing, falls back to the concrete step.
    void setFootstepSound(SoundType type);
    SoundType getFootstepSound() const { return m_footstepSound; }

    // Looping music on one dedicated track (a new call replaces the previous music).
    void playMusic(SoundType);
    void playMusicAt(float listenerX,float listenerY,float srcX ,float srcY,SoundType);

    // --- streamed music files (menu music + one music file per level) ---------------------------
    // Streams `path` on the single music track, looping. Calling it again with the path that is already
    // playing does nothing (the music keeps going - e.g. when a level is restarted after dying).
    // An empty path or a file that can't be opened = silence. `baseGain` is a per-track volume trim
    // (level 'M' record); the player's music slider is applied on top.
    void playMusicFile(const std::string& path,float baseGain =1.0f);
    // The menu / title music. Only the menus play it - levels bring their own (or none).
    void playMenuMusic();
    // Like playMusicFile(), but if the file is missing/unreadable it returns false and leaves whatever is
    // playing UNTOUCHED (playMusicFile would go silent). Boss fights use it so a missing phase track falls
    // back to the previous track / the level music instead of silence.
    bool tryPlayMusicFile(const std::string& path,float baseGain =1.0f);
    // Fades the music track out over `ms` and then stops it (0 = stop at once).
    void fadeOutMusic(unsigned int ms);
    void stopMusic();
    // Music slider (0..1). Cheap: only touches the track when the value actually changed.
    void setMusicScale(float scale);

    // Master pitch/speed of everything you hear (music + sfx): 1.0 = normal, <1 = slower and lower.
    // Used for the "pitch drop" when the player gets hurt. Clamped to a sane range.
    void setPitchRatio(float ratio);

    // Cinematic damage audio on the whole mix (see HearingFx.h):
    //   muffle    0..1 : steep low-pass + small dark room reverb - the world goes heavy / out of focus (no pitch change)
    //   heartbeat 0..1 : "lub-dub" heartbeat (0 = off; starts on beat 1 when it turns on)
    // Both are smoothed on the audio thread, so you can set them every frame without clicks.
    void setHearingDamage(float muffle);
    void setHeartbeat(float level);
    // Synthesised low "boom" thump (no asset needed), strength 0..1.
    void playBoom(float strength);

    void loadTexture(TextureType type,const std::string_view& filepath);
    void loadSound(SoundType type,const std::string_view& filepath);

    void init(SDL_Renderer* renderer,MIX_Mixer* mixer);
    static constexpr int MAX_SFX_VOICES = 16;   // simultaneous one-shot sounds


    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;
    AssetManager()= default;
    ~AssetManager();
private:
    SDL_IOStream* m_fontFile         = SDL_IOFromFile("Fonts/Pixel Game.otf", "rb");

    std::array<SDL_Texture*,(unsigned int)TextureType::COUNT> m_textureTable{nullptr};
    std::array<MIX_Audio*,(unsigned int)SoundType::COUNT> m_soundTable{nullptr};
    std::array<TTF_Font*,5> m_fontTable{nullptr};

    SDL_Renderer* m_renderer= nullptr;
    MIX_Mixer* m_mixer =nullptr;

    // Tracks are owned by the mixer (freed by MIX_DestroyMixer) - never destroyed here.
    std::array<MIX_Track*,MAX_SFX_VOICES> m_sfxPool{nullptr};
    std::array<MIX_Track*,(unsigned int)SoundType::COUNT> m_idleTracks{nullptr}; // one per sound, for playSoundIfIdle
    SoundType m_footstepSound = SoundType::SFX_FOOTSTEPS;

    // environment beds: one streamed audio + one track each, created on first use
    static constexpr int AMBIENT_LOOP_COUNT = (int)AmbientLoop::COUNT;
    std::array<MIX_Audio*,AMBIENT_LOOP_COUNT> m_ambientAudio{nullptr};
    std::array<MIX_Track*,AMBIENT_LOOP_COUNT> m_ambientTrack{nullptr};   // owned by the mixer
    std::array<float,AMBIENT_LOOP_COUNT>      m_ambientGain{};
    std::array<bool,AMBIENT_LOOP_COUNT>       m_ambientPlaying{};
    std::array<bool,AMBIENT_LOOP_COUNT>       m_ambientFailed{};          // file missing: don't retry every frame

    MIX_Track* m_musicTrack =nullptr;
    MIX_Audio* m_musicFileAudio =nullptr;   // the streamed music file currently on m_musicTrack (owned here)
    std::string m_musicFilePath;
    float m_musicBaseGain =1.0f;

    // --- hearing-damage DSP (see HearingFx.h): runs on the AUDIO THREAD inside the MIX post-mix callback ---
    HearingFx m_hearingFx;
    static void SDLCALL postMixThunk(void* userdata, MIX_Mixer* mixer, const SDL_AudioSpec* spec, float* pcm, int samples);
    void postMix(const SDL_AudioSpec* spec, float* pcm, int samples);

    MIX_Track* acquireSfxTrack();
    void setTrackWorldPosition(MIX_Track* track,float listenerX,float listenerY,float srcX,float srcY);

};