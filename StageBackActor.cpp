#include "StageBackActor.h"

StageBackActor::StageBackActor(
    Scene* scene,
    const Vector2d& pos,
    int prevStage,
    int spawnIndex)
    : BlockActor(scene, pos)
    , m_prevStage(prevStage)
    , m_spawnIndex(spawnIndex)
{
}

bool StageBackActor::Init()
{
    return BlockActor::Init();
}