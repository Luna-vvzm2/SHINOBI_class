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
private:
    enum class ScarecrowState
    {
        Idle,       // 待機
        Hit,        // 弱攻撃でのけぞる
        Launch,     // 強攻撃で吹っ飛ぶ
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
    void UpdateHit(float dt);
    void UpdateLaunch(float dt);
    void UpdateDown(float dt);
    void UpdateRecover(float dt);
};