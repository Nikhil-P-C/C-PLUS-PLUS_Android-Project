//
// Bosses.h - 13 bosses: 6 biomes x (mini + main) + the Preacher (no mini at the Peak).
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
#include <memory>
#include "States/MenuState/MenuKit.h"   // title cards reuse the menu pixel font
#include "level/Enemies.h"   // EnemyType, EnemyHitEvent, combatType, ENEMY_* tuning, Platform/LevelGround/Block

// ----------------------------------------------------------------------------------------------------
// tuning
// ----------------------------------------------------------------------------------------------------
const int   BOSS_MAX_ATTACKS            = 12;
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
const unsigned int BOSS_PARRY_PRE_MS    = 260;     // a slash this long BEFORE a parryable strike lands counts as a parry
const unsigned int BOSS_PARRY_POST_MS   = 110;     // ...and this long after
const unsigned int BOSS_GRAB_MS         = 1100;    // how long a grab holds the player before the final bite
const int          BOSS_GRAB_BREAK_HITS = 2;       // hit the grabber this many times to break free
const int          BOSS_QTE_STEPS       = 5;       // samurai dashes the player must parry to finish the Ancient Sinner
const unsigned int BOSS_QTE_TRAVEL_MS   = 170;     // the dash flies in from off-screen this long before it reaches the player (= the parry moment)
const unsigned int BOSS_QTE_FOLLOW_MS   = 90;      // ...and carries on through the player this long after
const float        BOSS_QTE_HOVER_ABOVE_FLOOR = 250.0f;   // the QTE keeps the player pushed up this high above the floor
const float BOSS_MIN_ARENA_H           = 704.0f;  // arenas are never shorter than this (one full screen): QTE lift, tall bosses, jumps need the room
const float BOSS_PREACHER_UPPER_RISE    = 3840.0f; // phase 3: the upper arena is the fight arena moved this far UP (level_23 has its ground there)
const float BOSS_CLIMB_CHILD_SPEED      = 140.0f;  // px/s the sinner children fly upward
const float BOSS_BOUNCE_VELOCITY        = 1150.0f; // ghost pads (Preacher phase 3) throw the player up this hard

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
    MOSS_LUMINOUS_WYRM,      // mini  - burrows, pops its head out (the opening), spits acid both ways
    MOSS_CAVE_SPIDER,        // main  - webs, wall/ceiling crawling, stomp, acid; phase 2 faster
    // Stone cave
    STONE_GIANT_RAT,         // mini  - faster than the player: dash, grab + bite
    STONE_ANCIENT_SINNER,    // main  - spinosaur 3x player height; phase 2 = First Berserker; parry QTE finisher
    // Wooden shaft mine
    SHAFT_WARDEN,            // mini  - slow fat raptor: slam / stomp / jump by where the player is
    SHAFT_BRONTOS,           // main  - three necks, only the heads take damage; phase 2 = two attacks at once
    // Plains
    PLAINS_ANKYLOSAUR,       // mini  - slow armoured: club / stomp / jump by where the player is
    PLAINS_SENTINEL_PARA,    // main  - fast parasaur: slashes, dash, beam, teleport pounce; PARRYABLE
    // Woods
    WOODS_THORN_MATRIARCH,   // mini  - rooted carnivorous plant: eruptions, spore clouds, vine lash
    WOODS_TREEHORN_ELDER,    // main  - overgrown horned beast: charge, root eruptions, log rolls, seeds
    // Mountain
    MOUNTAIN_CAT_QUEEN,      // mini  - sword claws, high pounce, reactive dodge-dash, bite
    MOUNTAIN_BROTHER,        // main  - the preacher's brother: tail whip, flame breath, jump stomp; phase 2 faster + tankier
    // Peak
    PEAK_THE_PREACHER,       // main  - final boss, 3 phases (see Bosses.cpp)
    NONE
};

