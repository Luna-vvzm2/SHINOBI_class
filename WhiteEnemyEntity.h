#pragma once
#include "EnemyEntity.h"

class WhiteEnemyEntity : public EnemyEntity
{
public:
	WhiteEnemyEntity(Scene* scene, const Vector2d& pos);

	bool Init() override;
	void Update(float deltaTime) override;

	std::string GetTexturePath() const override;

private:
	int m_bulletCount;
	int m_attackOnce[3];
};

