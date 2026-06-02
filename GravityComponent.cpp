#include "GravityComponent.h"
#include "VelocityComponent.h"
#include "Actor.h"


GravityComponent::GravityComponent(Actor* owner, float gravity)
	: Component(owner),
	m_gravity(gravity)
{
}

void GravityComponent::Update(float deltaTime) {

	if (!m_owner) return;

	auto velocity = m_owner->GetComponent<VelocityComponent>();
	if (velocity) {
		auto vel = velocity->Get();
		vel.y += m_gravity * deltaTime;

		velocity->Set(vel);
	}

}