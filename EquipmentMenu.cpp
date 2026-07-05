#include "SkillMenu.h"

#include "Menu.h"
#include "Game.h"

#include <DxLib.h>

EquipmentMenu::EquipmentMenu(Menu* owner)
	: MenuPage(owner)
{
}

void EquipmentMenu::Initialize()
{
}

void EquipmentMenu::Draw()
{
    DrawNinjutsuSlots();
	DrawGofuSlots();
	DrawEquipmentInventory();
	DrawEquipmentDescription();
}

void EquipmentMenu::DrawNinjutsuSlots()
{
    const int x = 50;
    const int y = 120;
    const int size = 70;
    const int gap = 20;

    DrawString(
        x,
        y - 40,
        "”E–@",
        GetColor(255, 255, 255)
    );

    for (int i = 0; i < 4; i++)
    {
        DrawBox(
            x,
            y + i * (size + gap),
            x + size,
            y + i * (size + gap) + size,
            GetColor(255, 255, 255),
            FALSE
        );
    }
}

void EquipmentMenu::DrawGofuSlots()
{
	const int x = 350;
	const int y = 120;
	const int size = 70;
	const int gap = 30;

	DrawString(
		x,
		y - 40,
		"Œì•„",
		GetColor(255, 255, 255)
	);

	// Ž©“®”­“®
	DrawString(
		x,
		y + 20,
		"Ž©“®”­“®",
		GetColor(255, 255, 255)
	);

	DrawBox(
		x + 120,
		y,
		x + 120 + size,
		y + size,
		GetColor(255, 255, 255),
		FALSE
	);

	// ˜AŒ‚”­“®
	DrawString(
		x,
		y + size + gap + 20,
		"˜AŒ‚”­“®",
		GetColor(255, 255, 255)
	);

	DrawBox(
		x + 120,
		y + size + gap,
		x + 120 + size,
		y + size * 2 + gap,
		GetColor(255, 255, 255),
		FALSE
	);
}

void EquipmentMenu::DrawKatanaSlot()
{
	const int x = 350;
	const int y = 390;
	const int size = 70;

	DrawString(
		x,
		y - 40,
		"“",
		GetColor(255, 255, 255)
	);

	// “–¼
	DrawString(
		x,
		y + 20,
		"žOŒŽ",
		GetColor(255, 255, 255)
	);

	// ‘•”õ˜g
	DrawBox(
		x + 120,
		y,
		x + 120 + size,
		y + size,
		GetColor(255, 255, 255),
		FALSE
	);
}

void EquipmentMenu::DrawEquipmentInventory()
{
	const int startX = 50;
	const int startY = 560;

	const int size = 60;
	const int gap = 15;

	DrawString(
		startX,
		startY - 40,
		"ŠŽ‘•”õ",
		GetColor(255, 255, 255)
	);

	for (int i = 0; i < 16; i++)
	{
		int x = startX + (i % 8) * (size + gap);
		int y = startY + (i / 8) * (size + gap);

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

void EquipmentMenu::DrawEquipmentDescription()
{
	DrawBox(
		650,
		120,
		1220,
		680,
		GetColor(255, 255, 255),
		FALSE
	);

	DrawString(
		680,
		150,
		"à–¾",
		GetColor(255, 255, 255)
	);
}

void EquipmentMenu::ClampCursor()
{
	// ‚Ü‚¾‰½‚à‚µ‚È‚­‚ÄOK
}