
#include "GameState.h"
#include "States/MenuState/TitleScreenState.h"
#include "States/TransitionState/TransitionState.h"
#include "States/PauseState/PauseState.h"
#include "States/DebugState/DebugState.h"
#include "States/HUDOverlayState/HUDOverlayState.h"
#include "utils/utils.h"
#include <cmath>


//
// Created by LENOVO on 27-04-2026.
//
GameState::GameState(SDL_Renderer *renderer,int level) {

    this->Name = "GameState";
    SDL_AddEventWatch(HandleBackgroundEvents,this);
    m_renderer = renderer;
    LOGI("gamestate constructor:%p",this);
    m_windowH =GameData::getInstance().getWinHeight();
    m_windowW =GameData::getInstance().getWinWidth();

    //init player attributes
    m_player.setSize(SPRITE_WIDTH*P_scale-80.00f,SPRITE_HEIGHT*P_scale-35.00f);
    m_player.setPosition(200.00f,400.00f,P_scale);

    m_player.setSpriteOffset(-40.00f,-20.00f);
    m_player.setSpriteSize(SPRITE_WIDTH*P_scale,SPRITE_HEIGHT*P_scale);
    Camera::getInstance().setSize((float)m_windowW,(float)m_windowH);

    //font loading
    m_font = TTF_OpenFontIO(m_fontFile, false, 36);
    //background sprite loading;
    m_backGround = Engine::Get().getAssetManager().getTexture(TextureType::BG_SKY);
    // m_tileset loading
    m_tileset = Engine::Get().getAssetManager().getTexture(TextureType::TILESET_SPRITE);
    //player sprite loading
    m_yellowSkin = Engine::Get().getAssetManager().getTexture(TextureType::YELLOW_SKIN);
    m_blueSkin  = Engine::Get().getAssetManager().getTexture(TextureType::BLUE_SKIN);
    m_redSkin  = Engine::Get().getAssetManager().getTexture(TextureType::RED_SKIN);
    m_greenSkin  = Engine::Get().getAssetManager().getTexture(TextureType::GREEN_SKIN);

    //from player static data
    if(PlayerDetail::getInstance().getPlayerSkin() == PlayerSkin::YELLOW)
        m_playerTexture = m_yellowSkin;
    else if(PlayerDetail::getInstance().getPlayerSkin()  == PlayerSkin::BLUE)
        m_playerTexture = m_blueSkin;
    else if(PlayerDetail::getInstance().getPlayerSkin()  == PlayerSkin::RED)
        m_playerTexture = m_redSkin;
    else if(PlayerDetail::getInstance().getPlayerSkin()  == PlayerSkin::GREEN)
        m_playerTexture = m_greenSkin;

    //player name loading
    SDL_Surface* playerNameSurface = TTF_RenderText_Solid(m_font,PlayerDetail::getInstance().getPlayerName().c_str(),
                                                          PlayerDetail::getInstance().getPlayerName().length(),{255,255,255,255});
    m_playerNameTextue = SDL_CreateTextureFromSurface(renderer,playerNameSurface);
    SDL_DestroySurface(playerNameSurface);

    m_level = level;
    setLevel(m_level);
}

