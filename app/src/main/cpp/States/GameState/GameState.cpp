
#include "GameState.h"
#include "States/MenuState/TitleScreenState.h"
#include "States/PauseState/PauseState.h"
#include "States/DebugState/DebugState.h"
#include "utils/utils.h"
#include <cmath>


//
// Created by LENOVO on 27-04-2026.
//
GameState::GameState(SDL_Renderer *renderer) {


    m_renderer = renderer;
    LOGI("gamestate constructor:%p",this);
    m_windowH =GameData::getInstance().getWinHeight();
    m_windowW =GameData::getInstance().getWinWidth();
    //init player attributes
    m_player.setSize(SPRITE_WIDTH*P_scale-55.00f,SPRITE_HEIGHT*P_scale-35.00f);
    m_player.setPosition(100,0,m_windowH,SPRITE_HEIGHT,P_scale);
    m_player.setSpriteOffset(-35.00f,-20.00f);
    m_player.setSpriteSize(SPRITE_WIDTH*P_scale,SPRITE_HEIGHT*P_scale);
    Camera::getInstance().setSize(m_windowW,m_windowH);


    //font loading
    m_font = TTF_OpenFontIO(m_fontFile, false, 36);


    //background sprite loading;
    SDL_Surface* Backgroundsurface = IMG_Load_IO(m_backGroundSprite, false);
    if(!Backgroundsurface)return;
    m_backGround = SDL_CreateTextureFromSurface(renderer, Backgroundsurface);
    if(!m_backGround)return;
    SDL_DestroySurface(Backgroundsurface);

    //player sprite loading
    SDL_Surface* playerSurface = IMG_Load_IO(m_playerSprite, false);
    if(!playerSurface)return;
    m_playerTexture = SDL_CreateTextureFromSurface(renderer, playerSurface);
    if(!m_playerTexture)return;
    SDL_DestroySurface(playerSurface);

    // m_tileset loading
    SDL_Surface* Tilesurface = IMG_Load_IO(m_tilesetSprite, false);
    if(!Tilesurface)return;
    m_tileset = SDL_CreateTextureFromSurface(renderer, Tilesurface);
    if(!m_tileset)return;
    SDL_DestroySurface(Tilesurface);

    SDL_SetTextureScaleMode(m_playerTexture, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_tileset, SDL_SCALEMODE_NEAREST);


    setLevel(0);
}

