#include "YellowEnemyEntity.h"
#include "VelocityComponent.h"
#include "HPComponent.h"
#include "Vector2d.h"
#include "PlayScene.h"
#include "PlayerEntity.h"
#include "DropItemEntity.h"
#include "EntityActor.h"
#include "AnimationComponent.h"

#include <cmath>

YellowEnemyEntity::YellowEnemyEntity(Scene* scene, const Vector2d& pos) : EnemyEntity(scene, pos, Vector2d(96, 190))
{


}


bool YellowEnemyEntity::Init() {
    if (!EnemyEntity::Init()) return false;

    m_hp = AddComponent<HPComponent>(100);

    // 攻撃判定
    m_attackCollision = AddComponent<CollisionComponent>();
    m_attackCollision->SetNone();
    m_state = Idle;
    m_attackTimer = 0.0f;
    
    m_faceRight = true;
    
       // =========================
    // ドロップ設定（テスト用）
    // =========================
    m_dropTable.clear();

    // 必ずコインを落とす
    m_dropTable.push_back({ ItemType::Coin, 1.0f });
   
    if (!EnemyEntity::Init())
        return false;

    m_animation = AddComponent<AnimationComponent>();
    m_animation->SetSprite(m_sprite);

    AnimationClip stay;
    stay.frames = {0,1,2,3,4};
    stay.speed = 0.12f;
    stay.loop = true;


    AnimationClip walk;
    walk.frames = { 6,7,8,9,10,11,12,13,14,15 };
    walk.speed = 0.12f;
    walk.loop = true;

    AnimationClip attackReady;
    attackReady.frames = { 16,17,18,19 };
    attackReady.speed = 0.12f;
    attackReady.loop = false;

    AnimationClip attack1;
    attack1.frames = { 20,21,22 };
    attack1.speed = 0.08f;
    attack1.loop = false;

    AnimationClip attack2;
    attack2.frames = { 23,24,25,26 };
    attack2.speed = 0.08f;
    attack2.loop = false;

    AnimationClip hit;
    hit.frames = {27,28,29,30};
    hit.speed = 0.08f;
    hit.loop = false;

    AnimationClip hitback;
    hitback.frames = { 27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43 };
    hitback.speed = 0.08f;
    hitback.loop = false;


    AnimationClip dead;
    dead.frames = { 27,28,29,30,31,32,33,34,35,36,37,38,39,40 };
    dead.speed = 0.1f;
    dead.loop = false;

    // ④ 登録
    m_animation->AddClip("Stay", stay);
    m_animation->AddClip("Walk", walk);
    m_animation->AddClip("AttackReady", attackReady);
    m_animation->AddClip("Attack1", attack1);
    m_animation->AddClip("Attack2", attack2);
    m_animation->AddClip("Hit", hit);
    m_animation->AddClip("HitBack", hitback);
    m_animation->AddClip("Dead", dead);

    // ⑤ 最初は待機アニメーション
    m_animation->Play("Stay");
    return true;
}

