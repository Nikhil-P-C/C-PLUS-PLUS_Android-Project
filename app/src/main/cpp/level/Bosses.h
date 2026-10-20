//
// Bosses.h - 7 biomes x (1 mini boss + 1 main boss), 2 phases each, 4-5 attacks each.
//
// Mirrors EnemiesBuilder on purpose so it plugs into GameState at the same call sites:
//   init / update / render / applyPlayerDamage / checkAttackOnPlayer / shiftTimers / consume*Events
//
// Fight flow (per boss):
//   DORMANT -> INTRO -> IDLE -> TELEGRAPH -> ATTACKING -> RECOVER -> IDLE ...
//                         \--> PHASE_TRANSITION (once, at <= phase2Threshold hp) --> IDLE (phase 2)
//   any state --(hp <= 0)--> DEAD
//
// Arena: the level's 'A' rect (or one screen centred on the boss). The fight starts when the player walks
// into it; from then until a moment after the kill isArenaLocked() is true and GameState pins the camera on the
// arena and keeps the player inside it.
//
// Music: the level's own track plays until the fight, the phase 1 track starts with the fight, the phase 2 track
// starts when the boss roars into phase 2, the track fades out on the kill and the level music returns a few
// seconds later (see consumeRestoreLevelMusic()). Files: Sound/Music/Boss/<biome>_<tier>_phase<1|2>.ogg
//   e.g. Sound/Music/Boss/moss_mini_phase1.ogg, peak_main_phase2.ogg. A boss can name its own files on the 'K' line.
//   A missing phase-2 file keeps phase 1 playing, a missing phase-1 file keeps the level music playing (no crash).
//
#pragma once
#include <vector>
#include <string>
#include <random>
#include <SDL3/SDL.h>
#include "level/Enemies.h"   // EnemyType, EnemyHitEvent, combatType, ENEMY_* tuning, Platform/LevelGround/Block

// ----------------------------------------------------------------------------------------------------
// tuning
// ----------------------------------------------------------------------------------------------------
const int   BOSS_MAX_ATTACKS            = 5;
const int   BOSS_MINI_KILL_SCORE        = 250;
const int   BOSS_MAIN_KILL_SCORE        = 1000;
const float BOSS_BEAM_DAMAGE_MUL        = 0.6f;    // the player's channelled beam is weaker against bosses
const float BOSS_RECOVER_DAMAGE_MUL     = 1.5f;    // "punish window": hits during RECOVER hurt more
const unsigned int BOSS_PHASE_TRANSITION_MS = 1900;
const unsigned int BOSS_DEATH_MUSIC_RESTORE_MS = 3200;
const float BOSS_PHASE2_SPEED_MUL       = 1.25f;   // movement + attack speeds
const float BOSS_PHASE2_WINDUP_MUL      = 0.8f;    // shorter tells
const float BOSS_PHASE2_RECOVER_MUL     = 0.7f;    // shorter openings
const float BOSS_PHASE2_IDLE_MUL        = 0.7f;    // less breathing room between attacks
const float BOSS_PROJECTILE_GRAVITY     = 1300.0f; // PROJECTILE_ARC curvature
const float BOSS_ROCK_WARN_SEC          = 0.75f;   // shadow marker time before a rock drops
const float BOSS_ROCK_GRAVITY           = 2200.0f;
const unsigned int BOSS_BEAM_REARM_MS   = 600;     // BREATH_BEAM can hurt again after this long
const float BOSS_ARENA_ENTER_MARGIN     = 200.0f;  // the fight starts once the player is this far inside the arena
const unsigned int BOSS_ARENA_UNLOCK_MS = 2200;    // the arena stays locked this long after the kill (death burst, score)
const unsigned int BOSS_MUSIC_FADE_MS   = 1200;    // boss track fades out over this long on the kill

// ----------------------------------------------------------------------------------------------------
// enums
// ----------------------------------------------------------------------------------------------------
enum class Biome {
    MOSS_CAVE = 0,
    STONE_CAVE,
    WOODEN_SHAFT,
    PLAINS,
    WOODS,
    MOUNTAIN,
    PEAK,
    COUNT
};

