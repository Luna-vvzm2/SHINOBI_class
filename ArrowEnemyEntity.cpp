#include "ArrowEnemyEntity.h"
#include "VelocityComponent.h"
#include "HPComponent.h"
#include "Vector2d.h"

ArrowEnemyEntity::ArrowEnemyEntity(Scene* scene, const Vector2d& pos) : EnemyEntity(scene, pos, Vector2d(96, 190)), m_bulletCount(0)
{
}

bool ArrowEnemyEntity::Init() {
	if (!EnemyEntity::Init()) return false;

	m_hp = AddComponent<HPComponent>(100);

	return true;
}

void ArrowEnemyEntity::Update(float deltaTime) {
	Vector2d vel = m_velocity->Get();
	vel.x = -m_moveSpeed;
	m_velocity->Set(vel);
	EnemyEntity::Update(deltaTime);
}

std::string ArrowEnemyEntity::GetTexturePath() const {
	return "assets/images/enemy/arrowEnemy.png";
}
