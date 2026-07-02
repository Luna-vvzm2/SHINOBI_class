#pragma once
#include <string>
#include <memory>
#include <vector>
#include <set>
#include "Color.h"
#include "BlockActor.h"
#include "Vector2d.h"

class Scene;
class Input;
class EventBase;
class EventTexture;
class EventManager

{
public:
	EventManager(Scene* scene, EventTexture* eventTexture); //コンストラクタ
	~EventManager(); //デストラクタ

	void Init(std::unique_ptr<EventBase> event); //イベント開始
	void Update(float deltaTIme); //更新
	void End(); //イベント終了

	bool IsRunning() const; //イベントが実行中か
	bool IsEventTriggered(int eventId) const;
	void RegisterEvent(int eventId);

	void Draw();

	EventTexture* GetEventTexture() const;

private:
	Scene* m_scene; //イベントを管理するシーン
	EventTexture* m_eventTexture{ nullptr }; //イベントの画像を管理
	std::unique_ptr<EventBase> m_currentEvent{ nullptr }; //実行中のイベント
	std::set<int> m_triggeredEvents;
};


class EventBase
{
public:
	EventBase(); //コンストラクタ
	virtual ~EventBase(); //デストラクタ

	virtual void Init() = 0; //イベント開始時の処理
	virtual void Update(float deltaTime) = 0; //更新
	virtual void End() = 0; //終了

	virtual bool IsEnd() const = 0;

	virtual void Draw() = 0;

protected:
	std::string LoadConfig(const std::string& text, const std::string& configName) const; //テキストから設定を読み取る
};

class TalkEvent : public EventBase
{
public:
	TalkEvent(Scene* scene, const std::string& filePath, EventManager* eventManager); //コンストラクタ
	~TalkEvent() override; //デストラクタ

	void Init() override;
	void Update(float deltaTime) override;
	void End() override;

	bool IsEnd() const override;

	enum class ActorPosition
	{
		None,
		Left,
		Right
	};

private:
	Scene* m_scene{ nullptr };
	EventManager* m_eventManager{ nullptr };
	std::vector<std::string> m_texts; //表示テキスト
	int m_currentLine{ 0 }; //表示中のテキストインデックス
	int m_skipTimer{ 90 }; //スキップの長押しタイマー
	bool m_isEnd{ false }; //イベント終了確認

	std::string m_talkerName{ "" };
	std::string m_talkText{ "" };
	ActorPosition m_talkerPosition{ ActorPosition::None };
	int m_actorTextureId{ -1 }; //立ち絵のid
	int m_actorW{ 480 };
	int m_actorH{ 510 };

	int m_boxX{ 280 };
	int m_boxY{ 510 };
	int m_boxW{ 720 };
	int m_boxH{ 160 };

	int m_nameX{ 0 };
	int m_nameY{ m_boxY - 30 };
	int m_nameX2{ m_boxW + 230 };

	Color m_textColor{ 255, 255, 255, 255 };
	Color m_boxColor{ 0, 0, 0, 180 };
	Color m_nameColor{ 250, 190, 20, 255 };

	void ShowText(); //テキストを描画
	void NextText(); //次のテキストへ

	void Draw() override;
	
	//void PlayVoice();

	void LoadTexts(const std::string& filePath); //テキストファイルの読み込み
	void DeleteTexts(); //テキストデータの解放

}; 


class EventTrigger : public BlockActor
{
public:
	EventTrigger(Scene* scene, const Vector2d& pos, const Vector2d& size, int eventId, EventManager* eventManager);

	void Update(float deltaTime) override;
	void Draw() override {}

	ActorType GetType() const override;

private:
	int m_eventId;
	Vector2d m_size;
	bool m_isTriggered;
	EventManager* m_eventManager;

protected:
	std::string GetTexturePath() const override { return ""; }

};

