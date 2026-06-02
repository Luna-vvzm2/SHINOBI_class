#include "HPBarUI.h"
#include "HPComponent.h"
#include "TransformComponent.h"
#include "SpriteComponent.h"
#include "Game.h"

HPBarUI::HPBarUI(Scene* scene, HPComponent* hpComp)
    : UIActor(scene)
    , m_hp(hpComp)
    , m_maxWidth(1.0f)
    , m_height(1.0f)
{
    m_transform = AddComponent<TransformComponent>();
    m_sprite = AddComponent<SpriteComponent>("assets/images/uies/hpBar.png");
    m_backBar = AddComponent<SpriteComponent>("assets/images/uies/hpBarFrame.png");
    if (!m_sprite) {
        std::cerr << "‰æ‘œ“Ç‚Ýž‚ÝŽ¸”s\n";
    }
    else {
        std::cout << "‰æ‘œ“Ç‚Ýž‚Ý¬Œ÷" << std::endl;
    }
    SetName("HPBarUI");
}

bool HPBarUI::Init()
{
    if (!UIActor::Init()) return false;

    m_transform->SetPosition({ 20, 20 });

    return true;
}

void HPBarUI::Update(float deltaTime)
{
    UIActor::Update(deltaTime);

    if (!m_hp) return;


}

void HPBarUI::Draw() {
    if (!m_sprite || !m_hp) return;

    auto transform = GetComponent<TransformComponent>();
    if (!transform) return;

    Vector2d pos = transform->GetPosition();

    // HPŠ„‡‚Å‰¡•‚ðŒvŽZ
    float ratio = (float)m_hp->GetHP() / m_hp->GetMaxHP();
    int w, h;
    GetGraphSize(m_backBar->GetHandle(), &w, &h);

    int ScaleX = 3;
    int ScaleY = ScaleX;

    // ƒXƒNƒŠ[ƒ“À•W•`‰æiƒJƒƒ‰‰e‹¿‚È‚µj
    m_scene->GetGame()->GetRenderer()->DrawSpriteEx(
        pos, ScaleX, ScaleY, 0.0f, m_backBar->GetHandle(),
        true, Vector2d(0, 0), 255, false, false, false
    );

    ScaleX = static_cast<int>(ScaleX * 63 * ratio);

    m_scene->GetGame()->GetRenderer()->DrawSpriteEx(
        { 110, 29 }, ScaleX, ScaleY, 0.0f, m_sprite->GetHandle(),
        true, Vector2d(0, 0), 255, false, false, false
    );
}

void HPBarUI::SetPosition(float x, float y)
{
    if (m_transform)
        m_transform->SetPosition({ x, y });
}

void HPBarUI::SetBarSize(float w, float h)
{
    m_maxWidth = w;
    m_height = h;
    if (m_sprite)
        m_sprite->SetSize(w, h);
}