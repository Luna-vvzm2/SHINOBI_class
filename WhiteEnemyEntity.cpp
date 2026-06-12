#include "WhiteEnemyEntity.h"
#include "Scene.h"
#include "Actor.h"
#include "PlayerEntity.h"
#include "EnemyBullet.h"
#include "VelocityComponent.h"
#include "TransformComponent.h"
#include "HPComponent.h"
#include "SpriteComponent.h"
#include "AnimationComponent.h"
#include "Vector2d.h"

WhiteEnemyEntity::WhiteEnemyEntity(Scene* scene, const Vector2d& pos)
	: EnemyEntity(scene, pos, Vector2d(96, 190))
	, m_bulletCount(0)
	, m_attackOnce(false)
	, m_whiteState(0)
	, m_currentTexturePath("")
	, m_currentMotionName("")
	, m_chasePlayer(true)
	, m_hasLastMove(false)
	, m_lastWantedMoveX(0.0f)
	, m_wallStopTimer(0.0f)
	, m_lastMoveStartPos(pos)
{
}

bool WhiteEnemyEntity::Init()
{
	if (!EnemyEntity::Init()) return false;

	m_sprite->SetDrawSize(96.0f, 190.0f);

	m_anim = AddComponent<AnimationComponent>();
	m_anim->SetSprite(m_sprite);

	return true;
}

//
static const float WHITE_TILE_SIZE = 104.0f;
static const float WHITE_ENEMY_HALF_WIDTH = 48.0f;
static const float WHITE_PLAYER_HALF_WIDTH = 42.5f;
static const float WHITE_SWORD_GAP_RANGE = WHITE_TILE_SIZE * 0.7f;
static const float WHITE_FIND_RANGE = 800.0f;//
static const float WHITE_NEAR_SWORD_RANGE = WHITE_ENEMY_HALF_WIDTH + WHITE_PLAYER_HALF_WIDTH + WHITE_SWORD_GAP_RANGE;//
static const float WHITE_BACK_RANGE = 450.0f;//
static const float WHITE_STOP_SHURIKEN_RANGE = 612.0f;//
static const float WHITE_SHURIKEN_RANGE = 650.0f;//
static const float WHITE_BULLET_DELETE_RANGE = 1500.0f;//

static const float WHITE_APPROACH_SPEED = 120.0f;//
static const float WHITE_BACK_SPEED = 180.0f;//
static const float WHITE_FAR_APPROACH_SPEED = 250.0f;//
static const float WHITE_BULLET_SPEED = 250.0f;//

static const float WHITE_SAME_FLOOR_Y_RANGE = WHITE_TILE_SIZE * 0.5f;
static const float WHITE_STUCK_MOVE_EPS = 1.0f;
static const float WHITE_STUCK_IDLE_TIME = 0.25f;

static const float WHITE_ACTION_TIME_SCALE = 1.5f;

static const float WHITE_SHURIKEN_COOLDOWN = 0.45f * WHITE_ACTION_TIME_SCALE;//
static const float WHITE_SWORD_COOLDOWN = 0.25f * WHITE_ACTION_TIME_SCALE;//
static const float WHITE_RECHECK_TIME = 0.2f * WHITE_ACTION_TIME_SCALE;//

static const float WHITE_SWORD_ATTACK_RANGE = WHITE_BACK_RANGE;//
static const float WHITE_SWORD_HEIGHT_RANGE = 80.0f;//
static const int WHITE_SWORD_DAMAGE = 10;//
static const int WHITE_SHURIKEN_DAMAGE = 5;//

static const float WHITE_SHURIKEN_SHOT_TIME = 16.0f / 24.0f; // shuriken shot time
static const float WHITE_SHURIKEN_END_TIME = 24.0f / 24.0f;  // shuriken end time

static const float WHITE_SWORD_HIT_TIME = 18.0f / 24.0f; // sword hit timing
static const float WHITE_SWORD_END_TIME = 24.0f / 24.0f; // sword end timing

