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
    void Hit(const Vector2d& dir);
    void Launch(const Vector2d& dir, float power);
    void KnockBack(const Vector2d& attackDir, float power);
    void TakeDamage(int damage, const Vector2d& knockback);
    void KnockBack(const Vector2d& velocity);

private:
    enum class ScarecrowState
    {
        Idle,       // 待機
        BlowHit,
        Down,       // ダウン
        Recover     // 起き上がり・待機へ戻る
    };
    ScarecrowState m_state = ScarecrowState::Idle;
    float m_stateTimer = 0.0f;
    AnimationComponent* m_anim = nullptr;
    HPComponent* m_hp = nullptr;
    VelocityComponent* m_velocity = nullptr;
    float m_damageTimer = 0.0f;
    int m_damagePerSecond = 10;
    static constexpr int DUMMY_HP = 999999;
    void UpdateIdle(float dt);
    void UpdateBlowHit(float dt);
    void UpdateDown(float dt);
    void UpdateRecover(float dt);
    float m_hitTimer = 0.0f;
    bool m_isHit = false;
};