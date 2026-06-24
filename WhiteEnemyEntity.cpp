#include "WhiteEnemyEntity.h"
#include "Scene.h"
#include "Actor.h"
#include "PlayerEntity.h"
#include "EnemyBullet.h"
#include "GravityComponent.h"
#include "VelocityComponent.h"
#include "TransformComponent.h"
#include "HPComponent.h"
#include "SpriteComponent.h"
#include "AnimationComponent.h"
#include "Vector2d.h"
#include <DxLib.h>
#include <cmath>

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
	, m_damageState(0)
	, m_damageFrameIndex(0)
	, m_damageFrameTimer(0.0f)
	, m_damageHoldGroundFrame(false)
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


// Detection and spacing
static const float WHITE_TILE_SIZE = 104.0f;
static const float WHITE_ENEMY_HALF_WIDTH = 48.0f;
static const float WHITE_PLAYER_HALF_WIDTH = 42.5f;
static const float WHITE_SWORD_GAP_RANGE = WHITE_TILE_SIZE * 0.7f;
static const float WHITE_FIND_RANGE = 800.0f;
static const float WHITE_NEAR_SWORD_RANGE = WHITE_ENEMY_HALF_WIDTH + WHITE_PLAYER_HALF_WIDTH + WHITE_SWORD_GAP_RANGE;
static const float WHITE_BACK_RANGE = 450.0f;
static const float WHITE_STOP_SHURIKEN_RANGE = 612.0f;
static const float WHITE_SHURIKEN_RANGE = 650.0f;
static const float WHITE_BULLET_DELETE_RANGE = 1500.0f;
static const float WHITE_SAME_FLOOR_Y_RANGE = WHITE_TILE_SIZE * 0.5f;
static const float WHITE_STUCK_MOVE_EPS = 1.0f;
static const float WHITE_STUCK_IDLE_TIME = 0.25f;

// Movement
static const float WHITE_APPROACH_SPEED = 120.0f;
static const float WHITE_BACK_SPEED = 150.0f;
static const float WHITE_FAR_APPROACH_SPEED = 250.0f;
static const float WHITE_BULLET_SPEED = 250.0f;

// Shared timing
static const float WHITE_ACTION_TIME_SCALE = 1.5f;
static const float WHITE_RECHECK_TIME = 0.2f * WHITE_ACTION_TIME_SCALE;
static const float WHITE_ANIM_FPS = 24.0f;
static const float WHITE_FRAME_TIME = 1.0f / WHITE_ANIM_FPS;

// Shuriken attack
static const float WHITE_SHURIKEN_COOLDOWN = 0.45f * WHITE_ACTION_TIME_SCALE;
static const int WHITE_SHURIKEN_DAMAGE = 5;
static const float WHITE_SHURIKEN_SHOT_TIME = 16.0f / WHITE_ANIM_FPS;
static const float WHITE_SHURIKEN_END_TIME = 24.0f / WHITE_ANIM_FPS;
static const Vector2d WHITE_SHURIKEN_BULLET_DRAW_SIZE = Vector2d(96.0f, 96.0f);
static const float WHITE_SHURIKEN_BULLET_ROTATE_INTERVAL = 4.0f / 60.0f;
static const float WHITE_SHURIKEN_BULLET_ROTATE_STEP = 15.0f * 3.14159265f / 180.0f;

// Sword attack
static const float WHITE_SWORD_COOLDOWN = 0.15f * WHITE_ACTION_TIME_SCALE;
static const float WHITE_SWORD_ATTACK_RANGE = WHITE_BACK_RANGE;
static const float WHITE_SWORD_HEIGHT_RANGE = 80.0f;
static const int WHITE_SWORD_DAMAGE = 10;
static const float WHITE_SWORD_KNOCKBACK_X = 300.0f;
static const float WHITE_SWORD_KNOCKBACK_Y = -200.0f;
static const float WHITE_SWORD_HIT_TIME = 20.0f / WHITE_ANIM_FPS;
static const float WHITE_SWORD_ACTIVE_END_TIME = 29.0f / WHITE_ANIM_FPS;
static const float WHITE_SWORD_END_TIME = 35.0f / WHITE_ANIM_FPS;

// Textures and sprite sheets
static const char* WHITE_TEXTURE_IDLE = "assets/images/enemy/white/idle.png";
static const char* WHITE_TEXTURE_WALK = "assets/images/enemy/white/walk.png";
static const char* WHITE_TEXTURE_SHURIKEN = "assets/images/enemy/white/shuriken.png";
static const char* WHITE_TEXTURE_SWORD_PRE = "assets/images/enemy/white/sword_pre.png";
static const char* WHITE_TEXTURE_SWORD = "assets/images/enemy/white/sword.png";
static const char* WHITE_TEXTURE_SWORD_AFTER = "assets/images/enemy/white/sword_after.png";
static const char* WHITE_TEXTURE_SHEET = "assets/images/enemy/white/White.png";
static const char* WHITE_TEXTURE_HIT_WEAK = "assets/images/enemy/white/White2.png";
static const char* WHITE_TEXTURE_SHURIKEN_BULLET = "assets/images/enemy/white/Shuriken.png";
static const int WHITE_SHEET_X_NUM = 4;
static const int WHITE_SHEET_Y_NUM = 10;
static const int WHITE_HIT_WEAK_X_NUM = 4;
static const int WHITE_HIT_WEAK_Y_NUM = 1;

