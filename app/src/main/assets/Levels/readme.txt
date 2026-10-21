#Level: Level-Number

S spawn
W walls
R WallCollisionRect
P Platforms
G Grounds
B Blocks
F Fruits
T Traps
C CheckPoint
D darkness [playerRadius playerSoftness]
    ambient darkness 0..1 for the level + size of the light around the player (default 0.20 100 250)
L x y radius r g b intensity [flicker] [visible]
    light object in world pixels. r g b = 0..255, intensity 0..1, flicker 0..1 (torch wobble), visible 0/1 (glowing core)
    example:  L 600 430 420 255 170 90 1.0 0.35 1

E x y ENEMY_TYPE pathStartX pathEndX speed [AERIAL]
    enemy that patrols the horizontal line pathStartX..pathEndX (x should be between them) at speed px/s.
    ground enemies fall onto the floor from y, AERIAL ones fly and hover at y. ENEMY_TYPE = any EnemyType name
    (SPIDER_CHILD, CAVE_RAT, HUNTER_EAGLE, ...). Each enemy picks its attack by distance (see Enemies.cpp).
    example:  E 1200 600 CAVE_RAT 1000 1500 120

I x y w h SPEAKER line one|line two|line three
    interactable (sign / NPC): trigger area x y w h in world px, SPEAKER = one word shown in the name tab
    (underscores become spaces), then the dialog text; each '|' starts a new page.
    Walk into the area, press E (gamepad: north button, touch: tap the "!" bubble) to talk; the same
    button advances the text. The player is held still while a dialog is open.
    example:  I 300 560 64 208 OLD_MAN Welcome, traveler.|Mind the cave ahead.

K x y BOSS_TYPE [phase1Music [phase2Music [phase3Music] [gain]]]
    boss (see level/Bosses.cpp for the 14 types). x y = top-left of its hitbox, it falls onto the floor.
    The fight starts when the player walks into the arena (A); until the boss dies the camera is pinned on the
    arena and the player cannot leave it. Boss music: phase 1 track from the start of the fight, phase 2 track when
    the boss roars at 50% hp, fade out on the kill, the level's own music (M / level_<n>.ogg) returns after ~3 s.
    Phase 3 exists only for the Preacher (falls back to phase 2, then 1).
    Default files: Sound/Music/Boss/<biome>_<mini|main>_phase<1|2|3>.ogg   (biome = moss stone shaft plains woods mountain peak)
    A bare file name = Sound/Music/Boss/<name>; '-' = default for that phase. Missing phase 2 keeps phase 1 playing,
    missing phase 1 keeps the level music playing.
    example:  K 3385 360 PEAK_REX_EMPEROR rex_1.ogg rex_2.ogg 0.9

A x y w h
    boss arena = the rectangle the camera/player are locked to. Must fit one screen (<= 1600 x 720).
    Put it at the end of the level (level width 4000 -> A 2400 0 1600 704).
    The level's exit (C) is hidden and disabled until every boss in the level is dead.
