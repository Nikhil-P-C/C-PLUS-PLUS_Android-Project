#include "Interactables.h"
#include <algorithm>
#include <cmath>
#include "utils/utils.h"
#include "States/GameState/GameState.h"   // SCALE, Camera

namespace {
    using MenuKit::Rgb;
    using MenuKit::Align;
    using MenuKit::mix;
    using MenuKit::smooth;
    using MenuKit::approach;
    using MenuKit::to8;

    // speech bubble layout (logical px, the game runs in a fixed 1600x720 space). It hangs from the TOP of the screen.
    constexpr float BUB_W = 1040.0f;
    constexpr float BUB_TOP = 34.0f;
    constexpr float BUB_PAD_X = 64.0f;
    constexpr float NAME_ROW_H = 66.0f;           // speaker header
    constexpr float BUB_PAD_BOTTOM = 50.0f;       // room for the "next" diamond / pager
    constexpr int   NAME_SIZE = 34;
    constexpr int   BODY_SIZE = 30;
    constexpr float CHARS_PER_SEC = 45.0f;        // typewriter speed (bytes/sec)
    constexpr size_t MAX_FX = 420;                // per pool (world / bubble)
    const Rgb WHITE{1.0f, 1.0f, 1.0f};

    bool overlaps(const SDL_FRect& a, const SDL_FRect& b){
        return a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y;
    }
    // is this byte the middle of a UTF-8 character?
    bool isContinuationByte(unsigned char c){ return (c & 0xC0) == 0x80; }

    // tiny xorshift: only used for visual randomness
    float frand(float a, float b){
        static uint32_t s = 0x9E3779B9u;
        s ^= s << 13; s ^= s >> 17; s ^= s << 5;
        return a + (b - a) * ((float)(s >> 8) * (1.0f / 16777216.0f));
    }
}

InteractableBuilder::~InteractableBuilder() = default;   // labels free their textures themselves

void InteractableBuilder::init(const std::vector<Interactable>& interactables){
    m_interactables = interactables;
    m_worldFx.clear();
    m_screenFx.clear();
    closeDialog();
}

void InteractableBuilder::closeDialog(){
    m_active = -1;
    m_page = 0;
    m_shown = 0.0f;
    m_open = 0.0f;
    m_lines.clear();
    m_linesKey = -1;
    m_totalBytes = 0;
    m_nameLabel.reset();
    m_nameKey = -1;
}

// =============================================================================================
// resources + text layout
// =============================================================================================
void InteractableBuilder::ensureResources(SDL_Renderer* renderer){
    if(!m_menuFont) m_menuFont = std::make_unique<MenuFont>(BODY_SIZE);
    if(!m_painter)  m_painter  = std::make_unique<MenuPainter>(renderer);
}

// greedy word wrap on the real font metrics. Over-long words are broken between UTF-8 characters.
std::vector<std::string> InteractableBuilder::wrapText(const std::string& text, int maxW){
    std::vector<std::string> out;
    auto width = [&](const std::string& s){ int w = 0, h = 0; m_menuFont->measure(s.c_str(), BODY_SIZE, w, h); return w; };

    std::string cur;
    size_t pos = 0;
    while(pos <= text.size()){
        size_t sp = text.find(' ', pos);
        if(sp == std::string::npos) sp = text.size();
        const std::string word = text.substr(pos, sp - pos);
        pos = sp + 1;
        if(word.empty()) continue;

        const std::string cand = cur.empty() ? word : cur + " " + word;
        if(width(cand) <= maxW){ cur = cand; continue; }
        if(!cur.empty()){ out.push_back(cur); cur.clear(); }
        if(width(word) <= maxW){ cur = word; continue; }

        std::string piece;                                   // a single word wider than the bubble
        for(size_t k = 0; k < word.size();){
            size_t n = 1;
            while(k + n < word.size() && isContinuationByte((unsigned char)word[k + n])) ++n;
            std::string next = piece + word.substr(k, n);
            if(!piece.empty() && width(next) > maxW){ out.push_back(piece); next = word.substr(k, n); }
            piece = next;
            k += n;
        }
        cur = piece;
    }
    if(!cur.empty()) out.push_back(cur);
    if(out.empty()) out.push_back("");
    return out;
}