enum class BossTier { MINI = 0, MAIN };

// Names double as the level-file tokens: "K x y MOSS_BROODMOTHER" (see bossTypeFromName)
enum class BossType {
    // Moss cave
    MOSS_BROODMOTHER,        // mini  - giant spider
    MOSS_LUMINOUS_WYRM,      // main  - glow worm
    // Stone cave
    STONE_RAT_KING,          // mini
    STONE_THE_SINNER,        // main  - parent of the sinner's children
    // Wooden shaft mine
    SHAFT_FOREMAN_RAPTOR,    // mini
    SHAFT_BAT_MATRIARCH,     // main  - AERIAL
    // Plains
    PLAINS_ANKYLO_MATRIARCH, // mini
    PLAINS_SKY_TYRANT,       // main  - AERIAL eagle
    // Woods
    WOODS_ALPHA_CAT,         // mini
    WOODS_THEROPOD_KING,     // main
    // Mountain
    MOUNTAIN_SARCO_BRUTE,    // mini  - crocodylomorph
    MOUNTAIN_TARBO_TYRANT,   // main
    // Peak
    PEAK_DASPLETO_WARLORD,   // mini
    PEAK_REX_EMPEROR,        // main  - final boss
    NONE
};

enum class BossPhase { PHASE_1 = 0, PHASE_2 };

enum class BossState {
    DORMANT,           // waiting for the player to walk into the trigger radius
    INTRO,             // roar, invulnerable
    IDLE,              // walks / hovers toward the player until the next attack is due
    TELEGRAPH,         // wind-up: the attack outline is drawn so the player can react
    ATTACKING,         // hitbox / projectiles live
    RECOVER,           // opening: the boss takes extra damage
    PHASE_TRANSITION,  // roar into phase 2, invulnerable
    DEAD
};

enum class BossAttackKind {
    CLAW_SWIPE = 0,    // full-height box in front, small lunge
    BITE_SNAP,         // head-height box in front, lunge
    CHARGE,            // body-slam run across the arena; hitting a wall stuns the boss (longer RECOVER)
    LEAP_SLAM,         // ballistic leap onto the player; optional shockwaves on landing (count)
    SHOCKWAVE,         // stomp: `count` ground waves race out both ways - jump them
    ROCK_RAIN,         // `count` rocks fall from the ceiling, each shown by a warning marker first
    PROJECTILE_ARC,    // `count` lobbed projectiles aimed at the player (ballistic)
    PROJECTILE_FAN,    // `count` projectiles in a spread, aimed at the player
    BREATH_BEAM,       // long box from the mouth; in phase 2 it sweeps with the player
    AERIAL_DIVE,       // flier only: diagonal swoop onto the player
    TAIL_SWEEP,        // low box on BOTH sides - jump it
    SUMMON,            // calls `count` of the boss's minion enemy (needs the GameState hook)
    NONE
};

// ----------------------------------------------------------------------------------------------------
// data tables (see Bosses.cpp)
// ----------------------------------------------------------------------------------------------------
struct BossAttackDef {
    BossAttackKind kind;
    const char*    name;
    unsigned int   windupMs;      // TELEGRAPH length (outline drawn, nothing hurts yet)
    unsigned int   activeMs;      // ATTACKING length (LEAP_SLAM: safety cap, ends on landing)
    unsigned int   recoverMs;     // RECOVER length
    float          reach;         // box length / charge distance / beam length (px)
    float          speed;         // lunge, charge, projectile or dive speed (px/s)
    int            count;         // projectiles / rocks / waves / minions
    float          minGap;        // preferred horizontal gap to the player (px). 0 = no minimum
    float          maxGap;        // ...maximum. 0 = no maximum (ranged / anywhere)
    unsigned int   cooldownMs;    // before THIS attack can be picked again
    unsigned int   phaseMask;     // bit0 = phase 1, bit1 = phase 2
};

