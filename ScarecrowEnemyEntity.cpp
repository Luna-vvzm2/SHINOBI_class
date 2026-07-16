#include "ScarecrowEnemyEntity.h"
#include "VelocityComponent.h"
#include "HPComponent.h"
#include "Key.h"
#include "Input.h"
#include "Game.h"
#include "Scene.h"
#include "SpriteComponent.h"
#include "AnimationComponent.h"
#include "PlayScene.h"
#include "PlayerEntity.h"


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

    m_hp = AddComponent<HPComponent>(1);
    m_velocity = GetComponent<VelocityComponent>();

    // AnimationComponentŽæ“¾
    m_anim = AddComponent<AnimationComponent>();
    m_anim->SetSprite(m_sprite);

    m_sprite->LoadTextureDiv(
        "assets/images/Enemy/scarecrow.png",
        4,
        5
    );

    AnimationClip idle;
    idle.frames = { 0 };
    idle.speed = 0.15f;
    idle.loop = true;
    m_anim->AddClip("idle", idle);

    AnimationClip hit;
    hit.frames = { 1,2,3,4 };
    hit.speed = 0.15f;
    hit.loop = false;
    m_anim->AddClip("hit", hit);

    AnimationClip launch;
    launch.frames = { 1,2,3,4,5,6,7,8,9,10, 11,12,13,14,15,16 };
    launch.speed = 0.15f;
    launch.loop = false;
    m_anim->AddClip("launch", launch);



    m_anim->Play("idle");

    m_transform->SetScale(Vector2d(0.4f, 0.4f));

    return true;
}

void ScarecrowEnemyEntity::Update(float deltaTime)
{
    if (m_isHit)
    {
        m_hitTimer -= deltaTime;

        if (m_hitTimer <= 0.0f)
        {
            m_isHit = false;
        }

        EnemyEntity::Update(deltaTime);
        return;
    }
    

    switch (m_state)
    {
    case ScarecrowState::Idle:
        UpdateIdle(deltaTime);
        break;

    case ScarecrowState::BlowHit:
        UpdateBlowHit(deltaTime);
        break;


 
    }


    if (m_anim)
    {
        m_anim->Update(deltaTime);
    }
   
}

void ScarecrowEnemyEntity::Draw()
{
    EnemyEntity::Draw();
}

void ScarecrowEnemyEntity::UpdateIdle(float dt)
{
    m_sprite->SetDrawOffset(0.0f, 15.0f);
    m_anim->Play("idle");
}




void ScarecrowEnemyEntity::UpdateBlowHit(float dt)
{
    m_hitTimer -= dt;

    if (m_hitTimer <= 0.0f)
    {

        m_stateTimer = 1.0f;

        m_anim->Play("Idle");
    }
}

void ScarecrowEnemyEntity::TakeDamage(int damage, const Vector2d& knockback)
{
    PlayScene* playScene =
        static_cast<PlayScene*>(GetScene());

    PlayerEntity* player =
        playScene ? playScene->GetPlayer() : nullptr;

    Vector2d force = knockback;

    if (player)
    {
        float sign =
            GetPos().x < player->GetPos().x ? -1.0f : 1.0f;

        force.x = std::fabs(force.x) * sign;
    }

    KnockBack(force);
}

void ScarecrowEnemyEntity::KnockBack(const Vector2d& velocity)
{
    if (!m_velocity)
        return;

    m_velocity->Set(velocity);

    m_hitTimer = 0.3f;
    m_isHit = true;

    m_state = ScarecrowState::BlowHit;

    if (m_anim)
    {
        m_anim->Play("hit");
    }
}