void InteractableBuilder::ensureLayout(){
    if(m_active < 0) return;
    const auto& pages = m_interactables[m_active].lines;
    if(m_page < 0 || m_page >= (int)pages.size()) return;
    const int key = m_active * 1000 + m_page;
    if(key == m_linesKey) return;

    if(!m_menuFont) m_menuFont = std::make_unique<MenuFont>(BODY_SIZE);
    m_linesKey = key;
    m_lines.clear();
    m_totalBytes = 0;

    int w = 0, h = 0;
    if(m_menuFont->measure("Ag", BODY_SIZE, w, h)) m_lineH = (float)h + 10.0f;

    std::vector<std::string> wrapped;
    if(m_menuFont->ok()) wrapped = wrapText(pages[m_page], (int)(BUB_W - 2.0f * BUB_PAD_X));
    else                 wrapped.push_back(pages[m_page]);

    for(const auto& s : wrapped){
        LineTex lt;
        lt.text = s;
        int lw = 0, lh = 0;
        m_menuFont->measure(s.c_str(), BODY_SIZE, lw, lh);
        lt.fullW = (float)lw;
        m_totalBytes += (int)s.size();
        m_lines.push_back(std::move(lt));
    }
}

int InteractableBuilder::fullLength() const{
    if(m_active < 0) return 0;
    const auto& pages = m_interactables[m_active].lines;
    if(m_page < 0 || m_page >= (int)pages.size()) return 0;
    if(m_linesKey == m_active * 1000 + m_page) return m_totalBytes;
    return (int)pages[m_page].size();
}

float InteractableBuilder::targetBoxHeight() const{
    const float rows = (float)std::max<size_t>(m_lines.size(), 2);
    return NAME_ROW_H + rows * m_lineH + BUB_PAD_BOTTOM;
}

// =============================================================================================
// light motes, sparks, shock rings
// =============================================================================================
void InteractableBuilder::addFx(std::vector<Fx>& v, const Fx& f){
    if(v.size() < MAX_FX) v.push_back(f);
}

void InteractableBuilder::burstFx(std::vector<Fx>& v, float x, float y, int count, float speedMin, float speedMax,
                                  float lifeMin, float lifeMax, float sizeMin, float sizeMax, Fx::Kind kind,
                                  float drag, float gravity, float up){
    for(int i = 0; i < count; ++i){
        const float ang = frand(0.0f, 6.2831853f);
        const float sp = frand(speedMin, speedMax);
        Fx f;
        f.kind = kind;
        f.x = x; f.y = y;
        f.vx = std::cos(ang) * sp;
        f.vy = std::sin(ang) * sp - up;
        f.life = frand(lifeMin, lifeMax);
        f.size = frand(sizeMin, sizeMax);
        f.drag = drag; f.gravity = gravity;
        f.swayAmp = frand(3.0f, 9.0f); f.swayFreq = frand(1.2f, 2.6f); f.phase = frand(0.0f, 6.28f);
        f.hot = frand(0.3f, 0.8f);
        addFx(v, f);
    }
}

void InteractableBuilder::ringFx(std::vector<Fx>& v, float x, float y, float diameter, float life, float a){
    Fx f;
    f.kind = Fx::RING;
    f.x = x; f.y = y;
    f.size = diameter; f.life = life; f.a = a;
    addFx(v, f);
}

void InteractableBuilder::updateFx(std::vector<Fx>& v, float dt){
    for(size_t i = 0; i < v.size();){
        Fx& f = v[i];
        f.age += dt;
        if(f.age >= f.life){ f = v.back(); v.pop_back(); continue; }
        if(f.drag > 0.0f){ const float k = std::exp(-f.drag * dt); f.vx *= k; f.vy *= k; }
        f.vy += f.gravity * dt;
        f.x += f.vx * dt;
        f.y += f.vy * dt;
        ++i;
    }
}

