#pragma once
#include "Scene.h"
#include "Vector2d.h"
#include "Camera.h"
#include "MapData.h"


class PlayerEntity;
class HitEffect;



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

	// 最初の地面のY座標を取得
	float GetInitialGroundY() const { return m_initialGroundY; }

private:

	PlayerEntity* m_player;
	HitEffect* m_effect;
	MapData m_mapData;

	Camera m_camera;

	int m_stageIndex;
	int m_comboCount;
	int m_currentStage;
	bool m_resultShown = false;

    // 最初に配置された地面のY座標
	float m_initialGroundY = 0.0f;
};