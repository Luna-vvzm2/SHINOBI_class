#pragma once
#include "Scene.h"
#include "Vector2d.h"
#include "Camera.h"
#include "MapData.h"


class PlayerEntity;
class HitEffect;
class TransformComponent;
class VelocityComponent;
class HPComponent;
class GameOverMenuUI;



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

	// プレイヤーをリスポーン位置に戻す
	void RespawnPlayer();
	// ゲームオーバー時の処理
	void ShowGameOverMenu();

private:

	PlayerEntity* m_player;
	HitEffect* m_effect;
	MapData m_mapData;

	Camera m_camera;

	int m_stageIndex;
	int m_comboCount;
	int m_currentStage;
	bool m_resultShown = false;

	// リスポーン位置（初期位置）
	Vector2d m_respawnPos;

	// ゲームオーバーメニュー
	GameOverMenuUI* m_gameOverMenu;
	bool m_isGameOver;
	bool m_isPaused;  // ゲームが一時停止中か

    // 最初に配置された地面のY座標
	float m_initialGroundY = 0.0f;
};