#ifndef DRAWING_H
#define DRAWING_H

#include <imgui.h>
#include "Bot.h"

class Drawing
{
public:
	static Bot* Bot;
	static bool Done;

private:
	static LPCSTR lpWindowName;
	static ImVec2 vWindowSize;
	static ImGuiWindowFlags WindowFlags;
	static bool bDraw;

public:
	static void Active();
	static bool isActive();
	static void Draw();

	static void DrawGameController();
	static void CenteredText(std::string strValue);
	static void RightText(std::string strValue);

	static void DrawMainProtectionArea();
	static void DrawMainFeaturesArea();
	static void DrawMainAutoLootArea();

	static void DrawAutomatedAttackSkillTree();
	static void DrawAutomatedCharacterSkillTree();

	static void DrawMonsterListTree();
};

#endif