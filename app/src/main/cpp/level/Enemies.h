//
// Created by LENOVO on 08-09-2026.
//
#pragma once
#include <vector>
#include "Traps.h"
#include "engine/Engine.h"
#include "level/BlockShapeBuilder.h"
#include <SDL3/SDL.h>

const float ENEMY_GRAVITY = 1800.00f; // matches player's m_gravity (GameState.h) so enemies fall at the same rate the player does

// combat tuning
const int   ENEMY_MELEE_DAMAGE          = 25;     // dealt once per player slash (m_playerHitBox)
const float ENEMY_BEAM_DAMAGE_PER_SEC   = 40.0f;  // dealt continuously while standing in the beam
const unsigned int ENEMY_HIT_IFRAME_MS  = 1000;    // enemy can't be hit again this soon after being hit
const unsigned int ENEMY_HURT_FLASH_MS  = 150;     // visual flash duration on hit
const unsigned int ENEMY_WHITE_FLASH_MS  = 90;     // additive white flash at the very start of a melee hit

// stuck / return-to-post tuning
const float ENEMY_STUCK_MOVE_EPS        = 6.0f;    // must move at least this far to not be considered "stuck"
const unsigned int ENEMY_STUCK_TIME_MS  = 600;     // how long it can fail to make progress before giving up and heading home

// knockback (on the enemy, when the player hits it)
const float ENEMY_KNOCKBACK_SPEED_X     = 220.0f;  // px/s, horizontal pop away from the hit
const unsigned int ENEMY_KNOCKBACK_MS   = 120;     // brief - just enough to read as a flinch, then AI resumes
const unsigned int ENEMY_HIT_RECOVER_MS = 400;     // after being hit mid-attack, can't start a new attack for this long

// rendering
const int ENEMY_SPRITE_SCALE            = 5;       // source px -> screen px (same as the player's P_scale)
// footsteps (positional, heard relative to the player)
const float ENEMY_FOOTSTEP_HEAR_RANGE   = 1100.0f; // px from the player; further enemies are silent (saves mixer voices)
const float ENEMY_FOOTSTEP_GAIN         = 0.6f;    // below the player's own steps
const unsigned int ENEMY_FOOTSTEP_MIN_INTERVAL_MS = 300; // floor if the step clip is shorter than this
const float ENEMY_FACE_DEADZONE         = 6.0f;    // ignore tiny left/right offsets so facing doesn't flicker when lined up with the target

// attack timing: the attack hitbox is only live during this slice of the attack animation
// (fractions of frameCount), so there is a wind-up the player can react to and no damage
// before the swing is actually visible
const float ENEMY_ATTACK_ACTIVE_START   = 0.34f;
const float ENEMY_ATTACK_ACTIVE_END     = 0.75f;

// DASH: the enemy itself lunges toward the player during the active window. It covers its attack range
// (plus a little) inside that window, so the speed is derived from the range; this is only the minimum.
const float ENEMY_DASH_SPEED            = 520.0f;  // px/s minimum while the dash is live
const float ENEMY_DASH_OVERSHOOT        = 30.0f;   // px travelled past the attack range if nothing stops it

// Hitbox sizes. The player's hitbox is 40x85; enemies are sized relative to it:
//   small animals (rat, bugs, bats): half the player's height, same width
//   cats: half the player's height, twice the width
const float ENEMY_BASE_W = 40.0f;
const float ENEMY_BASE_H = 85.0f;

// aerial DASH = a diagonal swoop. The flier holds altitude, then dives at least this steep (down per across)...
const float ENEMY_DIVE_MIN_SLOPE = 0.6f;
// ...from anywhere up to this far above the player
const float ENEMY_DIVE_MAX_DROP  = 420.0f;

// POUNCE: ballistic leap (uses ENEMY_GRAVITY). Flight time scales with distance, then the launch is solved to land on the player.
const float ENEMY_POUNCE_SPEED      = 700.0f;  // horizontal px/s used to pick the flight time
const float ENEMY_POUNCE_MIN_FLIGHT = 0.35f;   // s
const float ENEMY_POUNCE_MAX_FLIGHT = 0.80f;   // s
const float ENEMY_POUNCE_MAX_AIR    = 1.5f;    // s safety: the leap ends even if it never lands

// PROJECTILE: a thrown object that follows a ballistic arc (gravity pulls it down) aimed at where the
// player is standing when it leaves the enemy's hand.
const float ENEMY_PROJECTILE_SPEED      = 600.0f;  // horizontal px/s used to pick the flight time
const float ENEMY_PROJECTILE_GRAVITY    = 1400.0f; // px/s^2 (the arc's curvature)
const float ENEMY_PROJECTILE_MIN_FLIGHT = 0.45f;   // s - close throws still arc visibly
const float ENEMY_PROJECTILE_MAX_FLIGHT = 1.30f;   // s
const float ENEMY_PROJECTILE_MAX_LIFE   = 4.0f;    // s before it vanishes if it hits nothing
const float ENEMY_PROJECTILE_SIZE       = 22.0f;   // placeholder rect (px)
const float ENEMY_CHASE_STOP_FRACTION   = 0.8f;    // chase stops once the gap to the player is <= this * attack range

