#pragma once
#include "EntityActor.h"
#include "DropData.h"

class PlayerEntity;



class DropItemEntity : public EntityActor
{
public:

	DropItemEntity(Scene* scene, const Vector2d& pos, ItemType type);

	enum class DropState
	{
		Spawn,      // èoåª
		Follow,     // é©ìÆí«è]
		Collected   // âÒé˚çœÇ›
	};

	DropState m_state = DropState::Spawn;

	float m_spawnTimer = 0.0f;

	virtual bool Init() override;
	virtual void Update(float deltaTime) override;
 std::string GetTexturePath() const override;
 virtual ActorType GetType() const override;
private:
	ItemType m_itemType;
	bool m_isCollected = false;
	void ApplyEffect(PlayerEntity* player);
	void MoveAndCollide(float deltaTime);

};
