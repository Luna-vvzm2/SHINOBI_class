#define NOMINMAX
#include "PlayScene.h"
#include "TransformComponent.h"
#include "VelocityComponent.h"
#include "HPComponent.h"
#include "GameOverMenuUI.h"
#include "Game.h"
#include "Renderer.h"
#include "Input.h"
#include "PlayerEntity.h"
#include "WhiteEnemyEntity.h"
#include "YellowEnemyEntity.h"
#include "ArrowEnemyEntity.h"
#include "HealerEnemyEntity.h"
#include "ArmorEnemyEntity.h"
#include "GunnerEnemyEntity.h"
#include "YoroiBossEntity.h"
#include "SekienkiBossEntity.h"
#include "HitEffect.h"
#include "GroundBlock.h"
#include "HPBarUI.h"
#include "BackGroundUI.h"

#include "Camera.h"
#include <iostream>
#include <algorithm>

PlayScene::PlayScene(Game* game)
	: Scene(game),
	m_player(nullptr),
	m_effect(nullptr),
	m_camera(static_cast<float>(game->GetWidth()), static_cast<float>(game->GetHeight())),
	m_stageIndex(0),
	m_comboCount(0),
	m_currentStage(1),
	m_respawnPos(200, 800),
	m_gameOverMenu(nullptr),
	m_isGameOver(false),
	m_isPaused(false)
{

}

bool PlayScene::Init() {
	m_isRunning = true;
	m_type = Type::Play;
	m_stageIndex = 0;
	
	// CSV からマップ読み込み
	if (!m_mapData.LoadStage("assets/maps/stage1")) {
		std::cerr << "マップ１読み込み失敗" << std::endl;
		return false;
	}
	if (!m_mapData.LoadStage("assets/maps/stage2")) {
		std::cerr << "マップ２読み込み失敗" << std::endl;
		return false;
	}
	if (!m_mapData.LoadStage("assets/maps/stage3")) {
		std::cerr << "マップ３読み込み失敗" << std::endl;
		return false;
	}
	m_mapData.tileSize = 104;

	StageData& stage = m_mapData.stages[m_stageIndex];

	// マップのタイルを配置する
	const float tileSize = static_cast<float>(m_mapData.tileSize);
	for (const auto& layer : stage.layers) {
		if (!layer.visible) continue;
		const Layer& mapLayer = stage.layers[0];

		for (int y = 0; y < stage.height; ++y) {
			for (int x = 0; x < stage.width; ++x) {
				int tileID = mapLayer.tiles[y * stage.width + x];
				if (tileID == 0) continue; // 空タイル

				Vector2d pos(x * tileSize, y * tileSize);

				// ここでタイルIDに応じてブロック生成
				// 例：GrassBlock と IceBlock を仮に切り替え
				switch (tileID) {
				case 1:
					AddActor(new GroundBlock(this, pos, Vector2d(tileSize, tileSize)));
					break;
				}
			}
		}

		const Layer& objLayer = stage.layers[1];

		for (int y = 0; y < stage.height; y++)
		{
			for (int x = 0; x < stage.width; x++)
			{
				int objID =
					objLayer.tiles[y * stage.width + x];

				Vector2d pos(x * tileSize, y * tileSize);

				switch (objID)
				{
				case 1:
					AddActor(new WhiteEnemyEntity(this, pos));
					break;

				case 2:
					AddActor(new YellowEnemyEntity(this, pos));
					break;

				case 3:
					AddActor(new ArrowEnemyEntity(this, pos));
					break;

				case 4:
					AddActor(new HealerEnemyEntity(this, pos));
					break;

				case 5:
					AddActor(new ArmorEnemyEntity(this, pos));
					break;

				case 6:
					AddActor(new GunnerEnemyEntity(this, pos));
					break;

				case 7:
					AddActor(new YoroiBossEntity(this, pos,Vector2d(192, 192)));
					break;

				case 8:
					AddActor(new SekienkiBossEntity(this,pos,Vector2d(192, 192)));
					break;
				}
			}
		}
	}
	
	// MapData に全ステージを読み込む
	for (const auto& f : m_stageFolders) {
		if (!m_mapData.LoadStage(f)) {
			std::cerr << "Failed to load stage: " << f << std::endl;
			return false;
		}
	}
	// リスポーン位置を初期位置として保存
	m_respawnPos = Vector2d(200, 800);

	m_player = new PlayerEntity(this, m_respawnPos, Vector2d({ 192, 64
		}));
	AddActor(m_player);
	
	// ---- HP UI 作成 ----
	HPBarUI* hpBar = new HPBarUI(
		this,
		m_player->GetHP()
	);
	AddUIActor(hpBar);

	// ---- ゲームオーバーメニューUI 作成 ----
	m_gameOverMenu = new GameOverMenuUI(this);
	AddUIActor(m_gameOverMenu);

	BackGroundUI* back = new BackGroundUI(this, "assets/images/uies/bg.png");
	AddBackActor(back);


	float halfTile = m_mapData.tileSize * 0.5f;
	m_camera.SetTileHalfSize(Vector2d(halfTile, halfTile));

	// Camera のマップ範囲設定
	float mapW = (float)stage.width * m_mapData.tileSize;
	float mapH = (float)stage.height * m_mapData.tileSize;
	m_camera.SetBounds(Vector2d(0, 0), Vector2d(mapW, mapH));

	// 最初のステージを組み立てる
	m_stageIndex = 0;
	BuildStage(m_stageIndex);

	// Renderer に Camera をセット
	m_game->GetRenderer()->SetCamera(&m_camera);

	return true;
}