struct BossDef {
    BossType   type;
    Biome      biome;
    BossTier   tier;
    const char* name;
    int        hp;
    float      w, h;              // hitbox (player is 40x85)
    float      moveSpeed;         // px/s while closing in
    combatType combat;            // GROUND walks, AERIAL hovers
    EnemyType  minion;            // used by SUMMON
    float      triggerRadius;     // (unused since the arena lock: the fight starts when the player enters the arena)
    float      arenaHalfWidth;    // (unused: the arena is the level's 'A' rect, or one screen centred on the boss)
    unsigned int idleMs;          // gap between two attacks in phase 1
    float      phase2Threshold;   // fraction of max hp that starts phase 2
    float      musicGain;
    int        attackCount;       // 4 (mini) or 5 (main)
    BossAttackDef attacks[BOSS_MAX_ATTACKS];
};

const BossDef* getBossDef(BossType type);
BossType       bossTypeFromName(const std::string& name);                 // "PEAK_REX_EMPEROR" -> enum
std::string    getBossMusicPath(const BossDef& def, BossPhase phase);

// ----------------------------------------------------------------------------------------------------
// runtime
// ----------------------------------------------------------------------------------------------------
// Anything that hurts the player and isn't the boss's own attack box: projectiles, rocks, ground waves.
struct BossHazard {
    BossAttackKind source = BossAttackKind::NONE;
    float x = 0, y = 0, w = 30, h = 30;
    float vx = 0, vy = 0, gravity = 0;
    float age = 0, life = 4.0f;
    float warn = 0;               // seconds still to wait: marker only, harmless and motionless
    bool  dieOnTerrain = true;    // projectiles/rocks vanish on ground/platform/block; waves only on blocks/walls
    bool  hugsFloor = false;      // ground wave
};

struct BossSummonRequest {
    EnemyType  type;
    float      x, y;
    combatType combat;   // AERIAL for the flying bosses' minions (bats / eagles), GROUND otherwise
};

struct Boss {
    // x,y = top-left of the hitbox where it spawns (it falls onto the floor like ground enemies do)
    Boss(float x, float y, BossType type);

    BossType type = BossType::NONE;
    float x = 0, y = 0, w = 200, h = 200;
    float baseX = 0, baseY = 0;
    float arenaL = 0, arenaR = 0;     // walkable x range inside the arena (arena rect clipped to the level walls)
    bool  arenaReady = false;
    // arena rect (world space): the 'A' record sets it, else BossBuilder derives one screen-wide around the spawn
    bool      hasArena = false;
    SDL_FRect arena{0, 0, 0, 0};
    // music override from the 'K' line (empty = default Sound/Music/Boss/<biome>_<tier>_phase<n>.ogg)
    std::string musicPhase1, musicPhase2;
    float musicGain = -1.0f;          // < 0 = BossDef::musicGain
    combatType combat = combatType::GROUND;

    int hp = 500, maxHp = 500;
    BossPhase phase = BossPhase::PHASE_1;
    BossState state = BossState::DORMANT;
    bool isFacingRight = false;

    float velocityY = 0.0f;
    bool  isGrounded = false;

    // scheduling (SDL_GetTicks ms, see BossBuilder::shiftTimers)
    unsigned int stateStart = 0;
    unsigned int nextAttackTime = 0;
    unsigned int attackReadyAt[BOSS_MAX_ATTACKS] = {0, 0, 0, 0, 0};
    int attack = -1;              // index into def.attacks of the attack in progress
    int lastAttack = -1;
    int streak = 0;               // how many times in a row `lastAttack` was used
    int forcedAttack = -1;        // set when phase 2 starts: the first attack of phase 2 is always its signature move

    // per-attack runtime (reset by beginAttack)
    bool  launched = false;
    bool  endEarly = false;
    float dirX = 0, dirY = 0;
    float travelled = 0, budget = 0;
    unsigned int extraRecoverMs = 0;
    int   spawned = 0;
    unsigned int nextSpawnTick = 0;
    unsigned int nextRearm = 0;

