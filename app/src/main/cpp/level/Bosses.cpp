//
// Bosses.cpp - see Bosses.h for the overview.
//
#include "Bosses.h"
#include "States/GameState/GameState.h"   // SCALE, TILE_SIZE (same include Enemies.cpp uses)
#include "utils/Camera.h"
#include <algorithm>
#include <cmath>

// ====================================================================================================
//  DATA: every boss. Tweak numbers here, nothing else needs to change.
//
//  BossAttackDef = { kind, name, windupMs, activeMs, recoverMs, reach, speed, count, minGap, maxGap,
//                    cooldownMs, phaseMask }
//  Rules used for every boss:
//   - minis have 4 attacks, mains have 5
//   - the LAST attack of every boss is phase-2 only (the "enraged" move)
//   - phase 2 also: +25% speed, -20% wind-up, -30% recover/idle, +2 projectiles/rocks, +1 shockwave
//   - gap = horizontal distance between boss box and player box. minGap/maxGap bias the pick
//     (inside the window = 3x as likely, outside = 0.4x); 0 = no limit
// ====================================================================================================
namespace {
constexpr unsigned int P1 = 1, P2 = 2, BOTH = 3;
using K = BossAttackKind;

const BossDef kDefs[] = {
    // ------------------------------------------------------------------ MOSS CAVE
    { BossType::MOSS_BROODMOTHER, Biome::MOSS_CAVE, BossTier::MINI, "Moss Brood Mother",
      300, 150, 120, 170, combatType::GROUND, EnemyType::SPIDER_CHILD, 700, 900, 900, 0.5f, 1.0f, 4, {
        { K::BITE_SNAP,      "Venom Bite",       350, 200, 550, 150, 220, 0, 0, 200, 1800, BOTH },
        { K::PROJECTILE_ARC, "Acid Spit",        500, 600, 700,   0, 650, 2, 150, 0, 2500, BOTH },
        { K::SUMMON,         "Hatch Brood",      800, 300, 1000,  0,   0, 2, 0, 0, 14000, BOTH },
        { K::PROJECTILE_FAN, "Web Volley",       650, 300, 900,   0, 540, 5, 0, 0, 3500, P2 },
    }},
    { BossType::MOSS_LUMINOUS_WYRM, Biome::MOSS_CAVE, BossTier::MAIN, "Luminous Wyrm",
      700, 260, 200, 130, combatType::GROUND, EnemyType::MOSS_GLOW_WORM, 900, 1100, 1000, 0.5f, 1.0f, 5, {
        { K::BITE_SNAP,      "Maw Snap",         450, 220, 650, 230, 260, 0, 0, 260, 1500, BOTH },
        { K::TAIL_SWEEP,     "Coil Sweep",       600, 450, 800, 260,   0, 0, 0, 300, 3000, BOTH },
        { K::ROCK_RAIN,      "Cavern Collapse",  900, 1400, 900,  0,   0, 5, 0, 0, 7000, BOTH },
        { K::BREATH_BEAM,    "Glow Beam",        800, 1500, 1000, 700, 0, 0, 200, 0, 6000, BOTH },
        { K::PROJECTILE_FAN, "Spore Burst",      700, 300, 900,   0, 480, 7, 0, 0, 4500, P2 },
    }},
    // ------------------------------------------------------------------ STONE CAVE
    { BossType::STONE_RAT_KING, Biome::STONE_CAVE, BossTier::MINI, "Rat King",
      320, 140, 110, 230, combatType::GROUND, EnemyType::CAVE_RAT, 700, 900, 800, 0.5f, 1.0f, 4, {
        { K::CLAW_SWIPE,     "Claw Rake",        350, 250, 500, 150, 140, 0, 0, 180, 1500, BOTH },
        { K::CHARGE,         "Gnaw Rush",        600, 900, 800, 900, 640, 0, 250, 0, 3500, BOTH },
        { K::SUMMON,         "Call the Swarm",   800, 300, 1000,  0,   0, 3, 0, 0, 15000, BOTH },
        { K::LEAP_SLAM,      "Crushing Pounce",  650, 2000, 900,  0, 700, 2, 200, 0, 4000, P2 },
    }},
    { BossType::STONE_THE_SINNER, Biome::STONE_CAVE, BossTier::MAIN, "The Sinner",
      750, 190, 230, 150, combatType::GROUND, EnemyType::SINNERS_CHILD, 900, 1100, 1000, 0.5f, 1.0f, 5, {
        { K::CLAW_SWIPE,     "Penance Slash",    500, 280, 650, 240, 160, 0, 0, 260, 1500, BOTH },
        { K::PROJECTILE_ARC, "Hurled Stone",     650, 800, 800,   0, 620, 3, 250, 0, 3000, BOTH },
        { K::SHOCKWAVE,      "Judgment Stomp",   700, 1100, 900,  0, 520, 1, 0, 0, 4000, BOTH },
        { K::ROCK_RAIN,      "Falling Sins",     900, 1500, 900,  0,   0, 6, 0, 0, 7500, BOTH },
        { K::SUMMON,         "Call the Children",900, 300, 1100,  0,   0, 2, 0, 0, 16000, P2 },
    }},
    // ------------------------------------------------------------------ WOODEN SHAFT MINE
    { BossType::SHAFT_FOREMAN_RAPTOR, Biome::WOODEN_SHAFT, BossTier::MINI, "Foreman Raptor",
      340, 130, 150, 250, combatType::GROUND, EnemyType::SHAFTS_RAPTOR, 700, 900, 800, 0.5f, 1.0f, 4, {
        { K::CLAW_SWIPE,     "Pick Swing",       380, 260, 520, 170, 200, 0, 0, 200, 1400, BOTH },
        { K::CHARGE,         "Cart Rush",        600, 1000, 850, 1000, 700, 0, 300, 0, 3500, BOTH },
        { K::PROJECTILE_ARC, "Thrown Pickaxe",   500, 500, 750,   0, 720, 1, 200, 0, 2200, BOTH },
        { K::LEAP_SLAM,      "Dynamite Leap",    700, 2000, 900,  0, 760, 2, 200, 0, 4500, P2 },
    }},
    { BossType::SHAFT_BAT_MATRIARCH, Biome::WOODEN_SHAFT, BossTier::MAIN, "Bat Matriarch",
      680, 230, 150, 260, combatType::AERIAL, EnemyType::GIANT_BATS, 900, 1100, 900, 0.5f, 1.0f, 5, {
        { K::AERIAL_DIVE,    "Screech Dive",     750, 1100, 900,  0, 820, 0, 0, 0, 3000, BOTH },
        { K::PROJECTILE_FAN, "Sonic Fan",        600, 300, 850,   0, 560, 5, 0, 0, 3200, BOTH },
        { K::ROCK_RAIN,      "Tremor Cave-In",   900, 1400, 900,  0,   0, 5, 0, 0, 7000, BOTH },
        { K::BITE_SNAP,      "Fang Rend",        400, 220, 600, 190, 300, 0, 0, 230, 1500, BOTH },
        { K::SUMMON,         "Call the Roost",   900, 300, 1100,  0,   0, 2, 0, 0, 16000, P2 },
    }},
    // ------------------------------------------------------------------ PLAINS
    { BossType::PLAINS_ANKYLO_MATRIARCH, Biome::PLAINS, BossTier::MINI, "Ankylosaur Matriarch",
      380, 220, 130, 120, combatType::GROUND, EnemyType::ANKYLOSAURUS_CHILD, 700, 900, 1000, 0.5f, 1.0f, 4, {
        { K::TAIL_SWEEP,     "Club Sweep",       600, 450, 800, 230,   0, 0, 0, 260, 2000, BOTH },
        { K::CHARGE,         "Armored Charge",   800, 1100, 1000, 1000, 520, 0, 300, 0, 4000, BOTH },
        { K::SHOCKWAVE,      "Ground Pound",     700, 900, 900,   0, 480, 1, 0, 0, 3500, BOTH },
        { K::PROJECTILE_ARC, "Boulder Fling",    650, 700, 800,   0, 600, 2, 200, 0, 3000, P2 },
    }},
    { BossType::PLAINS_SKY_TYRANT, Biome::PLAINS, BossTier::MAIN, "Sky Tyrant",
      720, 210, 130, 280, combatType::AERIAL, EnemyType::HUNTER_EAGLE, 900, 1100, 900, 0.5f, 1.0f, 5, {
        { K::AERIAL_DIVE,    "Talon Dive",       700, 1100, 850,  0, 880, 0, 0, 0, 2800, BOTH },
        { K::PROJECTILE_FAN, "Feather Storm",    650, 300, 900,   0, 600, 6, 0, 0, 3200, BOTH },
        { K::BITE_SNAP,      "Beak Strike",      380, 220, 550, 170, 320, 0, 0, 210, 1400, BOTH },
        { K::ROCK_RAIN,      "Feather Rain",     900, 1500, 900,  0,   0, 6, 0, 0, 7000, BOTH },
        { K::BREATH_BEAM,    "Cyclone Blast",    900, 1600, 1100, 640, 0, 0, 150, 0, 6500, P2 },
    }},
    // ------------------------------------------------------------------ WOODS
    { BossType::WOODS_ALPHA_CAT, Biome::WOODS, BossTier::MINI, "Alpha Predator Cat",
      330, 150, 100, 280, combatType::GROUND, EnemyType::PREDATOR_CAT, 700, 900, 750, 0.5f, 1.0f, 4, {
        { K::CLAW_SWIPE,     "Rend",             300, 220, 450, 150, 180, 0, 0, 170, 1300, BOTH },
        { K::CHARGE,         "Stalking Rush",    550, 800, 700, 800, 720, 0, 250, 0, 3000, BOTH },
        { K::LEAP_SLAM,      "Pounce",           600, 2000, 700,  0, 780, 0, 200, 0, 3200, BOTH },
        { K::BITE_SNAP,      "Frenzy Bite",      250, 200, 400, 130, 320, 0, 0, 160, 1200, P2 },
    }},
    { BossType::WOODS_THEROPOD_KING, Biome::WOODS, BossTier::MAIN, "Theropod King",
      780, 240, 250, 170, combatType::GROUND, EnemyType::THEROPOD_CHILD, 900, 1100, 1000, 0.5f, 1.0f, 5, {
        { K::BITE_SNAP,      "Crushing Bite",    500, 250, 700, 250, 260, 0, 0, 270, 1500, BOTH },
        { K::TAIL_SWEEP,     "Tail Whip",        650, 450, 850, 280,   0, 0, 0, 300, 3000, BOTH },
        { K::CHARGE,         "Rampage",          800, 1200, 1100, 1100, 640, 0, 350, 0, 4500, BOTH },
        { K::PROJECTILE_ARC, "Tree Hurl",        700, 900, 900,   0, 640, 3, 300, 0, 3500, BOTH },
        { K::ROCK_RAIN,      "Canopy Crash",     900, 1600, 1000, 0,   0, 7, 0, 0, 8000, P2 },
    }},
    // ------------------------------------------------------------------ MOUNTAIN
    { BossType::MOUNTAIN_SARCO_BRUTE, Biome::MOUNTAIN, BossTier::MINI, "Sarco Brute",
      360, 230, 110, 190, combatType::GROUND, EnemyType::CROCODYLOMORPH, 700, 900, 900, 0.5f, 1.0f, 4, {
        { K::BITE_SNAP,      "Death Roll Bite",  420, 260, 600, 240, 240, 0, 0, 260, 1500, BOTH },
        { K::TAIL_SWEEP,     "Tail Slam",        550, 420, 750, 240,   0, 0, 0, 270, 2400, BOTH },
        { K::LEAP_SLAM,      "Lunge",            650, 2000, 800,  0, 720, 0, 220, 0, 3500, BOTH },
        { K::SHOCKWAVE,      "Thrash",           650, 1200, 900,  0, 500, 2, 0, 0, 4500, P2 },
    }},
    { BossType::MOUNTAIN_TARBO_TYRANT, Biome::MOUNTAIN, BossTier::MAIN, "Tarbo Tyrant",
      820, 250, 260, 170, combatType::GROUND, EnemyType::TARBOSAURUS_CHILD, 900, 1100, 1000, 0.5f, 1.0f, 5, {
        { K::BITE_SNAP,      "Jaw Clamp",        480, 240, 700, 260, 280, 0, 0, 280, 1500, BOTH },
        { K::CHARGE,         "Avalanche Rush",   800, 1200, 1000, 1100, 680, 0, 350, 0, 4500, BOTH },
        { K::ROCK_RAIN,      "Avalanche",        950, 1700, 1000, 0,   0, 7, 0, 0, 7500, BOTH },
        { K::SHOCKWAVE,      "Quake Stomp",      700, 1300, 900,  0, 540, 1, 0, 0, 4000, BOTH },
        { K::BREATH_BEAM,    "Roar Blast",       900, 1700, 1100, 720, 0, 0, 200, 0, 7000, P2 },
    }},
    // ------------------------------------------------------------------ PEAK
    { BossType::PEAK_DASPLETO_WARLORD, Biome::PEAK, BossTier::MINI, "Dasplet Warlord",
      380, 200, 210, 220, combatType::GROUND, EnemyType::DASPELTOSAURUS, 700, 900, 850, 0.5f, 1.0f, 4, {
        { K::BITE_SNAP,      "Warlord Bite",     420, 230, 550, 220, 300, 0, 0, 240, 1400, BOTH },
        { K::CHARGE,         "Warhorn Rush",     650, 1000, 850, 1000, 720, 0, 300, 0, 3500, BOTH },
        { K::TAIL_SWEEP,     "Spiked Sweep",     550, 420, 700, 230,   0, 0, 0, 260, 2400, BOTH },
        { K::PROJECTILE_FAN, "Shard Volley",     650, 300, 850,   0, 600, 5, 0, 0, 3200, P2 },
    }},
    { BossType::PEAK_REX_EMPEROR, Biome::PEAK, BossTier::MAIN, "Rex Emperor",
      1000, 280, 290, 190, combatType::GROUND, EnemyType::TYRANNOSAURUS_REX_CHILD, 1000, 1200, 900, 0.5f, 1.0f, 5, {
        { K::BITE_SNAP,      "Emperor's Bite",   450, 240, 650, 280, 300, 0, 0, 300, 1400, BOTH },
        { K::TAIL_SWEEP,     "Tail Cyclone",     600, 500, 800, 300,   0, 0, 0, 320, 2500, BOTH },
        { K::LEAP_SLAM,      "Apex Leap",        750, 2000, 900,  0, 800, 1, 250, 0, 4500, BOTH },
        { K::ROCK_RAIN,      "Meteor Rain",      1000, 1800, 1000, 0,  0, 8, 0, 0, 8000, BOTH },
        { K::BREATH_BEAM,    "Cataclysm Roar",   950, 1900, 1200, 780, 0, 0, 250, 0, 7500, P2 },
    }},
};

struct BossToken { const char* name; BossType type; };
const BossToken kTokens[] = {
    {"MOSS_BROODMOTHER", BossType::MOSS_BROODMOTHER},       {"MOSS_LUMINOUS_WYRM", BossType::MOSS_LUMINOUS_WYRM},
    {"STONE_RAT_KING", BossType::STONE_RAT_KING},           {"STONE_THE_SINNER", BossType::STONE_THE_SINNER},
    {"SHAFT_FOREMAN_RAPTOR", BossType::SHAFT_FOREMAN_RAPTOR},{"SHAFT_BAT_MATRIARCH", BossType::SHAFT_BAT_MATRIARCH},
    {"PLAINS_ANKYLO_MATRIARCH", BossType::PLAINS_ANKYLO_MATRIARCH},{"PLAINS_SKY_TYRANT", BossType::PLAINS_SKY_TYRANT},
    {"WOODS_ALPHA_CAT", BossType::WOODS_ALPHA_CAT},         {"WOODS_THEROPOD_KING", BossType::WOODS_THEROPOD_KING},
    {"MOUNTAIN_SARCO_BRUTE", BossType::MOUNTAIN_SARCO_BRUTE},{"MOUNTAIN_TARBO_TYRANT", BossType::MOUNTAIN_TARBO_TYRANT},
    {"PEAK_DASPLETO_WARLORD", BossType::PEAK_DASPLETO_WARLORD},{"PEAK_REX_EMPEROR", BossType::PEAK_REX_EMPEROR},
};

const char* kBiomeTag[] = {"moss", "stone", "shaft", "plains", "woods", "mountain", "peak"};

constexpr float BOSS_ENEMY_PROJ_SIZE = 34.0f;
constexpr float BOSS_ROCK_SIZE       = 52.0f;
constexpr float BOSS_WAVE_W          = 64.0f;
constexpr float BOSS_WAVE_H          = 46.0f;

inline float wallBottomOf(const SDL_FRect& wall){
    return wall.y + std::ceil(wall.h / (SCALE * TILE_SIZE)) * (SCALE * TILE_SIZE) - TILE_SIZE * SCALE;
}
inline bool rectsOverlap(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh){
    return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}
inline float horizontalGap(const Boss& b, const SDL_FRect& p){
    return std::max(0.0f, std::max(p.x - (b.x + b.w), b.x - (p.x + p.w)));
}
}  // namespace

