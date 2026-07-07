#pragma once
#include "Actor.h"
#include "Vector2d.h"

class TransformComponent;
class VelocityComponent;
class SpriteComponent;
class CollisionComponent;



class EntityActor : public Actor {
public:

    enum class ActionState {
        IDLE,

        RUN_START,
        RUN,
        STOP_SHORT,
        STOP_LONG,

        CHANGE_DIR,
        CHANGE_DIR_RUN,

        SQUAT_START,
        SQUAT,
        SQUAT_IDLE,
        SQUAT_WALK,

        JUMP_START,
        JUMP,
        JUMP_SECOND,
        FALL,
        JUMP_LANDING,

        ATTACK,
        WEAK_ATTACK1,
        WEAK_ATTACK2,
        WEAK_ATTACK3,
        ATTACK_END,
        WEAK_ATTACK4,

        WEAK_AIR_ATTACK1,
        WEAK_AIR_ATTACK2,
        WEAK_AIR_ATTACK3,
        HAYABUSA,
        HAYABUSA_HIT,
        HAYABUSA_GROUND,

        STRONG_ATTACK1,
        STRONG_ATTACK_END,
        STRONG_ATTACK2,

        SQUAT_ATTACK,
        WALL_ATTACK,

        KUNAI,
        KUNAI_AIR,
        KUNAI_SQUAT,

        HIT,
        HIT_GROUND,
        HIT_AIR,
        HIT_AIR_LANDING,
        HIT_TRAP,

        ROLL,
        HIEN,
        SENTEN,
        ROLL_LANDING,

        WALL_HOLD,
        WALL_JUMP,
        WALL_CLIMB,
        WALL_CLIMB_UP,

        CHASE,
        DEAD,
    };

    explicit EntityActor(Scene* scene, const Vector2d& pos = Vector2d::Zero(), const Vector2d& size = { 32, 32 });
    virtual ~EntityActor() = default;

    bool Init() override;

    void SetPos(const Vector2d& pos);
    Vector2d GetPos() const;

    void SetVel(const Vector2d& vel);
    Vector2d GetVel() const;

    void SetSize(const Vector2d& size);
    Vector2d GetSize() const;

    CollisionComponent* GetCollision() const { return m_collision; }

protected:

    TransformComponent* m_transform;
    VelocityComponent* m_velocity;
    SpriteComponent* m_sprite;
    CollisionComponent* m_collision;

    Vector2d m_initialPos;
    Vector2d m_initialVel;
    Vector2d m_initialSize;

    ActionState m_state;
    bool m_isGround;

    void MoveAndCollide(float deltaTime);
    virtual std::string GetTexturePath() const = 0;
};