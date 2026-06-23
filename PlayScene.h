#pragma once
#include "Scene.h"
#include "Vector2d.h"
#include "Camera.h"
#include "MapData.h"
#include "ShurikenUI.h"
#include "MoneyUI.h"
#include <unordered_map>

class PlayerEntity;
class HitEffect;
class EnemyEntity;
class HPBarUI;
class EnemyHPBar;
class MoneyUI;

class PlayScene : public Scene
{
public:
	//	コンストラクタ
	PlayScene(class Game* game);
	//	デストラクタ
	~PlayScene() override = default;

	bool Init() override;
	//	更新
	void Update(float deltaTime) override;
	void Draw() override;

	PlayerEntity* GetPlayer() const { return m_player; }

	// 衝突イベント時に呼ぶ
	void SpawnHitEffect(const Vector2d& pos);

	void AddCombo();

	int GetCombo() const { return m_comboCount; }

	bool IsResult() const { return m_resultShown; }

	Camera& GetCamera() { return m_camera; }
	const Camera& GetCamera() const { return m_camera; }

private:

	PlayerEntity* m_player;
	HitEffect* m_effect;
	MapData m_mapData;

	Camera m_camera;

	int m_stageIndex;
	int m_comboCount = 0;
	float m_comboTimer = 0.0f; // コンボ表示の残り時間（秒）
	const float COMBO_DISPLAY_TIME = 2.0f; // コンボ表示時間（秒）
	int m_currentStage;
	bool m_resultShown = false;

	ShurikenUI* m_shurikenUI = nullptr;

	MoneyUI* m_moneyUI = nullptr;

	std::unordered_map<EnemyEntity*, EnemyHPBar*> m_enemyToHPBarMap;
};