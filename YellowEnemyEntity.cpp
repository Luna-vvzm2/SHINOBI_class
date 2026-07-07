#include "YellowEnemyEntity.h"
#include "VelocityComponent.h"
#include "HPComponent.h"
#include "Vector2d.h"

YellowEnemyEntity::YellowEnemyEntity(Scene* scene, const Vector2d& pos) : EnemyEntity(scene, pos, Vector2d(96, 190))
{
	printf("yellow Spawn\n");
}

bool YellowEnemyEntity::Init() {
	if (!EnemyEntity::Init()) return false;

	m_hp = AddComponent<HPComponent>(100);

	return true;
}

void YellowEnemyEntity::Update(float deltaTime) {
	Vector2d vel = m_velocity->Get();
	vel.x = -m_moveSpeed;
	m_velocity->Set(vel);
	EnemyEntity::Update(deltaTime);
}

std::string YellowEnemyEntity::GetTexturePath() const {
	return "assets/images/enemy/yellowEnemy.png";
}
