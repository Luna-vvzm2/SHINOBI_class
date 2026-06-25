#include "ArmorEnemyEntity.h"
#include "Scene.h"
#include "Actor.h"
#include "VelocityComponent.h"
#include "TransformComponent.h"
#include "HPComponent.h"
#include "SpriteComponent.h"
#include "AnimationComponent.h"
#include "Vector2d.h"

ArmorEnemyEntity::ArmorEnemyEntity(Scene* scene, const Vector2d& pos)
	: EnemyEntity(scene, pos, Vector2d(96, 190))
	, m_attackOnce(false)
	, m_armorState(0)
	, m_guardRecoverTimer(0.0f)
	, m_currentTexturePath("")
{
}

bool ArmorEnemyEntity::Init()
{
	if (!EnemyEntity::Init()) return false;

	m_guardMax = 100;
	m_guard = m_guardMax;

	m_anim = AddComponent<AnimationComponent>();
	m_anim->SetSprite(m_sprite);

	return true;
}

static const float ARMOR_FIND_RANGE = 800.0f;          // search range
static const float ARMOR_MOVE_SPEED = 120.0f;          // move speed
static const float ARMOR_COOLDOWN = 0.4f;              // attack cooldown
static const float ARMOR_ATTACK_RANGE = 120.0f;        // attack x range
static const float ARMOR_ATTACK_HEIGHT_RANGE = 90.0f;  // attack y range
static const int ARMOR_ATTACK_DAMAGE = 10;             // attack damage
static const float ARMOR_GUARD_RECOVER_TIME = 5.0f;    // guard recover seconds

static const float ARMOR_PRE_TIME = 0.5f;              // pre motion
static const float ARMOR_ATTACK_TIME = 0.3f;           // active motion
static const float ARMOR_AFTER_TIME = 0.7f;            // after motion
static const float ARMOR_RECHECK_TIME = 0.2f;          // state recheck time

static const char* ARMOR_TEXTURE_IDLE = "assets/images/enemy/armor/idle.png";
static const char* ARMOR_TEXTURE_WALK = "assets/images/enemy/armor/walk.png";
static const char* ARMOR_TEXTURE_ATTACK_PRE = "assets/images/enemy/armor/attack_pre.png";
static const char* ARMOR_TEXTURE_ATTACK = "assets/images/enemy/armor/attack.png";
static const char* ARMOR_TEXTURE_ATTACK_AFTER = "assets/images/enemy/armor/attack_after.png";

float ArmorEnemyEntity::GetDirSign() const
{
	return m_dir ? 1.0f : -1.0f;
}

bool ArmorEnemyEntity::TryGetPlayerInfo(Vector2d& playerPos, HPComponent*& playerHp) const
{
	playerHp = nullptr;

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
		playerHp = actor->GetComponent<HPComponent>();
		return true;
	}

	return false;
}

void ArmorEnemyEntity::UpdateGuardRecover(float deltaTime)
{
	if (m_guard > 0)
	{
		m_guardRecoverTimer = 0.0f;
		return;
	}

	m_guardRecoverTimer += deltaTime;

	if (m_guardRecoverTimer >= ARMOR_GUARD_RECOVER_TIME)
	{
		m_guard = m_guardMax;
		m_guardRecoverTimer = 0.0f;
		m_damageCancel = false;
	}
}