// Damage reactions
static const float WHITE_BLOW_MIN_KNOCKBACK = 180.0f;
static const float WHITE_BLOW_LARGE_MIN_KNOCKBACK = 520.0f;
static const int WHITE_DAMAGE_NONE = 0;
static const int WHITE_DAMAGE_WEAK = 1;
static const int WHITE_DAMAGE_BLOW = 2;
static const int WHITE_DAMAGE_BLOW_LARGE = 3;
static const int WHITE_DAMAGE_DEAD = 4;
static const int WHITE_LARGE_BLOW_GROUND_FRAME_INDEX = 9;
static const int WHITE_DEAD_FRAME = 37;
static const float WHITE_DEAD_SHOW_TIME = 6.0f * WHITE_FRAME_TIME;

// Attack motions
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
static const std::vector<int> WHITE_SWORD_FRAMES = { 12, 13, 14, 19, 20, 21, 22, 23, 24 };
static const std::vector<float> WHITE_SWORD_DURATIONS = {
	4.0f / WHITE_ANIM_FPS,
	6.0f / WHITE_ANIM_FPS,
	5.0f / WHITE_ANIM_FPS,
	3.0f / WHITE_ANIM_FPS,
	2.0f / WHITE_ANIM_FPS,
	1.0f / WHITE_ANIM_FPS,
	3.0f / WHITE_ANIM_FPS,
	5.0f / WHITE_ANIM_FPS,
	3.0f / WHITE_ANIM_FPS
};

// Movement motions
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

// Damage reaction motions
static const std::vector<int> WHITE_WEAK_HIT_FRAMES = { 0, 1, 2, 3 };
static const std::vector<float> WHITE_WEAK_HIT_DURATIONS = {
	5.0f * WHITE_FRAME_TIME,
	2.0f * WHITE_FRAME_TIME,
	2.0f * WHITE_FRAME_TIME,
	1.0f * WHITE_FRAME_TIME
};

static const std::vector<int> WHITE_BLOW_FRAMES = { 25, 26, 30, 31, 32, 35, 36, 37, 38, 39 };
static const std::vector<float> WHITE_BLOW_DURATIONS = {
	1.0f * WHITE_FRAME_TIME,
	3.0f * WHITE_FRAME_TIME,
	4.0f * WHITE_FRAME_TIME,
	2.0f * WHITE_FRAME_TIME,
	2.0f * WHITE_FRAME_TIME,
	2.0f * WHITE_FRAME_TIME,
	2.0f * WHITE_FRAME_TIME,
	1.0f * WHITE_FRAME_TIME,
	3.0f * WHITE_FRAME_TIME,
	2.0f * WHITE_FRAME_TIME
};

