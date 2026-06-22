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

private:

	PlayerEntity* m_player;
	HitEffect* m_effect;
	MapData m_mapData;

	Camera m_camera;

	int m_stageIndex;
	int m_comboCount;
	int m_currentStage;
	bool m_resultShown = false;

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
};