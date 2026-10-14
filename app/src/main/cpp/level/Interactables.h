//
// Interactables: things in the world the player can talk to / read (signs, NPCs, notes...).
// Standing next to one shows a "!" bubble; pressing interact (E / gamepad north / tap the bubble) opens a
// dialog box at the bottom of the screen that types out its lines one at a time.
//
// Level file line:  I x y w h SPEAKER line one|line two|line three
//   x y w h   the trigger area in world px (drawn as a placeholder rect until you have art)
//   SPEAKER   one word shown in the name tab; underscores become spaces (OLD_MAN -> "OLD MAN")
//   lines     everything after SPEAKER, split on '|' - each piece is one dialog page
//
#pragma once
#include <string>
#include <vector>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

struct Interactable{
    Interactable(float x, float y, float w, float h, std::string speaker, std::vector<std::string> lines)
        : x(x), y(y), w(w), h(h), speaker(std::move(speaker)), lines(std::move(lines)) {}
    float x = 0, y = 0, w = 0, h = 0;      // trigger area (world px)
    std::string speaker;
    std::vector<std::string> lines;         // one entry per dialog page
    bool inRange = false;                   // player overlaps the trigger area (set by update)
};

class InteractableBuilder{
public:
    InteractableBuilder() = default;
    ~InteractableBuilder();
    InteractableBuilder(const InteractableBuilder&) = delete;
    InteractableBuilder& operator=(const InteractableBuilder&) = delete;

    void init(const std::vector<Interactable>& interactables);   // keeps the font; closes any open dialog
    void setFont(TTF_Font* font) { m_font = font; }              // borrowed, the owner closes it
    const std::vector<Interactable>& getInteractables() const { return m_interactables; }

    void update(float dt, const SDL_FRect& player);
    void renderWorld(SDL_Renderer* renderer);     // placeholder rects + "!" bubbles (world space)
    void renderDialog(SDL_Renderer* renderer);    // the dialog box (screen space, draw last)

    // interact pressed: opens the dialog of the interactable in range, or advances the open one
    // (first press finishes the typing, next press goes to the next page, last page closes). true if used.
    bool interact();
    // tap/click in logical screen px: any tap advances an open dialog, otherwise a tap on an in-range bubble opens it
    bool onPointerDown(float px, float py, int camX, int camY);

    bool isDialogOpen() const { return m_active >= 0; }
    void closeDialog();

private:
    SDL_FRect bubbleRect(const Interactable& it, int camX, int camY) const;
    void rebuildTextTextures(SDL_Renderer* renderer);
    void destroyTextures();
    int  fullLength() const;                      // bytes in the current page

    std::vector<Interactable> m_interactables;
    TTF_Font* m_font = nullptr;

    int   m_active = -1;                          // index into m_interactables, -1 = no dialog
    int   m_page = 0;
    float m_shown = 0.0f;                         // bytes of the current page revealed so far
    float m_time = 0.0f;                          // drives the bubble bob + the "next" arrow blink

    // cached text
    SDL_Texture* m_textTex = nullptr;   int m_textShownBytes = -1; int m_textKey = -1;
    SDL_Texture* m_nameTex = nullptr;   int m_nameKey = -1;
    SDL_Texture* m_bangTex = nullptr;
};
