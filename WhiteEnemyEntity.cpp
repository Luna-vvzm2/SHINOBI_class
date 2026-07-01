#include "WhiteEnemyEntity.h"
#include "Scene.h"
#include "Actor.h"
#include "PlayerEntity.h"
#include "EntityActor.h"
#include "CollisionComponent.h"
#include "GravityComponent.h"
#include "VelocityComponent.h"
#include "TransformComponent.h"
#include "HPComponent.h"
#include "SpriteComponent.h"
#include "AnimationComponent.h"
#include "Game.h"
#include "Renderer.h"
#include "Vector2d.h"
#include <DxLib.h>
#include <cmath>

static const float WHITE_ENEMY_DRAW_SCALE = 0.5f;
static const Vector2d WHITE_SHURIKEN_BASE_DRAW_SIZE = Vector2d(96.0f, 96.0f);

namespace {
	std::vector<int> BuildTimedFrames(
		const std::vector<int>& frames,
		const std::vector<float>& frameDurations,
		float baseFrameTime
	)
	{
		std::vector<int> timedFrames;

		for (size_t i = 0; i < frames.size(); i++)
		{
			float duration = i < frameDurations.size() ? frameDurations[i] : baseFrameTime;
			int repeatCount = static_cast<int>((duration / baseFrameTime) + 0.5f);
			if (repeatCount < 1)
			{
				repeatCount = 1;
			}

			for (int count = 0; count < repeatCount; count++)
			{
				timedFrames.push_back(frames[i]);
			}
		}

		return timedFrames;
	}

	float GetTotalDuration(const std::vector<float>& frameDurations)
	{
		float totalDuration = 0.0f;
		for (float duration : frameDurations)
		{
			totalDuration += duration;
		}
		return totalDuration;
	}
}

class WhiteShurikenBullet : public EntityActor
{
public:
	WhiteShurikenBullet(
		Scene* scene,
		const Vector2d& pos,
		const Vector2d& velocity,
		float deleteRange,
		const std::string& texturePath,
		int damage,
		const Vector2d& drawSize,
		float rotateInterval,
		float rotateStep
	)
		: EntityActor(scene, pos, Vector2d(12.0f, 12.0f))
		, m_startPos(pos)
		, m_bulletVelocity(velocity)
		, m_deleteRange(deleteRange)
		, m_texturePath(texturePath)
		, m_damage(damage)
		, m_drawSize(drawSize)
		, m_rotateInterval(rotateInterval)
		, m_rotateStep(rotateStep)
		, m_rotationAngle(0.0f)
		, m_rotationTimer(0.0f)
	{
	}

	bool Init() override
	{
		if (!EntityActor::Init()) return false;
		m_velocity->SetVelocity(m_bulletVelocity);
		m_collision->SetCircle(6.0f);
		return true;
	}

	void Update(float deltaTime) override
	{
		Actor::Update(deltaTime);

		m_rotationTimer += deltaTime;
		while (m_rotateInterval > 0.0f && m_rotationTimer >= m_rotateInterval)
		{
			m_rotationTimer -= m_rotateInterval;
			m_rotationAngle += m_rotateStep;
		}

		Vector2d pos = m_transform->GetPosition();
		pos += m_velocity->GetVelocity() * deltaTime;
		m_transform->SetPosition(pos);

		if ((pos - m_startPos).length() > m_deleteRange || TryDamagePlayer())
		{
			SetState(Actor::State::Dead);
		}
	}

	void Draw() override
	{
		if (IsDead() || m_scene == nullptr || m_sprite == nullptr || m_transform == nullptr)
		{
			return;
		}

		Game* game = m_scene->GetGame();
		Renderer* renderer = game != nullptr ? game->GetRenderer() : nullptr;
		int handle = m_sprite->GetHandle();
		if (renderer == nullptr || handle < 0)
		{
			return;
		}

		int textureWidth = 0;
		int textureHeight = 0;
		GetGraphSize(handle, &textureWidth, &textureHeight);
		float scaleX = textureWidth > 0 ? m_drawSize.x / static_cast<float>(textureWidth) : 1.0f;
		float scaleY = textureHeight > 0 ? m_drawSize.y / static_cast<float>(textureHeight) : 1.0f;

		renderer->DrawSpriteEx(
			m_transform->GetPosition(),
			scaleX,
			scaleY,
			m_rotationAngle,
			handle,
			true,
			Vector2d(static_cast<float>(textureWidth), static_cast<float>(textureHeight)) * 0.5f
		);

#ifdef _DEBUG
		if (m_collision != nullptr)
		{
			m_collision->DrawDebug();
		}
#endif
	}

