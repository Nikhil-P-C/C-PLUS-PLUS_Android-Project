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
    constexpr unsigned int P1 = 1, P2 = 2, BOTH = 3, P3 = 4, ALL = 7;
    constexpr unsigned int PG = BOSS_C_PLAYER_GROUND, PA = BOSS_C_PLAYER_AIR, FL = BOSS_C_ON_FLOOR,
            CE = BOSS_C_ON_CEILING, WA = BOSS_C_ON_WALL, RE = BOSS_C_REACTIVE;
    using K = BossAttackKind;

    const BossDef kDefs[] = {
            // ------------------------------------------------------------------ MOSS CAVE
            // Luminous Wyrm: burrows under the floor like a train in a tunnel, comes up under the player, the head stays
            // out for a moment (that is your opening: extra damage) and spits 4-5 acid balls to both sides. No phase 2.
            { BossType::MOSS_LUMINOUS_WYRM, Biome::MOSS_CAVE, BossTier::MINI, "Luminous Wyrm",
                    420, 330, 130, 200, combatType::GROUND, EnemyType::MOSS_GLOW_WORM, 700, 900, 900, 0.0f, 1.0f, 3, {
                                                                                                                             { K::BURROW_AMBUSH, "Burrow Ambush",  600, 12000, 700,   0, 700, 5, 0, 0, 5000, BOTH },
                                                                                                                             { K::CHARGE,        "Train Rush",     700,  1400, 900, 1100, 760, 0, 250, 0, 3500, BOTH },
                                                                                                                             { K::ERUPT,         "Spine Burst",    800,  1500, 900,   0,   0, 3, 0, 0, 4500, BOTH },
                                                                                                                     }},
            // Cave Spider: shoots webs, crawls up walls and across the ceiling, sticks there and throws acid, drops with a stomp.
            // Phase 2: much faster.
            { BossType::MOSS_CAVE_SPIDER, Biome::MOSS_CAVE, BossTier::MAIN, "Cave Spider",
                    800, 230, 130, 270, combatType::GROUND, EnemyType::SPIDER_CHILD, 900, 1100, 850, 0.5f, 1.0f, 8, {
                                                                                                                             { K::PROJECTILE_ARC, "Web Shot",       500,  800, 800,   0, 720, 3, 250, 0, 2600, BOTH },
                                                                                                                             { K::SURFACE_MOVE,   "Climb Ceiling",  350, 4000, 300,   0, 650, 0, 0, 0, 7000, BOTH, FL | WA, false, 1 },
                                                                                                                             { K::SURFACE_MOVE,   "Wall Crawl",     350, 3000, 300,   0, 650, 0, 0, 0, 6000, BOTH, FL | CE, false, 2 },
                                                                                                                             { K::PROJECTILE_ARC, "Acid Throw",     550,  900, 900,   0, 660, 4, 0, 0, 3000, BOTH, CE | WA },
                                                                                                                             { K::LEAP_SLAM,      "Drop Stomp",     700, 2200, 1000,  0, 780, 1, 0, 0, 3800, BOTH, CE | WA },
                                                                                                                             { K::LEAP_SLAM,      "Pounce Stomp",   650, 2200, 900,   0, 760, 1, 200, 0, 4200, BOTH, FL },
                                                                                                                             { K::BITE_SNAP,      "Fang Bite",      380,  220, 600, 170, 260, 0, 0, 200, 1500, BOTH, FL },
                                                                                                                             { K::PROJECTILE_FAN, "Web Barrage",    650,  300, 900,   0, 560, 5, 0, 0, 5000, P2 },
                                                                                                                     }, BossBehavior::NONE, 1.5f },
            // ------------------------------------------------------------------ STONE CAVE
            // Giant Rat: faster than the player. Dash, and a grab-and-bite: get close and it snatches you.
            { BossType::STONE_GIANT_RAT, Biome::STONE_CAVE, BossTier::MINI, "Giant Rat",
                    380, 170, 110, 520, combatType::GROUND, EnemyType::CAVE_RAT, 700, 900, 650, 0.0f, 1.0f, 3, {
                                                                                                                             { K::CHARGE,     "Gnaw Dash",     350, 800, 500, 700, 1150, 0, 200, 0, 2200, BOTH },
                                                                                                                             { K::GRAB_BITE,  "Snatch & Bite", 420, 350, 900, 190,  700, 0, 0, 260, 3500, BOTH },
                                                                                                                             { K::CLAW_SWIPE, "Claw Rake",     300, 420, 450, 150,  300, 2, 0, 200, 1500, BOTH },
                                                                                                                     }},
            // Ancient Sinner: a spinosaur three times your height. Phase 2 = the First Berserker (overdrive: faster, thicker
            // beam, flurries). When its health is gone it does NOT die: it lunges at you in a parry QTE - parry 3 strikes.
            { BossType::STONE_ANCIENT_SINNER, Biome::STONE_CAVE, BossTier::MAIN, "Ancient Sinner",
                    1100, 250, 270, 150, combatType::GROUND, EnemyType::SINNERS_CHILD, 900, 1100, 1000, 0.5f, 1.0f, 4, {
                                                                                                                             { K::CLAW_SWIPE, "Penance Slash",   520, 300, 700, 300, 200, 1, 0, 340, 1400, P1 },
                                                                                                                             { K::CLAW_SWIPE, "Berserk Flurry",  380, 700, 600, 320, 350, 3, 0, 360, 2200, P2 },
                                                                                                                             { K::SHOCKWAVE,  "Hand Slam",       800, 1200, 900,  0, 560, 1, 0, 0, 4000, BOTH },
                                                                                                                             { K::BEAM_DIAG,  "Spino Beam",      900, 1500, 1000, 1800, 0, 0, 0, 0, 6000, BOTH, PG },
                                                                                                                     }, BossBehavior::NONE, 1.5f, 1.0f, true },
            // ------------------------------------------------------------------ WOODEN SHAFT MINE
            // Shaft Warden: slow and heavy. Slams and stomps when you are on the ground, jumps up at you when you are in the air.
            { BossType::SHAFT_WARDEN, Biome::WOODEN_SHAFT, BossTier::MINI, "Shaft Warden",
                    450, 200, 170, 110, combatType::GROUND, EnemyType::SHAFTS_RAPTOR, 700, 900, 1000, 0.0f, 1.0f, 3, {
                                                                                                                             { K::CLAW_SWIPE, "Heavy Slam",     750,  350, 950, 230, 160, 1, 0, 260, 2200, BOTH, PG },
                                                                                                                             { K::SHOCKWAVE,  "Stomp",          800, 1000, 1000, 0, 460, 1, 0, 0, 4200, BOTH, PG },
                                                                                                                             { K::LEAP_SLAM,  "Pouncing Jump",  800, 2200, 1100, 0, 520, 1, 0, 0, 3500, BOTH, PA },
                                                                                                                     }},
            // The Brontos: three brontosaurs' necks. Only the HEADS take damage. Heads slam one at a time, a neck sweeps the
            // floor, a head darts in and grabs. Phase 2: two heads attack at once. (Logic: updateBrontos)
            { BossType::SHAFT_BRONTOS, Biome::WOODEN_SHAFT, BossTier::MAIN, "The Brontos",
                    1200, 300, 260, 0, combatType::GROUND, EnemyType::SHAFTS_WORKER, 900, 1100, 1100, 0.5f, 1.0f, 0, {},
                                                                                                                        BossBehavior::BRONTOS },
            // ------------------------------------------------------------------ PLAINS
            { BossType::PLAINS_ANKYLOSAUR, Biome::PLAINS, BossTier::MINI, "Ankylosaur",
                    500, 250, 140, 100, combatType::GROUND, EnemyType::ANKYLOSAURUS_CHILD, 700, 900, 1000, 0.0f, 1.0f, 3, {
                                                                                                                             { K::TAIL_SWEEP, "Club Smash",    800,  450, 1000, 260,   0, 0, 0, 280, 2400, BOTH, PG },
                                                                                                                             { K::SHOCKWAVE,  "Armoured Stomp", 800, 1000, 1000,  0, 440, 1, 0, 0, 4200, BOTH, PG },
                                                                                                                             { K::LEAP_SLAM,  "Heavy Jump",    850, 2200, 1100,  0, 500, 1, 0, 0, 3600, BOTH, PA },
                                                                                                                     }},
            // Sentinel Para: a parasaur who'd rather not fight. Fast, and every strike of hers can be PARRIED (slash just
            // before it lands): the attack is cancelled and she is left open. Phase 2: faster, chains attacks at random.
            { BossType::PLAINS_SENTINEL_PARA, Biome::PLAINS, BossTier::MAIN, "Sentinel Para",
                    900, 170, 195, 360, combatType::GROUND, EnemyType::HUNTER_RAPTOR, 900, 1100, 800, 0.5f, 1.0f, 5, {
                                                                                                                             { K::CLAW_SWIPE,      "Blade Flurry",  300,  520, 500, 220, 500, 3, 0, 260, 1500, BOTH, 0, true },
                                                                                                                             { K::CHARGE,          "Sentinel Dash", 380,  500, 500, 800, 1300, 0, 300, 0, 2800, BOTH, 0, true },
                                                                                                                             { K::BEAM_DIAG,       "Duty Beam",     700, 1300, 900, 1800, 0, 0, 0, 0, 5500, BOTH, PG, true },
                                                                                                                             { K::TELEPORT_POUNCE, "Blink Pounce",  600, 4000, 1000, 0, 0, 1, 0, 0, 4500, BOTH },
                                                                                                                             { K::TELEPORT_POUNCE, "Blink Barrage", 500, 6000, 1000, 0, 0, 2, 0, 0, 8000, P2 },
                                                                                                                     }, BossBehavior::NONE, 1.45f, 1.0f, false, false, 0, 0.4f },
            // ------------------------------------------------------------------ WOODS (my own designs: "do something unique")
            // Thorn Matriarch: a rooted carnivorous plant. It cannot move: thorn columns burst under you, spore clouds linger,
            // a long vine lashes. Every big attack leaves the bulb open (extra damage).
            { BossType::WOODS_THORN_MATRIARCH, Biome::WOODS, BossTier::MINI, "Thorn Matriarch",
                    420, 200, 240, 0, combatType::GROUND, EnemyType::PREDATOR_CAT, 700, 900, 900, 0.0f, 1.0f, 3, {
                                                                                                                             { K::ERUPT,       "Thorn Snare",  700, 1600, 1300, 0, 0, 3, 0, 0, 4000, BOTH },
                                                                                                                             { K::SPORE_CLOUD, "Spore Cloud",  800,  600, 1300, 0, 0, 2, 0, 0, 7000, BOTH },
                                                                                                                             { K::CLAW_SWIPE,  "Vine Lash",    600,  300, 900, 360, 0, 1, 0, 380, 1800, BOTH },
                                                                                                                     }},
            // Treehorn Elder: an ancient moss-and-bark horned beast. Horn charge (stuns on the wall), roots erupting in a line,
            // rolling logs (jump them), seed volleys; phase 2 drops branches from the canopy and gets faster.
            { BossType::WOODS_TREEHORN_ELDER, Biome::WOODS, BossTier::MAIN, "Treehorn Elder",
                    900, 270, 230, 170, combatType::GROUND, EnemyType::THEROPOD_CHILD, 900, 1100, 1000, 0.5f, 1.0f, 5, {
                                                                                                                             { K::CHARGE,         "Horn Charge",     800, 1200, 1000, 1200, 760, 0, 300, 0, 4500, BOTH },
                                                                                                                             { K::ERUPT,          "Root Eruption",   800, 1800, 1000, 0, 0, 5, 0, 0, 5000, BOTH, 0, false, 1 },
                                                                                                                             { K::SHOCKWAVE,      "Log Roll",        700, 1600, 900, 70, 460, 2, 0, 0, 4500, BOTH },
                                                                                                                             { K::PROJECTILE_FAN, "Seed Barrage",    650,  300, 900, 0, 520, 5, 0, 0, 3300, BOTH },
                                                                                                                             { K::ROCK_RAIN,      "Falling Branches", 900, 1600, 1000, 0, 0, 7, 0, 0, 8000, P2 },
                                                                                                                     }},
            // ------------------------------------------------------------------ MOUNTAIN
            // Cat Queen: agile. Sword-like claws, a high pounce, a bite - and she dodges: swing at her up close and she dashes
            // away (i-frames) and snaps back with a bite.
            { BossType::MOUNTAIN_CAT_QUEEN, Biome::MOUNTAIN, BossTier::MINI, "Cat Queen",
                    400, 170, 105, 430, combatType::GROUND, EnemyType::PREDATOR_CAT, 700, 900, 700, 0.0f, 1.0f, 4, {
                                                                                                                             { K::CLAW_SWIPE, "Sword Claws",  280,  450, 400, 200, 520, 2, 0, 220, 1200, BOTH },
                                                                                                                             { K::LEAP_SLAM,  "High Pounce",  650, 2600, 800, 0, 650, 0, 220, 0, 3200, BOTH, 0, false, 1 },
                                                                                                                             { K::BITE_SNAP,  "Savage Bite",  220,  200, 500, 160, 380, 0, 0, 190, 1400, BOTH },
                                                                                                                             { K::DODGE_DASH, "Evade",          0,  260, 100, 330, 1500, 0, 0, 0, 1800, BOTH, RE },
                                                                                                                     }},
            // Brother: the Preacher's brother, a tarbosaurus. Tail whip, flame breath, jump stomp, jaws. Phase 2: faster and
            // tankier (takes 35% less damage).
            { BossType::MOUNTAIN_BROTHER, Biome::MOUNTAIN, BossTier::MAIN, "Brother",
                    1000, 270, 270, 170, combatType::GROUND, EnemyType::TARBOSAURUS_CHILD, 900, 1100, 1000, 0.5f, 1.0f, 4, {
                                                                                                                             { K::TAIL_SWEEP,   "Tail Whip",    650,  450, 850, 330, 0, 0, 0, 340, 2600, BOTH },
                                                                                                                             { K::FLAMETHROWER, "Flame Breath", 900, 1700, 1100, 650, 0, 0, 150, 0, 6000, BOTH },
                                                                                                                             { K::LEAP_SLAM,    "Jump Stomp",   750, 2200, 900, 0, 700, 2, 200, 0, 4200, BOTH },
                                                                                                                             { K::BITE_SNAP,    "Jaw Clamp",    450,  240, 700, 280, 280, 0, 0, 290, 1500, BOTH },
                                                                                                                     }, BossBehavior::NONE, 1.4f, 0.65f },
            // ------------------------------------------------------------------ PEAK
            { BossType::PEAK_DASPLETO_WARLORD, Biome::PEAK, BossTier::MINI, "Dasplet Warlord",
                    380, 200, 210, 220, combatType::GROUND, EnemyType::DASPELTOSAURUS, 700, 900, 850, 0.5f, 1.0f, 4, {
                                                                                                                             { K::BITE_SNAP,      "Warlord Bite",     420, 230, 550, 220, 300, 0, 0, 240, 1400, BOTH },
                                                                                                                             { K::CHARGE,         "Warhorn Rush",     650, 1000, 850, 1000, 720, 0, 300, 0, 3500, BOTH },
                                                                                                                             { K::TAIL_SWEEP,     "Spiked Sweep",     550, 420, 700, 230,   0, 0, 0, 260, 2400, BOTH },
                                                                                                                             { K::PROJECTILE_FAN, "Shard Volley",     650, 300, 850,   0, 600, 5, 0, 0, 3200, P2 },
                                                                                                                     }},
            // The Preacher (final boss). Phase 1/2: Sentinel's blades + blink, the Sinner's beam + slam, the Brother's whip,
            // flame and stomp. Phase 2 = Second Berserker (overdrive). At 0 hp he "dies"... then a god ray lifts him back up with
            // angelic wings and he flees to the high platform. Phase 3: the sinner children's ghosts let you pogo up to him;
            // he conjures hovering orbs and light columns. Only his body takes damage, never the wings.
            { BossType::PEAK_THE_PREACHER, Biome::PEAK, BossTier::MAIN, "The Preacher",
                    1500, 180, 250, 330, combatType::GROUND, EnemyType::SINNERS_CHILD, 1000, 1200, 800, 0.5f, 1.0f, 11, {
                                                                                                                             { K::CLAW_SWIPE,      "Blade Flurry",   320,  520, 500, 230, 480, 3, 0, 270, 1600, BOTH, 0, true },
                                                                                                                             { K::TELEPORT_POUNCE, "Blink Pounce",   600, 4000, 1000, 0, 0, 1, 0, 0, 5000, BOTH },
                                                                                                                             { K::BEAM_DIAG,       "Sinner's Beam",  850, 1500, 1000, 1800, 0, 0, 0, 0, 6500, BOTH, PG, true },
                                                                                                                             { K::SHOCKWAVE,       "Hand Slam",      750, 1200, 900, 0, 540, 1, 0, 0, 4200, BOTH },
                                                                                                                             { K::TAIL_SWEEP,      "Brother's Whip", 650,  450, 850, 320, 0, 0, 0, 330, 2800, BOTH },
                                                                                                                             { K::FLAMETHROWER,    "Brother's Flame", 900, 1700, 1100, 650, 0, 0, 150, 0, 6500, BOTH },
                                                                                                                             { K::LEAP_SLAM,       "Jump Stomp",     750, 2200, 900, 0, 700, 2, 200, 0, 4500, BOTH },
                                                                                                                             { K::ORB_SUMMON,      "Heavenly Orbs",  900, 1800, 1200, 0, 380, 5, 0, 0, 6500, P3 },
                                                                                                                             { K::SMITE,           "Smite",          900, 1800, 1000, 0, 0, 5, 0, 0, 5500, P3 },
                                                                                                                             { K::BEAM_DIAG,       "Divine Ray",     900, 1500, 1100, 1800, 0, 0, 0, 0, 6500, P3 },
                                                                                                                             { K::PROJECTILE_FAN,  "Feather Volley", 650,  300, 900, 0, 560, 6, 0, 0, 3500, P3 },
                                                                                                                     }, BossBehavior::PREACHER, 1.5f, 1.0f, false, true, 700, 0.35f },
    };

    struct BossToken { const char* name; BossType type; };
    const BossToken kTokens[] = {
            {"MOSS_LUMINOUS_WYRM", BossType::MOSS_LUMINOUS_WYRM},       {"MOSS_CAVE_SPIDER", BossType::MOSS_CAVE_SPIDER},
            {"STONE_GIANT_RAT", BossType::STONE_GIANT_RAT},             {"STONE_ANCIENT_SINNER", BossType::STONE_ANCIENT_SINNER},
            {"SHAFT_WARDEN", BossType::SHAFT_WARDEN},                   {"SHAFT_BRONTOS", BossType::SHAFT_BRONTOS},
            {"PLAINS_ANKYLOSAUR", BossType::PLAINS_ANKYLOSAUR},         {"PLAINS_SENTINEL_PARA", BossType::PLAINS_SENTINEL_PARA},
            {"WOODS_THORN_MATRIARCH", BossType::WOODS_THORN_MATRIARCH}, {"WOODS_TREEHORN_ELDER", BossType::WOODS_TREEHORN_ELDER},
            {"MOUNTAIN_CAT_QUEEN", BossType::MOUNTAIN_CAT_QUEEN},       {"MOUNTAIN_BROTHER", BossType::MOUNTAIN_BROTHER},
            {"PEAK_DASPLETO_WARLORD", BossType::PEAK_DASPLETO_WARLORD}, {"PEAK_THE_PREACHER", BossType::PEAK_THE_PREACHER},
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
           (phase == BossPhase::PHASE_1 ? "_phase1.ogg" : phase == BossPhase::PHASE_2 ? "_phase2.ogg" : "_phase3.ogg");
}

