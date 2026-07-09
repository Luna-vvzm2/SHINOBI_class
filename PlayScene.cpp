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
#include "ScarecrowEnemyEntity.h"
#include "WhiteEnemyEntity.h"
#include "YellowEnemyEntity.h"
#include "ArrowEnemyEntity.h"
#include "HealerEnemyEntity.h"
#include "ArmorEnemyEntity.h"
#include "GunnerEnemyEntity.h"
#include "YoroiBossEntity.h"
#include "SekienkiBossEntity.h"
#include "EffectActor.h"
#include "StageBackActor.h"
#include "StageExitActor.h"
#include "GroundBlock.h"
#include "ClearBlock.h"
#include "HouseBlock.h"
#include "YaguraSBlock.h"
#include "YaguraMBlock.h"
#include "YaguraLBlock.h"
#include "YaguraLLBlock.h"
#include "PlatformBlock.h"
#include "StoneSBlock.h"
#include "StoneLBlock.h"
#include "PlatformOboroSBlock.h"
#include "PlatformOboroMBlock.h"
#include "PlatformOboroLBlock.h"
#include "WoodSBlock.h"
#include "WoodLBlock.h"
#include "ClearPlatformBlock.h"
#include "StructureABlock.h"
#include "StructureBBlock.h"
#include "Trap.h"
#include "HPBarUI.h"
#include "ShurikenUI.h"
#include "BackGroundUI.h"
#include "TreasureBox.h"

#include "Camera.h"
#include "MoneyUI.h"
#include "EnemyHPBar.h"

//イベントのため変更
#include "EventManager.h"
#include "EventTexture.h"

#include <DxLib.h>
#include <algorithm>

PlayScene::SkillData* PlayScene::GetSelectedSkill()
{
	switch (m_skillCursorY)
	{
	case 0:
		return &m_ninjutsu[m_skillCursorX];


	case 1:
		return &m_ninpou[m_skillCursorX];


	case 2:
		return &m_ningi[m_skillCursorX];


	case 3:
	{
		int index = m_skillCursorX;
		return &m_combat[index];
	}

	case 4:
	{
		int index = 9 + m_skillCursorX;
		return &m_combat[index];
	}
	}

	return nullptr;
}

void PlayScene::ClampSkillCursor()
{
	// 全体範囲
	if (m_skillCursorY < 0)
		m_skillCursorY = 0;

	if (m_skillCursorY > 4)
		m_skillCursorY = 4;


	int max = 0;


	switch (m_skillCursorY)
	{
	case 0:
		// 忍術
		max = 4;
		break;


	case 1:
		// 忍法
		max = 8;
		break;


	case 2:
		// 忍技
		max = 7;
		break;


	case 3:
		// 戦闘技 1段目
		max = 9;
		break;


	case 4:
		// 戦闘技 2段目
		max = 8;
		break;
	}



	if (m_skillCursorX < 0)
		m_skillCursorX = 0;


	if (m_skillCursorX >= max)
		m_skillCursorX = max - 1;
}

void PlayScene::DrawSkillRow(
	const std::vector<SkillData>& skills,
	int startX,
	int startY
)
{
	const int size = 60;
	const int gap = 15;

	for (int i = 0; i < skills.size(); i++)
	{
		int x =
			startX + i * (size + gap);

		DrawSkillSlot(
			x,
			startY,
			skills[i]
		);
	}
}

void PlayScene::DrawSkillGrid(
	const std::vector<SkillData>& skills,
	int startX,
	int startY,
	int columns
)
{
	const int size = 60;
	const int gap = 15;

	for (int i = 0; i < skills.size(); i++)
	{
		int x =
			startX + (i % columns) * (size + gap);

		int y =
			startY + (i / columns) * (size + gap);

		DrawSkillSlot(
			x,
			y,
			skills[i]
		);
	}
}

void PlayScene::DrawSkillSlot(
	int x,
	int y,
	const SkillData& skill
) const
{
	const int size = 60;

	int handle =
		skill.unlocked ?
		skill.iconHandle :
		m_lockedSkillIcon;

	if (handle >= 0)
	{
		DrawExtendGraph(
			x,
			y,
			x + size,
			y + size,
			handle,
			TRUE
		);
	}
	else
	{
		DrawBox(
			x,
			y,
			x + size,
			y + size,
			GetColor(255, 255, 255),
			FALSE
		);
	}
}

int PlayScene::GetSkillMaxX(int y)
{
	switch (y)
	{
	case 0:
		return 4; // 忍術

	case 1:
		return 8; // 忍法

	case 2:
		return 7; // 忍技

	case 3:
		return 9; // 戦闘技上段

	case 4:
		return 8; // 戦闘技下段
	}

	return 0;
}

void PlayScene::UpdateSkillMenu(float deltaTime)
{
	const Input& input = m_game->GetInput();


	bool move = false;


	// 押した瞬間
	if (input.IsTrigger(Action::RIGHT))
	{
		m_skillCursorX++;
		move = true;
	}

	if (input.IsTrigger(Action::LEFT))
	{
		m_skillCursorX--;
		move = true;
	}

	if (input.IsTrigger(Action::DOWN))
	{
		m_skillCursorY++;
		move = true;
	}

	if (input.IsTrigger(Action::UP))
	{
		m_skillCursorY--;
		move = true;
	}



	// 長押し中
	if (input.IsDown(Action::RIGHT) ||
		input.IsDown(Action::LEFT) ||
		input.IsDown(Action::DOWN) ||
		input.IsDown(Action::UP))
	{

		m_cursorRepeatTimer += deltaTime;


		if (m_cursorRepeatTimer >= m_cursorRepeatDelay)
		{
			if (m_cursorRepeatTimer >= m_cursorRepeatDelay + m_cursorRepeatInterval)
			{
				m_cursorRepeatTimer -= m_cursorRepeatInterval;


				if (input.IsDown(Action::RIGHT))
					m_skillCursorX++;

				if (input.IsDown(Action::LEFT))
					m_skillCursorX--;

				if (input.IsDown(Action::DOWN))
					m_skillCursorY++;

				if (input.IsDown(Action::UP))
					m_skillCursorY--;
			}
		}
	}
	else
	{
		// 離したらリセット
		m_cursorRepeatTimer = 0.0f;
	}


	ClampSkillCursor();
}

