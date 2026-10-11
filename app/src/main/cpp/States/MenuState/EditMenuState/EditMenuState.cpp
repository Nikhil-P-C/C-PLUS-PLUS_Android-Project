//
// Created by LENOVO on 21-07-2026.
//
#include "EditMenuState.h"
#include <algorithm>
#include <cmath>
#include "engine/Engine.h"
#include "States/MenuState/MenuState.h"

using namespace MenuKit;

namespace {
    constexpr float CORE_X = 800.0f, CORE_Y = 300.0f;
    constexpr int   FONT_SIZE = 36, NAME_FONT = 40;

    // skins (left)
    constexpr float SKIN_Y = 320.0f, SKIN_X0 = 230.0f, SKIN_DX = 220.0f, SKIN_SIZE = 150.0f;
    constexpr float SKIN_HIT_W = 190.0f, SKIN_HIT_H = 210.0f;
    // preview (right)
    constexpr float PREVIEW_X = 1250.0f, PREVIEW_Y = 330.0f, PREVIEW_SIZE = 380.0f;
    // name field
    constexpr float NAME_Y = 520.0f, NAME_TAG_X = 330.0f, NAME_FIELD_X = 520.0f, NAME_FIELD_W = 400.0f;
    constexpr size_t NAME__MAX = 9;
    // buttons
    constexpr float SAVE_X = 560.0f, SAVE_Y = 600.0f, SAVE_HIT_W = 240.0f, SAVE_HIT_H = 70.0f;
    constexpr float BACK_X = 800.0f, BACK_Y = 665.0f, BACK_HIT_W = 240.0f, BACK_HIT_H = 70.0f;

    inline float skinX(int i) { return SKIN_X0 + SKIN_DX * (float)i; }

    // The skin textures are shared with the game: whatever colour / alpha mod we set is undone on scope exit.
    struct TextureModGuard {
        explicit TextureModGuard(SDL_Texture* t) : tex(t) {
            if(tex){ SDL_GetTextureColorMod(tex,&r,&g,&b); SDL_GetTextureAlphaMod(tex,&a); }
        }
        ~TextureModGuard() {
            if(tex){ SDL_SetTextureColorMod(tex,r,g,b); SDL_SetTextureAlphaMod(tex,a); }
        }
        SDL_Texture* tex;
        Uint8 r = 255, g = 255, b = 255, a = 255;
    };
}

EditMenuState::EditMenuState(SDL_Renderer *renderer)
        : m_renderer(renderer),
          m_backdrop(renderer,CORE_X,CORE_Y,0.4f),
          m_font(FONT_SIZE),
          m_editName(PlayerDetail::getInstance().getPlayerName()) {
    this->Name = "EditMenuState";

    m_nameTag.set(renderer,m_font,"NAME",FONT_SIZE);
    m_saveLabel.set(renderer,m_font,"SAVE",FONT_SIZE);
    m_savedLabel.set(renderer,m_font,"SAVED",FONT_SIZE);
    m_backLabel.set(renderer,m_font,"BACK",FONT_SIZE);
    if(!m_editName.empty()) m_nameValue.set(renderer,m_font,m_editName.c_str(),NAME_FONT);

    AssetManager& assets = Engine::Get().getAssetManager();
    m_skinTex[PlayerSkin::YELLOW] = assets.getTexture(TextureType::YELLOW_SKIN);
    m_skinTex[PlayerSkin::BLUE]   = assets.getTexture(TextureType::BLUE_SKIN);
    m_skinTex[PlayerSkin::RED]    = assets.getTexture(TextureType::RED_SKIN);
    m_skinTex[PlayerSkin::GREEN]  = assets.getTexture(TextureType::GREEN_SKIN);

    m_skin = (int)PlayerDetail::getInstance().getPlayerSkin();
    for(int i=0;i<SKIN_COUNT;i++) m_skinAnim[i] = (i == m_skin) ? 1.0f : 0.0f;

    m_rows.setup(ROW_COUNT,0.0f,0.0f);     // only used for focus + reveal, rows are placed by hand
}

EditMenuState::~EditMenuState() {
    if(m_textActive) SDL_StopTextInput(Engine::Get().getWindow());
}

SDL_Texture* EditMenuState::skinTexture(int skin) const {
    return (skin >= 0 && skin < SKIN_COUNT) ? m_skinTex[skin] : nullptr;
}

