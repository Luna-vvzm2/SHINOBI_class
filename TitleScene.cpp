#include "TitleScene.h"
#include "Game.h"
#include "Renderer.h"
#include "Input.h"
#include "Color.h"
#include "TitleUI.h"
#include <Dxlib.h>
#include <stdlib.h> // EXIT用

TitleScene::TitleScene(Game* game)
	: Scene(game),
	titlePos(0.0f, 0.0f),
	m_subState(SubState::Title), // 最初はタイトル画面
	titleSelect(0),
	settingSelect(0),
	extraSelect(0),
	imgTitleBg(-1),
	imgSettingBg(-1),
	imgSoundBg(-1),
	imgGraphicBg(-1),
	imgLangBg(-1),
	prevEnter(false),
	prevEsc(false),
	prevUp(false),
	prevDown(false),
	prevMouseLeft(false)
{
	titleItems = { "コンティニュー", "New Game", "設定", "エクストラ", "EXIT" };
	setItems = { "サウンド", "グラフィック", "言語選択" };
}

TitleScene::~TitleScene() {
	if (imgTitleBg != -1)       DeleteGraph(imgTitleBg);
	if (imgSettingBg != -1)     DeleteGraph(imgSettingBg);
	if (imgSoundBg != -1)       DeleteGraph(imgSoundBg);
	if (imgGraphicBg != -1)     DeleteGraph(imgGraphicBg);
	if (imgLangBg != -1)        DeleteGraph(imgLangBg);
}

bool TitleScene::Init() {
	titlePos = { m_game->GetWidth() / 2.0f, m_game->GetHeight() / 10.0f };
	m_type = Type::Title;

	TitleUI* title = new TitleUI(this);
	AddUIActor(title);

	if (imgTitleBg == -1)    imgTitleBg = LoadGraph("kari/basho.png");
	if (imgSettingBg == -1)  imgSettingBg = LoadGraph("kari/haikei1.png");
	if (imgSoundBg == -1)    imgSoundBg = LoadGraph("kari/haikei2.png");
	if (imgGraphicBg == -1)  imgGraphicBg = LoadGraph("kari/haikei3.png");
	if (imgLangBg == -1)     imgLangBg = LoadGraph("kari/haikei4.png");

	m_isRunning = true;
	return true;
}