// ====================================================================================================
//  lookups
// ====================================================================================================
const BossDef* getBossDef(BossType type){
    for(const auto& d : kDefs) if(d.type == type) return &d;
    return nullptr;
}

BossType bossTypeFromName(const std::string& name){
    for(const auto& t : kTokens) if(name == t.name) return t.type;
    return BossType::NONE;
}

std::string getBossMusicPath(const BossDef& def, BossPhase phase){
    return std::string("Sound/Music/Boss/") + kBiomeTag[(int)def.biome] + "_" +
           (def.tier == BossTier::MINI ? "mini" : "main") +
           (phase == BossPhase::PHASE_1 ? "_phase1.ogg" : "_phase2.ogg");
}

Boss::Boss(float sx, float sy, BossType t) : type(t) {
    x = baseX = sx;
    y = baseY = sy;
    if(const BossDef* d = getBossDef(t)){
        w = d->w; h = d->h;
        hp = maxHp = d->hp;
        combat = d->combat;
    }
}

// ====================================================================================================
//  lifecycle
// ====================================================================================================
void BossBuilder::init(const std::vector<Boss>& bosses){
    // if a fight was in progress (player died / level restarted) the level music has to come back
    if(m_activeIdx >= 0){
        m_restorePending = true;
        m_restoreAt = SDL_GetTicks();
    }
    m_bosses = bosses;
    m_template = bosses;
    m_arenaValid = false;
    m_unlockAt = 0;
    m_hazards.clear();
    m_hitEvents.clear();
    m_deathEvents.clear();
    m_summons.clear();
    m_activeIdx = -1;
    m_killScore = 0;
    m_rng.seed(SDL_GetTicks());
}

