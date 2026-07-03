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

	m_hp = AddComponent<HPComponent>(50);

	return true;
}

void ArrowEnemyEntity::Update(float deltaTime)
{
   

    if (m_hp && m_hp->GetHP() <= 0)
    {
        std::cout << "ArrowEnemy Dead" << std::endl;
        SetState(State::Dead);
        return;
    }

    auto player =
        static_cast<PlayScene*>(m_scene)->GetPlayer();

    switch (m_attackState)
    {
    case AttackState::Idle:

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

        m_attackTimer -= deltaTime;

        if (m_attackTimer <= 0.0f)
        {
            m_attackTimer = 0.2f;

            m_attackState = AttackState::Attack;
        }

        break;

    case AttackState::Attack:

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

        m_attackTimer -= deltaTime;

        if (m_attackTimer <= 0.0f)
        {
            m_attackState = AttackState::Idle;
        }

        break;

    case AttackState::Dead:

        SetVel(Vector2d::Zero());

        m_deathTimer -= deltaTime;

        if (m_deathTimer <= 0)
        {
            SetState(State::Dead);
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
    
    return "assets/images/enemy/arrowEnemy.png";

}
