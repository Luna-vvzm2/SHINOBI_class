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
static const std::vector<int> WHITE_SHURIKEN_FRAMES = 
{
	12,12,12,12,

	13,13,13,13,13,13,

	14,14,14,14,14,

	12,

	15,

	16,16,16,

	17,

	18,18,18
};

static const std::vector<int> WHITE_SWORD_FRAMES =
{
	12,12,12,12,
	13,13,13,13,13,13,
	14,14,14,14,14,
	19,19,19,
	20,20,
	21,
	22,22,22,
	23,23,23,23,23,
	24,24,24
};

// Movement motions
static const std::vector<int> WHITE_IDLE_FRAMES =
{
	0,0,
	1,1,
	2,2,
	3,3
};


static const std::vector<int> WHITE_WALK_FRAMES = 
{
	4,4,
	5,5,
	6,6,
	7,7,7,
	8,8,
	9,9,9,
	10,
	11,11,11
};

static const std::vector<int> WHITE_BACK_WALK_FRAMES =
{
	4,4,
	5,5,
	6,6,
	7,7,7,
	8,8,
	9,9,9,
	10,
	11,11,11
};

// Damage reaction motions
static const std::vector<int> WHITE_WEAK_HIT_FRAMES = 
{
	4,4,4,
	5,5,5,
	6,6,6,
	7,7,7,7,
	8,8,8,
	9,9,9,9,
	10,10,
	11,11,11,11
};


static const std::vector<int> WHITE_BLOW_FRAMES = 
{
	25,

	26,26,26,

	30,30,30,30,

	31,31,

	32,32,

	35,35,

	36,36,

	37,

	38,38,38,

	39,39
};

static const std::vector<int> WHITE_BLOW_LARGE_FRAMES = 
{
	25,

	26,26,26,26,26,26,

	27,

	28,28,

	29,29,

	30,30,

	31,31,31,

	32,32,

	33,33,

	34,34,34,

	35,35,

	36,36,

	37,

	38,38,38,

	39,39
};


bool WhiteEnemyEntity::Init()
{
	if (!EnemyEntity::Init()) return false;

	m_sprite->SetDrawSize(96.0f, 190.0f);

	m_anim = AddComponent<AnimationComponent>();
	m_anim->SetSprite(m_sprite);

	AnimationClip idle;
	idle.frames = WHITE_IDLE_FRAMES;
	idle.speed = WHITE_FRAME_TIME;
	idle.loop = true;
	m_anim->AddClip("idle", idle);

	AnimationClip walk;
	walk.frames = WHITE_WALK_FRAMES;
	walk.speed = WHITE_FRAME_TIME;
	walk.loop = true;
	m_anim->AddClip("walk", walk);

	AnimationClip backWalk;
	backWalk.frames = WHITE_BACK_WALK_FRAMES;
	backWalk.speed = WHITE_FRAME_TIME;
	backWalk.loop = true;
	m_anim->AddClip("back_walk", backWalk);

	AnimationClip shuriken;
	shuriken.frames = WHITE_SHURIKEN_FRAMES;
	shuriken.speed = WHITE_FRAME_TIME;
	shuriken.loop = false;
	m_anim->AddClip("shuriken", shuriken);

	AnimationClip sword;
	sword.frames = WHITE_SWORD_FRAMES;
	sword.speed = WHITE_FRAME_TIME;
	sword.loop = false;
	m_anim->AddClip("sword", sword);

	AnimationClip weakHit;
	weakHit.frames = WHITE_WEAK_HIT_FRAMES;
	weakHit.speed = WHITE_FRAME_TIME;
	weakHit.loop = false;
	m_anim->AddClip("weakHit", weakHit);

	AnimationClip blowHit;
	blowHit.frames = WHITE_BLOW_FRAMES;
	blowHit.speed = WHITE_FRAME_TIME;
	blowHit.loop = false;
	m_anim->AddClip("blowHit", blowHit);

	AnimationClip blowLargeHit;
	blowLargeHit.frames = WHITE_BLOW_LARGE_FRAMES;
	blowLargeHit.speed = WHITE_FRAME_TIME;
	blowLargeHit.loop = false;
	m_anim->AddClip("blowLargeHit", blowLargeHit);

	AnimationClip deadHit;
	m_anim->AddClip("deadHit", deadHit);

	m_sprite->LoadTextureDiv(
		WHITE_TEXTURE_SHEET,
		WHITE_SHEET_X_NUM,
		WHITE_SHEET_Y_NUM
	);

	m_anim->Play("idle", true);
	return true;
}

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
	bool loop
)
{
	if (m_currentMotionName == motionName)
	{
		return;
	}
	m_anim->Play(motionName, true);
	m_currentMotionName = motionName;
}


