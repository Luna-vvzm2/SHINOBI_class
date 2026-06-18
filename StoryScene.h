#pragma once
#include "Scene.h"
#include <vector>
#include <string>

class StoryScene : public Scene
{
public:
	enum class StoryState {
		StoryText1,      // ENE登場
		StoryText2,      // 臓一族
		StoryText3,      // 圧倒的な力
		StoryText4,      // すべての始まりには
		StoryText5,      // 何人たりとも
		StoryText6,      // ─────死
		StoryText7,      // ルーズ卿の侵攻
		StoryText8,      // ナナユのシーン（優しい心）
		StoryText9,      // おはようあなた
		StoryText10,     // ルーズ卿への対抗心
		StoryText11,     // 臓一族の使命
		StoryText12,     // ナオコのセリフ
		Complete,	   // ストーリー完了
		story
	};

	StoryScene(class Game* game);
	~StoryScene() override;

	bool Init() override;
	void Update(float deltaTime) override;
	void Draw() override;

private:
	StoryState m_storyState;
	float m_elapsedTime;
	float m_stateChangeTime;

	bool prevEnter;
	bool prevEsc;

	// 各ストーリーのテキスト
	std::string GetStoryText() const;
};
