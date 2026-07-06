#pragma once
#include "EnemyEntity.h"
#include "BlockActor.h"
#include "Vector2d.h"
#include "SpriteComponent.h"
#include "TransformComponent.h"
#include <functional>

struct AttackHitbox
{
    Vector2d offset;
    float width = 0;
    float height = 0;
    int damage = 0;
    int metsu = 0;
};

struct SensorData
{
    BlockActor* front = nullptr;
    BlockActor* frontUpper = nullptr;
    BlockActor* frontBottom = nullptr;
    BlockActor* frontGround = nullptr;
    BlockActor* frontNearGround = nullptr;
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

    void UpdateIgnorePlatform();
    void UpdateInvincible(float deltaTime);
    void UpdateSensor();

    void UpdateKamae(float deltaTime);
    void UpdateExecution(float deltaTime);
    void CollectExecutionTargets();

    void UpdateMove(float deltaTime);
    void UpdateJump(float deltaTime);
    void UpdateGravity(float deltaTime);
    void UpdateAttack(float deltaTime);
    void UpdateState();
    void ChangeState(ActionState newState);
    void UpdateDead(float deltaTime); // 死亡時の専用アップデート

    ActorType GetType() const override { return ActorType::Player; }
    CollisionComponent* GetCollision() const { return m_collision; }
    CollisionComponent* GetAttackCol() const { return m_attackCol; }

    bool OnGround() const { return m_isGround; }
    void SetCanMove(bool canMove) { m_canMove = canMove; }
    void SetPosition(Vector2d pos) { m_transform->SetPosition(pos); }

    HPComponent* GetHP() const { return m_hp; }
    bool GetDir() const { return m_dir; }
    int GetCombo() const { return m_combo; }

    bool IgnorePlatform() const override { return m_ignorePlatform; }
    void SetIgnorePlatform(BlockActor* block)
    {
        m_ignorePlatform = true;
        m_ignorePlatformBlock = block;
    }

    void CheckAttackHit(const AttackHitbox& hitbox);
    void TakeDamage(int damage, const Vector2d& knockback);
    void HitTrap();

    void CheckCanStand();
    BlockActor* CheckSensor(const Vector2d& offset, const Vector2d& size = {4.0f, 4.0f});

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

    Vector2d GetDrawOffset() const { return m_sprite->GetDrawOffset(); }
    // ★追加: 手裏剣の数を返す関数
    int GetShurikenCount() const { return m_kunai; }

    std::function<void(int newMoney, int oldMoney)> OnMoneyChanged; // コールバック
    int GetMoney() const { return m_money; }
    void SetMoney(int amount);
    void AddMoney(int delta);

    void ResetStageState();

    // ===== ドロップアイテム用 =====
    void AddCoin(int value);
    void AddKunai(int value);
    void AddHaku(int value);
    void HealHP(int value);

    int GetCoin() const { return m_coin; }
    int GetKunai() const { return m_kunai; }
    int GetHaku() const { return m_haku; }
    int GetMaxHaku() const { return m_maxHaku; }

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
    float m_dashAirSpeed;
    float m_dashTimer;
    bool m_HienCount;
    bool m_isSenten;

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

    bool m_isKamae;
    bool m_isExecution;
    std::vector<EnemyEntity*> m_executionTargets;
    float m_executionTimer;

    bool m_ignorePlatform;
    BlockActor* m_ignorePlatformBlock = nullptr;

    SensorData m_sensor;

    bool m_isDeadTriggered = false; // 死亡時の初回処理用フラグ

    // ===== ドロップアイテム用の所持値 =====
    int m_money = 0;
    int m_coin;
    int m_haku;
    int m_maxHaku;

    std::string GetTexturePath() const override;

};



