#pragma once

#include "Input.h"

class MenuCursor
{
public:

    int GetCategory() const;
    void SetCategory(int category);

    int GetIndex() const;
    void SetIndex(int index);

    void Update(
        const Input& input,
        float deltaTime
    );

    void SetPosition(
        int category,
        int index
    );

    void ClampCategory(
        int maxCategory
    );

    void ClampIndex(
        int maxIndex
    );

private:

    int m_category = 0;
    int m_index = 0;

    float m_repeatTimer = 0.0f;

    static constexpr float RepeatDelay = 0.25f;
    static constexpr float RepeatInterval = 0.08f;
};