#include "WhiteEnemyEntity.h"
#include "Scene.h"
#include "Actor.h"
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
{
}

bool WhiteEnemyEntity::Init()
{
	if (!EnemyEntity::Init()) return false;

	m_anim = AddComponent<AnimationComponent>();
	m_anim->SetSprite(m_sprite);

	return true;
}

//彼の状態遷移
static const float WHITE_FIND_RANGE = 800.0f;//索敵可能範囲
static const float WHITE_NEAR_SWORD_RANGE = 90.0f;//剣攻撃に移る範囲
static const float WHITE_BACK_RANGE = 220.0f;//後退に移る範囲
static const float WHITE_STOP_SHURIKEN_RANGE = 250.0f;//停止して手裏剣攻撃に移る範囲
static const float WHITE_SHURIKEN_RANGE = 900.0f;//手裏剣攻撃を狙う範囲
static const float WHITE_BULLET_DELETE_RANGE = 900.0f;//弾を消す距離

static const float WHITE_APPROACH_SPEED = 120.0f;//接近速度
static const float WHITE_BACK_SPEED = 180.0f;//後退速度
static const float WHITE_FAR_APPROACH_SPEED = 250.0f;//遠距離からの接近速度
static const float WHITE_BULLET_SPEED = 250.0f;//弾速

static const float WHITE_SHURIKEN_COOLDOWN = 0.4f;//手裏剣の攻撃不可時間
static const float WHITE_SWORD_COOLDOWN = 0.5f;//剣の攻撃不可時間
static const float WHITE_RECHECK_TIME = 0.2f;//行動を再判定するまでの時間

static const float WHITE_SWORD_ATTACK_RANGE = 120.0f;//剣攻撃の横範囲
static const float WHITE_SWORD_HEIGHT_RANGE = 80.0f;//剣攻撃の縦範囲
static const int WHITE_SWORD_DAMAGE = 10;//剣攻撃のダメージ
static const int WHITE_SHURIKEN_DAMAGE = 5;//手裏剣攻撃のダメージ

static const float WHITE_SHURIKEN_SHOT_TIME = 0.7f; // shuriken shot time
static const float WHITE_SHURIKEN_END_TIME = 0.8f;  // shuriken end time

static const float WHITE_SWORD_PRE_TIME = 0.5f;     // sword pre motion
static const float WHITE_SWORD_ATTACK_TIME = 0.3f;  // sword active motion
static const float WHITE_SWORD_AFTER_TIME = 0.3f;   // sword after motion

static const char* WHITE_TEXTURE_IDLE = "assets/images/enemy/white/idle.png";
static const char* WHITE_TEXTURE_WALK = "assets/images/enemy/white/walk.png";
static const char* WHITE_TEXTURE_SHURIKEN = "assets/images/enemy/white/shuriken.png";
static const char* WHITE_TEXTURE_SWORD_PRE = "assets/images/enemy/white/sword_pre.png";
static const char* WHITE_TEXTURE_SWORD = "assets/images/enemy/white/sword.png";
static const char* WHITE_TEXTURE_SWORD_AFTER = "assets/images/enemy/white/sword_after.png";

float WhiteEnemyEntity::GetDirSign() const
{
	return m_dir ? 1.0f : -1.0f;
}

bool WhiteEnemyEntity::TryGetPlayerInfo(Vector2d& playerPos, HPComponent*& playerHp) const
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
	clip.speed = frameSpeed;
	clip.loop = loop;

	m_anim->AddClip(motionName, clip);
	m_anim->Play(motionName, true);
	m_currentTexturePath = texturePath;
}

// 手裏剣攻撃開始呼び出し関数
void WhiteEnemyEntity::StartShurikenAttack()
{
	m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

	m_attackType = 1;
	m_attackTimer = 0.0f;

	m_attackActive = false;

	//アニメーションリセット 必要
	//m_anim->SetAnimation(1);こんな感じ
	PlayMotion("shuriken", WHITE_TEXTURE_SHURIKEN, 3, 0.08f, false);

	m_attackOnce = false;
	m_actionLock = true;
}

// 剣攻撃開始呼び出し関数
void WhiteEnemyEntity::StartSwordAttack()
{
	m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

	m_attackType = 2;
	m_attackTimer = 0.0f;

	m_attackActive = false;

	//アニメーションリセット　必要
	//m_anim->SetAnimation(1);こんな感じ
	PlayMotion("sword_pre", WHITE_TEXTURE_SWORD_PRE, 3, 0.12f, false);

	m_attackOnce = false;
	m_actionLock = true;
}

