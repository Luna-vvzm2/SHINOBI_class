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

//イベントのため追加
class EventTexture;
class EventManager;

class PlayScene : public Scene
{
public:
	//	コンストラクタ
	PlayScene(class Game* game);
	//	デストラクタ
	~PlayScene() override = default;

	bool Init() override;
	bool StageInit(int stageNo);

	void ChangeStage(int index, int spawnIndex);
	void ClearStageActors();
	void RequestStageChange(int stage, int spawnIndex);

	//	更新
	void Update(float deltaTime) override;
	void Draw() override;

	PlayerEntity* GetPlayer() const { return m_player; }

	// 衝突イベント時に呼ぶ
	//void SpawnHitEffect(const Vector2d& pos);

	void AddCombo();

	int GetCombo() const { return m_comboCount; }

	bool IsResult() const { return m_resultShown; }
	Camera& GetCamera() { return m_camera; }
	const Camera& GetCamera() const { return m_camera; }

	int m_bgHandle;
	int m_fgHandle;
private:

	PlayerEntity* m_player;
	HitEffect* m_effect;
	MapData m_mapData;
	std::vector<Vector2d> m_playerSpawnPoints;
	bool m_requestStageChange = false;
	int m_nextStage = 0;
	int m_nextSpawnIndex = 0;

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

	enum class MenuState
	{
		None,
		Skill,
		Equipment,
		Item
	};

	MenuState m_menuState = MenuState::None;


	// スキルデータ
	struct SkillData
	{
		std::string name;
		std::string description;

		int iconHandle = -1; 

		bool unlocked = true;
	};

	int m_lockedSkillIcon = -1;

	std::vector<SkillData> m_ninjutsu;
	std::vector<SkillData> m_ninpou;
	std::vector<SkillData> m_ningi;
	std::vector<SkillData> m_combat;


	// スキルメニュー
	void DrawSkillMenu();
	void UpdateSkillMenu(float deltaTime);
	int GetSkillMaxX(int y);
	void ClampSkillCursor();
	SkillData* GetSelectedSkill();

	void DrawSkillRow(
		const std::vector<SkillData>& skills,
		int startX,
		int startY
	);

	void DrawSkillSlot(
		int x,
		int y,
		const SkillData& skill
	) const;

	void DrawSkillGrid(
		const std::vector<SkillData>& skills,
		int startX,
		int startY,
		int columns
	);

	int m_skillCursorX = 0;
	int m_skillCursorY = 0;

	float m_cursorRepeatTimer = 0.0f;

	const float m_cursorRepeatDelay = 0.3f; // 最初の待ち時間
	const float m_cursorRepeatInterval = 0.08f; // 連続移動間隔
	
	//イベントのため変更
	std::unique_ptr<EventTexture> m_eventTexture;
	std::unique_ptr<EventManager> m_eventManager;

};