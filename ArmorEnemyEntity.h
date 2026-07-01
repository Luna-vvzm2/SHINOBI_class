#pragma once
#include "EnemyEntity.h"

class PlayerEntity;

class ArmorEnemyEntity : public EnemyEntity
{
public:
	ArmorEnemyEntity(Scene* scene, const Vector2d& pos);

	bool Init() override;
	void Update(float deltaTime) override;
	void TakeDamage(int damage, const Vector2d& knockback);

	std::string GetTexturePath() const override;
	int GetGuard() const { return m_guard; }
	int GetGuardMax() const { return m_guardMax; }
	bool IsGuardBroken() const { return m_guard <= 0; }

private:
	bool TryGetPlayerInfo(Vector2d& playerPos, PlayerEntity*& player) const;
	void UpdateGuardRecover(float deltaTime);
	void PlayMotion(
		const std::string& motionName,
		const std::string& texturePath,
		int frameCount,
		float frameSpeed,
		bool loop
	);
	void StartArmorsAttack();
	void TriggerExplosion(const Vector2d& playerPos, PlayerEntity* player);
	void OnHPChanged(int newHP, int oldHP);
	void StartKnockback(const Vector2d& knockback);
	void BreakGuard(const Vector2d& knockback);
	void RecoverGuard();
	float GetDirSign() const;

private:
	bool m_attackOnce;
	int m_armorState;
	float m_guardRecoverTimer;
	bool m_ignoreHPChange;
	bool m_guardRecovering;
	float m_guardRecoverMotionTimer;
	float m_knockbackTimer;
	std::string m_currentTexturePath;
};
