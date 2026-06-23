#include <fstream>
#include "EventManager.h"
#include "Scene.h"
#include "Input.h"
#include "PlayScene.h"
#include "PlayerEntity.h"
#include "Game.h"


EventManager::EventManager(Scene* scene)
	: m_scene(scene)
{
}

EventManager::~EventManager() = default;

void EventManager::Init(std::unique_ptr<EventBase> event)
{

	if (!event) return;
	m_currentEvent = std::move(event);
	m_currentEvent->Init();
}

void EventManager::Update(float deltaTime)
{
	if (!m_currentEvent) return;
	m_currentEvent->Update(deltaTime);
	if (m_currentEvent->IsEnd())
	{
		m_currentEvent->End();
		m_currentEvent.reset();
	}
}

void EventManager::End()
{
	if (m_currentEvent)
	{
		m_currentEvent->End();
		m_currentEvent.reset();
	}
}

bool EventManager::IsRunning() const
{
	return m_currentEvent != nullptr;
}

bool EventManager::IsEventTriggered(int eventId) const
{
	return m_triggeredEvents.find(eventId) != m_triggeredEvents.end();
}

void EventManager::RegisterEvent(int eventId)
{
	m_triggeredEvents.insert(eventId);
}

void EventManager::Draw()
{
	m_currentEvent->Draw();
}

EventBase::EventBase() = default;
EventBase::~EventBase() = default;

TalkEvent::TalkEvent(Scene* scene, const std::string& filePath)
	: m_scene(scene)
{
	LoadTexts(filePath);
}

TalkEvent::~TalkEvent() = default;

void TalkEvent::Init()
{
	for (auto actor : m_scene->GetActors())
	{
		if (actor->GetType() == ActorType::Player)
		{
			auto p = static_cast<PlayerEntity*>(actor);
			p->SetCanMove(false);
		}
		actor->SetState(Actor::State::Paused);
	}
	m_currentLine = 0;
	m_isEnd = false;
	ShowText();
}

void TalkEvent::Update(float deltaTime)
{
	const Input& input = m_scene->GetGame()->GetInput();
	if (input.IsTrigger(Action::ENTER))
	{
		NextText();
	}
	if (input.IsDown(Action::ESCAPE)) //スキップ処理
	{
		if (input.GetPressFrame(Action::ESCAPE) >= m_skipTimer)
		{
			m_isEnd = true;
		}
	}
}

void TalkEvent::End()
{
	for (auto actor : m_scene->GetActors())
	{
		actor->SetState(Actor::State::Active);
		if (actor->GetType() == ActorType::Player)
		{
			auto p = static_cast<PlayerEntity*>(actor);
			p->SetCanMove(true);
		}
	}
	DeleteTexts();
}

bool TalkEvent::IsEnd() const
{
	return m_isEnd;
}

void TalkEvent::ShowText()
{
	if (m_currentLine < 0 || m_currentLine >= static_cast<int>(m_texts.size())) return;

	const std::string& text = m_texts[m_currentLine];

	size_t separatorPos = text.find(':'); //コロンで名前とセリフを分割

	if (separatorPos == std::string::npos) return;

	std::string header = text.substr(0, separatorPos);
	m_talkText = text.substr(separatorPos + 1);

	size_t slashPos = header.find('/'); //スラッシュでL,Rを識別
	if (slashPos != std::string::npos)
	{
		m_talkerName = header.substr(0, slashPos);
		std::string positionStr = header.substr(slashPos + 1);
		if (positionStr == "R")
		{
			m_talkerPosition = ActorPosition::Right;
		}
		else if(positionStr == "L")
		{
			m_talkerPosition = ActorPosition::Left;
		}
	}
	else
	{
		m_talkerName = header;
		m_talkerPosition = ActorPosition::Left;
	}

	//改行処理
	std::string target = "<br>";
	size_t pos = m_talkText.find(target);
	while (pos != std::string::npos)
	{
		m_talkText.replace(pos, target.length(), "\n");
		pos = m_talkText.find(target, pos + 1);
	}

}

void TalkEvent::NextText()
{
	m_currentLine++;
	if (m_currentLine >= static_cast<int>(m_texts.size()))
	{
		m_isEnd = true;
	}
	else
	{
		ShowText();
	}
}

