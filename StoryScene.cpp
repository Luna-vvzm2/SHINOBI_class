#include "StoryScene.h"
#include "Game.h"
#include "Renderer.h"
#include "Input.h"
#include "Color.h"
#include <Dxlib.h>

StoryScene::StoryScene(Game* game)
	: Scene(game),
	m_storyState(StoryState::StoryText1),
	m_elapsedTime(0.0f),
	m_stateChangeTime(4.0f),
	prevEnter(false),
	prevEsc(false)
{
}

StoryScene::~StoryScene() {
}

bool StoryScene::Init() {
	m_type = Type::Story;
	m_isRunning = true;
	m_elapsedTime = 0.0f;
	return true;
}

void StoryScene::Update(float deltaTime) {
	m_elapsedTime += deltaTime;

	bool nowEnter = (CheckHitKey(KEY_INPUT_RETURN) != 0);
	bool nowEsc = (CheckHitKey(KEY_INPUT_ESCAPE) != 0);

	bool triggerEnter = (nowEnter && !prevEnter);
	bool triggerEsc = (nowEsc && !prevEsc);

	// Escキーで終了
	if (triggerEsc) {
		m_game->ChangeScene(Scene::Type::Title);
		prevEsc = nowEsc;
		prevEnter = nowEnter;
		return;
	}

	// 自動進行または手動進行
	if (triggerEnter || m_elapsedTime >= m_stateChangeTime) {
		m_elapsedTime = 0.0f;

		switch (m_storyState) {
		case StoryState::StoryText1:
			m_storyState = StoryState::StoryText2;
			break;
		case StoryState::StoryText2:
			m_storyState = StoryState::StoryText3;
			break;
		case StoryState::StoryText3:
			m_storyState = StoryState::StoryText4;
			break;
		case StoryState::StoryText4:
			m_storyState = StoryState::StoryText5;
			break;
		case StoryState::StoryText5:
			m_storyState = StoryState::StoryText6;
			break;
		case StoryState::StoryText6:
			m_storyState = StoryState::StoryText7;
			break;
		case StoryState::StoryText7:
			m_storyState = StoryState::StoryText8;
			break;
		case StoryState::StoryText8:
			m_storyState = StoryState::StoryText9;
			break;
		case StoryState::StoryText9:
			m_storyState = StoryState::StoryText10;
			break;
		case StoryState::StoryText10:
			m_storyState = StoryState::StoryText11;
			break;
		case StoryState::StoryText11:
			m_storyState = StoryState::StoryText12;
			break;
		case StoryState::StoryText12:
			m_storyState = StoryState::Complete;
			m_game->ChangeScene(Scene::Type::Play);
			break;
		case StoryState::Complete:
			break;
		}
	}

	prevEnter = nowEnter;
	prevEsc = nowEsc;
}

