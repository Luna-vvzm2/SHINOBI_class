#pragma once
#include "UIActor.h"
#include <string>

class HPComponent;
class TransformComponent;
class SpriteComponent;

class EnemyHPBar : public UIActor
{
public:
    EnemyHPBar(Scene* scene, HPComponent* hp, const std::string& framePath = "assets/images/uies/HP_enemy_black.png");
    virtual ~EnemyHPBar() = default;

    virtual bool Init() override;
    virtual void Update(float deltaTime) override;
    virtual void Draw() override;

    // スクリーン座標で位置を指定
    void SetPosition(float x, float y);

    void SetBarSize(float width, float height);

    void ShowFor(float seconds);
    void SetVisible(bool visible);

    // フレーム（枠）を相対でずらす（px）
    void SetFrameOffset(float offsetX, float offsetY);

    // transform が「左上座標」を表すか「中心座標」を表すか切替（デフォルト false = 左上）
    void SetPosIsCenter(bool center);

    // ゲージ幅/高さの比率（0..1）。例 0.9, 0.6
    void SetGaugeScale(float widthScale, float heightScale);

    // 内側パディング（左, top, right, bottom）px
    void SetPadding(float left, float top, float right, float bottom);

    void SetGaugeOffset(float offsetX, float offsetY);

    // 現在値/最大値を設定
    void SetMetsuValue(int value, int maxValue);
    // 満タンのとき表示する画像パス
    void SetMetsuFullImagePath(const std::string& path);
    void SetMetsuImageOffset(float offsetY); // 正の値で画像を上に移動
    // 滅ゲージの表示サイズ/オフセット調整
    void SetMetsuOffset(float offsetY, float height = 6.0f);
    void SetMetsuWidthScale(float widthScale); // 0..1 の比率（例 0.6 = 枠幅の60%）
private:
    HPComponent* m_hp = nullptr;

    TransformComponent* m_transform = nullptr;
    SpriteComponent* m_frameSprite = nullptr;

    float m_maxWidth = 63.0f;
    float m_height = 8.0f;

    std::string m_barImagePath;
    std::string m_frameImagePath;

    bool m_visible = false;

    // 調整パラメータ（デフォルト値）
    float m_frameOffsetX = 0.0f;
    float m_frameOffsetY = 0.0f;
    bool  m_posIsCenter = false;

    float m_gaugeWidthScale = 0.92f;
    float m_gaugeHeightScale = 0.6f;

    float m_padLeft = 4.0f;
    float m_padTop = 2.0f;
    float m_padRight = 4.0f;
    float m_padBottom = 2.0f;
    float m_gaugeOffsetX = 0.0f;
    float m_gaugeOffsetY = 0.0f;

    // --- 滅ゲージ用メンバ ---
    int m_metsu = 0;
    int m_metsuMax = 100;
    float m_metsuOffsetY = 6.0f; // HPバーの下からのオフセット
    float m_metsuImageOffsetY = 8.0f; // デフォルトで 8px 上に移動（お好みで調整）
    float m_metsuHeight = 6.0f;
    float m_metsuWidthScale = 0.75f;// 滅ゲージ幅のスケール（0..1）。1.0 = 内側幅いっぱい、0.5 = 半分など
    std::string m_metsuFullImagePath; // 満タン時に表示する画像
};