void BossBuilder::setState(Boss& b, BossState s, unsigned int now){
    b.state = s;
    b.stateStart = now;
    b.isAttackBoxActive = false;
}

void BossBuilder::faceToward(Boss& b, float targetCenterX) const {
    const float d = targetCenterX - (b.x + b.w * 0.5f);
    if(d >  ENEMY_FACE_DEADZONE) b.isFacingRight = true;
    else if(d < -ENEMY_FACE_DEADZONE) b.isFacingRight = false;
}

void BossBuilder::beginFight(int idx, unsigned int now){
    Boss& b = m_bosses[(size_t)idx];
    const BossDef* def = getBossDef(b.type);
    if(!def) return;
    m_activeIdx = idx;
    m_restorePending = false;
    m_arena = b.arena;                       // the arena locks now (GameState reads isArenaLocked()/getArena())
    m_arenaInnerL = b.arenaL;
    m_arenaInnerR = b.arenaR;
    m_arenaValid = true;
    m_unlockAt = 0;
    setState(b, BossState::INTRO, now);
    playBossMusic(b, *def, BossPhase::PHASE_1);   // level music -> boss phase 1
    Camera::getInstance().addTrauma(0.5f);
}

void BossBuilder::beginPhaseTransition(Boss& b, unsigned int now){
    const BossDef* def = getBossDef(b.type);
    if(!def) return;
    setState(b, BossState::PHASE_TRANSITION, now);
    b.attack = -1;
    b.forcedAttack = -1;
    for(int i = 0; i < def->attackCount; ++i)         // phase 2 opens with the phase-2-only move so it is always seen
        if(def->attacks[i].phaseMask == P2){ b.forcedAttack = i; break; }
    m_hazards.clear();   // a clean screen for the roar
    playBossMusic(b, *def, BossPhase::PHASE_2);   // boss phase 1 -> boss phase 2 (starts with the roar)
    Camera::getInstance().addTrauma(0.8f);
}

void BossBuilder::startDeath(Boss& b, unsigned int now){
    if(b.isDead()) return;
    const BossDef* def = getBossDef(b.type);
    setState(b, BossState::DEAD, now);
    b.deathTime = now;
    b.isHurt = false;
    b.attackHasHit = true;
    b.hp = 0;
    m_hazards.clear();
    m_deathEvents.push_back({b.x + b.w * 0.5f, b.y + b.h * 0.5f});
    m_killScore += (def && def->tier == BossTier::MAIN) ? BOSS_MAIN_KILL_SCORE : BOSS_MINI_KILL_SCORE;
    Camera::getInstance().addTrauma(1.0f);
    Engine::Get().getAssetManager().fadeOutMusic(BOSS_MUSIC_FADE_MS);
    const int idx = (int)(&b - m_bosses.data());
    if(idx == m_activeIdx){
        m_activeIdx = -1;
        m_unlockAt = now + BOSS_ARENA_UNLOCK_MS;          // arena stays pinned for the death burst, then releases
        m_restorePending = true;
        m_restoreAt = now + BOSS_DEATH_MUSIC_RESTORE_MS;
    }
}

BossHudInfo BossBuilder::getHudInfo() const {
    BossHudInfo info;
    if(m_activeIdx < 0) return info;
    const Boss& b = m_bosses[(size_t)m_activeIdx];
    const BossDef* def = getBossDef(b.type);
    if(!def) return info;
    info.name = def->name;
    info.hpFraction = std::clamp((float)b.hp / (float)std::max(1, b.maxHp), 0.0f, 1.0f);
    info.phase = b.phase;
    info.tier = def->tier;
    info.active = b.state != BossState::DORMANT;
    return info;
}

void BossBuilder::shiftTimers(unsigned int ms){
    for(auto& b : m_bosses){
        if(b.stateStart)     b.stateStart     += ms;
        if(b.nextAttackTime) b.nextAttackTime += ms;
        for(auto& r : b.attackReadyAt) if(r) r += ms;
        if(b.nextSpawnTick)  b.nextSpawnTick  += ms;
        if(b.nextRearm)      b.nextRearm      += ms;
        if(b.hurtFlashEnd)   b.hurtFlashEnd   += ms;
        if(b.whiteFlashEnd)  b.whiteFlashEnd  += ms;
        if(b.nextBeamFx)     b.nextBeamFx     += ms;
        if(b.deathTime)      b.deathTime      += ms;
    }
    if(m_restoreAt) m_restoreAt += ms;
    if(m_unlockAt)  m_unlockAt  += ms;
}

// ====================================================================================================
//  arena + music + reset
// ====================================================================================================
// The arena is a rectangle that fits ONE screen: the camera is pinned on it during the fight, so everything the
// player has to see (boss, projectiles, rock warnings) is always on screen and nothing scrolls.
void BossBuilder::setupArena(Boss& b, const SDL_FRect& wall){
    const float wallT = TILE_SIZE * SCALE;
    const float viewW = Camera::getInstance().getCamera().w > 0.0f ? Camera::getInstance().getCamera().w : 1600.0f;
    const float innerL = wall.x + wallT, innerR = wall.x + wall.w - wallT;

    SDL_FRect a = b.arena;
    if(!b.hasArena){
        const float cx = b.baseX + b.w * 0.5f;
        a.w = std::min(viewW, wall.w);
        a.x = std::clamp(cx - a.w * 0.5f, wall.x, std::max(wall.x, wall.x + wall.w - a.w));
        a.y = wall.y;
        a.h = std::ceil(wall.h / wallT) * wallT;
    }
    if(a.w > viewW){ a.x += (a.w - viewW) * 0.5f; a.w = viewW; }     // must fit the fixed camera

    b.arena = a;
    b.arenaL = std::max(a.x, innerL);
    b.arenaR = std::min(a.x + a.w, innerR);
    if(b.arenaR - b.arenaL < b.w + 10.0f){ b.arenaL = innerL; b.arenaR = innerR; }   // degenerate 'A': fall back to the whole level
    b.x = std::clamp(b.x, b.arenaL, std::max(b.arenaL, b.arenaR - b.w));
    b.baseX = b.x;
    b.arenaReady = true;
}

// Boss tracks never leave the game silent:
//   phase 1: boss file -> (missing) the level music simply keeps playing
//   phase 2: boss file -> (missing) phase 1 keeps playing
void BossBuilder::playBossMusic(const Boss& b, const BossDef& def, BossPhase phase){
    AssetManager& am = Engine::Get().getAssetManager();
    auto pathFor = [&](BossPhase ph){
        const std::string& o = (ph == BossPhase::PHASE_1) ? b.musicPhase1 : b.musicPhase2;
        return o.empty() ? getBossMusicPath(def, ph) : o;
    };
    const float gain = b.musicGain >= 0.0f ? b.musicGain : def.musicGain;
    if(am.tryPlayMusicFile(pathFor(phase), gain)) return;
    if(phase == BossPhase::PHASE_2) am.tryPlayMusicFile(pathFor(BossPhase::PHASE_1), gain);
}

bool BossBuilder::isArenaLocked() const {
    if(!m_arenaValid) return false;
    return m_activeIdx >= 0 || SDL_GetTicks() < m_unlockAt;
}

bool BossBuilder::getArena(SDL_FRect& out) const {
    if(!m_arenaValid) return false;
    out = m_arena;
    return true;
}

bool BossBuilder::hasLivingBoss() const {
    for(const auto& b : m_bosses) if(!b.isDead()) return true;
    return false;
}

bool BossBuilder::resetFight(){
    const bool wasActive = m_activeIdx >= 0;
    for(size_t i = 0; i < m_bosses.size() && i < m_template.size(); ++i){
        if(m_bosses[i].isDead()) continue;                 // a boss that was beaten stays beaten
        const bool ready = m_bosses[i].arenaReady;         // keep the computed arena, it doesn't change
        const SDL_FRect arena = m_bosses[i].arena;
        const float aL = m_bosses[i].arenaL, aR = m_bosses[i].arenaR, bx = m_bosses[i].baseX;
        m_bosses[i] = m_template[i];
        if(ready){
            m_bosses[i].arenaReady = true; m_bosses[i].arena = arena;
            m_bosses[i].arenaL = aL; m_bosses[i].arenaR = aR;
            m_bosses[i].baseX = bx; m_bosses[i].x = bx;
        }
    }
    m_hazards.clear();
    m_hitEvents.clear();
    m_summons.clear();
    m_activeIdx = -1;
    m_unlockAt = 0;
    if(wasActive){                                         // the boss track is playing: bring the level music back now
        m_restorePending = true;
        m_restoreAt = SDL_GetTicks();
    }
    return wasActive;
}

