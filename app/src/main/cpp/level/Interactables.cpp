#include "Interactables.h"
#include <algorithm>
#include <cmath>
#include "utils/utils.h"
#include "States/GameState/GameState.h"   // SCALE, Camera

namespace {
    // dialog box layout (logical px, the game runs in a fixed 1600x720 space)
    constexpr float BOX_MARGIN_X = 60.0f;
    constexpr float BOX_MARGIN_BOTTOM = 30.0f;
    constexpr float BOX_HEIGHT = 200.0f;
    constexpr float BOX_PAD = 28.0f;
    constexpr float NAME_TAB_H = 44.0f;
    constexpr float CHARS_PER_SEC = 45.0f;        // typewriter speed (bytes/sec)

    bool overlaps(const SDL_FRect& a, const SDL_FRect& b){
        return a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y;
    }
    // is this byte the middle of a UTF-8 character?
    bool isContinuationByte(unsigned char c){ return (c & 0xC0) == 0x80; }

    SDL_FRect dialogBox(){
        const float w = (float)GameData::getInstance().getWinWidth();
        const float h = (float)GameData::getInstance().getWinHeight();
        return SDL_FRect{BOX_MARGIN_X, h - BOX_HEIGHT - BOX_MARGIN_BOTTOM, w - 2.0f * BOX_MARGIN_X, BOX_HEIGHT};
    }
}

InteractableBuilder::~InteractableBuilder(){
    destroyTextures();
    if(m_bangTex) SDL_DestroyTexture(m_bangTex);
}

void InteractableBuilder::destroyTextures(){
    if(m_textTex){ SDL_DestroyTexture(m_textTex); m_textTex = nullptr; }
    if(m_nameTex){ SDL_DestroyTexture(m_nameTex); m_nameTex = nullptr; }
    m_textShownBytes = -1; m_textKey = -1; m_nameKey = -1;
}

void InteractableBuilder::init(const std::vector<Interactable>& interactables){
    m_interactables = interactables;
    closeDialog();
}

void InteractableBuilder::closeDialog(){
    m_active = -1;
    m_page = 0;
    m_shown = 0.0f;
    destroyTextures();
}

int InteractableBuilder::fullLength() const{
    if(m_active < 0) return 0;
    const auto& lines = m_interactables[m_active].lines;
    if(m_page < 0 || m_page >= (int)lines.size()) return 0;
    return (int)lines[m_page].size();
}

void InteractableBuilder::update(float dt, const SDL_FRect& player){
    m_time += dt;
    for(auto& it : m_interactables)
        it.inRange = overlaps(player, SDL_FRect{it.x, it.y, it.w, it.h});

    if(m_active >= 0){
        const int len = fullLength();
        if(m_shown < (float)len) m_shown = std::min((float)len, m_shown + CHARS_PER_SEC * dt);
    }
}

bool InteractableBuilder::interact(){
    if(m_active < 0){
        for(int i = 0; i < (int)m_interactables.size(); ++i){
            if(!m_interactables[i].inRange || m_interactables[i].lines.empty()) continue;
            m_active = i;
            m_page = 0;
            m_shown = 0.0f;
            return true;
        }
        return false;
    }
    if(m_shown < (float)fullLength()){          // still typing -> finish the page instantly
        m_shown = (float)fullLength();
        return true;
    }
    ++m_page;
    if(m_page >= (int)m_interactables[m_active].lines.size()){
        closeDialog();
    } else {
        m_shown = 0.0f;
    }
    return true;
}

SDL_FRect InteractableBuilder::bubbleRect(const Interactable& it, int camX, int camY) const{
    const float bob = std::sin(m_time * 4.0f) * 5.0f;
    const float bw = it.inRange ? 36.0f : 30.0f;
    const float bh = it.inRange ? 44.0f : 38.0f;
    return SDL_FRect{it.x + it.w * 0.5f - bw * 0.5f - camX, it.y - bh - 10.0f + bob - camY, bw, bh};
}