void StoryScene::Draw() {
	// 背景を黒で塗りつぶし
	DrawBox(0, 0, 1280, 720, GetColor(0, 0, 0), TRUE);

	Renderer* renderer = m_game->GetRenderer();
	if (!renderer) return;

	SetFontSize(28);

	switch (m_storyState) {
	case StoryState::StoryText1:
		DrawString(200, 300, "平和な世界に、突如現れた武装組織、", GetColor(255, 255, 255));
		DrawString(200, 360, "「ENE コーポレーション」。", GetColor(255, 255, 255));
		break;

	case StoryState::StoryText2:
		DrawString(200, 300, "その脅威に立ち向かう力を持ちうるのは", GetColor(255, 255, 255));
		DrawString(200, 360, "ジョー・ムサシ率いる忍軍団「臓一族」のみ。", GetColor(255, 255, 255));
		break;

	case StoryState::StoryText3:
		DrawString(200, 300, "彼は、闇向かう者を圧倒的な力で滅ぼし、", GetColor(255, 255, 255));
		DrawString(200, 360, "世界を恐怖と絶望に陥れてゆく。", GetColor(255, 255, 255));
		break;

	case StoryState::StoryText4:
		DrawString(200, 300, "すべての始まりには......", GetColor(255, 255, 255));
		DrawString(200, 360, "必ず、終わりがあるのだ。", GetColor(255, 255, 255));
		break;

	case StoryState::StoryText5:
		DrawString(200, 300, "何人たりとも、", GetColor(255, 255, 255));
		DrawString(200, 360, "その運命から逃れることはできない。", GetColor(255, 255, 255));
		break;

	case StoryState::StoryText6:
		DrawString(200, 300, "　　　　　　　　　　─────死。", GetColor(255, 255, 255));
		break;

	case StoryState::StoryText7:
		DrawString(200, 300, "その組織を操るリーダーの名は「ルーズ卿」。", GetColor(255, 255, 255));
		DrawString(200, 360, "ENE-CORPは大規模な侵攻を開始し、", GetColor(255, 255, 255));
		DrawString(200, 420, "瞬く間に世界のあらゆる都市を征服した。", GetColor(255, 255, 255));
		break;

	case StoryState::StoryText8:
		DrawString(200, 300, "風が暖かくて心地いい。", GetColor(255, 255, 255));
		DrawString(200, 360, "もうすぐ生まれるこの子を", GetColor(255, 255, 255));
		DrawString(200, 420, "まるで世界が祝福しているみたい。", GetColor(255, 255, 255));
		break;

	case StoryState::StoryText9:
		DrawString(200, 300, "おはようあなた。", GetColor(255, 255, 255));
		DrawString(200, 360, "今日もいい天気ね。", GetColor(255, 255, 255));
		break;

	case StoryState::StoryText10:
		DrawString(200, 300, "だが、ルーズ卿もまた同じように、", GetColor(255, 255, 255));
		DrawString(200, 360, "臓一族こそが最大の障害になると、そう感じていたのである......", GetColor(255, 255, 255));
		break;

	case StoryState::StoryText11:
		DrawString(200, 300, "樹秘拳に使命を果たし、数々の歴史を守ってきた臓一族。", GetColor(255, 255, 255));
		DrawString(200, 360, "彼らであればENE-CORPの野望を打ち砕くことができるはずだ。", GetColor(255, 255, 255));
		break;

	case StoryState::StoryText12:
		DrawString(200, 300, "そう言えば、今日は道場で", GetColor(255, 255, 255));
		DrawString(200, 360, "弟子たちに稽古をつける日だったわね。", GetColor(255, 255, 255));
		DrawString(200, 420, "私のことは大丈夫だから、いってらっしゃい。", GetColor(255, 255, 255));
		break;

	case StoryState::Complete:
		break;
	}

	SetFontSize(16);
	DrawString(100, 680, "[ENTER]次へ  [ESC]タイトルに戻る", GetColor(150, 150, 150));

#ifdef _DEBUG
	const std::string& debugFont = m_game->GatDebugFont();
	renderer->DrawTextL(Vector2d(m_game->GetWidth() - 150.0f, 0), "StoryScene", Color(255, 64, 0), debugFont, 24, false);
#endif
}

std::string StoryScene::GetStoryText() const {
	switch (m_storyState) {
	case StoryState::StoryText1:
		return "平和な世界に、突如現れた武装組織、「ENE コーポレーション」。";
	case StoryState::StoryText2:
		return "その脅威に立ち向かう力を持ちうるのはジョー・ムサシ率いる忍軍団「臓一族」のみ。";
	case StoryState::StoryText3:
		return "彼は、闇向かう者を圧倒的な力で滅ぼし、世界を恐怖と絶望に陥れてゆく。";
	case StoryState::StoryText4:
		return "すべての始まりには......必ず、終わりがあるのだ。";
	case StoryState::StoryText5:
		return "何人たりとも、その運命から逃れることはできない。";
	case StoryState::StoryText6:
		return "─────死。";
	case StoryState::StoryText7:
		return "その組織を操るリーダーの名は「ルーズ卿」。ENE-CORPは大規模な侵攻を開始し、瞬く間に世界のあらゆる都市を征服した。";
	case StoryState::StoryText8:
		return "風が暖かくて心地いい。もうすぐ生まれるこの子をまるで世界が祝福しているみたい。";
	case StoryState::StoryText9:
		return "おはようあなた。今日もいい天気ね。";
	case StoryState::StoryText10:
		return "だが、ルーズ卿もまた同じように、臓一族こそが最大の障害になると、そう感じていたのである......";
	case StoryState::StoryText11:
		return "樹秘拳に使命を果たし、数々の歴史を守ってきた臓一族。彼らであればENE-CORPの野望を打ち砕くことができるはずだ。";
	case StoryState::StoryText12:
		return "そう言えば、今日は道場で弟子たちに稽古をつける日だったわね。私のことは大丈夫だから、いってらっしゃい。";
	default:
		return "";
	}
}
