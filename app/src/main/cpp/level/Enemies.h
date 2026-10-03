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

// stuck / return-to-post tuning
const float ENEMY_STUCK_MOVE_EPS        = 6.0f;    // must move at least this far to not be considered "stuck"
const unsigned int ENEMY_STUCK_TIME_MS  = 600;     // how long it can fail to make progress before giving up and heading home

// knockback (on the enemy, when the player hits it)
const float ENEMY_KNOCKBACK_SPEED_X     = 220.0f;  // px/s, horizontal pop away from the hit
const unsigned int ENEMY_KNOCKBACK_MS   = 120;     // brief - just enough to read as a flinch, then AI resumes
const unsigned int ENEMY_HIT_RECOVER_MS = 400;     // after being hit mid-attack, can't start a new attack for this long

// rendering
const int ENEMY_SPRITE_SCALE            = 5;       // source px -> screen px (same as the player's P_scale)
const float ENEMY_FACE_DEADZONE         = 6.0f;    // ignore tiny left/right offsets so facing doesn't flicker when lined up with the target

// attack timing: the attack hitbox is only live during this slice of the attack animation
// (fractions of frameCount), so there is a wind-up the player can react to and no damage
// before the swing is actually visible
const float ENEMY_ATTACK_ACTIVE_START   = 0.34f;
const float ENEMY_ATTACK_ACTIVE_END     = 0.75f;

// melee lunge: DASH attacks (the tackle) carry the enemy forward during the active window
const float ENEMY_DASH_SPEED            = 520.0f;  // px/s while the dash is live
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
    NONE
};
enum class AttackType{
    SLASH=0,
    BITE,
    DASH,
    BEAM,
    PROJECTILE,
    SHOOT,
    NONE,
};
struct EnemyFrameInfo{
    TextureType texture;
    int frameW, frameH;
    int frameCount;
    int frameDelay;
    bool loop;
    int footPad = 0;   // transparent source pixels under the feet in this sheet, so the feet land on the hitbox bottom
};

struct EnemyAIConfig {
    float detectRadius;
    float loseRadius;
    float leashRadius;
    float attackRange;
    unsigned int attackCooldownMs;

    AttackType attacks[2];        // e.g. {BITE, DASH} for MOSS_BAT
    float attackRanges[2];        // range each attack triggers at
};

const EnemyAIConfig* getEnemyAIConfig(EnemyType type);
const EnemyFrameInfo* getEnemyFrameInfo(EnemyType type,EnemyAction Action,AttackType attackType);

struct Enemy{
    Enemy(float x,float y,EnemyType type,EnemyAction action,AttackType attackType,
          float startPath,float endPath,float speed,PathAxis axis,PathShape shape,float radius =0,
          combatType combat =combatType::GROUND);


    float x=0.00f,y=0.00f;
    float w=40.00f,h=85.00f;

    EnemyType type = EnemyType::NONE;
    EnemyAction action =EnemyAction::NONE;
    AttackType attackType =AttackType::NONE;
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

    // --- damaged-by-player state ---
    bool isDead =false;
    bool isHurt =false;
    unsigned int hurtFlashEndTime =0;
    int lastMeleeAttackId =-1;           // id of the last player swing that damaged this enemy (one hit per swing)
    float beamDamageAccum =0.0f;         // fractional beam damage carried between frames (hp is an int)

    // --- knockback (from being hit by the player) ---
    float knockbackVelocityX =0.0f;
    unsigned int knockbackEndTime =0;

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
    int applyPlayerDamage(const SDL_FRect& meleeHitBox, bool meleeActive,
                          const SDL_FRect& beamHitBox, bool beamActive, float dt, int attackId);

    // Call once per frame (while the player isn't already invincible) to see if
    // an attacking enemy's hitbox is touching the player. Returns true and fills
    // outSide (which way to knock the player) on the first hit found.
    // canDamage=false (player invincible, or already hit by something else this frame) still
    // 'spends' an overlapping swing so it can't land later in the same attack.
    bool checkAttackOnPlayer(float playerX, float playerY, float playerW, float playerH,
                             unsigned int now, bool canDamage, gameMath::collisionSide& outSide);

    const std::vector<Enemy>& getEnemies() const { return m_enemies; }

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
    // sprite rect in world space: bottom-centre of the frame is anchored to the bottom-centre
    // of the hitbox, so changing sheet/frame size or flipping never shifts the enemy.
    SDL_FRect getEnemyRenderedRect(const Enemy& enemy, const EnemyFrameInfo& info) const;

    // single place to change action: resets animation + per-attack state
    void setAction(Enemy& e, EnemyAction next);
    // sets isFacingRight toward a world-space X (deadzoned)
    void faceToward(Enemy& e, float targetCenterX);
    // keeps ground enemies from stacking on top of each other
    void separateEnemies();

    void moveToward(Enemy &e, float targetX, float targetY, float speed, float dt);

    void updateAI(Enemy &e, const SDL_FRect& player, float dt,
                 const SDL_FRect& wallCollisionRect, const std::vector<Block>& blocks);

    void updatePath(Enemy& enemy, float dt);

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

};