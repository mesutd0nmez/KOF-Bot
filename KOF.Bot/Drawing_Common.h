#ifndef DRAWING_H
#define DRAWING_H

#include "Bot.h"

#ifdef UI_COMMON

class Drawing
{
public:
	static Bot* Bot;
	static bool Done;
	static bool bDraw;
	static bool bDrawRoutePlanner;
	static bool bDrawInventory;

private:
	static void InitializeSceneData();

public:
	static void Initialize();

	static void Active();
	static bool isActive();
	static void Draw();

	static void DrawScene();

	static void DrawProtectionArea();
	static void DrawMainSettingsArea();
	static void DrawTransformationArea();
	static void DrawPriestManagementArea();
	static void DrawRogueManagementArea();
	static void DrawSpeedController();
	static void DrawDistanceController();
	static void DrawTargetListController();
	static void DrawSkillController();
	static void DrawSizeController();
	static void DrawAttackSettingsController();
	static void DrawSupplySettingsArea();
	static void DrawSupplyListArea();
	static void DrawBottomControllerArea();
	static void DrawRouteListController();
	static void DrawRoutePlanner();
	static void DrawRoutePlannerArea();
	static void DrawFlashController();
	static void DrawListenerController();
	static void DrawSystemSettingsController();
	static void DrawInventory();
	static void DrawInventoryArea();
	static void DrawAutoLootArea();
};

#endif

#endif