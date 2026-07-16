#include "ArrowEnemyEntity.h"
#include "VelocityComponent.h"
#include "HPComponent.h"
#include "Vector2d.h"
#include "EnemyBullet.h"
#include "PlayScene.h"
#include "PlayerEntity.h"
#include "Renderer.h"
#include "Game.h"
#include "BlockActor.h"
#include "DropItemEntity.h"
#include "EntityActor.h"
#include "AnimationComponent.h"
#include "SpriteComponent.h"
#include "CollisionComponent.h"
#include <cmath>


ArrowEnemyEntity::ArrowEnemyEntity(Scene* scene, const Vector2d& pos)
    : EnemyEntity(scene, pos, Vector2d(96, 190))
    , m_attackState(AttackState::Idle)
    , m_attackTimer(0.0f)
    , m_attackInterval(3.0f)
    , m_attackExecuted(false)
    , m_deathTimer(0.0f)
    , m_hpTestTimer(0.0f)

{
}
bool ArrowEnemyEntity::Init() {
	if (!EnemyEntity::Init()) return false;

	m_hp = AddComponent<HPComponent>(420);

    // =========================
  // ドロップ設定（テスト用）
  // =========================
    m_dropTable.clear();

    m_dropTable.push_back({ ItemType::Coin, 1.0f });
    m_dropTable.push_back({ ItemType::Kunai,1.0f });

    m_collision->SetRect(82.4f, 182.4f);
    m_collision->SetOffset(Vector2d(-30.0f, 0.0f));

    m_sprite->LoadTextureDiv(GetTexturePath(), 4, 7);
    m_sprite->SetDrawSize(0.0f, 324.0f);
    m_sprite->SetDrawOffset(15.0f, -50.0f);

    m_animation = AddComponent<AnimationComponent>();
    m_animation->SetSprite(m_sprite);

    AnimationClip stay;
    stay.frames = { 0 };
    stay.speed = 1.0f / 24.0f;
    stay.loop = true;

    AnimationClip attackReady;
    attackReady.frames = { 2,2,2,2,3,4,4,4,4,4,5,5,5,5,5,5,6,6,6,7,7,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,9,9,9,10 };
    attackReady.speed = 1.0f / 24.0f;
    attackReady.loop = false;

    AnimationClip attack;
    attack.frames = { 0 };
    attack.speed = 1.0f / 24.0f;
    attack.loop = false;

    AnimationClip hit;
    hit.frames = { 11,15,15,16,16,18,18,18,19,19,20,22,23,25,24 };
    hit.speed = 1.0f / 24.0f;
    hit.loop = false;

    AnimationClip dead;
    dead.frames = { 11,15,15,16,16,18,18,18,19,19,20,22,23,25,24 };
    dead.speed = 1.0f / 24.0f;
    dead.loop = false;

    m_animation->AddClip("Stay", stay);
    m_animation->AddClip("AttackReady", attackReady);
    m_animation->AddClip("Attack", attack);
    m_animation->AddClip("Hit", hit);
    m_animation->AddClip("Dead", dead);
    m_animation->Play("Stay");

	return true;
}

void ArrowEnemyEntity::CancelAttackForDamage()
{
    m_attackExecuted = false;
    m_attackTimer = 0.0f;
}

void ArrowEnemyEntity::StartHit(const Vector2d& knockback)
{
    constexpr float ARROW_HIT_KNOCKBACK_SCALE = 0.35f;

    CancelAttackForDamage();
    m_velocity->Set(knockback * ARROW_HIT_KNOCKBACK_SCALE);
    m_attackState = AttackState::Hit;

    if (m_animation != nullptr)
    {
        m_animation->Play("Hit", true);
    }
}

void ArrowEnemyEntity::StartDeadHit()
{
    CancelAttackForDamage();
    m_isDying = true;
    m_attackState = AttackState::Dead;
    m_deathTimer = 0.0f;
    m_velocity->Set(Vector2d::Zero());

    if (m_animation != nullptr)
    {
        m_animation->Play("Dead", true);
    }
}

void ArrowEnemyEntity::TakeDamage(int damage, const Vector2d& knockback)
{
    if (m_hp == nullptr || m_isDying)
    {
        return;
    }

    m_hp->Damage(damage);

    if (m_hp->GetHP() <= 0)
    {
        StartDeadHit();

        return;
    }

    Vector2d damageKnockback = knockback;
    PlayScene* play = static_cast<PlayScene*>(m_scene);
    PlayerEntity* player = play != nullptr ? play->GetPlayer() : nullptr;
    if (player != nullptr)
    {
        float awaySign = GetPos().x < player->GetPos().x ? -1.0f : 1.0f;
        damageKnockback.x = std::fabs(damageKnockback.x) * awaySign;
    }

    StartHit(damageKnockback);
}