void WhiteEnemyEntity::Update(float deltaTime)
{
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

	// 弾がプレイヤーから離れすぎたら無効化
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
			PlayMotion("idle", WHITE_TEXTURE_IDLE, 4, 0.16f, true);
			m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

			//continue;
			EnemyEntity::Update(deltaTime);
			return;
		}
	}

	/*----------------

			状態一覧
			||whiteState||
			0 待機
			1 接近手裏剣攻撃
			2 後退手裏剣攻撃
			3 停止して手裏剣攻撃
			4 停止して剣攻撃
			5 接近
			6 死 nashi

			攻撃一覧
			||attackType||
			0 移動
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

			// 超近距離：剣
			if (distance < WHITE_NEAR_SWORD_RANGE)
			{
				if (m_cooldownTimer <= 0.0f)
				{
					m_whiteState = 4;
				}
				else
				{
					m_whiteState = 0;
				}
			}
			// 近すぎる：後退
			else if (distance < WHITE_BACK_RANGE)
			{
				m_whiteState = 2;
			}
			//ちょうどいい：停止して手裏剣
			else if (distance < WHITE_STOP_SHURIKEN_RANGE)
			{
				m_whiteState = 3;
			}
			// 遠い：接近しながら手裏剣待ち
			else if (distance < WHITE_SHURIKEN_RANGE)
			{
				m_whiteState = 1;
			}
			// 遠すぎる：接近のみ
			else
			{
				m_whiteState = 5;
			}

			m_actionTimer = 0.0f;
			m_actionLock = true;
		}

		m_actionTimer += deltaTime;

		float dir = GetDirSign();

		switch (m_whiteState)
		{
		case 0:
			PlayMotion("idle", WHITE_TEXTURE_IDLE, 4, 0.16f, true);
			m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));
			m_actionLock = false;
			break;

		case 1:
			// 接近しながら手裏剣クールタイム待ち
			PlayMotion("walk", WHITE_TEXTURE_WALK, 4, 0.12f, true);
			m_velocity->SetVelocity(Vector2d(WHITE_APPROACH_SPEED * dir, 0.0f));

			if (distance < WHITE_STOP_SHURIKEN_RANGE || distance > WHITE_SHURIKEN_RANGE)
			{
				m_actionLock = false;
			}

			if (m_cooldownTimer <= 0.0f)
			{
				StartShurikenAttack();
			}
			else if (m_actionTimer >= WHITE_RECHECK_TIME)
			{
				m_actionLock = false;
			}
			break;

		case 2:
			// 後退しながら手裏剣クールタイム待ち
			PlayMotion("walk", WHITE_TEXTURE_WALK, 4, 0.12f, true);
			m_velocity->SetVelocity(Vector2d(-WHITE_BACK_SPEED * dir, 0.0f));

			if (distance >= WHITE_BACK_RANGE || distance < WHITE_NEAR_SWORD_RANGE)
			{
				m_actionLock = false;
			}

			if (m_cooldownTimer <= 0.0f)
			{
				StartShurikenAttack();
			}
			else if (m_actionTimer >= WHITE_RECHECK_TIME)
			{
				m_actionLock = false;
			}
			break;

		case 3:
			// ちょうどいい距離なので停止して手裏剣
			PlayMotion("idle", WHITE_TEXTURE_IDLE, 4, 0.16f, true);
			m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

			if (distance < WHITE_BACK_RANGE || distance >= WHITE_SHURIKEN_RANGE)
			{
				m_actionLock = false;
			}

			if (m_cooldownTimer <= 0.0f)
			{
				StartShurikenAttack();
			}
			else if (m_actionTimer >= WHITE_RECHECK_TIME)
			{
				m_actionLock = false;
			}
			break;

		case 4:
			// 剣
			PlayMotion("idle", WHITE_TEXTURE_IDLE, 4, 0.16f, true);
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
			// 遠距離なので接近
			PlayMotion("walk", WHITE_TEXTURE_WALK, 4, 0.12f, true);
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
			//やられモーションのあとプレイヤーが画面外に行ったら待機へ
			//一旦は死にます

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

		// 0.7秒後に手裏剣発射

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

		// 手裏剣攻撃終了
		// 0.7秒後に攻撃、攻撃不可時間0.1秒へ移行

		if (attackTime >= WHITE_SHURIKEN_END_TIME)
		{
			m_attackType = 0;

			// 手裏剣の攻撃不可時間
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

		m_attackTimer += deltaTime;
		float attackTime = m_attackTimer;

		const float ATTACK_START = WHITE_SWORD_PRE_TIME;
		const float ATTACK_END = WHITE_SWORD_PRE_TIME + WHITE_SWORD_ATTACK_TIME;
		const float END_TIME = WHITE_SWORD_PRE_TIME + WHITE_SWORD_ATTACK_TIME + WHITE_SWORD_AFTER_TIME;

		if (attackTime < ATTACK_START)
		{
			PlayMotion("sword_pre", WHITE_TEXTURE_SWORD_PRE, 3, 0.12f, false);
			m_attackActive = false;
			m_attackType = 2;
		}
		else if (attackTime < ATTACK_END)
		{
			PlayMotion("sword", WHITE_TEXTURE_SWORD, 3, 0.08f, false);
			m_attackActive = true;
			m_attackType = 2;

			// 0.7秒後に剣攻撃発生
			if (m_attackOnce == false)
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
		}
		else if (attackTime < END_TIME)
		{
			PlayMotion("sword_after", WHITE_TEXTURE_SWORD_AFTER, 3, 0.12f, false);
			m_attackType = 2;
			m_attackActive = false;
		}
		// 剣攻撃終了
		// 0.7秒後に攻撃、攻撃不可時間0.1秒へ移行
		else
		{
			m_attackType = 0;

			// 剣の攻撃不可時間

			m_cooldownTimer = WHITE_SWORD_COOLDOWN;
			m_actionTimer = 0.0f;
			m_actionLock = false;
			m_attackOnce = false;
			m_attackActive = false;
			m_attackType = 0;
		}
	}

	// 敵本体の横移動
	//ここAIで修正したから怪しい

	// 敵本体の縦移動

	// 弾の移動
	/*for (int j = 0; j < 3; j++)
	{
		int bulletIndex = i * 3 + j;

		if (white->bulletActive[bulletIndex] == true)
		{
			white->bulletX[bulletIndex] += WHITE_BULLET_SPEED * white->bulletMuki[bulletIndex] * dt;

		}
	}*/

	EnemyEntity::Update(deltaTime);
}

std::string WhiteEnemyEntity::GetTexturePath() const
{
	return WHITE_TEXTURE_IDLE;
}