static const std::vector<int> WHITE_BLOW_LARGE_FRAMES = {
	25, 26, 27, 28, 29,
	30, 31, 32, 33, 34,
	35, 36, 37, 38, 39
};
static const std::vector<float> WHITE_BLOW_LARGE_DURATIONS = {
	1.0f * WHITE_FRAME_TIME,
	6.0f * WHITE_FRAME_TIME,
	1.0f * WHITE_FRAME_TIME,
	2.0f * WHITE_FRAME_TIME,
	2.0f * WHITE_FRAME_TIME,
	2.0f * WHITE_FRAME_TIME,
	3.0f * WHITE_FRAME_TIME,
	2.0f * WHITE_FRAME_TIME,
	2.0f * WHITE_FRAME_TIME,
	3.0f * WHITE_FRAME_TIME,
	2.0f * WHITE_FRAME_TIME,
	2.0f * WHITE_FRAME_TIME,
	1.0f * WHITE_FRAME_TIME,
	3.0f * WHITE_FRAME_TIME,
	2.0f * WHITE_FRAME_TIME
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

bool WhiteEnemyEntity::TryGetPlayerInfo(Vector2d& playerPos, PlayerEntity*& player, bool& playerOnGround) const
{
	player = nullptr;
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
		player = static_cast<PlayerEntity*>(actor);
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

void WhiteEnemyEntity::CancelAttackForDamage()
{
	m_attackType = 0;
	m_attackActive = false;
	m_attackOnce = false;
	m_actionTimer = 0.0f;
	m_cooldownTimer = 0.0f;
	m_actionLock = true;
	m_hasLastMove = false;
}

void WhiteEnemyEntity::StartWeakHit()
{
	CancelAttackForDamage();

	if (m_sprite != nullptr)
	{
		m_sprite->LoadTextureDiv(WHITE_TEXTURE_HIT_WEAK, WHITE_HIT_WEAK_X_NUM, WHITE_HIT_WEAK_Y_NUM);
		m_sprite->SetFrame(WHITE_WEAK_HIT_FRAMES[0]);
		m_sprite->SetDrawSize(96.0f, 190.0f);
	}

	m_currentTexturePath = WHITE_TEXTURE_HIT_WEAK;
	m_currentMotionName = "hit_weak";
	m_damageState = WHITE_DAMAGE_WEAK;
	m_damageFrameIndex = 0;
	m_damageFrameTimer = 0.0f;
	m_damageHoldGroundFrame = false;
}

void WhiteEnemyEntity::StartBlowHit(const Vector2d& knockback)
{
	CancelAttackForDamage();
	m_velocity->SetVelocity(knockback);

	if (m_sprite != nullptr)
	{
		m_sprite->LoadTextureDiv(WHITE_TEXTURE_SHEET, WHITE_SHEET_X_NUM, WHITE_SHEET_Y_NUM);
		m_sprite->SetFrame(WHITE_BLOW_FRAMES[0]);
		m_sprite->SetDrawSize(96.0f, 190.0f);
	}

	m_currentTexturePath = WHITE_TEXTURE_SHEET;
	m_currentMotionName = "hit_blow";
	m_damageState = WHITE_DAMAGE_BLOW;
	m_damageFrameIndex = 0;
	m_damageFrameTimer = 0.0f;
	m_damageHoldGroundFrame = false;
}

void WhiteEnemyEntity::StartLargeBlowHit(const Vector2d& knockback)
{
	CancelAttackForDamage();
	m_velocity->SetVelocity(knockback);

	if (m_sprite != nullptr)
	{
		m_sprite->LoadTextureDiv(WHITE_TEXTURE_SHEET, WHITE_SHEET_X_NUM, WHITE_SHEET_Y_NUM);
		m_sprite->SetFrame(WHITE_BLOW_LARGE_FRAMES[0]);
		m_sprite->SetDrawSize(96.0f, 190.0f);
	}

	m_currentTexturePath = WHITE_TEXTURE_SHEET;
	m_currentMotionName = "hit_blow_large";
	m_damageState = WHITE_DAMAGE_BLOW_LARGE;
	m_damageFrameIndex = 0;
	m_damageFrameTimer = 0.0f;
	m_damageHoldGroundFrame = false;
}

void WhiteEnemyEntity::StartDeadHit()
{
	CancelAttackForDamage();
	m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

	if (m_sprite != nullptr)
	{
		m_sprite->LoadTextureDiv(WHITE_TEXTURE_SHEET, WHITE_SHEET_X_NUM, WHITE_SHEET_Y_NUM);
		m_sprite->SetFrame(WHITE_DEAD_FRAME);
		m_sprite->SetDrawSize(96.0f, 190.0f);
	}

	m_currentTexturePath = WHITE_TEXTURE_SHEET;
	m_currentMotionName = "hit_dead";
	m_damageState = WHITE_DAMAGE_DEAD;
	m_damageFrameIndex = 0;
	m_damageFrameTimer = 0.0f;
	m_damageHoldGroundFrame = false;
}

void WhiteEnemyEntity::TakeDamage(int damage, const Vector2d& knockback)
{
	if (m_hp == nullptr)
	{
		return;
	}

	m_hp->Damage(damage);

	if (m_hp->GetHP() <= 0)
	{
		StartDeadHit();
		return;
	}

	Vector2d damageKnockback = knockback;
	Vector2d playerPos = Vector2d::Zero();
	PlayerEntity* player = nullptr;
	bool playerOnGround = false;
	if (TryGetPlayerInfo(playerPos, player, playerOnGround))
	{
		Vector2d myPos = GetPos();
		float awaySign = myPos.x < playerPos.x ? -1.0f : 1.0f;
		damageKnockback.x = std::fabs(damageKnockback.x) * awaySign;
	}

	float knockbackPower = damageKnockback.length();
	if (knockbackPower >= WHITE_BLOW_LARGE_MIN_KNOCKBACK)
	{
		StartLargeBlowHit(damageKnockback);
	}
	else if (knockbackPower >= WHITE_BLOW_MIN_KNOCKBACK)
	{
		StartBlowHit(damageKnockback);
	}
	else
	{
		StartWeakHit();
	}
}

void WhiteEnemyEntity::UpdateDamageMotion(float deltaTime)
{
	if (m_damageState == WHITE_DAMAGE_NONE)
	{
		return;
	}

	const std::vector<int>* frames = nullptr;
	const std::vector<float>* durations = nullptr;

	if (m_damageState == WHITE_DAMAGE_WEAK)
	{
		frames = &WHITE_WEAK_HIT_FRAMES;
		durations = &WHITE_WEAK_HIT_DURATIONS;
	}
	else if (m_damageState == WHITE_DAMAGE_BLOW)
	{
		frames = &WHITE_BLOW_FRAMES;
		durations = &WHITE_BLOW_DURATIONS;
	}
	else if (m_damageState == WHITE_DAMAGE_BLOW_LARGE)
	{
		frames = &WHITE_BLOW_LARGE_FRAMES;
		durations = &WHITE_BLOW_LARGE_DURATIONS;
	}
	else if (m_damageState == WHITE_DAMAGE_DEAD)
	{
		m_damageFrameTimer += deltaTime;
		if (m_damageFrameTimer >= WHITE_DEAD_SHOW_TIME)
		{
			SetState(Actor::State::Dead);
		}
		return;
	}

	if (frames == nullptr || durations == nullptr || frames->empty())
	{
		m_damageState = WHITE_DAMAGE_NONE;
		return;
	}

	if (m_damageState == WHITE_DAMAGE_BLOW_LARGE &&
		m_damageFrameIndex == WHITE_LARGE_BLOW_GROUND_FRAME_INDEX &&
		m_isGround == false)
	{
		return;
	}

	m_damageFrameTimer += deltaTime;
	while (m_damageFrameIndex < static_cast<int>(durations->size()) &&
		m_damageFrameTimer >= (*durations)[m_damageFrameIndex])
	{
		m_damageFrameTimer -= (*durations)[m_damageFrameIndex];
		m_damageFrameIndex++;

		if (m_damageFrameIndex >= static_cast<int>(frames->size()))
		{
			m_damageState = WHITE_DAMAGE_NONE;
			m_actionLock = false;
			m_currentMotionName = "";
			return;
		}

		if (m_sprite != nullptr)
		{
			m_sprite->SetFrame((*frames)[m_damageFrameIndex]);
		}

	// 
	/*for (int j = 0; j < 3; j++)
	{
		int bulletIndex = j;

void WhiteEnemyEntity::Update(float deltaTime)
{
	

	if (m_damageState != WHITE_DAMAGE_NONE)
	{
		UpdateDamageMotion(deltaTime);
		if (m_damageState == WHITE_DAMAGE_NONE)
		{
			EnemyEntity::Update(deltaTime);
		}
		else if (m_damageState != WHITE_DAMAGE_DEAD)
		{
			if (m_gravity != nullptr)
			{
				m_gravity->Update(deltaTime);
			}
			MoveAndCollide(deltaTime);
		}
		return;
	}

	Vector2d playerPos = Vector2d::Zero();
	PlayerEntity* player = nullptr;
	bool playerOnGround = false;

	if (!TryGetPlayerInfo(playerPos, player, playerOnGround))
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
					WHITE_TEXTURE_SHURIKEN_BULLET,
					WHITE_SHURIKEN_DAMAGE,
					WHITE_SHURIKEN_BULLET_DRAW_SIZE,
					true,
					WHITE_SHURIKEN_BULLET_ROTATE_INTERVAL,
					WHITE_SHURIKEN_BULLET_ROTATE_STEP
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

		m_attackActive = attackTime >= WHITE_SWORD_HIT_TIME && attackTime < WHITE_SWORD_ACTIVE_END_TIME;

		if (m_attackActive == true && m_attackOnce == false)
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
					if (player != nullptr)
					{
						float knockbackX = playerPos.x < myPos.x ? -WHITE_SWORD_KNOCKBACK_X : WHITE_SWORD_KNOCKBACK_X;
						player->TakeDamage(
							WHITE_SWORD_DAMAGE,
							Vector2d(knockbackX, WHITE_SWORD_KNOCKBACK_Y)
						);
						m_attackOnce = true;
					}
				}
			}
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





/* DEBUG DAMAGE TEST START
static const int WHITE_DEBUG_DAMAGE = 10;
static const Vector2d WHITE_DEBUG_KNOCKBACK = Vector2d(260.0f, -320.0f);

void WhiteEnemyEntity::UpdateDebugDamageInput()
{
	static bool wasDebugDamageKeyDown = false;
	bool isDebugDamageKeyDown = CheckHitKey(KEY_INPUT_H) != 0;

	if (isDebugDamageKeyDown && !wasDebugDamageKeyDown)
	{
		TakeDamage(WHITE_DEBUG_DAMAGE, WHITE_DEBUG_KNOCKBACK);
	}

	wasDebugDamageKeyDown = isDebugDamageKeyDown;
}
// DEBUG DAMAGE TEST END*/
