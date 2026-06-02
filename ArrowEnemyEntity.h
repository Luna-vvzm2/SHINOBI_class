#pragma once
#include "EnemyEntity.h"

class ArrowEnemyEntity: public EnemyEntity
{
public:
	ArrowEnemyEntity(Scene* scene, const Vector2d& pos);

	bool Init() override;
	void Update(float deltaTime) override;

	std::string GetTexturePath() const override;

private:
	int m_bulletCount;
};