// PHASE_3 only exists for the Preacher (after his false death)
enum class BossPhase { PHASE_1 = 0, PHASE_2, PHASE_3 };

// special per-boss logic that the generic attack table can't express
enum class BossBehavior { NONE = 0, BRONTOS, PREACHER };

enum class BossState {
    DORMANT,           // waiting for the player to walk into the trigger radius
    INTRO,             // roar, invulnerable
    IDLE,              // walks / hovers toward the player until the next attack is due
    TELEGRAPH,         // wind-up: the attack outline is drawn so the player can react
    ATTACKING,         // hitbox / projectiles live
    RECOVER,           // opening: the boss takes extra damage
    PHASE_TRANSITION,  // roar into phase 2, invulnerable
    QTE,               // Ancient Sinner finisher: parry his strikes (see BOSS_QTE_STEPS)
    FALSE_DEATH,       // Preacher: "dies" at the end of phase 2
    REVIVE,            // Preacher: god ray, winged rebirth, flees up (phase 3 starts after it)
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
    SPIT_BOTH,         // `count` acid balls lobbed in arcs to BOTH sides
    GRAB_BITE,         // box in front; on hit the player is held, hit the boss twice to break free or take the final bite
    BEAM_DIAG,         // diagonal beam from the mouth through the player's position (phase 2 / berserk: thicker, tracks)
    TELEPORT_POUNCE,   // vanish, marker on the player's spot, reappear above it and drop (count = repeats)
    FLAMETHROWER,      // like BREATH_BEAM, low from the mouth, rearms faster
    ERUPT,             // `count` ground columns with a floor marker (aux 1 = a line from the boss toward the player)
    SPORE_CLOUD,       // `count` lingering gas clouds on the floor (damage over time while inside)
    BURROW_AMBUSH,     // dive, travel under the floor, erupt under the player, head exposed (takes extra damage) + acid, dive
    SURFACE_MOVE,      // spider: climb to the ceiling (aux 1) or a wall (aux 2)
    DODGE_DASH,        // reactive: dash away from a player swing, i-frames, then an instant counter bite
    ORB_SUMMON,        // `count` magical orbs appear in the air, hover, then fly at the player (speed)
    SMITE,             // `count` full-height light columns with a warning
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
    unsigned int   phaseMask;     // bit0 = phase 1, bit1 = phase 2, bit2 = phase 3
    unsigned int   cond = 0;      // BOSS_C_* bits: where the player / the boss itself must be for this move to be picked
    bool           parry = false; // the player can parry it (slash inside the window right before it lands)
    float          aux = 0.0f;    // kind specific (see the kinds above)
};

// Every slash (claw swipe / bite snap) can be parried; other attacks only when their table entry says `parry`.
inline bool bossAttackParryable(const BossAttackDef& a){
    return a.parry || a.kind == BossAttackKind::CLAW_SWIPE || a.kind == BossAttackKind::BITE_SNAP;
}

// BossAttackDef::cond
const unsigned int BOSS_C_PLAYER_GROUND = 1;    // player standing on something
const unsigned int BOSS_C_PLAYER_AIR    = 2;    // player in the air
const unsigned int BOSS_C_ON_FLOOR      = 4;    // boss (spider) on the floor...
const unsigned int BOSS_C_ON_CEILING    = 8;    // ...on the ceiling...
const unsigned int BOSS_C_ON_WALL       = 16;   // ...on a wall. Any of these three set = boss must be on one of them
const unsigned int BOSS_C_REACTIVE      = 32;   // never picked by the scheduler (DODGE_DASH)

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
    int        attackCount;
    BossAttackDef attacks[BOSS_MAX_ATTACKS];
    BossBehavior behavior = BossBehavior::NONE;
    float      p2SpeedMul = 1.25f;      // phase 2 speed (BOSS_PHASE2_SPEED_MUL by default)
    float      p2DamageMul = 1.0f;      // < 1 = tankier in phase 2 (Brother)
    bool       finisherQte = false;     // hp 0 in phase 2 -> parry QTE, success kills (Ancient Sinner)
    bool       revive = false;          // hp 0 in phase 2 -> false death + winged phase 3 (Preacher)
    int        hpPhase3 = 0;            // hp of the phase 3 bar
    float      chainP2 = 0.0f;          // phase 2: chance to start another attack with no pause ("more random")
};

