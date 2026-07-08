#include "Trap.h"
#include "PlayScene.h"
#include "PlayerEntity.h"
#include "CollisionComponent.h"
#include "HPComponent.h"

Trap::Trap(Scene* scene, const Vector2d& pos, const Vector2d& size)
    : EntityActor(scene, pos, Vector2d(64, 64)),
    m_damage(1000),            // 例: 1000ダメージ
    m_currentCooldown(0.0f)
{
} 

bool Trap::Init() {
    if (!EntityActor::Init()) return false;

    if (m_collision) {
        m_collision->SetRect(64, 64); // 当たり判定のサイズ
    }

    return true;
}

void Trap::Update(float deltaTime) {
    EntityActor::Update(deltaTime);

    // クールダウンを毎フレーム減らす処理を追加
    if (m_currentCooldown > 0.0f) {
        m_currentCooldown -= deltaTime;
    }

    // PlayScene経由でプレイヤーを取得
    PlayScene* playScene = dynamic_cast<PlayScene*>(m_scene);
    if (!playScene) return;

    PlayerEntity* player = playScene->GetPlayer();
    if (!player) return;

    CollisionComponent* playerCol = player->GetCollision();

    // ダメージ判定処理
    // クールダウンが0以下の時のみ判定を行う
    if (m_currentCooldown <= 0.0f && m_collision && playerCol) {

        // 交差判定
        if (m_collision->CheckCollision(playerCol)) {
            HPComponent* playerHP = player->GetHP();

            if (playerHP) {
                playerHP->Damage(m_damage); // HPを減らす

                // クールダウンをリセット（例: 1秒間は連続ヒットしない）
                m_currentCooldown = 1.0f;
            }
        }
    }
}