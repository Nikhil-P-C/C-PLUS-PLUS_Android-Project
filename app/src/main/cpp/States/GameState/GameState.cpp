
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
    m_player.setSize(SPRITE_WIDTH * P_scale, SPRITE_HEIGHT * P_scale);
    m_player.setPosition(100,0,m_windowH,SPRITE_HEIGHT,P_scale);
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
        SDL_FRect tileDst = {m_platforms[i].x-Camera::getInstance().getCamera().x,
                                 m_platforms[i].y-Camera::getInstance().getCamera().y,
                                 m_platforms[i].w * P_scale, m_platforms[i].h * 5};
        SDL_FRect tileSrc = {0+96, 0, 48, 16};
        SDL_RenderTexture(renderer, m_tileset, &tileSrc, &tileDst);
    }

    SDL_FRect dst = {m_player.x-Camera::getInstance().getCamera().x,m_player.y-Camera::getInstance().getCamera().y,m_player.w,m_player.h};

    SDL_FRect src = {(float) (0 + (SPRITE_WIDTH * m_currentFrame)), 0, SPRITE_WIDTH, SPRITE_HEIGHT};
    if(!m_isPlayerfacingRight)
        SDL_RenderTextureRotated(renderer, m_playerTexture, &src, &dst,0.0f,NULL,
                             SDL_FLIP_HORIZONTAL);
    else
        SDL_RenderTexture(renderer,m_playerTexture,&src,&dst);

}

void GameState::update(float dt){
    Camera::getInstance().lockCameraOn(m_player.x,m_player.y,m_player.h,m_player.w);
    if(InputDispatcher::getInstance().released){
        m_playerAction=PlayerAction::IDLE;
    }
    if(InputDispatcher::getInstance().movingLeft){
        m_isPlayerfacingRight =false;
        m_playerAction=PlayerAction::MOVINGLEFT;
        m_player.x -= 200 * dt;

    }
    if(InputDispatcher::getInstance().movingRight){
        m_isPlayerfacingRight =true;
        m_playerAction=PlayerAction::MOVINGRIGHT;
        m_player.x +=200 * dt;
    }

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
        Engine::Get().pushState(std::make_unique<PauseState>(m_renderer));
        return true;
        }
    }

    return false;
}