void GameState::render(SDL_Renderer* renderer)  {
    int camX = (int)std::round(Camera::getInstance().getCamera().x);
    int camY = (int)std::round(Camera::getInstance().getCamera().y);
    SDL_FRect backgroundDst{static_cast<float>(0-camX), static_cast<float>(0-camY), 1600, 720};
    SDL_RenderTexture(renderer, m_backGround, nullptr, &backgroundDst);

    {
        int tileSize =(TILE_SIZE*SCALE);
        const int platformWidth = (int)m_levelWalls.w;
        const int platformHeight = (int)m_levelWalls.h;
        int widthTiles = static_cast<int>(std::ceil(platformWidth  / (float)tileSize));
        int heightTiles = static_cast<int>(std::ceil(platformHeight / (float)tileSize));
        LOGI("Total  height tile:%d , total Width tile: %d ",heightTiles,widthTiles);
        for (int y = 0; y < heightTiles; y++) {
            for (int x = 0; x < widthTiles; x++) {
                SDL_FRect src;

                bool left = (x == 0);
                bool right = (x == widthTiles - 1);
                bool top = (y == 0);
                bool bottom = (y == heightTiles - 1);

                bool edge =false;
                if (top) {
                    if (left) {
                        src = {SpriteCollection::mossWallsEdges.x,
                               0, TILE_SIZE, TILE_SIZE};
                        edge =true;
                        }
                    else if (right) {
                        src = {SpriteCollection::mossWallsEdges.x + TILE_SIZE,
                               0, TILE_SIZE, TILE_SIZE};
                        edge =true;
                    }
                    else {
                        src = {SpriteCollection::mossWalls.x + TILE_SIZE,
                               0, TILE_SIZE, TILE_SIZE};
                        edge = false;
                    }
                }
                else if (bottom) {
                    if (left) {
                        src = {SpriteCollection::mossWallsEdges.x,
                               0+TILE_SIZE, TILE_SIZE, TILE_SIZE};
                        edge =true;
                    }
                    else if (right) {
                        src = {SpriteCollection::mossWallsEdges.x + TILE_SIZE,
                               0+TILE_SIZE, TILE_SIZE, TILE_SIZE};
                        edge= true;
                    }
                    else {
                        src = {SpriteCollection::mossWalls.x + TILE_SIZE,
                               0+32.00f, TILE_SIZE, TILE_SIZE};
                        edge =false;
                    }
                } else {
                    edge =false;
                    if (left) src = {SpriteCollection::mossWalls.x,
                                     0+ TILE_SIZE, TILE_SIZE, TILE_SIZE};
                    else if (right)
                        src = {SpriteCollection::mossWalls.x + 32.00f,
                               0+TILE_SIZE, TILE_SIZE, TILE_SIZE};
                    else src = {SpriteCollection::mossWalls.x + TILE_SIZE,
                                0+TILE_SIZE, TILE_SIZE, TILE_SIZE};
                }

//          SDL_FRect dst = {m_platforms[j].x + j * (48*5), m_platforms[j].y, 48*5, 48*5};
//                LOGI("TOP:%d,BOTTOM:%d,LEFT:%d,RIGHT:%d,Edge:%d",top,bottom,left,right,edge);

                if ((top || bottom || left || right )&& !edge) {
                    SDL_FRect dst = {
                            (m_levelWalls.x + x * (TILE_SIZE * SCALE)) - camX,
                            (m_levelWalls.y + y * (TILE_SIZE * SCALE)) - camY,
                            TILE_SIZE * SCALE, TILE_SIZE * SCALE};
                    SDL_RenderTextureRotated(renderer, m_tileset, &src, &dst,0.0f,NULL,SDL_FLIP_HORIZONTAL_AND_VERTICAL);
                }
                else if(((top || bottom) && (left || right) && edge)){
                    SDL_FRect dst = {
                            (m_levelWalls.x + x * (TILE_SIZE * SCALE)) - camX,
                            (m_levelWalls.y + y * (TILE_SIZE * SCALE)) - camY,
                            TILE_SIZE * SCALE, TILE_SIZE * SCALE};

                    SDL_RenderTexture(renderer, m_tileset, &src, &dst);

                }

            }
        }
    }
    for(int i=0;i<10;i++){
        int tileSize =TILE_SIZE;
        const int platformWidth = (int)m_platforms[i].w;
        const int platformHeight = (int)m_platforms[i].h;
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
                    if(m_platforms[i].platformType == SpriteType::GOLD_PLATFORM){
                        if (left) src = {SpriteCollection::goldPlatform.x, 0.00f, TILE_SIZE, TILE_SIZE};
                        else if (right)
                            src = {SpriteCollection::goldPlatform.x + 32.00f, 0.00f, TILE_SIZE,
                                   TILE_SIZE};
                        else
                            src = {SpriteCollection::goldPlatform.x + TILE_SIZE, 0.00f, TILE_SIZE,
                                   TILE_SIZE};
                    }
                    else if(m_platforms[i].platformType == SpriteType::WOODEN_PLATFORM){
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
                    else if(m_platforms[i].platformType == SpriteType::STONE_PLATFORM) {
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

//          SDL_FRect dst = {m_platforms[j].x + j * (48*SCALE), m_platforms[j].y, 48*SCALE, 48*SCALE};
                SDL_FRect dst = {(m_platforms[i].x+x* (TILE_SIZE * SCALE)) - camX,
                                 ( m_platforms[i].y+y * (TILE_SIZE * SCALE)) - camY,
                                 TILE_SIZE * SCALE, TILE_SIZE * SCALE};
                SDL_RenderTexture(renderer, m_tileset, &src, &dst);
                SDL_RenderRect(renderer,&dst);
            }

        }

    }

    SDL_FRect playerBorder{m_player.x-camX,
                           m_player.y-camY,
                           m_player.w, m_player.h};
    SDL_FRect dst = {m_player.x+m_player.spriteOffsetX-camX,
                     m_player.y+m_player.spriteOffsetY-camY,
                     m_player.spriteW,m_player.spriteH};

    SDL_FRect src = {(float) (0 + (SPRITE_WIDTH * m_currentFrame)), 0, SPRITE_WIDTH, SPRITE_HEIGHT};
    if(!m_isPlayerfacingRight)
        SDL_RenderTextureRotated(renderer, m_playerTexture, &src, &dst,0.0f,NULL,
                             SDL_FLIP_HORIZONTAL);
    else
        SDL_RenderTexture(renderer,m_playerTexture,&src,&dst);
    SDL_SetRenderDrawColor(renderer,33,31,48,255);
    SDL_RenderRect(renderer, &playerBorder);

}