// ====================================================================================================
//  main update
// ====================================================================================================
void BossBuilder::update(float dt, const SDL_FRect& player, const SDL_FRect& wall,
                         const std::vector<LevelGround>& grounds, const std::vector<Platform>& platforms,
                         const std::vector<Block>& blocks){
    const unsigned int now = SDL_GetTicks();
    m_floorY = wallBottomOf(wall);

    if(m_restorePending && m_activeIdx < 0 && now >= m_restoreAt){
        m_restorePending = false;
        m_restoreReady = true;
    }

    for(auto& b : m_bosses)
        updateBoss(b, dt, player, wall, grounds, platforms, blocks, now);

    updateHazards(dt, wall, grounds, platforms, blocks);
}

void BossBuilder::updateBoss(Boss& b, float dt, const SDL_FRect& player, const SDL_FRect& wall,
                             const std::vector<LevelGround>& grounds, const std::vector<Platform>& platforms,
                             const std::vector<Block>& blocks, unsigned int now){
    const BossDef* def = getBossDef(b.type);
    if(!def) return;

    if(!b.arenaReady) setupArena(b, wall);
    if(b.isHurt && now >= b.hurtFlashEnd) b.isHurt = false;

    const float pcx = player.x + player.w * 0.5f, pcy = player.y + player.h * 0.5f;
    const float bcx = b.x + b.w * 0.5f,           bcy = b.y + b.h * 0.5f;
    const bool  p2  = b.phase == BossPhase::PHASE_2;
    const float spd = def->moveSpeed * phaseSpeedMul(b);
    const unsigned int elapsed = now - b.stateStart;

    switch(b.state){
        case BossState::DORMANT: {
            if(m_activeIdx >= 0) break;     // one fight at a time
            // the fight starts once the player is properly inside the arena (BOSS_ARENA_ENTER_MARGIN from both ends)
            const bool inside = player.x >= b.arenaL + BOSS_ARENA_ENTER_MARGIN &&
                                player.x + player.w <= b.arenaR - BOSS_ARENA_ENTER_MARGIN &&
                                player.y + player.h > b.arena.y && player.y < b.arena.y + b.arena.h;
            if(inside) beginFight((int)(&b - m_bosses.data()), now);
            break;
        }

        case BossState::INTRO:
            faceToward(b, pcx);
            Camera::getInstance().rumble(0.25f);
            if(elapsed >= 1500){
                setState(b, BossState::IDLE, now);
                b.nextAttackTime = now + 700;
            }
            break;

        case BossState::IDLE: {
            faceToward(b, pcx);
            if(b.phase == BossPhase::PHASE_1 && (float)b.hp <= (float)b.maxHp * def->phase2Threshold){
                beginPhaseTransition(b, now);
                break;
            }
            if(def->combat == combatType::GROUND){
                const float gap = horizontalGap(b, player);
                const float stand = b.w * 0.25f;                       // close in to this gap, never walk into the player
                if(gap > stand){
                    const float step = std::min(spd * dt, gap - stand);
                    tryMoveX(b, b.x + ((pcx > bcx) ? step : -step), wall, blocks);
                }
            } else {
                hoverToward(b, pcx - b.w * 0.5f, b.baseY + std::sin(now * 0.003f) * 14.0f, spd * 0.8f, dt, wall);
            }
            if(now >= b.nextAttackTime){
                int pick = b.forcedAttack >= 0 ? b.forcedAttack : pickAttack(b, *def, player, now);
                b.forcedAttack = -1;
                if(pick >= 0){
                    b.attack = pick;
                    beginAttack(b, now);
                } else {
                    b.nextAttackTime = now + 250;      // everything on cooldown: look again shortly
                }
            }
            break;
        }

        case BossState::TELEGRAPH: {
            const BossAttackDef& a = def->attacks[b.attack];
            const unsigned int windup = (unsigned int)(a.windupMs * (p2 ? BOSS_PHASE2_WINDUP_MUL : 1.0f));
            if(elapsed < windup * 0.6f) faceToward(b, pcx);            // tracks early, then locks so the tell is fair
            if(def->combat == combatType::AERIAL)
                hoverToward(b, pcx - b.w * 0.5f, b.baseY, spd * 0.5f, dt, wall);
            b.attackHitBox = previewBox(b, a, player, wall);
            b.isAttackBoxActive = false;
            if(elapsed >= windup){
                setState(b, BossState::ATTACKING, now);
                b.attackHasHit = false;
            }
            break;
        }

        case BossState::ATTACKING: {
            const BossAttackDef& a = def->attacks[b.attack];
            runAttack(b, *def, a, dt, player, wall, blocks, now, elapsed);
            if(b.endEarly || elapsed >= a.activeMs){
                b.isAttackBoxActive = false;
                b.attackHasHit = true;
                b.attackReadyAt[b.attack] = now + a.cooldownMs;
                if(b.attack == b.lastAttack) b.streak++;
                else { b.lastAttack = b.attack; b.streak = 1; }
                setState(b, BossState::RECOVER, now);
            }
            break;
        }

        case BossState::RECOVER: {
            const BossAttackDef& a = def->attacks[std::max(0, b.attack)];
            const unsigned int rec = (unsigned int)(a.recoverMs * (p2 ? BOSS_PHASE2_RECOVER_MUL : 1.0f)) + b.extraRecoverMs;
            if(def->combat == combatType::AERIAL)
                hoverToward(b, b.x, b.baseY, spd * 0.7f, dt, wall);    // climb back to hover height after a dive
            if(elapsed >= rec){
                setState(b, BossState::IDLE, now);
                b.nextAttackTime = now + (unsigned int)(def->idleMs * (p2 ? BOSS_PHASE2_IDLE_MUL : 1.0f));
                b.attack = -1;
                b.extraRecoverMs = 0;
            }
            break;
        }

        case BossState::PHASE_TRANSITION:
            faceToward(b, pcx);
            Camera::getInstance().rumble(0.45f);
            if(def->combat == combatType::AERIAL)
                hoverToward(b, b.x, b.baseY, spd * 0.5f, dt, wall);
            if(elapsed >= BOSS_PHASE_TRANSITION_MS){
                b.phase = BossPhase::PHASE_2;
                setState(b, BossState::IDLE, now);
                b.nextAttackTime = now + 500;
            }
            break;

        case BossState::DEAD:
            break;   // corpse: gravity only (below)
    }

    // ground bosses (and every corpse) fall and collide like ground enemies do
    if(b.combat == combatType::GROUND || b.isDead())
        applyGravityAndCollision(b, dt, wall, grounds, platforms, blocks);
}

// ====================================================================================================
//  attack selection
// ====================================================================================================
int BossBuilder::pickAttack(Boss& b, const BossDef& def, const SDL_FRect& player, unsigned int now){
    const float gap = horizontalGap(b, player);
    const unsigned int bit = (b.phase == BossPhase::PHASE_2) ? P2 : P1;

    float weights[BOSS_MAX_ATTACKS] = {0, 0, 0, 0, 0};
    float total = 0.0f;
    for(int i = 0; i < def.attackCount; ++i){
        const BossAttackDef& a = def.attacks[i];
        if(!(a.phaseMask & bit)) continue;
        if(now < b.attackReadyAt[i]) continue;
        const bool groundOnly = a.kind == K::CHARGE || a.kind == K::LEAP_SLAM || a.kind == K::SHOCKWAVE || a.kind == K::TAIL_SWEEP;
        if(groundOnly && def.combat != combatType::GROUND) continue;
        if(a.kind == K::AERIAL_DIVE && def.combat != combatType::AERIAL) continue;

        float w = 1.0f;
        if(a.minGap > 0.0f || a.maxGap > 0.0f){
            const bool inWindow = (a.minGap <= 0.0f || gap >= a.minGap) && (a.maxGap <= 0.0f || gap <= a.maxGap);
            w = inWindow ? 3.0f : 0.4f;
        }
        if(i == b.lastAttack) w = (b.streak >= 2) ? 0.0f : w * 0.3f;   // never the same move three times running
        weights[i] = w;
        total += w;
    }
    if(total <= 0.0f) return -1;

    std::uniform_real_distribution<float> dist(0.0f, total);
    float r = dist(m_rng);
    for(int i = 0; i < def.attackCount; ++i){
        if(weights[i] <= 0.0f) continue;
        if(r < weights[i]) return i;
        r -= weights[i];
    }
    for(int i = def.attackCount - 1; i >= 0; --i) if(weights[i] > 0.0f) return i;   // float rounding fallback
    return -1;
}