void PlayScene::DrawSkillMenu()
{
	// 背景
	DrawBox(
		0,
		0,
		m_game->GetWidth(),
		m_game->GetHeight(),
		GetColor(20, 20, 20),
		TRUE
	);

	const int startX = 50;
	const int size = 60;
	const int gap = 15;


	// タイトル
	DrawString(
		800,
		50,
		"スキル",
		GetColor(255, 255, 255)
	);


	// カテゴリ名
	DrawString(
		startX,
		120,
		"忍術",
		GetColor(255, 255, 255)
	);


	// 忍術 4個
	DrawSkillRow(
		m_ninjutsu,
		startX,
		160
	);



	// 忍法 8個
	DrawString(
		startX,
		250,
		"忍法",
		GetColor(255, 255, 255)
	);


	DrawSkillRow(
		m_ninpou,
		startX,
		290
	);


	// 忍技 7個
	DrawString(
		startX,
		380,
		"忍技",
		GetColor(255, 255, 255)
	);


	DrawSkillRow(
		m_ningi,
		startX,
		420
	);


	// 戦闘技 17個
	DrawString(
		startX,
		510,
		"戦闘技",
		GetColor(255, 255, 255)
	);


	DrawSkillGrid(
		m_combat,
		startX,
		550,
		9
	);

	// カーソル描画

	int cursorX =
		startX + m_skillCursorX * (size + gap);

	int cursorY = 0;


	switch (m_skillCursorY)
	{
	case 0:
		cursorY = 160;
		break;

	case 1:
		cursorY = 290;
		break;

	case 2:
		cursorY = 420;
		break;

	case 3:
		cursorY = 550;
		break;

	case 4:
		cursorY = 550 + (size + gap);
		break;
	}


	DrawBox(
		cursorX - 5,
		cursorY - 5,
		cursorX + size + 5,
		cursorY + size + 5,
		GetColor(255, 0, 0),
		FALSE
	);

	const int demoX = 800;
	const int demoY = 150;
	const int demoW = 400;
	const int demoH = 250;


	DrawBox(
		demoX,
		demoY,
		demoX + demoW,
		demoY + demoH,
		GetColor(80, 80, 80),
		TRUE
	);

	SkillData* skill = GetSelectedSkill();


	if (skill)
	{
		DrawString(
			975,
			430,
			skill->name.c_str(),
			GetColor(255, 255, 255)
		);


		DrawString(
			975,
			600,
			skill->description.c_str(),
			GetColor(255, 255, 255)
		);
	}
}




PlayScene::PlayScene(Game* game)
	: Scene(game),
	m_player(nullptr),
	m_camera(static_cast<float>(game->GetWidth()), static_cast<float>(game->GetHeight())),
	m_stageIndex(0),
	m_comboCount(0),
	m_currentStage(1),
	m_skillCursorX(0),
	m_skillCursorY(0),
	m_bgHandle(0),
	m_fgHandle(0),
	m_eventTexture(std::make_unique<EventTexture>()),
	m_eventManager(std::make_unique<EventManager>(this, m_eventTexture.get())), //イベントのため変更
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
	//m_lockedSkillIcon = LoadGraph("assets/images/skills/locked.png");

	//スキルデータ
	m_ninjutsu =
	{
		{"忍術1", "技説明用（要修正）", /*LoadGraph("assets/images/skills/fireball.png")*/ -1,true},
		{"忍術2", "test"},
		{"忍術3", "test"},
		{"忍術4", "test"}
	};


	m_ninpou =
	{
		{"忍法1", "test"},
		{"忍法2", "test"},
		{"忍法3", "test"},
		{"忍法4", "test"},
		{"忍法5", "test"},
		{"忍法6", "test"},
		{"忍法7", "test"},
		{"忍法8", "test"}
	};


	m_ningi =
	{
		{"忍技1", "test"},
		{"忍技2", "test"},
		{"忍技3", "test"},
		{"忍技4", "test"},
		{"忍技5", "test"},
		{"忍技6", "test"},
		{"忍技7", "test"}
	};


	m_combat =
	{
		{"戦闘技1", "test"},
		{"戦闘技2", "test"},
		{"戦闘技3", "test"},
		{"戦闘技4", "test"},
		{"戦闘技5", "test"},
		{"戦闘技6", "test"},
		{"戦闘技7", "test"},
		{"戦闘技8", "test"},
		{"戦闘技9", "test"},

		{"戦闘技10", "test"},
		{"戦闘技11", "test"},
		{"戦闘技12", "test"},
		{"戦闘技13", "test"},
		{"戦闘技14", "test"},
		{"戦闘技15", "test"},
		{"戦闘技16", "test"},
		{"戦闘技17", "test"}
	};

	
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

	StageInit(m_stageIndex);

	m_player = new PlayerEntity(this, m_playerSpawnPoints[0], Vector2d({ 152, 64 }));
	AddActor(m_player);

	// ---- HP UI 作成 ----
	HPBarUI* hpBar = new HPBarUI(
		this,
		m_player->GetHP()
	);
	AddUIActor(hpBar);

	
	ShurikenUI* shuriken = new ShurikenUI(this, 18, 60);
	AddUIActor(shuriken);

	// プレイヤー所持金 UI（左上に表示）
	m_moneyUI = new MoneyUI(this, m_player, "assets/images/uies/money.png");
	m_moneyUI->SetPosition(20.0f, 110.0f);   // テキスト左上基準（スクリーン座標）
	m_moneyUI->SetImageSize(40.0f, 40.0f);   // 画像を 40x40 px に
	m_moneyUI->SetImageOffset(0.0f, 0.0f);   // 画像の相対オフセット（必要なら微調整）
	m_moneyUI->SetTextOffset(46.0f);         // 画像右側に数字を表示する距離
	m_moneyUI->SetAnchorTopRight(220.0f, 50.0f, -80.0f);// 右上に固定：右端から220px, 上から50px, 画像と数字の間隔を-60px にする
	AddUIActor(m_moneyUI);

	// プレイヤー金額変更時に MoneyUI を 3 秒表示（増えたときのみ）
	if (m_player) {
		m_player->OnMoneyChanged = [this](int newMoney, int oldMoney) {
			if (newMoney > oldMoney && m_moneyUI) {
				m_moneyUI->ShowFor(3.0f); // 3秒表示
			}
			};
	}

	BackGroundUI* back = new BackGroundUI(this, "assets/images/uies/bg1.png");
	AddBackActor(back);

	EffectActor::LoadEffects();

	// Renderer に Camera をセット
	m_game->GetRenderer()->SetCamera(&m_camera);

	return true;
}