void GameState::render(SDL_Renderer* renderer)  {

    int camX = (int)std::round(Camera::getInstance().getCamera().x);
    int camY = (int)std::round(Camera::getInstance().getCamera().y);

//    SDL_FRect backgroundDst{static_cast<float>(0-camX), static_cast<float>(0-camY), 3200, 1536};
//    SDL_RenderTexture(renderer, m_backGround, nullptr, &backgroundDst);

    // Group 1: background + walls + particles + traps + fruits, all drawn
    // before the player. One offscreen capture, bright-pass + blur applied
    // once to the whole group (see PostProcessor::beginBloomGroup).
    Engine::Get().getPostProcessor().beginBloomGroup(m_renderer);

    m_backgroundBuilder.render(m_renderer);

    for(const auto& level :m_levelWalls)
    {
        int tileSize =(int)(TILE_SIZE*SCALE);
        const int platformWidth = (int)level.w;
        const int platformHeight = (int)level.h;
        int widthTiles = static_cast<int>(std::ceil((float)platformWidth  / (float)tileSize));
        int heightTiles = static_cast<int>(std::ceil((float)platformHeight / (float)tileSize));
        for (int i = 0; i < heightTiles; i++) {
            for (int j = 0; j < widthTiles; j++) {
                SDL_FRect src;
                float x =level.x + (float)j * (TILE_SIZE * SCALE);
                float y =level.y + (float)i* (TILE_SIZE * SCALE);
                bool left = (j == 0);
                bool right = (j == widthTiles - 1);
                bool top = (i == 0);
                bool bottom = (i == heightTiles - 1);
                bool edge =false;
                SpriteValue CurrentSpriteValue;
                SpriteValue CurrentEdgeSpriteValue;
                if(level.spriteType == SpriteType::STONE_BRICK_WALL) {
                    CurrentSpriteValue =SpriteCollection::stoneBrickWall;
                    CurrentEdgeSpriteValue =SpriteCollection::stoneBrickWallEdges;
                }
                else if(level.spriteType == SpriteType::MOSS_WALL) {
                    CurrentSpriteValue =SpriteCollection::mossWall;
                    CurrentEdgeSpriteValue =SpriteCollection::mossWallEdges;
                }
                if(hasWallAbove(x,y)){
                    top =false;
                }
                if(hasWallBelow(x,y)){
                    bottom =false;
                }
                if(hasWallRight(x,y)){
                    right =false;
                }
                if(hasWallLeft(x,y)){
                    left =false;
                }
                if (top) {
                    if (left) {
                        src = {CurrentEdgeSpriteValue.x,
                               CurrentEdgeSpriteValue.y, TILE_SIZE, TILE_SIZE};
                        edge =true;
                        }
                    else if (right) {
                        src = {CurrentEdgeSpriteValue.x + TILE_SIZE,
                                CurrentEdgeSpriteValue.y, TILE_SIZE, TILE_SIZE};
                        edge =true;
                    }
                    else {
                        src = {CurrentSpriteValue.x + TILE_SIZE,
                               CurrentSpriteValue.y, TILE_SIZE, TILE_SIZE};
                        edge = false;
                    }
                }
                else if (bottom) {
                    if (left) {
                        src = {CurrentEdgeSpriteValue.x,
                               CurrentEdgeSpriteValue.y+TILE_SIZE, TILE_SIZE, TILE_SIZE};
                        edge =true;
                    }
                    else if (right) {
                        src = {CurrentEdgeSpriteValue.x + TILE_SIZE,
                               CurrentEdgeSpriteValue.y+TILE_SIZE, TILE_SIZE, TILE_SIZE};
                        edge= true;
                    }
                    else {
                        src = {CurrentSpriteValue.x + TILE_SIZE,
                               CurrentSpriteValue.y+32.00f, TILE_SIZE, TILE_SIZE};
                        edge =false;
                    }
                } else {
                    edge =false;
                    if (left) src = {CurrentSpriteValue.x,
                                     CurrentEdgeSpriteValue.y+ TILE_SIZE, TILE_SIZE, TILE_SIZE};
                    else if (right)
                        src = {CurrentSpriteValue.x + 32.00f,
                               CurrentEdgeSpriteValue.y+TILE_SIZE, TILE_SIZE, TILE_SIZE};
                    else src = {CurrentSpriteValue.x + TILE_SIZE,
                                CurrentSpriteValue.y+TILE_SIZE, TILE_SIZE, TILE_SIZE};
                }

                SDL_FRect dst = {
                        x - (float)camX,
                        y - (float)camY,
                        TILE_SIZE * SCALE, TILE_SIZE * SCALE};

                if ((top || bottom || left || right )&& !edge) {
                    SDL_RenderTextureRotated(renderer, m_tileset, &src, &dst,0.0f, nullptr,SDL_FLIP_HORIZONTAL_AND_VERTICAL);
                }
                else if(((top || bottom) && (left || right) && edge)){
                    SDL_RenderTexture(renderer, m_tileset, &src, &dst);
                }

            }
        }
    }

    for(auto const& tile:m_wallShape.tiles){
        SDL_FRect src = tile.src;
        SDL_FRect dst{tile.x-(float)camX,tile.y-(float)camY,tile.w,tile.h};
        SDL_RenderTexture(renderer,m_tileset,&src,&dst);

    }

    for(const auto& platform : m_platforms){
        int tileSize =TILE_SIZE;

        const int platformWidth = (int)platform.w;
        const int platformHeight = (int)platform.h;

        int widthTiles = platformWidth / tileSize;
        int heightTiles = platformHeight / tileSize;

        for (int y = 0; y < heightTiles; y++) {
            for (int x = 0; x < widthTiles; x++) {

                SDL_FRect src;

                bool left = (x == 0);
                bool right = (x == widthTiles - 1);
                bool top = (y == 0);
                bool bottom = (y == heightTiles - 1);

                if(heightTiles ==1){

                    if(platform.platformType == SpriteType::GOLD_PLATFORM){

                        if (left) src = {SpriteCollection::goldPlatform.x, 0.00f, TILE_SIZE, TILE_SIZE};

                        else if (right)
                            src = {SpriteCollection::goldPlatform.x + 32.00f, 0.00f, TILE_SIZE,
                                   TILE_SIZE};

                        else
                            src = {SpriteCollection::goldPlatform.x + TILE_SIZE, 0.00f, TILE_SIZE,
                                   TILE_SIZE};
                    }

                    else if(platform.platformType == SpriteType::WOODEN_PLATFORM){
                        if (left) src = {SpriteCollection::woodenPlatform.x,
                                         SpriteCollection::woodenPlatform.y, TILE_SIZE, TILE_SIZE};

                        else if (right)
                            src = {SpriteCollection::woodenPlatform.x + 32.00f,
                                   SpriteCollection::woodenPlatform.y, TILE_SIZE,TILE_SIZE};

                        else
                            src = {SpriteCollection::woodenPlatform.x + TILE_SIZE,
                                   SpriteCollection::woodenPlatform.y, TILE_SIZE,
                                   TILE_SIZE};

                    }

                    else if(platform.platformType == SpriteType::STONE_PLATFORM) {
                        if (left) src = {SpriteCollection::stonePlatform.x,
                                         SpriteCollection::stonePlatform.y, TILE_SIZE, TILE_SIZE};

                        else if (right)
                            src = {SpriteCollection::stonePlatform.x + 32.00f,
                                   SpriteCollection::stonePlatform.y, TILE_SIZE,TILE_SIZE};

                        else
                            src = {SpriteCollection::stonePlatform.x + TILE_SIZE,
                                   SpriteCollection::stonePlatform.y, TILE_SIZE,
                                   TILE_SIZE};
                    }

                }

                SDL_FRect dst = {(platform.x+x* (TILE_SIZE * SCALE)) - camX,
                                 ( platform.y+y * (TILE_SIZE * SCALE)) - camY,
                                 TILE_SIZE * SCALE, TILE_SIZE * SCALE};

                SDL_RenderTexture(renderer, m_tileset, &src, &dst);
            }

        }

    }
    SDL_FRect checkPointDst{m_checkPoint.x-camX,m_checkPoint.y-camY,m_checkPoint.w,m_checkPoint.h};
    if(m_checkPoint.aniType == CheckPointAni::NO_FLAG)
    {
        SDL_FRect checkPointSrc{0.00f,0.00f,64.00f,64.00f};
        SDL_RenderTexture(renderer,Engine::Get().getAssetManager().getTexture(TextureType::CHECKPOINT_FLAG_NO),
                          &checkPointSrc,&checkPointDst);
    }
    if(m_checkPoint.aniType == CheckPointAni::FLAG_OUT)
    {
        SDL_FRect checkPointSrc{0.00f+64.00f*m_checkPoint.currentFrame,0.00f,64.00f,64.00f};
        SDL_RenderTexture(renderer,Engine::Get().getAssetManager().getTexture(TextureType::CHECKPOINT_FLAG_OUT),
                          &checkPointSrc,&checkPointDst);
    }
    if(m_checkPoint.aniType == CheckPointAni::FLAG_IDLE)
    {
        SDL_FRect checkPointSrc{0.00f+64.00f*m_checkPoint.currentFrame,0.00f,64.00f,64.00f};
        SDL_RenderTexture(renderer,Engine::Get().getAssetManager().getTexture(TextureType::CHECKPOINT_FLAG_IDLE),
                          &checkPointSrc,&checkPointDst);
    }

    m_particleSystem.render(m_renderer);
    m_trapBuilder.render(m_renderer);
    m_fruitBuilder.render(m_renderer);

    Engine::Get().getPostProcessor().endBloomGroup(m_renderer); // composites Group 1 onto the window

    // Player draws directly to the backbuffer from here — sharp, untouched
    // by bloom regardless of what's happening in it (invincibility flicker,
    // skin color, etc).
    SDL_FRect dst = {m_player.x+m_player.spriteOffsetX-(float)camX,
                     m_player.y+m_player.spriteOffsetY-(float)camY,
                     m_player.spriteW,m_player.spriteH};

    SDL_FRect src = {(float) (0 + (SPRITE_WIDTH * m_currentFrame)), 0, SPRITE_WIDTH, SPRITE_HEIGHT};
    if(PlayerDetail::getInstance().isInvincible())
        SDL_SetTextureAlphaMod(m_playerTexture, isBlinkFrame() ? 80 : 150);
    else
        SDL_SetTextureAlphaMod(m_playerTexture,255);

    if(!m_isPlayerfacingRight)
    {
        SDL_RenderTextureRotated(renderer, m_playerTexture, &src, &dst, 0.0f, nullptr,
                                 SDL_FLIP_HORIZONTAL);
    }
    else
    {
        SDL_RenderTexture(renderer, m_playerTexture, &src, &dst);
    }
    //player name rendering
    SDL_FRect playerNameDst{m_player.x+m_player.spriteOffsetX-(float)camX+(12.50f*P_scale)
                            -(50+1*static_cast<float>(PlayerDetail::getInstance().getPlayerName().length()))/2,
                            m_player.y+m_player.spriteOffsetY-(float)camY-(2.00f*P_scale) -10,
                            50+1*static_cast<float>(PlayerDetail::getInstance().getPlayerName().length()),
                            45.00f};
    SDL_RenderTexture(renderer,m_playerNameTextue, nullptr,&playerNameDst);


    //TODO BETTER COUNTER RENDERING
    //this is for test purpose
    std::string fruitCounter = "fruit:"+std::to_string(PlayerDetail::getInstance().getScore());

    SDL_Surface* fruitCounterSurface = TTF_RenderText_Solid(m_font,fruitCounter.c_str(),
                                                           fruitCounter.length(),{255,255,255,255});
    SDL_Texture* fruitCounterTexture= SDL_CreateTextureFromSurface(renderer,fruitCounterSurface);
    SDL_FRect fruitCounterDst{1420.00f,0.00f,160.00f,100.00f};


    m_blockBuilder.render(m_renderer); // platforms stay sharp — not in a bloom group

    // Group 2: foreground only, drawn after the player/blocks so it composites on top of them.
    Engine::Get().getPostProcessor().beginBloomGroup(m_renderer);
    m_foregroundBuilder.render(m_renderer);
    Engine::Get().getPostProcessor().endBloomGroup(m_renderer);
    SDL_RenderTexture(renderer,fruitCounterTexture, nullptr,&fruitCounterDst);
    SDL_DestroyTexture(fruitCounterTexture);
    SDL_DestroySurface(fruitCounterSurface);


}

