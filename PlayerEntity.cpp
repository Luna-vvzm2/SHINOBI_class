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
#include <Windows.h>

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
    , m_state()
    , m_shurikenCount(5) // ★追加: 初期の手裏剣の数を5に設定
{
}

bool PlayerEntity::Init() {
    if (!EntityActor::Init()) return false;

    m_hp = AddComponent<HPComponent>(100);
    m_transform->SetScale({ 1.0f, 1.0f });
    m_gravity = AddComponent<GravityComponent>(2800.0f);

    m_sprite->LoadTextureDiv("kari/player.png", 6, 4);

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
    if (GetAsyncKeyState(0x73) & 0x8000) {
        m_state = ActionState::DEAD;
    }

    if (m_state == ActionState::DEAD) {
        UpdateDead(deltaTime);
        if (m_anim) m_anim->Update(deltaTime);
        EntityActor::Update(deltaTime);
        return;
    }

    UpdateMove(deltaTime);
    UpdateJump(deltaTime);
    UpdateGravity(deltaTime);
    UpdateAttack(deltaTime);
    UpdateState();

    if (m_anim) m_anim->Update(deltaTime);
    EntityActor::Update(deltaTime);
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

    if (input.IsTrigger(Action::JUMP)) {
        if (m_jumpCount == 0) {
            vel.y = -700.0f;
            m_isGround = false;
            m_jumpCount++;
            m_jumpTime = 0.0f;
            m_collision->SetRect(85, 150);
        }
        else if (m_jumpCount == 1) {
            vel.y = -500.0f;
            m_jumpCount++;
        }
    }
    if (input.IsDown(Action::JUMP) && m_jumpCount == 1 && m_jumpTime < m_maxJumpTime) {
        vel.y += -1100.0f * deltaTime;
        m_jumpTime += deltaTime;
    }
    m_velocity->Set(vel);
}

void PlayerEntity::UpdateGravity(float deltaTime) {
    MoveAndCollide(deltaTime);
    if (m_isGround) m_jumpCount = 0;
}

void PlayerEntity::UpdateAttack(float deltaTime) {}

void PlayerEntity::UpdateState() {
    if (m_hp->GetHP() <= 0) { m_state = ActionState::DEAD; return; }
    if (m_attack) { m_state = ActionState::ATTACK; return; }

    Vector2d vel = m_velocity->Get();
    if (!m_isGround) {
        m_state = (vel.y < 0) ? ActionState::JUMP : ActionState::FALL;
        return;
    }

    const Input& input = m_scene->GetGame()->GetInput();
    if (input.IsDown(Action::DOWN)) { m_state = ActionState::SQUAT; return; }
    if (vel.x != 0) { m_state = ActionState::RUN; return; }

    m_state = ActionState::IDLE;
}

void PlayerEntity::UpdateDead(float deltaTime) {
    // 画面中央の目標座標（ゲームの画面解像度に合わせて数値を調整してください）
    Vector2d centerPos(960.0f, 540.0f);

    if (!m_isDeadTriggered) {
        m_isDeadTriggered = true;
        m_canMove = false;

        m_sprite->LoadTextureDiv("kari/det.png", 6, 1);

        AnimationClip dead;
        dead.frames = { 0, 1, 2, 3, 4, 5 };
        dead.speed = 0.12f;
        dead.loop = false; // 1回再生したら最後のコマで止まる
        m_anim->AddClip("dead", dead);
        m_anim->Play("dead");

        // 死亡した瞬間に一瞬で画面中央へワープさせる
        m_transform->SetPosition(centerPos);
        m_velocity->Set(Vector2d::Zero());
    }

    // 物理移動を完全に停止
    m_velocity->Set(Vector2d::Zero());

    // 毎フレーム画面中央の位置に完全に固定する
    m_transform->SetPosition(centerPos);
}

void PlayerEntity::SetMoney(int amount)
{
    int old = m_money;
    m_money = (amount >= 0) ? amount : 0;
    if (OnMoneyChanged) OnMoneyChanged(m_money, old);
}

void PlayerEntity::AddMoney(int delta)
{
    if (delta == 0) return;
    int old = m_money;
    m_money += delta;
    if (m_money < 0) m_money = 0;
    if (OnMoneyChanged) OnMoneyChanged(m_money, old);
}

std::string PlayerEntity::GetTexturePath() const { return ""; }