void BossBuilder::beginAttack(Boss& b, unsigned int now){
    b.launched = false;
    b.endEarly = false;
    b.dirX = b.dirY = 0.0f;
    b.travelled = b.budget = 0.0f;
    b.extraRecoverMs = 0;
    b.spawned = 0;
    b.nextSpawnTick = 0;
    b.nextRearm = 0;
    b.attackHasHit = false;
    setState(b, BossState::TELEGRAPH, now);
}

// the outline shown during the wind-up (also what the placeholder renderer draws)
SDL_FRect BossBuilder::previewBox(const Boss& b, const BossAttackDef& a, const SDL_FRect& player, const SDL_FRect& wall) const {
    auto front = [&](float len, float yOff, float hh){
        return SDL_FRect{ b.isFacingRight ? (b.x + b.w) : (b.x - len), b.y + yOff, len, hh };
    };
    switch(a.kind){
        case K::CLAW_SWIPE:  return front(a.reach, 0.0f, b.h);
        case K::BITE_SNAP:   return front(a.reach, 0.0f, b.h * 0.5f);
        case K::BREATH_BEAM: return front(a.reach, b.h * 0.2f, b.h * 0.45f);
        case K::TAIL_SWEEP:  return { b.x - a.reach, b.y + b.h * 0.55f, b.w + a.reach * 2.0f, b.h * 0.45f };
        case K::CHARGE: {
            const float toWall = b.isFacingRight ? (b.arenaR - (b.x + b.w)) : (b.x - b.arenaL);
            const float len = std::max(0.0f, std::min(a.reach > 0.0f ? a.reach : toWall, toWall));
            return front(len, 0.0f, b.h);
        }
        case K::LEAP_SLAM:
        case K::AERIAL_DIVE:
            // landing marker on the player's current spot
            return { player.x - 30.0f, std::min(player.y, wallBottomOf(wall) - player.h), player.w + 60.0f, player.h };
        default: return {0, 0, 0, 0};
    }
}

// ====================================================================================================
//  attack execution (one call per frame while ATTACKING)
// ====================================================================================================
void BossBuilder::runAttack(Boss& b, const BossDef& def, const BossAttackDef& a, float dt, const SDL_FRect& player,
                            const SDL_FRect& wall, const std::vector<Block>& blocks, unsigned int now, unsigned int elapsed){
    const bool  p2 = b.phase == BossPhase::PHASE_2;
    const float sp = a.speed * phaseSpeedMul(b);
    const float pcx = player.x + player.w * 0.5f, pcy = player.y + player.h * 0.5f;
    const float bcx = b.x + b.w * 0.5f,           bcy = b.y + b.h * 0.5f;
    const float dirSign = b.isFacingRight ? 1.0f : -1.0f;
    auto bodyBox = [&](){ return SDL_FRect{ b.x - 3.0f, b.y - 3.0f, b.w + 6.0f, b.h + 6.0f }; };
    auto frontBox = [&](float len, float yOff, float hh){
        return SDL_FRect{ b.isFacingRight ? (b.x + b.w) : (b.x - len), b.y + yOff, len, hh };
    };

    switch(a.kind){
        case K::CLAW_SWIPE:
        case K::BITE_SNAP: {
            // short lunge during the first half of the swing, then the box stays live for the whole window
            if(elapsed < a.activeMs * 0.5f && horizontalGap(b, player) > 12.0f)
                tryMoveX(b, b.x + dirSign * sp * dt, wall, blocks);
            b.attackHitBox = (a.kind == K::BITE_SNAP) ? frontBox(a.reach, 0.0f, b.h * 0.5f) : frontBox(a.reach, 0.0f, b.h);
            b.isAttackBoxActive = !b.attackHasHit;
            break;
        }

        case K::CHARGE: {
            if(!b.launched){
                b.launched = true;
                b.dirX = (pcx >= bcx) ? 1.0f : -1.0f;
                b.isFacingRight = b.dirX > 0.0f;
                b.travelled = 0.0f;
            }
            const float step = sp * dt;
            if(tryMoveX(b, b.x + b.dirX * step, wall, blocks)){
                b.travelled += step;
                if(a.reach > 0.0f && b.travelled >= a.reach) b.endEarly = true;
            } else {                                   // slammed into a wall: stunned, wide open
                b.endEarly = true;
                b.extraRecoverMs = 700;
                Camera::getInstance().addTrauma(0.55f);
            }
            Camera::getInstance().rumble(0.15f);
            b.attackHitBox = bodyBox();
            b.isAttackBoxActive = !b.attackHasHit;
            break;
        }

        case K::LEAP_SLAM: {
            if(!b.launched){
                b.launched = true;
                faceToward(b, pcx);
                const float dx = pcx - bcx, dy = pcy - bcy;
                const float T = std::clamp(std::fabs(dx) / std::max(sp, 1.0f), 0.4f, 0.9f);
                b.dirX = dx / T;                                                           // horizontal launch speed
                b.velocityY = std::max(-1300.0f, (dy - 0.5f * ENEMY_GRAVITY * T * T) / T); // vertical launch (up = negative)
                b.isGrounded = false;
                b.travelled = 0.0f;                                                        // seconds in the air
            }
            b.travelled += dt;
            if(!tryMoveX(b, b.x + b.dirX * dt, wall, blocks)) b.dirX = 0.0f;              // bonked a wall: drop straight down
            b.attackHitBox = bodyBox();
            b.isAttackBoxActive = !b.attackHasHit;
            if(b.travelled > 0.15f && b.isGrounded && b.velocityY >= 0.0f){
                Camera::getInstance().addTrauma(0.7f);
                const int waves = a.count;
                for(int i = 0; i < waves; ++i) spawnShockwavePair(b, a);                   // landing shock (count 0 = none)
                b.endEarly = true;
            }
            break;
        }

        case K::SHOCKWAVE: {
            const int total = a.count + (p2 ? 1 : 0);
            const float interval = (float)a.activeMs / (float)std::max(1, total);
            while(b.spawned < total && (float)elapsed >= (float)b.spawned * interval){
                spawnShockwavePair(b, a);
                Camera::getInstance().addTrauma(0.5f);
                b.spawned++;
            }
            break;
        }

        case K::ROCK_RAIN: {
            const int total = a.count + (p2 ? 2 : 0);
            const float interval = (float)a.activeMs / (float)std::max(1, total);
            Camera::getInstance().rumble(0.3f);
            while(b.spawned < total && (float)elapsed >= (float)b.spawned * interval){
                float x = pcx;                                    // every 3rd rock (and the 1st) lands where the player stands
                if(b.spawned % 3 != 0){
                    std::uniform_real_distribution<float> d(b.arenaL + 40.0f, std::max(b.arenaL + 41.0f, b.arenaR - 40.0f));
                    x = d(m_rng);
                }
                spawnRock(b, x, wall);
                b.spawned++;
            }
            break;
        }

        case K::PROJECTILE_ARC: {
            const int total = std::max(1, a.count);
            const float interval = (float)a.activeMs / (float)total;
            while(b.spawned < total && (float)elapsed >= (float)b.spawned * interval){
                faceToward(b, pcx);
                spawnArcProjectile(b, a, player);
                b.spawned++;
            }
            break;
        }

        case K::PROJECTILE_FAN: {
            if(!b.launched){
                b.launched = true;
                faceToward(b, pcx);
                spawnFan(b, a, player, a.count + (p2 ? 2 : 0));
                Camera::getInstance().addTrauma(0.25f);
            }
            break;
        }

        case K::BREATH_BEAM: {
            if(p2) faceToward(b, pcx);                            // phase 2: the beam follows the player
            if(!b.launched){ b.launched = true; b.nextRearm = now + BOSS_BEAM_REARM_MS; }
            else if(now >= b.nextRearm){ b.attackHasHit = false; b.nextRearm = now + BOSS_BEAM_REARM_MS; }
            b.attackHitBox = frontBox(a.reach, b.h * 0.2f, b.h * 0.45f);
            b.isAttackBoxActive = !b.attackHasHit;
            Camera::getInstance().rumble(0.3f);
            break;
        }

        case K::AERIAL_DIVE: {
            const float floorY = wallBottomOf(wall);
            if(!b.launched){
                b.launched = true;
                const float dx = pcx - bcx;
                float dy = pcy - bcy;
                dy = std::max(dy, std::max(std::fabs(dx) * ENEMY_DIVE_MIN_SLOPE, 120.0f));   // never flatter than the min dive
                const float len = std::sqrt(dx * dx + dy * dy);
                b.dirX = dx / len;
                b.dirY = dy / len;
                b.isFacingRight = dx >= 0.0f;
                b.budget = len + 220.0f;
                b.travelled = 0.0f;
            }
            const float step = sp * dt;
            float nx = b.x + b.dirX * step;
            if(nx < b.arenaL || nx + b.w > b.arenaR) nx = std::clamp(nx, b.arenaL, b.arenaR - b.w);
            b.x = nx;
            b.y += b.dirY * step;
            b.travelled += step;
            if(b.travelled >= b.budget || b.y + b.h >= std::min(floorY, player.y + player.h + 10.0f)){
                b.y = std::min(b.y, floorY - b.h);
                b.endEarly = true;
            }
            b.attackHitBox = bodyBox();
            b.isAttackBoxActive = !b.attackHasHit;
            break;
        }

        case K::TAIL_SWEEP: {
            b.attackHitBox = { b.x - a.reach, b.y + b.h * 0.55f, b.w + a.reach * 2.0f, b.h * 0.45f };
            b.isAttackBoxActive = !b.attackHasHit;
            break;
        }

        case K::SUMMON: {
            if(!b.launched){
                b.launched = true;
                Camera::getInstance().addTrauma(0.3f);
                for(int i = 0; i < a.count; ++i){
                    const float side = (i % 2 == 0) ? 1.0f : -1.0f;
                    const float x = std::clamp(bcx + side * (b.w * 0.5f + 90.0f + (float)(i / 2) * 80.0f),
                                               b.arenaL + 30.0f, b.arenaR - 70.0f);
                    m_summons.push_back({ def.minion, x, b.y, def.combat });
                }
            }
            break;
        }

        default: break;
    }
}