static const char* WHITE_TEXTURE_IDLE = "assets/images/enemy/white/idle.png";
static const char* WHITE_TEXTURE_WALK = "assets/images/enemy/white/walk.png";
static const char* WHITE_TEXTURE_SHURIKEN = "assets/images/enemy/white/shuriken.png";
static const char* WHITE_TEXTURE_SWORD_PRE = "assets/images/enemy/white/sword_pre.png";
static const char* WHITE_TEXTURE_SWORD = "assets/images/enemy/white/sword.png";
static const char* WHITE_TEXTURE_SWORD_AFTER = "assets/images/enemy/white/sword_after.png";
static const char* WHITE_TEXTURE_SHEET = "assets/images/enemy/white/White.png";
static const int WHITE_SHEET_X_NUM = 4;
static const int WHITE_SHEET_Y_NUM = 10;
static const float WHITE_ANIM_FPS = 24.0f;


static const std::vector<int> WHITE_SHURIKEN_FRAMES = { 12, 13, 14, 12, 15, 16, 17, 18 };
static const std::vector<float> WHITE_SHURIKEN_DURATIONS = {
	4.0f / WHITE_ANIM_FPS,
	6.0f / WHITE_ANIM_FPS,
	5.0f / WHITE_ANIM_FPS,
	1.0f / WHITE_ANIM_FPS,
	1.0f / WHITE_ANIM_FPS,
	3.0f / WHITE_ANIM_FPS,
	1.0f / WHITE_ANIM_FPS,
	3.0f / WHITE_ANIM_FPS
};
static const std::vector<int> WHITE_SWORD_FRAMES = { 12, 13, 14, 12, 15, 16, 17, 18 };
static const std::vector<float> WHITE_SWORD_DURATIONS = {
	4.0f / WHITE_ANIM_FPS,
	6.0f / WHITE_ANIM_FPS,
	5.0f / WHITE_ANIM_FPS,
	1.0f / WHITE_ANIM_FPS,
	1.0f / WHITE_ANIM_FPS,
	3.0f / WHITE_ANIM_FPS,
	1.0f / WHITE_ANIM_FPS,
	3.0f / WHITE_ANIM_FPS
};
static const std::vector<int> WHITE_IDLE_FRAMES = { 0, 1, 2, 3 };
static const std::vector<float> WHITE_IDLE_DURATIONS = {
	(2.0f / WHITE_ANIM_FPS) * WHITE_ACTION_TIME_SCALE,
	(2.0f / WHITE_ANIM_FPS) * WHITE_ACTION_TIME_SCALE,
	(2.0f / WHITE_ANIM_FPS) * WHITE_ACTION_TIME_SCALE,
	(2.0f / WHITE_ANIM_FPS) * WHITE_ACTION_TIME_SCALE
};

static const std::vector<int> WHITE_WALK_FRAMES = { 4, 5, 6, 7, 8, 9, 10, 11 };
static const std::vector<float> WHITE_WALK_DURATIONS = {
	(2.0f / WHITE_ANIM_FPS) * WHITE_ACTION_TIME_SCALE,
	(2.0f / WHITE_ANIM_FPS) * WHITE_ACTION_TIME_SCALE,
	(2.0f / WHITE_ANIM_FPS) * WHITE_ACTION_TIME_SCALE,
	(3.0f / WHITE_ANIM_FPS) * WHITE_ACTION_TIME_SCALE,
	(2.0f / WHITE_ANIM_FPS) * WHITE_ACTION_TIME_SCALE,
	(3.0f / WHITE_ANIM_FPS) * WHITE_ACTION_TIME_SCALE,
	(1.0f / WHITE_ANIM_FPS) * WHITE_ACTION_TIME_SCALE,
	(3.0f / WHITE_ANIM_FPS) * WHITE_ACTION_TIME_SCALE
};
static const std::vector<float> WHITE_BACK_WALK_DURATIONS = {
	(2.5f / WHITE_ANIM_FPS) * WHITE_ACTION_TIME_SCALE,
	(2.5f / WHITE_ANIM_FPS) * WHITE_ACTION_TIME_SCALE,
	(2.5f / WHITE_ANIM_FPS) * WHITE_ACTION_TIME_SCALE,
	(3.5f / WHITE_ANIM_FPS) * WHITE_ACTION_TIME_SCALE,
	(2.5f / WHITE_ANIM_FPS) * WHITE_ACTION_TIME_SCALE,
	(3.5f / WHITE_ANIM_FPS) * WHITE_ACTION_TIME_SCALE,
	(1.5f / WHITE_ANIM_FPS) * WHITE_ACTION_TIME_SCALE,
	(3.5f / WHITE_ANIM_FPS) * WHITE_ACTION_TIME_SCALE
};

