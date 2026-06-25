#pragma once
#include "UIActor.h"

class SpriteComponent;

class ShurikenUI : public UIActor
{
public:
    ShurikenUI(Scene* scene, float x, float y);
    void SetCount(int count);
    void Draw() override;

private:
    SpriteComponent* m_icon = nullptr; // ¬‚³‚Èè— Œ•ƒAƒCƒRƒ“
    int m_count = 0;
};
