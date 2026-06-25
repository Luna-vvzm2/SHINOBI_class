#pragma once
#include "UIActor.h"
#include "Vector2d.h"

class HPComponent;
class TransformComponent;
class SpriteComponent;

class HPBarUI : public UIActor
{
public:
    HPBarUI(Scene* scene, HPComponent* hp);
    virtual ~HPBarUI() = default;

    virtual bool Init() override;
    virtual void Update(float deltaTime) override;
    virtual void Draw() override;

    // HPBar のサイズを設定（最大値）
    void SetBarSize(float width, float height);

    // UI の位置設定
    void SetPosition(float x, float y);

    float GetBarHeight() const { return m_height; }
private:
    HPComponent* m_hp = nullptr;

    TransformComponent* m_transform = nullptr;
    // SpriteComponent* m_sprite = nullptr;
    SpriteComponent* m_backBar = nullptr;

    float m_maxWidth;
    float m_height;

    std::string m_barImagePath;
    std::string m_frameImagePath;

    Vector2d m_barOffset = { 0, 0 }; // フレーム内のゲージ位置

    float m_displayRatio = 1.0f; // 画面表示用のHP割合（0.0 ～ 1.0）
};