void PlayScene::Update(float deltaTime) {
	// ゲームオーバー中はゲーム更新を停止
	if (!m_isPaused) {
		updateActors(m_backactors, deltaTime);
		updateActors(m_actors, deltaTime);
	}
	updateActors(m_UIactors, deltaTime);

	if (!m_isPaused) {
		RemoveDeadActors();
	}

	// ゲームオーバーメニューの処理
	if (m_gameOverMenu && m_gameOverMenu->IsActive()) {
		if (m_gameOverMenu->IsDecided()) {
			m_gameOverMenu->ResetDecided();

			switch (m_gameOverMenu->GetSelectedItem()) {
			case GameOverMenuUI::MenuItem::CONTINUE:
				// コンティニュー：リスポーン
				m_isPaused = false;
				m_isGameOver = false;
				m_gameOverMenu->SetActive(false);
				RespawnPlayer();
				break;

			case GameOverMenuUI::MenuItem::WORLD_MAP:
				// ワールドマップ画面へ移動
				// TODO: ワールドマップシーンへの遷移処理を追加
				std::cout << "Transition to World Map (not implemented yet)" << std::endl;
				break;

			case GameOverMenuUI::MenuItem::TITLE:
				// タイトル画面へ移動
				m_isRunning = false;  // PlaySceneを終了
				break;

			default:
				break;
			}
		}
	}
	if (m_player) {
		Vector2d playerPos = m_player->GetComponent<TransformComponent>()->GetPosition();

		// 中間点をカメラ位置に
		Vector2d camPos = playerPos;
		camPos.y -= 200;
		m_camera.SetCenter(camPos);

		// 距離に応じたズーム
		/*
		float distance = (playerPos).length();
		float minZoom = 1.0f;
		float maxZoom = 2.5f;
		float idealDistance = 300.0f;
		float targetZoom = std::clamp(idealDistance / distance, minZoom, maxZoom);

		// 線形補間で徐々にズーム変更
		float currentZoom = m_camera.GetZoom();
		float zoomSpeed = 5.0f;
		float newZoom = currentZoom + (targetZoom - currentZoom) * std::min(zoomSpeed * deltaTime, 1.0f);
		*/
		m_camera.SetZoom(0.8f);

		float fixedCameraY = 400.0f;  // スクロール開始位置の上限

		if (playerPos.y < fixedCameraY) {
			camPos.y = playerPos.y - 200;  // 上に移動したらカメラもスクロール
		}
		else {
			camPos.y = fixedCameraY - 200;  // それ以外はカメラ固定
		}
	}
	// 遷移ロックのタイマー更新
	if (m_stageTransitionLock) {
		m_stageTransitionTimer -= deltaTime;
		if (m_stageTransitionTimer <= 0.0f) {
			m_stageTransitionLock = false;
			m_stageTransitionTimer = 0.0f;
		}
	}

	
    // 「最後のステージなら右に進めない」処理
	if (m_player && !m_mapData.stages.empty()) {
		int lastIndex = static_cast<int>(m_mapData.stages.size()) - 1;
		if (m_stageIndex == lastIndex) {
			// 現在のマップ幅（ワールド座標）
			StageData& cur = m_mapData.stages[m_stageIndex];
			float mapWidth = static_cast<float>(cur.width) * m_mapData.tileSize;

			// 右端の閾値（マップ右端からこのピクセル分内が最右可動範囲）
			const float RIGHT_THRESHOLD = 16.0f; // 必要に応じ調整（例: 32.0f, 64.0f）

			// プレイヤーの位置取得
			auto transform = m_player->GetComponent<TransformComponent>();
			if (transform) {
				Vector2d pos = transform->GetPosition();

				// 右に出ていたら戻す
				if (pos.x > mapWidth - RIGHT_THRESHOLD - 150) {
					pos.x = mapWidth - RIGHT_THRESHOLD - 150;
					transform->SetPosition(pos);

					// 水平移動を止める（VelocityComponent があれば X 方向ゼロにする）
					auto velComp = m_player->GetComponent<VelocityComponent>();
					if (velComp) {
						Vector2d v = velComp->Get();
						v.x = 0.0f;
						velComp->Set(v);
					}

					// 必要ならプレイヤーの入力フラグをオフにする等の追加処理を行ってください
					// 例: m_player->SetCanMove(false); // ただしこれだと左右とも動けなくなるため注意
				}
			}
		}
	}

	// プレイヤーが右端に到達したら遷移（プレイヤーの右端が map width を越えたら）
	// 例: Update 内の右端チェック部分（呼び出し前に確認）
	if (!m_stageTransitionLock && m_player) {
		StageData& cur = m_mapData.stages[m_stageIndex];
		float mapWidth = static_cast<float>(cur.width) * m_mapData.tileSize;
		const float threshold = 16.0f;
		auto transform = m_player->GetComponent<TransformComponent>();
		if (transform) {
			Vector2d p = transform->GetPosition();
			if (p.x > mapWidth - threshold) {
				// 最後のステージなら遷移しない
				if (m_stageIndex + 1 >= static_cast<int>(m_mapData.stages.size())) {
					// 最後なら右端に戻す処理（押し戻し）
					p.x = mapWidth - threshold;
					transform->SetPosition(p);
					if (auto vel = m_player->GetComponent<VelocityComponent>()) {
						Vector2d v = vel->Get(); v.x = 0.0f; vel->Set(v);
					}
					// ロック設定（任意）
					m_stageTransitionLock = true; m_stageTransitionTimer = STAGE_TRANSITION_COOLDOWN;
				}
				else {
					// 次があるなら遷移
					NextStage();
				}
			}
		}
	}
}

