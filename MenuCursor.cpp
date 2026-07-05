#include "MenuCursor.h"

int MenuCursor::GetCategory() const
{
    return m_category;
}

void MenuCursor::SetCategory(int category)
{
    m_category = category;
}

int MenuCursor::GetIndex() const
{
    return m_index;
}

void MenuCursor::SetIndex(int index)
{
    m_index = index;
}

void MenuCursor::SetPosition(
    int category,
    int index
)
{
    m_category = category;
    m_index = index;
}

void MenuCursor::ClampCategory(int maxCategory)
{
    if (m_category < 0)
        m_category = 0;

    if (m_category > maxCategory)
        m_category = maxCategory;
}

void MenuCursor::Update(
    const Input& input,
    float deltaTime
)
{
    //---------------------------------
    // ‰‰ñ“ü—Í
    //---------------------------------

    if (input.IsTrigger(Action::RIGHT))
        ++m_index;

    if (input.IsTrigger(Action::LEFT))
        --m_index;

    //---------------------------------
    // ’·‰Ÿ‚µ
    //---------------------------------

    if (
        input.IsDown(Action::RIGHT) ||
        input.IsDown(Action::LEFT) ||
        input.IsDown(Action::DOWN) ||
        input.IsDown(Action::UP)
        )
    {
        m_repeatTimer += deltaTime;

        if (m_repeatTimer >= RepeatDelay)
        {
            while (m_repeatTimer >= RepeatDelay + RepeatInterval)
            {
                m_repeatTimer -= RepeatInterval;

                if (input.IsDown(Action::RIGHT))
                    ++m_index;

                if (input.IsDown(Action::LEFT))
                    --m_index;
            }
        }
    }
    else
    {
        m_repeatTimer = 0.0f;
    }
}
void MenuCursor::ClampIndex(int maxIndex)
{
    if (m_index < 0)
        m_index = 0;

    if (m_index > maxIndex)
        m_index = maxIndex;
}
