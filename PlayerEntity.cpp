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
    , m_attackType()
    , m_weakAttackIdx(0)
    , m_strongAttackIdx(0)
    , m_attackTimer(0.0f)

    , m_hitTimer(0.0f)
    , m_invincibleTime(0.0f)

    , m_canMove(true)
    , m_squat(false)
    , m_canCharge(true)
{
}

bool PlayerEntity::Init() {

    if (!EntityActor::Init()) return false;

    m_hp = AddComponent<HPComponent>(100);
    m_transform->SetScale({ 1.0f, 1.0f });
    m_gravity = AddComponent<GravityComponent>(3200.0f);

    //m_sprite = AddComponent<SpriteComponent>();
    m_sprite->LoadTextureDiv("assets/images/entities/players/musashiIdle.png", 6, 6);


    m_anim = AddComponent<AnimationComponent>();
    m_anim->SetSprite(m_sprite);

    AnimationClip idle;
    idle.frames = { 0,1,2,3,4,5 };
    idle.speed = 0.2f;
    idle.loop = true;
    m_anim->AddClip("idle", idle);


    AnimationClip run;
    run.frames = {6,7,8,9,10,11 };
    run.speed = 0.1f;
    run.loop = true;
    m_anim->AddClip("run", run);


    AnimationClip squat;
    squat.frames = { 12,13,14,15 };
    squat.speed = 0.1f;
    squat.loop = false;
    m_anim->AddClip("squat", squat);


    AnimationClip squatWalk;
    squatWalk.frames = { 18,19,20,21,22,23 };
    squatWalk.speed = 0.1f;
    squatWalk.loop = true;
    m_anim->AddClip("squatWalk", squatWalk);


    AnimationClip jump;
    jump.frames = { 24,25,26,27,28,29 };
    jump.speed = 0.2f;
    jump.loop = false;
    m_anim->AddClip("jump", jump);
    

    AnimationClip fall;
    fall.frames = { 25,26,27 };
    fall.speed = 0.1f;
    fall.loop = false;
    m_anim->AddClip("fall", fall);

    m_collision->SetRect(85, 192);

    m_anim->Play("idle");



    m_state = ActionState::IDLE;

    return true;
}

void PlayerEntity::Update(float deltaTime) {
    UpdateInvincible(deltaTime);

    UpdateMove(deltaTime);
    UpdateJump(deltaTime);
    UpdateGravity(deltaTime);
    UpdateAttack(deltaTime);
    UpdateState();

    if (m_anim) {
        m_anim->Update(deltaTime);
    }

    EntityActor::Update(deltaTime);
}

void PlayerEntity::UpdateInvincible(float deltaTime)
{
    if (m_invincibleTime > 0.0f)
    {
        m_invincibleTime -= deltaTime;
    }
    printf("HP: %d\n", m_hp->GetHP());
}

void PlayerEntity::UpdateMove(float deltaTime) {
    const Input& input = m_scene->GetGame()->GetInput();

    Vector2d move(0, 0);
    if (m_canMove) {
        if (input.IsDown(Action::DOWN)) {
            if (input.IsDown(Action::LEFT))  move.x -= 0.5;
            if (input.IsDown(Action::RIGHT)) move.x += 0.5;
            m_squat = true;
        }
        else {
            if (input.IsDown(Action::LEFT))  move.x -= 1;
            if (input.IsDown(Action::RIGHT)) move.x += 1;
            m_squat = false;
        }
    }
    move.normalize();

    move.x *= m_moveSpeed;


    Vector2d vel = m_velocity->Get();
    
    if (move.x != 0) {
        vel.x = move.x;
    }
    else {
        vel.x *= 0.8f;

        if (std::abs(vel.x) < 1.0f) {
            vel.x = 0.0f;
        }
    }
    
    m_velocity->Set(vel);
}

void PlayerEntity::UpdateJump(float deltaTime) {
    const Input& input = m_scene->GetGame()->GetInput();

    Vector2d vel = m_velocity->Get();

    if (input.IsTrigger(Action::JUMP)) {
        if (m_jumpCount == 0) {
            vel.y = -800.0f;

            m_isGround = false;
            m_jumpCount++;

            m_jumpTime = 0.0f;
            m_collision->SetRect(85, 150);
        }
        else if (m_jumpCount == 1) {
            vel.y = -600.0f;
            m_jumpCount++;
        }
    }

    if (input.IsDown(Action::JUMP) && m_jumpCount == 1 && m_jumpTime < m_maxJumpTime) {
        vel.y += -1500.0f * deltaTime;
        m_jumpTime += deltaTime;
    }

    m_velocity->Set(vel);
}

void PlayerEntity::UpdateGravity(float deltaTime) {
    MoveAndCollide(deltaTime);
    
    if (m_isGround) {
        m_jumpCount = 0;
        m_collision->SetRect(85, 192);
    }
}

void PlayerEntity::UpdateAttack(float deltaTime) {
    const Input& input = m_scene->GetGame()->GetInput();

    if (input.IsTrigger(Action::WEAK_ATTACK)) {
        m_attack = true;
        m_attackType = AttackType::WEAK_ATTACK;
        m_attackTimer = 0.2f;
        m_weakAttackIdx++;
    }

    if (input.IsTrigger(Action::STRONG_ATTACK)) {
        m_attack = true;
        m_attackType = AttackType::STRONG_ATTACK;
        m_attackTimer = 0.3f;
        m_strongAttackIdx++;
    }

    if (m_attack) {
        m_attackTimer -= deltaTime;
        if (m_attackTimer <= 0) {
            m_attack = false;
            m_weakAttackIdx = 0;
            m_strongAttackIdx = 0;
        }
    }

}

void PlayerEntity::UpdateState() {
    if (m_hp->GetHP() <= 0) {
        m_state = ActionState::DEAD;
        return;
    }

    if (m_attack) {
        m_state = ActionState::ATTACK;
        m_anim->Play("attack");
        return;
    }

    Vector2d vel = m_velocity->Get();

    if (!m_isGround) {
        if (vel.y < 0) {
            m_state = ActionState::JUMP;
            m_anim->Play("jump");
        }
        else {
            m_state = ActionState::FALL;
            m_anim->Play("fall");
        }

        return;
    }

    const Input& input = m_scene->GetGame()->GetInput();

    if (input.IsDown(Action::DOWN)) {
        m_state = ActionState::SQUAT;
        if (vel.x == 0) {
            m_anim->Play("squat");
        }
        else {
            m_anim->Play("squatWalk");
        }
        m_collision->SetRect(90,95);
        return;
    }

    if (vel.x != 0) {
        m_state = ActionState::RUN;
        m_anim->Play("run");
        return;
    }

    m_state = ActionState::IDLE;
    m_anim->Play("idle");
}

void PlayerEntity::TakeDamage(int damage, const Vector2d& knockback) {
    if (m_invincibleTime > 0) return;

    m_hp->Damage(damage);
    
    m_velocity->Set(knockback);

    m_state = ActionState::HIT;

    m_invincibleTime = 2.0f;
}



std::string PlayerEntity::GetTexturePath() const {
    return "";
}