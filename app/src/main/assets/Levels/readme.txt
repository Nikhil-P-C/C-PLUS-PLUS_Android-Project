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
