#include "GunnerEnemyEntity.h"
#include "Scene.h"
#include "Actor.h"
#include "EnemyBullet.h"
#include "VelocityComponent.h"
#include "TransformComponent.h"
#include "SpriteComponent.h"
#include "AnimationComponent.h"
#include "Vector2d.h"

GunnerEnemyEntity::GunnerEnemyEntity(Scene* scene, const Vector2d& pos)
	: EnemyEntity(scene, pos, Vector2d(96, 190))
	, m_bulletCount(0)
	, m_attackOnce{}
	, m_attackDir(1)
	, m_gunnerState(0)
	, m_currentTexturePath("")
{
}

bool GunnerEnemyEntity::Init()
{
	if (!EnemyEntity::Init()) return false;

	m_anim = AddComponent<AnimationComponent>();
	m_anim->SetSprite(m_sprite);

	return true;
}

static const float GUNNER_FIND_RANGE = 800.0f;          // search range
static const float GUNNER_BACK_RANGE = 220.0f;          // back step range
static const float GUNNER_STOP_RANGE = 300.0f;          // stop attack range
static const float GUNNER_ATTACK_RANGE = 900.0f;        // attack range
static const float GUNNER_BULLET_DELETE_RANGE = 900.0f; // bullet delete range

static const float GUNNER_APPROACH_SPEED = 120.0f;      // approach speed
static const float GUNNER_BACK_SPEED = 180.0f;          // back speed
static const float GUNNER_BULLET_SPEED = 250.0f;        // bullet speed
static const float GUNNER_COOLDOWN = 0.4f;              // attack cooldown
static const float GUNNER_RECHECK_TIME = 0.2f;          // state recheck time

static const int GUNNER_SHOT_COUNT = 3;                 // shot count
static const float GUNNER_SHOT_TIME[GUNNER_SHOT_COUNT] = { 0.8f, 1.3f, 1.8f };
static const float GUNNER_ATTACK_END_TIME = 3.2f;       // attack end time

static const char* GUNNER_TEXTURE_IDLE = "assets/images/enemy/gunner/idle.png";
static const char* GUNNER_TEXTURE_WALK = "assets/images/enemy/gunner/walk.png";
static const char* GUNNER_TEXTURE_AIM = "assets/images/enemy/gunner/aim.png";
static const char* GUNNER_TEXTURE_SHOT = "assets/images/enemy/gunner/shot.png";
static const char* GUNNER_TEXTURE_RELOAD = "assets/images/enemy/gunner/reload.png";

float GunnerEnemyEntity::GetDirSign() const
{
	return m_dir ? 1.0f : -1.0f;
}

bool GunnerEnemyEntity::TryGetPlayerInfo(Vector2d& playerPos) const
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

		TransformComponent* transform = actor->GetComponent<TransformComponent>();
		if (transform == nullptr)
		{
			return false;
		}

		playerPos = transform->GetPosition();
		return true;
	}

	return false;
}

void GunnerEnemyEntity::PlayMotion(
	const std::string& motionName,
	const std::string& texturePath,
	int frameCount,
	float frameSpeed,
	bool loop
)
{
	if (m_currentTexturePath == texturePath)
	{
		return;
	}

	if (m_sprite == nullptr || m_anim == nullptr || frameCount <= 0)
	{
		return;
	}

	if (!m_sprite->LoadTextureDiv(texturePath, frameCount, 1))
	{
		return;
	}

	AnimationClip clip;
	for (int i = 0; i < frameCount; i++)
	{
		clip.frames.push_back(i);
	}
	clip.speed = frameSpeed;
	clip.loop = loop;

	m_anim->AddClip(motionName, clip);
	m_anim->Play(motionName, true);
	m_currentTexturePath = texturePath;
}

// Start gun attack.
void GunnerEnemyEntity::StartGunAttack(const Vector2d& playerPos)
{
	m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

	if (playerPos.x > m_transform->GetPosition().x)
	{
		m_attackDir = 1;
		m_dir = true;
	}
	else
	{
		m_attackDir = -1;
		m_dir = false;
	}

	m_attackType = 1;
	m_attackTimer = 0.0f;

	m_attackActive = false;
	PlayMotion("aim", GUNNER_TEXTURE_AIM, 3, 0.12f, false);

	for (int shot = 0; shot < GUNNER_SHOT_COUNT; shot++)
	{
		m_attackOnce[shot] = false;
	}

	m_actionLock = true;
}