// ====================================================================================================
//  hazards: projectiles, ground waves, rocks
// ====================================================================================================
void BossBuilder::spawnArcProjectile(const Boss& b, const BossAttackDef& a, const SDL_FRect& player){
    BossHazard h;
    h.source = K::PROJECTILE_ARC;
    h.w = h.h = BOSS_ENEMY_PROJ_SIZE;
    h.gravity = BOSS_PROJECTILE_GRAVITY;
    h.x = b.isFacingRight ? (b.x + b.w) : (b.x - h.w);
    h.y = b.y + b.h * 0.25f - h.h * 0.5f;
    const float dx = (player.x + player.w * 0.5f) - (h.x + h.w * 0.5f);
    const float dy = (player.y + player.h * 0.5f) - (h.y + h.h * 0.5f);
    const float T = std::clamp(std::fabs(dx) / std::max(a.speed * phaseSpeedMul(b), 1.0f),
                               ENEMY_PROJECTILE_MIN_FLIGHT, ENEMY_PROJECTILE_MAX_FLIGHT);
    h.vx = dx / T;
    h.vy = (dy - 0.5f * h.gravity * T * T) / T;
    h.life = ENEMY_PROJECTILE_MAX_LIFE;
    m_hazards.push_back(h);
}

void BossBuilder::spawnFan(const Boss& b, const BossAttackDef& a, const SDL_FRect& player, int count){
    count = std::max(1, count);
    const float ox = b.isFacingRight ? (b.x + b.w) : b.x;
    const float oy = b.y + b.h * 0.3f;
    const float base = std::atan2((player.y + player.h * 0.5f) - oy, (player.x + player.w * 0.5f) - ox);
    const float spread = (count > 1) ? 0.9f : 0.0f;               // total fan width in radians
    const float sp = a.speed * phaseSpeedMul(b);
    for(int i = 0; i < count; ++i){
        const float ang = base + (count > 1 ? (-spread * 0.5f + spread * (float)i / (float)(count - 1)) : 0.0f);
        BossHazard h;
        h.source = K::PROJECTILE_FAN;
        h.w = h.h = 28.0f;
        h.x = ox - h.w * 0.5f;
        h.y = oy - h.h * 0.5f;
        h.vx = std::cos(ang) * sp;
        h.vy = std::sin(ang) * sp;
        h.gravity = 250.0f;                                        // barely drops, so the fan stays readable
        h.life = 3.0f;
        m_hazards.push_back(h);
    }
}

void BossBuilder::spawnShockwavePair(const Boss& b, const BossAttackDef& a){
    const float feetY = b.y + b.h;
    const float sp = std::max(a.speed, 300.0f) * phaseSpeedMul(b);
    const float cx = b.x + b.w * 0.5f;
    for(int dir = -1; dir <= 1; dir += 2){
        BossHazard h;
        h.source = K::SHOCKWAVE;
        h.w = BOSS_WAVE_W; h.h = BOSS_WAVE_H;
        h.x = cx + (float)dir * (b.w * 0.5f) - (dir > 0 ? 0.0f : h.w);
        h.y = feetY - h.h - 1.0f;                                  // rides the floor
        h.vx = (float)dir * sp;
        h.life = 1.8f;
        h.dieOnTerrain = false;                                    // only blocks and the arena end it (see updateHazards)
        h.hugsFloor = true;
        m_hazards.push_back(h);
    }
}

void BossBuilder::spawnRock(const Boss& b, float x, const SDL_FRect& wall){
    (void)b;
    BossHazard h;
    h.source = K::ROCK_RAIN;
    h.w = h.h = BOSS_ROCK_SIZE;
    h.x = x - h.w * 0.5f;
    h.y = wall.y + TILE_SIZE * SCALE + 4.0f;                       // just under the ceiling
    h.gravity = BOSS_ROCK_GRAVITY;
    h.warn = BOSS_ROCK_WARN_SEC;
    h.life = 4.0f;
    m_hazards.push_back(h);
}

void BossBuilder::updateHazards(float dt, const SDL_FRect& wall, const std::vector<LevelGround>& grounds,
                                const std::vector<Platform>& platforms, const std::vector<Block>& blocks){
    if(m_hazards.empty()) return;
    const float wallT = TILE_SIZE * SCALE;
    float left = wall.x + wallT, right = wall.x + wall.w - wallT;
    const float top = wall.y + wallT, bottom = wallBottomOf(wall);
    if(m_arenaValid){ left = std::max(left, m_arenaInnerL); right = std::min(right, m_arenaInnerR); }   // nothing leaves the arena

    for(size_t i = 0; i < m_hazards.size(); ){
        BossHazard& h = m_hazards[i];
        if(h.warn > 0.0f){ h.warn -= dt; ++i; continue; }          // marker phase: nothing moves, nothing hurts

        h.age += dt;
        h.vy += h.gravity * dt;
        h.x += h.vx * dt;
        h.y += h.vy * dt;

        bool dead = h.age >= h.life;
        if(!dead) dead = h.x < left || h.x + h.w > right || h.y + h.h > bottom || (h.source != K::ROCK_RAIN && h.y < top);
        if(!dead){
            for(const auto& bl : blocks)
                if(rectsOverlap(h.x, h.y, h.w, h.h, bl.rect.x, bl.rect.y, bl.rect.w, bl.rect.h)){ dead = true; break; }
        }
        if(!dead && h.dieOnTerrain){
            for(const auto& g : grounds)
                if(rectsOverlap(h.x, h.y, h.w, h.h, g.x, g.y, g.w * SCALE, g.h * SCALE)){ dead = true; break; }
            if(!dead)
                for(const auto& pl : platforms){
                    if(pl.colliderType != ColliderType::SOLID) continue;     // one-way platforms let it through
                    if(rectsOverlap(h.x, h.y, h.w, h.h, pl.x, pl.y, pl.w * SCALE, pl.h * SCALE)){ dead = true; break; }
                }
        }
        if(!dead && h.hugsFloor){
            // a ground wave dies if it meets a step in the floor (ground taller than the wave's feet)
            for(const auto& g : grounds)
                if(rectsOverlap(h.x, h.y, h.w, h.h, g.x, g.y, g.w * SCALE, g.h * SCALE)){ dead = true; break; }
        }

        if(dead) m_hazards.erase(m_hazards.begin() + (long)i);
        else     ++i;
    }
}

// ====================================================================================================
//  movement / physics
// ====================================================================================================
bool BossBuilder::tryMoveX(Boss& b, float nextX, const SDL_FRect& wall, const std::vector<Block>& blocks) const {
    const float wallT = TILE_SIZE * SCALE;
    const float left  = std::max(wall.x + wallT, b.arenaL);
    const float right = std::min(wall.x + wall.w - wallT, b.arenaR);
    if(nextX < left || nextX + b.w > right) return false;
    if(b.combat == combatType::GROUND){
        for(const auto& bl : blocks)
            if(gameMath::checkcollision(nextX, b.y, bl.rect.x, bl.rect.y, b.h, b.w, bl.rect.h, bl.rect.w)) return false;
    }
    b.x = nextX;
    return true;
}

void BossBuilder::hoverToward(Boss& b, float targetX, float targetY, float speed, float dt, const SDL_FRect& wall) const {
    const float wallT = TILE_SIZE * SCALE;
    const float step = speed * dt;
    b.x += std::clamp(targetX - b.x, -step, step);
    b.y += std::clamp(targetY - b.y, -step, step);
    b.x = std::clamp(b.x, b.arenaL, std::max(b.arenaL, b.arenaR - b.w));
    b.y = std::clamp(b.y, wall.y + wallT + 4.0f, std::max(wall.y + wallT + 5.0f, wallBottomOf(wall) - b.h - 4.0f));
}