void GameState::update(float dt){
//    LOGI("player x : %f , player : %f",m_player.x,m_player.y);
    if(PlayerDetail::getInstance().getPlayerHP() <= 0){
        //respawn
        PlayerDetail::getInstance().addPlayerHP(5);
        m_player.setPosition(109.00f,0.00f,P_scale);
    }
    m_fruitBuilder.update(dt);

    m_previousY =m_player.y;

    handlePhysicAndInput(dt);

    m_isGrounded=false;

    handleCollision();

    for(int i =0;i<m_traps.size();i++)
    {
        //fire
        if(m_traps[i].type == TrapType::FIRE)
            m_trapBuilder.checkFireCollision(i,m_player.x,m_player.y,m_player.w,m_player.h);
        if(m_trapBuilder.isSolid(i)){
            gameMath::collisionSide side =m_trapBuilder.resolveTrapCollision(i, m_player.x, m_player.y,
                                                                             m_player.w, m_player.h,
                                                                             m_previousY,m_velocityY);


            if(side != gameMath::collisionSide::NONE&&(m_trapBuilder.trapHasPath(m_traps[i].type)||m_traps[i].type==TrapType::FALLING_PLATFORM)){
                SDL_FPoint trapDelta =m_trapBuilder.getTrapDelta(i);
                m_player.x+=trapDelta.x;
                if(m_traps[i].colliderType != ColliderType::ONE_WAY && side ==gameMath::collisionSide::TOP)
                    m_player.y+=trapDelta.y;
            }
            switch (side) {
                case gameMath::collisionSide::TOP:
                    if(m_traps[i].type == TrapType::FALLING_PLATFORM) {
                        m_trapBuilder.triggerFall(i);
                    }
                    m_isGrounded =true;
                    m_velocityY =0.0f;
                    break;
                case gameMath::collisionSide::BOTTOM:
                    m_velocityY =0.0f;
                case gameMath::collisionSide::LEFT:
                case gameMath::collisionSide::RIGHT:
                case gameMath::collisionSide::NONE:
                    break;
            }
        }
        //trampoline
        bool trampolineJump =false;
        if(m_traps[i].type == TrapType::TRAMPOLINE)
            trampolineJump=m_trapBuilder.checkTrampolineBounce(i,m_player.x,m_player.y,m_player.w,m_player.h,m_particleSystem);
        if(trampolineJump){
            m_velocityY=0.0f;
            m_isGrounded = false;
            m_velocityY += -1500;
        }

    }

    m_trapBuilder.updatePath(dt);

    TrapType type;
    gameMath::collisionSide side =gameMath::collisionSide::NONE;

    bool hazardColl =m_trapBuilder.checkHazard(m_player.x,m_player.y,
                                               m_player.w,m_player.h,type ,side);


    unsigned int hitNow =SDL_GetTicks();

    if(hitNow-PlayerDetail::getInstance().getLastHitTime() >m_invincibilityTimer){
        PlayerDetail::getInstance().setInvincibility(false);
        if(hazardColl)
        {
            handlePlayerHit(type, side, hitNow);
        }
    }


    int score = m_fruitBuilder.onCollision(m_player.x,m_player.y,m_player.w,m_player.h);
    PlayerDetail::getInstance().addScore(score);


    //camera
    Camera::getInstance().lockCameraOn(m_player.x,m_player.y,m_player.h,m_player.w);

    SDL_FRect cameraBounds{m_wallCollisionRect.x,m_wallCollisionRect.y,m_wallCollisionRect.w,m_wallCollisionRect.h+TILE_SIZE*SCALE};
    Camera::getInstance().cameraClamp(cameraBounds);

    float Force = m_trapBuilder.checkFanForce(m_player.x,m_player.y,m_player.w,m_player.h,m_particleSystem);
    if(Force <0)
    {
        m_velocityY=0.0f;
        m_velocityY += Force;
        m_isGrounded = false;
    }


    if(gameMath::checkcollision(m_player.x,m_player.y,m_checkPoint.x,m_checkPoint.y,
                                m_player.h,m_player.w,m_checkPoint.h,m_checkPoint.w)){
        triggerCheckpoint();

        if(!m_isCompleted)
            m_lastTransitionTime = SDL_GetTicks();
        m_isCompleted =true;
    }

    if(m_isCompleted && SDL_GetTicks() - m_lastTransitionTime>m_transitionDelay){
        if(m_levelTransitioning)return;
        m_levelTransitioning =true;

        Engine::Get().popOverlayState();
        Engine::Get().popOverlayState();
        if(GameData::getInstance().isDebugEnabled())
            Engine::Get().popOverlayState();
        Engine::Get().pushOverlayState(std::make_unique<TransitionState>(m_renderer,m_level+1));
    }

    updateAnimation();

    m_particleSystem.update(dt);
    m_fruitBuilder.update(dt);
    m_trapBuilder.update(dt);

}

