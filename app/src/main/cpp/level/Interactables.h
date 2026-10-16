//
// Interactables: things in the world the player can talk to / read (signs, NPCs, notes...).
// Standing next to one shows a glowing diamond marker; pressing interact (W / E / gamepad stick up or north / the touch
// up button / tap the marker) opens a speech bubble at the TOP of the screen that types out its lines one at a time.
//
// Look: the same language as the main menu (see States/MenuState/MenuKit.h) - black soft shade, the breathing core
// colour + heartbeat from MenuTheme, diamonds and fading lines, the pixel font. Every interactable gives off rising
// light motes (more when you are near it), and opening / advancing / closing a dialog sends out sparks and a
// diamond shock ring.
//
// Level file line:  I x y w h SPEAKER line one|line two|line three
//   x y w h   the trigger area in world px (drawn as a faint placeholder until you have art)
//   SPEAKER   one word shown in the name header; underscores become spaces (OLD_MAN -> "OLD MAN")
//   lines     everything after SPEAKER, split on '|' - each piece is one dialog page
//
#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "States/MenuState/MenuKit.h"

struct Interactable{
    Interactable(float x, float y, float w, float h, std::string speaker, std::vector<std::string> lines)
            : x(x), y(y), w(w), h(h), speaker(std::move(speaker)), lines(std::move(lines)) {}
    float x = 0, y = 0, w = 0, h = 0;      // trigger area (world px)
    std::string speaker;
    std::vector<std::string> lines;         // one entry per dialog page
    bool inRange = false;                   // player overlaps the trigger area (set by update)
    float emitAcc = 0.0f;                   // fractional light motes owed (set by update)
};

class InteractableBuilder{
public:
    InteractableBuilder() = default;
    ~InteractableBuilder();
    InteractableBuilder(const InteractableBuilder&) = delete;
    InteractableBuilder& operator=(const InteractableBuilder&) = delete;

    void init(const std::vector<Interactable>& interactables);   // keeps the font; closes any open dialog
    void setFont(TTF_Font* font) { m_font = font; }              // kept for the old API; the dialog uses the menu font
    const std::vector<Interactable>& getInteractables() const { return m_interactables; }

    void update(float dt, const SDL_FRect& player);
    void renderWorld(SDL_Renderer* renderer);     // glow + light motes + diamond markers (world space)
    void renderDialog(SDL_Renderer* renderer);    // the speech bubble (screen space, top of the screen, draw last)

    // interact pressed: opens the dialog of the interactable in range, or advances the open one
    // (first press finishes the typing, next press goes to the next page, last page closes). true if used.
    bool interact();
    // tap/click in logical screen px: any tap advances an open dialog, otherwise a tap on an in-range marker opens it
    bool onPointerDown(float px, float py, int camX, int camY);

    bool isDialogOpen() const { return m_active >= 0; }
    void closeDialog();

private:
    // one light mote / spark / shock ring. World-space ones live in m_worldFx, bubble ones in m_screenFx.
    struct Fx{
        enum Kind : uint8_t { MOTE, SPARK, RING };
        Kind  kind = MOTE;
        float x = 0, y = 0, vx = 0, vy = 0;
        float age = 0, life = 1;
        float size = 8;                   // MOTE/SPARK: glow diameter base; RING: final diameter
        float swayAmp = 0, swayFreq = 0, phase = 0;   // MOTE: sideways wander
        float drag = 0, gravity = 0;
        float hot = 0.4f;                 // 0 = theme mote colour .. 1 = white
        float a = 1.0f;                   // opacity multiplier
    };
    struct LineTex{
        std::string text;                 // the full wrapped line
        float fullW = 0;                  // its full width, so the line stays put while it types out
        int bytes = -1;                   // bytes currently in `label`
        std::unique_ptr<MenuLabel> label;
    };

    SDL_FRect markerRect(const Interactable& it, int camX, int camY) const;
    void ensureResources(SDL_Renderer* renderer);
    void ensureLayout();                           // wraps the current page (needs the font, not the renderer)
    std::vector<std::string> wrapText(const std::string& text, int maxW);
    int  fullLength() const;                       // bytes in the current page (as wrapped)
    float targetBoxHeight() const;

    // light + sparks
    void addFx(std::vector<Fx>& v, const Fx& f);
    void burstFx(std::vector<Fx>& v, float x, float y, int count, float speedMin, float speedMax,
                 float lifeMin, float lifeMax, float sizeMin, float sizeMax, Fx::Kind kind,
                 float drag, float gravity, float up = 0.0f);
    void ringFx(std::vector<Fx>& v, float x, float y, float diameter, float life, float a = 1.0f);
    void updateFx(std::vector<Fx>& v, float dt);
    void renderFx(const std::vector<Fx>& v, float offX, float offY) const;
    void emitWorldMotes(float dt, const SDL_FRect& view);
    void emitBubbleMotes(float dt);
    void openedBurst(const Interactable& it);
    void pageBurst(const Interactable& it);
    void closedBurst(const Interactable& it);

    std::vector<Interactable> m_interactables;
    TTF_Font* m_font = nullptr;                    // borrowed (old API), unused by the new look

    int   m_active = -1;                           // index into m_interactables, -1 = no dialog
    int   m_page = 0;
    float m_shown = 0.0f;                          // bytes of the current page revealed so far
    float m_time = 0.0f;                           // drives the bobbing, orbiting and blinking
    float m_open = 0.0f;                           // bubble reveal 0..1
    float m_boxH = 150.0f;                         // bubble height, eased toward what the page needs
    float m_bubbleEmit = 0.0f;
    int   m_caretCount = 0;

    std::vector<Fx> m_worldFx, m_screenFx;

    std::unique_ptr<MenuFont> m_menuFont;          // created on first use
    std::unique_ptr<MenuPainter> m_painter;
    std::vector<LineTex> m_lines;                  // the wrapped current page
    int   m_linesKey = -1;
    float m_lineH = 38.0f;
    int   m_totalBytes = 0;
    MenuLabel m_nameLabel;  int m_nameKey = -1;
};