void PlayScene::Draw() {
	Renderer* renderer = m_game->GetRenderer();



	// --- アクター描画 ---
	drawActors(m_backactors);
	drawActors(m_actors);
	drawActors(m_UIactors);

	std::vector<NumberInfo> comboInfo = {
		{ (float)m_comboCount, 0 }
	};

	if (m_resultShown) {
		const std::string& debugFont = m_game->GatDebugFont();
		renderer->DrawNumberFormatW(
			Vector2d(m_game->GetWidth() / 2.4f, m_game->GetHeight() / 2.2f),
			Color(0, 0, 0),
			debugFont,
			32,
			"{0} Combo",
			comboInfo,
			false
		);
	}

	// --- デバッグ文字 ---
#ifdef _DEBUG
	const std::string& debugFont = m_game->GatDebugFont();
	renderer->DrawTextL(Vector2d(m_game->GetWidth() - 150.0f, 0), "PlayScene", Color(255, 64, 0), debugFont, 24, false);
#endif
}

void PlayScene::SpawnHitEffect(const Vector2d& pos) {
	m_effect = new HitEffect(this, pos, {32, 32});
	AddActor(m_effect);
	std::cout << "Spawned HitEffect at: " << pos.x << ", " << pos.y << std::endl;
	
}

void PlayScene::AddCombo() {
	m_comboCount++;
	std::cout << "Combo: " << m_comboCount << std::endl;
}

