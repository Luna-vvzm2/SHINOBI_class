#pragma once
#include "BossEntity.h"

class SekienkiBossEntity : public BossEntity
{
public:

    SekienkiBossEntity(Scene* scene, const Vector2d& pos, const Vector2d& size);

    bool Init() override;
    int GetMaxHP() const override { return 1000; }
    void StartJumpAttack();
    void PhaseChange();
protected:

    void UpdateAI(float deltaTime) override;
    void UpdateAttack(float deltaTime) override;

private:

    int m_attackStep;
    int m_phase;

    float m_attackTimer;
    float m_jumpAttackCooldown;

    float m_rollDistanceLeft;

    bool m_bulletActive;
    float m_fallStartY;
    float m_hoverY;
    bool m_secondJump;
    float m_tornadoDistanceLeft;

    Vector2d m_bulletPos;
    Vector2d m_bulletVel;
};

