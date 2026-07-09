#include "SekienkiBossEntity.h"
#include "TransformComponent.h"
#include "VelocityComponent.h"
#include "HPComponent.h"
#include "PlayScene.h"
#include "PlayerEntity.h"
#include "SpriteComponent.h"
#include "AnimationComponent.h"
#include "Game.h"
#include "Renderer.h"
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
    , m_dead(false)
{
}

bool SekienkiBossEntity::Init()
{
    if (!BossEntity::Init())
        return false;

    m_hp = AddComponent<HPComponent>(GetMaxHP());

    // コンポーネントの取得
    m_anim = GetComponent<AnimationComponent>();

    if (m_sprite)
    {
        m_sprite->LoadTextureDiv("assets/images/enemy/sekienki/Sekienki01.png", 4, 8);
    }

    if (m_anim && m_sprite)
    {
        m_anim->SetSprite(m_sprite);

        //-----------------------------------------
        // 1. 立ち（待機） 
        //-----------------------------------------
        AnimationClip idleClip;
        idleClip.frames = { 1, 2, 3, 4, 5, 6 };
        idleClip.frameDurations = {
            1.0f / 24.0f, 1.0f / 24.0f, 1.0f / 24.0f,
            1.0f / 24.0f, 1.0f / 24.0f, 1.0f / 24.0f
        };
        idleClip.loop = true;
        m_anim->AddClip("idle", idleClip);

        //-----------------------------------------
        // 2. 振り向き 2枚 
        //-----------------------------------------
        AnimationClip turnClip;
        turnClip.frames = { 7, 7 };
        turnClip.frameDurations = {
            2.0f / 24.0f, // 07 を 2フレーム分
            3.0f / 24.0f  // 07 を 3フレーム分
        };
        turnClip.loop = false;
        m_anim->AddClip("turn", turnClip);

        //-----------------------------------------
        // 3. 近接攻撃(火炎) 
        //-----------------------------------------
        AnimationClip attackClip;
        attackClip.frames = { 8, 22, 23, 24, 25, 26, 27, 28, 29, 28, 8 };
        attackClip.frameDurations = {
            1.0f / 24.0f, 1.0f / 24.0f, 1.0f / 24.0f, 1.0f / 24.0f,
            1.0f / 24.0f, 1.0f / 24.0f, 1.0f / 24.0f, 1.0f / 24.0f,
            1.0f / 24.0f, 1.0f / 24.0f, 1.0f / 24.0f
        };
        attackClip.loop = false;
        m_anim->AddClip("attack", attackClip);

        //-----------------------------------------
        // 4. ローリング 
        //-----------------------------------------
        AnimationClip rollClip;
        rollClip.frames = {
            8, 9, 10, 11, 12, 13, 14,          // 08~14
            9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 // 09~21
        };
        rollClip.frameDurations.assign(rollClip.frames.size(), 1.0f / 24.0f); // 全て 1/24秒
        rollClip.loop = true; // 突進中ループさせる場合は true
        m_anim->AddClip("roll", rollClip);

        //-----------------------------------------
        // 5. ドラミング(形態変化) 
        //-----------------------------------------
        AnimationClip drummingClip;
        drummingClip.frames = {
            14,                                 // 14
            9, 10, 11, 12, 13, 14,             // 09~14
            9, 10, 11, 12, 13, 14,             // 09~14
            8, 9, 10, 11, 12, 13, 14,          // 08~14
            9, 10, 11, 12, 13                  // 09~13
        };
        drummingClip.frameDurations.assign(drummingClip.frames.size(), 1.0f / 24.0f); // 全て 1/24秒
        drummingClip.loop = false;
        m_anim->AddClip("drumming", drummingClip);

        // 最初は待機モーションを再生
        m_anim->Play("idle");
    }

    return true;
}

void SekienkiBossEntity::Update(float deltaTime)
{
    BossEntity::Update(deltaTime);

    if (m_dead) return;

    if (m_sprite)
    {
        float vx = GetVel().x;

        // 速度が一定以上なら向いている方向を更新
        if (vx > 0.1f)
        {
            m_sprite->SetFlipX(true);  // 右移動時は反転（※画像素材が左向きベースの場合）
        }
        else if (vx < -0.1f)
        {
            m_sprite->SetFlipX(false); // 左移動時はそのまま
        }
    }
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

    if (m_phase < 2 && hpRate <= 0.5f)
    {
        m_phase = 2;

        m_attackStep = 12;

        m_attackTimer = 2.0f;

        SetVel({ 0.0f,0.0f });

        m_hp->SetInvincible(999.0f);
    }
    else if (m_phase < 1 && hpRate <= 0.7f)
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
    if (m_dead) return;

    PhaseChange();
    if (m_attackStep == 11 || m_attackStep == 12)
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
    if (m_dead) return;

    //---------------------------------
    // デバッグ用(attackStep確認)
    //---------------------------------

    /*static int prevStep = -1;

    if (prevStep != m_attackStep)
    {
        prevStep = m_attackStep;
    }*/


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

void SekienkiBossEntity::TakeDamage(int damage, const Vector2d& knockback)
{
    // すでに死亡しているなら何もしない
    if (m_dead) return;

    if (m_hp)
    {
        // HPを減らす
        m_hp->Damage(damage);

        // HPが0以下になったら死亡処理へ
        if (m_hp->GetHP() <= 0)
        {
            m_dead = true;
            m_attackStep = 0;       // 攻撃ステートをリセット
            m_attackTimer = 0.0f;   // タイマー停止
            SetVel({ 0.0f, 0.0f }); // その場に停止
            m_bulletActive = false; // 攻撃用の弾（火炎など）を強制非アクティブ化

            // 死亡アニメーションを再生
            if (m_anim)
            {
                m_anim->Play("dead");
            }
        }
    }
}