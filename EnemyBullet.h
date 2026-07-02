#pragma once
#include "EntityActor.h"

class EnemyBullet : public EntityActor
{
public:

    EnemyBullet(
        Scene* scene,
        const Vector2d& pos,
        const Vector2d& dir);

    
    void Update(float deltaTime) override;

    std::string GetTexturePath() const override;

private:

    Vector2d m_dir;
    float m_speed;
};

