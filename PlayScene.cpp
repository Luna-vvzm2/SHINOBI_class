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

#include <algorithm>

void PlayScene::ClampSkillCursor()
{
	// ‘S‘Ì”ÍˆÍ
	if (m_skillCursorY < 0)
		m_skillCursorY = 0;

	if (m_skillCursorY > 4)
		m_skillCursorY = 4;


	int max = 0;


	switch (m_skillCursorY)
	{
	case 0:
		// ”Ep
		max = 4;
		break;


	case 1:
		// ”E–@
		max = 8;
		break;


	case 2:
		// ”E‹Z
		max = 7;
		break;


	case 3:
		// í“¬‹Z 1’i–Ú
		max = 9;
		break;


	case 4:
		// í“¬‹Z 2’i–Ú
		max = 8;
		break;
	}



	if (m_skillCursorX < 0)
		m_skillCursorX = 0;


	if (m_skillCursorX >= max)
		m_skillCursorX = max - 1;
}

int PlayScene::GetSkillMaxX(int y)
{
	switch (y)
	{
	case 0:
		return 4; // ”Ep

	case 1:
		return 8; // ”E–@

	case 2:
		return 7; // ”E‹Z

	case 3:
		return 9; // í“¬‹Zã’i

	case 4:
		return 8; // í“¬‹Z‰º’i
	}

	return 0;
}

void PlayScene::UpdateSkillMenu(float deltaTime)
{
	const Input& input = m_game->GetInput();


	bool move = false;


	// ‰Ÿ‚µ‚½uŠÔ
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



	// ’·‰Ÿ‚µ’†
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
		// —£‚µ‚½‚çƒŠƒZƒbƒg
		m_cursorRepeatTimer = 0.0f;
	}


	ClampSkillCursor();
}