// ---------------------------------------------------------------------------------------------
// Actions
// ---------------------------------------------------------------------------------------------
void EditMenuState::selectSkin(int skin,bool sound) {
    skin = (skin + SKIN_COUNT) % SKIN_COUNT;
    if(skin == m_skin) return;
    m_skin = skin;
    PlayerDetail::getInstance().setPlayerSkin(static_cast<PlayerSkin>(skin));     // applies immediately, like before
    m_backdrop.spark(skinX(skin),SKIN_Y,10);
    if(sound) Engine::Get().getAssetManager().playSound(SoundType::SFX_UI_CONFIRM);
}

void EditMenuState::beginTextInput() {
    if(m_textActive) return;
    m_textActive = true;
    SDL_Window* window = Engine::Get().getWindow();
    SDL_Rect area{(int)NAME_FIELD_X,(int)(NAME_Y - 30.0f),(int)NAME_FIELD_W,60};
    SDL_SetTextInputArea(window,&area,(int)m_editName.length());
    SDL_StartTextInput(window);
}

void EditMenuState::endTextInput() {
    if(!m_textActive) return;
    m_textActive = false;
    SDL_StopTextInput(Engine::Get().getWindow());
}

void EditMenuState::saveName() {
    PlayerDetail::getInstance().setPlayerName(m_editName);
    m_savedTimer = 1.2f;
    m_backdrop.burst(SAVE_X,SAVE_Y);
    Engine::Get().getAssetManager().playSound(SoundType::SFX_UI_CONFIRM);
}

void EditMenuState::leave() {
    if(m_leaving) return;
    m_leaving = true;
    endTextInput();
    Engine::Get().getAssetManager().playSound(SoundType::SFX_UI_ENTER);
    Engine::Get().changeState(std::make_unique<MenuState>(m_renderer, false));
}

// ---------------------------------------------------------------------------------------------
// Hit testing
// ---------------------------------------------------------------------------------------------
int EditMenuState::skinAt(float x,float y) const {
    for(int i=0;i<SKIN_COUNT;i++)
        if(InputUtils::PointInRect(x,y,skinX(i) - SKIN_HIT_W*0.5f,SKIN_Y - SKIN_HIT_H*0.5f,SKIN_HIT_W,SKIN_HIT_H)) return i;
    return -1;
}

int EditMenuState::rowAt(float x,float y) const {
    if(skinAt(x,y) >= 0) return ROW_SKIN;
    if(InputUtils::PointInRect(x,y,NAME_TAG_X - 20.0f,NAME_Y - 40.0f,NAME_FIELD_X + NAME_FIELD_W - NAME_TAG_X + 40.0f,80.0f)) return ROW_NAME;
    if(InputUtils::PointInRect(x,y,SAVE_X - SAVE_HIT_W*0.5f,SAVE_Y - SAVE_HIT_H*0.5f,SAVE_HIT_W,SAVE_HIT_H)) return ROW_SAVE;
    if(InputUtils::PointInRect(x,y,BACK_X - BACK_HIT_W*0.5f,BACK_Y - BACK_HIT_H*0.5f,BACK_HIT_W,BACK_HIT_H)) return ROW_BACK;
    return -1;
}

// ---------------------------------------------------------------------------------------------
// Update / render
// ---------------------------------------------------------------------------------------------
void EditMenuState::update(float dt) {
    m_backdrop.update(dt);
    m_rows.update(dt);
    m_blink += dt;

    m_animTimer += dt;
    while(m_animTimer >= 0.1f){ m_animTimer -= 0.1f; m_frame = (m_frame + 1) % 4; }

    for(int i=0;i<SKIN_COUNT;i++)
        m_skinAnim[i] = approach(m_skinAnim[i],(i == m_skin) ? 1.0f : 0.0f,12.0f,dt);
    if(m_savedTimer > 0.0f) m_savedTimer -= dt;
    m_saveAnim = approach(m_saveAnim,(m_rows.focus() == ROW_SAVE) ? 1.0f : 0.0f,12.0f,dt);
    m_backAnim = approach(m_backAnim,(m_rows.focus() == ROW_BACK) ? 1.0f : 0.0f,12.0f,dt);

    if(!m_textActive) perform(m_nav.update(dt));            // held dpad / stick
}