	ActorType GetType() const override { return ActorType::Ball; }

private:
	bool TryDamagePlayer()
	{
		for (Actor* actor : m_scene->GetActors())
		{
			if (actor == nullptr || actor->GetType() != ActorType::Player || actor->IsDead())
			{
				continue;
			}

			CollisionComponent* playerCollision = actor->GetComponent<CollisionComponent>();
			if (playerCollision == nullptr || !m_collision->CheckCollision(playerCollision))
			{
				continue;
			}

			PlayerEntity* player = static_cast<PlayerEntity*>(actor);
			float knockbackX = player->GetPos().x < m_transform->GetPosition().x ? -300.0f : 300.0f;
			player->TakeDamage(m_damage, Vector2d(knockbackX, -200.0f));
			return true;
		}

		return false;
	}

	std::string GetTexturePath() const override { return m_texturePath; }

	Vector2d m_startPos;
	Vector2d m_bulletVelocity;
	float m_deleteRange;
	std::string m_texturePath;
	int m_damage;
	Vector2d m_drawSize;
	float m_rotateInterval;
	float m_rotateStep;
	float m_rotationAngle;
	float m_rotationTimer;
};

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

	m_transform->SetScale(Vector2d(WHITE_ENEMY_DRAW_SCALE, WHITE_ENEMY_DRAW_SCALE));

	m_anim = AddComponent<AnimationComponent>();
	m_anim->SetSprite(m_sprite);

	return true;
}


// 索敵範囲と距離判定
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

// 移動関係
static const float WHITE_APPROACH_SPEED = 120.0f;
static const float WHITE_BACK_SPEED = 150.0f;
static const float WHITE_FAR_APPROACH_SPEED = 250.0f;
static const float WHITE_BULLET_SPEED = 250.0f;

// 時間関係
static const float WHITE_ACTION_TIME_SCALE = 1.5f;
static const float WHITE_RECHECK_TIME = 0.2f * WHITE_ACTION_TIME_SCALE;
static const float WHITE_ANIM_FPS = 24.0f;
static const float WHITE_FRAME_TIME = 1.0f / WHITE_ANIM_FPS;

// 手裏剣攻撃
static const float WHITE_SHURIKEN_COOLDOWN = 0.45f * WHITE_ACTION_TIME_SCALE;
static const int WHITE_SHURIKEN_DAMAGE = 5;
static const float WHITE_SHURIKEN_SHOT_TIME = 16.0f / WHITE_ANIM_FPS;
static const float WHITE_SHURIKEN_END_TIME = 24.0f / WHITE_ANIM_FPS;
static const Vector2d WHITE_SHURIKEN_BULLET_DRAW_SIZE = WHITE_SHURIKEN_BASE_DRAW_SIZE * WHITE_ENEMY_DRAW_SCALE;
static const float WHITE_SHURIKEN_BULLET_ROTATE_INTERVAL = 4.0f / 60.0f;
static const float WHITE_SHURIKEN_BULLET_ROTATE_STEP = 15.0f * 3.14159265f / 180.0f;

// 剣攻撃
static const float WHITE_SWORD_COOLDOWN = 0.15f * WHITE_ACTION_TIME_SCALE;
static const float WHITE_SWORD_ATTACK_RANGE = WHITE_BACK_RANGE;
static const float WHITE_SWORD_HEIGHT_RANGE = 80.0f;
static const int WHITE_SWORD_DAMAGE = 10;
static const float WHITE_SWORD_KNOCKBACK_X = 300.0f;
static const float WHITE_SWORD_KNOCKBACK_Y = -200.0f;
static const float WHITE_SWORD_HIT_TIME = 20.0f / WHITE_ANIM_FPS;
static const float WHITE_SWORD_ACTIVE_END_TIME = 29.0f / WHITE_ANIM_FPS;
static const float WHITE_SWORD_END_TIME = 35.0f / WHITE_ANIM_FPS;

