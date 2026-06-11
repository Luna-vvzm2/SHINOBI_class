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
	bool TryGetPlayerInfo(Vector2d& playerPos, HPComponent*& playerHp) const;
	void PlayMotion(
		const std::string& motionName,
		const std::string& texturePath,
		int frameCount,
		float frameSpeed,
		bool loop
	);
	void StartShurikenAttack();
	void StartSwordAttack();

	float GetDirSign() const;

private:
	int m_bulletCount;
	bool m_attackOnce;
	int m_whiteState;
	std::string m_currentTexturePath;
};