enum class EnemyType{
    //Moss cave enemies
    SPIDER_CHILD,
    MOSS_GLOW_WORM,
    MOSS_BAT,

    //Stone cave enemies
    SINNERS_CHILD,
    CAVE_PREDATOR,
    CAVE_RAT,

    //Wooden Shaft mine enemies
    SHAFTS_RAPTOR,
    GIANT_BATS,
    SHAFTS_WORKER,

    //PLAINS enemies
    ANKYLOSAURUS_CHILD,
    HUNTER_RAPTOR,
    HUNTER_EAGLE,

    //WOODS enemies,
    PREDATOR_THEROPOD,
    THEROPOD_CHILD,
    PREDATOR_CAT,

    //MOUNTAIN enemies,
    MOUNTAIN_CAT,
    CROCODYLOMORPH,
    TARBOSAURUS_CHILD,

    //PEAK enemies,
    TYRANNOSAURUS_REX_CHILD,
    ALBERTOSAURUS,
    DASPELTOSAURUS,

    NONE
};
enum class combatType{
    AERIAL=0,
    GROUND,
    NONE
};
enum class EnemyAction{
    PATROL=0,
    SEEK,
    ATTACK,
    RETURN,
    DEAD,      // death animation: plays once, then the last frame is held (corpse stays where it fell)
    NONE
};
enum class AttackType{
    SLASH=0,
    BITE,
    DASH,
    BEAM,
    PROJECTILE,
    SHOOT,
    POUNCE,   // a cat's leap: crouch, then jump along an arc onto the player (ranks with DASH)
    NONE,
};

// Reach ordering that holds for EVERY enemy (each enemy only picks its own pixel values):
//   bite < slash < dash (or pounce) < shoot < beam < projectile
// The enum order above is used as a table key, so the ordering lives here instead.
inline int attackRank(AttackType t){
    switch(t){
        case AttackType::BITE:       return 0;
        case AttackType::SLASH:      return 1;
        case AttackType::DASH:       return 2;
        case AttackType::POUNCE:     return 2;   // same tier as dash; no enemy owns both
        case AttackType::SHOOT:      return 3;
        case AttackType::BEAM:       return 4;
        case AttackType::PROJECTILE: return 5;
        default:                     return 99;
    }
}
struct EnemyFrameInfo{
    TextureType texture;
    int frameW, frameH;
    int frameCount;      // 0 = work it out from the texture width (width / frameW) - used by the DEAD row
    int frameDelay;
    bool loop;
    int footPad = 0;   // transparent source pixels under the feet in this sheet, so the feet land on the hitbox bottom
};

// a thrown object in flight (PROJECTILE attacks). Hurts the player on contact, vanishes on terrain.
struct EnemyProjectile{
    float x =0.0f, y =0.0f;
    float w =ENEMY_PROJECTILE_SIZE, h =ENEMY_PROJECTILE_SIZE;
    float vx =0.0f, vy =0.0f;
    float age =0.0f;
    EnemyType owner =EnemyType::NONE;
};

struct EnemyAIConfig {
    float detectRadius;
    float loseRadius;
    float leashRadius;
    float attackRange;
    unsigned int attackCooldownMs;

    // The two attacks this enemy owns and the pixel reach of each. getEnemyAIConfig() sorts them by
    // attackRank() and forces attackRanges to be strictly increasing, so [0] is always the shorter-reach
    // attack. updateAI picks the shortest attack that currently reaches the player.
    AttackType attacks[2];        // e.g. {BITE, DASH} for MOSS_BAT
    float attackRanges[2];        // pixel reach of each attack (box length in front of the enemy)
};

const EnemyAIConfig* getEnemyAIConfig(EnemyType type);
const EnemyFrameInfo* getEnemyFrameInfo(EnemyType type,EnemyAction Action,AttackType attackType);

// One landed player hit (melee swing, or a rate-limited tick of the beam). World space.
// GameState turns these into sparks / shake / hit-stop / combo (see GameState::onEnemyHit).
struct EnemyHitEvent{
    float x, y;       // centre of the weapon/enemy overlap
    float dirX;       // +1 / -1: direction the hit pushes the enemy (sparks fly this way)
    bool  killed;     // this hit took the enemy to 0 hp
    bool  isBeam;     // beam tick - sparks/rumble only, no combo or hit-stop
};