// 画像参照
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

// ダメージ関係
static const int WHITE_BLOW_MIN_DAMAGE = 10;
static const int WHITE_BLOW_LARGE_MIN_DAMAGE = 25;
static const int WHITE_DEBUG_WEAK_DAMAGE = 1;
static const int WHITE_DEBUG_BLOW_DAMAGE = WHITE_BLOW_MIN_DAMAGE;
static const int WHITE_DEBUG_LARGE_BLOW_DAMAGE = WHITE_BLOW_LARGE_MIN_DAMAGE;
static const Vector2d WHITE_DEBUG_DAMAGE_KNOCKBACK = Vector2d(300.0f, -160.0f);
static const int WHITE_DAMAGE_NONE = 0;
static const int WHITE_DAMAGE_WEAK = 1;
static const int WHITE_DAMAGE_BLOW = 2;
static const int WHITE_DAMAGE_BLOW_LARGE = 3;
static const int WHITE_DAMAGE_DEAD = 4;
static const int WHITE_LARGE_BLOW_GROUND_FRAME_INDEX = 9;
static const int WHITE_DEAD_FRAME = 37;
static const float WHITE_DEAD_SHOW_TIME = 6.0f * WHITE_FRAME_TIME;

// 攻撃アニメーション
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

// 移動アニメーション
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

// ダメージアニメーション
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

void WhiteEnemyEntity::SetFacing(bool flipH)
{
	if (m_transform == nullptr)
	{
		return;
	}

	Vector2d scale = m_transform->GetScale();
	float scaleX = std::fabs(scale.x);
	scale.x = flipH ? -scaleX : scaleX;
	m_transform->SetScale(scale);
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
	float totalDuration = 0.0f;
	for (float duration : frameDurations)
	{
		totalDuration += duration;
	}
	clip.speed = frameDurations.empty()
		? (2.0f / WHITE_ANIM_FPS) * WHITE_ACTION_TIME_SCALE
		: totalDuration / static_cast<float>(frameDurations.size());
	clip.loop = loop;

	m_anim->AddClip(motionName, clip);
	m_anim->Play(motionName, true);
	m_currentMotionName = motionName;
}