void GunnerEnemyEntity::Update(float deltaTime)
{
	if (deltaTime > 0.05f)
	{
		deltaTime = 0.05f;
	}

	Vector2d playerPos = Vector2d::Zero();

	if (!TryGetPlayerInfo(playerPos))
	{
		m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));
		EnemyEntity::Update(deltaTime);
		return;
	}

	Vector2d myPos = m_transform->GetPosition();

	float distanceX = playerPos.x - myPos.x;
	float distance = distanceX;

	if (distance < 0.0f)
	{
		distance *= -1.0f;
	}

	// Cooldown.
	if (m_cooldownTimer > 0.0f)
	{
		m_cooldownTimer -= deltaTime;

		if (m_cooldownTimer < 0.0f)
		{
			m_cooldownTimer = 0.0f;
		}
	}

	// Wait until player is found.
	if (m_findPlayer == false)
	{
		if (distance < GUNNER_FIND_RANGE)
		{
			m_findPlayer = true;
		}
		else
		{
			m_gunnerState = 0;
			PlayMotion("idle", GUNNER_TEXTURE_IDLE, 4, 0.16f, true);
			m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));
			EnemyEntity::Update(deltaTime);
			return;
		}
	}

	if (m_attackType == 0)
	{
		if (m_actionLock == false)
		{
			if (distanceX >= 0.0f)
			{
				m_dir = true;
			}
			else
			{
				m_dir = false;
			}

			/*----------------

			state list
			||status||
			0 wait
			1 approach shot
			2 back shot
			3 stop shot
			4 dead

			attack list
			||attackType||
			0 move
			1 gun attack

			---------------*/

			if (distance < GUNNER_BACK_RANGE)
			{
				m_gunnerState = 2;
			}
			else if (distance < GUNNER_STOP_RANGE)
			{
				m_gunnerState = 3;
			}
			else if (distance < GUNNER_ATTACK_RANGE)
			{
				m_gunnerState = 1;
			}
			else
			{
				m_gunnerState = 1;
			}

			m_actionTimer = 0.0f;
			m_actionLock = true;
		}

		m_actionTimer += deltaTime;

		float dir = GetDirSign();

		switch (m_gunnerState)
		{
		case 0:
			PlayMotion("idle", GUNNER_TEXTURE_IDLE, 4, 0.16f, true);
			m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));
			m_actionLock = false;
			break;

		case 1:
			PlayMotion("walk", GUNNER_TEXTURE_WALK, 4, 0.12f, true);
			m_velocity->SetVelocity(Vector2d(GUNNER_APPROACH_SPEED * dir, 0.0f));

			if (distance < GUNNER_STOP_RANGE)
			{
				m_actionLock = false;
			}

			if (m_cooldownTimer <= 0.0f)
			{
				StartGunAttack(playerPos);
			}
			else if (m_actionTimer >= GUNNER_RECHECK_TIME)
			{
				m_actionLock = false;
			}
			break;

		case 2:
			PlayMotion("walk", GUNNER_TEXTURE_WALK, 4, 0.12f, true);
			m_velocity->SetVelocity(Vector2d(-GUNNER_BACK_SPEED * dir, 0.0f));

			if (distance >= GUNNER_BACK_RANGE)
			{
				m_actionLock = false;
			}

			if (m_cooldownTimer <= 0.0f)
			{
				StartGunAttack(playerPos);
			}
			else if (m_actionTimer >= GUNNER_RECHECK_TIME)
			{
				m_actionLock = false;
			}
			break;

		case 3:
			PlayMotion("idle", GUNNER_TEXTURE_IDLE, 4, 0.16f, true);
			m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

			if (distance < GUNNER_BACK_RANGE || distance >= GUNNER_STOP_RANGE)
			{
				m_actionLock = false;
			}

			if (m_cooldownTimer <= 0.0f)
			{
				StartGunAttack(playerPos);
			}
			else if (m_actionTimer >= GUNNER_RECHECK_TIME)
			{
				m_actionLock = false;
			}
			break;

		case 4:
			SetState(Actor::State::Dead);
			break;
		}
	}
	else if (m_attackType == 1)
	{
		m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

		m_attackTimer += deltaTime;
		float attackTime = m_attackTimer;

		m_attackActive = false;
		PlayMotion("aim", GUNNER_TEXTURE_AIM, 3, 0.12f, false);

		for (int shot = 0; shot < GUNNER_SHOT_COUNT; shot++)
		{
			if (attackTime >= GUNNER_SHOT_TIME[shot] &&
				m_attackOnce[shot] == false)
			{
				Vector2d bulletPos(
					myPos.x + 48.0f * static_cast<float>(m_attackDir),
					myPos.y - 16.0f
				);
				Vector2d bulletVel(
					GUNNER_BULLET_SPEED * static_cast<float>(m_attackDir),
					0.0f
				);

				m_scene->SpawnActor(
					new EnemyBullet(
						m_scene,
						bulletPos,
						bulletVel,
						GUNNER_BULLET_DELETE_RANGE,
						"assets/images/enemy/bullet/gunner.png"
					)
				);

				m_bulletCount++;
				m_attackOnce[shot] = true;
				m_attackActive = true;
				PlayMotion("shot", GUNNER_TEXTURE_SHOT, 3, 0.06f, false);
			}
		}

		if (attackTime >= GUNNER_SHOT_TIME[2])
		{
			PlayMotion("reload", GUNNER_TEXTURE_RELOAD, 4, 0.14f, true);
		}

		if (attackTime >= GUNNER_ATTACK_END_TIME)
		{
			m_attackType = 0;

			m_cooldownTimer = GUNNER_COOLDOWN;

			m_actionTimer = 0.0f;
			m_actionLock = false;
			m_attackActive = false;

			for (int shot = 0; shot < GUNNER_SHOT_COUNT; shot++)
			{
				m_attackOnce[shot] = false;
			}
		}
	}

	EnemyEntity::Update(deltaTime);
}

std::string GunnerEnemyEntity::GetTexturePath() const
{
	return GUNNER_TEXTURE_IDLE;
}
