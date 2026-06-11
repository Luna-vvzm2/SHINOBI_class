#pragma once
#include "EnemyEntity.h"

class GunnerEnemyEntity : public EnemyEntity
{
public:
	GunnerEnemyEntity(Scene* scene, const Vector2d& pos);

	bool Init() override;
	void Update(float deltaTime) override;

	std::string GetTexturePath() const override;

private:
	bool TryGetPlayerInfo(Vector2d& playerPos) const;
	void PlayMotion(
		const std::string& motionName,
		const std::string& texturePath,
		int frameCount,
		float frameSpeed,
		bool loop
	);
	void StartGunAttack(const Vector2d& playerPos);
	float GetDirSign() const;

private:
	int m_bulletCount;
	bool m_attackOnce[3];
	int m_attackDir;
	int m_gunnerState;
	std::string m_currentTexturePath;
};