bool PlayScene::StageInit(int stageNo) {
	StageData& stage = m_mapData.stages[stageNo];
	if (m_player != nullptr) m_player->ResetStageState();

	// マップのタイルを配置する
	const float tileSize = static_cast<float>(m_mapData.tileSize);
	const Layer& mapLayer = stage.layers[0];

	for (int y = 0; y < stage.height; ++y) {
		for (int x = 0; x < stage.width; ++x) {
			int tileID = mapLayer.tiles[static_cast<std::vector<int, std::allocator<int>>::size_type>(y) * stage.width + x];
			if (tileID == 0) continue; // 空タイル

			Vector2d pos(x * tileSize, y * tileSize);

			// ここでタイルIDに応じてブロック生成
			// 例：GrassBlock と IceBlock を仮に切り替え
			switch (tileID) {
			case 1:
				AddActor(new GroundBlock(this, pos, Vector2d(tileSize, tileSize)));
				break;
			case 2:
			 	AddActor(new ClearBlock(this, pos, Vector2d(tileSize, tileSize)));
				break;
			case 5:
				pos = Vector2d(x * tileSize - tileSize * 0.5f, y * tileSize);
				AddActor(new HouseBlock(this, pos, Vector2d(tileSize * 4.0f, tileSize)));
				break;
			case 8:
				AddActor(new YaguraSBlock(this, pos, Vector2d(tileSize, tileSize)));
				break;
			case 9:
				pos = Vector2d(x * tileSize + tileSize * 0.5f, y * tileSize - tileSize * 0.5f);
				AddActor(new YaguraMBlock(this, pos, Vector2d(tileSize * 2.0f, tileSize * 2.0f)));
				break;
			case 10:
				pos = Vector2d(x * tileSize + tileSize * 0.5f, y * tileSize - tileSize * 1.0f);
				AddActor(new YaguraLBlock(this, pos, Vector2d(tileSize * 2.0f, tileSize * 3.0f)));
				break;
			case 11:
				pos = Vector2d(x * tileSize + tileSize * 0.5f, y * tileSize - tileSize * 1.2f);
				AddActor(new YaguraLLBlock(this, pos, Vector2d(tileSize * 2.0f, tileSize * 3.25f)));
				break;
			case 14:
				pos = Vector2d(x * tileSize + tileSize * 0.5f, y * tileSize - tileSize * 0.5f);
				AddActor(new PlatformBlock(this, pos));
				break;
			case 18:
				pos = Vector2d(x * tileSize - tileSize * 0.5f, y * tileSize);
				AddActor(new StoneSBlock(this, pos, Vector2d(tileSize * 2.0f, tileSize)));
				break;
			case 19:
				pos = Vector2d(x * tileSize - tileSize * 0.5f, y * tileSize);
				AddActor(new StoneLBlock(this, pos, Vector2d(tileSize * 2.0f, tileSize * 3.75f)));
				break;
			case 24:
				pos = Vector2d(x * tileSize + tileSize * 3.5f, y * tileSize - tileSize * 0.5f);
				AddActor(new PlatformOboroLBlock(this, pos, Vector2d(tileSize * 8.0f, tileSize * 0.20f)));
				break;
			case 27:
				pos = Vector2d(x * tileSize, y * tileSize - tileSize * 0.5f);
				AddActor(new PlatformOboroSBlock(this, pos, Vector2d(tileSize, tileSize * 0.20f)));
				pos = Vector2d(x * tileSize + tileSize, y * tileSize - tileSize * 0.5f);
				AddActor(new PlatformOboroSBlock(this, pos, Vector2d(tileSize, tileSize * 0.20f)));
				break;
			case 28:
				pos = Vector2d(x * tileSize, y * tileSize - tileSize * 0.5f);
				AddActor(new PlatformOboroSBlock(this, pos, Vector2d(tileSize, tileSize * 0.20f)));
				pos = Vector2d(x * tileSize + tileSize, y * tileSize - tileSize * 0.5f);
				AddActor(new PlatformOboroSBlock(this, pos, Vector2d(tileSize, tileSize * 0.20f)));
				break;
			case 29:
				pos = Vector2d(x * tileSize + tileSize, y * tileSize - tileSize * 0.5f);
				AddActor(new PlatformOboroMBlock(this, pos, Vector2d(tileSize * 3.0f, tileSize * 0.20f)));
				break;
			case 31:
				AddActor(new WoodSBlock(this, pos, Vector2d(tileSize, tileSize)));
				break;
			case 32:
				pos = Vector2d(x * tileSize + tileSize * 0.5f, y * tileSize);
				AddActor(new WoodLBlock(this, pos, Vector2d(tileSize * 2.0f, tileSize)));
				break;
			case 38:
				pos = Vector2d(x * tileSize + tileSize * 1.0f, y * tileSize - tileSize * 1.5f);
				AddActor(new StructureABlock(this, pos, Vector2d(tileSize * 5.0f, tileSize * 4.0f)));
				pos = Vector2d(x * tileSize + tileSize * 1.0f, y * tileSize - tileSize * 4.5f);
				AddActor(new ClearBlock(this, pos, Vector2d(tileSize * 5.0f, tileSize * 2.0f)));
				break;
			case 39:
				pos = Vector2d(x * tileSize + tileSize * 1.0f, y * tileSize - tileSize * 1.5f);
				AddActor(new StructureBBlock(this, pos, Vector2d(tileSize * 5.0f, tileSize * 4.0f)));
				pos = Vector2d(x * tileSize + tileSize * 1.0f, y * tileSize - tileSize * 4.5f);
				AddActor(new ClearBlock(this, pos, Vector2d(tileSize * 5.0f, tileSize * 2.0f)));
				break;
			case 40:
				pos = Vector2d(x * tileSize + tileSize, y * tileSize - tileSize * 0.5f);
				AddActor(new PlatformOboroMBlock(this, pos, Vector2d(tileSize * 3.0f, tileSize * 0.20f)));
				break;
			case 41:
				pos = Vector2d(x * tileSize - tileSize * 0.5f, y * tileSize);
				AddActor(new ClearPlatformBlock(this, pos, Vector2d(tileSize * 2.0f, tileSize * 0.25f)));
				break;
			case 42:
				pos = Vector2d(x * tileSize - tileSize, y * tileSize - tileSize * 0.5f);
				AddActor(new ClearBlock(this, pos, Vector2d(tileSize, tileSize * 2.0f)));
				break;
			case 44:
				pos = Vector2d(x * tileSize, y * tileSize + tileSize * 0.625f);
				AddActor(new ClearPlatformBlock(this, pos, Vector2d(tileSize * 3.0f, tileSize * 0.25f)));
				break;
			}
		}
	}

	const Layer& objLayer = stage.layers[1];

	m_playerSpawnPoints.clear();

	for (int y = 0; y < stage.height; y++)
	{
		for (int x = 0; x < stage.width; x++)
		{
			int objID =
				objLayer.tiles[static_cast<std::vector<int, std::allocator<int>>::size_type>(y) * stage.width + x];


			Vector2d pos(x * tileSize, y * tileSize);

			switch (objID)
			{
			case 1:
			case 3:
				m_playerSpawnPoints.push_back(pos);
				break;

			case 2:
			{
				pos += Vector2d(0.0f, -tileSize * 2.0f);
				AddActor(new StageExitActor(this, pos, m_stageIndex + 1));
			} break;

			case 4:
			{
				pos += Vector2d(-tileSize * 0.5f, 20.0f);
				AddActor(new TreasureBoxEntity(this, pos));
				break;
			}

			case 9:
				AddActor(new Trap(this, pos, Vector2d(192, 192)));
				break;

			case 14:
			{
				pos += Vector2d(0.0f, -tileSize * 2.0f);
				switch (m_stageIndex) {
				case 1:
					AddActor(new StageBackActor(this, pos, m_stageIndex - 1, 1));
					break;
				case 2:
					AddActor(new StageBackActor(this, pos, m_stageIndex - 1, 4));
					break;
				}
			}
			break;

			case 201:
			{
				AddActor(new ScarecrowEnemyEntity(this, pos));
			}
			break;

			case 202:
			{
				WhiteEnemyEntity* enemy = new WhiteEnemyEntity(this, pos);
				AddActor(enemy);

				// HPBar を PlayScene 側で作成して登録（UI は AddUIActor で登録）
				EnemyHPBar* hpBar = new EnemyHPBar(this, enemy->GetHP(), "assets/images/uies/HP_enemy_black.png");
				hpBar->SetPosIsCenter(false);          // transform は左上座標を使う（デフォルト）
				hpBar->SetFrameOffset(80.0f, 50.0f);   // 少し上に出す
				hpBar->SetGaugeScale(0.8f, 0.05f);     // 幅を小さめ, 高さ半分
				hpBar->SetPadding(3.0f, 1.0f, 3.0f, 1.0f);
				hpBar->SetGaugeOffset(10.0f, 0.0f);// 枠はそのまま、ゲージを右に +3px、下に +1px 移動
				hpBar->SetGaugeOffset(0.0f, 0.0f);// ゲージを枠の中心より少し上に表示（上にずらすなら負の値）
				AddUIActor(hpBar);
				m_enemyToHPBarMap[enemy] = hpBar;

				// ダメージで表示：HP が減ったとき表示する
				enemy->GetHP()->OnHPChanged = [hpBar](int newHP, int oldHP) {
					if (hpBar && newHP < oldHP) {
						hpBar->ShowFor(0.0f); // 表示継続時間は調整可
					}
					};
				// 敵の死亡時にバーを消す（簡易）
				enemy->GetHP()->OnDeath = [hpBar]() {
					if (hpBar) hpBar->SetState(Actor::State::Dead);
					};
			} break;

			case 203:
			{
				YellowEnemyEntity* enemy = new YellowEnemyEntity(this, pos);
				AddActor(enemy);
				EnemyHPBar* hpBar = new EnemyHPBar(this, enemy->GetHP(), "assets/images/uies/HP_enemy_black.png");
				hpBar->SetPosIsCenter(false);          // transform は左上座標を使う（デフォルト）
				hpBar->SetFrameOffset(80.0f, 50.0f);   // 少し上に出す
				hpBar->SetGaugeScale(0.8f, 0.05f);     // 幅を小さめ, 高さ半分
				hpBar->SetPadding(3.0f, 1.0f, 3.0f, 1.0f);
				hpBar->SetGaugeOffset(10.0f, 0.0f);// 枠はそのまま、ゲージを右に +3px、下に +1px 移動
				hpBar->SetGaugeOffset(0.0f, 0.0f);// ゲージを枠の中心より少し上に表示（上にずらすなら負の値）
				AddUIActor(hpBar);
				m_enemyToHPBarMap[enemy] = hpBar;

				// ダメージで表示：HP が減ったときだけ表示する（2秒）
				enemy->GetHP()->OnHPChanged = [hpBar](int newHP, int oldHP) {
					if (hpBar && newHP < oldHP) {
						hpBar->ShowFor(0.0f); // 表示継続時間は調整可
					}
					};
				// 敵の死亡時にバーを消す（簡易）
				enemy->GetHP()->OnDeath = [hpBar]() {
					if (hpBar) hpBar->SetState(Actor::State::Dead);
					};
			} break;

			case 204:
			{
				ArrowEnemyEntity* enemy = new ArrowEnemyEntity(this, pos);
				AddActor(enemy);
				EnemyHPBar* hpBar = new EnemyHPBar(this, enemy->GetHP(), "assets/images/uies/HP_enemy_black.png");
				hpBar->SetPosIsCenter(false);          // transform は左上座標を使う（デフォルト）
				hpBar->SetFrameOffset(80.0f, 50.0f);   // 少し上に出す
				hpBar->SetGaugeScale(0.8f, 0.05f);     // 幅を小さめ, 高さ半分
				hpBar->SetPadding(3.0f, 1.0f, 3.0f, 1.0f);
				hpBar->SetGaugeOffset(10.0f, 0.0f);// 枠はそのまま、ゲージを右に +3px、下に +1px 移動
				hpBar->SetGaugeOffset(0.0f, 0.0f);// ゲージを枠の中心より少し上に表示（上にずらすなら負の値）
				AddUIActor(hpBar);
				m_enemyToHPBarMap[enemy] = hpBar;

				// ダメージで表示：HP が減ったときだけ表示する（2秒）
				enemy->GetHP()->OnHPChanged = [hpBar](int newHP, int oldHP) {
					if (hpBar && newHP < oldHP) {
						hpBar->ShowFor(0.0f); // 表示継続時間は調整可
					}
					};
				// 敵の死亡時にバーを消す（簡易）
				enemy->GetHP()->OnDeath = [hpBar]() {
					if (hpBar) hpBar->SetState(Actor::State::Dead);
					};
			} break;

			case 208:
			{
				HealerEnemyEntity* enemy = new HealerEnemyEntity(this, pos);
				AddActor(enemy);
				EnemyHPBar* hpBar = new EnemyHPBar(this, enemy->GetHP(), "assets/images/uies/HP_enemy_black.png");
				hpBar->SetPosIsCenter(false);          // transform は左上座標を使う（デフォルト）
				hpBar->SetFrameOffset(80.0f, 50.0f);   // 少し上に出す
				hpBar->SetGaugeScale(0.8f, 0.05f);     // 幅を小さめ, 高さ半分
				hpBar->SetPadding(3.0f, 1.0f, 3.0f, 1.0f);
				hpBar->SetGaugeOffset(10.0f, 0.0f);// 枠はそのまま、ゲージを右に +3px、下に +1px 移動
				hpBar->SetGaugeOffset(0.0f, 0.0f);// ゲージを枠の中心より少し上に表示（上にずらすなら負の値）
				AddUIActor(hpBar);
				m_enemyToHPBarMap[enemy] = hpBar;

				// ダメージで表示：HP が減ったときだけ表示する（2秒）
				enemy->GetHP()->OnHPChanged = [hpBar](int newHP, int oldHP) {
					if (hpBar && newHP < oldHP) {
						hpBar->ShowFor(0.0f); // 表示継続時間は調整可
					}
					};
				// 敵の死亡時にバーを消す（簡易）
				enemy->GetHP()->OnDeath = [hpBar]() {
					if (hpBar) hpBar->SetState(Actor::State::Dead);
					};
			} break;

			case 206:
			{
				ArmorEnemyEntity* enemy = new ArmorEnemyEntity(this, pos);
				AddActor(enemy);
				EnemyHPBar* hpBar = new EnemyHPBar(this, enemy->GetHP(), "assets/images/uies/HP_enemy_armor.png");
				hpBar->SetPosIsCenter(false);          // transform は左上座標を使う（デフォルト）
				hpBar->SetFrameOffset(80.0f, 50.0f);   // 少し上に出す
				hpBar->SetGaugeScale(0.8f, 0.05f);     // 幅を小さめ, 高さ半分
				hpBar->SetPadding(3.0f, 1.0f, 3.0f, 1.0f);
				hpBar->SetGaugeOffset(10.0f, 0.0f);// 枠はそのまま、ゲージを右に +3px、下に +1px 移動
				hpBar->SetGaugeOffset(0.0f, 0.0f);// ゲージを枠の中心より少し上に表示（上にずらすなら負の値）
				AddUIActor(hpBar);
				m_enemyToHPBarMap[enemy] = hpBar;

				// ダメージで表示：HP が減ったときだけ表示する（2秒）
				enemy->GetHP()->OnHPChanged = [hpBar](int newHP, int oldHP) {
					if (hpBar && newHP < oldHP) {
						hpBar->ShowFor(0.0f); // 表示継続時間は調整可
					}
					};
				// 敵の死亡時にバーを消す（簡易）
				enemy->GetHP()->OnDeath = [hpBar]() {
					if (hpBar) hpBar->SetState(Actor::State::Dead);
					};
			} break;

			case 207:
			{
				GunnerEnemyEntity* enemy = new GunnerEnemyEntity(this, pos);
				AddActor(enemy);
				EnemyHPBar* hpBar = new EnemyHPBar(this, enemy->GetHP(), "assets/images/uies/HP_enemy_black.png");
				hpBar->SetPosIsCenter(false);          // transform は左上座標を使う（デフォルト）
				hpBar->SetFrameOffset(80.0f, 50.0f);   // 少し上に出す
				hpBar->SetGaugeScale(0.8f, 0.05f);     // 幅を小さめ, 高さ半分
				hpBar->SetPadding(3.0f, 1.0f, 3.0f, 1.0f);
				hpBar->SetGaugeOffset(10.0f, 0.0f);// 枠はそのまま、ゲージを右に +3px、下に +1px 移動
				hpBar->SetGaugeOffset(0.0f, 0.0f);// ゲージを枠の中心より少し上に表示（上にずらすなら負の値）
				AddUIActor(hpBar);
				m_enemyToHPBarMap[enemy] = hpBar;

				// ダメージで表示：HP が減ったときだけ表示する（2秒）
				enemy->GetHP()->OnHPChanged = [hpBar](int newHP, int oldHP) {
					if (hpBar && newHP < oldHP) {
						hpBar->ShowFor(0.0f); // 表示継続時間は調整可
					}
					};
				// 敵の死亡時にバーを消す（簡易）
				enemy->GetHP()->OnDeath = [hpBar]() {
					if (hpBar) hpBar->SetState(Actor::State::Dead);
					};
			} break;

			case 205:
			{
				YoroiBossEntity* enemy = new YoroiBossEntity(this, pos, Vector2d(192, 192));
				AddActor(enemy);
				EnemyHPBar* hpBar = new EnemyHPBar(this, enemy->GetHP(), "assets/images/uies/HP_enemy_black.png");
				hpBar->SetPosIsCenter(false);          // transform は左上座標を使う（デフォルト）
				hpBar->SetFrameOffset(80.0f, 50.0f);   // 少し上に出す
				hpBar->SetGaugeScale(0.8f, 0.05f);     // 幅を小さめ, 高さ半分
				hpBar->SetPadding(3.0f, 1.0f, 3.0f, 1.0f);
				hpBar->SetGaugeOffset(10.0f, 0.0f);// 枠はそのまま、ゲージを右に +3px、下に +1px 移動
				hpBar->SetGaugeOffset(0.0f, 0.0f);// ゲージを枠の中心より少し上に表示（上にずらすなら負の値）
				AddUIActor(hpBar);
				m_enemyToHPBarMap[enemy] = hpBar;

				// ダメージで表示：HP が減ったときだけ表示する（2秒）
				enemy->GetHP()->OnHPChanged = [hpBar](int newHP, int oldHP) {
					if (hpBar && newHP < oldHP) {
						hpBar->ShowFor(0.0f); // 表示継続時間は調整可
					}
					};
				// 敵の死亡時にバーを消す（簡易）
				enemy->GetHP()->OnDeath = [hpBar]() {
					if (hpBar) hpBar->SetState(Actor::State::Dead);
					};
			} break;

			case 210:
			{
				SekienkiBossEntity* enemy = new SekienkiBossEntity(this, pos, Vector2d(192, 192));
				AddActor(enemy);
				EnemyHPBar* hpBar = new EnemyHPBar(this, enemy->GetHP(), "assets/images/uies/HP_enemy_black.png");
				hpBar->SetPosIsCenter(false);          // transform は左上座標を使う（デフォルト）
				hpBar->SetFrameOffset(80.0f, 50.0f);   // 少し上に出す
				hpBar->SetGaugeScale(0.8f, 0.05f);     // 幅を小さめ, 高さ半分
				hpBar->SetPadding(3.0f, 1.0f, 3.0f, 1.0f);
				hpBar->SetGaugeOffset(10.0f, 0.0f);// 枠はそのまま、ゲージを右に +3px、下に +1px 移動
				hpBar->SetGaugeOffset(0.0f, 0.0f);// ゲージを枠の中心より少し上に表示（上にずらすなら負の値）
				AddUIActor(hpBar);
				m_enemyToHPBarMap[enemy] = hpBar;

				// ダメージで表示：HP が減ったときだけ表示する（2秒）
				enemy->GetHP()->OnHPChanged = [hpBar](int newHP, int oldHP) {
					if (hpBar && newHP < oldHP) {
						hpBar->ShowFor(0.0f); // 表示継続時間は調整可
					}
					};
				// 敵の死亡時にバーを消す（簡易）
				enemy->GetHP()->OnDeath = [hpBar]() {
					if (hpBar) hpBar->SetState(Actor::State::Dead);
					};
			} break;

			default:
				if (objID >= 100)
				{
					AddActor(new EventTrigger(this, pos, Vector2d(tileSize, tileSize), objID, m_eventManager.get()));
				}
				break;
			} // switch
		} // for x
	} // for y

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

	switch (m_stageIndex) {
	case 0:
		m_bgHandle = LoadGraph("assets/images/uies/bg1.png");
		m_fgHandle = LoadGraph("assets/images/uies/fg1.png");
		break;
	case 1:
		m_bgHandle = LoadGraph("assets/images/uies/bg2.png");
		m_fgHandle = LoadGraph("assets/images/uies/fg2.png");
		break;
	case 2:
		m_bgHandle = LoadGraph("assets/images/uies/bg3.png");
		m_fgHandle = LoadGraph("assets/images/uies/fg3.png");
		break;
	}

	return true;
}