void EditMenuState::render(SDL_Renderer *renderer) {
    const MenuTheme& theme = MenuTheme::get();
    const MenuPainter& p = m_backdrop.painter();
    const float intro = smooth(m_backdrop.time() / 0.4f);

    m_backdrop.render(renderer);

    // calm band behind everything
    p.shade(CORE_X,370.0f,1380.0f,640.0f,150.0f / 255.0f);

    // ---- skin row ----
    {
        const float f  = m_rows.focusAnim(ROW_SKIN);
        const float rv = m_rows.reveal(ROW_SKIN) * intro;
        p.rowFrame(SKIN_X0 - 150.0f,skinX(SKIN_COUNT - 1) + 150.0f,SKIN_Y,f,rv);

        for(int i=0;i<SKIN_COUNT;i++){
            const float s = m_skinAnim[i];
            if(s > 0.02f) p.glow(skinX(i),SKIN_Y,280.0f,280.0f,theme.core(),0.35f * s * rv);
            SDL_Texture* tex = skinTexture(i);
            if(tex){
                TextureModGuard guard(tex);
                const Uint8 shade = to8(lerp(0.45f,1.0f,s));
                SDL_SetTextureColorMod(tex,shade,shade,shade);
                SDL_SetTextureAlphaMod(tex,to8(lerp(0.65f,1.0f,s) * rv));
                const SDL_FRect src{(float)m_frame * 24.0f,0.0f,24.0f,24.0f};
                const float size = SKIN_SIZE * (1.0f + 0.08f * s);
                const SDL_FRect dst{skinX(i) - size*0.5f,SKIN_Y - size*0.5f,size,size};
                SDL_RenderTexture(renderer,tex,&src,&dst);
            }
            if(s > 0.02f) p.diamond(skinX(i),SKIN_Y + SKIN_SIZE*0.5f + 38.0f,5.0f,theme.accent(),s * rv);
        }
    }

    // ---- preview ----
    {
        const float rv = m_rows.reveal(ROW_SKIN) * intro;
        p.glow(PREVIEW_X,PREVIEW_Y,PREVIEW_SIZE * 1.5f,PREVIEW_SIZE * 1.5f,theme.core(),0.30f * rv);
        SDL_Texture* tex = skinTexture(m_skin);
        if(tex){
            TextureModGuard guard(tex);
            SDL_SetTextureColorMod(tex,255,255,255);
            SDL_SetTextureAlphaMod(tex,to8(rv));
            const SDL_FRect src{(float)m_frame * 24.0f,0.0f,24.0f,24.0f};
            const SDL_FRect dst{PREVIEW_X - PREVIEW_SIZE*0.5f,PREVIEW_Y - PREVIEW_SIZE*0.5f,PREVIEW_SIZE,PREVIEW_SIZE};
            SDL_RenderTextureRotated(renderer,tex,&src,&dst,0.0,nullptr,SDL_FLIP_HORIZONTAL);
        }
    }

    // ---- name field ----
    {
        const float f  = m_rows.focusAnim(ROW_NAME);
        const float rv = m_rows.reveal(ROW_NAME) * intro;
        const float a  = lerp(0.70f,1.0f,f) * rv;
        const Rgb col  = mix(theme.dim(),theme.accent(),f);
        const float editing = m_textActive ? 1.0f : 0.0f;

        p.rowFrame(NAME_TAG_X - 40.0f,NAME_FIELD_X + NAME_FIELD_W + 40.0f,NAME_Y,f,rv);
        p.label(m_nameTag,NAME_TAG_X,NAME_Y,1.0f,theme.dim(),0.85f * rv,Align::LEFT);
        p.fadeLine(NAME_FIELD_X - 10.0f,NAME_FIELD_X + NAME_FIELD_W,NAME_Y + 30.0f,
                   theme.accent(),(0.35f + 0.5f * editing) * rv,0.05f * rv);
        float caretX = NAME_FIELD_X;
        if(m_nameValue.tex){
            p.label(m_nameValue,NAME_FIELD_X,NAME_Y,1.0f,col,a,Align::LEFT);
            caretX += m_nameValue.w + 6.0f;
        }
        if(m_textActive && std::fmod(m_blink,1.0f) < 0.6f){
            SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer,to8(theme.accent().r),to8(theme.accent().g),to8(theme.accent().b),to8(rv));
            const SDL_FRect caret{caretX,NAME_Y - 18.0f,3.0f,36.0f};
            SDL_RenderFillRect(renderer,&caret);
        }
    }

    // ---- SAVE / BACK ----
    {
        const float rv = m_rows.reveal(ROW_SAVE) * intro;
        const float s = m_saveAnim;
        const float scale = 1.0f + 0.08f * s;
        const MenuLabel& lab = m_savedTimer > 0.0f ? m_savedLabel : m_saveLabel;
        p.flank(lab,SAVE_X,SAVE_Y,scale,s,rv);
        p.label(lab,SAVE_X,SAVE_Y,scale,mix(theme.dim(),theme.accent(),std::max(s,m_savedTimer > 0.0f ? 1.0f : 0.0f)),
                lerp(0.70f,1.0f,s) * rv);
    }
    {
        const float rv = m_rows.reveal(ROW_BACK) * intro;
        const float s = m_backAnim;
        const float scale = 1.0f + 0.08f * s;
        p.flank(m_backLabel,BACK_X,BACK_Y,scale,s,rv);
        p.label(m_backLabel,BACK_X,BACK_Y,scale,mix(theme.dim(),theme.accent(),s),lerp(0.70f,1.0f,s) * rv);
    }

    m_backdrop.renderFront(renderer);
    m_backdrop.renderFade(renderer);
}