bool GameState::handleEvents(SDL_Event& event) {
//    if (m_transitioning)return true;

    if(InputUtils::IsBackKey(event)) {
        InputDispatcher::getInstance().setInputReleased(true);
        InputDispatcher::getInstance().inputLogClear();
        GameData::getInstance().setPaused(true);
        m_transitioning = true;
        Engine::Get().popOverlayState();//controls
        Engine::Get().popOverlayState();//HUD
        if (GameData::getInstance().isDebugEnabled())
            Engine::Get().popOverlayState();
        Engine::Get().pushState(std::make_unique<PauseState>(m_renderer, this));
        return true;
    }

    return false;
}

void GameState::handleCollision() {
    //Walls

    const float renderedHeight = (std::ceil(m_wallCollisionRect.h / (SCALE*TILE_SIZE)) *(SCALE *TILE_SIZE));
    gameMath::collisionSide wallCollisionSide;

    wallCollisionSide = gameMath::checkcollisionXY(m_player.x,m_player.y,m_wallCollisionRect.x,
                                                   m_wallCollisionRect.y,
                                                   m_player.h,m_player.w,
                                                   TILE_SIZE * SCALE,m_wallCollisionRect.w);
    if(wallCollisionSide == gameMath::collisionSide::BOTTOM)
        m_velocityY =0.0f;
    wallCollisionSide =gameMath::checkcollisionXY(m_player.x, m_player.y, m_wallCollisionRect.x,
                                                  m_wallCollisionRect.y,
                                                  m_player.h, m_player.w, renderedHeight,
                                                  TILE_SIZE * SCALE);
    if(wallCollisionSide == gameMath::collisionSide::BOTTOM)
        m_velocityY =0.0f;
    wallCollisionSide =gameMath::checkcollisionXY(m_player.x, m_player.y,
                                                  m_wallCollisionRect.x + m_wallCollisionRect.w -
                                                  TILE_SIZE * SCALE, m_wallCollisionRect.y,
                                                  m_player.h, m_player.w, renderedHeight,
                                                  TILE_SIZE * SCALE);
    if(wallCollisionSide == gameMath::collisionSide::BOTTOM)
        m_velocityY =0.0f;
    wallCollisionSide =gameMath::checkcollisionXY(m_player.x, m_player.y, m_wallCollisionRect.x,
                                                  m_wallCollisionRect.y + renderedHeight -
                                                  TILE_SIZE * SCALE,
                                                  m_player.h, m_player.w, TILE_SIZE * SCALE,
                                                  m_wallCollisionRect.w);
    if (wallCollisionSide == gameMath::collisionSide::BOTTOM)
        m_velocityY = 0.0f;
    if (wallCollisionSide == gameMath::collisionSide::TOP) {
        m_isGrounded = true;
        m_velocityY = 0.0f;
    }


    //platforms
    for(const auto& platform : m_platforms) {
        if (platform.colliderType == ColliderType::SOLID) {

            gameMath::collisionSide side = gameMath::checkcollisionXY(m_player.x, m_player.y,
                                                                      platform.x,
                                                                      platform.y,
                                                                      m_player.h, m_player.w,
                                                                      platform.h * SCALE,
                                                                      platform.w * SCALE);
            if (side == gameMath::collisionSide::TOP) {

                m_velocityY = 0.0f;
                m_isGrounded =true;
            }
            if (side == gameMath::collisionSide::BOTTOM) {
                m_velocityY = 0.0f;
            }
        } else if (platform.colliderType == ColliderType::ONE_WAY) {
            float previousBottom = m_previousY + m_player.h;
            float currentBottom = m_player.y + m_player.h;
            float platformTop = platform.y;

            if (m_velocityY > 0 && previousBottom <= platformTop
                && currentBottom >= platformTop
                && gameMath::checkcollisionX(m_player.x, m_player.y, platform.x,
                                             platform.y,
                                             m_player.h, m_player.w, platform.h * SCALE,
                                             platform.w * SCALE)) {

                m_player.y = platformTop - m_player.h;
                m_velocityY = 0.0f;
                m_isGrounded = true;
            }

        }

    }

    //ground
    for(const auto& ground : m_grounds){
        gameMath::collisionSide groundCollisionSide=
                gameMath::checkcollisionXY(m_player.x,m_player.y,ground.x,ground.y,
                                           m_player.h,m_player.w,ground.h*SCALE,ground.w*SCALE);

        if (groundCollisionSide == gameMath::collisionSide::TOP) {
            m_isGrounded = true;
            m_velocityY = 0.0f;
        }

        if(groundCollisionSide == gameMath::collisionSide::BOTTOM){
            m_velocityY =0.0f;
        }
    }

    for(const auto& block : m_blocks){
        gameMath::collisionSide blockCollisionSide=
                gameMath::checkcollisionXY(m_player.x,m_player.y,block.rect.x,block.rect.y,
                                           m_player.h,m_player.w,block.rect.h,block.rect.w);
        if (blockCollisionSide == gameMath::collisionSide::TOP) {
            m_isGrounded = true;
            m_velocityY = 0.0f;
        }
        if(blockCollisionSide == gameMath::collisionSide::BOTTOM){
            m_velocityY =0.0f;
        }
    }
}

