#include "Game.h"
#include <sstream>
#include <iomanip>
#include "TitleScene.h"
#include "PlayScene.h"
#include "SpriteComponent.h"

Game::Game()
	: m_window(nullptr),
	m_winWidth(0),
	m_winHeight(0),
	m_winColor(0),
	m_fps(0.0f),
	m_freq{},
	m_prevTime{},
	m_running(true),
	m_ended(false)
{
}

Game::~Game() {
	End();
}

bool Game::Init(const std::string& title, UINT width, UINT height, UINT color, const std::string& gamefont, const std::string& debugfont) {
	//	ウィンドウモードに変更
	ChangeWindowMode(TRUE);

	//	画面モード設定
	SetGraphMode(width, height, color);
	SetBackgroundColor(0, 0, 0);
	SetWaitVSyncFlag(FALSE);

	// ウィンドウタイトル設定
	SetWindowTextA(m_window, title.c_str());
	//	DirectInput系コントローラなどXInput系を使う設定
	SetUseXInputFlag(TRUE);
	//	初期化
	//	DxLib初期化
	if (DxLib_Init() != 0) {
		std::cerr << "DxLib初期化失敗\n";
		MessageBoxA(m_window, "DxLib初期化失敗", "エラー", MB_OK);
		return false;
	}

	m_window = GetMainWindowHandle();
	//	Renderer初期化
	m_renderer = std::make_unique<Renderer>(m_window, width, height);
	if (!m_renderer) {
		std::cerr << "Renderer初期化失敗\n";
		MessageBoxA(m_window, "Renderer初期化失敗", "エラー", MB_OK);
		return false;
	}

	m_input.Init();

	SetDrawScreen(DX_SCREEN_BACK);
	m_winWidth = width;
	m_winHeight = height;
	m_winColor = color;
	m_gameFont = gamefont;
	m_debugFont = debugfont;
	m_running = true;

	m_scene = std::make_unique<TitleScene>(this);
	if (!m_scene->Init()) {
		std::cerr << "TitleScene 初期化失敗" << std::endl;
		return false;
	}

	QueryPerformanceFrequency(&m_freq);
	QueryPerformanceCounter(&m_prevTime);

	std::cout << "アプリ初期化成功\n";
	return true;
}

bool Game::Run() {
	while (ProcessMessage() == 0 && m_running) {
		float deltaTime = 0.0f;
		if (tick(deltaTime, 60)) {
			m_fps = 1.0f / deltaTime;
			m_input.Update();

			Update(deltaTime);
			Draw();
			if (!m_scene) return false;
			if (!m_scene->IsRunning()) return false;
		}
	}
	return m_scene->IsRunning();
}

void Game::Update(float deltaTime) {
	// ★修正：一画的な一律キー遷移を削除し、シーン独自のメニュー入力を活かせるようにしました
	// シーン更新
	if (m_scene) { m_scene->Update(deltaTime); }
}