// Title cards: every boss gets a name card at the start of the fight and a card for each phase.
// THREAT = normal boss, CHALLENGE = the three big ones (Ancient Sinner, Sentinel Para, The Preacher).
enum class BossRank { THREAT = 0, CHALLENGE };
struct BossTitleInfo {
    BossRank    rank;
    const char* phaseName[3];     // [0] = shown with the name card, [1] = phase 2 card, [2] = phase 3 card (nullptr = no card)
};
BossTitleInfo getBossTitleInfo(BossType type);

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
    bool  persistent = false;     // spore cloud: not spent on contact, ticks damage instead
    unsigned int nextHit = 0;
    float hoverT = 0;             // orb: seconds it hangs in the air before it launches
    float launchSpeed = 0;        // orb launch speed
};

// one head of the Brontos (the necks are drawn from the anchor to the head and are NOT damageable)
struct BossPart {
    SDL_FRect rect{0, 0, 120, 96};
    float homeX = 0, homeY = 0, anchorX = 0, anchorY = 0;
    int   hp = 0, maxHp = 0;
    int   state = 0;              // 0 idle, 1 telegraph, 2 strike, 3 stuck, 4 retract, 5 dead
    int   kind = 0;               // 0 slam, 1 sweep, 2 grab
    unsigned int t0 = 0;
    float tx = 0, ty = 0, dirX = 0;
    SDL_FRect hit{0, 0, 0, 0};
    bool  hitActive = false, hasHit = false;
    unsigned int hurtFlashEnd = 0;
    int   lastMeleeId = -1;
    float beamAccum = 0.0f;
    float swayPhase = 0.0f;
};

struct BossParryEvent { float x, y; bool big; };

// what the boss logic needs to know about the player each frame (GameState fills it in)
struct BossPlayerInfo {
    bool slashStarted = false;    // a melee swing began this frame
    bool attacking = false;       // a swing is in progress
    bool grounded = true;
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
    float baseW = 0, baseH = 0;       // the spider swaps w/h on walls; these keep the floor dimensions
    float arenaL = 0, arenaR = 0;     // walkable x range inside the arena (arena rect clipped to the level walls)
    bool  arenaReady = false;
    // arena rect (world space): the 'A' record sets it, else BossBuilder derives one screen-wide around the spawn
    bool      hasArena = false;
    SDL_FRect arena{0, 0, 0, 0};
    // music override from the 'K' line (empty = default Sound/Music/Boss/<biome>_<tier>_phase<n>.ogg)
    std::string musicPhase1, musicPhase2, musicPhase3;
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
    unsigned int attackReadyAt[BOSS_MAX_ATTACKS] = {};
    int attack = -1;              // index into def.attacks of the attack in progress
    int lastAttack = -1;
    int streak = 0;               // how many times in a row `lastAttack` was used
    int forcedAttack = -1;        // set when phase 2 starts: the first attack of phase 2 is always its signature move