// everything is tinted from the live menu theme each frame, so the light shifts colour with the main menu
void InteractableBuilder::renderFx(const std::vector<Fx>& v, float offX, float offY) const{
    if(!m_painter) return;
    const MenuTheme& th = MenuTheme::get();
    for(const Fx& f : v){
        const float t = f.age / f.life;
        float px = f.x - offX;
        const float py = f.y - offY;
        switch(f.kind){
            case Fx::MOTE: {
                const float env = t < 0.2f ? smooth(t / 0.2f) : smooth((1.0f - t) / 0.8f);
                px += std::sin(f.phase + f.age * f.swayFreq) * f.swayAmp;
                const Rgb c = mix(th.mote(), WHITE, f.hot);
                m_painter->glow(px, py, f.size * 4.2f, f.size * 4.2f, c, env * 0.42f * f.a);
                m_painter->glow(px, py, f.size * 1.5f, f.size * 1.5f, mix(c, WHITE, 0.55f), env * 0.95f * f.a);
            } break;
            case Fx::SPARK: {
                const float env = (1.0f - t) * (1.0f - t);
                const float s = f.size * (1.0f - 0.55f * t);
                m_painter->glow(px, py, s * 3.4f, s * 3.4f, th.accent(), env * 0.55f * f.a);
                m_painter->glow(px, py, s * 1.2f, s * 1.2f, WHITE, env * 0.95f * f.a);
            } break;
            case Fx::RING: {
                const float e = 1.0f - (1.0f - t) * (1.0f - t);          // ease out
                const float half = f.size * 0.5f * e;
                m_painter->diamondOutline(px, py, half, 2.0f + 2.0f * (1.0f - t), th.accent(),
                                          (1.0f - t) * (1.0f - t) * 0.8f * f.a);
            } break;
        }
    }
}

// every interactable gives off rising light: a little when idle, more when you are near, a lot while it is talking
void InteractableBuilder::emitWorldMotes(float dt, const SDL_FRect& view){
    for(int i = 0; i < (int)m_interactables.size(); ++i){
        Interactable& it = m_interactables[i];
        if(!overlaps(SDL_FRect{it.x, it.y, it.w, it.h}, view)) continue;
        const float rate = (i == m_active) ? 36.0f : (it.inRange ? 22.0f : 8.0f);
        it.emitAcc = std::min(it.emitAcc + rate * dt, 6.0f);
        while(it.emitAcc >= 1.0f){
            it.emitAcc -= 1.0f;
            Fx f;
            f.kind = Fx::MOTE;
            f.x = it.x + frand(0.0f, it.w);
            f.y = it.y + it.h * frand(0.3f, 1.0f);
            f.vx = frand(-6.0f, 6.0f);
            f.vy = -frand(22.0f, 60.0f);
            f.swayAmp = frand(5.0f, 16.0f);
            f.swayFreq = frand(1.2f, 2.6f);
            f.phase = frand(0.0f, 6.283f);
            f.life = frand(1.8f, 3.4f);
            f.size = frand(5.0f, 12.0f);
            f.hot = frand(0.2f, 0.7f);
            if(frand(0.0f, 1.0f) < 0.18f){               // now and then a big soft bokeh
                f.size = frand(14.0f, 22.0f);
                f.a = 0.35f;
                f.life = frand(2.6f, 4.2f);
                f.vy *= 0.6f;
            }
            addFx(m_worldFx, f);
        }
    }
}

// light drifting up off the top and bottom edge of the open bubble
void InteractableBuilder::emitBubbleMotes(float dt){
    m_bubbleEmit = std::min(m_bubbleEmit + 26.0f * dt, 6.0f);
    const float cx = MenuKit::SCREEN_W * 0.5f;
    while(m_bubbleEmit >= 1.0f){
        m_bubbleEmit -= 1.0f;
        Fx f;
        f.kind = Fx::MOTE;
        f.x = cx + frand(-BUB_W * 0.5f - 20.0f, BUB_W * 0.5f + 20.0f);
        f.y = BUB_TOP + (frand(0.0f, 1.0f) < 0.5f ? 0.0f : m_boxH);
        f.vx = frand(-8.0f, 8.0f);
        f.vy = -frand(14.0f, 40.0f);
        f.swayAmp = frand(4.0f, 10.0f);
        f.swayFreq = frand(1.2f, 2.4f);
        f.phase = frand(0.0f, 6.283f);
        f.life = frand(1.6f, 3.0f);
        f.size = frand(4.0f, 9.0f);
        f.hot = 0.5f;
        f.a = 0.7f;
        addFx(m_screenFx, f);
    }
}

