#include "SkillMenu.h"

#include "PlayScene.h"
#include "Game.h"

#include <DxLib.h>
#include "MenuCursor.h"

SkillMenu::SkillMenu(Menu* owner)
    : MenuPage(owner)
{
}

void SkillMenu::DrawSkillSlot(
    int x,
    int y,
    const SkillData& skill
) const
{
    const int size = 60;

    int handle =
        skill.unlocked ?
        skill.iconHandle :
        m_owner->GetLockedSkillIcon();

    if (handle >= 0)
    {
        DrawExtendGraph(
            x,
            y,
            x + size,
            y + size,
            handle,
            TRUE
        );
    }
    else
    {
        DrawBox(
            x,
            y,
            x + size,
            y + size,
            GetColor(255, 255, 255),
            FALSE
        );
    }
}

void SkillMenu::Initialize()
{
    m_ninjutsu =
    {
        {"”Ep1","‹Zà–¾—p"},
        {"”Ep2","test"},
        {"”Ep3","test"},
        {"”Ep4","test"}
    };

    m_ninpou =
    {
        {"”E–@1","test"},
        {"”E–@2","test"},
        {"”E–@3","test"},
        {"”E–@4","test"},
        {"”E–@5","test"},
        {"”E–@6","test"},
        {"”E–@7","test"},
        {"”E–@8","test"}
    };

    m_ningi =
    {
        {"”E‹Z1","test"},
        {"”E‹Z2","test"},
        {"”E‹Z3","test"},
        {"”E‹Z4","test"},
        {"”E‹Z5","test"},
        {"”E‹Z6","test"},
        {"”E‹Z7","test"}
    };

    m_combat =
    {
        {"í“¬‹Z1","test"},
        {"í“¬‹Z2","test"},
        {"í“¬‹Z3","test"},
        {"í“¬‹Z4","test"},
        {"í“¬‹Z5","test"},
        {"í“¬‹Z6","test"},
        {"í“¬‹Z7","test"},
        {"í“¬‹Z8","test"},
        {"í“¬‹Z9","test"},

        {"í“¬‹Z10","test"},
        {"í“¬‹Z11","test"},
        {"í“¬‹Z12","test"},
        {"í“¬‹Z13","test"},
        {"í“¬‹Z14","test"},
        {"í“¬‹Z15","test"},
        {"í“¬‹Z16","test"},
        {"í“¬‹Z17","test"}
    };

    m_categories =
    {
        {"”Ep",&m_ninjutsu,false,4,120,160},

        {"”E–@",&m_ninpou,false,8,250,290},

        {"”E‹Z",&m_ningi,false,7,380,420},

        {"í“¬‹Z",&m_combat,true,9,510,550}
    };
}

void SkillMenu::Draw()
{

    auto& cursor = m_owner->GetCursor();

    const int startX = 50;
    const int size = 60;
    const int gap = 15;

    for (const auto& category : m_categories)
    {
        DrawString(
            startX,
            category.titleY,
            category.name.c_str(),
            GetColor(255, 255, 255)
        );

        for (int i = 0;
            i < category.skills->size();
            i++)
        {
            Vector2d pos =
                GetSkillPosition(category, i);

            DrawSkillSlot(
                static_cast<int>(pos.x),
                static_cast<int>(pos.y),
                (*category.skills)[i]
            );
        }
    }

    //----------------------------------
    // ƒJ[ƒ\ƒ‹
    //----------------------------------
    if (m_owner->GetCursorArea() == Menu::MenuCursorArea::SkillList)
    {
    if (cursor.GetCategory() < 0 ||
        cursor.GetCategory() >= static_cast<int>(m_categories.size()))
    {
        return;
    }

    auto& category =
        m_categories[cursor.GetCategory()];

    if (cursor.GetIndex() < 0 ||
        cursor.GetIndex() >= static_cast<int>(category.skills->size()))
    {
        return;
    }

    Vector2d pos =
        GetSkillPosition(
            category,
            cursor.GetIndex()
        );

    DrawBox(
        static_cast<int>(pos.x) - 5,
        static_cast<int>(pos.y) - 5,
        static_cast<int>(pos.x) + size + 5,
        static_cast<int>(pos.y) + size + 5,
        GetColor(255, 0, 0),
        FALSE
    );

    //----------------------------------
    // à–¾—“
    //----------------------------------

    DrawBox(
        800,
        150,
        1200,
        400,
        GetColor(80, 80, 80),
        TRUE
    );

    SkillData* skill =
        GetSelectedSkill();

    if (skill)
    {
        DrawString(
            975,
            430,
            skill->name.c_str(),
            GetColor(255, 255, 255)
        );

        DrawString(
            975,
            600,
            skill->description.c_str(),
            GetColor(255, 255, 255)
        );
    }
}
}