void Game::Draw() {
	ClearDrawScreen();
	if (m_scene) m_scene->Draw();

#ifdef _DEBUG
	Vector2d ls = m_input.GetPad().GetStickL();
	Vector2d rs = m_input.GetPad().GetStickR();
	// FPS描画
	std::vector<NumberInfo> GameInfo = {
		{ m_fps, 2 },
		{ ls.x, 1 },
		{ ls.y, 1 },
		{ rs.x, 1 },
		{ rs.y, 1 }
	};
	m_renderer->DrawNumberFormatW(Vector2d(m_winWidth - 150.0f, 0), Color(255, 255, 0), m_debugFont, 20, "\nFPS: {0}\nLX: {1}\nLY: {2}\nRX: {3}\nRY: {4}", GameInfo, false);

	if (m_input.GetPad().IsDown(Joypad::UP)) m_renderer->DrawTextL(Vector2d(150.0f, 0), "UP", Color(255, 64, 0), m_debugFont, 24, false);
	if (m_input.GetPad().IsDown(Joypad::DOWN)) m_renderer->DrawTextL(Vector2d(150.0f, 0), "DOWN", Color(255, 64, 0), m_debugFont, 24, false);
	if (m_input.GetPad().IsDown(Joypad::LEFT)) m_renderer->DrawTextL(Vector2d(150.0f, 0), "LEFT", Color(255, 64, 0), m_debugFont, 24, false);
	if (m_input.GetPad().IsDown(Joypad::RIGHT)) m_renderer->DrawTextL(Vector2d(150.0f, 0), "RIGHT", Color(255, 64, 0), m_debugFont, 24, false);
	if (m_input.GetPad().IsDown(Joypad::A)) m_renderer->DrawTextL(Vector2d(150.0f, 0), "A", Color(255, 64, 0), m_debugFont, 24, false);
	if (m_input.GetPad().IsDown(Joypad::B)) m_renderer->DrawTextL(Vector2d(150.0f, 0), "B", Color(255, 64, 0), m_debugFont, 24, false);
	if (m_input.GetPad().IsDown(Joypad::X)) m_renderer->DrawTextL(Vector2d(150.0f, 0), "X", Color(255, 64, 0), m_debugFont, 24, false);
	if (m_input.GetPad().IsDown(Joypad::Y)) m_renderer->DrawTextL(Vector2d(150.0f, 0), "Y", Color(255, 64, 0), m_debugFont, 24, false);
	if (m_input.GetPad().IsDown(Joypad::LB)) m_renderer->DrawTextL(Vector2d(150.0f, 0), "LB", Color(255, 64, 0), m_debugFont, 24, false);
	if (m_input.GetPad().IsDown(Joypad::RB)) m_renderer->DrawTextL(Vector2d(150.0f, 0), "RB", Color(255, 64, 0), m_debugFont, 24, false);
	if (m_input.GetPad().IsDown(Joypad::BACK)) m_renderer->DrawTextL(Vector2d(150.0f, 0), "BACK", Color(255, 64, 0), m_debugFont, 24, false);
	if (m_input.GetPad().IsDown(Joypad::START)) m_renderer->DrawTextL(Vector2d(150.0f, 0), "START", Color(255, 64, 0), m_debugFont, 24, false);
	if (m_input.GetPad().IsDown(Joypad::LSB)) m_renderer->DrawTextL(Vector2d(150.0f, 0), "LSB", Color(255, 64, 0), m_debugFont, 24, false);
	if (m_input.GetPad().IsDown(Joypad::RSB)) m_renderer->DrawTextL(Vector2d(150.0f, 0), "RSB", Color(255, 64, 0), m_debugFont, 24, false);

	if (!m_renderer) return;

	std::vector<NumberInfo> stickIndex = {
		{ m_input.GetPad().GetStickLT(), 1},
		{ m_input.GetPad().GetStickRT(), 1 }
	};
	m_renderer->DrawNumberFormatW(Vector2d(m_winWidth - 150.0f, 110), Color(255, 255, 0), m_debugFont, 20, "\nLT: {0}\nRT: {1} ", stickIndex, false);
#endif // _DEBUG

	ScreenFlip();
}

void Game::ChangeScene(Scene::Type type) {
	m_scene.reset(); // 現在のシーンを安全に破棄

	if (type == Scene::Type::Title) {
		m_scene = std::make_unique<TitleScene>(this);
	}
	else if (type == Scene::Type::Play) {
		m_scene = std::make_unique<PlayScene>(this);
	}

	if (m_scene) {
		m_scene->Init(); // 新しいシーンの初期化を実行
	}
}

void Game::End() {
	if (m_ended) return;
	m_ended = true;

	if (m_scene) { m_scene.reset(); }
	if (m_renderer) { m_renderer.reset(); }
	DxLib_End();	//	DxLib終了処理
	std::cout << "アプリ終了" << std::endl;
}

void Game::InitConsole() {
	if (GetConsoleWindow()) return;
	AllocConsole();
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONOUT$", "w", stderr);
	freopen_s(&fp, "CONIN$", "r", stdin);
	SetConsoleOutputCP(932);
	std::cout << "コンソール初期化完了" << std::endl;
}

bool Game::tick(float& deltaTime, int targetFPS, float maxDeltaTime) {
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);

	deltaTime = float(currentTime.QuadPart - m_prevTime.QuadPart) / float(m_freq.QuadPart);

	float minDelta = 1.0f / targetFPS;
	if (deltaTime < minDelta) return false;

	if (deltaTime > maxDeltaTime) deltaTime = maxDeltaTime;

	m_prevTime = currentTime;
	return true;
}