void PlayScene::RespawnPlayer() {
	if (!m_player) return;

	// プレイヤーの位置をリスポーン位置に戻す
	TransformComponent* transform = m_player->GetComponent<TransformComponent>();
	if (transform) {
		transform->SetPosition(m_respawnPos);
	}

	// プレイヤーの速度をリセット
	VelocityComponent* velocity = m_player->GetComponent<VelocityComponent>();
	if (velocity) {
		velocity->Set(Vector2d::Zero());
	}

	// HPを最大値に回復
	HPComponent* hp = m_player->GetHP();
	if (hp) {
		hp->Heal(hp->GetMaxHP());
	}

	std::cout << "Player respawned at: " << m_respawnPos.x << ", " << m_respawnPos.y << std::endl;
}
void PlayScene::ShowGameOverMenu() {
	m_isGameOver = true;
	m_isPaused = true;

	if (m_gameOverMenu) {
		m_gameOverMenu->SetActive(true);
		std::cout << "Game Over Menu displayed" << std::endl;
	}
}
void PlayScene::ClearStageActors()
{
	// m_actors: world の Actor（プレイヤー含むことがある）を走査
	for (auto it = m_actors.begin(); it != m_actors.end();) {
		Actor* a = *it;
		if (a == m_player) {
			++it; // プレイヤーは残す
		}
		else {
			// delete とベクタから除去
			delete a;
			it = m_actors.erase(it);
		}
	}

	// バックグラウンドも差し替えたいのでクリア
	for (Actor* b : m_backactors) delete b;
	m_backactors.clear();

	// UI は m_UIactors にあるので触らない（HP/Money UI 等を残す）
}
void PlayScene::BuildStage(int idx)
{
	if (idx < 0 || idx >= static_cast<int>(m_mapData.stages.size())) {
		std::cerr << "[BuildStage] invalid index: " << idx << std::endl;
		return;
	}

	// 既存ステージを削除（プレイヤーと UI は残る）
	ClearStageActors();

	StageData& stage = m_mapData.stages[idx];
	const float tileSize = static_cast<float>(m_mapData.tileSize);

	// find layers
	const Layer* mapLayer = nullptr;
	const Layer* objLayer = nullptr;
	for (auto& lyr : stage.layers) {
		if (lyr.name == "Map") mapLayer = &lyr;
		if (lyr.name == "Object") objLayer = &lyr;
	}
	// defensive
	if (!mapLayer && !stage.layers.empty()) mapLayer = &stage.layers[0];
	if (!objLayer && stage.layers.size() >= 2) objLayer = &stage.layers[1];

	// Map layer -> ground blocks etc.
	if (mapLayer) {
		for (int y = 0; y < stage.height; ++y) {
			for (int x = 0; x < stage.width; ++x) {
				int tileID = mapLayer->tiles[y * stage.width + x];
				if (tileID == 0) continue;
				Vector2d pos(x * tileSize, y * tileSize);
				switch (tileID) {
				case 1:
					AddActor(new GroundBlock(this, pos, Vector2d(tileSize, tileSize)));
					break;
					// 必要に応じて他タイルIDを追加
				default:
					break;
				}
			}
		}
	}

	// Object layer -> enemies / bosses
	if (objLayer) {
		for (int y = 0; y < stage.height; ++y) {
			for (int x = 0; x < stage.width; ++x) {
				int objID = objLayer->tiles[y * stage.width + x];
				if (objID == 0) continue;
				Vector2d pos(x * tileSize, y * tileSize);
				switch (objID) {
				case 1: AddActor(new WhiteEnemyEntity(this, pos)); break;
				case 2: AddActor(new YellowEnemyEntity(this, pos)); break;
				case 3: AddActor(new ArrowEnemyEntity(this, pos)); break;
				case 4: AddActor(new HealerEnemyEntity(this, pos)); break;
				case 5: AddActor(new ArmorEnemyEntity(this, pos)); break;
				case 6: AddActor(new GunnerEnemyEntity(this, pos)); break;
				case 7: AddActor(new YoroiBossEntity(this, pos, Vector2d(192, 192))); break;
				case 8: AddActor(new SekienkiBossEntity(this, pos, Vector2d(192, 192))); break;
				default: break;
				}
			}
		}
	}

	// 背景など（必要に応じて差し替え）
	BackGroundUI* back = new BackGroundUI(this, "assets/images/uies/bg.png");
	AddBackActor(back);

	// カメラ境界を更新
	float mapW = static_cast<float>(stage.width) * m_mapData.tileSize;
	float mapH = static_cast<float>(stage.height) * m_mapData.tileSize;
	m_camera.SetBounds(Vector2d(0, 0), Vector2d(mapW, mapH));

	// プレイヤーを次ステージの左端に移動（Y は現状維持または固定）
	if (m_player) {
		float spawnX = 64.0f; // 左端の何pxに置くか。必要に応じ調整
		Vector2d playerPos = m_player->GetComponent<TransformComponent>()->GetPosition();
		m_player->GetComponent<TransformComponent>()->SetPosition(Vector2d(spawnX, playerPos.y));
	}

	// カメラもプレイヤーに合わせる
	if (m_player) {
		Vector2d playerPos = m_player->GetComponent<TransformComponent>()->GetPosition();
		Vector2d camPos = playerPos;
		camPos.y -= 150;
		m_camera.SetCenter(camPos);
	}
}