void GameState::handlePlayerHit(TrapType hazardType, gameMath::collisionSide side, unsigned int now) {

    PlayerDetail::getInstance().setLastHitTime(now);

    PlayerDetail::getInstance().subPlayerHP(1);

    PlayerDetail::getInstance().setInvincibility(true);
    m_hurtAnimEndTime = now + HURT_ANIM_MS;
    m_knockbackEndTime = now + KNOCKBACK_MS;
    const float KNOCK_H = 600.0f, KNOCK_V = 600.0f;
    switch(side){
        case gameMath::collisionSide::TOP:    m_velocityY = -KNOCK_V; break; // pushed up
        case gameMath::collisionSide::BOTTOM: m_velocityY =  KNOCK_V; break; // pushed down
        case gameMath::collisionSide::LEFT:   m_velocityX = -KNOCK_H; break; // pushed left
        case gameMath::collisionSide::RIGHT:  m_velocityX =  KNOCK_H; break; // pushed right
        default: break;
    }
    m_isGrounded = false;


}

void GameState::triggerCheckpoint(){
    if(m_checkPoint.aniType == CheckPointAni::FLAG_OUT)return;
    m_checkPoint.aniType = m_checkPoint.aniType == CheckPointAni::NO_FLAG ? CheckPointAni::FLAG_OUT:CheckPointAni::FLAG_IDLE;
}