    // attack hitbox (own body / box attacks only; projectiles live in BossBuilder::m_hazards)
    SDL_FRect attackHitBox{0, 0, 0, 0};
    bool isAttackBoxActive = false;
    bool attackHasHit = false;

    // damage taken
    bool isHurt = false;
    unsigned int hurtFlashEnd = 0;
    unsigned int whiteFlashEnd = 0;
    unsigned int nextBeamFx = 0;
    int   lastMeleeAttackId = -1;
    float beamAccum = 0.0f;

    unsigned int deathTime = 0;

    bool isDead() const { return state == BossState::DEAD; }
};

// what a HUD needs to draw a boss bar
struct BossHudInfo {
    const char* name = "";
    float hpFraction = 1.0f;
    BossPhase phase = BossPhase::PHASE_1;
    BossTier tier = BossTier::MINI;
    bool active = false;
};

class BossBuilder {
public:
    void init(const std::vector<Boss>& bosses);

    void update(float dt, const SDL_FRect& playerRect,
                const SDL_FRect& wallCollisionRect,
                const std::vector<LevelGround>& grounds,
                const std::vector<Platform>& platforms,
                const std::vector<Block>& blocks);

    void render(SDL_Renderer* renderer);            // call right after EnemiesBuilder::render
    void renderHud(SDL_Renderer* renderer) const;   // boss health bar (screen space), call with the HUD

    // same contract as EnemiesBuilder::applyPlayerDamage; returns how many bosses died this call
    // (award BOSS_MINI_KILL_SCORE / BOSS_MAIN_KILL_SCORE via consumeKillScore()).
    int applyPlayerDamage(const SDL_FRect& meleeHitBox, bool meleeActive,
                          const SDL_FRect& beamHitBox, bool beamActive,
                          float dt, int attackId, float knockbackMul = 1.0f);

    // same contract as EnemiesBuilder::checkAttackOnPlayer (boss boxes + every hazard)
    bool checkAttackOnPlayer(float playerX, float playerY, float playerW, float playerH,
                             unsigned int now, bool canDamage, gameMath::collisionSide& outSide);

    void shiftTimers(unsigned int ms);              // hit-stop compensation, same as EnemiesBuilder

    // ---- one-shot event queues (cleared on read) ----
    std::vector<EnemyHitEvent> consumeHitEvents(){ std::vector<EnemyHitEvent> o; o.swap(m_hitEvents); return o; }
    std::vector<SDL_FPoint> consumeDeathEvents(){ std::vector<SDL_FPoint> o; o.swap(m_deathEvents); return o; }
    std::vector<BossSummonRequest> consumeSummons(){ std::vector<BossSummonRequest> o; o.swap(m_summons); return o; }
    int  consumeKillScore(){ int s = m_killScore; m_killScore = 0; return s; }
    // true once, a few seconds after the last fight ended: GameState should play the level music again
    bool consumeRestoreLevelMusic(){ bool r = m_restoreReady; m_restoreReady = false; return r; }

    bool isFightActive() const { return m_activeIdx >= 0; }

