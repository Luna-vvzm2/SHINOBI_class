#include "SekienkiBossEntity.h"

#include "PlayScene.h"
#include "PlayerEntity.h"

#include "HPComponent.h"

#include <cmath>

SekienkiBossEntity::SekienkiBossEntity(
    Scene* scene,
    const Vector2d& pos,
    const Vector2d& size)
    : BossEntity(scene, pos, size)

    , m_attackStep(0)
    , m_phase(0)
    , m_attackTimer(1.0f)
    , m_jumpAttackCooldown(0.0f)
    , m_rollDistanceLeft(0.0f)
    , m_bulletActive(false)
    , m_fallStartY(0.0f)
    , m_hoverY(0.0f)
    , m_secondJump(false)
{
}

bool SekienkiBossEntity::Init()
{
    if (!BossEntity::Init())
        return false;

    return true;
}

void SekienkiBossEntity::StartJumpAttack()
{
    m_attackStep = 3;

    m_attackTimer = 1.5f;

    SetVel({ 0.0f,0.0f });

    Vector2d pos = GetPos();
    pos.y -= 200.0f;

    SetPos(pos);

    m_hoverY = pos.y;
}

void SekienkiBossEntity::UpdateAI(float deltaTime)
{

    auto* player =
        static_cast<PlayScene*>(GetScene())->GetPlayer();

    if (!player)
        return;

    if (m_attackStep != 0)
        return;

    if (m_attackTimer > 0.0f)
        return;

    const float distanceX =
        player->GetPos().x - GetPos().x;

    const float absDistance =
        fabsf(distanceX);

    const float dir =
        distanceX > 0.0f ? 1.0f : -1.0f;

    const float farDistance = 150.0f;


    //---------------------------------
    // 行動選択
    //---------------------------------

    if (absDistance >= farDistance)
    {
        m_attackStep = 1;
    }
    else
    {
        m_attackStep =
            (rand() % 100 < 80) ? 2 : 1;
    }

    if (m_hp->GetHP() <= GetMaxHP() * 0.5f)
    {
        if (rand() % 100 < 30)
        {
            m_attackStep = 8;

            SetVel( { -dir * 600.0f,GetVel().y});

            m_rollDistanceLeft = 500.0f;

            return;
        }
    }

    //---------------------------------
    // ローリング
    //---------------------------------

    if (m_attackStep == 1)
    {
        SetVel({ dir * 600.0f, GetVel().y });

        m_rollDistanceLeft = 500.0f;
    }

    //---------------------------------
    // 火炎弾
    //---------------------------------

    else if (m_attackStep == 2)
    {
        m_bulletActive = true;

        if (dir > 0.0f)
        {
            m_bulletPos =
            {
                GetPos().x + 64.0f,
                GetPos().y - 55.0f
            };

            m_bulletVel =
            {
                300.0f,
                0.0f
            };
        }
        else
        {
            m_bulletPos =
            {
                GetPos().x - 80.0f,
                GetPos().y - 55.0f
            };

            m_bulletVel =
            {
                -300.0f,
                0.0f
            };
        }

        m_attackTimer = 0.6f;
    }

    //---------------------------------
    // 飛び上がり
    //---------------------------------

    if (m_jumpAttackCooldown <= 0.0f)
    {
        if (rand() % 100 < 40)
        {
            m_secondJump = false;
            StartJumpAttack();
            return;;
        }
    }
}

