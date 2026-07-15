
#include "GameState.h"
#include "States/MenuState/TitleScreenState.h"
#include "States/PauseState/PauseState.h"
#include "States/DebugState/DebugState.h"
#include "utils/utils.h"


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


    setPlatform();
}

void GameState::render(SDL_Renderer* renderer)  {
    SDL_FRect backgroundDst{0, 0, 1600, 720};
    SDL_RenderTexture(renderer, m_backGround, nullptr, &backgroundDst);
    for(int i=0;i<50;i++){
        int tileSize = 16;
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

                if (top) {
                    if (left) src = {96.00f, 0.00f, 16.00f, 16.00f};
                    else if (right) src = {96.00f + 32.00f, 0.00f, 16.00f, 16.00f};
                    else src = {96.00f + 16.00f, 0.00f, 16.00f, 16.00f};
                } else if (bottom) {
                    if (left) src = {96.00f, 32.00f, 16.00f, 16.00f};
                    else if (right) src = {96.00f + 32.00f, 32.00f, 16.00f, 16.00f};
                    else src = {96.00f + 16.00f, 32.00f, 16.00f, 16.00f};
                } else {
                    if (left) src = {96.00f, 16.00f, 16.00f, 16.00f};
                    else if (right) src = {96.00f + 32.00f, 16.00f, 16.00f, 16.00f};
                    else src = {96.00f + 16.00f, 16.00f, 16.00f, 16.00f};
                }
                if(heightTiles ==1){
                    if(left) src = {96.00f+96.00, 0.00f, 16.00f, 16.00f};
                    else if (right) src = {96.00f +96.00+ 32.00f, 0.00f, 16.00f, 16.00f};
                    else src = {96.00f +96.00f+ 16.00f, 0.00f, 16.00f, 16.00f};
                }

//          SDL_FRect dst = {m_platforms[j].x + j * (48*5), m_platforms[j].y, 48*5, 48*5};
                SDL_FRect dst = {(m_platforms[i].x+x* (16 * 5)) - Camera::getInstance().getCamera().x,
                                 ( m_platforms[i].y+y * (16 * 5)) - Camera::getInstance().getCamera().y,
                                 16 * 5, 16 * 5};
                SDL_RenderTexture(renderer, m_tileset, &src, &dst);
            }

        }


        SDL_FRect tileDst = {m_platforms[i].x-Camera::getInstance().getCamera().x,
                                 m_platforms[i].y-Camera::getInstance().getCamera().y,
                                 m_platforms[i].w * 5, m_platforms[i].h * 5};

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderRect(renderer, &tileDst);
    }
    int tileSize = 16;
    const int platformWidth = 1600;
    const int platformHeight = 160;
    int widthTiles = platformWidth / tileSize;
    int heightTiles = platformHeight / tileSize;
    for (int y = 0; y < heightTiles; y++)
    {
        for (int x = 0; x < widthTiles; x++)
        {
            SDL_FRect src;

            bool left   = (x == 0);
            bool right  = (x == widthTiles - 1);
            bool top    = (y == 0);
            bool bottom = (y == heightTiles - 1);

            if (top)
            {
                if (left)      src = {96.00f,0.00f,16.00f,16.00f};
                else if (right) src = {96.00f+32.00f,0.00f,16.00f,16.00f};
                else            src = {96.00f+16.00f,0.00f,16.00f,16.00f};
            }
            else if (bottom)
            {
                if (left)      src = {96.00f,32.00f,16.00f,16.00f};
                else if (right) src = {96.00f+32.00f,32.00f,16.00f,16.00f};
                else            src = {96.00f+16.00f,32.00f,16.00f,16.00f};
            }
            else
            {
                if (left)      src = {96.00f,16.00f,16.00f,16.00f};
                else if (right) src = {96.00f+32.00f,16.00f,16.00f,16.00f};
                else            src = {96.00f+16.00f,16.00f,16.00f,16.00f};
            }


//          SDL_FRect dst = {m_platforms[j].x + j * (48*5), m_platforms[j].y, 48*5, 48*5};
            SDL_FRect dst = {(-2000.00f + x * (16*5))-Camera::getInstance().getCamera().x
                         ,(720.00f+y*(16*5))-Camera::getInstance().getCamera().y, 16*5, 16*5};
            SDL_RenderTexture(renderer, m_tileset, &src, &dst);
        }
    }
    SDL_FRect playerBorder{m_player.x-Camera::getInstance().getCamera().x,
                           m_player.y-Camera::getInstance().getCamera().y,
                           m_player.w, m_player.h};
    SDL_FRect dst = {m_player.x+m_player.spriteOffsetX-Camera::getInstance().getCamera().x,
                     m_player.y+m_player.spriteOffsetY-Camera::getInstance().getCamera().y,
                     m_player.spriteW,m_player.spriteH};

    SDL_FRect src = {(float) (0 + (SPRITE_WIDTH * m_currentFrame)), 0, SPRITE_WIDTH, SPRITE_HEIGHT};
    if(!m_isPlayerfacingRight)
        SDL_RenderTextureRotated(renderer, m_playerTexture, &src, &dst,0.0f,NULL,
                             SDL_FLIP_HORIZONTAL);
    else
        SDL_RenderTexture(renderer,m_playerTexture,&src,&dst);
    SDL_RenderRect(renderer, &playerBorder);
}

