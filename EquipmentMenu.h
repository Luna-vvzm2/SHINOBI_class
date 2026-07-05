#pragma once
#include "MenuPage.h"

#include <vector>

class EquipmentMenu : public MenuPage
{
public:
    explicit EquipmentMenu(Menu* owner);

    void Initialize() override;
    void Draw() override;

protected:
    void ClampCursor() override;

private:

    void DrawNinjutsuSlots();
    void DrawGofuSlots();
    void DrawKatanaSlot();
    void DrawEquipmentInventory();
    void DrawEquipmentDescription();
};