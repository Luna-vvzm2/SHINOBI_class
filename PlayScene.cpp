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
#include "EffectActor.h"
#include "HitEffect.h"
#include "GroundBlock.h"
#include "HPBarUI.h"
#include "ShurikenUI.h"
#include "BackGroundUI.h"
#include "TransformComponent.h"
#include "HPComponent.h"
#include "Camera.h"
#include "MoneyUI.h"
#include "EnemyHPBar.h"

#include <algorithm>

//====================
// スキル画面
//====================

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

	//=========================
	// タブ選択中
	//=========================
	if (m_cursorArea == MenuCursorArea::Tab)
	{
		if (input.IsTrigger(Action::RIGHT))
		{
			m_tabCursor = 0;
		}

		if (input.IsTrigger(Action::LEFT))
		{
			m_tabCursor = 1;
		}

		if (input.IsTrigger(Action::ENTER))
		{
			if (m_tabCursor == 0)
			{
				m_menuTab = MenuTab::Skill;
			}
			else
			{
				m_menuTab = MenuTab::Equipment;
			}

			m_cursorArea = MenuCursorArea::SkillList;
		}

		if (input.IsTrigger(Action::DOWN))
		{
			m_cursorArea = MenuCursorArea::SkillList;
			return;
		}

	}
		//=========================
		// スキル一覧選択中
		//=========================

		// 一番上ならタブへ戻る
		if (m_skillCursorY == 0 &&
			input.IsTrigger(Action::UP))
		{
			m_cursorArea = MenuCursorArea::Tab;
			return;
		}

		// 押した瞬間
		if (input.IsTrigger(Action::RIGHT))
			m_skillCursorX++;

		if (input.IsTrigger(Action::LEFT))
			m_skillCursorX--;

		if (input.IsTrigger(Action::DOWN))
			m_skillCursorY++;

		if (input.IsTrigger(Action::UP))
			m_skillCursorY--;

		// 長押し
		if (input.IsDown(Action::RIGHT) ||
			input.IsDown(Action::LEFT) ||
			input.IsDown(Action::DOWN) ||
			input.IsDown(Action::UP))
		{
			m_cursorRepeatTimer += deltaTime;

			if (m_cursorRepeatTimer >= m_cursorRepeatDelay)
			{
				if (m_cursorRepeatTimer >=
					m_cursorRepeatDelay + m_cursorRepeatInterval)
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
	DrawMenuTabs();

	const int startX = 50;
	const int size = 60;
	const int gap = 15;


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


	if (m_cursorArea == MenuCursorArea::SkillList)
	{
		DrawBox(
			cursorX - 5,
			cursorY - 5,
			cursorX + size + 5,
			cursorY + size + 5,
			GetColor(255, 0, 0),
			FALSE
		);
	}

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

//====================
// 装備画面
//====================

void PlayScene::DrawEquipmentMenu()
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

	DrawMenuTabs();

	DrawNinjutsuSlots();

	DrawGofuSlots();

	DrawKatanaSlot();

	DrawEquipmentInventory();

	DrawEquipmentDescription();
}

void PlayScene::DrawNinjutsuSlots()
{
	const int x = 50;
	const int y = 120;
	const int size = 70;
	const int gap = 20;

	DrawString(
		x,
		y - 40,
		"忍法",
		GetColor(255, 255, 255)
	);

	for (int i = 0; i < 4; i++)
	{
		DrawBox(
			x,
			y + i * (size + gap),
			x + size,
			y + i * (size + gap) + size,
			GetColor(255, 255, 255),
			FALSE
		);
	}
}

void PlayScene::DrawGofuSlots()
{
	const int x = 350;
	const int y = 120;
	const int size = 70;
	const int gap = 30;

	DrawString(
		x,
		y - 40,
		"護符",
		GetColor(255, 255, 255)
	);

	// 自動発動
	DrawString(
		x,
		y + 20,
		"自動発動",
		GetColor(255, 255, 255)
	);

	DrawBox(
		x + 120,
		y,
		x + 120 + size,
		y + size,
		GetColor(255, 255, 255),
		FALSE
	);

	// 連撃発動
	DrawString(
		x,
		y + size + gap + 20,
		"連撃発動",
		GetColor(255, 255, 255)
	);

	DrawBox(
		x + 120,
		y + size + gap,
		x + 120 + size,
		y + size * 2 + gap,
		GetColor(255, 255, 255),
		FALSE
	);
}

void PlayScene::DrawKatanaSlot()
{
	const int x = 350;
	const int y = 390;
	const int size = 70;

	DrawString(
		x,
		y - 40,
		"刀",
		GetColor(255, 255, 255)
	);

	// 刀名
	DrawString(
		x,
		y + 20,
		"朧月",
		GetColor(255, 255, 255)
	);

	// 装備枠
	DrawBox(
		x + 120,
		y,
		x + 120 + size,
		y + size,
		GetColor(255, 255, 255),
		FALSE
	);
}

void PlayScene::DrawEquipmentInventory()
{
	const int startX = 50;
	const int startY = 560;

	const int size = 60;
	const int gap = 15;

	DrawString(
		startX,
		startY - 40,
		"所持装備",
		GetColor(255, 255, 255)
	);

	for (int i = 0; i < 16; i++)
	{
		int x = startX + (i % 8) * (size + gap);
		int y = startY + (i / 8) * (size + gap);

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

void PlayScene::DrawEquipmentDescription()
{
	DrawBox(
		650,
		120,
		1220,
		680,
		GetColor(255, 255, 255),
		FALSE
	);

	DrawString(
		680,
		150,
		"説明",
		GetColor(255, 255, 255)
	);
}



//====================
// タブ
//====================

void PlayScene::DrawMenuTabs()
{
	const int tabY = 20;
	const int tabW = 180;
	const int tabH = 50;
	const int tabGap = 10;

	// タブ全体の幅
	const int totalWidth = tabW * 2 + tabGap;

	// 画面中央
	const int equipX = (m_game->GetWidth() - totalWidth) / 2;
	const int skillX = equipX + tabW + tabGap;

	//====================
	// 装備
	//====================
	DrawBox(
		equipX,
		tabY,
		equipX + tabW,
		tabY + tabH,
		GetColor(80, 80, 80),
		TRUE
	);

	DrawString(
		equipX + 70,
		tabY + 15,
		"装備",
		GetColor(255, 255, 255)
	);

	//====================
	// スキル
	//====================
	DrawBox(
		skillX,
		tabY,
		skillX + tabW,
		tabY + tabH,
		GetColor(80, 80, 80),
		TRUE
	);

	DrawString(
		skillX + 60,
		tabY + 15,
		"スキル",
		GetColor(255, 255, 255)
	);

	//====================
	// タブカーソル
	//====================
	if (m_cursorArea == MenuCursorArea::Tab)
	{
		int x = (m_tabCursor == 0) ? skillX : equipX;

		DrawBox(
			x - 4,
			tabY - 4,
			x + tabW + 4,
			tabY + tabH + 4,
			GetColor(255, 0, 0),
			FALSE
		);
	}
}


PlayScene::PlayScene(Game* game)
	: Scene(game),
	m_player(nullptr),
	m_effect(nullptr),
	m_camera(static_cast<float>(game->GetWidth()), static_cast<float>(game->GetHeight())),
	m_stageIndex(0),
	m_comboCount(0),
	m_currentStage(1),
	m_skillCursorX(0),
	m_skillCursorY(0)
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

	StageData& stage = m_mapData.stages[m_stageIndex];

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

			}
		}
	}

	const Layer& objLayer = stage.layers[1];

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

				case 2:
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

				case 3:
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

				case 4:
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

				case 5:
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

				case 6:
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

				case 7:
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

				case 8:
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
					break;
				} // switch
			} // for x
		} // for y
	m_player = new PlayerEntity(this, Vector2d({200, 800}), Vector2d({192, 64
		}));
	AddActor(m_player);

	// ---- HP UI 作成 ----
	HPBarUI* hpBar = new HPBarUI(
		this,
		m_player->GetHP()
	);
	AddUIActor(hpBar);

	// プレイヤーの HP が減ったらコンボをリセットする
	if (m_player && m_player->GetHP()) {
		m_player->GetHP()->OnHPChanged = [this](int newHP, int oldHP) {
			if (newHP < oldHP) {
				m_comboCount = 0;
				// 必要ならログやサウンドを追加
				std::cout << "Combo reset due to player damage\n";
			}
			};
	}

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

	BackGroundUI* back = new BackGroundUI(this, "assets/images/uies/bg.png");
	AddBackActor(back);

	EffectActor::LoadEffects();

	float halfTile = m_mapData.tileSize * 0.5f;
	m_camera.SetTileHalfSize(Vector2d(halfTile, halfTile));

	// Camera のマップ範囲設定
	float mapW = (float)stage.width * m_mapData.tileSize;
	float mapH = (float)stage.height * m_mapData.tileSize;
	m_camera.SetBounds(Vector2d(0, 0), Vector2d(mapW, mapH));

	// Renderer に Camera をセット
	m_game->GetRenderer()->SetCamera(&m_camera);

	return true;
}

