#include "ArmorEnemyEntity.h"
#include "VelocityComponent.h"
#include "HPComponent.h"
#include "Vector2d.h"

ArmorEnemyEntity::ArmorEnemyEntity(Scene* scene, const Vector2d& pos) : EnemyEntity(scene, pos, Vector2d(96, 190))
{
}

bool ArmorEnemyEntity::Init() {
	if (!EnemyEntity::Init()) return false;

	m_hp = AddComponent<HPComponent>(100);

	return true;
}

void ArmorEnemyEntity::Update(float deltaTime) {
	Vector2d vel = m_velocity->Get();
	vel.x = -m_moveSpeed;
	m_velocity->Set(vel);
	EnemyEntity::Update(deltaTime);
}

std::string ArmorEnemyEntity::GetTexturePath() const {
	return "assets/images/enemy/armorEnemy.png";
}