void InteractableBuilder::openedBurst(const Interactable& it){
    const float cx = it.x + it.w * 0.5f, cy = it.y + it.h * 0.45f;
    burstFx(m_worldFx, cx, cy, 30, 70.0f, 270.0f, 0.45f, 1.1f, 6.0f, 11.0f, Fx::SPARK, 2.6f, 0.0f);
    burstFx(m_worldFx, cx, cy, 16, 25.0f, 90.0f, 1.0f, 2.0f, 6.0f, 13.0f, Fx::MOTE, 1.2f, 0.0f, 40.0f);
    ringFx(m_worldFx, cx, it.y + it.h * 0.8f, 260.0f, 0.75f);
    ringFx(m_worldFx, cx, cy, 140.0f, 0.5f, 0.8f);

    // the bubble opens in a spray of light along its top edge
    const float sx = MenuKit::SCREEN_W * 0.5f;
    for(int i = 0; i < 34; ++i){
        Fx f;
        f.kind = Fx::SPARK;
        f.x = sx + frand(-BUB_W * 0.5f, BUB_W * 0.5f);
        f.y = BUB_TOP + frand(0.0f, 20.0f);
        f.vx = frand(-30.0f, 30.0f);
        f.vy = -frand(20.0f, 110.0f);
        f.life = frand(0.5f, 1.1f);
        f.size = frand(5.0f, 10.0f);
        f.drag = 2.0f;
        f.a = 0.9f;
        addFx(m_screenFx, f);
    }
    ringFx(m_screenFx, sx, BUB_TOP + 80.0f, 900.0f, 0.8f, 0.6f);
}

void InteractableBuilder::pageBurst(const Interactable& it){
    burstFx(m_worldFx, it.x + it.w * 0.5f, it.y + it.h * 0.4f, 10, 50.0f, 170.0f, 0.4f, 0.8f, 5.0f, 9.0f, Fx::SPARK, 2.8f, 0.0f);
    const float sx = MenuKit::SCREEN_W * 0.5f;
    for(int i = 0; i < 12; ++i){
        Fx f;
        f.kind = Fx::SPARK;
        f.x = sx + frand(-60.0f, 60.0f);
        f.y = BUB_TOP + m_boxH - 20.0f;
        f.vx = frand(-70.0f, 70.0f);
        f.vy = -frand(20.0f, 90.0f);
        f.life = frand(0.4f, 0.8f);
        f.size = frand(4.0f, 8.0f);
        f.drag = 2.4f;
        addFx(m_screenFx, f);
    }
}

void InteractableBuilder::closedBurst(const Interactable& it){
    const float cx = it.x + it.w * 0.5f, cy = it.y + it.h * 0.5f;
    burstFx(m_worldFx, cx, cy, 24, 20.0f, 80.0f, 1.2f, 2.2f, 6.0f, 13.0f, Fx::MOTE, 1.0f, 0.0f, 70.0f);
    ringFx(m_worldFx, cx, cy, 180.0f, 0.6f, 0.7f);
    // the bubble dissolves into light
    const float sx = MenuKit::SCREEN_W * 0.5f;
    for(int i = 0; i < 28; ++i){
        Fx f;
        f.kind = Fx::MOTE;
        f.x = sx + frand(-BUB_W * 0.5f, BUB_W * 0.5f);
        f.y = BUB_TOP + frand(0.0f, m_boxH);
        f.vx = frand(-10.0f, 10.0f);
        f.vy = -frand(30.0f, 90.0f);
        f.swayAmp = frand(4.0f, 10.0f); f.swayFreq = frand(1.2f, 2.4f); f.phase = frand(0.0f, 6.28f);
        f.life = frand(0.8f, 1.6f);
        f.size = frand(5.0f, 10.0f);
        f.hot = 0.6f;
        addFx(m_screenFx, f);
    }
}

// =============================================================================================
// update / interact
// =============================================================================================
void InteractableBuilder::update(float dt, const SDL_FRect& player){
    m_time += dt;
    if(!m_interactables.empty()) MenuTheme::get().update(dt);   // in game nothing else ticks the menu colour cycle + heartbeat

    for(auto& it : m_interactables)
        it.inRange = overlaps(player, SDL_FRect{it.x, it.y, it.w, it.h});

    if(m_active >= 0){
        ensureLayout();
        const int len = fullLength();
        if(m_shown < (float)len) m_shown = std::min((float)len, m_shown + CHARS_PER_SEC * dt);
        m_open = approach(m_open, 1.0f, 9.0f, dt);
        m_boxH = approach(m_boxH, targetBoxHeight(), 12.0f, dt);
        emitBubbleMotes(dt);
    }

    const SDL_FRect cam = Camera::getInstance().getCamera();
    const SDL_FRect view{cam.x - 160.0f, cam.y - 160.0f,
                         (float)GameData::getInstance().getWinWidth() + 320.0f,
                         (float)GameData::getInstance().getWinHeight() + 320.0f};
    emitWorldMotes(dt, view);
    updateFx(m_worldFx, dt);
    updateFx(m_screenFx, dt);
}

