#include "WhiteEnemyEntity.h"
#include "VelocityComponent.h"
#include "HPComponent.h"
#include "Vector2d.h"

WhiteEnemyEntity::WhiteEnemyEntity(Scene* scene, const Vector2d& pos) : EnemyEntity(scene, pos, Vector2d(96, 190)) ,m_bulletCount(0), m_attackOnce{}
{

}

bool WhiteEnemyEntity::Init() {
	if (!EnemyEntity::Init()) return false;

	return true;
}

void WhiteEnemyEntity::Update(float deltaTime) {
	Vector2d vel = m_velocity->Get();
	vel.x = -m_moveSpeed;
	m_velocity->Set(vel);
	EnemyEntity::Update(deltaTime);
}

std::string WhiteEnemyEntity::GetTexturePath() const {
	return "assets/images/enemy/whiteEnemy.png";
}