void PlayScene::ChangeStage(int index, int spawnIndex) {
	DeleteGraph(m_bgHandle);
	DeleteGraph(m_fgHandle);
	m_stageIndex = index;

	ClearStageActors();
	StageInit(index);

	auto it = std::find(m_actors.begin(), m_actors.end(), m_player);
	if (it != m_actors.end())
	{
		m_actors.erase(it);
		m_actors.push_back(m_player);
	}

	if (spawnIndex >= 0 &&
		spawnIndex < static_cast<int>(m_playerSpawnPoints.size()))
	{
		m_player->SetPosition(m_playerSpawnPoints[spawnIndex]);
		if (m_player)
		{
			Vector2d playerPos = m_player->GetPos();

			Vector2d camPos = playerPos;
			camPos.y -= 150;

			m_camera.SetCenter(camPos);
		}
	}
}

void PlayScene::ClearStageActors()
{
	for (Actor* actor : m_actors)
	{
		switch (actor->GetType())
		{
		case ActorType::Block:
		case ActorType::Enemy:
		case ActorType::Effect:
		case ActorType::StageExit:
		case ActorType::StageBack:
		case ActorType::TreasureBox:
			actor->SetState(Actor::State::Dead);
			break;

		default:
			break;
		}
	}

	// 敵HPバーも消す
	for (auto& pair : m_enemyToHPBarMap)
	{
		if (pair.second)
			pair.second->SetState(Actor::State::Dead);
	}

	m_enemyToHPBarMap.clear();
//#ifdef _DEBUG
//	for (Actor* actor : m_actors)
//	{
//		if (actor->GetType() == ActorType::Block)
//			continue;
//
//		printf("%s\n", typeid(*actor).name());
//	}
//#endif
	m_metsuEnemies.clear();
}

