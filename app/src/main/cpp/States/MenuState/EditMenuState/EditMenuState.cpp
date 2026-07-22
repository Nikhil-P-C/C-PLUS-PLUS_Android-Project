//
// Created by LENOVO on 21-07-2026.
//
#include "EditMenuState.h"
#include "engine/Engine.h"
#include "States/MenuState/MenuState.h"


void EditMenuState::render(SDL_Renderer *renderer)
{
    SDL_FRect backgroundDst {0.00f,0.00f,1600.00f,720.00f};
    SDL_RenderTexture(renderer,m_backgroundTexture, nullptr,&backgroundDst);

    SDL_FRect editMenuSrc {0,0,682,385};
    SDL_FRect editMenuDst {50,25,1500,675};
    SDL_RenderTexture(renderer,m_editMenuTexture,&editMenuSrc,&editMenuDst);

    SDL_FRect previewTileSrc {0,0,260,265};
    SDL_FRect previewTileDst {1038,35,500,390};
    SDL_RenderTexture(renderer,m_previewTileTexture,&previewTileSrc,&previewTileDst);

    SDL_FRect nameTileSrc{0,0,100,50};
    if(m_saveButtonActive)
        nameTileSrc = {101,0,100,50};
    SDL_FRect nameTileDst {1038,420,500,270};
    SDL_RenderTexture(renderer,m_nameTileTexture,&nameTileSrc,&nameTileDst);

    SDL_FRect backButtonDst ={m_backButton.x,m_backButton.y,m_backButton.w,m_backButton.h};
    SDL_SetRenderDrawColor(renderer,0,0,255,255);
    SDL_RenderTexture(renderer,m_backButtonTexture, nullptr,&backButtonDst);

    SDL_FRect leftChevronDst{60.00f,36.00f,25.00f,25.00f};
    SDL_FRect leftChevronSrc{0.00f,0.00f,9.00f,14.00f};
    SDL_RenderTexture(renderer,m_leftChevronTexture,&leftChevronSrc,&leftChevronDst);

    SDL_FRect yellowSkinTileDst{m_yellowSkinButton.x,m_yellowSkinButton.y,m_yellowSkinButton.w,m_yellowSkinButton.h};
    SDL_FRect yellowSkinTileSrc{0.00f,0.00f,65.00f,88.00f};
    if(m_playerSkin == PlayerSkin::YELLOW)
        yellowSkinTileSrc={66.00f,0.00f,65.00f,88.00f};
    SDL_RenderTexture(renderer,m_skinTileTexture,&yellowSkinTileSrc,&yellowSkinTileDst);

    SDL_FRect blueSkinTileDst{m_blueSkinButton.x,m_blueSkinButton.y,m_blueSkinButton.w,m_blueSkinButton.h};
    SDL_FRect blueSkinTileSrc{0.00f,0.00f,65.00f,88.00f};
    if(m_playerSkin == PlayerSkin::BLUE)
        blueSkinTileSrc={66.00f,0.00f,65.00f,88.00f};
    SDL_RenderTexture(renderer,m_skinTileTexture,&blueSkinTileSrc,&blueSkinTileDst);

    SDL_FRect redSkinTileDst{m_redSkinButton.x,m_redSkinButton.y,m_redSkinButton.w,m_redSkinButton.h};
    SDL_FRect redSkinTileSrc{0.00f,0.00f,65.00f,88.00f};
    if(m_playerSkin == PlayerSkin::RED)
        redSkinTileSrc={66.00f,0.00f,65.00f,88.00f};
    SDL_RenderTexture(renderer,m_skinTileTexture,&redSkinTileSrc,&redSkinTileDst);

    SDL_FRect greenSkinTileDst{m_greenSkinButton.x,m_greenSkinButton.y,m_greenSkinButton.w,m_greenSkinButton.h};
    SDL_FRect greenSkinTileSrc{0.00f,0.00f,65.00f,88.00f};
    if(m_playerSkin == PlayerSkin::GREEN)
        greenSkinTileSrc={66.00f,0.00f,65.00f,88.00f};
    SDL_RenderTexture(renderer,m_skinTileTexture,&greenSkinTileSrc,&greenSkinTileDst);

    SDL_FRect yellowSkinDst{m_yellowSkinButton.x , m_yellowSkinButton.y , m_yellowSkinButton.w, m_yellowSkinButton.h-100.00f};
    SDL_FRect yellowSkinSrc{0.00f+m_currentFrame*24.00f,0.00f,24.00f,24.00f};
    SDL_RenderTexture(renderer,m_yellowSkinTexture,&yellowSkinSrc,&yellowSkinDst);

    SDL_FRect blueSkinDst{m_blueSkinButton.x , m_blueSkinButton.y , m_blueSkinButton.w, m_blueSkinButton.h-100.00f};
    SDL_FRect blueSkinSrc{0.00f+m_currentFrame*24.00f,0.00f,24.00f,24.00f};
    SDL_RenderTexture(renderer,m_blueSkinTexture,&blueSkinSrc,&blueSkinDst);

    SDL_FRect redSkinDst{m_redSkinButton.x , m_redSkinButton.y , m_redSkinButton.w, m_redSkinButton.h-100.00f};
    SDL_FRect redSkinSrc{0.00f+m_currentFrame*24.00f,0.00f,24.00f,24.00f};
    SDL_RenderTexture(renderer,m_redSkinTexture,&redSkinSrc,&redSkinDst);

    SDL_FRect greenSkinDst{m_greenSkinButton.x , m_greenSkinButton.y , m_greenSkinButton.w, m_greenSkinButton.h-100.00f};
    SDL_FRect greenSkinSrc{0.00f+m_currentFrame*24.00f,0.00f,24.00f,24.00f};
    SDL_RenderTexture(renderer,m_greenSkinTexture,&greenSkinSrc,&greenSkinDst);

    SDL_FRect previewSkinDst{1038,35,500,390};
    SDL_FRect previewSkinSrc{0.00f+m_currentFrame*24,0.00f,24.00f,24.00f};
    SDL_RenderTexture(renderer,m_previewTexture,&previewSkinSrc,&previewSkinDst);

    SDL_SetRenderDrawColor(renderer,255,0,0,255);
    SDL_RenderRect(renderer,&m_textbox);
    SDL_Surface* editNamesSurface = TTF_RenderText_Solid(m_font,m_editName.c_str(),m_editName.length(),{0,0,0,255});
    SDL_Texture* editNameTexture = SDL_CreateTextureFromSurface(renderer,editNamesSurface);
    SDL_SetTextureScaleMode(editNameTexture,SDL_SCALEMODE_NEAREST);
    SDL_FRect nameDst{m_textbox.x,m_textbox.y,static_cast<float>(0+ m_editName.length() *50),m_textbox.h};
    SDL_RenderTexture(renderer,editNameTexture, nullptr,&nameDst);
    SDL_DestroyTexture(editNameTexture);
    SDL_DestroySurface(editNamesSurface);
}

