#pragma once
#include "EntityActor.h"
#include "Vector2d.h"
#include <functional>

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

    void UpdateMove(float deltaTime);
    void UpdateJump(float deltaTime);
    void UpdateGravity(float deltaTime);
    void UpdateAttack(float deltaTime);
    void UpdateState();
    void UpdateDead(float deltaTime); // 死亡時の専用アップデート

    ActorType GetType() const override { return ActorType::Player; }
    CollisionComponent* GetCollision() const { return m_collision; }

    bool OnGround() const { return m_isGround; }
    void SetCanMove(bool canMove) { m_canMove = canMove; }

    HPComponent* GetHP() const { return m_hp; }

    // ★追加: 手裏剣の数を返す関数
    int GetShurikenCount() const { return m_shurikenCount; }

    std::function<void(int newMoney, int oldMoney)> OnMoneyChanged; // コールバック
    int GetMoney() const { return m_money; }
    void SetMoney(int amount);
    void AddMoney(int delta);
    
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
    int m_attackType;

    int m_weakAttackIdx;
    int m_strongAttackIdx;

    float m_attackTimer;

    bool m_canMove;       // 移動可否
    bool m_canCharge;
    ActionState m_state;

    bool m_isDeadTriggered = false; // 死亡時の初回処理用フラグ

    // ---- 手裏剣所持数 ----
    int m_shurikenCount = 5;   // 初期所持数

    int m_money = 0;

    std::string GetTexturePath() const override;

};