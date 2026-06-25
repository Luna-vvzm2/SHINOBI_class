#include "EnemyBullet.h"
#include "Scene.h"
#include "Actor.h"
#include "TransformComponent.h"
#include "VelocityComponent.h"
#include "CollisionComponent.h"
#include "HPComponent.h"

EnemyBullet::EnemyBullet(
	Scene* scene,
	const Vector2d& pos,
	const Vector2d& velocity,
	float deleteRange,
	const std::string& texturePath,
	int damage
)
	: EntityActor(scene, pos, Vector2d(12.0f, 12.0f))
	, m_startPos(pos)
	, m_bulletVelocity(velocity)
	, m_deleteRange(deleteRange)
	, m_texturePath(texturePath)
	, m_damage(damage)
{
}

bool EnemyBullet::Init()
{
	if (!EntityActor::Init()) return false;

	m_velocity->SetVelocity(m_bulletVelocity);
	m_collision->SetCircle(6.0f);

	return true;
}

void EnemyBullet::Update(float deltaTime)
{
	Actor::Update(deltaTime);

	Vector2d pos = m_transform->GetPosition();
	Vector2d vel = m_velocity->GetVelocity();

	pos += vel * deltaTime;
	m_transform->SetPosition(pos);

	if ((pos - m_startPos).length() > m_deleteRange)
	{
		SetState(Actor::State::Dead);
		return;
	}

	if (TryDamagePlayer())
	{
		SetState(Actor::State::Dead);
	}
}

bool EnemyBullet::TryDamagePlayer()
{
	if (m_scene == nullptr)
	{
		return false;
	}

	for (Actor* actor : m_scene->GetActors())
	{
		if (actor == nullptr || actor->GetType() != ActorType::Player || actor->IsDead())
		{
			continue;
		}

		CollisionComponent* playerCollision = actor->GetComponent<CollisionComponent>();
		if (playerCollision == nullptr || m_collision == nullptr)
		{
			return false;
		}

		if (!m_collision->CheckCollision(playerCollision))
		{
			continue;
		}

		HPComponent* playerHp = actor->GetComponent<HPComponent>();
		if (playerHp != nullptr)
		{
			playerHp->Damage(m_damage);
			playerHp->SetInvincible(0.3f);
		}

		return true;
	}

	return false;
}

std::string EnemyBullet::GetTexturePath() const
{
	return m_texturePath;
}