void GameState::updateAnimation() {
    if(SDL_GetTicks() < m_hurtAnimEndTime) m_playerAction = PlayerAction::HURT;
    switch(m_playerAction){
        case IDLE:
            m_Animation.startIndex=0;
            m_Animation.lastIndex =3;
            break;
        case MOVINGLEFT:
        case MOVINGRIGHT:
            m_Animation.startIndex=4;
            m_Animation.lastIndex =10;
            break;
        case JUMP:
            m_Animation.startIndex=11;
            m_Animation.lastIndex =13;
            break;
        case HURT:
            m_Animation.startIndex=14;
            m_Animation.lastIndex =16;
            break;
        case CROUCHING:
            m_Animation.startIndex=17;
            m_Animation.lastIndex =23;
            break;

    }

    m_aniNowTime = SDL_GetTicks();
    if(m_aniNowTime - m_aniLastTime > m_aniframeDelay){
        if(m_currentFrame < m_Animation.startIndex)
            m_currentFrame = m_Animation.startIndex;
        if(m_currentFrame < m_Animation.lastIndex)
            m_currentFrame++;
        else
            m_currentFrame = m_Animation.startIndex;
        m_aniLastTime = m_aniNowTime;
    }


    switch(m_checkPoint.aniType){
        case CheckPointAni::NO_FLAG:
            m_checkPoint.ani.startIndex = 0;
            m_checkPoint.ani.lastIndex = 0;
            break;
        case CheckPointAni::FLAG_OUT:
            m_checkPoint.ani.startIndex =0;
            m_checkPoint.ani.lastIndex =24;
            break;
        case CheckPointAni::FLAG_IDLE:
            m_checkPoint.ani.startIndex =0;
            m_checkPoint.ani.lastIndex =9;
    }
    uint32_t nowTime = SDL_GetTicks();
    if(nowTime - m_checkPoint.aniLastTime > m_aniframeDelay){
        m_checkPoint.aniLastTime = nowTime;

        if(m_checkPoint.currentFrame < m_checkPoint.ani.startIndex)
            m_checkPoint.currentFrame = m_checkPoint.ani.startIndex;

        if(m_checkPoint.currentFrame < m_checkPoint.ani.lastIndex){
            m_checkPoint.currentFrame++;
        }
        else{
            if(m_checkPoint.aniType == CheckPointAni::FLAG_OUT){
                m_checkPoint.aniType = CheckPointAni::FLAG_IDLE;
                m_checkPoint.currentFrame = 0;
            }
            else if(m_checkPoint.aniType == CheckPointAni::FLAG_IDLE){
                m_checkPoint.currentFrame = m_checkPoint.ani.startIndex;
            }
        }
    }

}

