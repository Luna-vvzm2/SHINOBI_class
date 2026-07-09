#include "ScarecrowEnemyEntity.h"
#include "VelocityComponent.h"
#include "HPComponent.h"
#include "Key.h"
#include "Input.h"
#include "Game.h"
#include "Scene.h"
#include "SpriteComponent.h"
#include "AnimationComponent.h"

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
    launch.frames = { 1,2,3,4,5,6,7,8,9,10 };
    launch.speed = 0.15f;
    launch.loop = false;
    m_anim->AddClip("launch", launch);

    AnimationClip down;
    down.frames = { 11,12,13 };
    down.speed = 0.15f;
    down.loop = false;
    m_anim->AddClip("down", down);

    AnimationClip recover;
    recover.frames = { 14,15,16 };
    recover.speed = 0.15f;
    recover.loop = false;
    m_anim->AddClip("recover", recover);

    m_anim->Play("idle");

    m_transform->SetScale(Vector2d(0.4f, 0.4f));

    return true;
}

void ScarecrowEnemyEntity::Update(float deltaTime)
{
    EnemyEntity::Update(deltaTime);

    switch (m_state)
    {
    case ScarecrowState::Idle:
        UpdateIdle(deltaTime);
        break;

    case ScarecrowState::Hit:
        UpdateHit(deltaTime);
        break;

    case ScarecrowState::Launch:
        UpdateLaunch(deltaTime);
        break;

    case ScarecrowState::Down:
        UpdateDown(deltaTime);
        break;

    case ScarecrowState::Recover:
        UpdateRecover(deltaTime);
        break;
    }

    // í‚É‘Ì—Í‘S‰õ
    if (m_hp)
    {
        m_hp->Heal(DUMMY_HP);
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

void ScarecrowEnemyEntity::Hit(const Vector2d& dir)
{
    m_state = ScarecrowState::Hit;

    m_stateTimer = 0.25f;

    KnockBack(dir, 150.0f);

    m_anim->Play("hit");
}

void ScarecrowEnemyEntity::UpdateHit(float dt)
{
    m_stateTimer -= dt;

    if (m_stateTimer <= 0.0f)
    {
        m_velocity->SetVelocity(Vector2d::Zero());

        m_state = ScarecrowState::Idle;
    }
}
void ScarecrowEnemyEntity::Launch(
    const Vector2d& dir,
    float power)
{
    m_state = ScarecrowState::Launch;

    m_anim->Play("launch");

    m_velocity->SetVelocity(
        Vector2d(
            dir.x * power,
            -power * 0.6f
        )
    );
}

void ScarecrowEnemyEntity::UpdateLaunch(float dt)
{
    if (m_isGround)
    {
        m_velocity->SetVelocity(Vector2d::Zero());

        m_state = ScarecrowState::Down;

        m_stateTimer = 1.0f;

        m_anim->Play("down");
    }
}
void ScarecrowEnemyEntity::UpdateDown(float dt)
{
    m_stateTimer -= dt;

    if (m_stateTimer <= 0.0f)
    {
        m_state = ScarecrowState::Recover;

        m_anim->Play("recover");
    }
}

void ScarecrowEnemyEntity::UpdateRecover(float dt)
{
    m_stateTimer -= dt;

    if (m_stateTimer <= 0.0f)
    {
        m_state = ScarecrowState::Idle;

        m_anim->Play("idle");
    }
}

void ScarecrowEnemyEntity::KnockBack(
    const Vector2d& dir,
    float power)
{
    if (!m_velocity)
        return;

    m_velocity->SetVelocity(dir * power);
}