Boss::Boss(float sx, float sy, BossType t) : type(t) {
    x = baseX = sx;
    y = baseY = sy;
    if(const BossDef* d = getBossDef(t)){
        w = baseW = d->w; h = baseH = d->h;
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
    m_parryEvents.clear();
    m_ghosts.clear();
    m_grab = Grab{};
    m_pendingBite = false;
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
    endAttackCleanup(b, SDL_FRect{0, 0, 0, 0});
    endGrab();
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
    m_ghosts.clear();
    m_grab = Grab{};
    m_pendingBite = false;
    b.beamOn = b.beamAim = b.markerOn = false;
    b.hidden = b.untargetable = b.exposed = false;
    b.noPhysics = false;
    b.surface = Boss::Surface::FLOOR;
    b.w = b.winged ? b.w : b.baseW; b.h = b.baseH;
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
    info.hasPhase2 = def->phase2Threshold > 0.0f;
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

    if(b.type == BossType::SHAFT_BRONTOS && b.parts.empty()){
        // the body stands at the right end of the arena; three heads hang from it on long necks
        const float floorY = wall.y + std::ceil(wall.h / wallT) * wallT - wallT;
        const float ceilY = wall.y + wallT;
        b.noPhysics = true;
        b.x = b.baseX = b.arenaR - b.w;
        b.y = floorY - b.h;
        for(int i = 0; i < 3; ++i){
            BossPart p;
            p.maxHp = p.hp = b.maxHp / 3;
            p.anchorX = b.arenaR - 50.0f;
            p.anchorY = floorY - b.h + 40.0f + (float)i * 60.0f;
            p.homeX = b.arenaL + 380.0f + (float)i * 330.0f;
            p.homeY = ceilY + 110.0f + (float)(i % 2) * 70.0f;
            p.rect = { p.homeX, p.homeY, 130.0f, 100.0f };
            p.swayPhase = (float)i * 2.1f;
            b.parts.push_back(p);
        }
    }
}

// Boss tracks never leave the game silent:
//   phase 1: boss file -> (missing) the level music simply keeps playing
//   phase 2: boss file -> (missing) phase 1 keeps playing
void BossBuilder::playBossMusic(const Boss& b, const BossDef& def, BossPhase phase){
    AssetManager& am = Engine::Get().getAssetManager();
    auto pathFor = [&](BossPhase ph){
        const std::string& o = (ph == BossPhase::PHASE_1) ? b.musicPhase1 : (ph == BossPhase::PHASE_2) ? b.musicPhase2 : b.musicPhase3;
        return o.empty() ? getBossMusicPath(def, ph) : o;
    };
    const float gain = b.musicGain >= 0.0f ? b.musicGain : def.musicGain;
    if(am.tryPlayMusicFile(pathFor(phase), gain)) return;
    // missing track: fall back phase 3 -> 2 -> 1, and finally the level music just keeps playing
    if(phase == BossPhase::PHASE_3 && am.tryPlayMusicFile(pathFor(BossPhase::PHASE_2), gain)) return;
    if(phase != BossPhase::PHASE_1) am.tryPlayMusicFile(pathFor(BossPhase::PHASE_1), gain);
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
        m_bosses[i] = m_template[i];                       // (arena + Brontos heads are rebuilt by setupArena)
    }
    m_grab = Grab{};
    m_pendingBite = false;
    m_ghosts.clear();
    m_parryEvents.clear();
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

    m_playerRect = player;
    if(m_grab.active && now - m_grab.start >= BOSS_GRAB_MS){      // held too long: the final bite lands
        m_pendingBite = true;
        m_pendingBiteUntil = now + 900;
        endGrab();
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
    m_ceilY = wall.y + TILE_SIZE * SCALE;

    const float pcx = player.x + player.w * 0.5f, pcy = player.y + player.h * 0.5f;
    const float bcx = b.x + b.w * 0.5f;
    const bool  p2  = b.phase != BossPhase::PHASE_1;
    const float spd = def->moveSpeed * phaseSpeedMul(b);
    const unsigned int elapsed = now - b.stateStart;
    const bool brontos = def->behavior == BossBehavior::BRONTOS;
    const int myIdx = (int)(&b - m_bosses.data());

    // held in a grab (rat / dino): the boss stays put until the grab is over
    if(m_grab.active && m_grab.boss == myIdx && m_grab.part < 0 && !b.isDead()){
        faceToward(b, pcx);
        return;
    }

    switch(b.state){
        case BossState::DORMANT: {
            if(m_activeIdx >= 0) break;     // one fight at a time
            // the fight starts once the player is properly inside the arena (BOSS_ARENA_ENTER_MARGIN from both ends)
            const bool inside = player.x >= b.arenaL + BOSS_ARENA_ENTER_MARGIN &&
                                player.x + player.w <= b.arenaR - BOSS_ARENA_ENTER_MARGIN &&
                                player.y + player.h > b.arena.y && player.y < b.arena.y + b.arena.h;
            if(inside) beginFight(myIdx, now);
            break;
        }

        case BossState::INTRO:
            if(!brontos) faceToward(b, pcx);
            Camera::getInstance().rumble(0.25f);
            if(brontos) updateBrontos(b, *def, dt, player, wall, now);
            if(elapsed >= 1500){
                setState(b, BossState::IDLE, now);
                b.nextAttackTime = now + 700;
            }
            break;

        case BossState::IDLE: {
            if(!brontos) faceToward(b, pcx);
            if(b.phase == BossPhase::PHASE_1 && def->phase2Threshold > 0.0f &&
               (float)b.hp <= (float)b.maxHp * def->phase2Threshold){
                beginPhaseTransition(b, now);
                break;
            }
            if(brontos){ updateBrontos(b, *def, dt, player, wall, now); break; }

            // reactive dodge (Cat Queen): a player swing right next to her makes her dash away and counter
            for(int i = 0; i < def->attackCount; ++i){
                const BossAttackDef& d = def->attacks[i];
                if(d.kind != K::DODGE_DASH || now < b.attackReadyAt[i]) continue;
                if(m_info.attacking && horizontalGap(b, player) < 230.0f){
                    b.attack = i;
                    beginAttack(b, now);
                    b.afterDodge = true;
                    b.forcedAttack = -1;
                    for(int j = 0; j < def->attackCount; ++j)
                        if(def->attacks[j].kind == K::BITE_SNAP){ b.forcedAttack = j; break; }
                }
                break;
            }
            if(b.state != BossState::IDLE) break;

            // --- movement between attacks ---
            if(spd > 0.0f){
                if(b.combat == combatType::AERIAL){
                    if(def->behavior == BossBehavior::PREACHER){   // phase 3: he stays up on his perch
                        hoverToward(b, b.arenaR - b.w - 40.0f, b.baseY + std::sin(now * 0.002f) * 16.0f, spd * 0.6f, dt, wall);
                    } else {
                        hoverToward(b, pcx - b.w * 0.5f, b.baseY + std::sin(now * 0.003f) * 14.0f, spd * 0.8f, dt, wall);
                    }
                } else if(b.surface == Boss::Surface::CEILING){
                    const float step = spd * dt;
                    b.x = std::clamp(b.x + std::clamp(pcx - b.w * 0.5f - b.x, -step, step), b.arenaL, std::max(b.arenaL, b.arenaR - b.w));
                    b.y = m_ceilY + 2.0f;
                } else if(b.surface == Boss::Surface::WALL){
                    const float step = spd * 0.7f * dt;
                    b.y = std::clamp(b.y + std::clamp(pcy - b.h * 0.5f - b.y, -step, step), m_ceilY + 4.0f, std::max(m_ceilY + 5.0f, m_floorY - b.h - 4.0f));
                } else {
                    const float gap = horizontalGap(b, player);
                    const float stand = b.w * 0.25f;                       // close in to this gap, never walk into the player
                    if(gap > stand){
                        const float step = std::min(spd * dt, gap - stand);
                        tryMoveX(b, b.x + ((pcx > bcx) ? step : -step), wall, blocks);
                    }
                }
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
            if(a.kind != K::TELEPORT_POUNCE && elapsed < windup * 0.6f) faceToward(b, pcx);   // tracks early, then locks so the tell is fair
            if(b.combat == combatType::AERIAL){
                if(def->behavior == BossBehavior::PREACHER) hoverToward(b, b.x, b.baseY, spd * 0.4f, dt, wall);
                else hoverToward(b, pcx - b.w * 0.5f, b.baseY, spd * 0.5f, dt, wall);
            }
            if(a.kind == K::BEAM_DIAG){
                b.beamAim = true;
                if(elapsed < windup * 0.65f) aimBeam(b, player, a);          // the beam locks on, then the tell stays put
            } else {
                b.attackHitBox = previewBox(b, a, player, wall);
            }
            b.isAttackBoxActive = false;
            if(a.parry && tryParry(b, a, player, b.stateStart + windup, now)) break;
            if(elapsed >= windup){
                setState(b, BossState::ATTACKING, now);
                b.attackHasHit = false;
                b.beamAim = false;
            }
            break;
        }

        case BossState::ATTACKING: {
            const BossAttackDef& a = def->attacks[b.attack];
            runAttack(b, *def, a, dt, player, wall, blocks, now, elapsed);
            if(a.parry && elapsed < BOSS_PARRY_POST_MS && tryParry(b, a, player, b.stateStart, now)) break;
            if(b.endEarly || elapsed >= a.activeMs){
                endAttackCleanup(b, wall);
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
            if(b.combat == combatType::AERIAL)
                hoverToward(b, b.x, b.baseY, spd * 0.7f, dt, wall);    // climb back to hover height
            if(elapsed >= rec){
                setState(b, BossState::IDLE, now);
                b.nextAttackTime = now + (unsigned int)(def->idleMs * (p2 ? BOSS_PHASE2_IDLE_MUL : 1.0f));
                if(b.afterDodge){ b.nextAttackTime = now; b.afterDodge = false; }          // the counter comes right away
                else if(p2 && def->chainP2 > 0.0f){
                    std::uniform_real_distribution<float> d(0.0f, 1.0f);
                    if(d(m_rng) < def->chainP2) b.nextAttackTime = now + 120;               // phase 2: "more random": no pause
                }
                b.attack = -1;
                b.extraRecoverMs = 0;
            }
            break;
        }

        case BossState::PHASE_TRANSITION:
            if(!brontos) faceToward(b, pcx);
            Camera::getInstance().rumble(0.45f);
            if(b.combat == combatType::AERIAL)
                hoverToward(b, b.x, b.baseY, spd * 0.5f, dt, wall);
            if(brontos) updateBrontos(b, *def, dt, player, wall, now);
            if(elapsed >= BOSS_PHASE_TRANSITION_MS){
                b.phase = BossPhase::PHASE_2;
                setState(b, BossState::IDLE, now);
                b.nextAttackTime = now + 500;
            }
            break;

        case BossState::QTE:
            faceToward(b, pcx);
            updateQte(b, player, now);
            break;

        case BossState::FALSE_DEATH:
            Camera::getInstance().rumble(0.1f);
            if(elapsed >= 3600){
                setState(b, BossState::REVIVE, now);
                b.hp = b.maxHp = std::max(1, def->hpPhase3);
                m_hazards.clear();
            }
            break;

        case BossState::REVIVE:
            updateRevive(b, wall, now);
            break;

        case BossState::DEAD:
            break;   // corpse: gravity only (below)
    }

    // ground bosses (and every corpse) fall and collide like ground enemies do
    if((b.combat == combatType::GROUND && !b.noPhysics && b.surface == Boss::Surface::FLOOR) || b.isDead())
        applyGravityAndCollision(b, dt, wall, grounds, platforms, blocks);
}

void BossBuilder::endAttackCleanup(Boss& b, const SDL_FRect& wall){
    (void)wall;
    b.isAttackBoxActive = false;
    b.attackHasHit = true;
    b.beamOn = b.beamAim = false;
    b.markerOn = false;
    b.hidden = false;
    b.exposed = false;
    b.untargetable = false;
    if(b.surface == Boss::Surface::FLOOR && b.combat == combatType::GROUND && b.type != BossType::SHAFT_BRONTOS) b.noPhysics = false;
}

float BossBuilder::phaseSpeedMul(const Boss& b) const {
    if(b.phase == BossPhase::PHASE_1) return 1.0f;
    const BossDef* d = getBossDef(b.type);
    return d ? d->p2SpeedMul : BOSS_PHASE2_SPEED_MUL;
}

// ----------------------------------------------------------------------------------------------------
//  parry: slash inside the window right before / after a parryable strike lands
// ----------------------------------------------------------------------------------------------------
bool BossBuilder::tryParry(Boss& b, const BossAttackDef& a, const SDL_FRect& player, unsigned int impactAt, unsigned int now){
    if(m_slashTick <= m_parryUsed) return false;
    if(m_slashTick + BOSS_PARRY_PRE_MS < impactAt || m_slashTick > impactAt + BOSS_PARRY_POST_MS) return false;
    const float reach = a.reach > 0.0f ? std::min(a.reach + 140.0f, 560.0f) : 560.0f;
    if(horizontalGap(b, player) > reach) return false;
    m_parryUsed = m_slashTick;
    doParry(b, now);
    return true;
}

void BossBuilder::doParry(Boss& b, unsigned int now){
    endAttackCleanup(b, SDL_FRect{0, 0, 0, 0});
    m_parryEvents.push_back({ std::clamp(m_playerRect.x + m_playerRect.w * 0.5f, b.x, b.x + b.w),
                              b.y + b.h * 0.4f, false });
    b.extraRecoverMs = 900;
    b.staggerUntil = now + 1500;
    if(b.attack >= 0){
        const BossDef* def = getBossDef(b.type);
        if(def) b.attackReadyAt[b.attack] = now + def->attacks[b.attack].cooldownMs;
    }
    setState(b, BossState::RECOVER, now);
    Camera::getInstance().addTrauma(0.45f);
}

// ----------------------------------------------------------------------------------------------------
//  Ancient Sinner finisher: parry BOSS_QTE_STEPS strikes in a row (a miss costs a hit and repeats the step)
// ----------------------------------------------------------------------------------------------------
void BossBuilder::beginQte(Boss& b, unsigned int now){
    setState(b, BossState::QTE, now);
    b.hp = 1;
    b.qteStep = 0;
    b.qteStrike = now + 1600;
    m_hazards.clear();
    b.attack = -1;
    Camera::getInstance().addTrauma(0.8f);
}

void BossBuilder::updateQte(Boss& b, const SDL_FRect& player, unsigned int now){
    Camera::getInstance().rumble(0.12f);
    if(now < b.qteStrike + BOSS_PARRY_POST_MS) return;
    const bool ok = m_slashTick > m_parryUsed && m_slashTick + 340 >= b.qteStrike && m_slashTick <= b.qteStrike + BOSS_PARRY_POST_MS;
    if(ok){
        m_parryUsed = m_slashTick;
        m_parryEvents.push_back({ (m_playerRect.x + b.x + b.w * 0.5f) * 0.5f, b.y + b.h * 0.4f, true });
        Camera::getInstance().addTrauma(0.6f);
        b.qteStep++;
        if(b.qteStep >= BOSS_QTE_STEPS){
            b.qteDone = true;
            b.hp = 0;
            startDeath(b, now);
            return;
        }
        b.qteStrike = now + 950;
    } else {
        // missed: he connects, then tries again
        b.attackHitBox = { player.x - 12.0f, player.y - 12.0f, player.w + 24.0f, player.h + 24.0f };
        b.isAttackBoxActive = true;
        b.attackHasHit = false;
        b.qteStrike = now + 1500;
        Camera::getInstance().addTrauma(0.4f);
    }
}

// ----------------------------------------------------------------------------------------------------
//  Preacher: false death -> god ray -> winged phase 3
// ----------------------------------------------------------------------------------------------------
void BossBuilder::beginFalseDeath(Boss& b, unsigned int now){
    setState(b, BossState::FALSE_DEATH, now);
    b.hp = 0;
    b.attack = -1;
    endAttackCleanup(b, SDL_FRect{0, 0, 0, 0});
    m_hazards.clear();
    Camera::getInstance().addTrauma(1.0f);
    Engine::Get().getAssetManager().fadeOutMusic(BOSS_MUSIC_FADE_MS);   // silence: it feels like the game is over
    if(b.type == BossType::PEAK_THE_PREACHER) m_deathEvents.push_back({b.x + b.w * 0.5f, b.y + b.h * 0.5f});
}

void BossBuilder::updateRevive(Boss& b, const SDL_FRect& wall, unsigned int now){
    Camera::getInstance().rumble(0.3f);
    const unsigned int el = now - b.stateStart;
    if(el < 1400) b.winged = false;
    else b.winged = true;
    if(el >= 3200) startPhase3(b, wall, now);
}

void BossBuilder::startPhase3(Boss& b, const SDL_FRect& wall, unsigned int now){
    (void)wall;
    const BossDef* def = getBossDef(b.type);
    if(!def) return;
    b.phase = BossPhase::PHASE_3;
    b.winged = true;
    b.combat = combatType::AERIAL;
    b.noPhysics = true;
    b.velocityY = 0.0f;
    const float oldW = b.w;
    b.w = b.baseW * 2.4f;                       // wing span; hurtRect() is only the body in the middle
    b.x -= (b.w - oldW) * 0.5f;
    b.x = std::clamp(b.x, b.arenaL, std::max(b.arenaL, b.arenaR - b.w));
    b.baseY = m_ceilY + 24.0f;
    // the sinner children's ghosts: pogo pads that climb toward the perch
    m_ghosts.clear();
    const float ax = b.arena.x, fy = m_floorY;
    m_ghosts.push_back({ ax + 520.0f,  fy - 190.0f, 120.0f, 34.0f });
    m_ghosts.push_back({ ax + 820.0f,  fy - 320.0f, 120.0f, 34.0f });
    m_ghosts.push_back({ ax + 1090.0f, fy - 420.0f, 120.0f, 34.0f });
    playBossMusic(b, *def, BossPhase::PHASE_3);
    setState(b, BossState::IDLE, now);
    b.nextAttackTime = now + 1800;
    Camera::getInstance().addTrauma(0.8f);
}

bool BossBuilder::checkBounce(const SDL_FRect& player, float velocityY, float& outVelocityY){
    if(m_ghosts.empty() || velocityY <= 0.0f) return false;
    const float feet = player.y + player.h;
    for(const auto& g : m_ghosts){
        if(player.x + player.w > g.x && player.x < g.x + g.w && feet >= g.y - 6.0f && feet <= g.y + 34.0f){
            outVelocityY = -BOSS_BOUNCE_VELOCITY;
            return true;
        }
    }
    return false;
}

void BossBuilder::setPlayerInfo(const BossPlayerInfo& info){
    m_info = info;
    if(info.slashStarted) m_slashTick = SDL_GetTicks();
}

// ----------------------------------------------------------------------------------------------------
//  grabs
// ----------------------------------------------------------------------------------------------------
void BossBuilder::startGrab(int bossIdx, int part, unsigned int now){
    m_grab.active = true; m_grab.boss = bossIdx; m_grab.part = part; m_grab.start = now; m_grab.struggle = 0;
    Camera::getInstance().addTrauma(0.35f);
}

void BossBuilder::endGrab(){
    if(!m_grab.active) return;
    const unsigned int now = SDL_GetTicks();
    if(m_grab.boss >= 0 && m_grab.boss < (int)m_bosses.size()){
        Boss& b = m_bosses[(size_t)m_grab.boss];
        if(m_grab.part >= 0 && m_grab.part < (int)b.parts.size()){
            BossPart& p = b.parts[(size_t)m_grab.part];
            if(p.state != 5){ p.state = 3; p.t0 = now; p.hitActive = false; }
        } else if(!b.isDead() && b.state == BossState::ATTACKING){
            endAttackCleanup(b, SDL_FRect{0, 0, 0, 0});
            b.extraRecoverMs = 500;
            setState(b, BossState::RECOVER, now);
        }
    }
    m_grab = Grab{};
}

bool BossBuilder::isPlayerGrabbed(SDL_FPoint& pos) const {
    if(!m_grab.active || m_grab.boss < 0 || m_grab.boss >= (int)m_bosses.size()) return false;
    const Boss& b = m_bosses[(size_t)m_grab.boss];
    if(m_grab.part >= 0 && m_grab.part < (int)b.parts.size()){
        const BossPart& p = b.parts[(size_t)m_grab.part];
        pos = { p.rect.x + p.rect.w * 0.5f - m_playerRect.w * 0.5f, p.rect.y + p.rect.h * 0.5f - m_playerRect.h * 0.5f };
    } else {
        const float mouthX = b.isFacingRight ? (b.x + b.w) : b.x;
        pos = { mouthX - m_playerRect.w * 0.5f, b.y + b.h * 0.15f };
    }
    return true;
}

// ----------------------------------------------------------------------------------------------------
//  spider surface paths
// ----------------------------------------------------------------------------------------------------
void BossBuilder::setSurface(Boss& b, Boss::Surface s){
    if(b.surface == s) return;
    const float cx = b.x + b.w * 0.5f, cy = b.y + b.h * 0.5f;
    b.surface = s;
    if(s == Boss::Surface::WALL){ b.w = b.baseH; b.h = b.baseW; }
    else                         { b.w = b.baseW; b.h = b.baseH; }
    b.x = cx - b.w * 0.5f;
    b.y = cy - b.h * 0.5f;
}

void BossBuilder::buildSurfacePath(Boss& b, int target, const SDL_FRect& player, const SDL_FRect& wall){
    (void)wall;
    const float floorY = m_floorY, ceilY = m_ceilY;
    const bool left = (b.x + b.w * 0.5f) < (b.arenaL + b.arenaR) * 0.5f;
    const float wallX = left ? b.arenaL : (b.arenaR - b.baseH);           // flush x for the wall dims (w' = baseH)
    const float floorX = left ? b.arenaL : (b.arenaR - b.baseW);
    const float pcx = player.x + player.w * 0.5f;
    std::uniform_real_distribution<float> dy(ceilY + 70.0f, std::max(ceilY + 71.0f, floorY - b.baseW - 70.0f));
    using S = Boss::Surface;
    int n = 0;
    auto add = [&](float x, float y, S s){ b.wpX[n] = x; b.wpY[n] = y; b.wpSurf[n] = s; ++n; };
    const S cur = b.surface;
    if(target == 1){   // ceiling
        if(cur == S::FLOOR){ add(floorX, floorY - b.baseH, S::FLOOR); add(wallX, ceilY + 4.0f, S::WALL); }
        else if(cur == S::WALL){ add(b.x, ceilY + 4.0f, S::WALL); }
        add(std::clamp(pcx - b.baseW * 0.5f, b.arenaL, std::max(b.arenaL, b.arenaR - b.baseW)), ceilY + 2.0f, S::CEILING);
    } else {           // wall
        if(cur == S::FLOOR){ add(floorX, floorY - b.baseH, S::FLOOR); }
        else if(cur == S::CEILING){ add(floorX, ceilY + 2.0f, S::CEILING); }
        add(wallX, dy(m_rng), S::WALL);
    }
    b.wpCount = n;
    b.wpIdx = 0;
}

void BossBuilder::aimBeam(Boss& b, const SDL_FRect& player, const BossAttackDef& a){
    (void)a;
    const bool winged = b.winged;
    const float ox = winged ? (b.x + b.w * 0.5f) : (b.isFacingRight ? b.x + b.w * 0.85f : b.x + b.w * 0.15f);
    const float oy = winged ? (b.y + b.h * 0.45f) : (b.y + b.h * 0.2f);
    float dx = (player.x + player.w * 0.5f) - ox, dy = (player.y + player.h * 0.5f) - oy;
    if(!winged && std::fabs(dx) < 60.0f) dx = b.isFacingRight ? 60.0f : -60.0f;   // never straight down through the boss
    const float len = std::max(1.0f, std::sqrt(dx * dx + dy * dy));
    b.beamX1 = ox; b.beamY1 = oy;
    b.beamX2 = ox + dx / len * 2200.0f;
    b.beamY2 = oy + dy / len * 2200.0f;
}

// ====================================================================================================
//  attack selection
// ====================================================================================================
int BossBuilder::pickAttack(Boss& b, const BossDef& def, const SDL_FRect& player, unsigned int now){
    const float gap = horizontalGap(b, player);
    const unsigned int bit = (b.phase == BossPhase::PHASE_3) ? P3 : (b.phase == BossPhase::PHASE_2) ? P2 : P1;
    const unsigned int surfBit = b.surface == Boss::Surface::FLOOR ? FL : b.surface == Boss::Surface::CEILING ? CE : WA;

    float weights[BOSS_MAX_ATTACKS] = {};
    for(int pass = 0; pass < 2; ++pass){
        float total = 0.0f;
        for(int i = 0; i < def.attackCount; ++i){
            const BossAttackDef& a = def.attacks[i];
            if(!(a.phaseMask & bit)) continue;
            if(now < b.attackReadyAt[i]) continue;
            if(a.cond & RE) continue;                                          // reactive moves are never scheduled
            if((a.cond & PG) && !m_info.grounded) continue;                   // needs the player on something
            if((a.cond & PA) && m_info.grounded) continue;                    // needs the player in the air
            if((a.cond & (FL | CE | WA)) && !(a.cond & surfBit)) continue;   // spider: only from certain surfaces
            const bool groundOnly = a.kind == K::CHARGE || a.kind == K::LEAP_SLAM || a.kind == K::SHOCKWAVE || a.kind == K::TAIL_SWEEP;
            if(groundOnly && b.combat != combatType::GROUND) continue;
            if(a.kind == K::AERIAL_DIVE && b.combat != combatType::AERIAL) continue;

            float w = 1.0f;
            if(a.minGap > 0.0f || a.maxGap > 0.0f){
                const bool inWindow = (a.minGap <= 0.0f || gap >= a.minGap) && (a.maxGap <= 0.0f || gap <= a.maxGap);
                w = inWindow ? 3.0f : 0.4f;
            }
            if(i == b.lastAttack) w = (b.streak >= 2 && pass == 0) ? 0.0f : w * 0.3f;   // never the same move three times running
            weights[i] = w;
            total += w;
        }
        if(total <= 0.0f){ if(pass == 0) continue; return -1; }

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
    b.stage = 0;
    b.stageT = now;
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
        case K::BITE_SNAP:
        case K::GRAB_BITE:   return front(a.reach, 0.0f, b.h * 0.5f);
        case K::BREATH_BEAM: return front(a.reach, b.h * 0.2f, b.h * 0.45f);
        case K::FLAMETHROWER:return front(a.reach, b.h * 0.45f, b.h * 0.4f);
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
    const bool  p2 = b.phase != BossPhase::PHASE_1;
    const float sp = a.speed * phaseSpeedMul(b);
    const float pcx = player.x + player.w * 0.5f, pcy = player.y + player.h * 0.5f;
    const float bcx = b.x + b.w * 0.5f,           bcy = b.y + b.h * 0.5f;
    const float dirSign = b.isFacingRight ? 1.0f : -1.0f;
    (void)bcy;
    auto bodyBox = [&](){ return SDL_FRect{ b.x - 3.0f, b.y - 3.0f, b.w + 6.0f, b.h + 6.0f }; };
    auto frontBox = [&](float len, float yOff, float hh){
        return SDL_FRect{ b.isFacingRight ? (b.x + b.w) : (b.x - len), b.y + yOff, len, hh };
    };
    const unsigned int stageEl = now - b.stageT;

    switch(a.kind){
        case K::CLAW_SWIPE:
        case K::BITE_SNAP:
        case K::GRAB_BITE: {
            // short lunge during the first half of each swing (a.count > 1 = a combo of that many swings)
            const int swings = std::max(1, a.count);
            const float interval = (float)a.activeMs / (float)swings;
            const int step = std::min(swings - 1, (int)((float)elapsed / interval));
            if(step > b.spawned){ b.spawned = step; b.attackHasHit = false; faceToward(b, pcx); }
            const float local = (float)elapsed - (float)step * interval;
            if(local < interval * 0.5f && horizontalGap(b, player) > 12.0f)
                tryMoveX(b, b.x + dirSign * sp * dt, wall, blocks);
            const bool bite = a.kind != K::CLAW_SWIPE;
            b.attackHitBox = bite ? frontBox(a.reach, 0.0f, b.h * 0.5f) : frontBox(a.reach, 0.0f, b.h);
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
                if(b.surface != Boss::Surface::FLOOR){ setSurface(b, Boss::Surface::FLOOR); }   // spider drops off the wall/ceiling
                b.noPhysics = false;
                faceToward(b, pcx);
                const float bx = b.x + b.w * 0.5f, by = b.y + b.h * 0.5f;
                const float dx = pcx - bx, dy = pcy - by;
                const float T = a.aux > 0.0f ? 0.95f : std::clamp(std::fabs(dx) / std::max(sp, 1.0f), 0.4f, 0.9f);
                b.dirX = dx / T;                                                           // horizontal launch speed
                b.velocityY = std::max(-1500.0f, (dy - 0.5f * ENEMY_GRAVITY * T * T) / T); // vertical launch (up = negative)
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
                Camera::getInstance().addTrauma(0.8f);                                      // the slam shakes the camera
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

        case K::SPIT_BOTH: {
            const int total = std::max(1, a.count);
            const float interval = (float)a.activeMs / (float)total;
            while(b.spawned < total && (float)elapsed >= (float)b.spawned * interval){
                spawnSpit(b, (b.spawned % 2 == 0) ? 1 : -1, a);
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

        case K::BREATH_BEAM:
        case K::FLAMETHROWER: {
            const bool flame = a.kind == K::FLAMETHROWER;
            if(p2) faceToward(b, pcx);                            // phase 2: it follows the player
            if(!b.launched){ b.launched = true; b.nextRearm = now + (flame ? 320 : BOSS_BEAM_REARM_MS); }
            else if(now >= b.nextRearm){ b.attackHasHit = false; b.nextRearm = now + (flame ? 320 : BOSS_BEAM_REARM_MS); }
            b.attackHitBox = flame ? frontBox(a.reach, b.h * 0.45f, b.h * 0.4f) : frontBox(a.reach, b.h * 0.2f, b.h * 0.45f);
            b.isAttackBoxActive = !b.attackHasHit;
            Camera::getInstance().rumble(0.3f);
            break;
        }

        case K::BEAM_DIAG: {
            const bool strong = p2;                                // berserk / phase 2+: thicker, tracks, rearms faster
            if(!b.launched){ b.launched = true; b.nextRearm = now; }
            if(strong) aimBeam(b, player, a);                      // follows the player's feet
            b.beamOn = true;
            b.beamThick = strong ? 78.0f : 48.0f;
            if(now >= b.nextRearm){ b.attackHasHit = false; b.nextRearm = now + (strong ? 380 : 520); }
            Camera::getInstance().rumble(0.35f);
            break;
        }

        case K::TELEPORT_POUNCE: {
            const int repeats = std::max(1, a.count);
            if(b.stage == 0){                                       // vanish
                b.hidden = true; b.untargetable = true; b.noPhysics = true;
                b.markerOn = true; b.markerX = pcx;
                b.stage = 1; b.stageT = now;
            } else if(b.stage == 1){                                // marker follows, then locks (anticipation)
                if(stageEl < 420) b.markerX = pcx;
                b.markerX = std::clamp(b.markerX, b.arenaL + b.w * 0.5f, b.arenaR - b.w * 0.5f);
                if(stageEl >= 820){
                    b.x = b.markerX - b.w * 0.5f;
                    b.y = m_ceilY + 4.0f;
                    b.velocityY = 700.0f;
                    b.hidden = false; b.noPhysics = false; b.isGrounded = false;
                    b.isFacingRight = pcx >= b.markerX;
                    b.stage = 2; b.stageT = now;
                    b.attackHasHit = false;
                }
            } else {                                                // drop onto the marker
                b.untargetable = false;
                b.attackHitBox = bodyBox();
                b.isAttackBoxActive = !b.attackHasHit;
                if(stageEl > 120 && b.isGrounded){
                    Camera::getInstance().addTrauma(0.6f);
                    b.markerOn = false;
                    b.isAttackBoxActive = false;
                    if(++b.spawned < repeats){ b.stage = 0; b.stageT = now; }
                    else b.endEarly = true;
                }
            }
            break;
        }

        case K::DODGE_DASH: {
            if(!b.launched){
                b.launched = true;
                b.dirX = (pcx >= bcx) ? -1.0f : 1.0f;              // away from the player
                b.isFacingRight = b.dirX < 0.0f;                    // ...but keep looking at him
                b.iframesUntil = now + 320;
                b.travelled = 0.0f;
            }
            const float step = sp * dt;
            if(tryMoveX(b, b.x + b.dirX * step, wall, blocks)){
                b.travelled += step;
                if(b.travelled >= a.reach) b.endEarly = true;
            } else b.endEarly = true;
            if(b.endEarly){ faceToward(b, pcx); }
            break;
        }

        case K::ERUPT: {
            const int total = a.count + (p2 ? 2 : 0);
            const float interval = (float)a.activeMs / (float)std::max(1, total);
            while(b.spawned < total && (float)elapsed >= (float)b.spawned * interval){
                float cx;
                if(a.aux > 0.0f){                                   // a line of eruptions marching from the boss to the player
                    const float dir = (pcx >= bcx) ? 1.0f : -1.0f;
                    cx = bcx + dir * (b.w * 0.5f + 90.0f + (float)b.spawned * 150.0f);
                } else {                                            // under the player, then fanning out beside him
                    const int i = b.spawned;
                    const float off = (i == 0) ? 0.0f : ((i % 2) ? 1.0f : -1.0f) * (float)((i + 1) / 2) * 170.0f;
                    cx = pcx + off;
                }
                cx = std::clamp(cx, b.arenaL + 50.0f, b.arenaR - 50.0f);
                spawnErupt(cx, 90.0f, 230.0f, 0.8f, 0.4f);
                b.spawned++;
            }
            break;
        }

        case K::SPORE_CLOUD: {
            if(!b.launched){
                b.launched = true;
                const int total = std::max(1, a.count);
                for(int i = 0; i < total; ++i){
                    const float off = (i == 0) ? 0.0f : ((i % 2) ? 1.0f : -1.0f) * (float)((i + 1) / 2) * 300.0f;
                    spawnSpore(std::clamp(pcx + off, b.arenaL + 130.0f, b.arenaR - 130.0f));
                }
                Camera::getInstance().addTrauma(0.3f);
            }
            break;
        }

        case K::BURROW_AMBUSH: {
            const float floorY = m_floorY;
            switch(b.stage){
                case 0:                                              // dig in
                    b.noPhysics = true; b.velocityY = 0.0f; b.stage = 1; b.stageT = now; break;
                case 1:
                    b.y += 1100.0f * dt;
                    Camera::getInstance().rumble(0.25f);
                    if(b.y >= floorY + 4.0f){ b.hidden = true; b.untargetable = true; b.stage = 2; b.stageT = now; }
                    break;
                case 2: {                                            // tunnelling toward the player, a dust marker on the floor
                    const float tx = pcx - b.w * 0.5f, step = sp * dt;
                    b.x = std::clamp(b.x + std::clamp(tx - b.x, -step, step), b.arenaL, std::max(b.arenaL, b.arenaR - b.w));
                    b.markerX = b.x + b.w * 0.5f; b.markerOn = true;
                    Camera::getInstance().rumble(0.2f);
                    if((std::fabs(tx - b.x) < 24.0f && stageEl > 700) || stageEl > 2400){
                        b.stage = 3; b.stageT = now;
                        spawnErupt(b.markerX, b.w * 0.55f, b.h * 1.4f, 0.65f, 0.35f);   // the eruption itself hurts
                    }
                    break; }
                case 3:
                    if(stageEl >= 650){ b.stage = 4; b.stageT = now; b.hidden = false; b.markerOn = false; }
                    break;
                case 4:                                              // burst out
                    b.y -= 1700.0f * dt;
                    if(b.y <= floorY - b.h){
                        b.y = floorY - b.h; b.untargetable = false; b.exposed = true;
                        b.stage = 5; b.stageT = now; b.spawned = 0;
                        Camera::getInstance().addTrauma(0.5f);
                    }
                    break;
                case 5: {                                            // head out: the opening. It spits both ways.
                    const int total = std::max(1, a.count);
                    const float interval = 2400.0f / (float)total;
                    while(b.spawned < total && (float)stageEl >= (float)b.spawned * interval){
                        spawnSpit(b, (b.spawned % 2 == 0) ? 1 : -1, a);
                        b.spawned++;
                    }
                    if(stageEl >= 2700){ b.exposed = false; b.stage = 6; b.stageT = now; }
                    break; }
                case 6:
                    b.y += 1000.0f * dt;
                    if(b.y >= floorY + 4.0f){ b.hidden = true; b.untargetable = true; b.stage = 7; b.stageT = now; }
                    break;
                default:
                    if(stageEl >= 450){
                        b.y = floorY - b.h; b.hidden = false; b.untargetable = false;
                        b.noPhysics = false; b.velocityY = 0.0f; b.endEarly = true;
                    }
                    break;
            }
            b.isAttackBoxActive = false;
            break;
        }

        case K::SURFACE_MOVE: {
            if(!b.launched){
                b.launched = true;
                buildSurfacePath(b, (int)a.aux, player, wall);
                b.noPhysics = true; b.velocityY = 0.0f;
            }
            if(b.wpIdx >= b.wpCount){ b.endEarly = true; break; }
            setSurface(b, b.wpSurf[b.wpIdx]);
            b.x = std::clamp(b.x, b.arenaL, std::max(b.arenaL, b.arenaR - b.w));   // switching w/h must not push it out of the arena
            const float dx = b.wpX[b.wpIdx] - b.x, dy = b.wpY[b.wpIdx] - b.y;
            const float dist = std::sqrt(dx * dx + dy * dy), step = sp * dt;
            if(dist <= step){ b.x = b.wpX[b.wpIdx]; b.y = b.wpY[b.wpIdx]; b.wpIdx++; }
            else            { b.x += dx / dist * step; b.y += dy / dist * step; }
            break;
        }

        case K::ORB_SUMMON: {
            const int total = std::max(1, a.count);
            const float interval = (float)a.activeMs / (float)total;
            while(b.spawned < total && (float)elapsed >= (float)b.spawned * interval){
                std::uniform_real_distribution<float> dx(b.arenaL + 80.0f, std::max(b.arenaL + 81.0f, b.arenaR - 80.0f));
                std::uniform_real_distribution<float> dy(m_ceilY + 120.0f, std::max(m_ceilY + 121.0f, m_floorY - 220.0f));
                spawnOrb(dx(m_rng), dy(m_rng), sp, 1.2f + 0.12f * (float)b.spawned);
                b.spawned++;
            }
            break;
        }

        case K::SMITE: {
            const int total = std::max(1, a.count) + (p2 ? 1 : 0);
            const float interval = (float)a.activeMs / (float)total;
            while(b.spawned < total && (float)elapsed >= (float)b.spawned * interval){
                float x = pcx;
                if(b.spawned % 2 == 1){
                    std::uniform_real_distribution<float> d(b.arenaL + 60.0f, std::max(b.arenaL + 61.0f, b.arenaR - 60.0f));
                    x = d(m_rng);
                }
                spawnSmite(std::clamp(x, b.arenaL + 50.0f, b.arenaR - 50.0f), wall);
                b.spawned++;
            }
            break;
        }

        case K::AERIAL_DIVE: {
            const float floorY = wallBottomOf(wall);
            if(!b.launched){
                b.launched = true;
                const float dx = pcx - bcx;
                float dy = pcy - (b.y + b.h * 0.5f);
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
//  The Brontos: three heads on long necks. Heads slam (one at a time; two at once in phase 2), a neck sweeps the
//  floor, a head darts in and grabs. Only the heads take damage (the necks and body are scenery with a hitbox).
// ====================================================================================================
void BossBuilder::updateBrontos(Boss& b, const BossDef& def, float dt, const SDL_FRect& player, const SDL_FRect& wall, unsigned int now){
    const bool p2 = b.phase != BossPhase::PHASE_1;
    int sum = 0;
    for(size_t i = 0; i < b.parts.size(); ++i){
        updatePart(b, b.parts[i], dt, player, wall, now);
        sum += std::max(0, b.parts[i].hp);
    }
    b.hp = sum;
    if(sum <= 0 && !b.isDead()){ endGrab(); startDeath(b, now); return; }
    if(b.state != BossState::IDLE) return;                 // no new attacks during the intro / roar

    if(now < b.nextAttackTime) return;
    int busy = 0;
    std::vector<int> ready;
    for(size_t i = 0; i < b.parts.size(); ++i){
        const int st = b.parts[i].state;
        if(st == 1 || st == 2 || st == 3 || st == 4) busy++;
        else if(st == 0) ready.push_back((int)i);
    }
    if(busy > 0 || ready.empty()) return;                  // wait until every head is back home

    std::shuffle(ready.begin(), ready.end(), m_rng);
    const int n = std::min((int)ready.size(), p2 ? 2 : 1);
    std::uniform_int_distribution<int> kd(0, 2);
    int firstKind = kd(m_rng);
    for(int k = 0; k < n; ++k){
        int kind = (k == 0) ? firstKind : (firstKind + 1 + kd(m_rng) % 2) % 3;   // two heads never repeat the same move
        BossPart& p = b.parts[(size_t)ready[(size_t)k]];
        startPartAttack(b, p, kind, player, now + (unsigned int)k * 380);
    }
    b.nextAttackTime = now + (unsigned int)(def.idleMs * (p2 ? BOSS_PHASE2_IDLE_MUL : 1.0f));
}

void BossBuilder::startPartAttack(Boss& b, BossPart& p, int kind, const SDL_FRect& player, unsigned int startAt){
    (void)b; (void)player;
    p.kind = kind;
    p.state = 1;
    p.t0 = startAt;
    p.hasHit = false;
    p.hitActive = false;
}

void BossBuilder::updatePart(Boss& b, BossPart& p, float dt, const SDL_FRect& player, const SDL_FRect& wall, unsigned int now){
    (void)wall;
    const float floorY = m_floorY, ceilY = m_ceilY;
    const float pcx = player.x + player.w * 0.5f, pcy = player.y + player.h * 0.5f;
    const int partIdx = (int)(&p - b.parts.data());
    const int myBoss = (int)(&b - m_bosses.data());
    const bool held = m_grab.active && m_grab.boss == myBoss && m_grab.part == partIdx;
    p.hitActive = false;

    if(p.state == 5){                                       // dead head: slumps to the floor
        p.rect.y = std::min(p.rect.y + 900.0f * dt, floorY - p.rect.h * 0.6f);
        return;
    }
    if(p.hp <= 0){ p.state = 5; if(held) endGrab(); return; }

    auto moveTo = [&](float tx, float ty, float speed){
        const float dx = tx - p.rect.x, dy = ty - p.rect.y;
        const float dist = std::sqrt(dx * dx + dy * dy), step = speed * dt;
        if(dist <= step){ p.rect.x = tx; p.rect.y = ty; return true; }
        p.rect.x += dx / dist * step; p.rect.y += dy / dist * step; return false;
    };
    const float minX = b.arenaL, maxX = b.arenaR - p.rect.w;
    const float el = (float)((int)now - (int)p.t0);

    switch(p.state){
        case 0:
            p.swayPhase += dt * 1.4f;
            p.rect.x = p.homeX + std::sin(p.swayPhase) * 40.0f;
            p.rect.y = p.homeY + std::sin(p.swayPhase * 1.3f) * 18.0f;
            break;

        case 1: {   // telegraph
            if(el < 0.0f){ p.swayPhase += dt * 1.4f; break; }
            if(p.kind == 0){                                   // SLAM: rise above the player, lock the spot, fall
                if(el < 650.0f){ p.tx = std::clamp(pcx - p.rect.w * 0.5f, minX, maxX); }
                moveTo(p.tx, ceilY + 8.0f, 1100.0f);
                if(el >= 950.0f){ p.state = 2; p.t0 = now; }
            } else if(p.kind == 1){                            // SWEEP: head goes low on the far side
                moveTo(minX, floorY - p.rect.h, 1700.0f);
                if(el >= 850.0f){ p.state = 2; p.t0 = now; }
            } else {                                           // GRAB: pull back, aim, lock
                if(el < 480.0f){ p.tx = pcx - p.rect.w * 0.5f; p.ty = pcy - p.rect.h * 0.5f; }
                const float sx = (pcx > (b.arenaL + b.arenaR) * 0.5f) ? pcx - 520.0f : pcx + 520.0f;
                moveTo(std::clamp(sx, minX, maxX), std::clamp(pcy - 60.0f, ceilY + 10.0f, floorY - p.rect.h), 900.0f);
                if(el >= 700.0f){ p.state = 2; p.t0 = now; p.dirX = 0.0f; }
            }
            break;
        }

        case 2: {   // strike
            if(p.kind == 0){
                p.rect.y += 2300.0f * dt;
                p.hit = p.rect; p.hitActive = true;
                if(p.rect.y + p.rect.h >= floorY){
                    p.rect.y = floorY - p.rect.h;
                    p.hitActive = false;
                    Camera::getInstance().addTrauma(0.75f);
                    spawnWavesAt(p.rect.x + p.rect.w * 0.5f, p.rect.w * 0.5f, floorY, 430.0f, 44.0f, 1.2f);
                    p.state = 3; p.t0 = now;
                }
            } else if(p.kind == 1){
                p.rect.x += 880.0f * dt;
                p.hit = { p.rect.x - 50.0f, p.rect.y + 10.0f, p.rect.w + 50.0f, p.rect.h - 10.0f }; p.hitActive = !p.hasHit;
                Camera::getInstance().rumble(0.2f);
                if(p.rect.x >= p.anchorX - 330.0f){ p.state = 3; p.t0 = now; }
            } else {
                if(held){ p.hitActive = false; break; }         // holding the player
                const float dx = p.tx - p.rect.x, dy = p.ty - p.rect.y;
                const float dist = std::max(1.0f, std::sqrt(dx * dx + dy * dy));
                const float step = 1500.0f * dt;
                p.rect.x += dx / dist * step; p.rect.y += dy / dist * step;
                p.dirX += step;
                p.hit = p.rect; p.hitActive = !p.hasHit;
                if(dist <= step || p.dirX > 900.0f){ p.state = 3; p.t0 = now; }
            }
            break;
        }

        case 3: {   // stuck / recovering: the opening. Heads take extra damage here.
            if(held){ p.hitActive = false; break; }
            const float dur = (p.kind == 0) ? 1700.0f : (p.kind == 1) ? 700.0f : 900.0f;
            if(el >= dur){ p.state = 4; p.t0 = now; }
            break;
        }

        case 4:     // back home
            if(moveTo(p.homeX, p.homeY, 950.0f)){ p.state = 0; }
            break;
    }
    if(p.rect.x < minX && p.state != 2) p.rect.x = minX;
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

void BossBuilder::spawnSpit(const Boss& b, int side, const BossAttackDef& a){
    (void)a;
    std::uniform_real_distribution<float> sx(260.0f, 640.0f), sy(760.0f, 1080.0f);
    BossHazard h;
    h.source = K::SPIT_BOTH;
    h.w = h.h = BOSS_ENEMY_PROJ_SIZE;
    h.gravity = BOSS_PROJECTILE_GRAVITY;
    h.x = b.x + b.w * 0.5f + (float)side * b.w * 0.35f - h.w * 0.5f;
    h.y = b.y + 6.0f;
    h.vx = (float)side * sx(m_rng);
    h.vy = -sy(m_rng);
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

void BossBuilder::spawnWavesAt(float cx, float halfW, float feetY, float speed, float height, float life){
    for(int dir = -1; dir <= 1; dir += 2){
        BossHazard h;
        h.source = K::SHOCKWAVE;
        h.w = BOSS_WAVE_W; h.h = height;
        h.x = cx + (float)dir * halfW - (dir > 0 ? 0.0f : h.w);
        h.y = feetY - h.h - 1.0f;                                  // rides the floor
        h.vx = (float)dir * speed;
        h.life = life;
        h.dieOnTerrain = false;                                    // only blocks and the arena end it (see updateHazards)
        h.hugsFloor = true;
        m_hazards.push_back(h);
    }
}

void BossBuilder::spawnShockwavePair(const Boss& b, const BossAttackDef& a){
    const float sp = std::max(a.speed, 300.0f) * phaseSpeedMul(b);
    // for SHOCKWAVE moves `reach` is the wave height (Treehorn's logs are taller than a stomp's ripple)
    const float height = (a.kind == K::SHOCKWAVE && a.reach > 0.0f) ? a.reach : BOSS_WAVE_H;
    spawnWavesAt(b.x + b.w * 0.5f, b.w * 0.5f, b.y + b.h, sp, height, 1.8f);
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

// a column that bursts out of the floor after a floor marker
void BossBuilder::spawnErupt(float cx, float width, float height, float warn, float lifeAfter){
    BossHazard h;
    h.source = K::ERUPT;
    h.w = width; h.h = height;
    h.x = cx - width * 0.5f;
    h.y = m_floorY - height - 1.0f;
    h.warn = warn;
    h.life = lifeAfter;
    h.dieOnTerrain = false;
    m_hazards.push_back(h);
}

void BossBuilder::spawnSpore(float cx){
    BossHazard h;
    h.source = K::SPORE_CLOUD;
    h.w = 250.0f; h.h = 130.0f;
    h.x = cx - h.w * 0.5f;
    h.y = m_floorY - h.h - 1.0f;
    h.warn = 0.5f;
    h.life = 5.5f;
    h.persistent = true;
    h.dieOnTerrain = false;
    m_hazards.push_back(h);
}

void BossBuilder::spawnOrb(float x, float y, float launchSpeed, float hover){
    BossHazard h;
    h.source = K::ORB_SUMMON;
    h.w = h.h = 42.0f;
    h.x = x - h.w * 0.5f; h.y = y - h.h * 0.5f;
    h.warn = 0.7f;                  // the orb fades in (harmless)
    h.hoverT = hover;               // then hangs in the air...
    h.launchSpeed = launchSpeed;    // ...then flies at where the player is
    h.life = 4.5f;
    m_hazards.push_back(h);
}

void BossBuilder::spawnSmite(float cx, const SDL_FRect& wall){
    (void)wall;
    BossHazard h;
    h.source = K::SMITE;
    h.w = 86.0f;
    h.h = m_floorY - m_ceilY - 2.0f;
    h.x = cx - h.w * 0.5f;
    h.y = m_ceilY;
    h.warn = 0.9f;
    h.life = 0.4f;
    h.dieOnTerrain = false;
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

        if(h.source == K::ORB_SUMMON && h.hoverT > 0.0f){          // hanging in the air
            h.hoverT -= dt;
            h.y += std::sin(h.hoverT * 9.0f) * 20.0f * dt;
            if(h.hoverT <= 0.0f){
                const float dx = (m_playerRect.x + m_playerRect.w * 0.5f) - (h.x + h.w * 0.5f);
                const float dy = (m_playerRect.y + m_playerRect.h * 0.5f) - (h.y + h.h * 0.5f);
                const float len = std::max(1.0f, std::sqrt(dx * dx + dy * dy));
                h.vx = dx / len * h.launchSpeed; h.vy = dy / len * h.launchSpeed;
            }
            ++i; continue;
        }

        h.age += dt;
        h.vy += h.gravity * dt;
        h.x += h.vx * dt;
        h.y += h.vy * dt;

        bool dead = h.age >= h.life;
        if(!dead) dead = h.x < left || h.x + h.w > right || h.y + h.h > bottom || (h.source != K::ROCK_RAIN && h.y < top - 1.0f);
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
namespace {
// does a thick segment touch a rect? (sampled every ~16px, clipped to x in [minX,maxX])
    bool segmentHitsRect(float x1, float y1, float x2, float y2, float thick, const SDL_FRect& r, float minX, float maxX){
        const float dx = x2 - x1, dy = y2 - y1;
        const float len = std::sqrt(dx * dx + dy * dy);
        const int steps = std::max(1, (int)(len / 16.0f));
        for(int i = 0; i <= steps; ++i){
            const float t = (float)i / (float)steps;
            const float px = x1 + dx * t, py = y1 + dy * t;
            if(px < minX || px > maxX) continue;
            if(rectsOverlap(px - thick * 0.5f, py - thick * 0.5f, thick, thick, r.x, r.y, r.w, r.h)) return true;
        }
        return false;
    }
}

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
        if(!d || d->phase2Threshold <= 0.0f) return;
        const int gateHp = (int)((float)boss.maxHp * d->phase2Threshold) - 1;
        if(boss.hp < gateHp) boss.hp = gateHp;
    };
    auto hitEvent = [&](const SDL_FRect& hb, const SDL_FRect& body, bool lethal){
        const float hitterCx = hb.x + hb.w * 0.5f;
        const float knockDirX = ((body.x + body.w * 0.5f) >= hitterCx) ? 1.0f : -1.0f;
        const float ox1 = std::max(hb.x, body.x), ox2 = std::min(hb.x + hb.w, body.x + body.w);
        const float oy1 = std::max(hb.y, body.y), oy2 = std::min(hb.y + hb.h, body.y + body.h);
        m_hitEvents.push_back({(ox1 + ox2) * 0.5f, (oy1 + oy2) * 0.5f, knockDirX, lethal, false});
    };
    auto overlaps = [](const SDL_FRect& a, const SDL_FRect& r){
        return gameMath::checkcollision(a.x, a.y, r.x, r.y, a.h, a.w, r.h, r.w);
    };

    for(size_t bi = 0; bi < m_bosses.size(); ++bi){
        Boss& b = m_bosses[bi];
        const BossDef* def = getBossDef(b.type);
        if(!def) continue;
        // untouchable while waking up, roaring, being "dead", reborn, or dead
        if(b.state == BossState::DORMANT || b.state == BossState::INTRO || b.state == BossState::PHASE_TRANSITION ||
           b.state == BossState::QTE || b.state == BossState::FALSE_DEATH || b.state == BossState::REVIVE ||
           b.state == BossState::DEAD) continue;

        // ---- the Brontos: only the heads can be hurt ----
        if(def->behavior == BossBehavior::BRONTOS){
            for(size_t pi = 0; pi < b.parts.size(); ++pi){
                BossPart& p = b.parts[pi];
                if(p.state == 5 || p.hp <= 0) continue;
                const float mul = (p.state == 3 ? BOSS_RECOVER_DAMAGE_MUL : 1.0f);
                bool hurt = false;
                if(meleeActive && p.lastMeleeId != attackId && overlaps(meleeHitBox, p.rect)){
                    p.lastMeleeId = attackId;
                    p.hp -= (int)((float)ENEMY_MELEE_DAMAGE * mul);
                    hitEvent(meleeHitBox, p.rect, p.hp <= 0);
                    hurt = true;
                }
                if(beamActive && overlaps(beamHitBox, p.rect)){
                    p.beamAccum += ENEMY_BEAM_DAMAGE_PER_SEC * BOSS_BEAM_DAMAGE_MUL * mul * dt;
                    const int whole = (int)p.beamAccum;
                    if(whole > 0){ p.hp -= whole; p.beamAccum -= (float)whole; }
                    hurt = true;
                }
                if(hurt){
                    p.hurtFlashEnd = now + ENEMY_HURT_FLASH_MS;
                    if(m_grab.active && m_grab.boss == (int)bi && m_grab.part == (int)pi && ++m_grab.struggle >= BOSS_GRAB_BREAK_HITS){
                        m_pendingBite = false; endGrab();
                    }
                }
            }
            int sum = 0; for(auto& p : b.parts) sum += std::max(0, p.hp);
            b.hp = sum;
            if(b.hp <= 0){ endGrab(); startDeath(b, now); kills++; }
            continue;
        }

        if(b.untargetable || b.hidden || now < b.iframesUntil) continue;

        float mul = (b.state == BossState::RECOVER || b.exposed) ? BOSS_RECOVER_DAMAGE_MUL : 1.0f;
        if(now < b.staggerUntil) mul = 2.2f;                          // a parried boss is wide open
        if(b.phase != BossPhase::PHASE_1) mul *= def->p2DamageMul;    // Brother: tankier in phase 2
        const SDL_FRect hr = b.hurtRect();

        if(meleeActive && b.lastMeleeAttackId != attackId && overlaps(meleeHitBox, hr)){
            b.lastMeleeAttackId = attackId;
            b.hp -= std::max(1, (int)((float)ENEMY_MELEE_DAMAGE * mul));
            phaseGate(b);
            b.isHurt = true;
            b.hurtFlashEnd = now + ENEMY_HURT_FLASH_MS;
            b.whiteFlashEnd = now + ENEMY_WHITE_FLASH_MS;
            hitEvent(meleeHitBox, hr, b.hp <= 0);
            if(m_grab.active && m_grab.boss == (int)bi && ++m_grab.struggle >= BOSS_GRAB_BREAK_HITS){
                m_pendingBite = false; endGrab();                      // hit it enough: it lets go
            }
        }

        if(beamActive && overlaps(beamHitBox, hr)){
            b.beamAccum += ENEMY_BEAM_DAMAGE_PER_SEC * BOSS_BEAM_DAMAGE_MUL * mul * dt;
            const int whole = (int)b.beamAccum;
            if(whole > 0){ b.hp -= whole; b.beamAccum -= (float)whole; phaseGate(b); }
            b.isHurt = true;
            b.hurtFlashEnd = now + ENEMY_HURT_FLASH_MS;
            if(now >= b.nextBeamFx){
                b.nextBeamFx = now + 90;
                const float ox1 = std::max(beamHitBox.x, hr.x), ox2 = std::min(beamHitBox.x + beamHitBox.w, hr.x + hr.w);
                const float oy1 = std::max(beamHitBox.y, hr.y), oy2 = std::min(beamHitBox.y + beamHitBox.h, hr.y + hr.h);
                const bool goesRight = (beamHitBox.x + beamHitBox.w * 0.5f) < (hr.x + hr.w * 0.5f);
                m_hitEvents.push_back({goesRight ? ox1 : ox2, (oy1 + oy2) * 0.5f, goesRight ? -1.0f : 1.0f, b.hp <= 0, true});
            }
        }

        if(b.hp <= 0){
            if(def->finisherQte && !b.qteDone)                                      beginQte(b, now);
            else if(def->revive && b.phase == BossPhase::PHASE_2)                    beginFalseDeath(b, now);
            else { startDeath(b, now); kills++; }
        }
    }
    return kills;
}

bool BossBuilder::checkAttackOnPlayer(float playerX, float playerY, float playerW, float playerH,
                                      unsigned int now, bool canDamage, gameMath::collisionSide& outSide){
    const float pcx = playerX + playerW * 0.5f;
    const SDL_FRect pr{playerX, playerY, playerW, playerH};

    // the final bite of a grab that was not broken
    if(m_pendingBite){
        if(canDamage){
            m_pendingBite = false;
            outSide = gameMath::collisionSide::LEFT;
            return true;
        }
        if(now > m_pendingBiteUntil) m_pendingBite = false;
    }

    for(size_t bi = 0; bi < m_bosses.size(); ++bi){
        Boss& b = m_bosses[bi];
        if(b.isDead()) continue;
        const BossDef* def = getBossDef(b.type);

        // head attacks (Brontos)
        for(size_t pi = 0; pi < b.parts.size(); ++pi){
            BossPart& p = b.parts[pi];
            if(!p.hitActive || p.hasHit) continue;
            if(rectsOverlap(p.hit.x, p.hit.y, p.hit.w, p.hit.h, playerX, playerY, playerW, playerH)){
                p.hasHit = true;
                if(!canDamage) continue;
                outSide = (pcx >= p.rect.x + p.rect.w * 0.5f) ? gameMath::collisionSide::RIGHT : gameMath::collisionSide::LEFT;
                if(p.kind == 2 && !m_grab.active) startGrab((int)bi, (int)pi, now);
                return true;
            }
        }

        // diagonal beams
        if(b.beamOn && !b.attackHasHit &&
           segmentHitsRect(b.beamX1, b.beamY1, b.beamX2, b.beamY2, b.beamThick, pr, m_arenaInnerL, m_arenaInnerR)){
            b.attackHasHit = true;
            if(!canDamage) continue;
            outSide = (pcx >= b.beamX1) ? gameMath::collisionSide::RIGHT : gameMath::collisionSide::LEFT;
            return true;
        }

        if(!b.isAttackBoxActive || b.attackHasHit) continue;
        if(gameMath::checkcollision(b.attackHitBox.x, b.attackHitBox.y, playerX, playerY,
                                    b.attackHitBox.h, b.attackHitBox.w, playerH, playerW)){
            b.attackHasHit = true;                       // spent either way (a swing into an invincible player whiffs)
            if(!canDamage) continue;
            outSide = (pcx >= b.x + b.w * 0.5f) ? gameMath::collisionSide::RIGHT : gameMath::collisionSide::LEFT;   // shoved away from the boss
            if(def && b.state == BossState::ATTACKING && b.attack >= 0 && def->attacks[b.attack].kind == K::GRAB_BITE && !m_grab.active)
                startGrab((int)bi, -1, now);                                                                      // snatched
            return true;
        }
    }
    for(size_t i = 0; i < m_hazards.size(); ++i){
        BossHazard& h = m_hazards[i];
        if(h.warn > 0.0f) continue;                      // still only a warning marker
        if(h.source == K::ORB_SUMMON && h.hoverT > 0.0f) continue;   // hanging orbs are harmless until they launch
        if(rectsOverlap(h.x, h.y, h.w, h.h, playerX, playerY, playerW, playerH)){
            const bool playerIsRight = pcx >= h.x + h.w * 0.5f;
            if(h.persistent){                            // gas: ticks damage, is not used up
                if(now < h.nextHit) continue;
                h.nextHit = now + 800;
                if(!canDamage) continue;
                outSide = playerIsRight ? gameMath::collisionSide::RIGHT : gameMath::collisionSide::LEFT;
                return true;
            }
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
    renderExtras(renderer, camX, camY);
}

namespace {
    void fillQuad(SDL_Renderer* r, float x1, float y1, float x2, float y2, float thick, SDL_FColor c){
        const float dx = x2 - x1, dy = y2 - y1, len = std::max(1.0f, std::sqrt(dx * dx + dy * dy));
        const float nx = -dy / len * thick * 0.5f, ny = dx / len * thick * 0.5f;
        SDL_Vertex v[4] = {
                {{x1 + nx, y1 + ny}, c, {0, 0}}, {{x2 + nx, y2 + ny}, c, {0, 0}},
                {{x2 - nx, y2 - ny}, c, {0, 0}}, {{x1 - nx, y1 - ny}, c, {0, 0}},
        };
        const int idx[6] = {0, 1, 2, 0, 2, 3};
        SDL_RenderGeometry(r, nullptr, v, 4, idx, 6);
    }
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
    else if(b.state == BossState::RECOVER || b.exposed){ cr = (Uint8)std::min(255, cr + 40); cg = (Uint8)std::min(255, cg + 40); cb = (Uint8)std::min(255, cb + 40); }
    if(b.state == BossState::FALSE_DEATH){ cr = cg = cb = 70; a = 150; }

    SDL_BlendMode prev = SDL_BLENDMODE_NONE;
    SDL_GetRenderDrawBlendMode(r, &prev);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);

    // ---- Brontos: body, necks, heads ----
    if(def->behavior == BossBehavior::BRONTOS){
        SDL_FRect bodyR{ b.x - camX, b.y - camY, b.w, b.h };
        SDL_SetRenderDrawColor(r, (Uint8)(cr * 0.6f), (Uint8)(cg * 0.6f), (Uint8)(cb * 0.6f), 230);   // body: not a target
        SDL_RenderFillRect(r, &bodyR);
        SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
        SDL_RenderRect(r, &bodyR);
        for(const auto& p : b.parts){
            const bool flash = now < p.hurtFlashEnd;
            const bool dead = p.state == 5;
            fillQuad(r, p.anchorX - camX, p.anchorY - camY, p.rect.x + p.rect.w * 0.5f - camX, p.rect.y + p.rect.h * 0.5f - camY, 46.0f,
                     SDL_FColor{ cr / 255.0f * 0.7f, cg / 255.0f * 0.7f, cb / 255.0f * 0.7f, dead ? 0.35f : 0.9f });   // neck
            SDL_FRect head{ p.rect.x - camX, p.rect.y - camY, p.rect.w, p.rect.h };
            if(dead)       SDL_SetRenderDrawColor(r, 90, 90, 90, 130);
            else if(flash) SDL_SetRenderDrawColor(r, 255, 130, 130, 255);
            else if(p.state == 3) SDL_SetRenderDrawColor(r, (Uint8)std::min(255, cr + 60), (Uint8)std::min(255, cg + 60), (Uint8)std::min(255, cb + 60), 245);
            else           SDL_SetRenderDrawColor(r, cr, cg, cb, 245);
            SDL_RenderFillRect(r, &head);
            SDL_SetRenderDrawColor(r, b.phase != BossPhase::PHASE_1 && !dead ? 255 : 255, b.phase != BossPhase::PHASE_1 && !dead ? 40 : 255, b.phase != BossPhase::PHASE_1 && !dead ? 40 : 255, 255);
            SDL_RenderRect(r, &head);
            if(!dead){                                                                         // head hp pip bar
                SDL_FRect back{ head.x, head.y - 12.0f, head.w, 6.0f };
                SDL_SetRenderDrawColor(r, 0, 0, 0, 180); SDL_RenderFillRect(r, &back);
                SDL_FRect fill{ head.x, head.y - 12.0f, head.w * (float)p.hp / (float)std::max(1, p.maxHp), 6.0f };
                SDL_SetRenderDrawColor(r, 220, 60, 60, 255); SDL_RenderFillRect(r, &fill);
            }
        }
        SDL_SetRenderDrawBlendMode(r, prev);
        return;
    }

    if(b.hidden){                                         // burrowed / teleporting: just a rumbling mound on the floor
        if(def->behavior != BossBehavior::PREACHER && b.markerOn){
            SDL_SetRenderDrawColor(r, 120, 90, 60, 220);
            for(int i = 0; i < 5; ++i){
                SDL_FRect d{ b.markerX - camX - 40.0f + (float)i * 18.0f, m_floorY - camY - 8.0f - (float)((now / 70 + i) % 3) * 5.0f, 12.0f, 12.0f };
                SDL_RenderFillRect(r, &d);
            }
        }
        SDL_SetRenderDrawBlendMode(r, prev);
        return;
    }

    // crouch during the wind-up so the tell reads even without art
    const float squash = (b.state == BossState::TELEGRAPH) ? 10.0f : 0.0f;
    float bodyH = b.h - squash;
    if(b.y + b.h > m_floorY + 1.0f && m_floorY > 0.0f) bodyH = std::max(4.0f, m_floorY - b.y - squash);   // burrowing: clipped at the floor
    SDL_FRect body{ b.x - camX, b.y - camY + squash, b.w, bodyH };

    if(b.winged){                                          // wings: drawn, but NOT where the damage is
        SDL_SetRenderDrawColor(r, 255, 245, 200, 150);
        const float flap = std::sin(now * 0.008f) * 18.0f;
        SDL_FRect lw{ body.x, body.y + body.h * 0.1f + flap, b.w * 0.3f, body.h * 0.6f };
        SDL_FRect rw{ body.x + b.w * 0.7f, body.y + body.h * 0.1f + flap, b.w * 0.3f, body.h * 0.6f };
        SDL_RenderFillRect(r, &lw); SDL_RenderFillRect(r, &rw);
        SDL_SetRenderDrawColor(r, 255, 255, 255, 200);
        SDL_RenderRect(r, &lw); SDL_RenderRect(r, &rw);
        const SDL_FRect hr = b.hurtRect();
        body = { hr.x - camX, hr.y - camY, hr.w, hr.h };
    }

    SDL_SetRenderDrawColor(r, cr, cg, cb, a);
    SDL_RenderFillRect(r, &body);
    if(b.phase != BossPhase::PHASE_1 && !b.isDead()) SDL_SetRenderDrawColor(r, 255, 40, 40, 255);   // outline: red once enraged
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
            case K::GRAB_BITE:    cr = 255; cg = 40;  cb = 140; break;
            case K::CHARGE:       cr = 255; cg = 230; cb = 60;  break;
            case K::LEAP_SLAM:    cr = 255; cg = 100; cb = 180; break;
            case K::BREATH_BEAM:  cr = 60;  cg = 200; cb = 255; break;
            case K::FLAMETHROWER: cr = 255; cg = 120; cb = 20;  break;
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
            const bool blink = ((now / (h.warn < 0.3f ? 60 : 120)) % 2) == 0;
            if(h.source == K::ERUPT || h.source == K::SPORE_CLOUD){
                // floor marker where something is about to burst out
                SDL_SetRenderDrawColor(r, 255, 90, 40, blink ? 200 : 100);
                SDL_FRect m{ h.x - camX, m_floorY - camY - 14.0f, h.w, 14.0f };
                SDL_RenderFillRect(r, &m);
                SDL_SetRenderDrawColor(r, 255, 255, 255, 220);
                SDL_RenderRect(r, &m);
            } else if(h.source == K::SMITE){
                SDL_SetRenderDrawColor(r, 255, 245, 170, blink ? 90 : 40);
                SDL_FRect col{ h.x - camX + h.w * 0.35f, h.y - camY, h.w * 0.3f, h.h };
                SDL_RenderFillRect(r, &col);
            } else if(h.source == K::ORB_SUMMON){
                SDL_SetRenderDrawColor(r, 255, 245, 170, blink ? 160 : 70);
                SDL_FRect o{ h.x - camX, h.y - camY, h.w, h.h };
                SDL_RenderRect(r, &o);
            } else {
                // rock shadow: a column from the rock down to the floor that blinks faster as the rock is about to drop
                SDL_SetRenderDrawColor(r, 255, 70, 70, blink ? 90 : 40);
                SDL_FRect col{ h.x - camX, h.y - camY, h.w, std::max(0.0f, m_floorY - h.y) };
                SDL_RenderFillRect(r, &col);
                SDL_SetRenderDrawColor(r, 255, 70, 70, 220);
                SDL_FRect rock{ h.x - camX, h.y - camY, h.w, h.h };
                SDL_RenderRect(r, &rock);
            }
            continue;
        }
        SDL_FRect box{ h.x - camX, h.y - camY, h.w, h.h };
        switch(h.source){
            case K::ROCK_RAIN:  SDL_SetRenderDrawColor(r, 150, 110, 70, 230);  break;
            case K::SHOCKWAVE:  SDL_SetRenderDrawColor(r, 255, 190, 60, 220);  break;
            case K::PROJECTILE_FAN: SDL_SetRenderDrawColor(r, 120, 255, 140, 230); break;
            case K::SPIT_BOTH:  SDL_SetRenderDrawColor(r, 150, 255, 60, 230);  break;
            case K::ERUPT:      SDL_SetRenderDrawColor(r, 120, 200, 60, 230);  break;
            case K::SPORE_CLOUD:SDL_SetRenderDrawColor(r, 170, 220, 80, 110);  break;
            case K::SMITE:      SDL_SetRenderDrawColor(r, 255, 245, 170, 230); break;
            case K::ORB_SUMMON: SDL_SetRenderDrawColor(r, 255, 250, 200, 240); break;
            default:            SDL_SetRenderDrawColor(r, 190, 100, 255, 230); break;
        }
        SDL_RenderFillRect(r, &box);
        SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
        SDL_RenderRect(r, &box);
    }
    SDL_SetRenderDrawBlendMode(r, prev);
}

// beams, markers, parry cues, the QTE ring, ghost pads, the god ray, head attack markers
void BossBuilder::renderExtras(SDL_Renderer* r, int camX, int camY) const {
    SDL_BlendMode prev = SDL_BLENDMODE_NONE;
    SDL_GetRenderDrawBlendMode(r, &prev);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    const unsigned int now = SDL_GetTicks();

    for(const auto& b : m_bosses){
        if(b.isDead()) continue;
        const BossDef* def = getBossDef(b.type);
        if(!def) continue;

        // diagonal beam: thin flashing aim line during the wind-up, full beam while it hurts
        if(b.beamAim || b.beamOn){
            const bool blink = ((now / 80) % 2) == 0;
            const float t = b.beamOn ? b.beamThick : 5.0f;
            fillQuad(r, b.beamX1 - camX, b.beamY1 - camY, b.beamX2 - camX, b.beamY2 - camY, t,
                     b.beamOn ? SDL_FColor{0.5f, 0.85f, 1.0f, 0.75f} : SDL_FColor{1.0f, 0.3f, 0.3f, blink ? 0.9f : 0.4f});
        }
        // teleport landing marker
        if(b.markerOn && def->behavior != BossBehavior::PREACHER && b.type != BossType::MOSS_LUMINOUS_WYRM){
            SDL_SetRenderDrawColor(r, 255, 60, 60, (now / 70) % 2 ? 200 : 100);
            SDL_FRect m{ b.markerX - camX - b.baseW * 0.5f, m_floorY - camY - 16.0f, b.baseW, 16.0f };
            SDL_RenderFillRect(r, &m);
        }
        if(b.markerOn && def->behavior == BossBehavior::PREACHER){
            SDL_SetRenderDrawColor(r, 255, 60, 60, (now / 70) % 2 ? 200 : 100);
            SDL_FRect m{ b.markerX - camX - b.baseW * 0.5f, m_floorY - camY - 16.0f, b.baseW, 16.0f };
            SDL_RenderFillRect(r, &m);
        }
        // parry cue: a white flash on the boss during the last moments before a parryable strike lands
        if(b.state == BossState::TELEGRAPH && b.attack >= 0 && def->attacks[b.attack].parry){
            const BossAttackDef& a = def->attacks[b.attack];
            const unsigned int windup = (unsigned int)(a.windupMs * (b.phase != BossPhase::PHASE_1 ? BOSS_PHASE2_WINDUP_MUL : 1.0f));
            const unsigned int impact = b.stateStart + windup;
            if(now + BOSS_PARRY_PRE_MS >= impact){
                SDL_SetRenderDrawColor(r, 255, 255, 255, 200);
                SDL_FRect f{ b.x - camX - 6.0f, b.y - camY - 6.0f, b.w + 12.0f, b.h + 12.0f };
                SDL_RenderRect(r, &f);
                SDL_FRect bang{ b.x + b.w * 0.5f - camX - 6.0f, b.y - camY - 52.0f, 12.0f, 30.0f };
                SDL_RenderFillRect(r, &bang);
                SDL_FRect dot{ bang.x, bang.y + 36.0f, 12.0f, 12.0f };
                SDL_RenderFillRect(r, &dot);
            }
        }
        // QTE ring: shrinks onto the boss; slash when it closes
        if(b.state == BossState::QTE){
            const float left = (float)((int)b.qteStrike - (int)now);
            const float f = std::clamp(left / 1200.0f, 0.0f, 1.0f);
            const float cx = b.x + b.w * 0.5f - camX, cy = b.y + b.h * 0.45f - camY;
            const float rad = 50.0f + 190.0f * f;
            const int N = 40;
            SDL_SetRenderDrawColor(r, 255, 255, 255, 230);
            for(int i = 0; i < N; ++i){
                const float a1 = (float)i / (float)N * 6.2831853f, a2 = (float)(i + 1) / (float)N * 6.2831853f;
                SDL_RenderLine(r, cx + std::cos(a1) * rad, cy + std::sin(a1) * rad, cx + std::cos(a2) * rad, cy + std::sin(a2) * rad);
            }
            SDL_SetRenderDrawColor(r, 255, 60, 60, 255);
            for(int i = 0; i < N; ++i){
                const float a1 = (float)i / (float)N * 6.2831853f, a2 = (float)(i + 1) / (float)N * 6.2831853f;
                SDL_RenderLine(r, cx + std::cos(a1) * 50.0f, cy + std::sin(a1) * 50.0f, cx + std::cos(a2) * 50.0f, cy + std::sin(a2) * 50.0f);
            }
            // progress pips
            for(int i = 0; i < BOSS_QTE_STEPS; ++i){
                SDL_FRect pip{ cx - 40.0f + (float)i * 30.0f, b.y - camY - 28.0f, 22.0f, 12.0f };
                SDL_SetRenderDrawColor(r, i < b.qteStep ? 255 : 90, i < b.qteStep ? 255 : 90, i < b.qteStep ? 120 : 90, 230);
                SDL_RenderFillRect(r, &pip);
            }
        }
        // god ray
        if(b.state == BossState::REVIVE){
            const float t = std::clamp((float)(now - b.stateStart) / 1400.0f, 0.0f, 1.0f);
            const float cx = b.x + b.w * 0.5f - camX;
            fillQuad(r, cx, -50.0f, cx, m_floorY - camY, 140.0f + 60.0f * t, SDL_FColor{1.0f, 0.97f, 0.75f, 0.25f + 0.25f * t});
            fillQuad(r, cx, -50.0f, cx, m_floorY - camY, 60.0f, SDL_FColor{1.0f, 1.0f, 1.0f, 0.5f * t});
        }
        // head attack markers
        for(const auto& p : b.parts){
            if(p.state != 1 || (int)now < (int)p.t0) continue;
            const float tx = (p.kind == 0) ? p.tx : (p.rect.x);
            if(p.kind == 0){
                SDL_SetRenderDrawColor(r, 255, 70, 70, (now / 80) % 2 ? 140 : 60);
                SDL_FRect col{ tx - camX, m_ceilY - camY, p.rect.w, m_floorY - m_ceilY };
                SDL_RenderFillRect(r, &col);
            } else if(p.kind == 1){
                SDL_SetRenderDrawColor(r, 255, 70, 70, (now / 80) % 2 ? 140 : 60);
                SDL_FRect lane{ b.arenaL - camX, m_floorY - camY - 70.0f, (b.arenaR - b.arenaL) * 0.8f, 70.0f };
                SDL_RenderFillRect(r, &lane);
            } else {
                fillQuad(r, p.rect.x + p.rect.w * 0.5f - camX, p.rect.y + p.rect.h * 0.5f - camY, p.tx + p.rect.w * 0.5f - camX, p.ty + p.rect.h * 0.5f - camY, 6.0f,
                         SDL_FColor{1.0f, 0.3f, 0.3f, 0.7f});
            }
        }
    }

    // sinner children's ghosts (pogo pads)
    for(const auto& g : m_ghosts){
        const float bob = std::sin(now * 0.004f + g.x * 0.01f) * 6.0f;
        SDL_FRect d{ g.x - camX, g.y - camY + bob, g.w, g.h };
        SDL_SetRenderDrawColor(r, 170, 210, 255, 150);
        SDL_RenderFillRect(r, &d);
        SDL_SetRenderDrawColor(r, 255, 255, 255, 230);
        SDL_RenderRect(r, &d);
        SDL_FRect e1{ d.x + 28.0f, d.y + 8.0f, 12.0f, 12.0f }, e2{ d.x + 78.0f, d.y + 8.0f, 12.0f, 12.0f };
        SDL_SetRenderDrawColor(r, 40, 60, 120, 255);
        SDL_RenderFillRect(r, &e1); SDL_RenderFillRect(r, &e2);
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
    if(info.phase == BossPhase::PHASE_3)      SDL_SetRenderDrawColor(renderer, 250, 235, 150, 255);
    else if(info.phase == BossPhase::PHASE_2) SDL_SetRenderDrawColor(renderer, 220, 50, 40, 255);
    else                                       SDL_SetRenderDrawColor(renderer, 200, 160, 40, 255);
    SDL_RenderFillRect(renderer, &fill);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderRect(renderer, &back);
    if(info.phase == BossPhase::PHASE_1 && info.hasPhase2)
        SDL_RenderLine(renderer, x + barW * 0.5f, y - 3, x + barW * 0.5f, y + barH + 3);   // phase-2 marker

    SDL_SetRenderDrawBlendMode(renderer, prev);
}