bool InteractableBuilder::onPointerDown(float px, float py, int camX, int camY){
    if(m_active >= 0) return interact();         // any tap while a dialog is open advances it
    for(const auto& it : m_interactables){
        if(!it.inRange) continue;
        SDL_FRect b = bubbleRect(it, camX, camY);
        b.x -= 24; b.y -= 24; b.w += 48; b.h += 48;   // generous touch target
        if(px >= b.x && px <= b.x + b.w && py >= b.y && py <= b.y + b.h) return interact();
    }
    return false;
}

void InteractableBuilder::renderWorld(SDL_Renderer* renderer){
    if(m_interactables.empty()) return;
    const int camX = (int)std::round(Camera::getInstance().getCamera().x);
    const int camY = (int)std::round(Camera::getInstance().getCamera().y);

    if(!m_bangTex && m_font){
        SDL_Surface* s = TTF_RenderText_Blended(m_font, "!", 0, SDL_Color{30, 30, 40, 255});
        if(s){ m_bangTex = SDL_CreateTextureFromSurface(renderer, s); SDL_DestroySurface(s); }
    }

    SDL_BlendMode prev = SDL_BLENDMODE_NONE;
    SDL_GetRenderDrawBlendMode(renderer, &prev);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    for(int i = 0; i < (int)m_interactables.size(); ++i){
        const auto& it = m_interactables[i];
        // placeholder body (swap for a sprite later)
        SDL_FRect body{it.x - camX, it.y - camY, it.w, it.h};
        SDL_SetRenderDrawColor(renderer, 80, 200, 200, it.inRange ? 120 : 70);
        SDL_RenderFillRect(renderer, &body);
        SDL_SetRenderDrawColor(renderer, 160, 255, 255, 255);
        SDL_RenderRect(renderer, &body);

        if(i == m_active) continue;              // no bubble while talking to it
        SDL_FRect b = bubbleRect(it, camX, camY);
        if(it.inRange) SDL_SetRenderDrawColor(renderer, 255, 220, 80, 255);
        else           SDL_SetRenderDrawColor(renderer, 235, 235, 235, 230);
        SDL_RenderFillRect(renderer, &b);
        SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
        SDL_RenderRect(renderer, &b);
        if(m_bangTex){
            float tw = 0, th = 0; SDL_GetTextureSize(m_bangTex, &tw, &th);
            SDL_FRect t{b.x + (b.w - tw) * 0.5f, b.y + (b.h - th) * 0.5f, tw, th};
            SDL_RenderTexture(renderer, m_bangTex, nullptr, &t);
        }
    }
    SDL_SetRenderDrawBlendMode(renderer, prev);
}

void InteractableBuilder::rebuildTextTextures(SDL_Renderer* renderer){
    if(!m_font || m_active < 0) return;
    const auto& it = m_interactables[m_active];
    const SDL_FRect box = dialogBox();

    // speaker name (only changes with the interactable)
    if(m_nameKey != m_active){
        if(m_nameTex){ SDL_DestroyTexture(m_nameTex); m_nameTex = nullptr; }
        std::string name = it.speaker;
        std::replace(name.begin(), name.end(), '_', ' ');
        if(!name.empty()){
            SDL_Surface* s = TTF_RenderText_Blended(m_font, name.c_str(), 0, SDL_Color{255, 220, 80, 255});
            if(s){ m_nameTex = SDL_CreateTextureFromSurface(renderer, s); SDL_DestroySurface(s); }
        }
        m_nameKey = m_active;
    }

    // the typed-out part of the current page, wrapped to the text area
    int bytes = (int)m_shown;
    const std::string& line = it.lines[std::min(m_page, (int)it.lines.size() - 1)];
    bytes = std::min(bytes, (int)line.size());
    while(bytes > 0 && bytes < (int)line.size() && isContinuationByte((unsigned char)line[bytes])) --bytes;
    const int key = m_active * 1000 + m_page;
    if(bytes == m_textShownBytes && key == m_textKey) return;
    if(m_textTex){ SDL_DestroyTexture(m_textTex); m_textTex = nullptr; }
    m_textShownBytes = bytes;
    m_textKey = key;
    if(bytes <= 0) return;
    SDL_Surface* s = TTF_RenderText_Blended_Wrapped(m_font, line.c_str(), (size_t)bytes,
                                                    SDL_Color{255, 255, 255, 255}, (int)(box.w - 2.0f * BOX_PAD));
    if(s){ m_textTex = SDL_CreateTextureFromSurface(renderer, s); SDL_DestroySurface(s); }
}

