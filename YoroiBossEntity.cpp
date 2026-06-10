#include "YoroiBossEntity.h"
#include "TransformComponent.h"
#include "VelocityComponent.h"
#include "PlayScene.h"
#include "PlayerEntity.h"
#include <cmath>

YoroiBossEntity::YoroiBossEntity(Scene* scene, const Vector2d& pos, const Vector2d& size)
    : BossEntity(scene, pos, size)
    , m_attackTimer(1.0f)
    , m_rollDistanceLeft(0.0f)
    , m_attackStep(-1)
    , m_darkAttackCount(0)
    , m_maxDarkAttacks(0)
    , m_bulletActive(false)
{
    printf("Yoroi Spawn\n");
}

bool YoroiBossEntity::Init()
{
    if (!BossEntity::Init()) return false;
    // 必要ならここで初期化処理を追加
    return true;
}

void YoroiBossEntity::UpdateAI(float deltaTime)
{
    if (m_attackTimer > 0.0f)
    {
        m_attackTimer -= deltaTime;
        return;
    }

    auto* player =
        static_cast<PlayScene*>(GetScene())->GetPlayer();

    if (!player) return;

    const float distanceX =
        player->GetPos().x - GetPos().x;

    const float dir =
        (distanceX > 0.0f) ? 1.0f : -1.0f;

    // 移動中は新しい攻撃をしない
    if (fabsf(GetVel().x) > 0.0f)
        return;

    // 初回行動決定
    if (m_attackStep == -1)
    {
        m_attackStep = rand() % 2;
    }

    //---------------------------------
    // ローリング
    //---------------------------------
    if (m_attackStep == 0)
    {
        SetVel({ dir * 1000.0f, GetVel().y });

        m_rollDistanceLeft = 1000.0f;

        m_attackStep = 1;
        m_darkAttackCount = 0;
    }
    //---------------------------------
    // ダークアタック
    //---------------------------------
    else
    {
        if (m_darkAttackCount == 0)
        {
            m_maxDarkAttacks = (rand() % 2) + 1;
        }

        Vector2d pos = GetPos();

        m_bulletPos =
        {
            pos.x + 32.0f,
            pos.y - 32.0f
        };

        const float bulletSpeed = 500.0f;

        m_bulletVel =
        {
            (distanceX > 0.0f) ? bulletSpeed : -bulletSpeed,
            0.0f
        };

        m_bulletActive = true;

        m_darkAttackCount++;

        if (m_darkAttackCount >= m_maxDarkAttacks)
        {
            m_attackStep = 0;
        }

        m_attackTimer = 3.0f;
    }
}

void YoroiBossEntity::UpdateAttack(float deltaTime)
{

    //---------------------------------
    // ローリング移動
    //---------------------------------
    if (m_rollDistanceLeft > 0.0f)
    {
        float moveStep =
            fabsf(GetVel().x) * deltaTime;

        m_rollDistanceLeft -= moveStep;

        if (m_rollDistanceLeft <= 0.0f)
        {
            m_rollDistanceLeft = 0.0f;

            SetVel({ 0.0f, GetVel().y });

            m_attackTimer = 3.0f;
        }
    }

    //---------------------------------
    // 弾更新
    //---------------------------------
    if (m_bulletActive)
    {
        m_bulletPos += m_bulletVel * deltaTime;

        constexpr float limit = 10000.0f;

        if (m_bulletPos.x < -100.0f ||
            m_bulletPos.x > limit)
        {
            m_bulletActive = false;
        }
    }
}