void GameState::handlePhysicAndInput(float dt) {
    if(m_wasGrounded && m_isGrounded){
        m_particleSystem.emitLandDust(m_player.x+20.00f, m_player.y+m_player.h-50.00f,
                                      m_player.x-40.00f, m_player.y+m_player.h-50.00f);
        m_wasGrounded =false;
    }
    bool inKnockback = SDL_GetTicks() < m_knockbackEndTime;

    if(!inKnockback)
    {
        if (!InputDispatcher::getInstance().movingLeft &&
            !InputDispatcher::getInstance().movingRight) {
            m_playerAction = PlayerAction::IDLE;
            m_velocityX = 0.0f;
        }
    }
    if(InputDispatcher::getInstance().movingLeft){
        m_isPlayerfacingRight =false;
        m_playerAction=PlayerAction::MOVINGLEFT;
        m_velocityX = -400.00f;
        m_walkTimer += dt;
        if(m_isGrounded){
            if(m_walkTimer >0.2){
                m_walkTimer =0.00f;
                m_particleSystem.emitLeftDust(m_player.x+20.00f, m_player.y+m_player.h-50.00f);
                m_particleSystem.emitLeftDust(m_player.x+20.00f, m_player.y+m_player.h-50.00f);
                m_particleSystem.emitLeftDust(m_player.x+20.00f, m_player.y+m_player.h-50.00f);
            }
        }
        else{
            m_walkTimer =0.00f;
        }
    }
    if(InputDispatcher::getInstance().movingRight){
        m_isPlayerfacingRight =true;
        m_playerAction=PlayerAction::MOVINGRIGHT;
        m_velocityX=400.00f;

        m_walkTimer += dt;
        if(m_isGrounded){
            if(m_walkTimer >0.2){
                m_walkTimer =0.00f;
                m_particleSystem.emitRightDust(m_player.x-40.00f, m_player.y+m_player.h-50.00f);
                m_particleSystem.emitRightDust(m_player.x-40.00f, m_player.y+m_player.h-50.00f);
                m_particleSystem.emitRightDust(m_player.x-40.00f, m_player.y+m_player.h-50.00f);
            }
        }
        else{
            m_walkTimer =0.00f;
        }
    }
    if(InputDispatcher::getInstance().jump && m_isGrounded){
        m_playerAction=PlayerAction::JUMP;
        m_velocityY =-m_jumpVelocity;
        m_wasGrounded =true;
        m_particleSystem.emitJumpDust(m_player.x, m_player.y + m_player.h - 40.00f);
    }

    m_player.x +=m_velocityX * dt;
    m_velocityY+=m_gravity*dt;
    m_player.y +=m_velocityY*dt;
}

