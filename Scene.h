#pragma once
#include <vector>
#include <algorithm>
#include <iostream>
#include "Actor.h"
#include "CollisionComponent.h"

class Game;
class Actor;

class Scene
{
public:
	/*
	Scene / Actor 管理メモ

	1.	Scene と Actor の親子関係である。
		- Scene が破棄されると全 Actor が delete される。
	2.	Actor の追加は AddActor(Actor*) で行う。
	*/

	enum class Type {
		Title,
		Play,
		Story,

	};

	explicit Scene(class Game* game);
	virtual ~Scene();

	virtual bool Init() = 0;

	virtual void Update(float deltaTime) = 0;
	virtual void Draw() = 0;

	Game* GetGame() { return m_game; }
	bool IsRunning() const { return m_isRunning; }
	Type GetType() const { return m_type; }
	const std::vector<Actor*>& GetActors() const { return m_actors; }

	void SpawnActor(Actor* actor) { AddActor(actor); }



protected:
	void AddActor(Actor* actor);
	void AddUIActor(Actor* UIactor);
	void AddBackActor(Actor* backactor);
	void RemoveDeadActors();   // 死んだActorを削除
	void ReleaseAllActors();  // 全Actorを削除

	Game* m_game;
	bool m_isRunning;
	Type m_type;
	std::vector<Actor*> m_actors;  // ポインタで保持
	std::vector<Actor*> m_UIactors;  // ポインタで保持
	std::vector<Actor*> m_backactors;

	std::vector<Segment> m_outerSegments;
};
