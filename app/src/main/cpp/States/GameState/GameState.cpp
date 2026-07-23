
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

    SDL_AddEventWatch(HandleBackgroundEvents,this);
    m_renderer = renderer;
    LOGI("gamestate constructor:%p",this);
    m_windowH =GameData::getInstance().getWinHeight();
    m_windowW =GameData::getInstance().getWinWidth();
    //init player attributes
    m_player.setSize(SPRITE_WIDTH*P_scale-80.00f,SPRITE_HEIGHT*P_scale-35.00f);
    m_player.setPosition(400.00f,0.00f,m_windowH,SPRITE_HEIGHT,P_scale);
    m_player.setSpriteOffset(-40.00f,-20.00f);
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


    if(!m_yellowSkinFile)
        LOGI("couldnt load yellowskin file");
    SDL_Surface* yellowSkinSurface = IMG_Load_IO(m_yellowSkinFile,false);
//    m_yellowSkin  = SDL_CreateTextureFromSurface(renderer,yellowSkinSurface);
    if(!Engine::Get().getAssetManager().getTexture("yellowSkin"))
        LOGI("texture not  found");
    m_yellowSkin = Engine::Get().getAssetManager().getTexture("yellowSkin");
    SDL_DestroySurface(yellowSkinSurface);
    SDL_CloseIO(m_yellowSkinFile);

    if(!m_blueSkinFile)
        LOGI("couldnt load blueskin file");
    SDL_Surface* blueSkinSurface = IMG_Load_IO(m_blueSkinFile,false);
    m_blueSkin  = SDL_CreateTextureFromSurface(renderer,blueSkinSurface);
    SDL_DestroySurface(blueSkinSurface);
    SDL_CloseIO(m_blueSkinFile);

    if(!m_redSkinFile)
        LOGI("couldnt load redskin file");
    SDL_Surface* redSkinSurface = IMG_Load_IO(m_redSkinFile,false);
    m_redSkin  = SDL_CreateTextureFromSurface(renderer,redSkinSurface);
    SDL_DestroySurface(redSkinSurface);
    SDL_CloseIO(m_redSkinFile);

    if(!m_greenSkinFile)
        LOGI("couldnt load greenskin file");
    SDL_Surface* greenSkinSurface = IMG_Load_IO(m_greenSkinFile,false);
    m_greenSkin  = SDL_CreateTextureFromSurface(renderer,greenSkinSurface);
    SDL_DestroySurface(greenSkinSurface);
    SDL_CloseIO(m_greenSkinFile);

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
    // m_tileset loading
    SDL_Surface* Tilesurface = IMG_Load_IO(m_tilesetSprite, false);
    if(!Tilesurface)return;
    m_tileset = SDL_CreateTextureFromSurface(renderer, Tilesurface);
    if(!m_tileset)return;
    SDL_DestroySurface(Tilesurface);

    SDL_SetTextureScaleMode(m_playerTexture, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_tileset, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_playerNameTextue,SDL_SCALEMODE_NEAREST);

    setLevel(0);
}

