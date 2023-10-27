#ifndef DRAWING_H
#define DRAWING_H

#include "Bot.h"

#ifdef UI_DEFAULT

class Drawing
{
public:
	static Bot* Bot;
	static bool Done;
	static bool bDraw;

private:
	static bool bDrawRoutePlanner;
	static bool bDrawInventory;

	static void InitializeSceneData();

public:
	static void Initialize();

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
	static void DrawMainDeveloperOnlyArea();
	static void DrawMainSettingsArea();
	static void DrawMainTransformationArea();
	static void DrawMainListenerArea();
	static void DrawMainRogueArea();
	static void DrawMainPriestArea();

	static void DrawRoutePlannerArea();

	static void DrawAutomatedAttackSkillTree();
	static void DrawAutomatedCharacterSkillTree();

	static void DrawMonsterListTree();

	static void DrawInventory();
	static void DrawInventoryArea();
};

#endif

#endif