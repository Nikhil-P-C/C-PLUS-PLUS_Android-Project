//
// Created by LENOVO on 08-09-2026.
//
#include "Enemies.h"


namespace {
    inline uint32_t enemyActionKey(EnemyType type, EnemyAction action)
    {
        return (static_cast<uint32_t>(type) << 8 | static_cast<uint32_t>(action));
    }
    inline uint32_t enemyAttackKey(EnemyType type, AttackType attack)
    {
        return (static_cast<uint32_t>(type) << 8 | static_cast<uint32_t>(attack));
    }
}


void EnemiesBuilder::render(SDL_Renderer *renderer) {

}

void EnemiesBuilder::update(float dt) {

}


void EnemiesBuilder::init(const std::vector<Enemy> enemies) {
    m_enemies.emplace_back(0.0f,0.0f,EnemyType::SINNERS_CHILD,EnemyAction::PATROL,AttackType::NONE,
                           0.00f,0.00f,250.00f,PathAxis::HORIZONTAL,PathShape::LINE);
}

Enemy::Enemy(float x, float y, EnemyType type, EnemyAction action, AttackType attackType,
             float startPath, float endPath, float speed, PathAxis axis, PathShape shape,
             float radius)
             :x(x),y(y),type(type),action(action),attackType(attackType),startPath(startPath),endPath(endPath),
             movingSpeed(speed),axis(axis),pathShape(shape),radius(radius),baseX(x),baseY(y),previousX(x),previousY(y){

}