void PlayScene::DrawSkillMenu()
{
	// ”wŒi
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


	// ƒ^ƒCƒgƒ‹
	DrawString(
		800,
		50,
		"ƒXƒLƒ‹",
		GetColor(255, 255, 255)
	);


	// ƒJƒeƒSƒŠ–¼
	DrawString(
		startX,
		120,
		"”Ep",
		GetColor(255, 255, 255)
	);


	// ”Ep 4ŒÂ
	for (int i = 0; i < 4; i++)
	{
		int x = startX + i * (size + gap);
		int y = 160;

		DrawBox(
			x,
			y,
			x + size,
			y + size,
			GetColor(255, 255, 255),
			FALSE
		);
	}



	// ”E–@ 8ŒÂ
	DrawString(
		startX,
		250,
		"”E–@",
		GetColor(255, 255, 255)
	);


	for (int i = 0; i < 8; i++)
	{
		int x = startX + i * (size + gap);
		int y = 290;


		DrawBox(
			x,
			y,
			x + size,
			y + size,
			GetColor(255, 255, 255),
			FALSE
		);
	}



	// ”E‹Z 7ŒÂ
	DrawString(
		startX,
		380,
		"”E‹Z",
		GetColor(255, 255, 255)
	);


	for (int i = 0; i < 7; i++)
	{
		int x = startX + i * (size + gap);
		int y = 420;


		DrawBox(
			x,
			y,
			x + size,
			y + size,
			GetColor(255, 255, 255),
			FALSE
		);
	}



	// í“¬‹Z 17ŒÂ
	DrawString(
		startX,
		510,
		"í“¬‹Z",
		GetColor(255, 255, 255)
	);


	for (int i = 0; i < 17; i++)
	{
		int x =
			startX + (i % 9) * (size + gap);

		int y =
			550 + (i / 9) * (size + gap);


		DrawBox(
			x,
			y,
			x + size,
			y + size,
			GetColor(255, 255, 255),
			FALSE
		);
	}

	// ƒJ[ƒ\ƒ‹•`‰æ

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

	int index =
		m_skillCursorY * 4 +
		m_skillCursorX;

	DrawString(
		950,
		430,
		"ƒeƒXƒg—p•¶Í",
		GetColor(255, 255, 255)
	);

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

	//ƒXƒLƒ‹ƒf[ƒ^
	m_ninjutsu =
	{
		{"”Ep1", "test"},
		{"”Ep2", "test"},
		{"”Ep3", "test"},
		{"”Ep4", "test"}
	};


	m_ninpou =
	{
		{"”E–@1", "test"},
		{"”E–@2", "test"},
		{"”E–@3", "test"},
		{"”E–@4", "test"},
		{"”E–@5", "test"},
		{"”E–@6", "test"},
		{"”E–@7", "test"},
		{"”E–@8", "test"}
	};


	m_ningi =
	{
		{"”E‹Z1", "test"},
		{"”E‹Z2", "test"},
		{"”E‹Z3", "test"},
		{"”E‹Z4", "test"},
		{"”E‹Z5", "test"},
		{"”E‹Z6", "test"},
		{"”E‹Z7", "test"}
	};


	m_combat =
	{
		{"í“¬‹Z1", "test"},
		{"í“¬‹Z2", "test"},
		{"í“¬‹Z3", "test"},
		{"í“¬‹Z4", "test"},
		{"í“¬‹Z5", "test"},
		{"í“¬‹Z6", "test"},
		{"í“¬‹Z7", "test"},
		{"í“¬‹Z8", "test"},
		{"í“¬‹Z9", "test"},

		{"í“¬‹Z10", "test"},
		{"í“¬‹Z11", "test"},
		{"í“¬‹Z12", "test"},
		{"í“¬‹Z13", "test"},
		{"í“¬‹Z14", "test"},
		{"í“¬‹Z15", "test"},
		{"í“¬‹Z16", "test"},
		{"í“¬‹Z17", "test"}
	};


	
	// CSV ‚©‚çƒ}ƒbƒv“Ç‚İ‚İ
	if (!m_mapData.LoadStage("assets/maps/stage1")) {
		std::cerr << "ƒ}ƒbƒv‚P“Ç‚İ‚İ¸”s" << std::endl;
		return false;
	}
	if (!m_mapData.LoadStage("assets/maps/stage2")) {
		std::cerr << "ƒ}ƒbƒv‚Q“Ç‚İ‚İ¸”s" << std::endl;
		return false;
	}
	if (!m_mapData.LoadStage("assets/maps/stage3")) {
		std::cerr << "ƒ}ƒbƒv‚R“Ç‚İ‚İ¸”s" << std::endl;
		return false;
	}
	m_mapData.tileSize = 104;

	StageData& stage = m_mapData.stages[m_stageIndex];

	// ƒ}ƒbƒv‚Ìƒ^ƒCƒ‹‚ğ”z’u‚·‚é
	const float tileSize = static_cast<float>(m_mapData.tileSize);
	const Layer& mapLayer = stage.layers[0];

	for (int y = 0; y < stage.height; ++y) {
		for (int x = 0; x < stage.width; ++x) {
			int tileID = mapLayer.tiles[y * stage.width + x];
			if (tileID == 0) continue; // ‹óƒ^ƒCƒ‹

			Vector2d pos(x * tileSize, y * tileSize);

			// ‚±‚±‚Åƒ^ƒCƒ‹ID‚É‰‚¶‚ÄƒuƒƒbƒN¶¬
			// —áFGrassBlock ‚Æ IceBlock ‚ğ‰¼‚ÉØ‚è‘Ö‚¦
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
	
	m_player = new PlayerEntity(this, Vector2d({200, 800}), Vector2d({192, 64}));
	AddActor(m_player);
	
	// ---- HP UI ì¬ ----
	HPBarUI* hpBar = new HPBarUI(
		this,
		m_player->GetHP()
	);
	AddUIActor(hpBar);

	BackGroundUI* back = new BackGroundUI(this, "assets/images/uies/bg.png");
	AddBackActor(back);


	float halfTile = m_mapData.tileSize * 0.5f;
	m_camera.SetTileHalfSize(Vector2d(halfTile, halfTile));

	// Camera ‚Ìƒ}ƒbƒv”ÍˆÍİ’è
	float mapW = (float)stage.width * m_mapData.tileSize;
	float mapH = (float)stage.height * m_mapData.tileSize;
	m_camera.SetBounds(Vector2d(0, 0), Vector2d(mapW, mapH));

	// Renderer ‚É Camera ‚ğƒZƒbƒg
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
	RemoveDeadActors(); 

	if (m_player) {
		Vector2d playerPos = m_player->GetComponent<TransformComponent>()->GetPosition();

		// ’†ŠÔ“_‚ğƒJƒƒ‰ˆÊ’u‚É
		Vector2d camPos = playerPos;
		camPos.y -= 150;
		m_camera.SetCenter(camPos);

		// ‹——£‚É‰‚¶‚½ƒY[ƒ€
		/*
		float distance = (playerPos).length();
		float minZoom = 1.0f;
		float maxZoom = 2.5f;
		float idealDistance = 300.0f;
		float targetZoom = std::clamp(idealDistance / distance, minZoom, maxZoom);

		// üŒ`•âŠÔ‚Å™X‚ÉƒY[ƒ€•ÏX
		float currentZoom = m_camera.GetZoom();
		float zoomSpeed = 5.0f;
		float newZoom = currentZoom + (targetZoom - currentZoom) * std::min(zoomSpeed * deltaTime, 1.0f);
		*/
		m_camera.SetZoom(1.0f);
	}
}

void PlayScene::Draw() {
	Renderer* renderer = m_game->GetRenderer();



	// --- ƒAƒNƒ^[•`‰æ ---
	drawActors(m_backactors);
	drawActors(m_actors);
	drawActors(m_UIactors);

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

	// --- ƒfƒoƒbƒO•¶š ---
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