float WhiteEnemyEntity::GetDirSign() const
{
	return m_dir ? 1.0f : -1.0f;
}

void WhiteEnemyEntity::PrepareMoveTracking(float wantedMoveX)
{
	m_lastMoveStartPos = m_transform != nullptr ? m_transform->GetPosition() : Vector2d::Zero();
	m_lastWantedMoveX = wantedMoveX;
	m_hasLastMove = true;
}

bool WhiteEnemyEntity::TryGetPlayerInfo(Vector2d& playerPos, HPComponent*& playerHp, bool& playerOnGround) const
{
	playerHp = nullptr;
	playerOnGround = false;

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
		PlayerEntity* player = static_cast<PlayerEntity*>(actor);
		playerOnGround = player != nullptr && player->OnGround();
		return true;
	}

	return false;
}

void WhiteEnemyEntity::PlayMotion(
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
	clip.speed = frameSpeed * WHITE_ACTION_TIME_SCALE;
	clip.loop = loop;

	m_anim->AddClip(motionName, clip);
	m_anim->Play(motionName, true);
	m_currentTexturePath = texturePath;
	m_currentMotionName = motionName;
}

void WhiteEnemyEntity::PlaySheetMotion(
	const std::string& motionName,
	const std::vector<int>& frames,
	const std::vector<float>& frameDurations,
	bool loop
)
{
	if (m_currentMotionName == motionName)
	{
		return;
	}

	if (m_sprite == nullptr || m_anim == nullptr || frames.empty())
	{
		return;
	}

	if (m_currentTexturePath != WHITE_TEXTURE_SHEET)
	{
		if (!m_sprite->LoadTextureDiv(WHITE_TEXTURE_SHEET, WHITE_SHEET_X_NUM, WHITE_SHEET_Y_NUM))
		{
			return;
		}

		m_currentTexturePath = WHITE_TEXTURE_SHEET;
	}

	AnimationClip clip;
	clip.frames = frames;
	clip.frameDurations = frameDurations;
	clip.speed = (2.0f / WHITE_ANIM_FPS) * WHITE_ACTION_TIME_SCALE;
	clip.loop = loop;

	m_anim->AddClip(motionName, clip);
	m_anim->Play(motionName, true);
	m_currentMotionName = motionName;
}

//
void WhiteEnemyEntity::StartShurikenAttack()
{
	m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

	m_attackType = 1;
	m_attackTimer = 0.0f;

	m_attackActive = false;

	//	//m_anim->SetAnimation(1);
	PlaySheetMotion("shuriken", WHITE_SHURIKEN_FRAMES, WHITE_SHURIKEN_DURATIONS, false);

	m_attackOnce = false;
	m_actionLock = true;
}

// 
void WhiteEnemyEntity::StartSwordAttack()
{
	m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

	m_attackType = 2;
	m_attackTimer = 0.0f;

	m_attackActive = false;

	//	//m_anim->SetAnimation(1);
	m_currentMotionName = "";
	PlaySheetMotion("sword", WHITE_SWORD_FRAMES, WHITE_SWORD_DURATIONS, false);

	m_attackOnce = false;
	m_actionLock = true;
}

