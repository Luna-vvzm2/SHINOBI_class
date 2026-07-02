#include "DropItemEntity.h"
#include "TransformComponent.h"
#include "VelocityComponent.h"
#include "SpriteComponent.h"
#include "CollisionComponent.h"
#include "PlayScene.h"
#include "PlayerEntity.h"

#include "DropData.h"
#include <DxLib.h>

DropItemEntity::DropItemEntity(Scene* scene, const Vector2d& pos, ItemType type) 
	: EntityActor(scene, pos, Vector2d(48, 48))
	, m_itemType(type) 
{

}

bool DropItemEntity::Init()
{
 
	if (!EntityActor::Init())
		return false;

    GetTransform()->SetScale(Vector2d(0.3f, 0.3f));
   
    return true;
}

void DropItemEntity::Update(float deltaTime)
{
    PlayScene* playScene = dynamic_cast<PlayScene*>(m_scene);
    if (!playScene) return;

    PlayerEntity* player = playScene->GetPlayer();
    if (!player) return;

    Vector2d diff = player->GetPos() - GetPos();
    float dist = diff.length();


    switch (m_state)
    {
    case DropState::Spawn:
        m_velocity->Set(Vector2d::Zero());

        m_spawnTimer += deltaTime;

        if (m_spawnTimer >= 0.2f)
        {
            m_velocity->Set(Vector2d::Zero());
            m_state = DropState::Follow;
        }
        break;

    case DropState::Follow:
    {
        Vector2d dir = player->GetPos() - GetPos();
        float dist = dir.length();

        if (dist < 32.0f)
        {
            m_state = DropState::Collected;
            return;
        }

        if (dist > 0.01f)
        {
            dir.normalize();
            m_velocity->Set(dir * 5.0f);
        }
        break;
    }
    case DropState::Collected:
    {

        m_velocity->Set(Vector2d::Zero());

        ApplyEffect(player);

        Destroy();


        return;
    }
    }
    MoveAndCollide(deltaTime);
}

void DropItemEntity::ApplyEffect(PlayerEntity* player)
{
    if (!player)
        return;

   ;
    switch (m_itemType)
    {
    case ItemType::Coin:
        // 例：スコア加算や所持金加算
        player->AddCoin(1);
        break;

    case ItemType::Heal:
        // 例：HP回復
        player->HealHP(20);
        break;

    case ItemType::Kunai:
        // 例：クナイ
        player->AddKunai(1);
        break;

    case ItemType::Haku:
        // 例：拍
        player->AddHaku(20);
        break;

    default:
        break;
    }
}

std::string DropItemEntity::GetTexturePath() const
{
    switch (m_itemType)
    {
    case ItemType::Coin:
        return "assets/images/DropItem/coin.png";

    case ItemType::Heal:
        return "";

    case ItemType::Kunai:
        return "";

    case ItemType::Haku:
        return "";
    default:
        return "";
    }
}


ActorType DropItemEntity::GetType() const
{
    return ActorType::DropItem;
}

void DropItemEntity::MoveAndCollide(float deltaTime)
{
    Vector2d pos = GetTransform()->GetPosition();
    Vector2d vel = m_velocity->Get(); // ← Getが無ければ合わせて修正

    Vector2d nextPos = pos + vel * deltaTime;

    // ① 位置更新
    GetTransform()->SetPosition(nextPos);

}