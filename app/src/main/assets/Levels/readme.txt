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

Z TOKEN [TOKEN ...]   (also KEY=VALUE options, all optional)
    ambience / weather for the level. Without a Z line the biome decides (and the D darkness):
      forest / meadows : leaves or petals + wind, fireflies from dusk on (D >= 0.12), sun + lens flare on bright days (D <= 0.15)
      stone cave       : nothing extra (dust motes only)        bright (blue) cave : blue glow-worm fireflies
      There are no daytime flies any more: the only insect is the firefly (FLIES is accepted and means FIREFLIES).
    TOKENS: FIREFLIES PETALS LEAVES WIND RAIN SNOW THUNDER SUNFLARE STORM (= RAIN THUNDER WIND) NONE AUTO
    A Z line REPLACES the biome defaults. Put AUTO first to keep them, and prefix a token with - to remove it.
    (rain / snow always remove SUNFLARE)
    KEY=VALUE options (a flag name with a value switches it on with that strength, =0 switches it off):
      RAIN=0.2..3      rain density          SNOW=0.2..3    snow density        SNOWSIZE=0.5..2.5  flake size
      WIND=n           wind strength (1 = default, 3 = gale). WIND=-2 = strength 2 blowing LEFT. WINDDIR=-1|1 forces a side
      GUST=0..3        gust strength + how often
      ANGLE=-75..75    constant slant of rain/snow in degrees (+ leans right), added to what the wind does
                       (rain falls at the angle the wind pushes it: slight breeze = slight slant, WIND=3 = extreme)
      FOG=0..1         haze (default: a little in rain, more in snow and more still in a blizzard)
      THUNDER=0.2..5   lightning frequency   FIREFLIES=0..3 / LEAVES=0..3 / PETALS=0..3   density
      GLOW=r,g,b       firefly colour
      SUNFLARE=0..2    lens flare strength   SUNCOLOR=r,g,b   sun colour
      SUNPOS=x,y       a FIXED sun position on screen, as fractions of the screen (0,0 = top left, 1,1 = bottom right):
                         0.5,0.1 noon, top centre      0.5,0.85 sunset, near the bottom centre      0.9,0.12 top right corner
                       values may leave 0..1 (y=1.0 sun half below the bottom edge, x=-0.05 just outside the left edge)
      SUNPATH=x0,y0,x1,y1   a MOVING sun: where it is at the START and at the END of the level; it slides between them
                       as the player travels, flare ghosts/halo/god rays move with it (SUNPATH=x,y with 2 numbers = SUNPOS)
      SUNPRESET=NOON|MORNING|AFTERNOON|DAWN|SUNSET|DUSK|TRAVEL   ready-made position + colour (SUNCOLOR= overrides the colour)
      SUNSIZE=0.4..3   size of the sun's glow / core / starburst (a low big sunset sun: SUNSIZE=1.6)
      SUNDRIFT=-0.5..0.5   how much the sun follows the camera's vertical travel (default 0.04, 0 = nailed to the sky)
      note: the flare fades with the level's darkness (D line), so a sunset level should not be too dark (D <= 0.3)
      DENSITY=0.25..1.5  scales every particle count    SOUND=0..2  ambience volume of this level (0 = silent)
      STEPS=GRASS|LEAVES|SNOW|PUDDLE|CONCRETE   footstep surface (default: snow > rain=puddle > leaves > grass in forest/meadow > concrete)
    sounds are automatic: rain loop (rain_light_loop) + wind loop follow the weather, a gust whoosh when a gust starts,
    thunder plays on every lightning strike.
    examples:  Z SNOW WIND                  only snow + wind
               Z AUTO RAIN                  biome defaults + rain
               Z AUTO -SUNFLARE             biome defaults, no lens flare
               Z STORM                      rain + lightning + thunder + strong wind
               Z RAIN=1.5 WIND=-2.5 ANGLE=-20   heavy rain slanting hard to the left
               Z SNOW=1.4 WIND=3 GUST=2 FOG=0.4   blizzard
               Z AUTO SUNCOLOR=255,170,110 SUNPATH=0.2,0.3,0.8,0.12   sunset sun that crosses the sky
               Z AUTO SUNPRESET=SUNSET SUNSIZE=1.5   big orange sun low at the bottom centre, fixed
               Z AUTO SUNPOS=0.5,0.1 SUNDRIFT=0   noon sun, top centre, never moves
               Z FIREFLIES=1.5 GLOW=120,220,255   dense blue fireflies
               Z NONE                       nothing

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
    boss (see level/Bosses.cpp for the 13 types). x y = top-left of its hitbox, it falls onto the floor.
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