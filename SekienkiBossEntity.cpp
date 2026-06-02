#include "SekienkiBossEntity.h"
#include "HPComponent.h"

SekienkiBossEntity::SekienkiBossEntity(Scene* scene, const Vector2d& pos, const Vector2d& size)
    : BossEntity(scene, pos, size)
{
}

bool SekienkiBossEntity::Init()
{
    if (!BossEntity::Init())
        return false;

    return true;
}

void SekienkiBossEntity::UpdateAI(float deltaTime)
{
    // 赤猿鬼ボス専用AI
}

void SekienkiBossEntity::UpdateAttack(float deltaTime)
{
    // 赤猿鬼ボス専用攻撃
}