#pragma once
#include "BossEntity.h"

class YoroiBossEntity : public BossEntity
{
public:

    YoroiBossEntity(Scene* scene, const Vector2d& pos, const Vector2d& size);

    bool Init() override;
    int GetMaxHP() const override { return 1000; }
protected:

    void UpdateAI(float deltaTime) override;
    void UpdateAttack(float deltaTime) override;

private:
    // --- 状態管理用変数 ---
    float m_attackTimer;
    float m_rollDistanceLeft;
    int   m_attackStep;
    int   m_darkAttackCount;
    int   m_maxDarkAttacks;

    // 弾（斬撃）関連
    bool     m_bulletActive;
    Vector2d m_bulletPos;
    Vector2d m_bulletVel;
};

