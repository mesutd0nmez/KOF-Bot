#ifndef DRAWING_H
#define DRAWING_H

#include <imgui.h>
#include "Bot.h"

class Drawing
{
public:
	static Bot* Bot;
	static bool Done;
	static bool bDraw;

private:
	static bool bDrawRoutePlanner;

	static void InitializeSceneData();

public:
	static std::string m_szMainWindowName;
	static std::string m_szRoutePlannerWindowName;

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
	static void DrawMainSettingsArea();
	static void DrawMainPriestArea();

	static void DrawRoutePlannerArea();

	static void DrawAutomatedAttackSkillTree();
	static void DrawAutomatedCharacterSkillTree();

	static void DrawMonsterListTree();
};

#endif