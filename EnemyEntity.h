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

class EnemyEntity : public EntityActor {
public:
	explicit EnemyEntity(Scene* scene, const Vector2d& pos = Vector2d::Zero(), const Vector2d& size = { 90, 196 });
	~EnemyEntity() override = default;

	virtual bool Init() override;
	virtual void Update(float deltaTime) override;

    virtual void UpdateAI(){}
	void UpdateMove(float deltaTime);
	void UpdateGravity(float deltaTime);
	void UpdateAttack(float deltaTime);
	void UpdateState();
    //asdfghjk

    ActorType GetType() const override { return ActorType::Enemy; }
    CollisionComponent* GetCollision() const { return m_collision; }

    bool OnGround() const { return m_isGround; }
    virtual void SetHP(){}
    void SetCanMove(bool canMove) { m_canMove = canMove; }

    HPComponent* GetHP() const { return m_hp; }
    virtual int GetMaxHP() const { return 100;  }
    virtual void TakeDamage(int damage, const Vector2d& knockback);

protected:
    virtual void OnDamaged(int damage, const Vector2d& knockback) {}
    virtual void OnDeadFromDamage(int damage, const Vector2d& knockback);

    HPComponent* m_hp;
    GravityComponent* m_gravity;
    //SpriteComponent* m_sprite;
    AnimationComponent* m_anim;

    bool m_dir;
    float m_jumpSpeed;    // ジャンプ速度
    float m_moveSpeed;    // 移動速度

    int  m_guard;
    int  m_guardMax;

    int  m_metsuGauge;
    int  m_metsuMax;
    bool m_metsu;

    bool m_damageCancel;
    bool m_attackActive;

    bool m_findPlayer;

    bool m_attack;
    int m_attackType;
    float m_attackTimer;

    float m_actionTimer;
    float m_cooldownTimer;
    bool m_actionLock;

    bool m_canMove;       // 移動可否

    virtual std::string GetTexturePath() const override;

};