bool InteractableBuilder::interact(){
    if(m_active < 0){
        for(int i = 0; i < (int)m_interactables.size(); ++i){
            if(!m_interactables[i].inRange || m_interactables[i].lines.empty()) continue;
            m_active = i;
            m_page = 0;
            m_shown = 0.0f;
            m_open = 0.0f;
            m_linesKey = -1;
            ensureLayout();
            m_boxH = targetBoxHeight();
            openedBurst(m_interactables[i]);
            return true;
        }
        return false;
    }
    ensureLayout();
    if(m_shown < (float)fullLength()){          // still typing -> finish the page instantly
        m_shown = (float)fullLength();
        return true;
    }
    const int idx = m_active;
    ++m_page;
    if(m_page >= (int)m_interactables[idx].lines.size()){
        closedBurst(m_interactables[idx]);
        closeDialog();
    } else {
        m_shown = 0.0f;
        ensureLayout();
        pageBurst(m_interactables[idx]);
    }
    return true;
}

SDL_FRect InteractableBuilder::markerRect(const Interactable& it, int camX, int camY) const{
    const float bob = std::sin(m_time * 3.2f) * 5.0f;
    const float mw = 44.0f, mh = 52.0f;
    const float cx = it.x + it.w * 0.5f - (float)camX;
    const float cy = it.y - 40.0f + bob - (float)camY;
    return SDL_FRect{cx - mw * 0.5f, cy - mh * 0.5f, mw, mh};
}

bool InteractableBuilder::onPointerDown(float px, float py, int camX, int camY){
    if(m_active >= 0) return interact();         // any tap while a dialog is open advances it
    for(const auto& it : m_interactables){
        if(!it.inRange) continue;
        SDL_FRect b = markerRect(it, camX, camY);
        b.x -= 24; b.y -= 24; b.w += 48; b.h += 48;   // generous touch target
        if(px >= b.x && px <= b.x + b.w && py >= b.y && py <= b.y + b.h) return interact();
    }
    return false;
}

// =============================================================================================
// render: world
// =============================================================================================
void InteractableBuilder::renderWorld(SDL_Renderer* renderer){
    if(m_interactables.empty()) return;
    ensureResources(renderer);
    if(!m_painter) return;
    const int camX = (int)std::round(Camera::getInstance().getCamera().x);
    const int camY = (int)std::round(Camera::getInstance().getCamera().y);
    const MenuTheme& th = MenuTheme::get();
    const float pulse = 0.85f + 0.30f * th.pulse();
    const SDL_FRect screen{-220.0f, -220.0f, (float)GameData::getInstance().getWinWidth() + 440.0f,
                           (float)GameData::getInstance().getWinHeight() + 440.0f};

    SDL_BlendMode prev = SDL_BLENDMODE_NONE;
    SDL_GetRenderDrawBlendMode(renderer, &prev);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // 1) pools of light on the ground + the (faint) trigger area. Swap the placeholder for a sprite later.
    for(int i = 0; i < (int)m_interactables.size(); ++i){
        const auto& it = m_interactables[i];
        const SDL_FRect body{it.x - (float)camX, it.y - (float)camY, it.w, it.h};
        if(!overlaps(body, screen)) continue;
        const bool near = it.inRange || i == m_active;
        m_painter->glow(body.x + body.w * 0.5f, body.y + body.h, body.w * 1.9f + 60.0f, 90.0f,
                        th.core(), (near ? 0.55f : 0.28f) * pulse);
        m_painter->glow(body.x + body.w * 0.5f, body.y + body.h * 0.5f, body.w + 120.0f, body.h + 120.0f,
                        th.core(), near ? 0.20f : 0.08f);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, to8(th.core().r), to8(th.core().g), to8(th.core().b), to8(near ? 0.16f : 0.08f));
        SDL_RenderFillRect(renderer, &body);
        SDL_SetRenderDrawColor(renderer, to8(th.accent().r), to8(th.accent().g), to8(th.accent().b), to8(near ? 0.55f : 0.25f));
        SDL_RenderRect(renderer, &body);
    }

    // 2) rising light motes, sparks and shock rings
    renderFx(m_worldFx, (float)camX, (float)camY);

    // 3) the interact markers: a glowing diamond with orbiting lights when you are close
    for(int i = 0; i < (int)m_interactables.size(); ++i){
        const auto& it = m_interactables[i];
        if(i == m_active) continue;                  // no marker while talking to it
        const SDL_FRect mr = markerRect(it, camX, camY);
        if(!overlaps(mr, screen)) continue;
        const float mx = mr.x + mr.w * 0.5f, my = mr.y + mr.h * 0.5f;
        const float k = it.inRange ? 1.0f : 0.55f;

        m_painter->glow(mx, my, 130.0f, 130.0f, th.core(), 0.55f * k * pulse);
        if(it.inRange){
            for(int j = 0; j < 4; ++j){
                const float ang = m_time * 1.9f + (float)j * 1.5707963f;
                const float ox = std::cos(ang) * 32.0f, oy = std::sin(ang) * 13.0f;
                m_painter->glow(mx + ox, my + oy, 24.0f, 24.0f, th.accent(), 0.85f);
                m_painter->glow(mx + ox, my + oy, 9.0f, 9.0f, WHITE, 1.0f);
            }
        }
        const float s = it.inRange ? 19.0f : 14.0f;
        m_painter->diamond(mx, my, s + 6.0f, th.core(), 0.50f * k);
        m_painter->diamond(mx, my, s, mix(th.accent(), WHITE, 0.35f), 0.95f * k + 0.05f);
        m_painter->diamondOutline(mx, my, s + 9.0f, 2.0f, th.accent(), 0.60f * k);
    }
    SDL_SetRenderDrawBlendMode(renderer, prev);
}

