#include "EffectActor.h"
#include "TransformComponent.h"
#include "SpriteComponent.h"


EffectActor::EffectActor(Scene* scene, const Vector2d& pos, const Vector2d& size)
	: Actor(scene),
	m_transform(nullptr),
	m_sprite(nullptr),
	m_initialPos(pos),
	m_initialSize(size)
{
}

bool EffectActor::Init() {

	m_transform = AddComponent<TransformComponent>();
	m_sprite = AddComponent<SpriteComponent>(GetTexturePath());

	m_transform->SetPosition(m_initialPos);
	m_sprite->SetSize(m_initialSize.x, m_initialSize.y);

	return true;
}

void EffectActor::SetPos(const Vector2d& pos) {
	if (m_transform) m_transform->SetPosition(pos);
}

Vector2d EffectActor::GetPos() const {
	return m_transform ? m_transform->GetPosition() : Vector2d::Zero();
}

void EffectActor::SetSize(const Vector2d& size) {
	if (m_sprite) m_sprite->SetSize(size.x, size.y);
}

Vector2d EffectActor::GetSize() const {
	return m_sprite ? Vector2d(m_sprite->GetWidth(), m_sprite->GetHeight()) : Vector2d::Zero();
}