void ArmorEnemyEntity::PlayMotion(
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

// Start armor punch attack.
void ArmorEnemyEntity::StartArmorsAttack()
{
	m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

	m_attackType = 1;
	m_attackTimer = 0.0f;

	m_attackActive = false;
	m_attackOnce = false;
	PlayMotion("attack_pre", ARMOR_TEXTURE_ATTACK_PRE, 3, 0.12f, false);
	m_actionLock = true;
}

void ArmorEnemyEntity::Update(float deltaTime)
{
	if (deltaTime > 0.05f)
	{
		deltaTime = 0.05f;
	}

	UpdateGuardRecover(deltaTime);

	Vector2d playerPos = Vector2d::Zero();
	HPComponent* playerHp = nullptr;

	if (!TryGetPlayerInfo(playerPos, playerHp))
	{
		m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));
		EnemyEntity::Update(deltaTime);
		return;
	}

	Vector2d myPos = m_transform->GetPosition();

	float distanceX = playerPos.x - myPos.x;
	float distanceY = playerPos.y - myPos.y;
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
		if (distance < ARMOR_FIND_RANGE)
		{
			m_findPlayer = true;
		}
		else
		{
			m_armorState = 0;
			PlayMotion("idle", ARMOR_TEXTURE_IDLE, 4, 0.16f, true);
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
			1 approach
			2 attack
			4 dead

			attack list
			||attackType||
			0 move / non attack
			1 punch attack

			---------------*/

			if (distance < ARMOR_ATTACK_RANGE)
			{
				m_armorState = 2;
			}
			else if (distance < ARMOR_FIND_RANGE)
			{
				m_armorState = 1;
			}

			m_actionTimer = 0.0f;
			m_actionLock = true;
		}

		m_actionTimer += deltaTime;

		float dir = GetDirSign();

		switch (m_armorState)
		{
		case 0:
			PlayMotion("idle", ARMOR_TEXTURE_IDLE, 4, 0.16f, true);
			m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));
			m_actionLock = false;
			break;

		case 1:
			PlayMotion("walk", ARMOR_TEXTURE_WALK, 4, 0.12f, true);
			m_velocity->SetVelocity(Vector2d(ARMOR_MOVE_SPEED * dir, 0.0f));

			if (distance < ARMOR_ATTACK_RANGE)
			{
				m_actionLock = false;
			}
			break;

		case 2:
			PlayMotion("idle", ARMOR_TEXTURE_IDLE, 4, 0.16f, true);
			m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

			if (m_cooldownTimer <= 0.0f)
			{
				StartArmorsAttack();
			}
			else if (m_actionTimer >= ARMOR_RECHECK_TIME)
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

		const float ATTACK_START = ARMOR_PRE_TIME;
		const float ATTACK_END = ARMOR_PRE_TIME + ARMOR_ATTACK_TIME;
		const float END_TIME = ARMOR_PRE_TIME + ARMOR_ATTACK_TIME + ARMOR_AFTER_TIME;

		if (attackTime < ATTACK_START)
		{
			PlayMotion("attack_pre", ARMOR_TEXTURE_ATTACK_PRE, 3, 0.12f, false);
			m_attackActive = false;
		}
		else if (attackTime < ATTACK_END)
		{
			PlayMotion("attack", ARMOR_TEXTURE_ATTACK, 3, 0.08f, false);
			m_attackActive = true;

			if (m_attackOnce == false)
			{
				float dx = playerPos.x - myPos.x;
				float dy = distanceY;

				bool hit = false;

				if (m_dir == true)
				{
					if (dx > 0.0f && dx < ARMOR_ATTACK_RANGE)
					{
						hit = true;
					}
				}
				else
				{
					if (dx < 0.0f && dx > -ARMOR_ATTACK_RANGE)
					{
						hit = true;
					}
				}

				if (hit == true)
				{
					if (dy < 0.0f)
					{
						dy *= -1.0f;
					}

					if (dy < ARMOR_ATTACK_HEIGHT_RANGE && playerHp != nullptr)
					{
						playerHp->Damage(ARMOR_ATTACK_DAMAGE);
						playerHp->SetInvincible(0.3f);
					}
				}

				m_attackOnce = true;
			}
		}
		else if (attackTime < END_TIME)
		{
			PlayMotion("attack_after", ARMOR_TEXTURE_ATTACK_AFTER, 3, 0.12f, false);
			m_attackActive = false;
		}
		else
		{
			m_attackType = 0;
			m_attackActive = false;

			m_cooldownTimer = ARMOR_COOLDOWN;
			m_actionTimer = 0.0f;
			m_actionLock = false;
			m_attackOnce = false;
		}
	}

	EnemyEntity::Update(deltaTime);
}

std::string ArmorEnemyEntity::GetTexturePath() const
{
	return ARMOR_TEXTURE_IDLE;
}