void GameState::update(float dt){
    m_isGrounded =false;
    //ground check
    LOGI("velocity y :%f",m_velocityY);
    gameMath::collisionSide currentSide = gameMath::checkcollision(m_player.x,m_player.y,-2000.00,720.00f,
                                                                   m_player.h,m_player.w,160.00f,9600.00f);
    if(currentSide == gameMath::collisionSide::TOP){
        m_isGrounded =true;
        m_velocityY =0.0f;
    }
    if(currentSide == gameMath::collisionSide::BOTTOM){
        m_velocityY =0.0f;
    }


    for(int i=0;i<50;i++){
        gameMath::collisionSide side = gameMath::checkcollision(m_player.x,m_player.y,m_platforms[i].x,m_platforms[i].y,
                                                                m_player.h,m_player.w,m_platforms[i].h*5,m_platforms[i].w*5);
        if(side == gameMath::collisionSide::TOP){

            m_velocityY =0.0f;
            m_isGrounded =true;
        }
        if(side == gameMath::collisionSide::BOTTOM){
            m_velocityY =0.0f;
        }

    }

    Camera::getInstance().lockCameraOn(m_player.x,m_player.y,m_player.h,m_player.w);
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

void GameState::setPlatform() {
    m_platforms[0]  = {0,600,128,16};
    m_platforms[1]  = {320,520,160,32};
    m_platforms[2]  = {640,440,192,48};
    m_platforms[3]  = {960,360,224,16};
    m_platforms[4]  = {1280,280,128,32};
    m_platforms[5]  = {1600,200,160,48};
    m_platforms[6]  = {1920,120,192,16};
    m_platforms[7]  = {2240,40,224,32};
    m_platforms[8]  = {2560,-40,128,48};
    m_platforms[9]  = {2880,-120,160,16};
    m_platforms[10] = {3200,-200,192,32};
    m_platforms[11] = {3520,-280,224,48};
    m_platforms[12] = {3840,-360,128,16};
    m_platforms[13] = {4160,-440,160,32};
    m_platforms[14] = {4480,-520,192,48};
    m_platforms[15] = {4800,-600,224,16};
    m_platforms[16] = {5120,-680,128,32};
    m_platforms[17] = {5440,-760,160,48};
    m_platforms[18] = {5760,-840,192,16};
    m_platforms[19] = {6080,-920,224,32};
    m_platforms[20] = {6400,-1000,128,48};
    m_platforms[21] = {6720,-1080,160,16};
    m_platforms[22] = {7040,-1160,192,32};
    m_platforms[23] = {7360,-1240,224,48};
    m_platforms[24] = {7680,-1320,128,16};
    m_platforms[25] = {8000,-1400,160,32};
    m_platforms[26] = {8320,-1480,192,48};
    m_platforms[27] = {8640,-1560,224,16};
    m_platforms[28] = {8960,-1640,128,32};
    m_platforms[29] = {9280,-1720,160,48};
    m_platforms[30] = {9600,-1800,192,16};
    m_platforms[31] = {9920,-1880,224,32};
    m_platforms[32] = {10240,-1960,128,48};
    m_platforms[33] = {10560,-2040,160,16};
    m_platforms[34] = {10880,-2120,192,32};
    m_platforms[35] = {11200,-2200,224,48};
    m_platforms[36] = {11520,-2280,128,16};
    m_platforms[37] = {11840,-2360,160,32};
    m_platforms[38] = {12160,-2440,192,48};
    m_platforms[39] = {12480,-2520,224,16};
    m_platforms[40] = {12800,-2600,128,32};
    m_platforms[41] = {13120,-2680,160,48};
    m_platforms[42] = {13440,-2760,192,16};
    m_platforms[43] = {13760,-2840,224,32};
    m_platforms[44] = {14080,-2920,128,48};
    m_platforms[45] = {14400,-3000,160,16};
    m_platforms[46] = {14720,-3080,192,32};
    m_platforms[47] = {15040,-3160,224,48};
    m_platforms[48] = {15360,-3240,128,16};
    m_platforms[49] = {15680,-3320,160,32};
    m_platforms[50] = {16000,-3400,192,48};
    m_platforms[51] = {16320,-3480,224,16};
    m_platforms[52] = {16640,-3560,128,32};
    m_platforms[53] = {16960,-3640,160,48};
    m_platforms[54] = {17280,-3720,192,16};
    m_platforms[55] = {17600,-3800,224,32};
    m_platforms[56] = {17920,-3880,128,48};
    m_platforms[57] = {18240,-3960,160,16};
    m_platforms[58] = {18560,-4040,192,32};
    m_platforms[59] = {18880,-4120,224,48};
    m_platforms[60] = {19200,-4200,128,16};
    m_platforms[61] = {19520,-4280,160,32};
    m_platforms[62] = {19840,-4360,192,48};
    m_platforms[63] = {20160,-4440,224,16};
    m_platforms[64] = {20480,-4520,128,32};
    m_platforms[65] = {20800,-4600,160,48};
    m_platforms[66] = {21120,-4680,192,16};
    m_platforms[67] = {21440,-4760,224,32};
    m_platforms[68] = {21760,-4840,128,48};
    m_platforms[69] = {22080,-4920,160,16};
    m_platforms[70] = {22400,-5000,192,32};
    m_platforms[71] = {22720,-5080,224,48};
    m_platforms[72] = {23040,-5160,128,16};
    m_platforms[73] = {23360,-5240,160,32};
    m_platforms[74] = {23680,-5320,192,48};
    m_platforms[75] = {24000,-5400,224,16};
    m_platforms[76] = {24320,-5480,128,32};
    m_platforms[77] = {24640,-5560,160,48};
    m_platforms[78] = {24960,-5640,192,16};
    m_platforms[79] = {25280,-5720,224,32};
    m_platforms[80] = {25600,-5800,128,48};
    m_platforms[81] = {25920,-5880,160,16};
    m_platforms[82] = {26240,-5960,192,32};
    m_platforms[83] = {26560,-6040,224,48};
    m_platforms[84] = {26880,-6120,128,16};
    m_platforms[85] = {27200,-6200,160,32};
    m_platforms[86] = {27520,-6280,192,48};
    m_platforms[87] = {27840,-6360,224,16};
    m_platforms[88] = {28160,-6440,128,32};
    m_platforms[89] = {28480,-6520,160,48};
    m_platforms[90] = {28800,-6600,192,16};
    m_platforms[91] = {29120,-6680,224,32};
    m_platforms[92] = {29440,-6760,128,48};
    m_platforms[93] = {29760,-6840,160,16};
    m_platforms[94] = {30080,-6920,192,32};
    m_platforms[95] = {30400,-7000,224,48};
    m_platforms[96] = {30720,-7080,128,16};
    m_platforms[97] = {31040,-7160,160,32};
    m_platforms[98] = {31360,-7240,192,48};
    m_platforms[99] = {31680,-7320,224,16};
}