    // True from the moment the fight starts until BOSS_ARENA_UNLOCK_MS after the kill: GameState pins the camera
    // on getArena() and keeps the player inside it for exactly this long.
    bool isArenaLocked() const;
    // The arena (full rect, world space) of the fight in progress / just finished. False before the first fight.
    bool getArena(SDL_FRect& out) const;
    // any boss in the level still alive? (a level's exit stays shut until this is false)
    bool hasLivingBoss() const;
    // Player died mid-fight: every unfinished boss goes back to full hp and sleeping, hazards are cleared, the arena
    // unlocks and the level music is queued to return (consumeRestoreLevelMusic()). Bosses already dead stay dead.
    // Returns true if a fight was actually in progress.
    bool resetFight();
    BossHudInfo getHudInfo() const;
    const std::vector<Boss>& getBosses() const { return m_bosses; }
    const std::vector<BossHazard>& getHazards() const { return m_hazards; }

private:
    // --- state machine ---
    void updateBoss(Boss& b, float dt, const SDL_FRect& player, const SDL_FRect& wall,
                    const std::vector<LevelGround>& grounds, const std::vector<Platform>& platforms,
                    const std::vector<Block>& blocks, unsigned int now);
    void beginFight(int idx, unsigned int now);
    void beginPhaseTransition(Boss& b, unsigned int now);
    void setupArena(Boss& b, const SDL_FRect& wall);
    void playBossMusic(const Boss& b, const BossDef& def, BossPhase phase);
    void startDeath(Boss& b, unsigned int now);
    void setState(Boss& b, BossState s, unsigned int now);
    int  pickAttack(Boss& b, const BossDef& def, const SDL_FRect& player, unsigned int now);
    void beginAttack(Boss& b, unsigned int now);
    void runAttack(Boss& b, const BossDef& def, const BossAttackDef& a, float dt, const SDL_FRect& player,
                   const SDL_FRect& wall, const std::vector<Block>& blocks, unsigned int now, unsigned int elapsed);
    SDL_FRect previewBox(const Boss& b, const BossAttackDef& a, const SDL_FRect& player, const SDL_FRect& wall) const;

    // --- movement / physics ---
    bool tryMoveX(Boss& b, float nextX, const SDL_FRect& wall, const std::vector<Block>& blocks) const;
    void hoverToward(Boss& b, float targetX, float targetY, float speed, float dt, const SDL_FRect& wall) const;
    void applyGravityAndCollision(Boss& b, float dt, const SDL_FRect& wall,
                                  const std::vector<LevelGround>& grounds,
                                  const std::vector<Platform>& platforms,
                                  const std::vector<Block>& blocks);
    void faceToward(Boss& b, float targetCenterX) const;

    // --- hazards ---
    void spawnArcProjectile(const Boss& b, const BossAttackDef& a, const SDL_FRect& player);
    void spawnFan(const Boss& b, const BossAttackDef& a, const SDL_FRect& player, int count);
    void spawnShockwavePair(const Boss& b, const BossAttackDef& a);
    void spawnRock(const Boss& b, float x, const SDL_FRect& wall);
    void updateHazards(float dt, const SDL_FRect& wall, const std::vector<LevelGround>& grounds,
                       const std::vector<Platform>& platforms, const std::vector<Block>& blocks);

    // --- placeholder rendering (no boss art yet; same convention as EnemiesBuilder) ---
    void renderBoss(SDL_Renderer* r, const Boss& b, int camX, int camY) const;
    void renderAttackPlaceholders(SDL_Renderer* r, int camX, int camY) const;

    float phaseSpeedMul(const Boss& b) const { return b.phase == BossPhase::PHASE_2 ? BOSS_PHASE2_SPEED_MUL : 1.0f; }

private:
    std::vector<Boss> m_bosses;
    std::vector<Boss> m_template;   // the bosses exactly as the level defined them (resetFight restores from it)
    std::vector<BossHazard> m_hazards;
    std::vector<EnemyHitEvent> m_hitEvents;
    std::vector<SDL_FPoint> m_deathEvents;
    std::vector<BossSummonRequest> m_summons;
    float m_floorY = 0.0f;          // bottom of the arena (set every update, used by the rock warning columns)
    int  m_activeIdx = -1;          // the boss currently being fought (-1 = none)
    int  m_killScore = 0;
    bool m_restorePending = false;
    bool m_restoreReady = false;
    unsigned int m_restoreAt = 0;
    SDL_FRect m_arena{0, 0, 0, 0};  // arena of the active / last fight
    float m_arenaInnerL = 0.0f, m_arenaInnerR = 0.0f;   // its walkable x range (hazards die outside it)
    bool  m_arenaValid = false;
    unsigned int m_unlockAt = 0;    // arena stays locked until this tick after the kill
    std::mt19937 m_rng{12345u};
};
