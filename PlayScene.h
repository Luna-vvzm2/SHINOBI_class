#pragma once
#include "Scene.h"
#include "Vector2d.h"
#include "Camera.h"
#include "MapData.h"
#include <string>
#include <vector>

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
	// ステージ切替 API
	// idx は 0..(m_stageFolders.size()-1)
	void GoToStage(int idx);
	void NextStage();
	// 指定フォルダから再読み込み（map.csv / obj.csv を読み直してそのステージを置き換え）
	bool ReloadStageFromFolder(int idx, const std::string& stageFolder);

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

	void ClearStageActors();   // 現在の地形/オブジェクト/背景 を削除
	void BuildStage(int idx);  // m_mapData.stages[idx] を元にシーンを構築

	// 遷移制御
	bool m_stageTransitionLock = false;
	float m_stageTransitionTimer = 0.0f;
	const float STAGE_TRANSITION_COOLDOWN = 1.0f; // 遷移後1秒はロック
	std::vector<std::string> m_stageFolders;  // Init で読み込んだ stage フォルダパスを保持

	// ====== フェード遷移用 ======
	enum class FadeState {
		None,       // 通常プレイ中
		FadeOut,    // 暗転中（次ステージへ）
		Hold,       // 完全な黒画面のホールド
		FadeIn      // 明転中（新ステージ開始）
	};

	FadeState m_fadeState = FadeState::None;
	float     m_fadeTimer = 0.0f;   // 現在のフェード状態の経過時間
	int       m_pendingStageIndex = -1; // フェードアウト後に切り替えるステージ番号

	// 各フェーズの秒数（動画に合わせて調整）
	static constexpr float FADE_OUT_DURATION = 0.35f;
	static constexpr float FADE_HOLD_DURATION = 0.10f;
	static constexpr float FADE_IN_DURATION = 0.35f;

	// フェード遷移を開始する（次ステージへ）
	void StartFadeToStage(int idx);
	// フェード状態を更新
	void UpdateFade(float deltaTime);
	// フェードのオーバーレイを描画
	void DrawFadeOverlay();
};