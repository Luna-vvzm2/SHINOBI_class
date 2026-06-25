#include "EnemyHPBar.h"
#include "HPComponent.h"
#include "TransformComponent.h"
#include "SpriteComponent.h"
#include "Game.h"
#include "Renderer.h"
#include "Color.h"
#include <algorithm>

EnemyHPBar::EnemyHPBar(Scene* scene, HPComponent* hp, const std::string& framePath)
    : UIActor(scene)
    , m_hp(hp)
    , m_frameImagePath(framePath)
{
    // Transform は UIActor でもコンポーネントとして持たせておく
    m_transform = AddComponent<TransformComponent>();
    SetName("EnemyHPBar");
}

bool EnemyHPBar::Init()
{
    if (!UIActor::Init()) return false;

    // 枠だけスプライトとして持つ
    m_frameSprite = AddComponent<SpriteComponent>(m_frameImagePath);

    if (m_frameSprite) {
        int handle = m_frameSprite->GetHandle();
        if (handle >= 0) {
            int imgW = 0, imgH = 0;
            if (GetGraphSize(handle, &imgW, &imgH) != -1 && imgW > 0 && imgH > 0) {
                // 画像のピクセルサイズをバーの論理サイズに使う
                m_maxWidth = static_cast<float>(imgW);
                m_height = static_cast<float>(imgH);

                // スプライトにそのままのサイズをセット（描画でスケールせずに済む）
                m_frameSprite->SetSize(m_maxWidth, m_height);
            }
        }
    }

    m_transform->SetPosition({ 0.0f, 0.0f });

    // 初期は非表示
    m_visible = false;

    return true;
}

// Update の該当部分
void EnemyHPBar::Update(float deltaTime)
{
    
}

void EnemyHPBar::Draw()
{
    if (!m_visible) return;
    if (!m_hp) return;

    auto renderer = m_scene->GetGame()->GetRenderer();
    if (!renderer) return;

    // transform の位置を取得
    Vector2d basePos = m_transform->GetPosition();

    // transform が「中心座標」を表す場合は左上に変換
    Vector2d posLeftTop;
    if (m_posIsCenter) {
        posLeftTop.x = basePos.x - m_maxWidth * 0.5f;
        posLeftTop.y = basePos.y - m_height * 0.5f;
    }
    else {
        posLeftTop = basePos;
    }

    // フレームオフセットを反映
    posLeftTop.x += m_frameOffsetX;
    posLeftTop.y += m_frameOffsetY;

    // HP比率
    float ratio = static_cast<float>(m_hp->GetHP()) / static_cast<float>(m_hp->GetMaxHP());
    ratio = std::clamp(ratio, 0.0f, 1.0f);

    // --- 枠画像描画（安定ルート） ---
    bool frameDrawn = false;
    if (m_frameSprite) {
        int handle = m_frameSprite->GetHandle();
        if (handle >= 0) {
            int origW = 0, origH = 0;
            if (GetGraphSize(handle, &origW, &origH) != -1 && origW > 0 && origH > 0) {
                float scaleX = m_maxWidth / static_cast<float>(origW);
                // 描画 API: DrawSprite は中心座標を渡す仕様なので左上 -> 中心へ変換
                Vector2d centerPos(posLeftTop.x + m_maxWidth * 0.5f, posLeftTop.y + m_height * 0.5f);
                renderer->DrawSprite(centerPos, scaleX, 0.0f, handle, true, false);
                frameDrawn = true;
            }
            else {
                // フォールバック: DxLib 直描画（左上基準）
                DrawGraph(static_cast<int>(posLeftTop.x), static_cast<int>(posLeftTop.y), handle, TRUE);
                frameDrawn = true;
            }
        }
    }

    if (!frameDrawn) {
        // 画像がないときはシンプルな枠を描画
        renderer->DrawRect(posLeftTop, m_maxWidth, m_height, Color(0, 0, 0), true, false);
        renderer->DrawRect(posLeftTop, m_maxWidth, m_height, Color(255, 255, 255), false, false);
    }

    // --- 赤いゲージ（枠の内側）を計算して描画 ---
    // gaugeScale と padding を考慮して内側領域を決定
    float innerW = (std::max)(0.0f, m_maxWidth * m_gaugeWidthScale - (m_padLeft + m_padRight));
    float innerH = (std::max)(1.0f, (m_height - (m_padTop + m_padBottom)) * m_gaugeHeightScale);

    // 横方向は gaugeWidthScale の中心寄せ、縦方向も中心寄せ
    float innerX = posLeftTop.x + (m_maxWidth * (1.0f - m_gaugeWidthScale) * 0.5f) + m_padLeft;
    float innerY = posLeftTop.y + (m_height - (m_padTop + m_padBottom)) * 0.5f * (1.0f - m_gaugeHeightScale) + m_padTop;

    innerX += m_gaugeOffsetX;
    innerY += m_gaugeOffsetY;

    float drawW = innerW * ratio;

    renderer->DrawRect(Vector2d(innerX, innerY), drawW, innerH, Color(220, 32, 32), true, false);
}

void EnemyHPBar::SetPosition(float x, float y)
{
    if (m_transform) m_transform->SetPosition({ x, y });
}

void EnemyHPBar::SetBarSize(float width, float height)
{
    m_maxWidth = width;
    m_height = height;
    if (m_frameSprite) m_frameSprite->SetSize(width, height);
}

void EnemyHPBar::ShowFor(float /*seconds*/)
{
    m_visible = true;
}

void EnemyHPBar::SetVisible(bool visible)
{
    m_visible = visible;
}

void EnemyHPBar::SetFrameOffset(float offsetX, float offsetY) {
    m_frameOffsetX = offsetX;
    m_frameOffsetY = offsetY;
}

void EnemyHPBar::SetPosIsCenter(bool center) {
    m_posIsCenter = center;
}

void EnemyHPBar::SetGaugeScale(float widthScale, float heightScale) {
    m_gaugeWidthScale = std::clamp(widthScale, 0.0f, 1.0f);
    m_gaugeHeightScale = std::clamp(heightScale, 0.0f, 1.0f);
}

void EnemyHPBar::SetPadding(float left, float top, float right, float bottom) {
    m_padLeft = left;
    m_padTop = top;
    m_padRight = right;
    m_padBottom = bottom;
}

void EnemyHPBar::SetGaugeOffset(float offsetX, float offsetY) {
    m_gaugeOffsetX = offsetX;
    m_gaugeOffsetY = offsetY;
}