void PlayScene::GoToStage(int idx)
{
	if (idx < 0 || idx >= static_cast<int>(m_mapData.stages.size())) {
		std::cerr << "[GoToStage] invalid index: " << idx << std::endl;
		return;
	}
	m_stageIndex = idx;
	BuildStage(m_stageIndex);

	// 遷移ロックを設定して短時間再遷移を防ぐ
	m_stageTransitionLock = true;
	m_stageTransitionTimer = STAGE_TRANSITION_COOLDOWN;
}

void PlayScene::NextStage()
{
	int next = m_stageIndex + 1;
	// ステージ数が 0 のときは何もしない
	if (m_mapData.stages.empty()) return;

	// 最後のステージなら遷移しない（wrap させない）
	if (next >= static_cast<int>(m_mapData.stages.size())) {
		std::cout << "[NextStage] Already at last stage: index=" << m_stageIndex << std::endl;

		// プレイヤーが右に行けないように右端へ押し戻す（オプション）
		StageData& cur = m_mapData.stages[m_stageIndex];
		float mapWidth = static_cast<float>(cur.width) * m_mapData.tileSize;
		const float RIGHT_THRESHOLD = 16.0f;

		if (m_player) {
			auto transform = m_player->GetComponent<TransformComponent>();
			if (transform) {
				Vector2d pos = transform->GetPosition();
				if (pos.x > mapWidth - RIGHT_THRESHOLD) {
					pos.x = mapWidth - RIGHT_THRESHOLD;
					transform->SetPosition(pos);
				}
			}
			auto vel = m_player->GetComponent<VelocityComponent>();
			if (vel) {
				Vector2d v = vel->Get();
				v.x = 0.0f;
				vel->Set(v);
			}
		}

		// 短時間ロック（連打防止）
		m_stageTransitionLock = true;
		m_stageTransitionTimer = STAGE_TRANSITION_COOLDOWN;
		return;
	}

	GoToStage(next);
}

bool PlayScene::ReloadStageFromFolder(int idx, const std::string& stageFolder)
{
	if (idx < 0) return false;
	StageData newStage;
	// MapData::AddLayerFromCSV を使って stage データを作る
	if (!m_mapData.AddLayerFromCSV(newStage, stageFolder + "/map.csv", "Map")) return false;
	if (!m_mapData.AddLayerFromCSV(newStage, stageFolder + "/obj.csv", "Object")) return false;

	// 既存の stages の idx を置き換え（idx が範囲内なら置換、なければ push_back）
	if (idx < static_cast<int>(m_mapData.stages.size())) {
		m_mapData.stages[idx] = std::move(newStage);
	}
	else {
		m_mapData.stages.push_back(std::move(newStage));
		// 必要なら m_stageFolders も更新
		if (idx >= static_cast<int>(m_stageFolders.size())) m_stageFolders.resize(idx + 1);
		m_stageFolders[idx] = stageFolder;
	}
	// 置き換えたステージが現在のステージなら再構築
	if (idx == m_stageIndex) BuildStage(idx);
	return true;
}