void PlayScene::Update(float deltaTime) {

	const Input& input = m_game->GetInput();

	if (input.IsTrigger(Action::MENU))
	{
		if (m_menuState == MenuState::None)
		{
			m_menuState = MenuState::Skill;
			m_cursorArea = MenuCursorArea::Tab;
		}
		else
		{
			m_menuState = MenuState::None;
		}

		return;
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
	RemoveDeadActors(); 


	// カメラ設定など（既にある処理）
	if (m_player) {
		Vector2d playerPos = m_player->GetComponent<TransformComponent>()->GetPosition();

		// 中間点をカメラ位置に
		Vector2d camPos = playerPos;
		camPos.y -= 150;
		m_camera.SetCenter(camPos);
		m_camera.SetZoom(1.0f);
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


	// 仮背景
	DrawBox(0, 0, 1280, 720, GetColor(200, 200, 200), 1);

	// --- アクター描画 ---
	drawActors(m_backactors);
	drawActors(m_actors);
	drawActors(m_UIactors);

	if (m_menuState == MenuState::Skill)
	{
		if (m_menuTab == MenuTab::Skill)
		{
			DrawSkillMenu();
		}
		else
		{
			DrawEquipmentMenu();
		}
	}


	// 攻撃範囲描画 --------------------------
	Vector2d Pos = m_player->GetPos();
	AttackHitbox Weak1{ Vector2d(50,100), 100, 100, 30 };
	if (m_player->GetDir()) {
		Pos.x += Weak1.offset.x;
		Pos.y += Weak1.offset.y;
	} 
	else{
		Pos.x -= Weak1.offset.x;
		Pos.y += Weak1.offset.y;
	}
	renderer->DrawRectCenter(Pos, Weak1.width, Weak1.height, GetColor(0,255,0),false, true);
	//------------------------------------------

	std::vector<NumberInfo> comboInfo = {
		{ (float)m_comboCount, 0 }
	};

	// コンボ表示（m_comboCount が 1 以上なら表示）
	if (m_comboCount > 0) {
		const std::string& debugFont = m_game->GatDebugFont();
		// ここではフォントサイズを大きめ（例 48）で真ん中上に表示
		std::string comboText = std::to_string(m_comboCount) + " Combo";
		renderer->DrawTextL(Vector2d(20.0f, 120.0f), comboText, Color(255, 200, 32), debugFont, 32, false);
	}

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

//void PlayScene::SpawnHitEffect(const Vector2d& pos) {
//	m_effect = new HitEffect(this, pos, {32, 32});
//	AddActor(m_effect);
//	std::cout << "Spawned HitEffect at: " << pos.x << ", " << pos.y << std::endl;
//	
//}

void PlayScene::AddCombo() {
	m_comboCount++;
	// タイマーは廃止するため不要
	std::cout << "Combo: " << m_comboCount << std::endl;
}


