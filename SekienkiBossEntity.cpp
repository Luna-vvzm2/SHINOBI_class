#include "SekienkiBossEntity.h"

#include "PlayScene.h"
#include "PlayerEntity.h"

#include "HPComponent.h"
#include <DxLib.h>

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
    , m_tornadoDistanceLeft(0.0f)
{   
}

bool SekienkiBossEntity::Init()
{
    if (!BossEntity::Init())
        return false;

    return true;
}

//---------------------------------
// 飛び上がり攻撃管理
//---------------------------------

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

//---------------------------------
// 形態移行管理
//---------------------------------

void SekienkiBossEntity::PhaseChange()
{
    float hpRate =
        (float)m_hp->GetHP() / GetMaxHP();

    if (m_phase < 2 &&hpRate <= 0.5f)
    {
        m_phase = 2;

        m_attackStep = 12;

        m_attackTimer = 2.0f;

        SetVel({ 0.0f,0.0f });

        m_hp->SetInvincible(999.0f);
    }
    else if (m_phase < 1 &&hpRate <= 0.7f)
    {
        m_phase = 1;

        m_attackStep = 11;

        m_attackTimer = 2.0f;

        SetVel({ 0.0f,0.0f });

        m_hp->SetInvincible(999.0f);
    }
}



void SekienkiBossEntity::UpdateAI(float deltaTime)
{
    PhaseChange();
    if (m_attackStep == 11 ||m_attackStep == 12)
    {
        return;
    }


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

    int r = rand() % 100;

    float hp70 = GetMaxHP() * 0.7f;
    float hp50 = GetMaxHP() * 0.5f;

    //---------------------------------
    // HP50%
    //---------------------------------
    if (m_hp->GetHP() <= hp50)
    {
        if (r < 50)
        {
            m_attackStep = 8;
            SetVel({ -dir * 600.0f, GetVel().y });
            m_rollDistanceLeft = 500.0f;
            return;
        }
        else if (r < 90)
        {
            m_secondJump = false;
            StartJumpAttack();
            return;
        }
    }

    //---------------------------------
    // HP70%
    //---------------------------------
    else if (m_hp->GetHP() <= hp70)
    {
        if (r < 40)
        {
            m_secondJump = false;
            StartJumpAttack();
            return;
        }
    }

    //---------------------------------
    // 基本行動
    //---------------------------------
    if (absDistance >= farDistance)
    {
        m_attackStep = 1;
    }
    else
    {
        m_attackStep = (r < 80) ? 2 : 1;
    }

    //---------------------------------
    // 前転攻撃
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
        float hp70 = GetMaxHP() * 0.7f;
        float hp50 = GetMaxHP() * 0.5f;

        if (m_hp->GetHP() > hp70)
            return;

        if (rand() % 100 < 40)
        {
            m_secondJump = false;
            StartJumpAttack();
            return;
        }
    }
}

void SekienkiBossEntity::UpdateAttack(float deltaTime)
{
    //---------------------------------
    // デバッグ用(attackStep確認)
    //---------------------------------
    
    static int prevStep = -1;

    if (prevStep != m_attackStep)
    {
        prevStep = m_attackStep;
    }
 

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
        if (m_phase == 2 && !m_secondJump) 
        {
            m_attackStep = 7;

            m_attackTimer = 1.0f;

            SetVel({ 0.0f, 0.0f });

        }
        else
        {
            m_attackStep = 0;

            m_attackTimer = 1.0f;

            m_jumpAttackCooldown = 8.0f;

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

    if (m_attackStep == 9)
    {
        SetVel({ 0.0f, 0.0f });

        if (m_attackTimer <= 0.0f)
        {
            m_bulletActive = true;

            m_bulletPos =
            {
                GetPos().x,
                GetPos().y - 120.0f
            };

            auto* player =
                static_cast<PlayScene*>(GetScene())->GetPlayer();

            if (player)
            {
                float dir =
                    player->GetPos().x > GetPos().x
                    ? 1.0f
                    : -1.0f;

                m_bulletVel =
                {
                    dir * 300.0f,
                    0.0f
                };
            }

            m_tornadoDistanceLeft = 600.0f;

            m_attackStep = 10;
        }
    }

    if (m_attackStep == 10 &&
        m_bulletActive)
    {
        m_bulletPos.x +=
            m_bulletVel.x * deltaTime;

        float moveStep =
            fabsf(m_bulletVel.x) * deltaTime;

        m_tornadoDistanceLeft -= moveStep;

        if (m_tornadoDistanceLeft <= 0.0f)
        {
            m_bulletActive = false;

            m_attackStep = 0;

            m_attackTimer = 1.0f;

            m_jumpAttackCooldown = 7.0f;
        }
    }
    //---------------------------------
    // 形態移行管理
    //---------------------------------

    if (m_attackStep == 11)
    {
        if (m_attackTimer <= 0.0f)
        {
            m_hp->SetInvincible(0.0f);

            m_attackStep = 0;

            m_attackTimer = 1.0f;
        }

        return;
    }

    if (m_attackStep == 12)
    {
        if (m_attackTimer <= 0.0f)
        {
            m_hp->SetInvincible(0.0f);

            m_attackStep = 0;

            m_attackTimer = 1.0f;
        }

        return;
    }

}