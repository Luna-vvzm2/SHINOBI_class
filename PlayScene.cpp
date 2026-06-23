#define NOMINMAX
#include "PlayScene.h"
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

//イベントのため変更
#include "EventManager.h"

#include <algorithm>

PlayScene::PlayScene(Game* game)
	: Scene(game),
	m_player(nullptr),
	m_effect(nullptr),
	m_camera(static_cast<float>(game->GetWidth()), static_cast<float>(game->GetHeight())),
	m_stageIndex(0),
	m_comboCount(0),
	m_currentStage(1),
	m_eventManager(std::make_unique<EventManager>(this)) //イベントのため変更
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

			//イベントのため追加
			default:
				if (objID >= 100)
				{
					AddActor(new EventTrigger(this, pos, Vector2d(tileSize, tileSize), objID, m_eventManager.get()));
				}
				break;
			}
			
		}
	}
	
	m_player = new PlayerEntity(this, Vector2d({200, 800}), Vector2d({192, 64}));
	AddActor(m_player);
	
	// ---- HP UI 作成 ----
	HPBarUI* hpBar = new HPBarUI(
		this,
		m_player->GetHP()
	);
	AddUIActor(hpBar);

	BackGroundUI* back = new BackGroundUI(this, "assets/images/uies/bg.png");
	AddBackActor(back);


	float halfTile = m_mapData.tileSize * 0.5f;
	m_camera.SetTileHalfSize(Vector2d(halfTile, halfTile));

	// Camera のマップ範囲設定
	float mapW = (float)stage.width * m_mapData.tileSize;
	float mapH = (float)stage.height * m_mapData.tileSize;
	m_camera.SetBounds(Vector2d(0, 0), Vector2d(mapW, mapH));

	// Renderer に Camera をセット
	m_game->GetRenderer()->SetCamera(&m_camera);

	/*
	//イベントテスト(シーン開始時にテキストが描画,先頭の行がTitleからの遷移の際のENTER入力でスキップされています)
	std::unique_ptr<TalkEvent> testTalk = std::make_unique<TalkEvent>(this, "assets/events/event_101.txt");
	m_eventManager->Init(std::move(testTalk));
	//テスト2:プレイヤーのすぐ近くにイベントを配置
	AddActor(new EventTrigger(this, Vector2d(400, 800 + 560), Vector2d(104, 104), 101, m_eventManager.get()));
	*/

	return true;
}

void PlayScene::Update(float deltaTime) {
	//イベントのため変更
	if (m_eventManager->IsRunning())
	{
		m_eventManager->Update(deltaTime);
		return;
	}

	updateActors(m_backactors, deltaTime);
	updateActors(m_actors, deltaTime);
	updateActors(m_UIactors, deltaTime);
	RemoveDeadActors(); 
	if (m_player) {
		Vector2d playerPos = m_player->GetComponent<TransformComponent>()->GetPosition();

		// 中間点をカメラ位置に
		Vector2d camPos = playerPos;
		camPos.y -= 150;
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
		m_camera.SetZoom(1.0f);
	}

}

void PlayScene::Draw() {
	Renderer* renderer = m_game->GetRenderer();



	// --- アクター描画 ---
	drawActors(m_backactors);
	drawActors(m_actors);
	drawActors(m_UIactors);

	//イベントのために変更
	if (m_eventManager->IsRunning())
	{
		m_eventManager->Draw();
	}

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