const EnemyFrameInfo* getEnemyFrameInfo(EnemyType type, EnemyAction Action, AttackType attackType) {
    if(Action == EnemyAction::ATTACK){
        static const std::unordered_map<uint32_t, EnemyFrameInfo> attackTable{
                {enemyAttackKey(EnemyType::SPIDER_CHILD,AttackType::SHOOT), {TextureType::ENEMY_SPIDER_CHILD_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::SPIDER_CHILD,AttackType::BITE), {TextureType::ENEMY_SPIDER_CHILD_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::MOSS_GLOW_WORM,AttackType::DASH), {TextureType::ENEMY_MOSS_GLOW_WORM_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::MOSS_GLOW_WORM,AttackType::BITE), {TextureType::ENEMY_MOSS_GLOW_WORM_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::MOSS_BAT,AttackType::SLASH), {TextureType::ENEMY_MOSS_BAT_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::MOSS_BAT,AttackType::BITE), {TextureType::ENEMY_MOSS_BAT_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::SINNERS_CHILD,AttackType::SLASH), {TextureType::ENEMY_SINNERS_CHILD_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::SINNERS_CHILD,AttackType::BEAM), {TextureType::ENEMY_SINNERS_CHILD_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::CAVE_PREDATOR,AttackType::SLASH), {TextureType::ENEMY_CAVE_PREDATOR_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::CAVE_PREDATOR,AttackType::BITE), {TextureType::ENEMY_CAVE_PREDATOR_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::CAVE_RAT,AttackType::DASH), {TextureType::ENEMY_CAVE_RAT_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::CAVE_RAT,AttackType::BITE), {TextureType::ENEMY_CAVE_RAT_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::SHAFTS_RAPTOR,AttackType::SLASH), {TextureType::ENEMY_SHAFTS_RAPTOR_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::SHAFTS_RAPTOR,AttackType::BITE), {TextureType::ENEMY_SHAFTS_RAPTOR_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::GIANT_BATS,AttackType::SLASH), {TextureType::ENEMY_GIANT_BATS_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::GIANT_BATS,AttackType::BITE), {TextureType::ENEMY_GIANT_BATS_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::SHAFTS_WORKER,AttackType::SLASH), {TextureType::ENEMY_SHAFTS_WORKER_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::SHAFTS_WORKER,AttackType::PROJECTILE), {TextureType::ENEMY_SHAFTS_WORKER_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::HUNTER_RAPTOR,AttackType::BITE), {TextureType::ENEMY_HUNTER_RAPTOR_ATTACK2,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::HUNTER_RAPTOR,AttackType::SLASH), {TextureType::ENEMY_HUNTER_RAPTOR_ATTACK1,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::HUNTER_EAGLE,AttackType::SLASH), {TextureType::ENEMY_HUNTER_EAGLE_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::HUNTER_EAGLE,AttackType::DASH), {TextureType::ENEMY_HUNTER_EAGLE_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::PREDATOR_THEROPOD,AttackType::SLASH), {TextureType::ENEMY_PREDATOR_THEROPOD_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::PREDATOR_THEROPOD,AttackType::BITE), {TextureType::ENEMY_PREDATOR_THEROPOD_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::ANKYLOSAURUS_CHILD,AttackType::SLASH), {TextureType::ENEMY_ANKYLOSAURUS_CHILD_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::ANKYLOSAURUS_CHILD,AttackType::BITE), {TextureType::ENEMY_ANKYLOSAURUS_CHILD_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::THEROPOD_CHILD,AttackType::SLASH), {TextureType::ENEMY_THEROPOD_CHILD_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::THEROPOD_CHILD,AttackType::BITE), {TextureType::ENEMY_THEROPOD_CHILD_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::PREDATOR_CAT,AttackType::SLASH), {TextureType::ENEMY_PREDATOR_CAT_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::PREDATOR_CAT,AttackType::BITE), {TextureType::ENEMY_PREDATOR_CAT_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::MOUNTAIN_CAT,AttackType::DASH), {TextureType::ENEMY_MOUNTAIN_CAT_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::MOUNTAIN_CAT,AttackType::BITE), {TextureType::ENEMY_MOUNTAIN_CAT_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::CROCODYLOMORPH,AttackType::SLASH), {TextureType::ENEMY_CROCODYLOMORPH_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::CROCODYLOMORPH,AttackType::BITE), {TextureType::ENEMY_CROCODYLOMORPH_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::TARBOSAURUS_CHILD,AttackType::DASH), {TextureType::ENEMY_TARBOSAURUS_CHILD_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::TARBOSAURUS_CHILD,AttackType::BITE), {TextureType::ENEMY_TARBOSAURUS_CHILD_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::TYRANNOSAURUS_REX_CHILD,AttackType::BEAM), {TextureType::ENEMY_TYRANNOSAURUS_REX_CHILD_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::TYRANNOSAURUS_REX_CHILD,AttackType::BITE), {TextureType::ENEMY_TYRANNOSAURUS_REX_CHILD_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::ALBERTOSAURUS,AttackType::SLASH), {TextureType::ENEMY_ALBERTOSAURUS_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::ALBERTOSAURUS,AttackType::BITE), {TextureType::ENEMY_ALBERTOSAURUS_ATTACK2,64,64,6,80,false}},

                {enemyAttackKey(EnemyType::DASPELTOSAURUS,AttackType::SLASH), {TextureType::ENEMY_DASPELTOSAURUS_ATTACK1,64,64,6,80,false}},
                {enemyAttackKey(EnemyType::DASPELTOSAURUS,AttackType::BITE), {TextureType::ENEMY_DASPELTOSAURUS_ATTACK2,64,64,6,80,false}},
        };
        auto it = attackTable.find(enemyAttackKey(type, attackType));
        return it != attackTable.end() ? &it->second : nullptr;
    }

    static const std::unordered_map<uint32_t, EnemyFrameInfo> table{
            {enemyActionKey(EnemyType::SPIDER_CHILD,EnemyAction::PATROL), {TextureType::ENEMY_SPIDER_CHILD_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::SPIDER_CHILD,EnemyAction::SEEK), {TextureType::ENEMY_SPIDER_CHILD_SEEK,64,64,4,90,true}},

            {enemyActionKey(EnemyType::MOSS_GLOW_WORM,EnemyAction::PATROL), {TextureType::ENEMY_MOSS_GLOW_WORM_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::MOSS_GLOW_WORM,EnemyAction::SEEK), {TextureType::ENEMY_MOSS_GLOW_WORM_SEEK,64,64,4,90,true}},

            {enemyActionKey(EnemyType::MOSS_BAT,EnemyAction::PATROL), {TextureType::ENEMY_MOSS_BAT_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::MOSS_BAT,EnemyAction::SEEK), {TextureType::ENEMY_MOSS_BAT_SEEK,64,64,4,90,true}},

            {enemyActionKey(EnemyType::SINNERS_CHILD,EnemyAction::PATROL), {TextureType::ENEMY_SINNERS_CHILD_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::SINNERS_CHILD,EnemyAction::SEEK), {TextureType::ENEMY_SINNERS_CHILD_SEEK,64,64,4,90,true}},

            {enemyActionKey(EnemyType::CAVE_PREDATOR,EnemyAction::PATROL), {TextureType::ENEMY_CAVE_PREDATOR_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::CAVE_PREDATOR,EnemyAction::SEEK), {TextureType::ENEMY_CAVE_PREDATOR_SEEK,64,64,4,90,true}},

            {enemyActionKey(EnemyType::CAVE_RAT,EnemyAction::PATROL), {TextureType::ENEMY_CAVE_RAT_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::CAVE_RAT,EnemyAction::SEEK), {TextureType::ENEMY_CAVE_RAT_SEEK,64,64,4,90,true}},

            {enemyActionKey(EnemyType::SHAFTS_RAPTOR,EnemyAction::PATROL), {TextureType::ENEMY_SHAFTS_RAPTOR_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::SHAFTS_RAPTOR,EnemyAction::SEEK), {TextureType::ENEMY_SHAFTS_RAPTOR_SEEK,64,64,4,90,true}},

            {enemyActionKey(EnemyType::GIANT_BATS,EnemyAction::PATROL), {TextureType::ENEMY_GIANT_BATS_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::GIANT_BATS,EnemyAction::SEEK), {TextureType::ENEMY_GIANT_BATS_SEEK,64,64,4,90,true}},

            {enemyActionKey(EnemyType::SHAFTS_WORKER,EnemyAction::PATROL), {TextureType::ENEMY_SHAFTS_WORKER_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::SHAFTS_WORKER,EnemyAction::SEEK), {TextureType::ENEMY_SHAFTS_WORKER_SEEK,64,64,4,90,true}},

            {enemyActionKey(EnemyType::ANKYLOSAURUS_CHILD,EnemyAction::PATROL), {TextureType::ENEMY_ANKYLOSAURUS_CHILD_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::ANKYLOSAURUS_CHILD,EnemyAction::SEEK), {TextureType::ENEMY_ANKYLOSAURUS_CHILD_SEEK,64,64,4,90,true}},

            {enemyActionKey(EnemyType::HUNTER_RAPTOR,EnemyAction::PATROL), {TextureType::ENEMY_HUNTER_RAPTOR_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::HUNTER_RAPTOR,EnemyAction::SEEK), {TextureType::ENEMY_HUNTER_RAPTOR_SEEK,64,64,4,90,true}},

            {enemyActionKey(EnemyType::HUNTER_EAGLE,EnemyAction::PATROL), {TextureType::ENEMY_HUNTER_EAGLE_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::HUNTER_EAGLE,EnemyAction::SEEK), {TextureType::ENEMY_HUNTER_EAGLE_SEEK,64,64,4,90,true}},

            {enemyActionKey(EnemyType::PREDATOR_THEROPOD,EnemyAction::PATROL), {TextureType::ENEMY_PREDATOR_THEROPOD_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::PREDATOR_THEROPOD,EnemyAction::SEEK), {TextureType::ENEMY_PREDATOR_THEROPOD_SEEK,64,64,4,90,true}},

            {enemyActionKey(EnemyType::THEROPOD_CHILD,EnemyAction::PATROL), {TextureType::ENEMY_THEROPOD_CHILD_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::THEROPOD_CHILD,EnemyAction::SEEK), {TextureType::ENEMY_THEROPOD_CHILD_SEEK,64,64,4,90,true}},

            {enemyActionKey(EnemyType::PREDATOR_CAT,EnemyAction::PATROL), {TextureType::ENEMY_PREDATOR_CAT_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::PREDATOR_CAT,EnemyAction::SEEK), {TextureType::ENEMY_PREDATOR_CAT_SEEK,64,64,4,90,true}},

            {enemyActionKey(EnemyType::MOUNTAIN_CAT,EnemyAction::PATROL), {TextureType::ENEMY_MOUNTAIN_CAT_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::MOUNTAIN_CAT,EnemyAction::SEEK), {TextureType::ENEMY_MOUNTAIN_CAT_SEEK,64,64,4,90,true}},

            {enemyActionKey(EnemyType::CROCODYLOMORPH,EnemyAction::PATROL), {TextureType::ENEMY_CROCODYLOMORPH_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::CROCODYLOMORPH,EnemyAction::SEEK), {TextureType::ENEMY_CROCODYLOMORPH_SEEK,64,64,4,90,true}},

            {enemyActionKey(EnemyType::TARBOSAURUS_CHILD,EnemyAction::PATROL), {TextureType::ENEMY_TARBOSAURUS_CHILD_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::TARBOSAURUS_CHILD,EnemyAction::SEEK), {TextureType::ENEMY_TARBOSAURUS_CHILD_SEEK,64,64,4,90,true}},

            {enemyActionKey(EnemyType::TYRANNOSAURUS_REX_CHILD,EnemyAction::PATROL), {TextureType::ENEMY_TYRANNOSAURUS_REX_CHILD_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::TYRANNOSAURUS_REX_CHILD,EnemyAction::SEEK), {TextureType::ENEMY_TYRANNOSAURUS_REX_CHILD_SEEK,64,64,4,90,true}},

            {enemyActionKey(EnemyType::ALBERTOSAURUS,EnemyAction::PATROL), {TextureType::ENEMY_ALBERTOSAURUS_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::ALBERTOSAURUS,EnemyAction::SEEK), {TextureType::ENEMY_ALBERTOSAURUS_SEEK,64,64,4,90,true}},

            {enemyActionKey(EnemyType::DASPELTOSAURUS,EnemyAction::PATROL), {TextureType::ENEMY_DASPELTOSAURUS_PATROL,64,64,4,120,true}},
            {enemyActionKey(EnemyType::DASPELTOSAURUS,EnemyAction::SEEK), {TextureType::ENEMY_DASPELTOSAURUS_SEEK,64,64,4,90,true}},
    };
    auto it = table.find(enemyActionKey(type, Action));
    return it != table.end() ? &it->second : nullptr;
}

