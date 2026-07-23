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

    SDL_FRect previewSkinDst{1058,35.00f,450.00f,390.00f};
    SDL_FRect previewSkinSrc{0.00f+m_currentFrame*24,0.00f,24.00f,24.00f};
    SDL_RenderTextureRotated(renderer,m_previewTexture,&previewSkinSrc,&previewSkinDst,0.00f,
                             nullptr,SDL_FLIP_HORIZONTAL);

    SDL_Surface* editNamesSurface = TTF_RenderText_Solid(m_font,m_editName.c_str(),m_editName.length(),{84, 83, 83,255});
    SDL_Texture* editNameTexture = SDL_CreateTextureFromSurface(renderer,editNamesSurface);
    SDL_SetTextureScaleMode(editNameTexture,SDL_SCALEMODE_NEAREST);
    SDL_FRect nameDst{m_textbox.x,m_textbox.y+10.00f,static_cast<float>(0+ m_editName.length() *50),m_textbox.h-10.00f};
    SDL_RenderTexture(renderer,editNameTexture, nullptr,&nameDst);
    SDL_DestroyTexture(editNameTexture);
    SDL_DestroySurface(editNamesSurface);


    SDL_FRect saveNameDst{m_saveNameButton.x+15,m_saveNameButton.y,m_saveNameButton.w-30.00f,m_saveNameButton.h};
    SDL_SetTextureScaleMode(m_saveTextTexture,SDL_SCALEMODE_NEAREST);
    SDL_RenderTexture(renderer,m_saveTextTexture, nullptr,&saveNameDst);
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
    if(m_saveButtonActive){
        PlayerDetail::getInstance().setPlayerName(m_editName);
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
            SDL_StopTextInput(Engine::Get().getWindow());
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
            LOGI("active");
            SDL_StartTextInput(Engine::Get().getWindow());
            m_textboxActive=true;
        }

        if(TouchX >= m_saveNameButton.x && TouchX <= m_saveNameButton.x + m_saveNameButton.w &&
           TouchY >= m_saveNameButton.y && TouchY <= m_saveNameButton.y + m_saveNameButton.h)
        {
            m_saveButtonActive =true;
        }

        return true;
    }
    if(event.type == SDL_EVENT_FINGER_UP)
    {
        m_saveButtonActive =false;
    }

    if(m_textboxActive){

        switch (event.type) {
            case SDL_EVENT_KEY_DOWN:
                if(event.key.key == SDLK_BACKSPACE)
                {
                    if(!m_editName.empty())
                    {
                        m_editName.pop_back();
                        bool success =SDL_SetTextInputArea(Engine::Get().getWindow(),
                                                           reinterpret_cast<const SDL_Rect *>(&m_textbox),  m_editName.length());
                        if(success)LOGI("created text input area");
                        else LOGI("failed to create input area");
                    }
                    return true;
                }
                if(event.key.key ==SDLK_AC_BACK) {
                    m_textboxActive = false;
                    SDL_StopTextInput(Engine::Get().getWindow());
                    return true;
                }
                if(event.key.key == SDLK_RETURN)
                {
                    m_textboxActive = false;
                    SDL_StopTextInput(Engine::Get().getWindow());
                    return true;
                }
                if(event.key.key == SDLK_RETURN2) {
                    m_textboxActive = false;
                    SDL_StopTextInput(Engine::Get().getWindow());
                    return true;
                }
                if(event.key.key == SDLK_KP_ENTER) {
                    m_textboxActive = false;
                    SDL_StopTextInput(Engine::Get().getWindow());
                    return true;
                }
                break;
            case SDL_EVENT_TEXT_INPUT:
                LOGI("name:%s",m_editName.c_str());
                LOGI("text text:%s",event.text.text);
                LOGI("name:%s",m_editName.c_str());
                if(m_editName.length() <9)
                {
                    LOGI("str length:%d",m_editName.length());
                    m_editName += event.text.text[0];
                    bool success =SDL_SetTextInputArea(Engine::Get().getWindow(),
                                                       reinterpret_cast<const SDL_Rect *>(&m_textbox),  m_editName.length());
                    if(success)LOGI("created text input area");
                    else LOGI("failed to create input area");
                }
                return true;
                break;
            case SDL_EVENT_TEXT_EDITING:
                LOGI("text edit:%s",event.edit.text);
        }


    }
    if(event.type == SDL_EVENT_KEY_DOWN)
    {
        if(event.key.key == SDLK_AC_BACK)
        {
            Engine::Get().changeState(std::make_unique<MenuState>(m_renderer));
            return true;
        }
        return true;
    }
    return false;
}

EditMenuState::EditMenuState(SDL_Renderer *renderer)
{
    m_renderer =renderer;

    if(!m_fontfile)
        LOGI("failed to load font file");
    m_font = TTF_OpenFontIO(m_fontfile,false,18);
    SDL_Surface* saveNameSurface = TTF_RenderText_Solid(m_font,"save",4,{255,255,255,255});
    m_saveTextTexture = SDL_CreateTextureFromSurface(renderer,saveNameSurface);
    SDL_DestroySurface(saveNameSurface);
    SDL_Surface* backButtonSurface = TTF_RenderText_Solid(m_font,"  back",6,{255,255,255,255});
    m_backButtonTexture = SDL_CreateTextureFromSurface(renderer,backButtonSurface);
    SDL_DestroySurface(backButtonSurface);

    m_backgroundTexture =Engine::Get().getAssetManager().getTexture(TextureType::BG_CONFETTI);
    m_editMenuTexture = Engine::Get().getAssetManager().getTexture(TextureType::EDIT_MENU_TILE);
    m_nameTileTexture = Engine::Get().getAssetManager().getTexture(TextureType::EDIT_NAME_TILE);
    m_skinTileTexture = Engine::Get().getAssetManager().getTexture(TextureType::EDIT_SKIN_TILE);
    m_previewTileTexture = Engine::Get().getAssetManager().getTexture(TextureType::EDIT_PREVIEW_TILE);
    m_leftChevronTexture = Engine::Get().getAssetManager().getTexture(TextureType::EDIT_LEFT_CHEVRON);

    m_yellowSkinTexture=Engine::Get().getAssetManager().getTexture(TextureType::YELLOW_SKIN);
    m_blueSkinTexture = Engine::Get().getAssetManager().getTexture(TextureType::BLUE_SKIN);
    m_redSkinTexture = Engine::Get().getAssetManager().getTexture(TextureType::RED_SKIN);
    m_greenSkinTexture = Engine::Get().getAssetManager().getTexture(TextureType::GREEN_SKIN);



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
    TTF_CloseFont(m_font);
    SDL_CloseIO(m_fontfile);

    SDL_DestroyTexture(m_saveTextTexture);
    SDL_DestroyTexture(m_backButtonTexture);

}