struct Enemy{
    Enemy(float x,float y,EnemyType type,EnemyAction action,AttackType attackType,
          float startPath,float endPath,float speed,PathAxis axis,PathShape shape,float radius =0,
          combatType combat =combatType::GROUND);


    float x=0.00f,y=0.00f;
    float w=40.00f,h=85.00f;

    EnemyType type = EnemyType::NONE;
    EnemyAction action =EnemyAction::NONE;
    AttackType attackType =AttackType::NONE;   // attack in use: chosen by updateAI each time an attack starts (ctor value is only the initial one)
    combatType combat =combatType::GROUND;

    int hp = 50;
    // copied from trap , check traps for usage
    float startPath=0.00f,endPath=0.00f;
    float baseX =0.00f,baseY =0.00f;
    float movingSpeed = 150.00f;
    float previousX = 0.00f, previousY = 0.00f;
    float pathAngle = 0.00f;
    float radius = 0.00f;

    // physics, only applied/used when combat==combatType::GROUND
    float velocityY =0.00f;
    bool isGrounded =false;

    int pathIndex = 1;
    bool isMovingForward =true;
    bool isActivated =false;
    bool isFacingRight =true;
    bool hasHitEnd =false;

    bool aniDone= false;

    PathAxis axis = PathAxis::AUTO;
    PathShape pathShape = PathShape::LINE;

    unsigned int lastTime = 0;
    unsigned int lastSwitchTime =0;
    int aniStartFrame =0;
    int aniEndFrame=0;

    // --- attack hitbox (EnemyAction::ATTACK only) ---
    SDL_FRect attackHitBox{0.0f,0.0f,0.0f,0.0f};
    bool isAttackBoxActive =false;
    unsigned int lastAttackHitTime =0;
    bool attackHasHit =false;          // this attack swing has already connected (or whiffed on an invincible player) - one hit per swing
    unsigned int nextAttackTime =0;    // earliest tick a new attack may start (cooldown lives here, not on the damage)
    // DASH: direction is locked when the active window opens; travelled counts px so it stops at its reach
    bool  dashStarted =false;
    float dashDirX =0.0f, dashDirY =0.0f;   // POUNCE reuses dashDirX as the horizontal launch speed (px/s)
    float dashTravelled =0.0f;         // DASH: px covered. POUNCE: seconds in the air
    bool  dashDone =false;             // POUNCE: has landed, the leap is over
    bool  projectileFired =false;      // PROJECTILE: the object has left the hand (one per attack)

    // --- damaged-by-player state ---
    bool isDead =false;
    bool isHurt =false;
    unsigned int hurtFlashEndTime =0;
    unsigned int whiteFlashEndTime =0;   // short additive white flash on a melee hit (hurtFlash stays the red tint)
    unsigned int nextBeamFxTime =0;      // rate limit for beam spark events
    int lastMeleeAttackId =-1;           // id of the last player swing that damaged this enemy (one hit per swing)
    float beamDamageAccum =0.0f;         // fractional beam damage carried between frames (hp is an int)

    // --- knockback (from being hit by the player) ---
    float knockbackVelocityX =0.0f;
    unsigned int knockbackEndTime =0;

    // --- footsteps ---
    unsigned int nextFootstepTime =0;    // earliest tick this enemy may play its next step (keeps steps one after another)

    // --- stuck detection (PATROL path only) ---
    float stuckAnchorX =0.0f, stuckAnchorY =0.0f;
    unsigned int stuckTimerStart =0;
    bool stuckTimerActive =false;

};
class EnemiesBuilder{
public:

    void init(const std::vector<Enemy> enemies);

    void render(SDL_Renderer* renderer);

    void update(float dt,const SDL_FRect& playerRect,
                const SDL_FRect& wallCollisionRect,
                const std::vector<LevelGround>& grounds,
                const std::vector<Platform>& platforms,
                const std::vector<Block>& blocks);

    // Call after computing the player's melee/beam hitboxes for this frame.
    // Damages any overlapping, off-cooldown enemy and removes the ones that die.
    // Returns how many enemies died this call (e.g. to award score).
    // attackId must change once per player swing (see GameState::m_attackId) - that is
    // what limits a swing to a single hit per enemy.
    // knockbackMul scales the melee knockback speed (combo finisher = harder shove).
    int applyPlayerDamage(const SDL_FRect& meleeHitBox, bool meleeActive,
                          const SDL_FRect& beamHitBox, bool beamActive, float dt, int attackId,
                          float knockbackMul = 1.0f);

    // Hits that landed since the last call (cleared on read) - drives the hit VFX.
    std::vector<EnemyHitEvent> consumeHitEvents(){
        std::vector<EnemyHitEvent> out;
        out.swap(m_hitEvents);
        return out;
    }

