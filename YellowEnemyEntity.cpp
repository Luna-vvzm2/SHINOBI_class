#include "YellowEnemyEntity.h"
#include "VelocityComponent.h"
#include "HPComponent.h"
#include "Vector2d.h"
#include "PlayScene.h"
#include "PlayerEntity.h"

YellowEnemyEntity::YellowEnemyEntity(Scene* scene, const Vector2d& pos) : EnemyEntity(scene, pos, Vector2d(96, 190))
{


}

bool YellowEnemyEntity::Init() {
    if (!EnemyEntity::Init()) return false;

    m_hp = AddComponent<HPComponent>(600);

    m_state = Idle;
    m_attackTimer = 0.0f;

    return true;
}

void YellowEnemyEntity::Update(float deltaTime) {

    PlayScene* playScene =
        static_cast<PlayScene*>(GetScene());

    PlayerEntity* player =
        playScene->GetPlayer();


    if (!player) {
        EnemyEntity::Update(deltaTime);
        return;
    }
    float distance =
        player->GetPos().x -
        GetPos().x;
    Vector2d vel = m_velocity->Get();

    switch (m_state)
    {
    case Idle:

        vel.x = 0;

        if (std::abs(distance) < m_detectRange)
        {
            m_state = Chase;
        }
        break;

    case Chase:

        if (std::abs(distance) < m_attackRange)
        {
            m_state = AttackReady;
            m_attackTimer = 0.7f;
        }

        if (distance > 0)
        {
            vel.x = m_moveSpeed;
        }
        else
        {
            vel.x = -m_moveSpeed;
        }

        break;

    case AttackReady:

        vel.x = 0;

        m_attackTimer -= deltaTime;

        if (m_attackTimer <= 0)
        {
            m_state = Attack1;
            m_attackTimer = 0.2f;
        }

        break;

    case Attack1:

        vel.x = 0;

        m_attackTimer -= deltaTime;

        if (m_attackTimer <= 0)
        {
            m_state = Attack2;
            m_attackTimer = 0.2f;
        }

        break;

    case Attack2:

        vel.x = 0;

        m_attackTimer -= deltaTime;

        if (m_attackTimer <= 0)
        {
            m_state = Recovery;
            m_attackTimer = 1.0f;
        }

        break;

    case Recovery:

        vel.x = 0;

        m_attackTimer -= deltaTime;

        if (m_attackTimer <= 0)
        {
            m_state = Chase;
        }

        break;
    }

    m_velocity->Set(vel);
    EnemyEntity::Update(deltaTime);
}

std::string YellowEnemyEntity::GetTexturePath() const {
    return "assets/images/enemy/yellowEnemy.png";
}
