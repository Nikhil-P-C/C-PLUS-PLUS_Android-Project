//
// Created by LENOVO on 17-04-2026.
//

#pragma once
#include <SDL3/SDL.h>

#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>

#include "State/State.h"
#include "engine/Engine.h"
#include "utils/utils.h"
#include "level/GroundShapeBuilder.h"
#include "ParticleSystem.h"
#include "AmbientParticles.h"
#include "engine/Lighting.h"

#include "level/Fruits.h"
#include "level/Traps.h"
#include "level/Enemies.h"
#include "level/Interactables.h"
#include "level/BlockShapeBuilder.h"
#include "level/CheckPoint.h"
#include "level/BackGroundBuilder.h"
#include "level/ForeGroundBuilder.h"
#include "level/LevelLoader/LevelLoader.h"

#define LOG_TAG "GameState"

const float SCALE = 4;
const int TILE_SIZE =16;
const int HURT_ANIM_MS =300;
const int KNOCKBACK_MS =300;

const int DOUBLE_JUMP_FRAME_W =30;
const int DOUBLE_JUMP_FRAME_H =30;
const int DOUBLE_JUMP_LAST_FRAME =5; // 6 frames: 0-5

const float MAGIC_REGEN_PER_SEC = 8.0f;      // only ticks while not beaming
const float BEAM_MAGIC_DRAIN_PER_SEC = 25.0f;
const float HEAL_MAGIC_COST = 30.0f;
const int HEAL_AMOUNT = 1;
const unsigned int HEAL_COOLDOWN_MS = 800;
const int ENEMY_KILL_SCORE = 10;

// --- directional slash / pogo / pushback ---
const float POGO_VELOCITY          = 760.0f;    // px/s upward after a downward slash lands on an enemy / hazard trap / rock head
// (~160px rise = ~58% of a full jump, short + snappy like Hollow Knight's nail bounce)
const float UP_SLASH_PUSH_SPEED    = 320.0f;    // px/s downward shove when an upward slash connects
const float VERT_SLASH_REACH       = 122.5f;    // how far past the player's body the up/down slash reaches

// --- hit VFX / combo ---
const unsigned int COMBO_WINDOW_MS   = 1200;   // a hit within this long of the previous one continues the combo
const int          COMBO_FINISHER_EVERY = 5;   // every Nth combo hit is a "finisher" beat (bigger shake/stop/knockback)
// lighter than a trap hit (600px/s, 300ms) - just a firm nudge so it doesn't feel like a launch
const float PLAYER_ENEMY_KNOCKBACK_SPEED = 220.0f;
const unsigned int PLAYER_ENEMY_KNOCKBACK_MS = 150;

// player death (Mario-style: freeze on the white hurt frame, pop up, tumble off the bottom, respawn)
const float PLAYER_DEATH_WHITE_S    = 0.12f;    // white silhouette frame (sheet frame 15) at the start of the freeze
const float PLAYER_DEATH_FREEZE_S   = 0.40f;    // total hold before the pop
const float PLAYER_DEATH_POP_SPEED  = 900.0f;   // px/s upward
const float PLAYER_DEATH_SPIN_DEG_S = 420.0f;
const float PLAYER_DEATH_TOTAL_S    = 2.00f;    // time from the fatal hit until the respawn screen opens

// jump forgiveness (seconds)
const float COYOTE_TIME_S           = 0.10f;    // can still make the first (full) jump this long after walking off a ledge
const float JUMP_BUFFER_S           = 0.12f;    // a jump pressed this long before landing fires on touchdown
const float ATTACK_BUFFER_S         = 0.15f;    // an attack pressed during a swing is queued and starts the moment the swing ends
// power dash: a straight horizontal lunge with no gravity. One dash per airtime - it refills on touching the ground or on a pogo.
const float DASH_SPEED              = 1300.0f;  // px/s while dashing
const float DASH_DURATION_S         = 0.18f;    // ~234px lunge
const float DASH_COOLDOWN_S         = 0.20f;    // short lockout after a dash so a grounded dash can't be chained instantly
const float JUMP_CUT_GRAVITY_MULT   = 3.0f;     // gravity multiplier while rising with the jump button released (variable jump height)
// full hold ~278px, quick tap ~92px, anything in between depending on release time
// wall jump: only off REAL walls (the 'W' level walls, grounds, blocks) - never off the invisible world bounds
const float WALL_PROBE_W            = 6.0f;     // how far out from the player's side a wall still counts as touching
const float WALL_COYOTE_S           = 0.10f;    // can still wall-jump this long after leaving the wall
const float WALL_JUMP_VELOCITY_MULT = 0.95f;    // x the normal jump velocity
const float WALL_JUMP_SPEED_X       = 520.0f;   // px/s kick away from the wall
const float WALL_JUMP_LOCK_S        = 0.16f;    // horizontal input is ignored this long so the kick actually plays (~83px)
const float WALL_SLIDE_SPEED        = 220.0f;   // max fall speed while holding toward a wall (set to 100000 to disable sliding)
const int   PLAYER_DEATH_FRAME_WHITE = 15;      // hurt row is 14-16, 15 is the all-white flash frame
const int   PLAYER_DEATH_FRAME_POSE  = 14;

