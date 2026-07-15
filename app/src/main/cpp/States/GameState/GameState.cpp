
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
    for(int i=0;i<100;i++){
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
    if(gameMath::checkcollision(m_player.x,m_player.y,-2000.00,720.00f,
                                m_player.h,m_player.w,160.00f,9600.00f)){
        m_isGrounded =true;
        m_velocityY =0.0f;


    }
    for(int i=0;i<100;i++){
        if(gameMath::checkcollision(m_player.x,m_player.y,m_platforms[i].x,m_platforms[i].y,
                                    m_player.h,m_player.w,m_platforms[i].h*5,m_platforms[i].w*5)){
            m_isGrounded =true;
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
    m_platforms[0]  = {896, 350,192,48};

    m_platforms[1]  = {720,250,160,48};
    m_platforms[2]  = {1088,150,128,32};
    m_platforms[3]  = {544,50,144,32};
    m_platforms[4]  = {1312,-50,160,48};
    m_platforms[5]  = {864,-150,96,32};
    m_platforms[6]  = {320,-250,128,32};
    m_platforms[7]  = {1440,-350,112,32};
    m_platforms[8]  = {960,-450,80,32};
    m_platforms[9]  = {608,-550,128,32};

    m_platforms[10] = {224,-650,96,32};
    m_platforms[11] = {1376,-760,96,32};
    m_platforms[12] = {1024,-870,64,32};
    m_platforms[13] = {704,-980,128,32};
    m_platforms[14] = {1568,-1090,80,32};
    m_platforms[15] = {384,-1200,160,48};
    m_platforms[16] = {1152,-1310,96,32};
    m_platforms[17] = {608,-1420,64,32};
    m_platforms[18] = {1472,-1530,112,32};
    m_platforms[19] = {864,-1640,80,32};

    m_platforms[20] = {224,-1760,64,32};
    m_platforms[21] = {544,-1880,96,32};
    m_platforms[22] = {1248,-2000,128,32};
    m_platforms[23] = {1696,-2120,64,32};
    m_platforms[24] = {928,-2240,160,48};
    m_platforms[25] = {288,-2360,80,32};
    m_platforms[26] = {768,-2480,112,32};
    m_platforms[27] = {1504,-2600,64,32};
    m_platforms[28] = {1184,-2720,96,32};
    m_platforms[29] = {416,-2840,128,32};

    m_platforms[30] = {224,-2960,64,16};
    m_platforms[31] = {864,-3080,80,32};
    m_platforms[32] = {1568,-3200,64,16};
    m_platforms[33] = {1216,-3320,112,32};
    m_platforms[34] = {640,-3440,96,32};
    m_platforms[35] = {160,-3560,160,48};
    m_platforms[36] = {1024,-3680,64,16};
    m_platforms[37] = {1472,-3800,80,32};
    m_platforms[38] = {704,-3920,96,32};
    m_platforms[39] = {320,-4040,64,16};

    m_platforms[40] = {1184,-4170,128,32};
    m_platforms[41] = {1664,-4300,64,16};
    m_platforms[42] = {832,-4430,80,32};
    m_platforms[43] = {448,-4560,96,32};
    m_platforms[44] = {1344,-4690,64,16};
    m_platforms[45] = {608,-4820,160,48};
    m_platforms[46] = {224,-4950,80,32};
    m_platforms[47] = {1056,-5080,64,16};
    m_platforms[48] = {1568,-5210,96,32};
    m_platforms[49] = {896,-5340,128,32};

    m_platforms[50] = {512,-5470,96,32};
    m_platforms[51] = {1760,-5600,64,16};
    m_platforms[52] = {1184,-5730,80,32};
    m_platforms[53] = {320,-5860,112,32};
    m_platforms[54] = {832,-5990,64,16};
    m_platforms[55] = {1472,-6120,160,48};
    m_platforms[56] = {1056,-6250,80,32};
    m_platforms[57] = {576,-6380,64,16};
    m_platforms[58] = {160,-6510,96,32};
    m_platforms[59] = {1344,-6640,128,32};

    m_platforms[60] = {1792,-6770,64,16};
    m_platforms[61] = {960,-6900,80,32};
    m_platforms[62] = {416,-7030,96,32};
    m_platforms[63] = {1216,-7160,64,16};
    m_platforms[64] = {736,-7290,112,32};
    m_platforms[65] = {192,-7420,160,48};
    m_platforms[66] = {1472,-7550,80,32};
    m_platforms[67] = {1056,-7680,64,16};
    m_platforms[68] = {640,-7810,96,32};
    m_platforms[69] = {1600,-7940,64,16};

    m_platforms[70] = {896,-8070,80,32};
    m_platforms[71] = {288,-8200,112,32};
    m_platforms[72] = {1280,-8330,64,16};
    m_platforms[73] = {1760,-8460,96,32};
    m_platforms[74] = {736,-8590,64,16};
    m_platforms[75] = {384,-8720,160,48};
    m_platforms[76] = {1120,-8850,80,32};
    m_platforms[77] = {1536,-8980,64,16};
    m_platforms[78] = {640,-9110,96,32};
    m_platforms[79] = {224,-9240,64,16};

    m_platforms[80] = {928,-9370,112,32};
    m_platforms[81] = {1728,-9500,64,16};
    m_platforms[82] = {1216,-9630,80,32};
    m_platforms[83] = {512,-9760,96,32};
    m_platforms[84] = {160,-9890,64,16};
    m_platforms[85] = {1408,-10020,128,32};
    m_platforms[86] = {992,-10150,64,16};
    m_platforms[87] = {320,-10280,80,32};
    m_platforms[88] = {1184,-10410,96,32};
    m_platforms[89] = {1760,-10540,64,16};

    m_platforms[90] = {832,-10670,80,32};
    m_platforms[91] = {416,-10800,96,32};
    m_platforms[92] = {1472,-10930,64,16};
    m_platforms[93] = {640,-11060,112,32};
    m_platforms[94] = {224,-11190,80,32};
    m_platforms[95] = {1024,-11320,192,48};   // Final recovery platform
    m_platforms[96] = {1600,-11450,64,16};
    m_platforms[97] = {768,-11580,80,32};
    m_platforms[98] = {384,-11710,64,16};
    m_platforms[99] = {960,-11840,224,48};
}






