#pragma once
#include "Actor.h"
#include "Vector2d.h"

class TransformComponent;
class SpriteComponent;



class EffectActor : public Actor {
public:
	explicit EffectActor(Scene* scene, const Vector2d& pos = Vector2d::Zero(), const Vector2d& size = { 32, 32 });
	virtual ~EffectActor() = default;

	bool Init() override;

	void SetPos(const Vector2d& pos);
	Vector2d GetPos() const;

	void SetSize(const Vector2d& size);
	Vector2d GetSize() const;


protected:

	TransformComponent* m_transform;
	SpriteComponent* m_sprite;

	Vector2d m_initialPos;
	Vector2d m_initialSize;

	virtual std::string GetTexturePath() const = 0;

};

