#pragma once
#include "BossEntity.h"

class YoroiBossEntity : public BossEntity
{
public:

    YoroiBossEntity(Scene* scene, const Vector2d& pos, const Vector2d& size);

    bool Init() override;
    int GetMaxHP() const override { return 1000; }
protected:

    void UpdateAI(float deltaTime) override;
    void UpdateAttack(float deltaTime) override;


};