void TalkEvent::Draw()
{
	//テキストボックス描画
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_boxColor.a);
	DrawBox(m_boxX, m_boxY, m_boxW + m_boxX, m_boxH + m_boxY, m_boxColor.ToDxColor(), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	//テキストボックスの枠線描画
	DrawBox(m_boxX, m_boxY, m_boxW + m_boxX, m_boxH + m_boxY, m_nameColor.ToDxColor(), FALSE);

	//立ち絵の描画
	if (m_actorTextureId > 0)
	{
		if (m_talkerPosition == ActorPosition::Left)
		{
			int actorX = m_boxX + 20;
			int actorY = (m_boxY + m_boxH) - m_actorH + 30;
			DrawGraph(actorX, actorY, m_actorTextureId, TRUE);
		}
		else if (m_talkerPosition == ActorPosition::Right)
		{
			int actorX = (m_boxX + m_boxW) - m_actorW - 20;
			int actorY = (m_boxY + m_boxH) - m_actorH + 30;
			DrawGraph(actorX, actorY, m_actorTextureId, TRUE);
		}

	}

	int textX = m_boxX + 30;
	int textY = m_boxY + 30;

	if (m_actorTextureId > 0 && m_talkerPosition == ActorPosition::Left)
	{
		textX += (m_actorW + 20);
	}

	DrawString(textX, textY, m_talkText.c_str(), m_textColor.ToDxColor());

	if (m_talkerName.empty()) return;
	
	if (m_talkerPosition == ActorPosition::Right)
	{
		m_nameX = m_boxX;
	}
	else
	{
		m_nameX = m_nameX2;
	}

	DrawString(m_nameX, m_nameY, m_talkerName.c_str(), m_nameColor.ToDxColor());
}

void TalkEvent::LoadTexts(const std::string& filePath)
{
	DeleteTexts();

	std::ifstream ifs(filePath);

	if (!ifs.is_open())
	{
		std::cerr << "テキストファイルを読み込めませんでした\n";
		return;
	}

	std::string line;

	while (std::getline(ifs, line))
	{
		if (line.empty() || line.rfind("//", 0) == 0 ) continue; //空の行とコメント行を無視
		m_texts.push_back(line);
	}
	ifs.close();
}

void TalkEvent::DeleteTexts()
{
	m_texts.clear();
	//m_texts.shrink_to_fit();
}

EventTrigger::EventTrigger(Scene* scene, const Vector2d& pos, const Vector2d& size, int eventId, EventManager* eventManager)
	:Actor(scene)
	,m_eventId(eventId)
	,m_size(size)
	,m_isTriggered(false)
	,m_eventManager(eventManager)
{
	AddComponent<TransformComponent>();
	GetComponent<TransformComponent>()->SetPosition(pos);
}

void EventTrigger::Update(float deltaTime)
{
	if (m_isTriggered) return;


	PlayerEntity* player = nullptr;
	for (auto actor : m_scene->GetActors())
	{
		if (actor->GetType() == ActorType::Player)
		{
			player = static_cast<PlayerEntity*>(actor);
			break;
		}
	}
	if (!player) return;
	

	Vector2d playerPos = player->GetComponent<TransformComponent>()->GetPosition();
	Vector2d myPos = GetComponent<TransformComponent>()->GetPosition();

	if (playerPos.x >= myPos.x && playerPos.x <= myPos.x + m_size.x && playerPos.y >= myPos.y && playerPos.y <= myPos.y + m_size.y)
	{
		m_isTriggered = true;

		if (m_eventManager)
		{
			if (m_eventManager->IsEventTriggered(m_eventId))
			{
				SetState(Actor::State::Dead);
				return;
			}

			if (m_eventManager->IsRunning())
			{
				m_isTriggered = false; 
				return;
			}

			m_eventManager->RegisterEvent(m_eventId); //重複を避ける

			std::string filePath = "assets/events/event_" + std::to_string(m_eventId) + ".txt";
			auto talkEvent = std::make_unique<TalkEvent>(m_scene, filePath);

			m_eventManager->Init(std::move(talkEvent));
		}

		SetState(Actor::State::Dead);
	}

	
}

ActorType EventTrigger::GetType() const
{
	return ActorType::UI;
}