// ---------------------------------------------------------------------------------------------
// Input
// ---------------------------------------------------------------------------------------------
void EditMenuState::perform(InputUtils::MenuAction action) {
    AssetManager& audio = Engine::Get().getAssetManager();
    switch(action){
        case InputUtils::MenuAction::UP:
            if(m_rows.move(-1)) audio.playSound(SoundType::SFX_UI_MOVE);
            break;
        case InputUtils::MenuAction::DOWN:
            if(m_rows.move(1))  audio.playSound(SoundType::SFX_UI_MOVE);
            break;
        case InputUtils::MenuAction::LEFT:
            if(m_rows.focus() == ROW_SKIN) selectSkin(m_skin - 1,true);
            break;
        case InputUtils::MenuAction::RIGHT:
            if(m_rows.focus() == ROW_SKIN) selectSkin(m_skin + 1,true);
            break;
        case InputUtils::MenuAction::CONFIRM:
            switch(m_rows.focus()){
                case ROW_NAME: beginTextInput(); break;
                case ROW_SAVE: saveName();       break;
                case ROW_BACK: leave();          break;
                default: break;
            }
            break;
        case InputUtils::MenuAction::BACK:
            leave();
            break;
        default: break;
    }
}

bool EditMenuState::handleEvents(SDL_Event &event)
{
    if(m_leaving) return true;

    // ---- typing the name: letters are text, only arrows / Enter / Esc keep their menu meaning ----
    if(m_textActive){
        if(event.type == SDL_EVENT_TEXT_INPUT){
            bool changed = false;
            for(const char* c = event.text.text; c && *c && m_editName.length() < NAME__MAX; ++c){
                if(*c >= 0x20 && *c <= 0x7E){ m_editName += *c; changed = true; }   // the pixel font is ASCII only
            }
            if(changed) m_nameValue.set(m_renderer,m_font,m_editName.c_str(),NAME_FONT);
            return true;
        }
        if(event.type == SDL_EVENT_KEY_DOWN){
            if(event.key.key == SDLK_BACKSPACE){
                if(!m_editName.empty()){
                    m_editName.pop_back();
                    if(m_editName.empty()) m_nameValue.reset();
                    else m_nameValue.set(m_renderer,m_font,m_editName.c_str(),NAME_FONT);
                }
                return true;
            }
            const InputUtils::MenuAction a = InputUtils::GetMenuAction(event,false);
            if(a == InputUtils::MenuAction::CONFIRM || a == InputUtils::MenuAction::BACK) endTextInput();
            return true;                                   // swallow every other key while typing
        }
        // gamepad: A / B close the text field (a pad has no Enter key)
        if(event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN){
            const InputUtils::MenuAction a = InputUtils::GetMenuAction(event);
            if(a == InputUtils::MenuAction::CONFIRM || a == InputUtils::MenuAction::BACK){ endTextInput(); return true; }
        }
        // pointer events fall through below
    }

    if(m_nav.handleEvent(event)) return true;

    // ---- pointer ----
    if(InputUtils::IsPointerMotion(event)){
        float x,y;
        InputUtils::GetPointerPosition(event,m_renderer,x,y);
        const int row = rowAt(x,y);
        if(row >= 0 && m_rows.setFocus(row)) Engine::Get().getAssetManager().playSound(SoundType::SFX_UI_MOVE);
        return true;
    }
    if(InputUtils::IsPointerDown(event)){
        float x,y;
        InputUtils::GetPointerPosition(event,m_renderer,x,y);
        const int row = rowAt(x,y);
        if(m_textActive && row != ROW_NAME) endTextInput();   // tapping elsewhere closes the keyboard
        if(row < 0) return true;
        m_rows.setFocus(row);
        switch(row){
            case ROW_SKIN: selectSkin(skinAt(x,y),true); break;
            case ROW_NAME: beginTextInput();             break;
            case ROW_SAVE: saveName();                   break;
            case ROW_BACK: leave();                      break;
            default: break;
        }
        return true;
    }

    // ---- keyboard / gamepad buttons ----
    const InputUtils::MenuAction action = InputUtils::GetMenuAction(event);
    if(action == InputUtils::MenuAction::NONE) return false;
    perform(action);
    return true;
}
