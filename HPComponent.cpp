#include "HPComponent.h"
#include "Actor.h"
#include <algorithm>    // std::clamp

HPComponent::HPComponent(Actor* owner, int maxHP)
    : Component(owner)
    , m_owner(owner)
    , m_maxHP(maxHP)
    , m_hp(maxHP)
    , m_invincibleTimer(0.0f)
{
}

void HPComponent::Update(float deltaTime)
{
    if (m_invincibleTimer > 0.0f) {
        m_invincibleTimer -= deltaTime;
        if (m_invincibleTimer < 0.0f)
            m_invincibleTimer = 0.0f;
    }
}

void HPComponent::Damage(int amount)
{
    std::cout << "Damage ŒÄ‚Î‚ê‚½: " << amount << std::endl;

    int old = m_hp;
    m_hp = std::clamp(m_hp - amount, 0, m_maxHP);

    if (OnHPChanged) OnHPChanged(m_hp, old);

    if (m_hp == 0) {
        if (OnDeath) OnDeath();
        //m_owner->SetState(Actor::State::Dead); // •K—v‚È‚ç
    }
}

void HPComponent::Heal(int amount)
{
    int old = m_hp;
    m_hp = std::clamp(m_hp + amount, 0, m_maxHP);

    if (OnHPChanged) OnHPChanged(m_hp, old);
}
