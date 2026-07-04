#include <fstream>
#include "EventManager.h"
#include "EventTexture.h"
#include "Scene.h"
#include "Input.h"
#include "PlayScene.h"
#include "PlayerEntity.h"
#include "Game.h"



EventManager::EventManager(Scene* scene, EventTexture* eventTexture)
	: m_scene(scene)
	, m_eventTexture(eventTexture)
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

EventTexture* EventManager::GetEventTexture() const
{
	return m_eventTexture;
}

EventBase::EventBase() = default;
EventBase::~EventBase() = default;

std::string EventBase::LoadConfig(const std::string& text, const std::string& configName) const
{
	size_t pos = text.find(configName + "=\"");
	if (pos == std::string::npos) return "";
	size_t start = pos + configName.length() + 2;
	size_t end = text.find("\"", start);
	if (end == std::string::npos) return "";
	return text.substr(start, end - start);
}

TalkEvent::TalkEvent(Scene* scene, const std::string& filePath, EventManager* eventManager)
	: m_scene(scene)
	, m_eventManager(eventManager)
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

	if (m_eventManager)
	{
		EventTexture* texManager = m_eventManager->GetEventTexture();

		if (texManager)
		{
			for (const std::string& text : m_texts)
			{
				size_t configEndPos = text.find(']');
				if (!text.empty() && text.front() == '[' && configEndPos != std::string::npos)
				{
					std::string configPart = text.substr(1, configEndPos - 1);

					std::string imgPath = LoadConfig(configPart, "path");

					if (!imgPath.empty() && imgPath != "None")
					{
						texManager->LoadTexture(imgPath);
					}
				}
			}
		}

		m_currentLine = 0;
		m_isEnd = false;
		ShowText();
	}
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
	 
	if (m_eventManager)
	{
		EventTexture* texManager = m_eventManager->GetEventTexture();
		if (texManager)
		{
			texManager->Clear();
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

	size_t configEndPos = text.find(']'); 

	if (text.front() == '[' && configEndPos != std::string::npos)
	{
		std::string configPart = text.substr(1, configEndPos - 1);
		m_talkText = text.substr(configEndPos + 1);

		std::string nameStr = LoadConfig(configPart, "name");
		if (!nameStr.empty() && nameStr != "None")
		{
			m_talkerName = nameStr;
		}

		std::string imgPath = LoadConfig(configPart, "path");
		if (!imgPath.empty() && imgPath != "None")
		{
			if (m_eventManager && m_eventManager->GetEventTexture())
			{
				m_actorTextureId = m_eventManager->GetEventTexture()->LoadTexture(imgPath);
			}

			std::string wStr = LoadConfig(configPart, "w");
			std::string hStr = LoadConfig(configPart, "h");

			if (!wStr.empty() && wStr != "None" && std::isdigit(wStr[0]))
			{
				m_actorW = std::stoi(wStr);
			}
			else
			{
				m_actorW = 480;
			}

			if (!hStr.empty() && hStr != "None" && std::isdigit(hStr[0]))
			{
				m_actorH = std::stoi(hStr);
			}
			else
			{
				m_actorH = 510;
			}
		}

		std::string timeStr = LoadConfig(configPart, "time");
		if (!timeStr.empty() && timeStr != "None")
		{
			float displayTime = stof(timeStr);
		}

		std::string posStr = LoadConfig(configPart, "pos");
		if (!posStr.empty() && posStr != "None")
		{
			if (posStr == "R")
			{
				m_talkerPosition = ActorPosition::Right;
			}
			else if (posStr == "L")
			{
				m_talkerPosition = ActorPosition::Left;
			}
		}
	}
	else
	{
		//設定がない場合設定を保持して次のセリフを表示
		m_talkText = text;
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
	if (m_actorTextureId != -1)
	{
		if (m_talkerPosition == ActorPosition::Left)
		{
			int actorX = m_boxX - 220;
			int actorY = (m_boxY + m_boxH) - m_actorH + 50;
			DrawExtendGraph(actorX, actorY, actorX + m_actorW, actorY + m_actorH, m_actorTextureId, TRUE);
		}
		else if (m_talkerPosition == ActorPosition::Right) 
		{
			int actorX = (m_boxX + m_boxW) - m_actorW + 250;
			int actorY = (m_boxY + m_boxH) - m_actorH + 50;
			DrawExtendGraph(actorX, actorY, actorX + m_actorW, actorY + m_actorH, m_actorTextureId, TRUE);
		}

	}

	int textX = m_boxX + 30;
	int textY = m_boxY + 30;

	if (m_actorTextureId > 0 && m_talkerPosition == ActorPosition::Left)
	{
		textX += (m_actorW - 150);
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
	:BlockActor(scene)
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
			auto talkEvent = std::make_unique<TalkEvent>(m_scene, filePath, m_eventManager);

			m_eventManager->Init(std::move(talkEvent));
		}

		SetState(Actor::State::Dead);
	}

	
}

ActorType EventTrigger::GetType() const
{
	return ActorType::Block;
}