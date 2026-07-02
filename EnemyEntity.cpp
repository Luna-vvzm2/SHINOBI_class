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
#include "Renderer.h"
#include "BossEntity.h"
#include "EnemyHPBar.h"

// metu gauge shared handle (one-time load)
static int s_metuHandle = -1;
static int s_metuW = 0;
static int s_metuH = 0;
static const char* METU_GAGE_PATH = "assets/images/uies/metu_gage.png";

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

    // one-time load of metu gauge texture
    if (s_metuHandle == -1) {
        s_metuHandle = LoadGraph(METU_GAGE_PATH);
        if (s_metuHandle != -1) {
            GetGraphSize(s_metuHandle, &s_metuW, &s_metuH);
        }
        else {
            std::cerr << "[EnemyEntity] Failed to load metu gauge: " << METU_GAGE_PATH << std::endl;
        }
    }

    return true;
}

void EnemyEntity::Update(float deltaTime) {
    EntityActor::Update(deltaTime);
    MoveAndCollide(deltaTime);
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

void EnemyEntity::TakeDamage(int damage, const Vector2d& knockback)
{
    if (m_hp == nullptr)
    {
        return;
    }

    // HP にダメージ
    m_hp->Damage(damage);

    // --- 滅ゲージ増加ロジック (m_metsuPerDamage を使う) ---
    // 1ダメージあたり m_metsuPerDamage を加算
    if (m_metsuPerDamage > 0) {
        m_metsuGauge += damage * m_metsuPerDamage;
    }
    else {
        // 安全策: もし 0 や負が設定されていたらデフォルト増分を使う
        m_metsuGauge += damage;
    }

    // 上限 clamp
    if (m_metsuGauge >= m_metsuMax) {
        m_metsuGauge = m_metsuMax;
        m_metsu = true;
    }

    // HPBar に反映（m_hpBar がセットされていれば）
    if (m_hpBar) {
        m_hpBar->SetMetsuValue(m_metsuGauge, m_metsuMax);
        if (m_metsu) {
            m_hpBar->SetMetsuFullImagePath("assets/images/uies/metu_gage.png"); // 任意
            m_hpBar->ShowFor(9999.0f); // 満タンなら長時間表示（必要なら変更）
            m_hpBar->SetVisible(true);
        }
        else {
            m_hpBar->ShowFor(2.0f);
        }
    }

    // 死亡処理
    if (m_hp->GetHP() <= 0)
    {
        OnDeadFromDamage(damage, knockback);
        return;
    }

    // ノックバック等既存処理
    Vector2d vel = m_velocity->Get();
    vel = knockback;
    m_velocity->Set(vel);

    OnDamaged(damage, knockback);
}

void EnemyEntity::OnDeadFromDamage(int damage, const Vector2d& knockback)
{
    SetState(Actor::State::Dead);
}

void EnemyEntity::Draw()
{
    // 1) まず既存のコンポーネント描画や派生クラスが期待する描画を行わせる
    Actor::Draw();

    // 2) ボス敵には頭上ゲージをつけない（BossEntity 派生は除外）
	// ボス以外も同じようにifで除外することも可能
    if (dynamic_cast<BossEntity*>(this) != nullptr) {
        return; // ボスなら描画処理をスキップ
    }
    // 3) metu gauge を頭上に描く（ロードに成功していれば）
    if (s_metuHandle < 0) return; // テクスチャ未ロードなら何もしない

    if (m_transform == nullptr) return;
    Renderer* renderer = nullptr;
    if (m_scene && m_scene->GetGame()) renderer = m_scene->GetGame()->GetRenderer();
    if (!renderer) return;

    // 敵の中心位置（ワールド座標）
    Vector2d center = m_transform->GetPosition();

    // 敵のサイズ（ワールド単位）を使ってゲージ幅を決める（敵幅の 0.7 - 0.9 程度が自然）
    Vector2d enemySize = GetSize(); // EntityActor::GetSize が有効であればそれを使う
    float gaugeWidth = (enemySize.x > 0.0f) ? enemySize.x * 0.8f : 64.0f; // フォールバック幅
    // アスペクト比維持で高さを計算
    float gaugeHeight = (s_metuW > 0) ? gaugeWidth * static_cast<float>(s_metuH) / static_cast<float>(s_metuW) : gaugeWidth * 0.25f;

    // 頭上の Y 座標: 中心 - 半高 - (敵半高さ) - マージン
    float margin = 8.0f; // 頭上の余白（px 相当のワールド単位）
    float headY = center.y - (enemySize.y * 0.5f) - (gaugeHeight * 0.5f) - margin;

    Vector2d gaugePos(center.x, headY);

    // スケールはテクスチャサイズから求める
    float scaleX = (s_metuW > 0) ? (gaugeWidth / static_cast<float>(s_metuW)) : 1.0f;
    float scaleY = (s_metuH > 0) ? (gaugeHeight / static_cast<float>(s_metuH)) : 1.0f;

    // 3) metu gauge 表示：満タンのときのみ画像を表示する
    // --- metu gauge 表示：HPBar が無い場合のみ Entity 側で描画 ---
    if (m_hpBar == nullptr && s_metuHandle >= 0 && m_metsuMax > 0 && (m_metsu || m_metsuGauge >= m_metsuMax))
    {
        if (m_transform == nullptr) return;
        Renderer* renderer = nullptr;
        if (m_scene && m_scene->GetGame()) renderer = m_scene->GetGame()->GetRenderer();
        if (!renderer) return;

        Vector2d center = m_transform->GetPosition();
        Vector2d enemySize = GetSize();
        float gaugeWidth = (enemySize.x > 0.0f) ? enemySize.x * 0.8f : 64.0f;
        float gaugeHeight = (s_metuW > 0) ? gaugeWidth * static_cast<float>(s_metuH) / static_cast<float>(s_metuW) : gaugeWidth * 0.25f;

        float margin = 8.0f;
        float headY = center.y - (enemySize.y * 0.5f) - (gaugeHeight * 0.5f) - margin;
        Vector2d gaugePos(center.x, headY);

        float scaleX = (s_metuW > 0) ? (gaugeWidth / static_cast<float>(s_metuW)) : 1.0f;
        float scaleY = (s_metuH > 0) ? (gaugeHeight / static_cast<float>(s_metuH)) : 1.0f;

        renderer->DrawSpriteEx(
            gaugePos,
            scaleX,
            scaleY,
            0.0f,
            s_metuHandle,
            true,
            Vector2d(static_cast<float>(s_metuW), static_cast<float>(s_metuH)) * 0.5f
        );
    }
}