#pragma once
#include "EnemyEntity.h"
#include <vector>

class WhiteEnemyEntity : public EnemyEntity
{
public:
	WhiteEnemyEntity(Scene* scene, const Vector2d& pos);

	bool Init() override;
	void Update(float deltaTime) override;

	std::string GetTexturePath() const override;

private:
	bool TryGetPlayerInfo(Vector2d& playerPos, HPComponent*& playerHp, bool& playerOnGround) const;
	void PlayMotion(
		const std::string& motionName,
		const std::string& texturePath,
		int frameCount,
		float frameSpeed,
		bool loop
	);
	void PlaySheetMotion(
		const std::string& motionName,
		const std::vector<int>& frames,
		const std::vector<float>& frameDurations,
		bool loop
	);
	void StartShurikenAttack();
	void StartSwordAttack();

	float GetDirSign() const;
	void PrepareMoveTracking(float wantedMoveX);

private:
	int m_bulletCount;
	bool m_attackOnce;
	int m_whiteState;
	std::string m_currentTexturePath;
	std::string m_currentMotionName;
	bool m_chasePlayer;
	bool m_hasLastMove;
	float m_lastWantedMoveX;
	float m_wallStopTimer;
	Vector2d m_lastMoveStartPos;
};