void GameState::render(SDL_Renderer* renderer)  {
    int camX = (int)std::round(Camera::getInstance().getCamera().x);
    int camY = (int)std::round(Camera::getInstance().getCamera().y);
    SDL_FRect backgroundDst{static_cast<float>(0-camX), static_cast<float>(0-camY), 1600, 720};
    SDL_RenderTexture(renderer, m_backGround, nullptr, &backgroundDst);
    for(const auto& level :m_levelWalls)
    {
        int tileSize =(TILE_SIZE*SCALE);
        const int platformWidth = (int)level.w;
        const int platformHeight = (int)level.h;
        int widthTiles = static_cast<int>(std::ceil(platformWidth  / (float)tileSize));
        int heightTiles = static_cast<int>(std::ceil(platformHeight / (float)tileSize));
        for (int i = 0; i < heightTiles; i++) {
            for (int j = 0; j < widthTiles; j++) {
                SDL_FRect src;
                float x =level.x + j * (TILE_SIZE * SCALE);
                float y =level.y + i* (TILE_SIZE * SCALE);
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
                        x - camX,
                        y - camY,
                        TILE_SIZE * SCALE, TILE_SIZE * SCALE};

                if ((top || bottom || left || right )&& !edge) {
                    SDL_RenderTextureRotated(renderer, m_tileset, &src, &dst,0.0f, nullptr,SDL_FLIP_HORIZONTAL_AND_VERTICAL);
                }
                else if(((top || bottom) && (left || right) && edge)){
                    SDL_RenderTexture(renderer, m_tileset, &src, &dst);
                }
//                SDL_SetRenderDrawColor(renderer,255,0,0,255);
//                SDL_RenderRect(renderer,&dst);
            }
        }
    }
    for(auto const& tile:m_wallShape.tiles){
        float  srcX,srcY,srcW,srcH;

        SDL_FRect src = tile.src;
        SDL_FRect dst{tile.x-camX,tile.y-camY,tile.w,tile.h};
        SDL_RenderTexture(renderer,m_tileset,&src,&dst);
//        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
//        SDL_RenderRect(renderer, &dst);
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
//                SDL_RenderRect(renderer,&dst);
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
    //player name rendering
    SDL_FRect playerNameDst{m_player.x+m_player.spriteOffsetX-camX+(12.50f*P_scale)
                            -(50+1*static_cast<float>(PlayerDetail::getInstance().getPlayerName().length()))/2,
                            m_player.y+m_player.spriteOffsetY-camY-(2.00f*P_scale) -10,
                            50+1*static_cast<float>(PlayerDetail::getInstance().getPlayerName().length()),
                            45.00f};
    SDL_RenderTexture(renderer,m_playerNameTextue, nullptr,&playerNameDst);
}

void GameState::update(float dt){

    m_previousY =m_player.y;
    handlePhysicAndInput(dt);

    m_isGrounded =false;
    handleCollision();

    Camera::getInstance().lockCameraOn(m_player.x,m_player.y,m_player.h,m_player.w);

    updateAnimation();

}

bool GameState::handleEvents(SDL_Event& event) {
//    if (m_transitioning)return true;
    if(event.type == SDL_EVENT_KEY_DOWN)
    {
        if(event.key.key == SDLK_AC_BACK){
            GameData::getInstance().setPaused(true);
            m_transitioning = true;
            LOGI("game state transitions to menu state");
            Engine::Get().popOverlayState();
            if(GameData::getInstance().isDebugEnabled())
                Engine::Get().popOverlayState();
            Engine::Get().pushState(std::make_unique<PauseState>(m_renderer,this));
            return true;
        }

    }

    return false;
}