    // The game freezes for a few ms on impact (hit-stop) but these deadlines are wall-clock
    // (SDL_GetTicks): push them forward by the frozen time so flashes/knockback/cooldowns
    // don't get eaten by the freeze.
    void shiftTimers(unsigned int ms);

    // Call once per frame (while the player isn't already invincible) to see if
    // an attacking enemy's hitbox is touching the player. Returns true and fills
    // outSide (which way to knock the player) on the first hit found.
    // canDamage=false (player invincible, or already hit by something else this frame) still
    // 'spends' an overlapping swing so it can't land later in the same attack.
    bool checkAttackOnPlayer(float playerX, float playerY, float playerW, float playerH,
                             unsigned int now, bool canDamage, gameMath::collisionSide& outSide);

    const std::vector<Enemy>& getEnemies() const { return m_enemies; }
    const std::vector<EnemyProjectile>& getProjectiles() const { return m_projectiles; }

    // Centres (world space) of enemies that were killed since the last call -
    // GameState turns these into a puff of particles. Clears the list.
    std::vector<SDL_FPoint> consumeDeathEvents(){
        std::vector<SDL_FPoint> out;
        out.swap(m_deathEvents);
        return out;
    }

    // ranged attacks may hit from outside melee reach; everything else (SLASH/BITE/DASH) only hurts
    // while the player is inside the enemy's attack hitbox
    static bool isRangedAttack(AttackType t){
        return t == AttackType::PROJECTILE || t == AttackType::SHOOT || t == AttackType::BEAM;
    }
    // the box this enemy's attack would occupy right now (in front of it, `range` long, body height)
    static SDL_FRect getAttackBox(const Enemy& e, float range){
        return { e.isFacingRight ? (e.x + e.w) : (e.x - range), e.y, range, e.h };
    }
    // where the sprite is actually drawn (world space) - for DebugState to compare against the hitbox
    SDL_FRect getSpriteRect(const Enemy& e) const;

private:
    // PROJECTILE attack: throws an object from the enemy's hand, arced so it reaches the player's current position
    void spawnProjectile(const Enemy& e, const SDL_FRect& player);
    void updateProjectiles(float dt, const SDL_FRect& wallCollisionRect,
                           const std::vector<LevelGround>& grounds,
                           const std::vector<Platform>& platforms,
                           const std::vector<Block>& blocks);
    std::vector<EnemyProjectile> m_projectiles;

    void renderPlaceholderBody(SDL_Renderer* renderer, const Enemy& e, int camX, int camY) const;

    // placeholder rects for every attack type (wind-up outline, then filled while the hitbox is live)
    void renderAttackPlaceholders(SDL_Renderer* renderer, int camX, int camY) const;

    // sprite rect in world space: bottom-centre of the frame is anchored to the bottom-centre
    // of the hitbox, so changing sheet/frame size or flipping never shifts the enemy.
    SDL_FRect getEnemyRenderedRect(const Enemy& enemy, const EnemyFrameInfo& info) const;

    // single place to change action: resets animation + per-attack state
    void setAction(Enemy& e, EnemyAction next);
    // killing blow: switches to the DEAD action, freezes AI/attacks and queues a death-puff event
    void startDeath(Enemy& e);
    // sets isFacingRight toward a world-space X (deadzoned)
    void faceToward(Enemy& e, float targetCenterX);
    // keeps ground enemies from stacking on top of each other
    void separateEnemies();

    void moveToward(Enemy &e, float targetX, float targetY, float speed, float dt);

    void updateAI(Enemy &e, const SDL_FRect& player, float dt,
                  const SDL_FRect& wallCollisionRect, const std::vector<Block>& blocks);

    void updatePath(Enemy& enemy, float dt);

    // plays a positional step if this enemy actually walked this frame (ground enemies only)
    void updateFootsteps(Enemy& enemy, float prevX, const SDL_FRect& player, unsigned int now);

    void applyGravityAndCollision(Enemy& enemy, float dt,
                                  const SDL_FRect& wallCollisionRect,
                                  const std::vector<LevelGround>& grounds,
                                  const std::vector<Platform>& platforms,
                                  const std::vector<Block>& blocks);

    // true if the enemy can step to nextX without walking into a wall/block.
    // Only meaningful for combatType::GROUND enemies; aerial enemies are always "clear".
    bool isHorizontalPathClear(const Enemy& e, float nextX,
                               const SDL_FRect& wallCollisionRect,
                               const std::vector<Block>& blocks) const;

    // Tracks whether the enemy has made real progress since the last check.
    // Returns true once it has failed to move for longer than ENEMY_STUCK_TIME_MS.
    bool updateStuckTimer(Enemy& e, unsigned int now);

private:
    std::vector<Enemy> m_enemies;
    std::vector<SDL_FPoint> m_deathEvents;
    std::vector<EnemyHitEvent> m_hitEvents;

};