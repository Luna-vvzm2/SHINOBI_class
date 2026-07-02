#include "PlayerEntity.h"
#include "HitEffect.h"
#include "BlockActor.h"
#include "TransformComponent.h"
#include "VelocityComponent.h"
#include "GravityComponent.h"
#include "HPComponent.h"
#include "CollisionComponent.h"
#include "SpriteComponent.h"
#include "AnimationComponent.h"
#include "Input.h"
#include "Game.h"
#include "PlayScene.h"
#include "Scene.h"


PlayerEntity::PlayerEntity(Scene* scene, const Vector2d& pos, const Vector2d& size)
    : EntityActor(scene, pos, size)
    , m_hp(nullptr)
    , m_gravity(nullptr)
    , m_anim(nullptr)

    , m_dir(true)
    , m_jumpSpeed(0.0f)
    , m_moveSpeed(280.0f)
    , m_jumpCount(0)
    , m_jumpTime(0.0f)
    , m_maxJumpTime(1.5f)

    , m_attack(false)
    , m_attackType(0)
    , m_weakAttackIdx(0)
    , m_strongAttackIdx(0)
    , m_attackTimer(0.0f)

    , m_canMove(true)
    , m_canCharge(true)
    , m_coin(0)
    , m_kunai(0)
    , m_haku(0)
    , m_maxHaku(100)
{
}

bool PlayerEntity::Init() {

    if (!EntityActor::Init()) return false;

    m_hp = AddComponent<HPComponent>(100);
    m_transform->SetScale({ 1.0f, 1.0f });
    m_gravity = AddComponent<GravityComponent>(2800.0f);

    //m_sprite = AddComponent<SpriteComponent>();
    m_sprite->LoadTextureDiv("assets/images/entities/players/playerIdle.png", 6, 4);


    m_anim = AddComponent<AnimationComponent>();
    m_anim->SetSprite(m_sprite);

    AnimationClip idle;
    idle.frames = { 0,1,2,3,4,5,6,7,8,9,10,11 };
    idle.speed = 0.1f;
    idle.loop = true;
    m_anim->AddClip("idle", idle);


    AnimationClip jump;
    jump.frames = { 18, 19, 20 };
    jump.speed = 0.1f;
    jump.loop = false;
    m_anim->AddClip("jump", jump);


    AnimationClip fall;
    fall.frames = { 21, 22, 23 };
    fall.speed = 0.1f;
    fall.loop = false;
    m_anim->AddClip("fall", fall);

    m_collision->SetRect(85, 192);

    m_anim->Play("idle");



    m_state = ActionState::IDLE;

    return true;
}

void PlayerEntity::Update(float deltaTime) {

    UpdateMove(deltaTime);
    UpdateJump(deltaTime);
    UpdateGravity(deltaTime);
    UpdateAttack(deltaTime);
    UpdateState();

    EntityActor::Update(deltaTime);

    if (m_isGround)
    {
        m_jumpCount = 0;
    }

    UpdateState();

    if (m_anim) {
        m_anim->Update(deltaTime);
    }

    
}

void PlayerEntity::UpdateMove(float deltaTime) {
    const Input& input = m_scene->GetGame()->GetInput();

    Vector2d move(0, 0);
    if (m_canMove) {

        if (input.IsDown(Action::LEFT))  move.x -= 1;
        if (input.IsDown(Action::RIGHT)) move.x += 1;
    }
    move.normalize();

    move.x *= m_moveSpeed;


    Vector2d vel = m_velocity->Get();
    vel.x = move.x;

    m_velocity->Set(vel);
}

void PlayerEntity::UpdateJump(float deltaTime) {
    const Input& input = m_scene->GetGame()->GetInput();

    Vector2d vel = m_velocity->Get();

    if (input.IsTrigger(Action::JUMP))
    {
        if (m_jumpCount == 0)
        {
            vel.y = -700.0f;

            m_isGround = false;
            m_jumpCount++;

            m_jumpTime = 0.0f;
            m_collision->SetRect(85, 150);
        }
        else if (m_jumpCount == 1)
        {
            vel.y = -500.0f;
            m_jumpCount++;
        }
    }

    if (input.IsDown(Action::JUMP) && m_jumpCount == 1 && m_jumpTime < m_maxJumpTime)
    {
        vel.y += -1100.0f * deltaTime;
        m_jumpTime += deltaTime;
    }

    m_velocity->Set(vel);
}

void PlayerEntity::UpdateGravity(float deltaTime) {
    
    //追加
    // 重力加算
    Vector2d vel = m_velocity->Get();

    if (!m_isGround) {
        const float gravity = 1800.0f;
        vel.y += gravity * deltaTime;

        // 落下速度上限（必要なら）
        const float maxFallSpeed = 1200.0f;
        if (vel.y > maxFallSpeed)
        {
            vel.y = maxFallSpeed;
        }
    }

    m_velocity->Set(vel);

    MoveAndCollide(deltaTime);

    if (m_isGround) {
        m_jumpCount = 0;
    }
   

        // 着地したら当たり判定を元に戻す
        if (m_collision)
        {
            m_collision->SetRect(85, 192);
        }
    

}

void PlayerEntity::UpdateAttack(float deltaTime) {

}

void PlayerEntity::UpdateState()
{
    if (m_hp->GetHP() <= 0)
    {
        m_state = ActionState::DEAD;
        return;
    }

    if (m_attack)
    {
        m_state = ActionState::ATTACK;
        return;
    }

    Vector2d vel = m_velocity->Get();

    if (!m_isGround)
    {
        if (vel.y < 0)
        {
            m_state = ActionState::JUMP;
        }
        else
        {
            m_state = ActionState::FALL;
        }

        return;
    }

    const Input& input =
        m_scene->GetGame()->GetInput();

    if (input.IsDown(Action::DOWN))
    {
        m_state = ActionState::SQUAT;
        return;
    }

    if (vel.x != 0)
    {
        m_state = ActionState::RUN;
        return;
    }

    m_state = ActionState::IDLE;
}

std::string PlayerEntity::GetTexturePath() const {
    return "";
}
//======================================
// ドロップアイテムから呼ばれる関数
//======================================

//追加
void PlayerEntity::AddCoin(int value)
{
    m_coin += value;
    if (m_coin < 0) m_coin = 0;
}

void PlayerEntity::AddKunai(int value)
{
    m_kunai += value;
    if (m_kunai < 0) m_kunai = 0;
}

void PlayerEntity::AddHaku(int value)
{
    m_haku += value;

    if (m_haku < 0)
        m_haku = 0;

    if (m_haku > m_maxHaku)
        m_haku = m_maxHaku;
}

void PlayerEntity::HealHP(int value)
{
    if (!m_hp) return;

  
}