void ArrowEnemyEntity::Update(float deltaTime)
{
   
    m_isHit = false;
    // 先に死亡済みなら何もしない
    if (GetState() == Actor::State::Dead)
        return;

    /*m_damageTimer += deltaTime;
    if (m_damageTimer >= 1.0f)
    {
        m_damageTimer -= 1.0f;

        if (m_hp)
        {
            m_hp->Damage(10);
        }
    }*/

    if (m_hp && m_hp->GetHP() <= 0 && !m_isDying)
    {
        StartDeadHit();
    }

    auto player =
        static_cast<PlayScene*>(m_scene)->GetPlayer();

    switch (m_attackState)
    {
    case AttackState::Idle:
        if (m_animation->GetCurrentName() != "Stay")
        {
            m_animation->Play("Stay");
        }

        m_attackTimer += deltaTime;

        if (m_attackTimer >= 3.0f)
        {
            float groundY = GetGroundY(player->GetPos().x);

            float targetX = player->GetPos().x;
            float targetY = GetGroundY(targetX);

            m_targetPos = Vector2d(targetX, targetY);

            m_attackTimer = 1.0f;

            m_attackExecuted = false;

            m_attackState = AttackState::Warning;

        }

        break;

    case AttackState::Warning:
        if (m_animation->GetCurrentName() != "AttackReady")
        {
            m_animation->Play("AttackReady");
        }

        m_attackTimer -= deltaTime;

        if (m_attackTimer <= 0.0f)
        {
            m_attackTimer = 0.2f;

            m_attackState = AttackState::Attack;
        }

        break;

    case AttackState::Attack:
        if (m_animation->GetCurrentName() != "Attack")
        {
            m_animation->Play("Attack");
        }

        if (!m_attackExecuted)
        {
            Vector2d playerPos = player->GetPos();

            float radius = 120.0f;

            float dx = playerPos.x - m_targetPos.x;
            float dy = playerPos.y - m_targetPos.y;

            float distanceSq =
                dx * dx +
                dy * dy;

            if (distanceSq <= radius * radius)
            {
                // player->Damage(10);
            }

            m_attackExecuted = true;
        }

        m_attackTimer -= deltaTime;

        if (m_attackTimer <= 0.0f)
        {
            m_attackTimer = 2.0f;
            m_attackState = AttackState::Cooldown;
        }

        break;

        m_attackTimer -= deltaTime;

        if (m_attackTimer <= 0.0f)
        {
            m_attackTimer = 2.0f;
            m_attackState = AttackState::Cooldown;
        }

        break;

    case AttackState::Cooldown:
        if (m_animation->GetCurrentName() != "Stay")
        {
            m_animation->Play("Stay");
        }

        m_attackTimer -= deltaTime;

        if (m_attackTimer <= 0.0f)
        {
            m_attackState = AttackState::Idle;
        }

        break;

    case AttackState::Hit:
        if (m_animation->GetCurrentName() != "Hit")
        {
            m_animation->Play("Hit");
        }

        if (m_animation->IsFinished())
        {
            m_attackState = AttackState::Idle;
            m_attackTimer = 0.0f;
        }
        break;

    case AttackState::Dead:
        if (m_animation->GetCurrentName() != "Dead")
        {
            m_animation->Play("Dead");
        }

        SetVel(Vector2d::Zero());
        m_deathTimer += deltaTime;

        if (m_deathTimer >= 1.0f)
        {
            OnDead();
            return;
        }
        break;
    }

    EnemyEntity::Update(deltaTime);
}


void ArrowEnemyEntity::Draw()
{
   

    EnemyEntity::Draw();

#ifdef _DEBUG

    Renderer* renderer =
        m_scene->GetGame()->GetRenderer();
    if (!renderer)
        return;
    float groundY = GetGroundY(m_targetPos.x);
    if (m_attackState == AttackState::Warning)
    {
        float height = groundY;

        renderer->DrawRectCenter(
            Vector2d(m_targetPos.x, height*0.5f),
            64.0f, height,
            Color(255, 255, 0), false);
    }

    if (m_attackState == AttackState::Attack)
    {
        float height = groundY;
        renderer->DrawRectCenter(
            Vector2d(m_targetPos.x, height * 0.5f),
            64.0f, height,
            Color(255, 0, 0), false);
        renderer->DrawCircle(
            m_targetPos,
            200.0f,
            Color(255, 0, 0),
            false);
    }
#endif
}

float ArrowEnemyEntity::GetGroundY(float x)
{
    float groundY = 100000.0f;

    for (auto actor : m_scene->GetActors())
    {
        if (actor->GetType() != ActorType::Block)
            continue;

        auto block = static_cast<BlockActor*>(actor);

        Vector2d blockPos = block->GetPos();

        auto col = block->GetCollision();
        if (!col)
            continue;

        float halfW = col->GetWidth() * 0.5f;

        float left = blockPos.x - halfW;
        float right = blockPos.x + halfW;

        if (x >= left && x <= right)
        {
            float topY =
                blockPos.y - col->GetHeight() * 0.5f;

            if (topY < groundY)
            {
                groundY = topY;
            }
        }
    }

    return groundY;
}

std::string ArrowEnemyEntity::GetTexturePath() const {
    
    return "assets/images/enemy/arrow/arrow.png";

}
