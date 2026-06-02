#pragma once
#include "EnemyEntity.h"

class YellowEnemyEntity : public EnemyEntity
{
public:
	YellowEnemyEntity(Scene* scene, const Vector2d& pos);

	bool Init() override;
	void Update(float deltaTime) override;

	std::string GetTexturePath() const override;

};

