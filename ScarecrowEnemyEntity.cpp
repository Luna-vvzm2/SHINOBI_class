#include "ScarecrowEnemyEntity.h"
#include "VelocityComponent.h"
#include "HPComponent.h"
#include "Key.h"
#include "Input.h"
#include "Game.h"
#include "Scene.h"

ScarecrowEnemyEntity::ScarecrowEnemyEntity(
    Scene* scene,
    const Vector2d& pos)
    : EnemyEntity(scene, pos, Vector2d(96, 190))
{
}

bool ScarecrowEnemyEntity::Init()
{
    if (!EnemyEntity::Init())
        return false;

    m_hp = GetComponent<HPComponent>();
    m_velocity = AddComponent<VelocityComponent>();
    return true;
    
}

void ScarecrowEnemyEntity::Update(float deltaTime)
{
    EnemyEntity::Update(deltaTime);

    const Input& input =
        m_scene->GetGame()->GetInput();

   

    if (m_velocity)
    {
        Vector2d vel = m_velocity->GetVelocity();

        vel = vel* 0.90f;

        float len = sqrt(
            vel.x * vel.x +
            vel.y * vel.y
        );

        if (len < 10.0f)
          vel = Vector2d::Zero();

        m_velocity->SetVelocity(vel);
        if (m_knockBackTimer > 0.0f)
        {
            m_knockBackTimer -= deltaTime;

            if (m_knockBackTimer <= 0.0f)
            {
                m_velocity->SetVelocity(Vector2d::Zero());
            }
        }
    }
    
   
    // –ˆƒtƒŒ[ƒ€‘S‰ñ•œ
    if (m_hp)
    {
        m_hp->Heal(DUMMY_HP);
    }
}

void ScarecrowEnemyEntity::Draw()
{
    EnemyEntity::Draw();
}
void ScarecrowEnemyEntity::KnockBack(
    const Vector2d& attackDir,
    float power)
{
    if (!m_velocity) {
        return;
    }


    m_velocity->SetVelocity(
        attackDir * power
    );

    m_knockBackTimer = 0.3f;
   
}

void ScarecrowEnemyEntity::Launch(const Vector2d& attackDir, float power)
{
    if (!m_velocity)
        return;

    m_velocity->SetVelocity(
        Vector2d(
            attackDir.x * power,
            -power * 0.5f
        )
    );
}