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
	static bool bDraw;
	static bool bDrawRoutePlanner;

	static void InitializeSceneData();

public:
	static void Active();
	static bool isActive();
	static void Draw();
	static void DrawRoutePlanner();


	static void DrawGameController();
	static void CenteredText(std::string strValue);
	static void RightText(std::string strValue);

	static void DrawMainProtectionArea();
	static void DrawMainFeaturesArea();
	static void DrawMainAutoLootArea();
	static void DrawMainSupplyArea();
	static void DrawMainSettingsArea();

	static void DrawRoutePlannerArea();

	static void DrawAutomatedAttackSkillTree();
	static void DrawAutomatedCharacterSkillTree();

	static void DrawMonsterListTree();
};

#endif