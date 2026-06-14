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
    , m_attackCol(nullptr)
    , m_weakAttackIdx(0)
    , m_strongAttackIdx(0)
    , m_attackTimer(0.0f)

    , m_hitTimer(0.0f)
    , m_invincibleTime(0.0f)

    , m_canMove(true)
    , m_squat(false)
    , m_canStand(true)
    , m_canCharge(true)

    , m_drawOffset(Vector2d(0.0f, 10.0f))
{
}

bool PlayerEntity::Init() {

    if (!EntityActor::Init()) return false;

    m_hp = AddComponent<HPComponent>(100);
    m_transform->SetScale({ 1.0f, 1.0f });
    m_gravity = AddComponent<GravityComponent>(3200.0f);
    m_attackCol = AddComponent<CollisionComponent>();

    //m_sprite = AddComponent<SpriteComponent>();
    m_sprite->LoadTextureDiv("assets/images/entities/players/musashi_sheet.png", 20, 12);


    m_anim = AddComponent<AnimationComponent>();
    m_anim->SetSprite(m_sprite);

    AnimationClip idle;
    idle.frames = { 0, 1, 2, 3, 4, 5, };
    idle.speed = 0.3f;
    idle.loop = true;
    m_anim->AddClip("idle", idle);

    AnimationClip runStart;
    runStart.frames = { 6, 7 };
    runStart.speed = 0.1f;
    runStart.loop = false;
    m_anim->AddClip("runStart", runStart);

    AnimationClip run;
    run.frames = { 8, 9, 10, 11, 12, 13 };
    run.speed = 0.1f;
    run.loop = true;
    m_anim->AddClip("run", run);

    AnimationClip stopShort;
    stopShort.frames = { 14, 14, 17, 17, 18, 15 };
    stopShort.speed = 0.1f;
    stopShort.loop = false;
    m_anim->AddClip("stopShort", stopShort);

    AnimationClip stopLong;
    stopLong.frames = { 16, 17, 17, 18, 15 };
    stopLong.speed = 0.1f;
    stopLong.loop = false;
    m_anim->AddClip("stopLong", stopLong);

    AnimationClip changeDir;
    changeDir.frames = { 19, 20 };
    changeDir.speed = 0.1f;
    changeDir.loop = false;
    m_anim->AddClip("changeDir", changeDir);

    AnimationClip changeDirRun;
    changeDirRun.frames = { 21, 22, 23, 24, 25, 26 };
    changeDirRun.speed = 0.1f;
    changeDirRun.loop = false;
    m_anim->AddClip("changeDirRun", changeDirRun);

    AnimationClip squat;
    squat.frames = { 27, 28, 28, 28, 29 };
    squat.speed = 0.1f;
    squat.loop = false;
    m_anim->AddClip("squat", squat);

    AnimationClip squatIdle;
    squatIdle.frames = { 29 };
    squatIdle.speed = 0.1f;
    squatIdle.loop = false;
    m_anim->AddClip("squatIdle", squatIdle);

    AnimationClip squatWalk;
    squatWalk.frames = { 30, 30, 30, 31, 31, 32, 32, 32, 32, 32, 33, 33, 33,  34, 35, 35, 35 };
    squatWalk.speed = 0.1f;
    squatWalk.loop = true;
    m_anim->AddClip("squatWalk", squatWalk);

    AnimationClip squatAttack;
    squatAttack.frames = { 37, 38, 39, 40, 40, 40, 40, 41, 41, 42, 42 };
    squatAttack.speed = 0.1f;
    squatAttack.loop = false;
    m_anim->AddClip("squatAttack", squatAttack);

    AnimationClip jumpStart;
    jumpStart.frames = { 43 };
    jumpStart.speed = 0.1f;
    jumpStart.loop = false;
    m_anim->AddClip("jumpStart", jumpStart);

    AnimationClip jump;
    jump.frames = { 44, 45, 46, 47, 48 };
    jump.speed = 0.2f;
    jump.loop = false;
    m_anim->AddClip("jump", jump);

    AnimationClip jumpSecond;
    jumpSecond.frames = { 49, 50, 51, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 53 };
    jumpSecond.speed = 0.1f;
    jumpSecond.loop = false;
    m_anim->AddClip("jumpSecond", jumpSecond);

    AnimationClip fall;
    fall.frames = { 54, 55, 56 };
    fall.speed = 0.1f;
    fall.loop = false;
    m_anim->AddClip("fall", fall);

    AnimationClip jumpLanding;
    jumpLanding.frames = {57, 58, 59, 60, 61};
    jumpLanding.speed = 0.1f;
    jumpLanding.loop = false;
    m_anim->AddClip("jumpLanding", jumpLanding);

    AnimationClip roll;
    roll.frames = { 62, 63, 64, 65, 66 };
    roll.speed = 0.1f;
    roll.loop = false;
    m_anim->AddClip("roll", roll);

    AnimationClip Hien;
    Hien.frames = { 67, 68, 68, 68, 68, 68, 68, 69, 69 };
    Hien.speed = 0.1f;
    Hien.loop = false;
    m_anim->AddClip("Hien", Hien);

    AnimationClip Senten;
    Senten.frames = { 70, 71, 72, 73, 74, 75, 76, 77, 78 };
    Senten.speed = 0.1f;
    Senten.loop = false;
    m_anim->AddClip("Senten", Senten);

    AnimationClip rollLanding;
    rollLanding.frames = { 79, 80, 81 };
    rollLanding.speed = 0.1f;
    rollLanding.loop = false;
    m_anim->AddClip("rollLanding", rollLanding);

    AnimationClip wallHold;
    wallHold.frames = { 82 };
    wallHold.speed = 0.1f;
    wallHold.loop = false;
    m_anim->AddClip("wallHold", wallHold);

    AnimationClip wallJump;
    wallJump.frames = { 83, 84 };
    wallJump.speed = 0.1f;
    wallJump.loop = false;
    m_anim->AddClip("wallJump", wallJump);

    AnimationClip wallClimb;
    wallClimb.frames = { 85, 86, 87, 88 };
    wallClimb.speed = 0.1f;
    wallClimb.loop = true;
    m_anim->AddClip("wallClimb", wallClimb);

    AnimationClip wallClimbUp;
    wallClimbUp.frames = {89, 89, 90, 91, 91, 92, 93, 94,95, 96, 97, 97 };
    wallClimbUp.speed = 0.1f;
    wallClimbUp.loop = false;
    m_anim->AddClip("wallClimbUp", wallClimbUp);

    AnimationClip wallAttack;
    wallAttack.frames = { 98, 98, 99, 100, 100, 101, 102, 102, 103, 103, 104, 104 };
    wallAttack.speed = 0.1f;
    wallAttack.loop = false;
    m_anim->AddClip("wallAttack", wallAttack);

    AnimationClip weakAttack1;
    weakAttack1.frames = { 105, 106, 106, 107, 107, 107, 107, 107, 107, 107 };
    weakAttack1.speed = 0.1f;
    weakAttack1.loop = false;
    m_anim->AddClip("weakAttack1", weakAttack1);

    AnimationClip weakAttack2;
    weakAttack2.frames = { 108, 109, 110, 110, 111, 111, 112, 113 };
    weakAttack2.speed = 0.1f;
    weakAttack2.loop = false;
    m_anim->AddClip("weakAttack2", weakAttack2);

    AnimationClip weakAttack3;
    weakAttack3.frames = { 114, 115, 116, 116, 116, 116, 116, 116, 116, 116, 116, 116, 116, 116, 116, 116, 116, 116 };
    weakAttack3.speed = 0.1f;
    weakAttack3.loop = false;
    m_anim->AddClip("weakAttack3", weakAttack3);

    AnimationClip attackEnd;
    attackEnd.frames = { 117, 118, 118, 119, 119 };
    attackEnd.speed = 0.1f;
    attackEnd.loop = false;
    m_anim->AddClip("attackEnd", attackEnd);

    AnimationClip weakAttack4;
    weakAttack4.frames = { 120, 120, 121, 122, 122, 122, 123, 123, 124, 125, 126, 126, 126, 127, 127, 128, 128, 128, 129, 129, 130, 130, 130, 131, 131, 132 };
    weakAttack4.speed = 0.1f;
    weakAttack4.loop = false;
    m_anim->AddClip("weakAttack4", weakAttack4);

    AnimationClip weakAirAttack1;
    weakAirAttack1.frames = { 133, 134, 135, 135 };
    weakAirAttack1.speed = 0.1f;
    weakAirAttack1.loop = false;
    m_anim->AddClip("weakAirAttack", weakAirAttack1);

    AnimationClip weakAirAttack2;
    weakAirAttack2.frames = { 136, 137, 137, 137, 138 };
    weakAirAttack2.speed = 0.1f;
    weakAirAttack2.loop = false;
    m_anim->AddClip("weakAirAttack", weakAirAttack2);

    AnimationClip weakAirAttack3;
    weakAirAttack3.frames = { 139, 139, 140, 141, 141, 142, 143, 143, 143, 143, 143, 143, 143, 143, 143, 143, 143, 143, 144, 144, 144, 145, 145, 146, 146, 146 };
    weakAirAttack3.speed = 0.1f;
    weakAirAttack3.loop = false;
    m_anim->AddClip("weakAirAttack", weakAirAttack3);

    AnimationClip strongAttack1;
    strongAttack1.frames = { 147, 148, 149, 149, 150, 150, 151, 152, 152, 153, 153, 154, 155, 155, 156, 157, 157 };
    strongAttack1.speed = 0.1f;
    strongAttack1.loop = false;
    m_anim->AddClip("strongAttack1", strongAttack1);

    AnimationClip strongAttack2;
    strongAttack2.frames = { 158, 158, 159, 160, 160, 161, 162, 163, 163, 164, 164, 165, 165, 166, 166, 167, 168, 168, 168, 168 };
    strongAttack2.speed = 0.1f;
    strongAttack2.loop = false;
    m_anim->AddClip("strongAttack2", strongAttack2);

    AnimationClip Hayabusa;
    Hayabusa.frames = { 169, 169, 170 };
    Hayabusa.speed = 0.1f;
    Hayabusa.loop = false;
    m_anim->AddClip("Hayabusa", Hayabusa);

    AnimationClip HayabusaHit;
    HayabusaHit.frames = { 171, 171, 171, 171, 171, 171, 171, 171, 171, 172, 172, 173, 173, 174, 174, 175 };
    HayabusaHit.speed = 0.1f;
    HayabusaHit.loop = false;
    m_anim->AddClip("HitHayabusa", HayabusaHit);

    AnimationClip HayabusaGround;
    HayabusaGround.frames = { 176 };
    HayabusaGround.speed = 0.1f;
    HayabusaGround.loop = false;
    m_anim->AddClip("HayabusaGround", HayabusaGround);

    AnimationClip Kunai;
    Kunai.frames = { 177, 178, 179, 179, 180, 181 };
    Kunai.speed = 0.1f;
    Kunai.loop = false;
    m_anim->AddClip("Kunai", Kunai);

    AnimationClip KunaiAir;
    KunaiAir.frames = { 182, 183, 184, 184 };
    KunaiAir.speed = 0.1f;
    KunaiAir.loop = false;
    m_anim->AddClip("KunaiAir", KunaiAir);

    AnimationClip KunaiSquat;
    KunaiSquat.frames = { 185, 186, 186, 187, 187, 188, 188, 189, 190, 190 };
    KunaiSquat.speed = 0.1f;
    KunaiSquat.loop = false;
    m_anim->AddClip("KunaiSquat", KunaiSquat);

    AnimationClip hitTrap;
    hitTrap.frames = { 191, 192, 193, 194, 195, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196 };
    hitTrap.speed = 0.1f;
    hitTrap.loop = false;
    m_anim->AddClip("trapHit", hitTrap);

    AnimationClip hitGround;
    hitGround.frames = { 197, 197, 197, 197, 197, 197, 197, 197, 197, 197, 197, 197, 197, 198, 199, 200, 201, 202 };
    hitGround.speed = 0.1f;
    hitGround.loop = false;
    m_anim->AddClip("hitGround", hitGround);

    AnimationClip hitAir;
    hitAir.frames = { 203, 203, 203, 203, 203, 203, 203, 203, 203, 203, 203, 203, 203, 204, 204, 205, 205, 206, 206, 207, 208, 209, 210, 211, 212, 213, 214, 214, 215, 215, 216 };
    hitAir.speed = 0.1f;
    hitAir.loop = false;
    m_anim->AddClip("hitAir", hitAir);

    AnimationClip dead;
    dead.frames = { 217, 217, 217, 217, 217, 217, 217, 217, 217, 217, 217, 217, 217, 217, 217, 218, 218, 219, 219, 220, 220, 221, 221, 222, 222, 223, 223 };
    dead.speed = 0.1f;
    dead.loop = false;
    m_anim->AddClip("dead", dead);

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
            vel.y = -700.0f;

            m_isGround = false;
            m_jumpCount++;

            m_jumpTime = 0.0f;
            m_collision->SetRect(85, 150);
        }
        else if (m_jumpCount == 1) {
            vel.y = -700.0f;
            m_jumpCount++;
        }
    }

    if (input.IsDown(Action::JUMP) && m_jumpCount == 1 && m_jumpTime < m_maxJumpTime) {
        vel.y += -1300.0f * deltaTime;
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
            if (m_jumpCount == 1) {
                m_state = ActionState::JUMP;
                m_anim->Play("jump");
            }
            else if (m_jumpCount == 2) {
                m_state = ActionState::JUMP;
                m_anim->Play("jumpSecond");
            }
        }
        else {
            m_state = ActionState::FALL;
            m_anim->Play("fall");
        }

        return;
    }

    if (input.IsDown(Action::DOWN)) {
        m_drawOffset = Vector2d(0.0f, -40.0f);   // ã‚É40px

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
            m_drawOffset = Vector2d(0.0f, 10.0f);
        }
        else {
            if (vel.x == 0) {
                m_anim->Play("squat");
            }
            else {
                m_anim->Play("squatWalk");
            }
            return;
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