// which way the current melee swing points. Locked in when the swing starts.
enum class SlashDir{
    SIDE =0,   // the way the player faces
    UP,        // attack + up held
    DOWN       // attack + down held while airborne (this is the pogo swing)
};

enum PlayerAction{
    IDLE =0,
    MOVINGLEFT=1,
    MOVINGRIGHT=2,
    CROUCHING=3,
    HURT=4,
    JUMP=5,
    ATTACK=6,
};
struct PrecomputedWallTile{
    SDL_FRect src;
    float x;
    float y;
    bool flip;
};
class GameState : public State{
public:
    GameState(SDL_Renderer* renderer,int level);
    ~GameState() override{
        Engine::Get().getAssetManager().setPitchRatio(1.0f);          // never leave the audio muffled/slowed
        Engine::Get().getAssetManager().setHearingDamage(0.0f);
        Engine::Get().getAssetManager().setHeartbeat(0.0f);
        LOGI("gamestate destructor:%p",this);
        if (m_font)            TTF_CloseFont(m_font);
        if (m_playerNameTextue)SDL_DestroyTexture(m_playerNameTextue);
        if (m_comboTexture)    SDL_DestroyTexture(m_comboTexture);
        if (m_fontFile)        SDL_CloseIO(m_fontFile);
        SDL_SetTextureAlphaMod(m_playerTexture,255);
        SDL_RemoveEventWatch(HandleBackgroundEvents, this);
    }
    void buildWallTiles();
    bool hasWallAbove(float x,float y);
    bool hasWallBelow(float x,float y);
    bool hasWallRight(float x,float y);
    bool hasWallLeft(float x,float y);
    void setLevel(int level);

    void handleCollision();
    // wall jump helpers. -1 = real wall touching the player's left, +1 = right, 0 = none
    int  getWallContactDir() const;
    bool isWallJumpSurface(const SDL_FRect& probe) const;
    bool pointInLevelWall(float x, float y) const;
    void handlePhysicAndInput(float dt);
    void startPlayerDeath();
    void updateDeath(float dt);
    void respawnPlayer();
    // death animation finished -> show the respawn screen (RespawnState). RESPAWN there calls completeRespawn().
    void openRespawnScreen();
    void completeRespawn();
    bool hasReachedCheckpoint() const {
        return m_checkPoint.aniType == CheckPointAni::FLAG_OUT || m_checkPoint.aniType == CheckPointAni::FLAG_IDLE;
    }
    void flushEnemyDeathEffects();
    // hit VFX: sparks, screen shake, hit-stop, combo (fed by EnemiesBuilder::consumeHitEvents)
    void flushEnemyHitEffects();
    void onEnemyHit(const EnemyHitEvent& e);
    void requestHitStop(float seconds);
    // pushes the player away from what they just hit (dirX = -facing). A hurt-knockback in progress wins.
    void applyRecoil(float dirX, float speed);
    // vertical pushback: dirY = +1 shoves the player down, -1 up. Same hurt-knockback guard as applyRecoil.
    void applyRecoilY(float dirY, float speed);
    // downward slash landed on something bounceable: launch the player upward and refill an air jump
    void startPogo();
    // ends a dash in progress (wall hit, pogo, knockback, death). Gravity takes over again immediately.
    void endDash();
    // player took damage: big impact frame + the same hit spark made larger + particles
    void playerImpactEffect(float dirX);
    // Hollow-Knight style damage audio: boom, then the whole mix stays slowed + muffled for `ms` (= the
    // invincibility window) with a heartbeat, and eases back at the end. Wall-clock based (recovers while paused).
    // Call updateHurtAudio() every update AND render.
    void triggerHurtAudio(float intensity, unsigned int ms, bool heartbeat);
    // slow motion: after the hit-stop freeze, game time runs at `minScale` and eases back to 1.0 over `seconds`
    void triggerSlowMo(float seconds, float minScale);
    // everything that must keep running on REAL time even while the game is frozen / slowed
    void updateRealtimeFx(float realDt);
    void updateHurtAudio();
    void renderComboCounter(SDL_Renderer* renderer);
    void updateAnimation();

    void render(SDL_Renderer* renderer) override;

    void update(float dt) override;

    bool handleEvents(SDL_Event& event) override;