    // special state
    enum class Surface { FLOOR, CEILING, WALL } surface = Surface::FLOOR;   // spider
    bool hidden = false;          // burrowed / teleporting: not drawn, untargetable
    bool untargetable = false;
    bool noPhysics = false;       // gravity off (burrowed, on a wall/ceiling, teleporting)
    bool exposed = false;         // Wyrm head out: takes extra damage
    unsigned int iframesUntil = 0;
    int   stage = 0;              // sub-state of the current special attack
    unsigned int stageT = 0;
    float markerX = 0;            // teleport / burrow landing marker (world x)
    bool  markerOn = false;
    float wpX[3] = {0,0,0}, wpY[3] = {0,0,0};
    Surface wpSurf[3] = {Surface::FLOOR, Surface::FLOOR, Surface::FLOOR};
    int   wpCount = 0, wpIdx = 0;
    // diagonal beam
    float beamX1 = 0, beamY1 = 0, beamX2 = 0, beamY2 = 0, beamThick = 40.0f;
    bool  beamAim = false, beamOn = false;
    // reactive dodge
    bool  afterDodge = false;
    // parry / stagger
    unsigned int staggerUntil = 0;
    // QTE (Ancient Sinner)
    int   qteStep = 0;
    unsigned int qteStrike = 0;
    bool  qteDone = false;
    float qteFromX = 0, qteToX = 0;   // current samurai dash (x of the hitbox)
    float qteAimY = 0;                // height (centre) the dash flies along = the star glint's height
    bool  qteFromLeft = true;         // the dash comes in from the left edge (else the right edge)
    // Brontos
    std::vector<BossPart> parts;
    // Preacher phase 3
    bool  winged = false;

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
    // the part of the boss that can be damaged (Preacher phase 3: the body only, never the wings)
    SDL_FRect hurtRect() const {
        if(winged) return { x + w * 0.3f, y + h * 0.05f, w * 0.4f, h * 0.9f };
        return { x, y, w, h };
    }
};

// what a HUD needs to draw a boss bar
struct BossHudInfo {
    const char* name = "";
    float hpFraction = 1.0f;
    BossPhase phase = BossPhase::PHASE_1;
    BossTier tier = BossTier::MINI;
    bool active = false;
    bool hasPhase2 = false;     // draw the 50% tick
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

    // ---- player interaction (call every frame before update) ----
    void setPlayerInfo(const BossPlayerInfo& info);
    std::vector<BossParryEvent> consumeParryEvents(){ std::vector<BossParryEvent> o; o.swap(m_parryEvents); return o; }
    // true while a boss/head holds the player: put the player at `pos` (top-left), zero his velocity
    bool isPlayerGrabbed(SDL_FPoint& pos) const;
    // Ancient Sinner QTE: the player is pushed upward and held in the air (arena must be tall enough for it).
    // Returns true while that applies; outVelocityY is the vertical speed to force on the player this frame.
    bool getQteLift(const SDL_FRect& player, float& outVelocityY) const;
    // Preacher phase 3: ghost pads the player can bounce on. Returns true (and the new upward speed) when
    // the player (falling, feet crossing a pad's top) should be bounced.
    bool checkBounce(const SDL_FRect& player, float velocityY, float& outVelocityY);

