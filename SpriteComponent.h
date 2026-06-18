#pragma once
#include "Component.h"
#include <string>
#include <vector>
#include <unordered_map>
#include "Vector2d.h"

class Renderer;

// -------------------------------------------
// SpriteComponent: 描画のみ担当
// アニメーションフレームを AnimationComponent から渡す
// -------------------------------------------
class SpriteComponent : public Component {
public:
    SpriteComponent(Actor* actor); // 引数なしコンストラクタ
    SpriteComponent(Actor* actor, const std::string& texturePath);
    ~SpriteComponent() override;

    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;

    // 分割画像の読み込み
    bool LoadTextureDiv(const std::string& path, int xNum, int yNum);
    void SetFrame(int index);   // 指定フレームに切り替え
    void SetTexture(int handle); // 指定ハンドルに切り替え

    bool LoadTexture(const std::string& path);
    void SetSize(float w, float h);

    static void ReleaseTextures();

    float GetWidth() const { return m_width; }
    float GetHeight() const { return m_height; }

    int GetCurrentFrame() const { return m_currentFrame; }
    void SetDrawSize(float w, float h) { m_drawW = w; m_drawH = h; }
    void SetFlipH(bool flipH) { m_flipH = flipH; }
    void SetRotation(float angle) { m_rotation = angle; }

    int GetHandle() const { return m_handle; }

private:
    int m_handle;
    float m_width;
    float m_height;
    std::string m_texturePath;
    static std::unordered_map<std::string, int> s_textureCache;

    float m_drawW = 0;
    float m_drawH = 0;
    bool m_flipH = false;
    float m_rotation = 0.0f;

    std::vector<int> m_frames;   // 分割されたフレームのハンドル
    int m_currentFrame = 0;      // 現在のフレーム番号
};