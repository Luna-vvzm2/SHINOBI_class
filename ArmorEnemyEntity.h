#pragma once
#include "EnemyEntity.h"

class ArmorEnemyEntity : public EnemyEntity
{
public:
	ArmorEnemyEntity(Scene* scene, const Vector2d& pos);

	bool Init() override;
	void Update(float deltaTime) override;

	std::string GetTexturePath() const override;
	int GetGuard() const { return m_guard; }
	int GetGuardMax() const { return m_guardMax; }
	bool IsGuardBroken() const { return m_guard <= 0; }

private:
	bool TryGetPlayerInfo(Vector2d& playerPos, HPComponent*& playerHp) const;
	void UpdateGuardRecover(float deltaTime);
	void PlayMotion(
		const std::string& motionName,
		const std::string& texturePath,
		int frameCount,
		float frameSpeed,
		bool loop
	);
	void StartArmorsAttack();
	float GetDirSign() const;

private:
	bool m_attackOnce;
	int m_armorState;
	float m_guardRecoverTimer;
	std::string m_currentTexturePath;
};