void InteractableBuilder::renderDialog(SDL_Renderer* renderer){
    if(m_active < 0) return;
    rebuildTextTextures(renderer);

    const SDL_FRect box = dialogBox();
    SDL_BlendMode prev = SDL_BLENDMODE_NONE;
    SDL_GetRenderDrawBlendMode(renderer, &prev);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // speaker tab
    float nameW = 0, nameH = 0;
    if(m_nameTex) SDL_GetTextureSize(m_nameTex, &nameW, &nameH);
    if(m_nameTex){
        SDL_FRect tab{box.x + 24.0f, box.y - NAME_TAB_H, nameW + 40.0f, NAME_TAB_H + 3.0f};
        SDL_SetRenderDrawColor(renderer, 10, 12, 24, 235);
        SDL_RenderFillRect(renderer, &tab);
        SDL_SetRenderDrawColor(renderer, 255, 220, 80, 255);
        SDL_RenderRect(renderer, &tab);
        SDL_FRect n{tab.x + 20.0f, tab.y + (NAME_TAB_H - nameH) * 0.5f, nameW, nameH};
        SDL_RenderTexture(renderer, m_nameTex, nullptr, &n);
    }

    // the dialog box area
    SDL_SetRenderDrawColor(renderer, 10, 12, 24, 235);
    SDL_RenderFillRect(renderer, &box);
    SDL_SetRenderDrawColor(renderer, 255, 220, 80, 255);
    SDL_RenderRect(renderer, &box);
    SDL_FRect inner{box.x + 5.0f, box.y + 5.0f, box.w - 10.0f, box.h - 10.0f};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 120);
    SDL_RenderRect(renderer, &inner);

    // text, clipped to the text area so a long page can't spill out of the box
    const SDL_Rect clip{(int)(box.x + BOX_PAD), (int)(box.y + BOX_PAD),
                        (int)(box.w - 2.0f * BOX_PAD), (int)(box.h - 2.0f * BOX_PAD)};
    if(m_textTex){
        float tw = 0, th = 0; SDL_GetTextureSize(m_textTex, &tw, &th);
        SDL_SetRenderClipRect(renderer, &clip);
        SDL_FRect t{box.x + BOX_PAD, box.y + BOX_PAD, tw, th};
        SDL_RenderTexture(renderer, m_textTex, nullptr, &t);
        SDL_SetRenderClipRect(renderer, nullptr);
    }

    // blinking "next" arrow once the page is fully typed
    if(m_shown >= (float)fullLength() && std::fmod(m_time, 0.8f) < 0.5f){
        const float ax = box.x + box.w - BOX_PAD - 20.0f;
        const float ay = box.y + box.h - BOX_PAD - 6.0f;
        SDL_SetRenderDrawColor(renderer, 255, 220, 80, 255);
        for(int row = 0; row < 6; ++row){
            SDL_FRect r{ax + row * 2.0f, ay - 12.0f + row * 2.0f, 20.0f - row * 4.0f, 2.0f};
            SDL_RenderFillRect(renderer, &r);
        }
    }
    SDL_SetRenderDrawBlendMode(renderer, prev);
}
