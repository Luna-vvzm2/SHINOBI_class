#include "YellowEnemyEntity.h"
#include "VelocityComponent.h"
#include "HPComponent.h"
#include "Vector2d.h"
#include "PlayScene.h"
#include "PlayerEntity.h"
#include "DropItemEntity.h"
#include "EntityActor.h"

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
        vel.x = 0.0f;
        
        break;

    case Idle:
        vel.x = 0.0f;
        
        if (std::abs(distance) < m_detectRange)
        {
            m_state = Chase;
        }
        break;

    case Chase:
       

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
        vel.x = 0.0f;
        m_attackCollision->SetNone();
        m_attackCollision->SetOffset(
            Vector2d::Zero());
        m_attackTimer -= deltaTime;

        if (m_attackTimer <= 0)
        {
            m_state = Chase;
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
    return "assets/images/enemy/yellowEnemy.png";
}


void YellowEnemyEntity::Draw()
{
    EnemyEntity::Draw();

#ifdef _DEBUG

    m_attackCollision->DrawDebug();

#endif
}