void TitleScene::Update(float deltaTime) {
	if (m_subState == SubState::Title) {
		updateActors(m_UIactors, deltaTime);
	}

	bool nowEnter = (CheckHitKey(KEY_INPUT_RETURN) != 0);
	bool nowEsc = (CheckHitKey(KEY_INPUT_ESCAPE) != 0);
	bool nowUp = (CheckHitKey(KEY_INPUT_UP) != 0) || (CheckHitKey(KEY_INPUT_W) != 0);
	bool nowDown = (CheckHitKey(KEY_INPUT_DOWN) != 0) || (CheckHitKey(KEY_INPUT_S) != 0);

	int mouseX, mouseY;
	GetMousePoint(&mouseX, &mouseY);
	bool mouseLeft = ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0);

	bool triggerEnter = (nowEnter && !prevEnter);
	bool triggerEsc = (nowEsc && !prevEsc);
	bool triggerUp = (nowUp && !prevUp);
	bool triggerDown = (nowDown && !prevDown);
	bool triggerLeftClick = (mouseLeft && !prevMouseLeft);

	int titleX = 850;
	int titleY = 400;
	int setX = 500;
	int setY = 300;

	switch (m_subState) {
	case SubState::Title:
		for (int i = 0; i < 5; i++) {
			int y = titleY + i * 50;
			if (mouseX >= titleX - 40 && mouseX <= titleX + 250 && mouseY >= y && mouseY < y + 40) {
				titleSelect = i;
			}
		}

		if (triggerUp)   titleSelect = (titleSelect - 1 + 5) % 5;
		if (triggerDown) titleSelect = (titleSelect + 1) % 5;

		if (triggerEnter || triggerLeftClick) {
			if (titleSelect == 0) {
				// ★新しく Game クラスに追加したシーン切り替え関数を呼び出す
				m_game->ChangeScene(Scene::Type::Play);
			}
			else if (titleSelect == 1) {
				// New Game
			}
			else if (titleSelect == 2) {
				m_subState = SubState::Settings;
				settingSelect = 0;
			}
			else if (titleSelect == 3) {
				m_subState = SubState::Extra;
				extraSelect = 0;
			}
			else if (titleSelect == 4) {
				DxLib_End();
				exit(0);
			}
		}
		break;

	case SubState::Settings:
		for (int i = 0; i < 3; i++) {
			int y = setY + i * 80;
			if (mouseX >= setX - 40 && mouseX <= setX + 250 && mouseY >= y && mouseY < y + 60) {
				settingSelect = i;
			}
		}

		if (triggerUp)   settingSelect = (settingSelect - 1 + 3) % 3;
		if (triggerDown) settingSelect = (settingSelect + 1) % 3;

		if (triggerEnter || triggerLeftClick) {
			if (settingSelect == 0)      m_subState = SubState::SettingsSound;
			else if (settingSelect == 1) m_subState = SubState::SettingsGraphic;
			else if (settingSelect == 2) m_subState = SubState::SettingsLanguage;
		}

		if (triggerEsc || (triggerLeftClick && mouseX >= 1050 && mouseX <= 1250 && mouseY >= 650 && mouseY <= 710)) {
			m_subState = SubState::Title;
		}
		break;

	case SubState::SettingsSound:
	case SubState::SettingsGraphic:
	case SubState::SettingsLanguage:
		if (triggerEsc || (triggerLeftClick && mouseX >= 1050 && mouseX <= 1250 && mouseY >= 650 && mouseY <= 710)) {
			m_subState = SubState::Settings;
		}
		break;

	case SubState::Extra:
		if (mouseX >= 450 && mouseX <= 750 && mouseY >= 340 && mouseY <= 400) {
			extraSelect = 0;
		}

		if (triggerEnter || triggerLeftClick) {
			if (extraSelect == 0) m_subState = SubState::ExtraCredit;
		}

		if (triggerEsc || (triggerLeftClick && mouseX >= 1050 && mouseX <= 1250 && mouseY >= 650 && mouseY <= 710)) {
			m_subState = SubState::Title;
		}
		break;

	case SubState::ExtraCredit:
		if (triggerEsc || (triggerLeftClick && mouseX >= 1050 && mouseX <= 1250 && mouseY >= 650 && mouseY <= 710)) {
			m_subState = SubState::Extra;
		}
		break;
	}

	prevEnter = nowEnter; prevEsc = nowEsc; prevUp = nowUp; prevDown = nowDown;
	prevMouseLeft = mouseLeft;
}