void GameState::handleCollision() {
    //Walls

    for(int i = 0;i<1;i++)
    {
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
    }

//platforms
    for(int i=0;i<10;i++) {
        if (m_platforms[i].colliderType == ColliderType::SOLID) {

            gameMath::collisionSide side = gameMath::checkcollisionXY(m_player.x, m_player.y,
                                                                      m_platforms[i].x,
                                                                      m_platforms[i].y,
                                                                      m_player.h, m_player.w,
                                                                      m_platforms[i].h * SCALE,
                                                                      m_platforms[i].w * SCALE);
            if (side == gameMath::collisionSide::TOP) {

                m_velocityY = 0.0f;
                m_isGrounded =true;
            }
            if (side == gameMath::collisionSide::BOTTOM) {
                m_velocityY = 0.0f;
            }
        } else if (m_platforms[i].colliderType == ColliderType::ONE_WAY) {
            float previousBottom = m_previousY + m_player.h;
            float currentBottom = m_player.y + m_player.h;
            float platformTop = m_platforms[i].y;

            if (m_velocityY > 0 && previousBottom <= platformTop
                && currentBottom >= platformTop
                && gameMath::checkcollisionX(m_player.x, m_player.y, m_platforms[i].x,
                                             m_platforms[i].y,
                                             m_player.h, m_player.w, m_platforms[i].h * SCALE,
                                             m_platforms[i].w * SCALE)) {

                m_player.y = platformTop - m_player.h;
                m_velocityY = 0.0f;
                m_isGrounded = true;
            }
        }

    }
    //ground

    for(const auto& ground : m_grounds){
        gameMath::collisionSide groundCollisionSide = gameMath::checkcollisionXY(m_player.x,
                                                                               m_player.y,
                                                                               ground.x,
                                                                               ground.y,
                                                                               m_player.h,
                                                                               m_player.w,
                                                                               ground.h*SCALE,
                                                                               ground.w*SCALE);

        if (groundCollisionSide == gameMath::collisionSide::TOP) {
            m_isGrounded = true;
            m_velocityY = 0.0f;
        }
        if(groundCollisionSide == gameMath::collisionSide::BOTTOM){
            m_velocityY =0.0f;
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
    m_levelWalls.emplace_back(0,0,1600,384,SpriteType::STONE_BRICK_WALL,ColliderType::SOLID);
    m_levelWalls.emplace_back(0,384,1600,384,SpriteType::MOSS_WALL,ColliderType::SOLID);
    m_wallCollisionRect={0.00f,0.00f,1600.00f,768};

    m_platforms.emplace_back(0,600,128,16,ColliderType::ONE_WAY,SpriteType::WOODEN_PLATFORM);
    m_platforms.emplace_back(320,520,208,16,ColliderType::ONE_WAY,SpriteType::STONE_PLATFORM);
    m_platforms.emplace_back(640,440,192,16,ColliderType::ONE_WAY,SpriteType::GOLD_PLATFORM);
    m_platforms.emplace_back(960,360,224,16,ColliderType::ONE_WAY,SpriteType::WOODEN_PLATFORM);
    m_platforms.emplace_back(1280,280,128,16,ColliderType::ONE_WAY,SpriteType::GOLD_PLATFORM);
    m_platforms.emplace_back(1600,200,160,16,ColliderType::ONE_WAY,SpriteType::WOODEN_PLATFORM);
    m_platforms.emplace_back(1920,120,192,16,ColliderType::ONE_WAY,SpriteType::GOLD_PLATFORM);
    m_platforms.emplace_back(2240,40,224,16,ColliderType::ONE_WAY,SpriteType::WOODEN_PLATFORM);
    m_platforms.emplace_back(2560,-40,128,16,ColliderType::ONE_WAY,SpriteType::GOLD_PLATFORM);
    m_platforms.emplace_back(2880,-120,160,16,ColliderType::ONE_WAY,SpriteType::WOODEN_PLATFORM);
    m_platforms.emplace_back(3200,-200,192,16,ColliderType::ONE_WAY,SpriteType::GOLD_PLATFORM);

    m_grounds.emplace_back(64.0f,384.00f,48.00f,16.00f,SpriteType::GREEN_GRASS_GROUND,ColliderType::SOLID);
    m_grounds.emplace_back(64.0f,448.00f,64.00f,64.00f,SpriteType::GREEN_GRASS_GROUND,ColliderType::SOLID);
    m_grounds.emplace_back(704.0f,320.00f,48.00f,48.00f,SpriteType::PINK_GRASS_GROUND,ColliderType::SOLID);
    m_grounds.emplace_back(640.0f,512.00f,160.00f,48.00f,SpriteType::PINK_GRASS_GROUND,ColliderType::SOLID);


    GroundShapeBuilder builder;
    m_wallShape = builder.build(m_grounds,TILE_SIZE,SCALE);
    if(GameData::getInstance().isDebugEnabled()){
        Engine::Get().pushOverlayState(std::make_unique<DebugState>(m_renderer,this));
    }
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

bool SDLCALL GameState::HandleBackgroundEvents(void *userdata, SDL_Event *event) {
//    LOGI("Lifecycle event: 0x%X", event->type);
    auto* gameState = static_cast<GameState*>(userdata);
    if (event->type == SDL_EVENT_WILL_ENTER_BACKGROUND && !GameData::getInstance().isPaused()) {
        gameState->m_transitioning = true;
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







