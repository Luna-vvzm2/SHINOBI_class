#pragma once
#include "Scene.h"
#include "Vector2d.h"
#include <vector>
#include <string>

class TitleScene : public Scene
{
public:
	// タイトルシーン内部の画面状態（サブ状態）を定義
	enum class SubState {
		Opening, // ← 追加: オープニング画面（PRESS START等）
		Title,
		Settings,
		SettingsSound,
		SettingsGraphic,
		SettingsLanguage,
		Extra,
		ExtraCredit

	};

	//	コンストラクタ
	TitleScene(class Game* game);
	//	デストラクタ
	~TitleScene() override;

	bool Init() override;
	void Update(float deltaTime) override;
	void Draw() override;

private:
	Vector2d titlePos;

	// 現在の内部画面状態
	SubState m_subState;

	// 各画面での選択カーソル位置
	int titleSelect;
	int settingSelect;
	int extraSelect;

	// 設定・エキストラ関係の背景画像ハンドル
	int imgOpeningBg; // ← 追加: オープニング用背景
	int imgTitleBg;
	int imgSettingBg;
	int imgSoundBg;
	int imgGraphicBg;
	int imgLangBg;

	// ★黄色の警告対策：int型からbool型に変更
	bool prevEnter;
	bool prevEsc;
	bool prevUp;
	bool prevDown;
	bool prevMouseLeft;

	// メメニューの項目名リスト
	std::vector<std::string> titleItems;
	std::vector<std::string> setItems;
};