void SekienkiBossEntity::UpdateAttack(float deltaTime)
{
    //---------------------------------
    // タイマー
    //---------------------------------

    if (m_attackTimer > 0.0f)
    {
        m_attackTimer -= deltaTime;
    }

    //---------------------------------
    // 火炎攻撃
    //---------------------------------

    if (m_attackStep == 2 &&
        m_bulletActive)
    {
        if (m_attackTimer > 0.45f)
        {
            m_bulletPos.x +=
                m_bulletVel.x * deltaTime;
        }
    }

    //---------------------------------
    // 火炎攻撃終了
    //---------------------------------

    if (m_attackStep == 2 &&
        m_attackTimer <= 0.0f)
    {
        m_bulletActive = false;

        m_attackStep = 0;

        m_attackTimer = 2.0f;
    }

    //---------------------------------
    // 前転攻撃
    //---------------------------------

    if (m_attackStep == 1 &&
        m_rollDistanceLeft > 0.0f)
    {
        float moveStep =
            fabsf(GetVel().x) * deltaTime;

        m_rollDistanceLeft -= moveStep;

        if (m_rollDistanceLeft <= 0.0f)
        {
            m_rollDistanceLeft = 0.0f;

            SetVel({ 0.0f, GetVel().y });

            m_attackStep = 0;

            m_attackTimer = 3.0f;
        }
    }

    //---------------------------------
    // 飛び上がり
    //---------------------------------

    if (m_attackStep == 3)
    {
        SetVel({ 0.0f, 0.0f });

        SetPos(
            {
                GetPos().x,
                m_hoverY
            });
    }

    if (m_jumpAttackCooldown > 0.0f)
    {
        m_jumpAttackCooldown -= deltaTime;
    }

    if (m_attackStep == 3 &&
        m_attackTimer <= 0.0f)
    {
        m_attackStep = 4;

        m_attackTimer = 3.0f;

        SetPos(
            {
                GetPos().x,
                -150.0f
            });
    }

    //---------------------------------
    //空中追尾
    //---------------------------------

    if (m_attackStep == 4)
    {
        auto* player =
            static_cast<PlayScene*>(GetScene())->GetPlayer();

        if (player)
        {
            float dx =
                (player->GetPos().x + 16.0f) -
                (GetPos().x + 32.0f);

            SetVel(
                {
                    dx > 0.0f ?
                    750.0f :
                    -750.0f,
                    0.0f
                });
        }

        if (m_attackTimer <= 1.0f)
        {
            SetVel({ 0.0f,0.0f });
        }

        if (m_attackTimer <= 0.0f)
        {
            m_attackStep = 5;

            m_fallStartY = GetPos().y;
        }
    }

    //---------------------------------
    // 落下攻撃
    //---------------------------------

    if (m_attackStep == 5)
    {
        SetVel({ 0.0f, 1000.0f });

        if (m_isGround)
        {
            m_attackStep = 6;
            m_attackTimer = 0.6f;
        }
    }

    //---------------------------------
    // 落下攻撃終了＆連続飛び上がり
    //---------------------------------

    if (m_attackStep == 6 &&
        m_attackTimer <= 0.0f)
    {
        if (m_hp->GetHP() <= GetMaxHP() * 0.5f && !m_secondJump) 
        {
            m_attackStep = 7;

            m_attackTimer = 1.0f;

            SetVel({ 0.0f, 0.0f });

            printf("STEP7 START\n");
        }
        else
        {
            m_attackStep = 0;

            m_attackTimer = 1.0f;

            m_jumpAttackCooldown = 8.0f;

            printf("STEP6 END\n");
        }
    }

    if (m_attackStep == 7 &&
        m_attackTimer <= 0.0f)
    {
        m_secondJump = true;
        StartJumpAttack();
        //デバッグ用
        //printf("SECOND JUMP\n");
    }

    //---------------------------------
    // 前転攻撃（ダークアタック直前）
    //---------------------------------

    if (m_attackStep == 8 &&
        m_rollDistanceLeft > 0.0f)
    {
        float moveStep =
            fabsf(GetVel().x) * deltaTime;

        m_rollDistanceLeft -= moveStep;

        if (m_rollDistanceLeft <= 0.0f)
        {
            m_rollDistanceLeft = 0.0f;

            SetVel({ 0.0f, GetVel().y });

            m_attackStep = 9;
            m_attackTimer = 1.2f;
        }
    }

    //---------------------------------
    // ダークアタック
    //---------------------------------



    //---------------------------------
    // 形態移行
    //---------------------------------



}