void WhiteEnemyEntity::StartShurikenAttack()
{
	m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

	m_attackType = 1;
	m_attackTimer = 0.0f;

	m_attackActive = false;

	PlaySheetMotion("shuriken", WHITE_SHURIKEN_FRAMES, WHITE_SHURIKEN_DURATIONS, false);

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

void WhiteEnemyEntity::PlayDamageMotion(
	const std::string& motionName,
	const std::string& texturePath,
	int xNum,
	int yNum,
	const std::vector<int>& frames,
	const std::vector<float>& frameDurations
)
{
	if (m_sprite == nullptr || m_anim == nullptr || frames.empty())
	{
		return;
	}

	if (m_currentTexturePath != texturePath)
	{
		if (!m_sprite->LoadTextureDiv(texturePath, xNum, yNum))
		{
			return;
		}
		m_currentTexturePath = texturePath;
	}

	AnimationClip clip;
	clip.frames = BuildTimedFrames(frames, frameDurations, WHITE_FRAME_TIME);
	clip.speed = WHITE_FRAME_TIME;
	clip.loop = false;

	m_anim->AddClip(motionName, clip);
	m_anim->Play(motionName, true);
	m_currentMotionName = motionName;
}

void WhiteEnemyEntity::StartWeakHit()
{
	CancelAttackForDamage();

	PlayDamageMotion(
		"hit_weak",
		WHITE_TEXTURE_HIT_WEAK,
		WHITE_HIT_WEAK_X_NUM,
		WHITE_HIT_WEAK_Y_NUM,
		WHITE_WEAK_HIT_FRAMES,
		WHITE_WEAK_HIT_DURATIONS
	);
	m_damageState = WHITE_DAMAGE_WEAK;
	m_damageFrameIndex = 0;
	m_damageFrameTimer = 0.0f;
	m_damageHoldGroundFrame = false;
}

void WhiteEnemyEntity::StartBlowHit(const Vector2d& knockback)
{
	CancelAttackForDamage();
	m_velocity->SetVelocity(knockback);

	PlayDamageMotion(
		"hit_blow",
		WHITE_TEXTURE_SHEET,
		WHITE_SHEET_X_NUM,
		WHITE_SHEET_Y_NUM,
		WHITE_BLOW_FRAMES,
		WHITE_BLOW_DURATIONS
	);
	m_damageState = WHITE_DAMAGE_BLOW;
	m_damageFrameIndex = 0;
	m_damageFrameTimer = 0.0f;
	m_damageHoldGroundFrame = false;
}

void WhiteEnemyEntity::StartLargeBlowHit(const Vector2d& knockback)
{
	CancelAttackForDamage();
	m_velocity->SetVelocity(knockback);

	PlayDamageMotion(
		"hit_blow_large",
		WHITE_TEXTURE_SHEET,
		WHITE_SHEET_X_NUM,
		WHITE_SHEET_Y_NUM,
		WHITE_BLOW_LARGE_FRAMES,
		WHITE_BLOW_LARGE_DURATIONS
	);
	m_damageState = WHITE_DAMAGE_BLOW_LARGE;
	m_damageFrameIndex = 0;
	m_damageFrameTimer = 0.0f;
	m_damageHoldGroundFrame = false;
}

void WhiteEnemyEntity::StartDeadHit()
{
	CancelAttackForDamage();
	m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

	PlayDamageMotion(
		"hit_dead",
		WHITE_TEXTURE_SHEET,
		WHITE_SHEET_X_NUM,
		WHITE_SHEET_Y_NUM,
		std::vector<int>{ WHITE_DEAD_FRAME },
		std::vector<float>{ WHITE_DEAD_SHOW_TIME }
	);
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

	if (damage >= WHITE_BLOW_LARGE_MIN_DAMAGE)
	{
		StartLargeBlowHit(damageKnockback);
	}
	else if (damage >= WHITE_BLOW_MIN_DAMAGE)
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

	const std::vector<float>* durations = nullptr;

	if (m_damageState == WHITE_DAMAGE_WEAK)
	{
		durations = &WHITE_WEAK_HIT_DURATIONS;
	}
	else if (m_damageState == WHITE_DAMAGE_BLOW)
	{
		durations = &WHITE_BLOW_DURATIONS;
	}
	else if (m_damageState == WHITE_DAMAGE_BLOW_LARGE)
	{
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

	if (durations == nullptr || durations->empty())
	{
		m_damageState = WHITE_DAMAGE_NONE;
		return;
	}

	float holdStartTime = 0.0f;
	if (m_damageState == WHITE_DAMAGE_BLOW_LARGE)
	{
		for (int i = 0; i < WHITE_LARGE_BLOW_GROUND_FRAME_INDEX && i < static_cast<int>(durations->size()); i++)
		{
			holdStartTime += (*durations)[i];
		}
	}

	if (m_damageHoldGroundFrame)
	{
		if (m_isGround == false)
		{
			return;
		}

		m_damageHoldGroundFrame = false;
	}

	m_damageFrameTimer += deltaTime;

	if (m_damageState == WHITE_DAMAGE_BLOW_LARGE &&
		m_damageFrameTimer >= holdStartTime &&
		m_isGround == false)
	{
		if (m_sprite != nullptr && WHITE_LARGE_BLOW_GROUND_FRAME_INDEX < static_cast<int>(WHITE_BLOW_LARGE_FRAMES.size()))
		{
			m_sprite->SetFrame(WHITE_BLOW_LARGE_FRAMES[WHITE_LARGE_BLOW_GROUND_FRAME_INDEX]);
		}
		m_damageFrameTimer = holdStartTime;
		m_damageHoldGroundFrame = true;
		return;
	}

	if (m_damageFrameTimer >= GetTotalDuration(*durations))
	{
		m_damageState = WHITE_DAMAGE_NONE;
		m_actionLock = false;
		m_currentMotionName = "";
		m_damageHoldGroundFrame = false;
	}
}

void WhiteEnemyEntity::UpdateDebugDamageInput()
{
#ifdef _DEBUG
	if (m_scene == nullptr || m_hp == nullptr || IsDead() || m_hp->GetHP() <= 0)
	{
		return;
	}

	Vector2d playerPos = Vector2d::Zero();
	PlayerEntity* player = nullptr;
	bool playerOnGround = false;
	if (!TryGetPlayerInfo(playerPos, player, playerOnGround))
	{
		return;
	}

	WhiteEnemyEntity* closestWhite = nullptr;
	float closestDistanceSq = 0.0f;

	for (Actor* actor : m_scene->GetActors())
	{
		WhiteEnemyEntity* white = dynamic_cast<WhiteEnemyEntity*>(actor);
		if (white == nullptr || white->IsDead() || white->m_hp == nullptr || white->m_hp->GetHP() <= 0)
		{
			continue;
		}

		Vector2d diff = white->GetPos() - playerPos;
		float distanceSq = diff.lengthSq();
		if (closestWhite == nullptr || distanceSq < closestDistanceSq)
		{
			closestWhite = white;
			closestDistanceSq = distanceSq;
		}
	}

	if (closestWhite != this)
	{
		return;
	}

	static bool oldG = false;
	static bool oldH = false;
	static bool oldJ = false;

	bool nowG = CheckHitKey(KEY_INPUT_G) != 0;
	bool nowH = CheckHitKey(KEY_INPUT_H) != 0;
	bool nowJ = CheckHitKey(KEY_INPUT_J) != 0;

	if (nowG && !oldG)
	{
		TakeDamage(WHITE_DEBUG_WEAK_DAMAGE, WHITE_DEBUG_DAMAGE_KNOCKBACK);
	}
	else if (nowH && !oldH)
	{
		TakeDamage(WHITE_DEBUG_BLOW_DAMAGE, WHITE_DEBUG_DAMAGE_KNOCKBACK);
	}
	else if (nowJ && !oldJ)
	{
		TakeDamage(WHITE_DEBUG_LARGE_BLOW_DAMAGE, WHITE_DEBUG_DAMAGE_KNOCKBACK);
	}

	oldG = nowG;
	oldH = nowH;
	oldJ = nowJ;
#endif
}

void WhiteEnemyEntity::Update(float deltaTime)
{
	UpdateDebugDamageInput();

	if (m_damageState != WHITE_DAMAGE_NONE)
	{
		UpdateDamageMotion(deltaTime);
		if (m_anim != nullptr && m_damageState != WHITE_DAMAGE_NONE && m_damageHoldGroundFrame == false)
		{
			m_anim->Update(deltaTime);
		}
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

	SetFacing(m_attackType == 0 ? distanceX >= 0.0f : m_dir);


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

		状態一覧
		||whiteState||
		0 待機・索敵
		1 接近しながら手裏剣攻撃
		2 後退しながら剣攻撃の再使用を待つ
		3 停止して手裏剣攻撃
		4 停止して剣攻撃
		5 遠距離から接近
		6 死亡

		攻撃タイプ
		||attackType||
		0 攻撃なし
		1 手裏剣攻撃
		2 剣攻撃

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

			// 近距離では剣を使い、再使用待ちの間は後退する。
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
			// 手裏剣の射程まで接近する。
			PlaySheetMotion("walk", WHITE_WALK_FRAMES, WHITE_WALK_DURATIONS, true);
			PrepareMoveTracking(WHITE_APPROACH_SPEED * dir);
			m_velocity->SetVelocity(Vector2d(WHITE_APPROACH_SPEED * dir, 0.0f));

			if (distance < WHITE_BACK_RANGE || distance > WHITE_SHURIKEN_RANGE)
			{
				m_actionLock = false;
			}
			// 剣の再使用待ち中は間合いを取る。
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
			// 手裏剣の射程内で停止する。
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
			// 近距離で停止して剣を使う。
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
			// 死亡状態へ移行する。
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
				new WhiteShurikenBullet(
					m_scene,
					bulletPos,
					bulletVel,
					WHITE_BULLET_DELETE_RANGE,
					WHITE_TEXTURE_SHURIKEN_BULLET,
					WHITE_SHURIKEN_DAMAGE,
					WHITE_SHURIKEN_BULLET_DRAW_SIZE,
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

		SetFacing(m_dir);

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




