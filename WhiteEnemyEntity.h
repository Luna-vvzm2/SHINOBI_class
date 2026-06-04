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
	void StartShurikenAttack();
	void StartSwordAttack();

	float GetDirSign() const;

private:
	int m_bulletCount;
	bool m_attackOnce;
	int m_whiteState;
};