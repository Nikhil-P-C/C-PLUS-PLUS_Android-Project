//
// Per-level lighting data (parsed from the level file, see Levels/readme.txt).
//
//   D <darkness> [playerRadius playerSoftness]
//       Ambient darkness 0..1 for the whole level (0 = fully lit, 1 = pitch black) and the size of the
//       light that follows the player. Omit the line to get the default look (0.20 / 100 / 250).
//       playerRadius 0 turns the player light off.
//   L <x> <y> <radius> <r> <g> <b> <intensity> [flicker] [visible]
//       A light object in world pixels. radius = how far it reaches, r g b = colour 0..255,
//       intensity 0..1 (how much darkness it removes + how bright its glow is), flicker 0..1
//       (torch/fire wobble, default 0), visible 0/1 (draw a small glowing core at the source, default 1).
//
#pragma once
#include <cstdint>

struct LightSource {
    float x = 0.0f, y = 0.0f;
    float radius = 300.0f;
    uint8_t r = 255, g = 200, b = 120;
    float intensity = 1.0f;
    float flicker   = 0.0f;
    bool  visible   = true;
};

struct LevelAmbient {
    float darkness       = 0.20f;
    float playerRadius   = 100.0f;
    float playerSoftness = 250.0f;
};