void PlayScene::RequestStageChange(int stage, int spawnIndex)
{
	if (m_requestStageChange || m_fadeState != FadeState::None)
		return;
	m_requestStageChange = true;
	m_nextStage = stage;
	m_nextSpawnIndex = spawnIndex;
	printf("Request Stage %d  fade=%d\n",
		stage,
		(int)m_fadeState);
}

void PlayScene::Update(float deltaTime) {
	// ====== フェード遷移中はゲームロジックを止める ======
	if (m_fadeState != FadeState::None) {
		UpdateFade(deltaTime);
		// フェード中も UI（ゲームオーバーメニュー等）は動かしたいならここで
		updateActors(m_UIactors, deltaTime);
		return; // 通常更新はスキップ
	}

	if (m_requestStageChange)
	{
		m_requestStageChange = false;
		StartFadeToStage(m_nextStage, m_nextSpawnIndex);
	}

	//イベントのため変更
	if (m_eventManager->IsRunning())
	{
		m_eventManager->Update(deltaTime);

		if (!m_eventManager->IsBattleEvent())
		{
			return;
		}
	}
	updateActors(m_backactors, deltaTime);


	const Input& input = m_game->GetInput();

	if (input.IsTrigger(Action::MENU))
	{
		if (m_menuState == MenuState::None)
		{
			m_menuState = MenuState::Skill;
		}
		else
		{
			m_menuState = MenuState::None;
		}
	}

	if (m_menuState == MenuState::Skill)
	{
		UpdateSkillMenu(deltaTime);
		return;
	}


	if (m_menuState != MenuState::None)
	{
		return;
	}


	if (m_menuState == MenuState::Equipment)
	{
		return;
	}

	updateActors(m_backactors, deltaTime);
	updateActors(m_actors, deltaTime);
	updateActors(m_UIactors, deltaTime);

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
		camPos.y -= 150;
		m_camera.SetCenter(camPos);
		m_camera.SetZoom(1.0f);
		/*
		// 線形補間で徐々にズーム変更
		float currentZoom = m_camera.GetZoom();
		float zoomSpeed = 5.0f;
		float newZoom = currentZoom + (targetZoom - currentZoom) * std::min(zoomSpeed * deltaTime, 1.0f);
		*/

		float fixedCameraY = 400.0f;  // スクロール開始位置の上限

		if (playerPos.y < fixedCameraY) {
			camPos.y = playerPos.y - 200;  // 上に移動したらカメラもスクロール
		}
		else {
			camPos.y = fixedCameraY - 200;  // それ以外はカメラ固定
		}
	}

	// 敵ごとのHPバー追従：World -> Screen using m_camera (PlayScene の m_camera)
	const Vector2d baroffset(-90.0f, -250.0f); // 敵の頭上に表示したければ負の Y オフセット。要調整。
	for (auto it = m_enemyToHPBarMap.begin(); it != m_enemyToHPBarMap.end(); ) {
		EnemyEntity* enemy = it->first;
		EnemyHPBar* hpBar = it->second;

		bool enemyStillPresent = std::find(m_actors.begin(), m_actors.end(), enemy) != m_actors.end();
		bool hpBarStillPresent = std::find(m_UIactors.begin(), m_UIactors.end(), hpBar) != m_UIactors.end();

		if (!enemyStillPresent || !hpBarStillPresent) {
			// どちらか存在しなければマップから除去（まれに両方既に削除済みの場合もある）
			it = m_enemyToHPBarMap.erase(it);
			continue;
		}

		// 敵が存在しない、または死亡していれば UI を消して map から削除
		if (!enemy || enemy->IsDead() || enemy->GetComponent<HPComponent>() == nullptr) {
			if (hpBar) { hpBar->SetState(Actor::State::Dead); }
			it = m_enemyToHPBarMap.erase(it);
			continue;
		}

		// 敵位置取得
		if (hpBar && hpBar->GetState() != Actor::State::Dead) {
			hpBar->SetMetsuValue(enemy->GetMetsuGauge(), enemy->GetMetsuMax());

			auto transform = enemy->GetComponent<TransformComponent>();
			if (transform) {
				Vector2d worldPos = transform->GetPosition() + baroffset;
				// PlayScene のカメラでワールド→スクリーン
				Vector2d screenPos = m_camera.WorldToScreen(worldPos);

				const float barWidth = 63.0f; // SetBarSize と合わせる
				hpBar->SetPosition(screenPos.x - barWidth * 0.5f, screenPos.y);

			}
		}

		++it;
	}
	RemoveDeadActors();

	if (m_shurikenUI && m_player)
	{
		m_shurikenUI->SetCount(m_player->GetShurikenCount());
	}

}
		
