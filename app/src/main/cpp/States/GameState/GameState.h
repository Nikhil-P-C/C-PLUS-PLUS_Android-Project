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

#include "level/Fruits.h"
#include "level/Traps.h"
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

enum PlayerAction{
    IDLE =0,
    MOVINGLEFT=1,
    MOVINGRIGHT=2,
    CROUCHING=3,
    HURT=4,
    JUMP=5,

};

class GameState : public State{
public:
    GameState(SDL_Renderer* renderer,int level);
    ~GameState() override{
        LOGI("gamestate destructor:%p",this);
        if (m_font)            TTF_CloseFont(m_font);
        if (m_playerNameTextue)SDL_DestroyTexture(m_playerNameTextue);
        if (m_fontFile)        SDL_CloseIO(m_fontFile);
        SDL_SetTextureAlphaMod(m_playerTexture,255);
        SDL_RemoveEventWatch(HandleBackgroundEvents, this);
    }
    bool hasWallAbove(float x,float y);
    bool hasWallBelow(float x,float y);
    bool hasWallRight(float x,float y);
    bool hasWallLeft(float x,float y);
    void setLevel(int level);

    void handleCollision();
    void handlePhysicAndInput(float dt);
    void updateAnimation();
    void render(SDL_Renderer* renderer) override;

    void update(float dt) override;

    bool handleEvents(SDL_Event& event) override;
    static bool HandleBackgroundEvents(void *userdata, SDL_Event *event);
    bool isBlinkFrame() const;
    void handlePlayerHit(TrapType hazardType,gameMath::collisionSide side,unsigned int now);
    void triggerCheckpoint();

    Player getPlayer(){
        return m_player;
    }
    std::vector<Platform> getPlatforms(){
        return m_platforms;
    }
    SDL_FRect getLevelWalls(){
        return m_wallCollisionRect;
    }
    std::vector<LevelGround> getLevelGrounds(){
        return m_grounds;
    }
    std::vector<Fruit> getFruits(){
        return m_fruits;
    }
    TrapBuilder& getTrapBuilder(){
        return m_trapBuilder;
    }
    std::vector<Trap>& getTraps(){
        return m_trapBuilder.getTraps();
    }
    CheckPoint getCheckPoint(){
        return m_checkPoint;
    }
private:
    //level
    LevelLoader m_levelLoader;

    int m_level =0;
    int m_transitionDelay =3000;
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
    std::vector<Block> m_blocks;
    std::vector<BackGroundElement> m_backgroundElements;
    std::vector<ForeGroundElement> m_foregroundElements;

    SDL_FRect  m_wallCollisionRect{0.00f};
    FruitBuilder m_fruitBuilder;
    TrapBuilder m_trapBuilder;
    GroundShape m_wallShape;
    BlockBuilder m_blockBuilder;
    BackGroundBuilder m_backgroundBuilder;
    ForeGroundBuilder m_foregroundBuilder;
    ParticleSystem m_particleSystem;
    float m_walkTimer=0.0f;

    Player   m_player;
    //checkPoint
    CheckPoint m_checkPoint;

    //scaling and window
    constexpr static const float P_scale =5.0f;
    int m_windowH =0,m_windowW=0;
    SDL_Renderer* m_renderer = nullptr;

    //mechanics

    int m_invincibilityTimer = 5000;
    int m_blinkTimer =500;
    uint32_t m_knockbackEndTime =0;
    //physics
    float m_previousY = 0.0f;
    float m_velocityY =0.0f;
    float m_velocityX =0.0f;
    float m_gravity =1800.00f;
    float m_jumpVelocity =1000.00f;


    bool  m_isGrounded =true;
    bool  m_wasGrounded =false;
    bool m_isPlayerfacingRight =true;
    bool m_isCompleted=false;
    bool m_transitioning=false;
    bool m_levelTransitioning =false;

};

