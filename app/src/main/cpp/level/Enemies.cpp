//
// Created by LENOVO on 08-09-2026.
//
#include "Enemies.h"



Enemy::Enemy(float x, float y, EnemyType type, EnemyAction action, AttackType attackType,
             float startPath, float endPath, float speed, PathAxis axis, PathShape shape,
             float radius)
             :x(x),y(y),type(type),action(action),attackType(attackType),startPath(startPath),endPath(endPath),
             movingSpeed(speed),axis(axis),pathShape(shape),radius(radius),baseX(x),baseY(y),previousX(x),previousY(y){

}

const EnemyFrameInfo* getEnemyFrameInfo(EnemyType type, EnemyAction Action, AttackType attackType) {

    return nullptr;
}

const EnemyAIConfig* getEnemyAIConfig(EnemyType type) {
    return nullptr;
}