void EditMenuState::update(float dt)
{
    if(m_playerSkin == PlayerSkin::YELLOW)
        m_previewTexture = m_yellowSkinTexture;
    if(m_playerSkin == PlayerSkin::BLUE)
        m_previewTexture = m_blueSkinTexture;
    if(m_playerSkin == PlayerSkin::RED)
        m_previewTexture = m_redSkinTexture;
    if(m_playerSkin == PlayerSkin::GREEN)
        m_previewTexture = m_greenSkinTexture;
    m_nowTime =SDL_GetTicks();
    if(m_nowTime-m_lastTime >m_animationDelay)
    {
        m_currentFrame++;
        m_lastTime=m_nowTime;
    }

    m_currentFrame %= m_endFrame;
    if(m_playerSkin != PlayerDetail::getInstance().getPlayerSkin())
    {

        PlayerDetail::getInstance().setPlayerSkin(m_playerSkin);
    }
}

bool EditMenuState::handleEvents(SDL_Event &event)
{

    if(event.type == SDL_EVENT_FINGER_DOWN)
    {
        float TouchX = event.tfinger.x * (float)GameData::getInstance().getWinWidth();
        float TouchY = event.tfinger.y * (float)GameData::getInstance().getWinHeight();
        if(TouchX >= m_backButton.x && TouchX <= m_backButton.x + m_backButton.w &&
            TouchY >= m_backButton.y && TouchY <= m_backButton.y + m_backButton.h)
        {
            Engine::Get().changeState(std::make_unique<MenuState>(m_renderer));
        }

        if(TouchX >= m_yellowSkinButton.x && TouchX <= m_yellowSkinButton.x + m_yellowSkinButton.w &&
           TouchY >= m_yellowSkinButton.y && TouchY <= m_yellowSkinButton.y + m_yellowSkinButton.h) {

            m_playerSkin =PlayerSkin::YELLOW;
        }

        if(TouchX >= m_blueSkinButton.x && TouchX <= m_blueSkinButton.x + m_blueSkinButton.w &&
           TouchY >= m_blueSkinButton.y && TouchY <= m_blueSkinButton.y + m_blueSkinButton.h)
        {

            m_playerSkin =PlayerSkin::BLUE;
        }
        if(TouchX >= m_redSkinButton.x && TouchX <= m_redSkinButton.x + m_redSkinButton.w &&
           TouchY >= m_redSkinButton.y && TouchY <= m_redSkinButton.y + m_redSkinButton.h)
        {

            m_playerSkin =PlayerSkin::RED;
        }
        if(TouchX >= m_greenSkinButton.x && TouchX <= m_greenSkinButton.x + m_greenSkinButton.w &&
           TouchY >= m_greenSkinButton.y && TouchY <= m_greenSkinButton.y + m_greenSkinButton.h)
        {
            m_playerSkin =PlayerSkin::GREEN;
        }
        if(TouchX >= m_textbox.x && TouchX <= m_textbox.x + m_textbox.w &&
            TouchY >= m_textbox.y && TouchY <= m_textbox.y + m_textbox.h){

            m_textboxActive=true;
        }

        return true;
    }

    if(m_textboxActive){
        LOGI("active");
        SDL_StartTextInput(Engine::Get().getWindow());


        switch (event.type) {
            case SDL_EVENT_TEXT_INPUT:
                m_editName += event.text.text;
                break;
            case SDL_EVENT_KEY_DOWN:
                if(event.key.key == SDLK_BACKSPACE)
                {
                    m_editName.pop_back();
                }
                if(event.key.key ==SDLK_AC_BACK) {
                    m_textboxActive = false;
                    SDL_StopTextInput(Engine::Get().getWindow());
                }
                if(event.key.key == SDLK_RETURN)
                {
                    m_textboxActive = false;
                    SDL_StopTextInput(Engine::Get().getWindow());
                }
                if(event.key.key == SDLK_RETURN2) {
                    m_textboxActive = false;
                    SDL_StopTextInput(Engine::Get().getWindow());
                }
                if(event.key.key == SDLK_KP_ENTER) {
                    m_textboxActive = false;
                    SDL_StopTextInput(Engine::Get().getWindow());
                }
                break;
        }
        LOGI("textbox active :%d",m_textboxActive);

        LOGI("name:%s",m_editName.c_str());
    }
    if(event.type == SDL_EVENT_KEY_DOWN)
    {
        if(event.key.key == SDLK_AC_BACK)
        {
            Engine::Get().changeState(std::make_unique<MenuState>(m_renderer));
            return true;
        }
    }
    return false;
}

