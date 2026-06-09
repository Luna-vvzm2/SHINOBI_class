#pragma once
#include "EntityActor.h"
#include "Vector2d.h"

class TransformComponent;
class VelocityComponent;
class GravityComponent;
class SpriteComponent;
class CollisionComponent;
class AnimationComponent;
class HPComponent;

class PlayerEntity : public EntityActor {
public:
    explicit PlayerEntity(Scene* scene, const Vector2d& pos = Vector2d::Zero(), const Vector2d& size = { 32,32 });
    ~PlayerEntity() override = default;

    bool Init() override;
    void Update(float deltaTime) override;

    void UpdateInvincible(float deltaTime);

    void UpdateMove(float deltaTime);
    void UpdateJump(float deltaTime);
    void UpdateGravity(float deltaTime);
    void UpdateAttack(float deltaTime);
    void UpdateState();


    ActorType GetType() const override { return ActorType::Player; }
    CollisionComponent* GetCollision() const { return m_collision; }

    bool OnGround() const { return m_isGround; }
    void SetCanMove(bool canMove) { m_canMove = canMove; }

    HPComponent* GetHP() const { return m_hp; }

    void TakeDamage(int damage, const Vector2d& knockback);

    enum class AttackType {
        WEAK_ATTACK,
        STRONG_ATTACK,
        
    };
private:
    HPComponent* m_hp;
    GravityComponent* m_gravity;
    //SpriteComponent* m_sprite;
    AnimationComponent* m_anim;

    bool m_dir;
    float m_jumpSpeed;    // ジャンプ速度
    float m_moveSpeed;    // 移動速度
    // EntityActor.bool m_isGround;          // 接地フラグ
    int m_jumpCount;
    float m_jumpTime;
    float m_maxJumpTime;

    bool m_attack;
    AttackType m_attackType;

    int m_weakAttackIdx;
    int m_strongAttackIdx;

    float m_attackTimer;

    float m_hitTimer;
    float m_invincibleTime;

    bool m_canMove;       // 移動可否
    bool m_squat;         // しゃがみ
    bool m_canCharge;

    std::string GetTexturePath() const override;

};