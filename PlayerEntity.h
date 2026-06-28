#pragma once
#include "EntityActor.h"
#include "Vector2d.h"

struct AttackHitbox
{
    Vector2d offset;
    float width = 0;
    float height = 0;
    int damage = 0;
};

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
    void ChangeState(ActionState newState);


    ActorType GetType() const override { return ActorType::Player; }
    CollisionComponent* GetCollision() const { return m_collision; }
    CollisionComponent* GetAttackCol() const { return m_attackCol; }

    bool OnGround() const { return m_isGround; }
    void SetCanMove(bool canMove) { m_canMove = canMove; }

    HPComponent* GetHP() const { return m_hp; }
    bool GetDir() const { return m_dir; }
    int GetCombo() const { return m_combo; }

    void CheckAttackHit(const AttackHitbox& hitbox);
    void TakeDamage(int damage, const Vector2d& knockback);
    void HitTrap();

    void CheckCanStand();
    void EnterSquat();
    void ExitSquat();

    void SpawnKunai();

    enum class AttackType {
        NONE,
        WEAK_ATTACK,
        STRONG_ATTACK,
        KUNAI,
        SQUAT_ATTACK,
        SQUAT_KUNAI,
        AIR_ATTACK,
        AIR_KUNAI,
        WALL_ATTACK,
        WALL_KUNAI,
        HAYABUSA,
    };

private:
    HPComponent* m_hp;
    GravityComponent* m_gravity;
    //SpriteComponent* m_sprite;
    AnimationComponent* m_anim;

    int m_combo;
    int m_kunai;

    float m_kunaiSpawnTimer;
    bool m_kunaiPending;

    bool m_dir;
    bool m_prevDir;
    float m_jumpSpeed;    // ジャンプ速度
    float m_moveSpeed;    // 移動速度
    float m_dashSpeed;
    // EntityActor.bool m_isGround;          // 接地フラグ
    int m_jumpCount;
    float m_jumpTime;
    float m_maxJumpTime;

    bool m_attack;
    bool m_hit;           // 自身の攻撃が当たったかどうか
    bool m_HayabusaHit;
    AttackType m_attackType;
    CollisionComponent* m_attackCol;

    int m_weakAttackIdx;
    int m_strongAttackIdx;
    int m_airAttackIdx;

    float m_attackTimer;

    bool m_canAttack;
    float m_attackLockTimer;

    bool m_getHit;
    float m_getHitTimer;
    float m_invincibleTime;

    bool m_canMove;       // 移動可否
    bool m_squat;         // しゃがみ
    bool m_canStand;      // しゃがみ可否
    bool m_canCharge;


    std::string GetTexturePath() const override;

};



