#include "YoroiBossEntity.h"
#include "HPComponent.h"

YoroiBossEntity::YoroiBossEntity(Scene* scene, const Vector2d& pos, const Vector2d& size)
    : BossEntity(scene, pos, size)
{
}

bool YoroiBossEntity::Init()
{
    if (!BossEntity::Init())
        return false;

    return true;
}

void YoroiBossEntity::UpdateAI(float deltaTime)
{
    // 鎧ボス専用AI
}

void YoroiBossEntity::UpdateAttack(float deltaTime)
{
    // 鎧ボス専用攻撃
}