void YellowEnemyEntity::Update(float deltaTime) {
   
    m_isHit = false;
    // 先に死亡済みなら何もしない
    if (GetState() == Actor::State::Dead)
        return;

    // デバッグ用：毎秒ダメージ
    /*
    m_damageTimer += deltaTime;
    if (m_damageTimer >= 1.0f)
    {
        m_damageTimer -= 1.0f;

        if (m_hp)
        {
            m_hp->Damage(10);
        }
    }
   */

    PlayScene* playScene = static_cast<PlayScene*>(GetScene());
    PlayerEntity* player = playScene ? playScene->GetPlayer() : nullptr;
    Vector2d vel = m_velocity->Get();

    if (!player) {
        vel.x = 0.0f;
        m_velocity->Set(vel);
        EnemyEntity::Update(deltaTime);
        return;
    }
    float distance =
        player->GetPos().x - GetPos().x;

    
    if (m_hp && m_hp->GetHP() <= 0)
    {

        OnDead();
        return;
    }

    switch (m_state)
    {
    case Dead:
        if (m_animation->GetCurrentName() != "Dead")
        {
            m_animation->Play("Dead");
        }
        vel.x = 0.0f;
        
        break;

    case Idle:
        if (m_animation->GetCurrentName() != "Stay")
        {
            m_animation->Play("Stay");
        }
        vel.x = 0.0f;
        if (std::abs(distance) < m_detectRange)
        {
            m_state = Chase;
        }
        break;

    case Chase:
       
        if (m_animation->GetCurrentName() != "Walk")
        {
            m_animation->Play("Walk");
        }
        if (distance > 0)
        {
            m_faceRight = true;
            vel.x = m_moveSpeed;
        }
        else
        {
            m_faceRight = false;
            vel.x = -m_moveSpeed;
        }

        if (std::abs(distance) < m_attackRange)
        {
            m_state = AttackReady;
            m_attackTimer = 0.7f;
            vel.x = 0.0f;
        }
        break;

    case AttackReady:
        if (m_animation->GetCurrentName() != "AttackReady")
        {
            m_animation->Play("AttackReady");
        }
        vel.x = 0.0f;
        m_attackTimer -= deltaTime;

        if (m_attackTimer <= 0)
        {
            m_state = Attack1;
            m_attackTimer = 0.2f;
            m_attackHit = false;
        }
        break;

    case Attack1:
        if (m_animation->GetCurrentName() != "Attack1")
        {
            m_animation->Play("Attack1");
        }
        vel.x = 0.0f;
        m_attackCollision->SetRect(60.0f, 120.0f);
        if (m_faceRight)
        {
            m_attackCollision->SetOffset(
                Vector2d(1000.0f, 0.0f));

        }
        else
        {
        m_attackCollision->SetOffset(
            Vector2d(-60.0f, 0.0f));
            
        }

        m_attackTimer -= deltaTime;
        if (m_attackTimer <= 0)
        {
            m_state = Attack2;
            m_attackTimer = 0.2f;
            m_attackHit = false;
        }
        break;

    case Attack2:
        if (m_animation->GetCurrentName() != "Attack2")
        {
            m_animation->Play("Attack2");
        }
        vel.x = 0.0f;
        m_attackCollision->SetRect(200.0f, 120.0f);
        if (m_faceRight)
        {
            m_attackCollision->SetOffset(
                Vector2d(100.0f, 0.0f));
        }
        else
        {
            m_attackCollision->SetOffset(
                Vector2d(-100.0f,-200.0f));
        }

        m_attackTimer -= deltaTime;

        if (m_attackTimer <= 0)
        {
            m_state = Recovery;
            m_attackTimer = 1.0f;
        }
        break;

    case Recovery:
        if (m_animation->GetCurrentName() != "Stay")
        {
            m_animation->Play("Stay");
        }
        vel.x = 0.0f;
        m_attackCollision->SetNone();
        m_attackCollision->SetOffset(
            Vector2d::Zero());
        m_attackTimer -= deltaTime;

        if (m_attackTimer <= 0)
        {
            m_state = Idle;
        }
        break;
    }
    auto playerCollision =
        player->GetComponent<CollisionComponent>();

    if ((m_state == Attack1 || m_state == Attack2) &&
        playerCollision &&
        m_attackCollision->CheckCollision(playerCollision))
    {
        if (!m_attackHit)
        {
            if (m_state == Attack1)
            {
                player->GetComponent<HPComponent>()->Damage(10);
            }
            else
            {
                player->GetComponent<HPComponent>()->Damage(20);
            }

            m_attackHit = true;
        }
    }
    m_velocity->Set(vel);
    EnemyEntity::Update(deltaTime);
}


std::string YellowEnemyEntity::GetTexturePath() const {
    return "assets/images/Enemy/yellow.png";
}


void YellowEnemyEntity::Draw()
{
    EnemyEntity::Draw();

#ifdef _DEBUG

    m_attackCollision->DrawDebug();

#endif
}