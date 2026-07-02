#include "EnemyBullet.h"

EnemyBullet::EnemyBullet(
    Scene* scene,
    const Vector2d& pos,
    const Vector2d& dir) 
    : EntityActor(scene, pos, Vector2d(32, 16))
    , m_dir(dir)
    , m_speed(500.0f)
{
}

void EnemyBullet::Update(float deltaTime)
{
    auto pos = GetPos();

    pos += m_dir * m_speed * deltaTime;

    SetPos(pos);

    EntityActor::Update(deltaTime);
}

std::string EnemyBullet::GetTexturePath() const
{
    return "assets/images/enemy/arrow.png";
}