EditMenuState::EditMenuState(SDL_Renderer *renderer)
{
    bool success =SDL_SetTextInputArea(Engine::Get().getWindow(),
                                       reinterpret_cast<const SDL_Rect *>(&m_textbox), 0);
    if(success)LOGI("created text input area");
    else LOGI("failed to create input area");
    m_renderer =renderer;
    if(!m_backgroundFile)
        LOGI("failed to load background file");
    SDL_Surface* backgroundSurface = IMG_Load_IO(m_backgroundFile, false);
    m_backgroundTexture = SDL_CreateTextureFromSurface(renderer,backgroundSurface);
    SDL_DestroySurface(backgroundSurface);
    SDL_CloseIO(m_backgroundFile);

    if(!m_fontfile)
        LOGI("failed to load font file");
    m_font = TTF_OpenFontIO(m_fontfile,false,18);
    SDL_Surface* backButtonSurface = TTF_RenderText_Solid(m_font,"  back",6,{255,255,255,255});
    m_backButtonTexture = SDL_CreateTextureFromSurface(renderer,backButtonSurface);
    SDL_DestroySurface(backButtonSurface);
    if(!m_editMenuFile)
        LOGI("failed to load editMenu file");
    SDL_Surface* editMenuSurface = IMG_Load_IO(m_editMenuFile,false);
    m_editMenuTexture = SDL_CreateTextureFromSurface(renderer,editMenuSurface);
    SDL_DestroySurface(editMenuSurface);
    SDL_CloseIO(m_editMenuFile);
    if(!m_nameTileFile)
        LOGI("failed to load nameTile file");
    SDL_Surface* nameTileSurface = IMG_Load_IO(m_nameTileFile,false);
    m_nameTileTexture = SDL_CreateTextureFromSurface(renderer,nameTileSurface);
    SDL_DestroySurface(nameTileSurface);
    SDL_CloseIO(m_nameTileFile);
    if(!m_skinTileFile)
        LOGI("failed to load skinTile file");
    SDL_Surface* skinTileSurface = IMG_Load_IO(m_skinTileFile,false);
    m_skinTileTexture = SDL_CreateTextureFromSurface(renderer,skinTileSurface);
    SDL_DestroySurface(skinTileSurface);
    SDL_CloseIO(m_skinTileFile);
    if(!m_previewTileFile)
        LOGI("failed to load previewTile file");
    SDL_Surface* previewTileSurface = IMG_Load_IO(m_previewTileFile,false);
    m_previewTileTexture = SDL_CreateTextureFromSurface(renderer,previewTileSurface);
    SDL_DestroySurface(previewTileSurface);
    SDL_CloseIO(m_previewTileFile);
    if(!m_leftChevronFile)
        LOGI("failed to load leftChevron file");
    SDL_Surface* leftChevronSurface = IMG_Load_IO(m_leftChevronFile,false);
    m_leftChevronTexture = SDL_CreateTextureFromSurface(renderer,leftChevronSurface);
    SDL_DestroySurface(leftChevronSurface);
    SDL_CloseIO(m_leftChevronFile);

    if(!m_yellowSkinFile)
        LOGI("couldnt load yellowskin file");
    SDL_Surface* yellowSkinSurface = IMG_Load_IO(m_yellowSkinFile,false);
    m_yellowSkinTexture = SDL_CreateTextureFromSurface(renderer,yellowSkinSurface);
    SDL_DestroySurface(yellowSkinSurface);
    SDL_CloseIO(m_yellowSkinFile);

    if(!m_blueSkinFile)
        LOGI("couldnt load blueskin file");
    SDL_Surface* blueSkinSurface = IMG_Load_IO(m_blueSkinFile,false);
    m_blueSkinTexture = SDL_CreateTextureFromSurface(renderer,blueSkinSurface);
    SDL_DestroySurface(blueSkinSurface);
    SDL_CloseIO(m_blueSkinFile);

    if(!m_redSkinFile)
        LOGI("couldnt load redskin file");
    SDL_Surface* redSkinSurface = IMG_Load_IO(m_redSkinFile,false);
    m_redSkinTexture = SDL_CreateTextureFromSurface(renderer,redSkinSurface);
    SDL_DestroySurface(redSkinSurface);
    SDL_CloseIO(m_redSkinFile);

    if(!m_greenSkinFile)
        LOGI("couldnt load greenskin file");
    SDL_Surface* greenSkinSurface = IMG_Load_IO(m_greenSkinFile,false);
    m_greenSkinTexture = SDL_CreateTextureFromSurface(renderer,greenSkinSurface);
    SDL_DestroySurface(greenSkinSurface);
    SDL_CloseIO(m_greenSkinFile);



    SDL_SetTextureScaleMode(m_backgroundTexture,SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_editMenuTexture,SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_nameTileTexture,SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_skinTileTexture,SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_previewTileTexture,SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_backButtonTexture,SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_leftChevronTexture,SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_yellowSkinTexture,SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_blueSkinTexture,SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_redSkinTexture,SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(m_greenSkinTexture,SDL_SCALEMODE_NEAREST);
}

EditMenuState::~EditMenuState()
{
    SDL_CloseIO(m_fontfile);
    TTF_CloseFont(m_font);
    SDL_DestroyTexture(m_backgroundTexture);
    SDL_DestroyTexture(m_editMenuTexture);
    SDL_DestroyTexture(m_nameTileTexture);
    SDL_DestroyTexture(m_skinTileTexture);
    SDL_DestroyTexture(m_previewTileTexture);
    SDL_DestroyTexture(m_backButtonTexture);
    SDL_DestroyTexture(m_leftChevronTexture);
    SDL_DestroyTexture(m_yellowSkinTexture);
    SDL_DestroyTexture(m_blueSkinTexture);
    SDL_DestroyTexture(m_redSkinTexture);
    SDL_DestroyTexture(m_greenSkinTexture);
}