// same collision pass EnemiesBuilder uses (level walls, solid + one-way platforms, ground, blocks)
void BossBuilder::applyGravityAndCollision(Boss& b, float dt, const SDL_FRect& wallCollisionRect,
                                           const std::vector<LevelGround>& grounds,
                                           const std::vector<Platform>& platforms,
                                           const std::vector<Block>& blocks){
    const float previousY = b.y;
    b.isGrounded = false;
    b.velocityY += ENEMY_GRAVITY * dt;
    b.y += b.velocityY * dt;

    const float renderedHeight = (std::ceil(wallCollisionRect.h / (SCALE * TILE_SIZE)) * (SCALE * TILE_SIZE));
    gameMath::collisionSide wallSide;

    wallSide = gameMath::checkcollisionXY(b.x, b.y, wallCollisionRect.x, wallCollisionRect.y,
                                          b.h, b.w, TILE_SIZE * SCALE, wallCollisionRect.w);
    if(wallSide == gameMath::collisionSide::BOTTOM) b.velocityY = 0.0f;

    wallSide = gameMath::checkcollisionXY(b.x, b.y, wallCollisionRect.x, wallCollisionRect.y,
                                          b.h, b.w, renderedHeight, TILE_SIZE * SCALE);
    if(wallSide == gameMath::collisionSide::BOTTOM) b.velocityY = 0.0f;

    wallSide = gameMath::checkcollisionXY(b.x, b.y, wallCollisionRect.x + wallCollisionRect.w - TILE_SIZE * SCALE,
                                          wallCollisionRect.y, b.h, b.w, renderedHeight, TILE_SIZE * SCALE);
    if(wallSide == gameMath::collisionSide::BOTTOM) b.velocityY = 0.0f;

    wallSide = gameMath::checkcollisionXY(b.x, b.y, wallCollisionRect.x,
                                          wallCollisionRect.y + renderedHeight - TILE_SIZE * SCALE,
                                          b.h, b.w, TILE_SIZE * SCALE, wallCollisionRect.w);
    if(wallSide == gameMath::collisionSide::BOTTOM) b.velocityY = 0.0f;
    if(wallSide == gameMath::collisionSide::TOP){ b.isGrounded = true; b.velocityY = 0.0f; }

    for(const auto& platform : platforms){
        if(platform.colliderType == ColliderType::SOLID){
            gameMath::collisionSide side = gameMath::checkcollisionXY(b.x, b.y, platform.x, platform.y,
                                                                      b.h, b.w, platform.h * SCALE, platform.w * SCALE);
            if(side == gameMath::collisionSide::TOP){ b.velocityY = 0.0f; b.isGrounded = true; }
            if(side == gameMath::collisionSide::BOTTOM) b.velocityY = 0.0f;
        } else if(platform.colliderType == ColliderType::ONE_WAY){
            const float previousBottom = previousY + b.h;
            const float currentBottom = b.y + b.h;
            if(b.velocityY > 0 && previousBottom <= platform.y && currentBottom >= platform.y &&
               gameMath::checkcollisionX(b.x, b.y, platform.x, platform.y, b.h, b.w, platform.h * SCALE, platform.w * SCALE)){
                b.y = platform.y - b.h;
                b.velocityY = 0.0f;
                b.isGrounded = true;
            }
        }
    }
    for(const auto& ground : grounds){
        gameMath::collisionSide side = gameMath::checkcollisionXY(b.x, b.y, ground.x, ground.y,
                                                                  b.h, b.w, ground.h * SCALE, ground.w * SCALE);
        if(side == gameMath::collisionSide::TOP){ b.isGrounded = true; b.velocityY = 0.0f; }
        if(side == gameMath::collisionSide::BOTTOM) b.velocityY = 0.0f;
    }
    for(const auto& block : blocks){
        gameMath::collisionSide side = gameMath::checkcollisionXY(b.x, b.y, block.rect.x, block.rect.y,
                                                                  b.h, b.w, block.rect.h, block.rect.w);
        if(side == gameMath::collisionSide::TOP){ b.isGrounded = true; b.velocityY = 0.0f; }
        if(side == gameMath::collisionSide::BOTTOM) b.velocityY = 0.0f;
    }
}

// ====================================================================================================
//  damage in / damage out
// ====================================================================================================
int BossBuilder::applyPlayerDamage(const SDL_FRect& meleeHitBox, bool meleeActive,
                                   const SDL_FRect& beamHitBox, bool beamActive,
                                   float dt, int attackId, float knockbackMul){
    (void)knockbackMul;   // bosses have super armour: no knockback, no interrupted attacks
    const unsigned int now = SDL_GetTicks();
    int kills = 0;

    // Phase gate: in phase 1 hp can never drop past the phase-2 threshold until the roar has happened, so a burst of
    // damage can't skip phase 2 (and its music) by going from "above half" straight to dead.
    auto phaseGate = [](Boss& boss){
        if(boss.phase != BossPhase::PHASE_1) return;
        const BossDef* d = getBossDef(boss.type);
        if(!d) return;
        const int gateHp = (int)((float)boss.maxHp * d->phase2Threshold) - 1;
        if(boss.hp < gateHp) boss.hp = gateHp;
    };

    for(auto& b : m_bosses){
        // untouchable while waking up, roaring into phase 2, or dead
        if(b.state == BossState::DORMANT || b.state == BossState::INTRO ||
           b.state == BossState::PHASE_TRANSITION || b.state == BossState::DEAD) continue;

        const float mul = (b.state == BossState::RECOVER) ? BOSS_RECOVER_DAMAGE_MUL : 1.0f;

        if(meleeActive && b.lastMeleeAttackId != attackId &&
           gameMath::checkcollision(meleeHitBox.x, meleeHitBox.y, b.x, b.y, meleeHitBox.h, meleeHitBox.w, b.h, b.w)){
            b.lastMeleeAttackId = attackId;
            b.hp -= (int)((float)ENEMY_MELEE_DAMAGE * mul);
            phaseGate(b);
            b.isHurt = true;
            b.hurtFlashEnd = now + ENEMY_HURT_FLASH_MS;
            b.whiteFlashEnd = now + ENEMY_WHITE_FLASH_MS;

            const float hitterCx = meleeHitBox.x + meleeHitBox.w * 0.5f;
            const float knockDirX = ((b.x + b.w * 0.5f) >= hitterCx) ? 1.0f : -1.0f;
            const float ox1 = std::max(meleeHitBox.x, b.x), ox2 = std::min(meleeHitBox.x + meleeHitBox.w, b.x + b.w);
            const float oy1 = std::max(meleeHitBox.y, b.y), oy2 = std::min(meleeHitBox.y + meleeHitBox.h, b.y + b.h);
            m_hitEvents.push_back({(ox1 + ox2) * 0.5f, (oy1 + oy2) * 0.5f, knockDirX, b.hp <= 0, false});
        }

        if(beamActive &&
           gameMath::checkcollision(beamHitBox.x, beamHitBox.y, b.x, b.y, beamHitBox.h, beamHitBox.w, b.h, b.w)){
            b.beamAccum += ENEMY_BEAM_DAMAGE_PER_SEC * BOSS_BEAM_DAMAGE_MUL * mul * dt;
            const int whole = (int)b.beamAccum;
            if(whole > 0){ b.hp -= whole; b.beamAccum -= (float)whole; phaseGate(b); }
            b.isHurt = true;
            b.hurtFlashEnd = now + ENEMY_HURT_FLASH_MS;
            if(now >= b.nextBeamFx){
                b.nextBeamFx = now + 90;
                const float ox1 = std::max(beamHitBox.x, b.x), ox2 = std::min(beamHitBox.x + beamHitBox.w, b.x + b.w);
                const float oy1 = std::max(beamHitBox.y, b.y), oy2 = std::min(beamHitBox.y + beamHitBox.h, b.y + b.h);
                const bool goesRight = (beamHitBox.x + beamHitBox.w * 0.5f) < (b.x + b.w * 0.5f);
                m_hitEvents.push_back({goesRight ? ox1 : ox2, (oy1 + oy2) * 0.5f, goesRight ? -1.0f : 1.0f, b.hp <= 0, true});
            }
        }

        if(b.hp <= 0){ startDeath(b, now); kills++; }
    }
    return kills;
}

bool BossBuilder::checkAttackOnPlayer(float playerX, float playerY, float playerW, float playerH,
                                      unsigned int now, bool canDamage, gameMath::collisionSide& outSide){
    (void)now;
    const float pcx = playerX + playerW * 0.5f;

    for(auto& b : m_bosses){
        if(b.isDead() || !b.isAttackBoxActive || b.attackHasHit) continue;
        if(gameMath::checkcollision(b.attackHitBox.x, b.attackHitBox.y, playerX, playerY,
                                    b.attackHitBox.h, b.attackHitBox.w, playerH, playerW)){
            b.attackHasHit = true;                       // spent either way (a swing into an invincible player whiffs)
            if(!canDamage) continue;
            outSide = (pcx >= b.x + b.w * 0.5f) ? gameMath::collisionSide::RIGHT : gameMath::collisionSide::LEFT;   // shoved away from the boss
            return true;
        }
    }
    for(size_t i = 0; i < m_hazards.size(); ++i){
        const BossHazard& h = m_hazards[i];
        if(h.warn > 0.0f) continue;                      // still only a warning marker
        if(rectsOverlap(h.x, h.y, h.w, h.h, playerX, playerY, playerW, playerH)){
            const bool playerIsRight = pcx >= h.x + h.w * 0.5f;
            m_hazards.erase(m_hazards.begin() + (long)i);   // contact spends it, even on an invincible player
            if(!canDamage) return false;
            outSide = playerIsRight ? gameMath::collisionSide::RIGHT : gameMath::collisionSide::LEFT;
            return true;
        }
    }
    return false;
}

