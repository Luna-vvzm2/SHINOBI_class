#pragma once
#include "EnemyEntity.h"

class YellowEnemyEntity : public EnemyEntity
{
public:
	YellowEnemyEntity(Scene* scene, const Vector2d& pos);

	bool Init() override;
	void Update(float deltaTime) override;
	
	std::string GetTexturePath() const override;
private:
	enum State {
		Idle,
		Dead,
		Chase,
		AttackReady,
		Attack1,
		Attack2,
		Recovery

	};

	State m_state = Idle;

	float m_attackTimer = 0.0f;
	float m_detectRange = 100.0f;
	float m_attackRange = 50.0f;

};