void TitleScene::Draw() {
	Renderer* renderer = m_game->GetRenderer();
	if (!renderer) return;

	int titleX = 850;
	int titleY = 400;
	int setX = 500;
	int setY = 300;

	switch (m_subState) {
	case SubState::Title:
		if (imgTitleBg != -1) {
			DrawExtendGraph(0, 0, 1280, 720, imgTitleBg, TRUE);
		}
		else {
			DrawBox(0, 0, 1280, 720, GetColor(0, 0, 200), TRUE);
		}

		for (int i = 0; i < 5; i++) {
			int y = titleY + i * 50;
			if (titleSelect == i) {
				SetFontSize(36);
				DrawString(titleX - 20, y - 5, titleItems[i].c_str(), GetColor(255, 255, 255));
			}
			else {
				SetFontSize(24);
				DrawString(titleX, y, titleItems[i].c_str(), GetColor(150, 150, 150));
			}
		}
		SetFontSize(16);

		drawActors(m_UIactors);

		{
			const std::string& debugFont = m_game->GatDebugFont();
			renderer->DrawTextC(Vector2d(m_game->GetWidth() / 2.0f, m_game->GetHeight() * 0.8f), "Press [ENTER] or (B) to Start", Color(192, 192, 192), debugFont, 24, false);
		}
		break;

	case SubState::Settings:
		if (imgSettingBg != -1) {
			DrawExtendGraph(0, 0, 1280, 720, imgSettingBg, TRUE);
		}
		else {
			DrawBox(0, 0, 1280, 720, GetColor(0, 0, 0), TRUE);
		}
		SetFontSize(48);
		DrawString(100, 80, "設定", GetColor(255, 50, 50));
		DrawBox(100, 140, 1180, 142, GetColor(150, 50, 50), TRUE);

		for (int i = 0; i < 3; i++) {
			int y = setY + i * 80;
			if (settingSelect == i) {
				SetFontSize(40);
				DrawString(setX - 20, y - 10, setItems[i].c_str(), GetColor(255, 50, 50));
			}
			else {
				SetFontSize(28);
				DrawString(setX, y, setItems[i].c_str(), GetColor(150, 150, 150));
			}
		}
		SetFontSize(16);
		DrawString(1100, 680, "[Esc] 戻る", GetColor(200, 200, 200));
		break;

	case SubState::SettingsSound:
		if (imgSoundBg != -1) {
			DrawExtendGraph(0, 0, 1280, 720, imgSoundBg, TRUE);
		}
		else {
			DrawBox(0, 0, 1280, 720, GetColor(0, 0, 0), TRUE);
		}
		SetFontSize(48);
		DrawString(100, 80, "設定 ＞ サウンド", GetColor(50, 150, 255));
		DrawBox(100, 140, 1180, 142, GetColor(50, 100, 200), TRUE);

		SetFontSize(28);
		DrawString(200, 300, "・BGM 音量   [ 80 ]", GetColor(255, 255, 255));
		DrawString(200, 380, "・SE  音量   [ 90 ]", GetColor(255, 255, 255));

		SetFontSize(16);
		DrawString(1100, 680, "[Esc] 戻る", GetColor(200, 200, 200));
		break;

	case SubState::SettingsGraphic:
		if (imgGraphicBg != -1) {
			DrawExtendGraph(0, 0, 1280, 720, imgGraphicBg, TRUE);
		}
		else {
			DrawBox(0, 0, 1280, 720, GetColor(0, 0, 0), TRUE);
		}
		SetFontSize(48);
		DrawString(100, 80, "設定 ＞ グラフィック", GetColor(50, 255, 150));
		DrawBox(100, 140, 1180, 142, GetColor(50, 200, 100), TRUE);

		SetFontSize(28);
		DrawString(200, 300, "・画面モード : ウインドウモード", GetColor(255, 255, 255));
		DrawString(200, 380, "・解像度     : 1280 x 720", GetColor(255, 255, 255));

		SetFontSize(16);
		DrawString(1100, 680, "[Esc] 戻る", GetColor(200, 200, 200));
		break;

	case SubState::SettingsLanguage:
		if (imgLangBg != -1) {
			DrawExtendGraph(0, 0, 1280, 720, imgLangBg, TRUE);
		}
		else {
			DrawBox(0, 0, 1280, 720, GetColor(0, 0, 0), TRUE);
		}
		SetFontSize(48);
		DrawString(100, 80, "設定 ＞ 言語選択", GetColor(255, 50, 255));
		DrawBox(100, 140, 1180, 142, GetColor(200, 50, 200), TRUE);

		SetFontSize(28);
		DrawString(200, 300, "・日本語 (Japanese)  ＜選択中＞", GetColor(255, 255, 255));
		DrawString(200, 380, "・英語   (English)", GetColor(150, 150, 150));

		SetFontSize(16);
		DrawString(1100, 680, "[Esc] 戻る", GetColor(200, 200, 200));
		break;

	case SubState::Extra:
		DrawBox(0, 0, 1280, 720, GetColor(20, 10, 10), TRUE);
		SetFontSize(48);
		DrawString(100, 80, "エクストラ", GetColor(255, 50, 50));
		DrawBox(100, 140, 1180, 142, GetColor(150, 150, 50), TRUE);

		if (extraSelect == 0) {
			SetFontSize(40);
			DrawString(500, 350, "権利表記", GetColor(255, 50, 50));
		}

		SetFontSize(16);
		DrawString(1100, 680, "[Esc] 戻る", GetColor(200, 200, 200));
		break;

	case SubState::ExtraCredit:
		DrawBox(0, 0, 1280, 720, GetColor(0, 0, 0), TRUE);
		SetFontSize(48);
		DrawString(100, 80, "エクストラ ＞ 権利表記", GetColor(255, 255, 50));
		DrawBox(100, 140, 1180, 142, GetColor(200, 200, 50), TRUE);

		SetFontSize(24);
		DrawString(200, 260, "【プログラム / グラフィック】", GetColor(180, 180, 180));
		DrawString(200, 300, "　あなた (You)", GetColor(255, 255, 255));
		DrawString(200, 380, "【使用ライブラリ】", GetColor(180, 180, 180));
		DrawString(200, 420, "　DXライブラリ (Copyright (C) 2001-2026 Takumi Yamada)", GetColor(255, 255, 255));

		SetFontSize(16);
		DrawString(1100, 680, "[Esc] 戻る", GetColor(200, 200, 200));
		break;
	}

#ifdef _DEBUG
	const std::string& debugFont = m_game->GatDebugFont();
	renderer->DrawTextL(Vector2d(m_game->GetWidth() - 150.0f, 0), "TitleScene", Color(255, 64, 0), debugFont, 24, false);
#endif
}