const EnemyAIConfig* getEnemyAIConfig(EnemyType type) {
    static const std::unordered_map<EnemyType, EnemyAIConfig> table{
            {EnemyType::SPIDER_CHILD, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::SHOOT},{60.f,40.f}}},
            {EnemyType::MOSS_GLOW_WORM, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::DASH},{60.f,40.f}}},
            {EnemyType::MOSS_BAT, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::SLASH},{60.f,40.f}}},
            {EnemyType::SINNERS_CHILD, {300.f,400.f,500.f,60.f,1500u,{AttackType::SLASH,AttackType::BEAM},{60.f,40.f}}},
            {EnemyType::CAVE_PREDATOR, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::SLASH},{60.f,40.f}}},
            {EnemyType::CAVE_RAT, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::DASH},{60.f,40.f}}},
            {EnemyType::SHAFTS_RAPTOR, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::SLASH},{60.f,40.f}}},
            {EnemyType::GIANT_BATS, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::SLASH},{60.f,40.f}}},
            {EnemyType::SHAFTS_WORKER, {300.f,400.f,500.f,60.f,1500u,{AttackType::SLASH,AttackType::PROJECTILE},{60.f,40.f}}},
            {EnemyType::ANKYLOSAURUS_CHILD, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::SLASH},{60.f,40.f}}},
            {EnemyType::HUNTER_RAPTOR, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::SLASH},{60.f,40.f}}},
            {EnemyType::HUNTER_EAGLE, {300.f,400.f,500.f,60.f,1500u,{AttackType::SLASH,AttackType::DASH},{60.f,40.f}}},
            {EnemyType::PREDATOR_THEROPOD, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::SLASH},{60.f,40.f}}},
            {EnemyType::THEROPOD_CHILD, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::SLASH},{60.f,40.f}}},
            {EnemyType::PREDATOR_CAT, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::SLASH},{60.f,40.f}}},
            {EnemyType::MOUNTAIN_CAT, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::DASH},{60.f,40.f}}},
            {EnemyType::CROCODYLOMORPH, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::SLASH},{60.f,40.f}}},
            {EnemyType::TARBOSAURUS_CHILD, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::DASH},{60.f,40.f}}},
            {EnemyType::TYRANNOSAURUS_REX_CHILD, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::BEAM},{60.f,40.f}}},
            {EnemyType::ALBERTOSAURUS, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::SLASH},{60.f,40.f}}},
            {EnemyType::DASPELTOSAURUS, {300.f,400.f,500.f,60.f,1500u,{AttackType::BITE,AttackType::SLASH},{60.f,40.f}}},
    };
    auto it = table.find(type);
    return it != table.end() ? &it->second : nullptr;
}