void GameState::setLevel(int level) {

    m_levelLoader.loadLevel(level);

    m_checkPoint=m_levelLoader.getCheckPoint();
    m_blocks =m_levelLoader.getBlocks();
    m_levelWalls =m_levelLoader.getLevelWalls();
    m_platforms =m_levelLoader.getPlatforms();
    m_grounds =m_levelLoader.getLevelGrounds();
    m_fruits =m_levelLoader.getFruits();
    m_traps =m_levelLoader.getTraps();
    m_backgroundElements = m_levelLoader.getBackGroundElements();
    m_foregroundElements = m_levelLoader.getForeGroundElements();
    m_wallCollisionRect=m_levelLoader.getWallCollisionRect();

    m_blockBuilder.init(m_blocks,TILE_SIZE,SCALE);
    m_fruitBuilder.init(m_fruits);
    m_trapBuilder.init(m_traps);

    GroundShapeBuilder builder;
    m_wallShape = builder.build(m_grounds,TILE_SIZE,(int)SCALE);
    m_backgroundBuilder.init(m_backgroundElements);
    m_foregroundBuilder.init(m_foregroundElements);
    m_foregroundBuilder.init(m_foregroundElements);
}

bool GameState::hasWallAbove(float x, float y) {
    float checkX = x;
    float checkY = y - (TILE_SIZE * SCALE);
    for(const auto& wall : m_levelWalls){
        if(checkX >= wall.x&& checkX < wall.x + wall.w &&
            checkY >=wall.y&& checkY<wall.y+wall.h)
            return true;
    }
    return false;
}

bool GameState::hasWallBelow(float x, float y) {
    float checkX = x;
    float checkY = y + (TILE_SIZE * SCALE);
    for(const auto& wall : m_levelWalls){
        if(checkX >= wall.x&& checkX < wall.x + wall.w &&
           checkY >=wall.y&& checkY<wall.y+wall.h)
            return true;
    }
    return false;
}

bool GameState::hasWallRight(float x, float y) {
    float checkX = x+ (TILE_SIZE * SCALE);
    float checkY = y;
    for(const auto& wall : m_levelWalls){
        if(checkX >= wall.x&& checkX < wall.x + wall.w &&
           checkY >=wall.y&& checkY<wall.y+wall.h)
            return true;
    }
    return false;
}

bool GameState::hasWallLeft(float x, float y) {
    float checkX = x- (TILE_SIZE * SCALE);
    float checkY = y;
    for(const auto& wall : m_levelWalls){
        if(checkX >= wall.x&& checkX < wall.x + wall.w &&
           checkY >=wall.y&& checkY<wall.y+wall.h)
            return true;
    }
    return false;
}

bool GameState::isBlinkFrame() const {
    unsigned int elapsed = SDL_GetTicks() - PlayerDetail::getInstance().getLastHitTime();
    return (elapsed / m_blinkTimer) % 2 == 0;
}

bool SDLCALL GameState::HandleBackgroundEvents(void *userdata, SDL_Event *event) {
    auto* gameState = static_cast<GameState*>(userdata);
    if (event->type == SDL_EVENT_WILL_ENTER_BACKGROUND && !GameData::getInstance().isPaused()) {
        InputDispatcher::getInstance().setInputReleased(true);
        InputDispatcher::getInstance().inputLogClear();
        GameData::getInstance().setPaused(true);
        gameState->m_transitioning = true;
        Engine::Get().popOverlayState();
        Engine::Get().popOverlayState();
        if(GameData::getInstance().isDebugEnabled())
            Engine::Get().popOverlayState();
        Engine::Get().pushState(std::make_unique<PauseState>(gameState->m_renderer,gameState));
        return true;

    }
    else if (event->type == SDL_EVENT_DID_ENTER_BACKGROUND) {

    }
    else if (event->type == SDL_EVENT_WILL_ENTER_FOREGROUND){

    }
    return true; // Return true to keep the event in the queue for other systems
}








