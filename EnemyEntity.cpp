#include "EnemyEntity.h"
#include "BlockActor.h"
#include "TransformComponent.h"
#include "VelocityComponent.h"
#include "GravityComponent.h"
#include "HPComponent.h"
#include "CollisionComponent.h"
#include "SpriteComponent.h"
#include "AnimationComponent.h"
#include "Game.h"
#include "PlayScene.h"

EnemyEntity::EnemyEntity(Scene* scene, const Vector2d& pos, const Vector2d& size)
    : EntityActor(scene, pos, size)
    , m_hp(nullptr)
    , m_gravity(nullptr)
    , m_anim(nullptr)

    , m_dir(true)
    , m_jumpSpeed(0.0f)
    , m_moveSpeed(200.0f)

    , m_guard(0)
    , m_guardMax(100)

    , m_metsuGauge(0)
    , m_metsuMax(100)
    , m_metsu(0)

    , m_damageCancel(0)
    , m_attackActive(0)

    , m_findPlayer(0)

    , m_attack(false)
    , m_attackType(0)
    , m_attackTimer(0.0f)

    , m_actionTimer(0.0f)
    , m_cooldownTimer(0.0f)
    , m_actionLock(false)

    , m_canMove(true)
{
}

bool EnemyEntity::Init() {
    if (!EntityActor::Init())
        return false;

    m_hp = AddComponent<HPComponent>(GetMaxHP());
    m_gravity = AddComponent<GravityComponent>(2800.0f);

    return true;
}

void EnemyEntity::Update(float deltaTime) {
    EntityActor::Update(deltaTime);
    MoveAndCollide(deltaTime);
}

void EnemyEntity::TakeDamage(int damage, const Vector2d& knockback)
{
    if (m_hp == nullptr)
    {
        return;
    }

    m_hp->Damage(damage);

    if (m_hp->GetHP() <= 0)
    {
        OnDeadFromDamage(damage, knockback);
        return;
    }

    OnDamaged(damage, knockback);
}

void EnemyEntity::OnDeadFromDamage(int damage, const Vector2d& knockback)
{
    SetState(Actor::State::Dead);
}
std::string EnemyEntity::GetTexturePath() const {
    return "";
}

/*

弾生成
    auto bullet =
        new EnemyBullet(m_scene,　GetPos());
    
    m_scene->AddActor(bullet);


*/
void EnemyEntity::UpdateMove(float deltaTime)
{
    // 基本は derived class で上書きする想定だが、無くてもリンクエラーにならないよう空実装を用意
    // 例として、移動速度に基づいて velocity をセットする簡易実装:
    if (m_velocity) {
        Vector2d v = m_velocity->Get();
        // m_moveSpeed は正負方向の管理が派生側にある想定
        v.x = (m_dir ? 1.0f : -1.0f) * m_moveSpeed;
        m_velocity->Set(v);
    }
}

// 重力処理の最小実装
void EnemyEntity::UpdateGravity(float deltaTime)
{
    // 多くの処理は GravityComponent に任せているため空でも問題ない
    if (m_gravity) {
        // もし固有の処理が必要ならここに加える
    }
}

// 攻撃処理の最小実装（派生クラスで実装する想定）
void EnemyEntity::UpdateAttack(float deltaTime)
{
    // デフォルトは何もしない
}

// 状態更新の最小実装（派生で上書き）
void EnemyEntity::UpdateState()
{
    // デフォルトは何もしない
}