void PlayScene::Draw() {
	Renderer* renderer = m_game->GetRenderer();
	if (!renderer) return;
	Vector2d cam = m_camera.GetCenter();

	// 仮背景
	DrawBox(0, 0, 1280, 720, GetColor(200, 200, 200), 1);

	// 背景
	switch (m_stageIndex) {
	case 0:
		renderer->DrawSpriteEx(Vector2d(-350.0f + (cam.x * 0.5f), m_mapData.stages[m_stageIndex].height * m_mapData.tileSize - 1130.0f), 1.6f, 1.6f, 0.0f, m_bgHandle, true, Vector2d(0, 0), 255, false, false, true);
		break;
	case 1:
		renderer->DrawSpriteEx(Vector2d(-350.0f + (cam.x * 0.5f), m_mapData.stages[m_stageIndex].height * m_mapData.tileSize - 1760.0f), 1.45f, 1.45f, 0.0f, m_bgHandle, true, Vector2d(0, 0), 255, false, false, true);
		break;
	case 2:
		renderer->DrawSpriteEx(Vector2d(-350.0f + (cam.x * 0.5f), m_mapData.stages[m_stageIndex].height * m_mapData.tileSize - 1600.0f), 1.5f, 1.5f, 0.0f, m_bgHandle, true, Vector2d(0, 0), 255, false, false, true);
		break;
	}
	
	drawActors(m_backactors);
	drawActors(m_actors);

	// 前景
	switch (m_stageIndex) {
	case 0:
		for (int i = 0; i < 19; i++) {
			renderer->DrawSpriteEx(Vector2d(-1290.0f + i * 1600.0f - (cam.x * 0.5f), m_mapData.stages[m_stageIndex].height * m_mapData.tileSize - 680.0f), 0.8f, 0.8f, 0.0f, m_fgHandle, true, Vector2d(0, 0), 255, false, false, true);
		}
		break;
	case 1:
		renderer->DrawSpriteEx(Vector2d(0 - (cam.x * 0.5f), m_mapData.stages[m_stageIndex].height * m_mapData.tileSize - 4960.0f), 4.3f, 4.3f, 0.0f, m_fgHandle, true, Vector2d(0, 0), 255, false, false, true);
		break;
	case 2:
		for (int i = 0; i < 19; i++) {
			renderer->DrawSpriteEx(Vector2d(-1290.0f + i * 1600.0f - (cam.x * 0.5f), m_mapData.stages[m_stageIndex].height * m_mapData.tileSize - 870.0f), 0.8f, 0.8f, 0.0f, m_fgHandle, true, Vector2d(0, 0), 255, false, false, true);
		}
		break;
	}
	

	drawActors(m_UIactors);

	//イベントのために変更
	if (m_eventManager->IsRunning())
	{
		m_eventManager->Draw();
	}

	if (m_menuState == MenuState::Skill)
	{
		DrawSkillMenu();
	}


	if (m_menuState == MenuState::Equipment)
	{
		//DrawEquipmentMenu();
	}


	if (m_menuState == MenuState::Item)
	{
		//DrawItemMenu();
	}


	// 攻撃範囲描画 --------------------------
	/*Vector2d Pos = m_player->GetPos();
	AttackHitbox Weak1{ Vector2d(50,100), 100, 100, 30 };
	if (m_player->GetDir()) {
		Pos.x += Weak1.offset.x;
		Pos.y += Weak1.offset.y;
	} 
	else{
		Pos.x -= Weak1.offset.x;
		Pos.y += Weak1.offset.y;
	}
	renderer->DrawRectCenter(Pos, Weak1.width, Weak1.height, GetColor(0,255,0),false, true);*/
	//------------------------------------------
#ifdef _DEBUG
	// sensor描画
	Vector2d Pos = m_player->GetPos();
	Vector2d offset = { 0.0f, 50.0f };
	if (m_player->GetDir()) {
		Pos.x += offset.x;
		Pos.y += offset.y;
	}
	else {
		Pos.x -= offset.x;
		Pos.y += offset.y;
	}
	renderer->DrawRectCenter(Pos, 4.0f, 4.0f, GetColor(0, 255, 0), false, true);
#endif
	std::vector<NumberInfo> comboInfo = {
		{ (float)m_player->GetCombo(), 0 }
	};

	// コンボ表示（m_comboCount が 1 以上なら表示）
	if (m_player->GetCombo() > 0) {
		const std::string& debugFont = m_game->GatDebugFont();
		// ここではフォントサイズを大きめ（例 48）で真ん中上に表示
		std::string comboText = std::to_string(m_player->GetCombo()) + " Hits";
		renderer->DrawTextL(Vector2d(20.0f, 120.0f), comboText, Color(0, 0, 0), debugFont, 60, false);
	}

	if (m_resultShown) {
		const std::string& debugFont = m_game->GatDebugFont();

		renderer->DrawNumberFormatW(
			Vector2d(m_game->GetWidth() / 2.4f,
				m_game->GetHeight() / 2.2f),
			Color(0, 0, 0),
			debugFont,
			32,
			"{0} Hits",
			comboInfo,
			false
		);
	}

	// ←ここを追加
	DrawFadeOverlay();

#ifdef _DEBUG
	const std::string& debugFont = m_game->GatDebugFont();
	renderer->DrawTextL(
		Vector2d(m_game->GetWidth() - 150.0f, 0),
		"PlayScene",
		Color(255, 64, 0),
		debugFont,
		24,
		false
	);
#endif
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

//void PlayScene::SpawnHitEffect(const Vector2d& pos) {
//	m_effect = new HitEffect(this, pos, {32, 32});
//	AddActor(m_effect);
//	std::cout << "Spawned HitEffect at: " << pos.x << ", " << pos.y << std::endl;
//	
//}

void PlayScene::AddMetsuEnemy(EnemyEntity* enemy)
{
	if (enemy == nullptr) return;

	auto it = std::find(m_metsuEnemies.begin(), m_metsuEnemies.end(), enemy);
	if (it == m_metsuEnemies.end())
	{
		m_metsuEnemies.push_back(enemy);
	}
	printf("滅敵数: %zd\n", m_metsuEnemies.size());
}

void PlayScene::RemoveMetsuEnemy(EnemyEntity* enemy)
{
	auto it = std::remove(m_metsuEnemies.begin(), m_metsuEnemies.end(), enemy);
	m_metsuEnemies.erase(it, m_metsuEnemies.end());
}

// ====================================================
// フェード遷移
// ====================================================

void PlayScene::StartFadeToStage(int idx, int spawnIndex)
{
	// すでに遷移中なら無視
	if (m_fadeState != FadeState::None) return;

	m_pendingStageIndex = idx;
	m_nextSpawnIndex = spawnIndex;
	m_fadeState = FadeState::FadeOut;
	m_fadeTimer = 0.0f;

	// プレイヤーの動きを止める（暗転中は動かないほうが自然）
	if (m_player) {
		if (auto vel = m_player->GetComponent<VelocityComponent>()) {
			Vector2d v = vel->Get();
			v.x = 0.0f;
			vel->Set(v);
		}
	}
}

void PlayScene::UpdateFade(float deltaTime)
{
	m_fadeTimer += deltaTime;

	switch (m_fadeState)
	{
	case FadeState::FadeOut:
		// 徐々に真っ黒に
		if (m_fadeTimer >= FADE_OUT_DURATION) {
			// 真っ黒になった瞬間にステージを構築（見えないので違和感なし）
			m_stageIndex = m_pendingStageIndex;
			ChangeStage(m_nextStage, m_nextSpawnIndex);
			m_pendingStageIndex = -1;
			m_fadeState = FadeState::Hold;
			m_fadeTimer = 0.0f;
		}
		break;

	case FadeState::Hold:
		// 黒画面を一定時間ホールド
		if (m_fadeTimer >= FADE_HOLD_DURATION) {
			m_fadeState = FadeState::FadeIn;
			m_fadeTimer = 0.0f;
		}
		break;

	case FadeState::FadeIn:
		// 徐々に明るく
		if (m_fadeTimer >= FADE_IN_DURATION) {
			m_fadeState = FadeState::None;
			m_fadeTimer = 0.0f;
		}
		break;

	default:
		break;
	}
}

void PlayScene::DrawFadeOverlay()
{
	if (m_fadeState == FadeState::None) return;

	Renderer* renderer = m_game->GetRenderer();

	float t = 0.0f;

	switch (m_fadeState)
	{
	case FadeState::FadeOut:
		// 0 → 1
		t = m_fadeTimer / FADE_OUT_DURATION;
		break;

	case FadeState::Hold:
		// 完全に黒
		t = 1.0f;
		break;

	case FadeState::FadeIn:
		// 1 → 0
		t = 1.0f - (m_fadeTimer / FADE_IN_DURATION);
		break;

	default:
		return;
	}

	// クランプ
	if (t < 0.0f) t = 0.0f;
	if (t > 1.0f) t = 1.0f;

	int alpha = static_cast<int>(t * 255.0f);
	renderer->DrawFullScreenFill(Color(0, 0, 0), alpha);
}