void GameState::update(float dt){

    m_isGrounded =true;
    m_previousY =m_player.y;
    handlePhysicAndInput(dt);

    handleCollision();

    Camera::getInstance().lockCameraOn(m_player.x,m_player.y,m_player.h,m_player.w);

    updateAnimation();

}

bool GameState::handleEvents(SDL_Event& event) {
//    if (m_transitioning)return true;
    if(event.type == SDL_EVENT_KEY_DOWN)
    {
        if(event.key.key == SDLK_AC_BACK){
        m_transitioning = true;
        LOGI("game state transitions to menu state");
        Engine::Get().popOverlayState();
        if(GameData::getInstance().isDebugEnabled())
            Engine::Get().popOverlayState();
        Engine::Get().pushState(std::make_unique<PauseState>(m_renderer));
        return true;
        }
    }

    return false;
}

void GameState::handleCollision() {
    const float renderedHeight = (std::ceil(m_levelWalls.h / (SCALE*TILE_SIZE)) *(SCALE *TILE_SIZE));
    LOGI("rendered height :%f",renderedHeight);
    gameMath::collisionSide wallCollisionSide = gameMath::checkcollisionXY(m_player.x,m_player.y,m_levelWalls.x,m_levelWalls.y,
                                                                         m_player.h,m_player.w,TILE_SIZE*SCALE,m_levelWalls.w);
    if(wallCollisionSide ==  gameMath::collisionSide::BOTTOM)
        m_velocityY =0.0f;
    wallCollisionSide= gameMath::checkcollisionXY(m_player.x,m_player.y,m_levelWalls.x,m_levelWalls.y,
                                                m_player.h,m_player.w,renderedHeight,TILE_SIZE*SCALE);
    wallCollisionSide= gameMath::checkcollisionXY(m_player.x,m_player.y,m_levelWalls.x+m_levelWalls.w-TILE_SIZE*SCALE,m_levelWalls.y,
                                                m_player.h,m_player.w,renderedHeight,TILE_SIZE*SCALE);
    wallCollisionSide =gameMath::checkcollisionXY(m_player.x,m_player.y,m_levelWalls.x,m_levelWalls.y+renderedHeight-TILE_SIZE*SCALE,
                                                m_player.h,m_player.w,TILE_SIZE*SCALE,m_levelWalls.w);
    if(wallCollisionSide == gameMath::collisionSide::TOP){
        m_isGrounded = true;
        m_velocityY =0.0f;
    }



    for(int i=0;i<10;i++){
        if(m_platforms[i].colliderType == ColliderType::SOLID)
        {

            gameMath::collisionSide side = gameMath::checkcollisionXY(m_player.x, m_player.y,
                                                                    m_platforms[i].x,
                                                                    m_platforms[i].y,
                                                                    m_player.h, m_player.w,
                                                                    m_platforms[i].h * SCALE,
                                                                    m_platforms[i].w * SCALE);
            if (side == gameMath::collisionSide::TOP) {

                m_velocityY = 0.0f;
//          m_isGrounded =true;
            }
            if (side == gameMath::collisionSide::BOTTOM) {
                m_velocityY = 0.0f;
            }
        }
        else if(m_platforms[0].colliderType == ColliderType::ONE_WAY){
            float previousBottom = m_previousY + m_player.h;
            float currentBottom = m_player.y + m_player.h;
            float platformTop = m_platforms[i].y;

            if(m_velocityY >0 &&previousBottom <= platformTop
                && currentBottom>=platformTop
                &&gameMath::checkcollisionX(m_player.x, m_player.y,m_platforms[i].x,m_platforms[i].y,
                                            m_player.h, m_player.w,m_platforms[i].h * SCALE,m_platforms[i].w * SCALE))
            {

                m_player.y = platformTop - m_player.h;
                m_velocityY = 0.0f;
                m_isGrounded = true;
            }
        }

    }
}

