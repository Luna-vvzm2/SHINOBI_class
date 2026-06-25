#include "AnimationComponent.h"
#include "SpriteComponent.h"
#include "Actor.h"

// --------------------
// コンストラクタ: 自動で SpriteComponent を取得
// --------------------
AnimationComponent::AnimationComponent(Actor* owner)
    : Component(owner)
    , m_sprite(nullptr)
    , m_currentClip(nullptr)
    , m_timer(0.0f)
    , m_frameIndex(0)
{
}

// --------------------
// クリップ追加
// --------------------
void AnimationComponent::AddClip(const std::string& name, const AnimationClip& clip)
{
    m_clips[name] = clip;
}

// --------------------
// アニメ再生
// --------------------
void AnimationComponent::Play(const std::string& name, bool reset)
{
    if (m_clips.find(name) == m_clips.end())
        return;

    if (!reset && m_currentName == name)
        return;

    m_currentName = name;
    m_currentClip = &m_clips[name];

    m_timer = 0.0f;
    m_frameIndex = 0;

    if (m_sprite && !m_currentClip->frames.empty())
        m_sprite->SetFrame(m_currentClip->frames[0]);
}

bool AnimationComponent::IsFinished() const {
    if (!m_currentClip) return true;

    if (m_currentClip->loop) return false;

    return m_frameIndex >= m_currentClip->frames.size() - 1;
}

// --------------------
// 更新（フレーム切替）
// --------------------
void AnimationComponent::Update(float deltaTime)
{
    if (!m_currentClip || !m_sprite) return;

    const auto& frames = m_currentClip->frames;
    if (frames.empty()) return;

    int currentIndex = static_cast<int>(m_frameIndex);
    float duration = m_currentClip->speed;

    if (currentIndex >= 0 && currentIndex < static_cast<int>(m_currentClip->frameDurations.size()))
    {
        duration = m_currentClip->frameDurations[currentIndex];
    }

    m_timer += deltaTime;

    while (m_timer >= duration) {
        m_timer -= duration;
        m_frameIndex++;

        int last = static_cast<int>(frames.size()) - 1;

        if (m_frameIndex > last) {
            if (m_currentClip->loop)
                m_frameIndex = 0;
            else
                m_frameIndex = (float)last; // ループなしは最後のフレーム
        }

        if (!m_currentClip->loop && currentIndex == last)
        {
            break;
        }
    }
    m_sprite->SetFrame(frames[m_frameIndex]);
}