    static bool HandleBackgroundEvents(void *userdata, SDL_Event *event);
    bool isBlinkFrame() const;
    void handlePlayerHit(TrapType hazardType,gameMath::collisionSide side,unsigned int now,
                         float knockbackStrength = 600.0f, unsigned int knockbackDurationMs = KNOCKBACK_MS);
    const std::vector<Enemy>& getEnemies(){
        return m_enemyBuilder.getEnemies();
    }
    const EnemiesBuilder& getEnemyBuilder() const { return m_enemyBuilder; }
    void triggerCheckpoint();
    bool tryHeal(unsigned int now);
    void tryBeam(float dt);
    float computeBeamLength(float originX, float beamY, float beamH, bool facingRight);
    // nearest solid in front of originX (sword reach check) and what it is: 0 = wall/ground/platform, 1 = object (block/trap)
    float probeSolidAhead(float originX, float y, float h, bool facingRight, int& kind);
    // same idea for the up / down swings: nearest solid above (up=true) or below the player's body edge `originY`
    float probeSolidVertical(float originY, float x, float w, bool up, int& kind);

    const Player& getPlayer(){
        return m_player;
    }
    const SDL_FRect& getPlayerHitBox(){
        return m_playerHitBox;
    }
    const SDL_FRect& getBeamHitBox(){
        return m_beamHitBox;
    }
    const std::vector<Platform>& getPlatforms(){
        return m_platforms;
    }
    const SDL_FRect& getLevelWalls(){
        return m_wallCollisionRect;
    }
    const std::vector<LevelGround>& getLevelGrounds(){
        return m_grounds;
    }
    const std::vector<Fruit>& getFruits(){
        return m_fruits;
    }
    const TrapBuilder& getTrapBuilder(){
        return m_trapBuilder;
    }
    const std::vector<Trap>& getTraps(){
        return m_trapBuilder.getTraps();
    }
    const CheckPoint& getCheckPoint(){
        return m_checkPoint;
    }
private:
    //level
    LevelLoader m_levelLoader;

    int m_level =0;
    int m_transitionDelay =2000;
    uint32_t m_lastTransitionTime =0;



    //animation and texture
    uint32_t m_hurtAnimEndTime =0;
    uint32_t m_aniNowTime =0;
    uint32_t m_aniLastTime =SDL_GetTicks();
    int m_currentFrame=0;
    int m_aniframeDelay =50;//delayed by 50ms
    Animation m_Animation;
    PlayerAction m_playerAction = IDLE;


    SDL_IOStream* m_fontFile         = SDL_IOFromFile("Fonts/Pixel Game.otf", "rb");

    SDL_Texture* m_playerNameTextue  = nullptr;
    SDL_Texture* m_playerTexture     = nullptr;
    SDL_Texture* m_yellowSkin        = nullptr;
    SDL_Texture* m_blueSkin          = nullptr;
    SDL_Texture* m_redSkin           = nullptr;
    SDL_Texture* m_greenSkin         = nullptr;
    SDL_Texture* m_tileset           = nullptr;
    SDL_Texture* m_backGround        = nullptr;
    TTF_Font*    m_font              = nullptr;

    //sprites
    static const int SPRITE_HEIGHT =24;
    static const int SPRITE_WIDTH  =24;

    //player and platform

    std::vector<Platform> m_platforms;
    std::vector<LevelGround> m_grounds;
    std::vector<LevelGround> m_levelWalls;
    std::vector<Fruit> m_fruits;
    std::vector<Trap> m_traps;
    std::vector<Enemy> m_enemies;
    std::vector<Block> m_blocks;
    std::vector<BackGroundElement> m_backgroundElements;
    std::vector<ForeGroundElement> m_foregroundElements;
    std::vector<PrecomputedWallTile> m_wallTiles;

    SDL_FRect  m_wallCollisionRect{0.00f};
    FruitBuilder m_fruitBuilder;
    TrapBuilder m_trapBuilder;
    EnemiesBuilder m_enemyBuilder;
    InteractableBuilder m_interactableBuilder;
    GroundShape m_wallShape;
    BlockBuilder m_blockBuilder;
    BackGroundBuilder m_backgroundBuilder;
    ForeGroundBuilder m_foregroundBuilder;
    ParticleSystem m_particleSystem;
    ParticleSystem m_fxParticles;          // hit sparks/flashes: drawn AFTER enemies + player so they sit on top
    AmbientParticles m_ambientParticles;
    Lighting m_lighting;                   // ambient darkness + player light + level light objects
    float m_walkTimer=0.0f;

    Player   m_player;
    SDL_FRect m_playerHitBox{0.0f,0.0f,0.0f,0.0f};
    SDL_FRect m_beamHitBox{0.0f,0.0f,0.0f,0.0f};
    //checkPoint
    CheckPoint m_checkPoint;