// =============================================================================================
// render: the speech bubble (top of the screen, main-menu look)
// =============================================================================================
void InteractableBuilder::renderDialog(SDL_Renderer* renderer){
    if(m_active < 0) return;
    ensureResources(renderer);
    ensureLayout();
    if(!m_painter || !m_menuFont || !m_menuFont->ok()) return;

    const Interactable& it = m_interactables[m_active];
    const MenuTheme& th = MenuTheme::get();
    const int camX = (int)std::round(Camera::getInstance().getCamera().x);

    // speaker name (only changes with the interactable)
    if(m_nameKey != m_active){
        std::string name = it.speaker;
        std::replace(name.begin(), name.end(), '_', ' ');
        m_nameLabel.set(renderer, *m_menuFont, name.c_str(), NAME_SIZE);
        m_nameKey = m_active;
    }

    const float cx = MenuKit::SCREEN_W * 0.5f;
    const float e = smooth(m_open);
    const float top = BUB_TOP - (1.0f - e) * 26.0f;        // slides down into place as it fades in
    const float h = m_boxH;
    const float lx = cx - BUB_W * 0.5f, rx = cx + BUB_W * 0.5f;

    // the typed-out part of each wrapped line (only the line being typed gets a new texture)
    const int shown = (int)m_shown;
    int before = 0;
    for(size_t i = 0; i < m_lines.size(); ++i){
        LineTex& lt = m_lines[i];
        int n = std::clamp(shown - before, 0, (int)lt.text.size());
        while(n > 0 && n < (int)lt.text.size() && isContinuationByte((unsigned char)lt.text[n])) --n;
        before += (int)lt.text.size();
        if(n == lt.bytes) continue;
        lt.bytes = n;
        if(!lt.label) lt.label = std::make_unique<MenuLabel>();
        if(n > 0) lt.label->set(renderer, *m_menuFont, lt.text.substr(0, n).c_str(), BODY_SIZE);
        else      lt.label->reset();
        if(n > 0 && n < (int)lt.text.size() && (++m_caretCount % 2) == 0){   // glitter at the typing caret
            Fx f;
            f.kind = Fx::SPARK;
            f.x = cx - lt.fullW * 0.5f + lt.label->w;
            f.y = top + NAME_ROW_H + 12.0f + ((float)i + 0.5f) * m_lineH;
            f.vx = frand(-14.0f, 14.0f);
            f.vy = -frand(10.0f, 40.0f);
            f.life = frand(0.35f, 0.7f);
            f.size = frand(4.0f, 8.0f);
            f.drag = 2.0f;
            f.a = 0.9f;
            addFx(m_screenFx, f);
        }
    }

    SDL_BlendMode prev = SDL_BLENDMODE_NONE;
    SDL_GetRenderDrawBlendMode(renderer, &prev);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    const float midY = top + h * 0.5f;
    const float pulse = th.pulse();

    // 1) soft black so the text reads over any level, then the core colour breathing at the top edge
    m_painter->shade(cx, midY, BUB_W + 460.0f, h + 320.0f, 0.80f * e);
    m_painter->shade(cx, midY, BUB_W + 160.0f, h + 120.0f, 0.60f * e);
    m_painter->glow(cx, top, BUB_W + 380.0f, 280.0f, th.core(), (0.20f + 0.12f * pulse) * e);
    m_painter->glow(cx, top + h, BUB_W + 120.0f, 100.0f, th.core(), 0.14f * e);

    // 2) drifting light, behind the text
    renderFx(m_screenFx, 0.0f, 0.0f);

    // 3) frame: fading lines top and bottom, a diamond on top, a small tail toward the speaker
    m_painter->fadeLine(lx - 70.0f, cx, top, th.accent(), 0.0f, 0.85f * e);
    m_painter->fadeLine(cx, rx + 70.0f, top, th.accent(), 0.85f * e, 0.0f);
    m_painter->fadeLine(lx - 30.0f, cx, top + h, th.accent(), 0.0f, 0.55f * e);
    m_painter->fadeLine(cx, rx + 30.0f, top + h, th.accent(), 0.55f * e, 0.0f);
    m_painter->diamond(cx, top, 6.0f, th.accent(), e);
    {
        const float sx = std::clamp(it.x + it.w * 0.5f - (float)camX, lx + 90.0f, rx - 90.0f);
        const MenuKit::Rgb& a = th.accent();
        const SDL_FColor c0{a.r, a.g, a.b, 0.80f * e};
        const SDL_FColor c1{a.r, a.g, a.b, 0.10f * e};
        const SDL_Vertex v[3] = {
                {{sx - 12.0f, top + h}, c0, {0, 0}},
                {{sx + 12.0f, top + h}, c0, {0, 0}},
                {{sx, top + h + 20.0f}, c1, {0, 0}},
        };
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderGeometry(renderer, nullptr, v, 3, nullptr, 0);
    }

    // 4) speaker header, flanked like a main-menu title
    const float nameY = top + 34.0f;
    m_painter->flank(m_nameLabel, cx, nameY, 1.0f, 1.0f, e);
    m_painter->label(m_nameLabel, cx, nameY, 1.0f, th.accent(), e);
    m_painter->fadeLine(cx - 260.0f, cx, top + NAME_ROW_H, th.accent(), 0.0f, 0.35f * e);
    m_painter->fadeLine(cx, cx + 260.0f, top + NAME_ROW_H, th.accent(), 0.35f * e, 0.0f);

    // 5) the page, centred line by line (each line keeps its final position while it types out)
    const Rgb bodyColor = mix(WHITE, th.accent(), 0.16f);
    for(size_t i = 0; i < m_lines.size(); ++i){
        const LineTex& lt = m_lines[i];
        if(!lt.label || !lt.label->tex) continue;
        const float y = top + NAME_ROW_H + 12.0f + ((float)i + 0.5f) * m_lineH;
        m_painter->label(*lt.label, cx - lt.fullW * 0.5f, y, 1.0f, bodyColor, e, Align::LEFT);
    }

    // 6) footer: a pulsing diamond when the page is fully typed, page dots when there is more than one
    const float footY = top + h - 22.0f;
    if(m_shown >= (float)fullLength()){
        const float blink = 0.55f + 0.45f * std::sin(m_time * 5.0f);
        const float by = footY + std::sin(m_time * 3.2f) * 3.0f;
        m_painter->glow(cx, by, 60.0f, 60.0f, th.core(), 0.5f * blink * e);
        m_painter->diamond(cx, by, 6.0f, th.accent(), blink * e);
    }
    const int pages = (int)it.lines.size();
    if(pages > 1){
        for(int p = 0; p < pages; ++p){
            const float px = rx - 40.0f - (float)(pages - 1 - p) * 20.0f;
            if(p == m_page) m_painter->diamond(px, footY, 5.0f, th.accent(), e);
            else            m_painter->diamondOutline(px, footY, 5.0f, 1.6f, th.dim(), 0.8f * e);
        }
    }
    SDL_SetRenderDrawBlendMode(renderer, prev);
}
