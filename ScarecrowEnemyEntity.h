#pragma once

#include "EnemyEntity.h"

class HPComponent;

class ScarecrowEnemyEntity : public EnemyEntity
{
public:
    ScarecrowEnemyEntity(Scene* scene, const Vector2d& pos);

    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void KnockBack(const Vector2d& attackDir,float power);
    void Launch(const Vector2d& attackDir, float power);
    float m_knockBackTimer = 0.0f;
private:
    HPComponent* m_hp = nullptr;
    VelocityComponent* m_velocity = nullptr;
    float m_damageTimer = 0.0f;
    int m_damagePerSecond = 10;
    static constexpr int DUMMY_HP = 999999;
};