void WhiteEnemyEntity::StartShurikenAttack()
{
	m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

	m_attackType = 1;
	m_attackTimer = 0.0f;

	m_attackActive = false;

	PlaySheetMotion("shuriken", WHITE_SHURIKEN_FRAMES, false);

	m_attackOnce = false;
	m_actionLock = true;
}


void WhiteEnemyEntity::StartSwordAttack()
{
	m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

	m_attackType = 2;
	m_attackTimer = 0.0f;

	m_attackActive = false;

	m_currentMotionName = "";
	PlaySheetMotion("sword", WHITE_SWORD_FRAMES, false);

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
		m_anim->Play("weakHit", true);
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
		m_anim->Play("blowHit", true);
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
		m_anim->Play("blowLargeHit", true);
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
		m_anim->Play("deadHit", true);
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
	switch (m_damageState)
	{
	case WHITE_DAMAGE_WEAK:
	case WHITE_DAMAGE_BLOW:
	case WHITE_DAMAGE_BLOW_LARGE:

		if (m_anim->IsFinished())
		{
			m_damageState = WHITE_DAMAGE_NONE;

			m_sprite->LoadTextureDiv(
				WHITE_TEXTURE_SHEET,
				WHITE_SHEET_X_NUM,
				WHITE_SHEET_Y_NUM
			);

			m_currentMotionName = "";
			m_anim->Play("idle", true);
		}
		break;

	case WHITE_DAMAGE_DEAD:
		break;
	}
}

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
		m_sprite->SetFlipX(m_attackType == 0 ? distanceX >= 0.0f : m_dir);
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
			PlaySheetMotion("idle", WHITE_IDLE_FRAMES, true);
			m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

			//continue;
			EnemyEntity::Update(deltaTime);
			return;
		}
	}

	/*----------------
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

			else if (distance < WHITE_STOP_SHURIKEN_RANGE)
			{
				m_whiteState = 3;
			}

			else if (distance < WHITE_SHURIKEN_RANGE)
			{
				m_whiteState = 1;
			}
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
			PlaySheetMotion("idle", WHITE_IDLE_FRAMES, true);
			m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));
			StartSwordAttack();
			EnemyEntity::Update(deltaTime);
			m_hasLastMove = false;
			return;
		}

		switch (m_whiteState)
		{
		case 0:
			PlaySheetMotion("idle", WHITE_IDLE_FRAMES, true);
			m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));
			m_actionLock = false;
			break;

		case 1:
			PlaySheetMotion("walk", WHITE_WALK_FRAMES, true);
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
				PlaySheetMotion("idle", WHITE_IDLE_FRAMES, true);
				m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));
				StartSwordAttack();
			}
			else
			{
				PlaySheetMotion("back_walk", WHITE_WALK_FRAMES, true);
				PrepareMoveTracking(-WHITE_BACK_SPEED * dir);
				m_velocity->SetVelocity(Vector2d(-WHITE_BACK_SPEED * dir, 0.0f));
			}

			if (distance >= WHITE_BACK_RANGE || m_actionTimer >= WHITE_RECHECK_TIME)
			{
				m_actionLock = false;
			}
			break;

		case 3:
			PlaySheetMotion("idle", WHITE_IDLE_FRAMES, true);
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
			PlaySheetMotion("idle", WHITE_IDLE_FRAMES, true);
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
			PlaySheetMotion("walk", WHITE_WALK_FRAMES, true);
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
			//sinumotion

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


		if (attackTime >= WHITE_SHURIKEN_END_TIME)
		{
			m_attackType = 0;

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
			m_sprite->SetFlipX(m_dir);
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


	EnemyEntity::Update(deltaTime);

	if (m_hasLastMove && m_attackType == 0)
	{
		Vector2d movedPos = m_transform != nullptr ? m_transform->GetPosition() : Vector2d::Zero();
		float movedX = movedPos.x - m_lastMoveStartPos.x;
		bool wantedMove = m_lastWantedMoveX > WHITE_STUCK_MOVE_EPS || m_lastWantedMoveX < -WHITE_STUCK_MOVE_EPS;
		bool blockedMove = movedX > -WHITE_STUCK_MOVE_EPS && movedX < WHITE_STUCK_MOVE_EPS;

		if (wantedMove && blockedMove)
		{
			PlaySheetMotion("idle", WHITE_IDLE_FRAMES, true);
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