    //scaling and window
    constexpr static const float P_scale =5.0f;
    int m_windowH =0,m_windowW=0;
    SDL_Renderer* m_renderer = nullptr;

    //mechanics
    int m_tempInc =0;
    int m_invincibilityTimer = 5000;
    int m_blinkTimer =500;
    uint32_t m_knockbackEndTime =0;
    //physics
    float m_previousY = 0.0f;
    float m_velocityY =0.0f;
    float m_velocityX =0.0f;
    float m_gravity =1800.00f;
    float m_jumpVelocity =1000.00f;

    // death
    bool  m_isDead =false;
    bool  m_deathPopped =false;
    float m_deathTimer =0.0f;
    float m_deathDir =1.0f;

    // hit VFX state
    float    m_hitStop =0.0f;              // seconds of freeze left (sim paused, shake + sparks keep running)
    int      m_combo =0;
    uint32_t m_lastComboHit =0;
    float    m_comboPop =0.0f;             // 1 -> 0 after each hit, drives the counter's scale pop
    int      m_wallHitId =-1;              // last swing that already clinked on a wall
    int      m_enemyHitId =-1;             // last swing that already hit an enemy
    float    m_beamFxTimer =0.0f;
    float    m_timeScale =1.0f;            // game-time speed (1 = normal)
    float    m_slowMoLeft =0.0f, m_slowMoTotal =0.0f, m_slowMoMin =1.0f;
    float    m_timeDebtMs =0.0f;           // fractional ms carried between frames when shifting wall-clock timers
    float    m_hurtVignette =0.0f;         // seconds of red screen-edge pulse left
    uint32_t m_playerFlashEnd =0;          // tick until which the player is drawn with a white flash
    float    m_impactFlash =0.0f;          // seconds of white impact-frame left (real dt)
    uint32_t m_audioFxStart =0, m_audioFxMs =0;
    float    m_audioFxIntensity =0.0f;
    bool     m_audioFxHeartbeat =false;
    bool     m_audioFxActive =false;
    uint32_t m_recoilEndTime =0;           // while < now: held move input is ignored so the push-back actually plays
    bool     m_wallBump =false;
    SDL_Texture* m_comboTexture =nullptr;
    int      m_comboTexValue =0;
    float    m_comboTexW =0.0f, m_comboTexH =0.0f;

    bool m_isAttacking =false;
    SlashDir m_slashDir =SlashDir::SIDE;
    int  m_pogoId =-1;    // last swing that already bounced the player (one pogo per swing)
    int  m_attackId =0;   // bumped at the start of every melee swing (see applyPlayerDamage)
    bool m_isBeaming =false;

    //double jump
    int m_jumpsUsed =0;
    int m_maxJumps =2;
    float m_coyoteTimer =0.0f;       // >0 while the first jump is still allowed after leaving the ground
    float m_jumpBufferTimer =0.0f;   // >0 while a recent jump press is waiting for the ground
    float m_attackBufferTimer =0.0f; // >0 while a recent attack press is waiting for the current swing to finish
    bool  m_jumpCutEligible =false;  // true while the current upward motion came from a real jump (not pogo/trampoline/fan/recoil)
    bool  m_respawnScreenOpen =false;
    float m_respawnX =109.0f, m_respawnY =0.0f;   // level start (from the 'S' line when the level has one)
    bool m_isDoubleJumping =false;

    // wall jump
    int   m_wallDir =0;               // wall touching the player this frame (-1 left / +1 right / 0 none)
    int   m_wallCoyoteDir =0;         // side of the last wall touched (used while m_wallCoyoteTimer > 0)
    float m_wallCoyoteTimer =0.0f;
    float m_wallJumpLockTimer =0.0f;  // >0 while the kick away from the wall overrides horizontal input
    float m_wallJumpDir =0.0f;        // +1 / -1 direction of the kick

    // power dash
    bool  m_isDashing =false;
    bool  m_dashUsed =false;          // true after a dash; cleared on touching the ground or on a pogo
    bool  m_dashFromGround =false;    // dash started on the ground: keep a 1px press into the floor so it stays 'grounded'
    float m_dashTimer =0.0f;          // seconds left in the current dash
    float m_dashCooldown =0.0f;
    float m_dashDir =1.0f;            // +1 right / -1 left, locked at the start of the dash
    float m_dashTrailTimer =0.0f;
    int m_doubleJumpFrame =0;

    unsigned int m_healCooldownEndTime =0;
    unsigned int m_lastBeamTime =0;            // last tick the beam input was held (heal is locked out shortly after)
    bool  m_isGrounded =true;
    bool  m_wasGrounded =false;
    bool m_isPlayerfacingRight =true;
    bool m_isCompleted=false;
    bool m_transitioning=false;
    bool m_levelTransitioning =false;

};