void GameState::updateAnimation() {
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
}

void GameState::handlePhysicAndInput(float dt) {

    if(!InputDispatcher::getInstance().movingLeft&&!InputDispatcher::getInstance().movingRight){
        m_playerAction=PlayerAction::IDLE;
    }
    if(InputDispatcher::getInstance().movingLeft){
        m_isPlayerfacingRight =false;
        m_playerAction=PlayerAction::MOVINGLEFT;
        m_player.x -= 250.00f* dt;

    }
    if(InputDispatcher::getInstance().movingRight){
        m_isPlayerfacingRight =true;
        m_playerAction=PlayerAction::MOVINGRIGHT;
        m_player.x +=250.00f * dt;
    }
    if(InputDispatcher::getInstance().jump && m_isGrounded){

        m_playerAction=PlayerAction::JUMP;
        m_velocityY =-m_jumpVelocity;
    }

    m_velocityY+=m_gravity*dt;
    m_player.y +=m_velocityY*dt;
}

void GameState::setLevel(int level) {
    m_levelWalls ={0.00f,0.00f,1600.00f,720.00f};
    m_platforms[0]  = {0,600,128,16,ColliderType::ONE_WAY,SpriteType::WOODEN_PLATFORM};
    m_platforms[1]  = {320,520,208,16,ColliderType::ONE_WAY,SpriteType::STONE_PLATFORM};
    m_platforms[2]  = {640,440,192,16,ColliderType::ONE_WAY,SpriteType::GOLD_PLATFORM};
    m_platforms[3]  = {960,360,224,16,ColliderType::ONE_WAY,SpriteType::WOODEN_PLATFORM};
    m_platforms[4]  = {1280,280,128,16,ColliderType::ONE_WAY,SpriteType::GOLD_PLATFORM};
    m_platforms[5]  = {1600,200,160,16,ColliderType::ONE_WAY,SpriteType::WOODEN_PLATFORM};
    m_platforms[6]  = {1920,120,192,16,ColliderType::ONE_WAY,SpriteType::GOLD_PLATFORM};
    m_platforms[7]  = {2240,40,224,16,ColliderType::ONE_WAY,SpriteType::WOODEN_PLATFORM};
    m_platforms[8]  = {2560,-40,128,16,ColliderType::ONE_WAY,SpriteType::GOLD_PLATFORM};
    m_platforms[9]  = {2880,-120,160,16,ColliderType::ONE_WAY,SpriteType::WOODEN_PLATFORM};
    m_platforms[10] = {3200,-200,192,16,ColliderType::ONE_WAY,SpriteType::GOLD_PLATFORM};

}