    // True from the moment the fight starts until BOSS_ARENA_UNLOCK_MS after the kill: GameState pins the camera
    // on getArena() and keeps the player inside it for exactly this long.
    bool isArenaLocked() const;
    // Preacher phase 3: while the player climbs the ghost pads toward the upper arena the camera follows him inside
    // this tall column instead of being pinned. True only during the climb.
    bool getClimbBounds(SDL_FRect& out) const;
    // true from the Preacher's rise until the fight is reset (the camera needs the whole tall level then)
    bool isUpperActive() const { return m_upper; }
    // The arena (full rect, world space) of the fight in progress / just finished. False before the first fight.
    bool getArena(SDL_FRect& out) const;
    // any boss in the level still alive? (a level's exit stays shut until this is false)
    bool hasLivingBoss() const;
    // Player died mid-fight: every unfinished boss goes back to full hp and sleeping, hazards are cleared, the arena
    // unlocks and the level music is queued to return (consumeRestoreLevelMusic()). Bosses already dead stay dead.
    // Returns true if a fight was actually in progress.
    bool resetFight();
    BossHudInfo getHudInfo() const;
    BossRank getRank() const;
    void showTitleCard(const Boss& b, int phaseIdx, bool nameCard);
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
    // --- specials ---
    void updateBrontos(Boss& b, const BossDef& def, float dt, const SDL_FRect& player, const SDL_FRect& wall, unsigned int now);
    void startPartAttack(Boss& b, BossPart& p, int kind, const SDL_FRect& player, unsigned int now);
    void updatePart(Boss& b, BossPart& p, float dt, const SDL_FRect& player, const SDL_FRect& wall, unsigned int now);
    void beginQte(Boss& b, unsigned int now);
    void updateQte(Boss& b, const SDL_FRect& player, unsigned int now);
    void beginFalseDeath(Boss& b, unsigned int now);
    void updateRevive(Boss& b, const SDL_FRect& wall, unsigned int now);
    void startPhase3(Boss& b, const SDL_FRect& wall, unsigned int now);
    void spawnClimbGhosts();
    bool tryParry(Boss& b, const BossAttackDef& a, const SDL_FRect& player, unsigned int impactAt, unsigned int now);
    void doParry(Boss& b, unsigned int now);
    void setSurface(Boss& b, Boss::Surface s);
    void buildSurfacePath(Boss& b, int target, const SDL_FRect& player, const SDL_FRect& wall);
    void startGrab(int bossIdx, int part, unsigned int now);
    void endGrab();
    void spawnSpit(const Boss& b, int side, const BossAttackDef& a);
    void spawnErupt(float cx, float width, float height, float warn, float lifeAfter);
    void spawnWavesAt(float cx, float halfW, float feetY, float speed, float height, float life);
    void spawnOrb(float x, float y, float launchSpeed, float hover);
    void spawnSmite(float cx, const SDL_FRect& wall);
    void spawnSpore(float cx);
    void renderExtras(SDL_Renderer* r, int camX, int camY) const;
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

    float phaseSpeedMul(const Boss& b) const;
    void endAttackCleanup(Boss& b, const SDL_FRect& wall);
    void aimBeam(Boss& b, const SDL_FRect& player, const BossAttackDef& a);

private:
    std::vector<Boss> m_bosses;
    std::vector<Boss> m_template;   // the bosses exactly as the level defined them (resetFight restores from it)
    std::vector<BossHazard> m_hazards;
    std::vector<EnemyHitEvent> m_hitEvents;
    std::vector<BossParryEvent> m_parryEvents;
    // Preacher phase 3: sinner children with broken wings fly UP toward the Preacher. Landing on one (never a side
    // collision, they don't hurt) is a pogo bounce; they are the only way up. They loop: when one passes the top it
    // starts again from the bottom.
    struct ClimbChild { float x, y, w, h; float phase; bool faceRight; };
    std::vector<ClimbChild> m_ghosts;
    float m_ghostCycle = 0.0f, m_ghostTop = 0.0f;
    int   m_ghostWraps = 0;
    BossPlayerInfo m_info;
    SDL_FRect m_playerRect{0, 0, 40, 85};
    unsigned int m_slashTick = 0, m_parryUsed = 0;
    struct Grab { bool active = false; int boss = -1; int part = -1; unsigned int start = 0; int struggle = 0; } m_grab;
    bool m_pendingBite = false;
    unsigned int m_pendingBiteUntil = 0;
    float m_ceilY = 0.0f;
    bool  m_upper = false;          // Preacher: risen to the upper arena (phase 3)
    bool  m_upperReached = false;   // ...and the player is up there too: the arena lock moves up
    float m_lowerFloorY = 0.0f;
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
    // title card (see showTitleCard / renderTitleCard)
    struct TitleCard { bool on = false; unsigned int start = 0, dur = 0; bool nameCard = true; BossRank rank = BossRank::THREAT;
        std::string main, sub; } m_card;
    mutable std::unique_ptr<MenuFont> m_cardFont;
    mutable MenuLabel m_cardMain, m_cardTag, m_cardSub;
    mutable std::string m_cardMainKey, m_cardTagKey, m_cardSubKey;
    void renderTitleCard(SDL_Renderer* renderer) const;
};