void GameState::setPlatform() {

    m_platforms[0] = {200,350,48,16};
    m_platforms[1] = {1800,150,48,16};
    m_platforms[2] = {400,0,48,16};
    m_platforms[3] = {1600,-100,48,16};
    m_platforms[4] = {600,-200,48,16};
    m_platforms[5] = {1400,-300,48,16};
    m_platforms[6] = {100,-400,48,16};
    m_platforms[7] = {1700,-500,48,16};
    m_platforms[8] = {300,-600,48,16};
    m_platforms[9] = {1500,-700,48,16};

    m_platforms[10] = {500,-800,48,16};
    m_platforms[11] = {1300,-900,48,16};
    m_platforms[12] = {250,-1000,48,16};
    m_platforms[13] = {1750,-1100,48,16};
    m_platforms[14] = {450,-1200,48,16};
    m_platforms[15] = {1650,-1300,48,16};
    m_platforms[16] = {650,-1400,48,16};
    m_platforms[17] = {1200,-1500,48,16};
    m_platforms[18] = {150,-1600,48,16};
    m_platforms[19] = {1850,-1700,48,16};

    m_platforms[20] = {350,-1800,48,16};
    m_platforms[21] = {1550,-1900,48,16};
    m_platforms[22] = {750,-2000,48,16};
    m_platforms[23] = {1100,-2100,48,16};
    m_platforms[24] = {50,-2200,48,16};
    m_platforms[25] = {1950,-2300,48,16};
    m_platforms[26] = {550,-2400,48,16};
    m_platforms[27] = {1450,-2500,48,16};
    m_platforms[28] = {250,-2600,48,16};
    m_platforms[29] = {1750,-2700,48,16};

    m_platforms[30] = {450,-2800,48,16};
    m_platforms[31] = {1650,-2900,48,16};
    m_platforms[32] = {850,-3000,48,16};
    m_platforms[33] = {1050,-3100,48,16};
    m_platforms[34] = {150,-3200,48,16};
    m_platforms[35] = {1850,-3300,48,16};
    m_platforms[36] = {350,-3400,48,16};
    m_platforms[37] = {1550,-3500,48,16};
    m_platforms[38] = {950,-3600,48,16};
    m_platforms[39] = {1000,-3700,48,16};

    m_platforms[40] = {200,-3800,48,16};
    m_platforms[41] = {1800,-3900,48,16};
    m_platforms[42] = {400,-4000,48,16};
    m_platforms[43] = {1600,-4100,48,16};
    m_platforms[44] = {600,-4200,48,16};
    m_platforms[45] = {1400,-4300,48,16};
    m_platforms[46] = {100,-4400,48,16};
    m_platforms[47] = {1700,-4500,48,16};
    m_platforms[48] = {300,-4600,48,16};
    m_platforms[49] = {1500,-4700,48,16};

    m_platforms[50] = {500,-4800,48,16};
    m_platforms[51] = {1300,-4900,48,16};
    m_platforms[52] = {250,-5000,48,16};
    m_platforms[53] = {1750,-5100,48,16};
    m_platforms[54] = {450,-5200,48,16};
    m_platforms[55] = {1650,-5300,48,16};
    m_platforms[56] = {650,-5400,48,16};
    m_platforms[57] = {1200,-5500,48,16};
    m_platforms[58] = {150,-5600,48,16};
    m_platforms[59] = {1850,-5700,48,16};

    m_platforms[60] = {350,-5800,48,16};
    m_platforms[61] = {1550,-5900,48,16};
    m_platforms[62] = {750,-6000,48,16};
    m_platforms[63] = {1100,-6100,48,16};
    m_platforms[64] = {50,-6200,48,16};
    m_platforms[65] = {1950,-6300,48,16};
    m_platforms[66] = {550,-6400,48,16};
    m_platforms[67] = {1450,-6500,48,16};
    m_platforms[68] = {250,-6600,48,16};
    m_platforms[69] = {1750,-6700,48,16};

    m_platforms[70] = {450,-6800,48,16};
    m_platforms[71] = {1650,-6900,48,16};
    m_platforms[72] = {850,-7000,48,16};
    m_platforms[73] = {1050,-7100,48,16};
    m_platforms[74] = {150,-7200,48,16};
    m_platforms[75] = {1850,-7300,48,16};
    m_platforms[76] = {350,-7400,48,16};
    m_platforms[77] = {1550,-7500,48,16};
    m_platforms[78] = {950,-7600,48,16};
    m_platforms[79] = {1000,-7700,48,16};

    m_platforms[80] = {200,-7800,48,16};
    m_platforms[81] = {1800,-7900,48,16};
    m_platforms[82] = {400,-8000,48,16};
    m_platforms[83] = {1600,-8100,48,16};
    m_platforms[84] = {600,-8200,48,16};
    m_platforms[85] = {1400,-8300,48,16};
    m_platforms[86] = {100,-8400,48,16};
    m_platforms[87] = {1700,-8500,48,16};
    m_platforms[88] = {300,-8600,48,16};
    m_platforms[89] = {1500,-8700,48,16};

    m_platforms[90] = {500,-8800,48,16};
    m_platforms[91] = {1300,-8900,48,16};
    m_platforms[92] = {250,-9000,48,16};
    m_platforms[93] = {1750,-9100,48,16};
    m_platforms[94] = {450,-9200,48,16};
    m_platforms[95] = {1650,-9300,48,16};
    m_platforms[96] = {650,-9400,48,16};
    m_platforms[97] = {1200,-9500,48,16};
    m_platforms[98] = {150,-9600,48,16};
    m_platforms[99] = {1850,-9700,48,16};



}






