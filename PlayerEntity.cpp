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
    , m_canStand(true)
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

    CheckCanStand();

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
}

void PlayerEntity::UpdateMove(float deltaTime) {
    const Input& input = m_scene->GetGame()->GetInput();

    Vector2d move(0.0f, 0.0f);
    if (m_canMove) {
        if (input.IsDown(Action::DOWN)) {
            if (input.IsDown(Action::LEFT))  move.x -= 0.5f;
            if (input.IsDown(Action::RIGHT)) move.x += 0.5f;
        }
        else {
            if (input.IsDown(Action::LEFT))  move.x -= 1.0f;
            if (input.IsDown(Action::RIGHT)) move.x += 1.0f;
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
    if (!m_canStand) {
        return;
    }

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
        if (!m_squat)
        {
            m_collision->SetRect(85, 192);
        }
    }
}

void PlayerEntity::CheckCanStand()
{
    m_canStand = true;

    if (!m_squat)
        return;

    Vector2d standPos = m_transform->GetPosition();

    float squatHeight = m_collision->GetHeight();
    float standHeight = 192.0f;

    standPos.y -= (standHeight - squatHeight) * 0.5f;

    float halfW = 85.0f * 0.5f;
    float halfH = standHeight * 0.5f;

    for (auto actor : m_scene->GetActors())
    {
        if (actor->GetType() != ActorType::Block)
            continue;

        auto block = static_cast<BlockActor*>(actor);

        CollisionComponent* blockCol = block->GetCollision();

        Vector2d blockPos = block->GetPos();

        float blockHalfW = blockCol->GetWidth() * 0.5f;
        float blockHalfH = blockCol->GetHeight() * 0.5f;

        float diffX = standPos.x - blockPos.x;
        float diffY = standPos.y - blockPos.y;

        float overlapX = (halfW + blockHalfW) - std::abs(diffX);
        float overlapY = (halfH + blockHalfH) - std::abs(diffY);

        if (overlapX > 0 && overlapY > 0)
        {
            m_canStand = false;
            return;
        }
    }
}

void PlayerEntity::EnterSquat()
{
    float oldHeight = m_collision->GetHeight();
    float newHeight = 95.0f;

    Vector2d pos = m_transform->GetPosition();

    pos.y += (oldHeight - newHeight) * 0.5f;

    m_transform->SetPosition(pos);
    m_collision->SetRect(90, 95);
}

void PlayerEntity::ExitSquat()
{
    float oldHeight = m_collision->GetHeight();
    float newHeight = 192.0f;

    Vector2d pos = m_transform->GetPosition();

    pos.y -= (newHeight - oldHeight) * 0.5f;

    m_transform->SetPosition(pos);
    m_collision->SetRect(85, 192);
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

    const Input& input = m_scene->GetGame()->GetInput();

    if (m_attack) {
        if (input.IsDown(Action::DOWN)) {
            m_state = ActionState::SQUAT_ATTACK;
            m_anim->Play("squatAttack");
            m_collision->SetRect(90, 95);
        }
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

    if (input.IsDown(Action::DOWN)) {
        if (!m_squat)
        {
            EnterSquat();
            m_squat = true;
        }
        m_state = ActionState::SQUAT;
        if (vel.x == 0) {
            m_anim->Play("squat");
        }
        else {
            m_anim->Play("squatWalk");
        }
        return;
    }

    if (m_squat)
    {
        if (m_canStand)
        {
            ExitSquat();
            m_squat = false;
        }
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