void WhiteEnemyEntity::Update(float deltaTime)
{
	Vector2d playerPos = Vector2d::Zero();
	HPComponent* playerHp = nullptr;
	bool playerOnGround = false;

	if (!TryGetPlayerInfo(playerPos, playerHp, playerOnGround))
	{
		m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));
		EnemyEntity::Update(deltaTime);
		return;
	}

	Vector2d myPos = m_transform->GetPosition();

	float distanceX = playerPos.x - myPos.x;
	float distanceY = playerPos.y - myPos.y;
	float distance = distanceX;

	if (m_sprite != nullptr)
	{
		m_sprite->SetFlipH(m_attackType == 0 ? distanceX >= 0.0f : m_dir);
	}

	// 
	/*for (int j = 0; j < 3; j++)
	{
		int bulletIndex = j;

		if (m_bulletActive[bulletIndex] == true)
		{
			float bulletDistance = p->x - white->bulletX[bulletIndex];

			if (bulletDistance < 0.0f)
			{
				bulletDistance *= -1.0f;
			}

			if (bulletDistance > WHITE_BULLET_DELETE_RANGE)
			{
				white->bulletActive[bulletIndex] = false;
				white->bulletX[bulletIndex] = 0.0f;
				white->bulletY[bulletIndex] = 0.0f;
				white->bulletMuki[bulletIndex] = 0;
			}
		}
	}*/

	if (distance < 0.0f)
	{
		distance *= -1.0f;
	}

	if (m_cooldownTimer > 0.0f)
	{
		m_cooldownTimer -= deltaTime;

		if (m_cooldownTimer < 0.0f)
		{
			m_cooldownTimer = 0.0f;
		}
	}

	if (m_findPlayer == false)
	{
		if (distance < WHITE_FIND_RANGE)
		{
			m_findPlayer = true;
		}
		else
		{
			PlaySheetMotion("idle", WHITE_IDLE_FRAMES, WHITE_IDLE_DURATIONS, true);
			m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

			//continue;
			EnemyEntity::Update(deltaTime);
			return;
		}
	}

	/*----------------

			
			||whiteState||
			0 
			1 
			2 
			3 
			4 
			5 
			6  nashi

			
			||attackType||
			0 
			1 
			2 

   ---------------*/

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

			// Close range: sword when ready, back away while cooling down.
			if (distance < WHITE_BACK_RANGE)
			{
				if (m_cooldownTimer <= 0.0f)
				{
					m_whiteState = 4;
				}
				else
				{
					m_whiteState = 2;
				}
			}
			//
			else if (distance < WHITE_STOP_SHURIKEN_RANGE)
			{
				m_whiteState = 3;
			}
			// 
			else if (distance < WHITE_SHURIKEN_RANGE)
			{
				m_whiteState = 1;
			}
			// 
			else
			{
				m_whiteState = 5;
			}

			m_actionTimer = 0.0f;
			m_actionLock = true;
		}

		m_actionTimer += deltaTime;

		float dir = GetDirSign();

		if (m_cooldownTimer <= 0.0f && distance < WHITE_BACK_RANGE)
		{
			PlaySheetMotion("idle", WHITE_IDLE_FRAMES, WHITE_IDLE_DURATIONS, true);
			m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));
			StartSwordAttack();
			EnemyEntity::Update(deltaTime);
			m_hasLastMove = false;
			return;
		}

		switch (m_whiteState)
		{
		case 0:
			PlaySheetMotion("idle", WHITE_IDLE_FRAMES, WHITE_IDLE_DURATIONS, true);
			m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));
			m_actionLock = false;
			break;

		case 1:
			// 
			PlaySheetMotion("walk", WHITE_WALK_FRAMES, WHITE_WALK_DURATIONS, true);
			PrepareMoveTracking(WHITE_APPROACH_SPEED * dir);
			m_velocity->SetVelocity(Vector2d(WHITE_APPROACH_SPEED * dir, 0.0f));

			if (distance < WHITE_BACK_RANGE || distance > WHITE_SHURIKEN_RANGE)
			{
				m_actionLock = false;
			}

			if (m_cooldownTimer <= 0.0f)
			{
				if (distance < WHITE_BACK_RANGE)
				{
					StartSwordAttack();
				}
				else
				{
					StartShurikenAttack();
				}
			}
			else if (m_actionTimer >= WHITE_RECHECK_TIME)
			{
				m_actionLock = false;
			}
			break;

		case 2:
			// close range: swing sword in place when ready, retreat while cooling down
			if (m_cooldownTimer <= 0.0f)
			{
				PlaySheetMotion("idle", WHITE_IDLE_FRAMES, WHITE_IDLE_DURATIONS, true);
				m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));
				StartSwordAttack();
			}
			else
			{
				PlaySheetMotion("back_walk", WHITE_WALK_FRAMES, WHITE_BACK_WALK_DURATIONS, true);
				PrepareMoveTracking(-WHITE_BACK_SPEED * dir);
				m_velocity->SetVelocity(Vector2d(-WHITE_BACK_SPEED * dir, 0.0f));
			}

			if (distance >= WHITE_BACK_RANGE || m_actionTimer >= WHITE_RECHECK_TIME)
			{
				m_actionLock = false;
			}
			break;

		case 3:
			// 
			PlaySheetMotion("idle", WHITE_IDLE_FRAMES, WHITE_IDLE_DURATIONS, true);
			m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

			if (distance < WHITE_BACK_RANGE || distance >= WHITE_SHURIKEN_RANGE)
			{
				m_actionLock = false;
			}

			if (m_cooldownTimer <= 0.0f)
			{
				if (distance < WHITE_BACK_RANGE)
				{
					StartSwordAttack();
				}
				else
				{
					StartShurikenAttack();
				}
			}
			else if (m_actionTimer >= WHITE_RECHECK_TIME)
			{
				m_actionLock = false;
			}
			break;

		case 4:
			// 
			PlaySheetMotion("idle", WHITE_IDLE_FRAMES, WHITE_IDLE_DURATIONS, true);
			m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

			if (m_cooldownTimer <= 0.0f)
			{
				StartSwordAttack();
			}
			else if (m_actionTimer >= WHITE_RECHECK_TIME)
			{
				m_actionLock = false;
			}
			break;

		case 5:
			PlaySheetMotion("walk", WHITE_WALK_FRAMES, WHITE_WALK_DURATIONS, true);
			PrepareMoveTracking(WHITE_FAR_APPROACH_SPEED * dir);
			m_velocity->SetVelocity(Vector2d(WHITE_FAR_APPROACH_SPEED * dir, 0.0f));

			if (distance < WHITE_SHURIKEN_RANGE)
			{
				m_actionLock = false;
			}

			if (m_actionTimer >= WHITE_RECHECK_TIME)
			{
				m_actionLock = false;
			}
			break;

		case 6:
			//
			//

			SetState(Actor::State::Dead);
			break;
		}
	}
	else if (m_attackType == 1)
	{
		m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

		m_attackTimer += deltaTime;
		float attackTime = m_attackTimer;

		m_attackType = 1;

		// 0.7

		/*if (attackTime >= WHITE_SHURIKEN_SHOT_TIME && m_attackOnce == false)
		{
			for (int j = 0; j < 3; j++)
			{
				int bulletIndex = i * 3 + j;

				if (white->bulletActive[bulletIndex] == false)
				{
					white->bulletActive[bulletIndex] = true;

					white->bulletMuki[bulletIndex] = white->attackDir[i];
					white->bulletX[bulletIndex] = white->x[i] + 16.0f * white->attackDir[i];
					white->bulletY[bulletIndex] = white->y[i] - 16.0f;

					break;
				}
			}


			m_attackOnce = true;
			m_attackType = 2;
		}*/

		if (attackTime >= WHITE_SHURIKEN_SHOT_TIME && m_attackOnce == false)
		{
			float dir = GetDirSign();
			Vector2d bulletPos(myPos.x + 48.0f * dir, myPos.y - 16.0f);
			Vector2d bulletVel(WHITE_BULLET_SPEED * dir, 0.0f);

			m_scene->SpawnActor(
				new EnemyBullet(
					m_scene,
					bulletPos,
					bulletVel,
					WHITE_BULLET_DELETE_RANGE,
					"assets/images/enemy/bullet/shuriken.png",
					WHITE_SHURIKEN_DAMAGE
				)
			);

			m_attackOnce = true;
			m_attackActive = true;
		}

		// 		// 0.7

		if (attackTime >= WHITE_SHURIKEN_END_TIME)
		{
			m_attackType = 0;

			// 
			m_cooldownTimer = WHITE_SHURIKEN_COOLDOWN;

			m_actionTimer = 0.0f;
			m_actionLock = false;
			m_attackOnce = false;
			m_attackType = 0;
		}
	}
	else if (m_attackType == 2)
	{
		m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

		if (m_sprite != nullptr)
		{
			m_sprite->SetFlipH(m_dir);
		}

		m_attackTimer += deltaTime;
		float attackTime = m_attackTimer;

		m_attackActive = attackTime >= WHITE_SWORD_HIT_TIME && attackTime < WHITE_SWORD_END_TIME;

		if (attackTime >= WHITE_SWORD_HIT_TIME && m_attackOnce == false)
		{
			float attackRange = WHITE_SWORD_ATTACK_RANGE;
			float heightRange = WHITE_SWORD_HEIGHT_RANGE;

			float dx = playerPos.x - myPos.x;
			float dy = playerPos.y - myPos.y;

			bool hit = false;

			if (m_dir == true)
			{
				if (dx > 0.0f && dx < attackRange)
				{
					hit = true;
				}
			}
			else
			{
				if (dx < 0.0f && dx > -attackRange)
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

				if (dy < heightRange)
				{
					if (playerHp != nullptr)
					{
						playerHp->Damage(WHITE_SWORD_DAMAGE);
						playerHp->SetInvincible(0.3f);
					}
				}
			}

			m_attackOnce = true;
		}

		if (attackTime >= WHITE_SWORD_END_TIME)
		{
			m_cooldownTimer = WHITE_SWORD_COOLDOWN;
			m_actionTimer = 0.0f;
			m_actionLock = false;
			m_attackOnce = false;
			m_attackActive = false;
			m_attackType = 0;
		}
	}

	// 
	//

	// 

	// 
	/*for (int j = 0; j < 3; j++)
	{
		int bulletIndex = i * 3 + j;

		if (white->bulletActive[bulletIndex] == true)
		{
			white->bulletX[bulletIndex] += WHITE_BULLET_SPEED * white->bulletMuki[bulletIndex] * dt;

		}
	}*/

	EnemyEntity::Update(deltaTime);

	if (m_hasLastMove && m_attackType == 0)
	{
		Vector2d movedPos = m_transform != nullptr ? m_transform->GetPosition() : Vector2d::Zero();
		float movedX = movedPos.x - m_lastMoveStartPos.x;
		bool wantedMove = m_lastWantedMoveX > WHITE_STUCK_MOVE_EPS || m_lastWantedMoveX < -WHITE_STUCK_MOVE_EPS;
		bool blockedMove = movedX > -WHITE_STUCK_MOVE_EPS && movedX < WHITE_STUCK_MOVE_EPS;

		if (wantedMove && blockedMove)
		{
			PlaySheetMotion("idle", WHITE_IDLE_FRAMES, WHITE_IDLE_DURATIONS, true);
			m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));
			m_actionLock = false;
		}
	}

	m_hasLastMove = false;
}

std::string WhiteEnemyEntity::GetTexturePath() const
{
	return WHITE_TEXTURE_IDLE;
}