void SkillMenu::Update(float deltaTime)
{
    auto& cursor = m_owner->GetCursor();

    const Input& input =
        m_owner->GetGame()->GetInput();

    cursor.Update(input, deltaTime);

    //----------------------------------
    // ‰‰ñ“ü—Í
    //----------------------------------

    if (input.IsTrigger(Action::DOWN))
    {
        MoveDown();
    }

    if (input.IsTrigger(Action::UP))
    {
        MoveUp();
    }

    //----------------------------------
    // ’·‰Ÿ‚µ
    //----------------------------------

    if (input.IsDown(Action::DOWN) ||
        input.IsDown(Action::UP))
    {
        m_verticalRepeatTimer += deltaTime;

        if (m_verticalRepeatTimer >= VerticalRepeatDelay)
        {
            while (m_verticalRepeatTimer >=
                VerticalRepeatDelay + VerticalRepeatInterval)
            {
                m_verticalRepeatTimer -=
                    VerticalRepeatInterval;

                if (input.IsDown(Action::DOWN))
                    MoveDown();

                if (input.IsDown(Action::UP))
                    MoveUp();
            }
        }
    }
    else
    {
        m_verticalRepeatTimer = 0.0f;
    }

    cursor.ClampCategory(
        static_cast<int>(m_categories.size()) - 1
    );

    ClampCursor();
}

void SkillMenu::MoveDown()
{
    auto& cursor = m_owner->GetCursor();

    if (cursor.GetCategory() == 3)
    {
        if (cursor.GetIndex() < 9)
        {
            cursor.SetIndex(cursor.GetIndex() + 9);
        }
    }
    else
    {
        cursor.SetCategory(cursor.GetCategory() + 1);
    }
}

void SkillMenu::MoveUp()
{
    auto& cursor = m_owner->GetCursor();

    if (cursor.GetCategory() == 3)
    {
        if (cursor.GetIndex() >= 9)
        {
            cursor.SetIndex(cursor.GetIndex() - 9);
        }
        else
        {
            cursor.SetCategory(cursor.GetCategory() - 1);
        }
    }
    else
    {
        if (cursor.GetCategory() == 0)
        {
            m_owner->SetCursorArea(Menu::MenuCursorArea::Tab);
        }
        else
        {
            cursor.SetCategory(cursor.GetCategory() - 1);
        }
    }
}

void SkillMenu::ClampCursor()
{
    auto& cursor = m_owner->GetCursor();

    cursor.ClampCategory(
        static_cast<int>(m_categories.size()) - 1
    );

    auto& category =
        m_categories[cursor.GetCategory()];

    cursor.ClampIndex(
        static_cast<int>(category.skills->size()) - 1
    );
}

SkillData* SkillMenu::GetSelectedSkill()
{
    auto& cursor = m_owner->GetCursor();

    int category = cursor.GetCategory();

    if (category < 0 ||
        category >= m_categories.size())
    {
        return nullptr;
    }

    auto& skills =
        *m_categories[category].skills;

    int index =
        cursor.GetIndex();

    if (index < 0 ||
        index >= skills.size())
    {
        return nullptr;
    }

    return &skills[index];
}

Vector2d SkillMenu::GetSkillPosition(
    const SkillCategory& category,
    int index
) const
{
    const int startX = 50;
    const int size = 60;
    const int gap = 15;

    int columns =
        category.grid
        ? category.columns
        : static_cast<int>(category.skills->size());

    int x =
        startX
        + (index % columns) * (size + gap);

    int y =
        category.skillY
        + (index / columns) * (size + gap);

    return Vector2d(
        static_cast<float>(x),
        static_cast<float>(y)
    );
}