// ====================================================================================================
//  rendering (placeholder boxes until boss sprites exist - same convention as the enemies)
// ====================================================================================================
void BossBuilder::render(SDL_Renderer* renderer){
    const int camX = (int)std::round(Camera::getInstance().getCamera().x);
    const int camY = (int)std::round(Camera::getInstance().getCamera().y);
    for(const auto& b : m_bosses) renderBoss(renderer, b, camX, camY);
    renderAttackPlaceholders(renderer, camX, camY);
}

void BossBuilder::renderBoss(SDL_Renderer* r, const Boss& b, int camX, int camY) const {
    const BossDef* def = getBossDef(b.type);
    if(!def) return;

    Uint8 cr = 200, cg = 200, cb = 200;
    switch(def->biome){
        case Biome::MOSS_CAVE:    cr = 70;  cg = 190; cb = 110; break;
        case Biome::STONE_CAVE:   cr = 150; cg = 150; cb = 170; break;
        case Biome::WOODEN_SHAFT: cr = 170; cg = 120; cb = 70;  break;
        case Biome::PLAINS:       cr = 210; cg = 200; cb = 80;  break;
        case Biome::WOODS:        cr = 40;  cg = 130; cb = 60;  break;
        case Biome::MOUNTAIN:     cr = 110; cg = 150; cb = 210; break;
        case Biome::PEAK:         cr = 220; cg = 80;  cb = 80;  break;
        default: break;
    }
    if(def->tier == BossTier::MINI){ cr = (Uint8)(cr * 0.8f); cg = (Uint8)(cg * 0.8f); cb = (Uint8)(cb * 0.8f); }

    const unsigned int now = SDL_GetTicks();
    Uint8 a = 230;
    if(b.isDead()){ cr = cg = cb = 90; a = 120; }
    else if(b.isHurt || now < b.whiteFlashEnd){ cr = 255; cg = 130; cb = 130; }
    else if(b.state == BossState::RECOVER){ cr = (Uint8)std::min(255, cr + 40); cg = (Uint8)std::min(255, cg + 40); cb = (Uint8)std::min(255, cb + 40); }

    SDL_BlendMode prev = SDL_BLENDMODE_NONE;
    SDL_GetRenderDrawBlendMode(r, &prev);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);

    // crouch during the wind-up so the tell reads even without art
    const float squash = (b.state == BossState::TELEGRAPH) ? 10.0f : 0.0f;
    SDL_FRect body{ b.x - camX, b.y - camY + squash, b.w, b.h - squash };
    SDL_SetRenderDrawColor(r, cr, cg, cb, a);
    SDL_RenderFillRect(r, &body);

    // outline: white normally, red once enraged
    if(b.phase == BossPhase::PHASE_2 && !b.isDead()) SDL_SetRenderDrawColor(r, 255, 40, 40, 255);
    else                                              SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    SDL_RenderRect(r, &body);
    SDL_FRect inner{ body.x + 3, body.y + 3, body.w - 6, body.h - 6 };
    if(def->tier == BossTier::MAIN) SDL_RenderRect(r, &inner);      // mains get a double outline

    // eye + facing nub
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    SDL_FRect eye{ b.isFacingRight ? body.x + body.w - 34.0f : body.x + 12.0f, body.y + body.h * 0.18f, 22.0f, 14.0f };
    SDL_RenderFillRect(r, &eye);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_FRect pupil{ eye.x + (b.isFacingRight ? 12.0f : 2.0f), eye.y + 3.0f, 8.0f, 8.0f };
    SDL_RenderFillRect(r, &pupil);

    SDL_SetRenderDrawBlendMode(r, prev);
}

void BossBuilder::renderAttackPlaceholders(SDL_Renderer* r, int camX, int camY) const {
    SDL_BlendMode prev = SDL_BLENDMODE_NONE;
    SDL_GetRenderDrawBlendMode(r, &prev);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    const unsigned int now = SDL_GetTicks();

    for(const auto& b : m_bosses){
        if(b.isDead() || b.attack < 0) continue;
        const BossDef* def = getBossDef(b.type);
        if(!def) continue;
        const BossAttackKind kind = def->attacks[b.attack].kind;
        const bool telegraph = b.state == BossState::TELEGRAPH;
        if(!telegraph && !b.isAttackBoxActive) continue;
        if(b.attackHitBox.w <= 0.0f || b.attackHitBox.h <= 0.0f) continue;

        Uint8 cr = 255, cg = 255, cb = 255;
        switch(kind){
            case K::CLAW_SWIPE:   cr = 255; cg = 160; cb = 40;  break;
            case K::BITE_SNAP:    cr = 255; cg = 60;  cb = 60;  break;
            case K::CHARGE:       cr = 255; cg = 230; cb = 60;  break;
            case K::LEAP_SLAM:    cr = 255; cg = 100; cb = 180; break;
            case K::BREATH_BEAM:  cr = 60;  cg = 200; cb = 255; break;
            case K::AERIAL_DIVE:  cr = 255; cg = 230; cb = 60;  break;
            case K::TAIL_SWEEP:   cr = 255; cg = 120; cb = 40;  break;
            default: break;
        }
        SDL_FRect box{ b.attackHitBox.x - camX, b.attackHitBox.y - camY, b.attackHitBox.w, b.attackHitBox.h };
        if(telegraph){
            const bool blink = ((now / 90) % 2) == 0;                       // flashing outline = "this is coming"
            SDL_SetRenderDrawColor(r, cr, cg, cb, blink ? 200 : 90);
            SDL_RenderRect(r, &box);
        } else {
            SDL_SetRenderDrawColor(r, cr, cg, cb, 150);
            SDL_RenderFillRect(r, &box);
            SDL_SetRenderDrawColor(r, cr, cg, cb, 255);
            SDL_RenderRect(r, &box);
        }
    }

    for(const auto& h : m_hazards){
        if(h.warn > 0.0f){
            // rock shadow: a column from the rock down to the floor that blinks faster as the rock is about to drop
            const bool blink = ((now / (h.warn < 0.3f ? 60 : 120)) % 2) == 0;
            SDL_SetRenderDrawColor(r, 255, 70, 70, blink ? 90 : 40);
            SDL_FRect col{ h.x - camX, h.y - camY, h.w, std::max(0.0f, m_floorY - h.y) };
            SDL_RenderFillRect(r, &col);
            SDL_SetRenderDrawColor(r, 255, 70, 70, 220);
            SDL_FRect rock{ h.x - camX, h.y - camY, h.w, h.h };
            SDL_RenderRect(r, &rock);
            continue;
        }
        SDL_FRect box{ h.x - camX, h.y - camY, h.w, h.h };
        switch(h.source){
            case K::ROCK_RAIN:  SDL_SetRenderDrawColor(r, 150, 110, 70, 230);  break;
            case K::SHOCKWAVE:  SDL_SetRenderDrawColor(r, 255, 190, 60, 220);  break;
            case K::PROJECTILE_FAN: SDL_SetRenderDrawColor(r, 120, 255, 140, 230); break;
            default:            SDL_SetRenderDrawColor(r, 190, 100, 255, 230); break;
        }
        SDL_RenderFillRect(r, &box);
        SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
        SDL_RenderRect(r, &box);
    }
    SDL_SetRenderDrawBlendMode(r, prev);
}

// Boss health bar, screen space. The name is exposed through getHudInfo() so the HUD can draw it with its own font.
// The tick at 50% marks where phase 2 starts.
void BossBuilder::renderHud(SDL_Renderer* renderer) const {
    const BossHudInfo info = getHudInfo();
    if(!info.active) return;

    // the game renders in a fixed logical 1600x720 space (see Engine), not window pixels
    const float sw = Camera::getInstance().getCamera().w;
    const float barW = std::min(sw * 0.6f, 720.0f);
    const float barH = info.tier == BossTier::MAIN ? 22.0f : 16.0f;
    const float x = (sw - barW) * 0.5f, y = 26.0f;

    SDL_BlendMode prev = SDL_BLENDMODE_NONE;
    SDL_GetRenderDrawBlendMode(renderer, &prev);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    SDL_FRect back{ x - 3, y - 3, barW + 6, barH + 6 };
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 170);
    SDL_RenderFillRect(renderer, &back);

    SDL_FRect fill{ x, y, barW * info.hpFraction, barH };
    if(info.phase == BossPhase::PHASE_2) SDL_SetRenderDrawColor(renderer, 220, 50, 40, 255);
    else                                  SDL_SetRenderDrawColor(renderer, 200, 160, 40, 255);
    SDL_RenderFillRect(renderer, &fill);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderRect(renderer, &back);
    SDL_RenderLine(renderer, x + barW * 0.5f, y - 3, x + barW * 0.5f, y + barH + 3);   // phase-2 marker

    SDL_SetRenderDrawBlendMode(renderer, prev);
}
