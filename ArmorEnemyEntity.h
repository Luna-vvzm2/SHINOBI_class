#pragma once
#include "EnemyEntity.h"

class ArmorEnemyEntity : public EnemyEntity
{
public:
	ArmorEnemyEntity(Scene* scene, const Vector2d& pos);

	bool Init() override;
	void Update(float deltaTime) override;

	std::string GetTexturePath() const override;

};

