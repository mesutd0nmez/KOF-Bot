#ifndef DRAWING_H
#define DRAWING_H

#include "Bot.h"

#ifdef UI_HAZAR

class Drawing
{
public:
	static Bot* Bot;
	static bool Done;
	static bool bDraw;

private:
	static void InitializeSceneData();

public:
	static void Initialize();

	static void Active();
	static bool isActive();
	static void Draw();

	static void DrawScene();

	static void DrawMainController();
	static void DrawProtectionController();
	static void DrawAutoLootController();
	static void DrawTransformationController();
	static void DrawSpeedController();
	static void DrawDistanceController();
	static void DrawAttackController();
	static void DrawIllegalController();
	static void DrawTargetListController();
	static void DrawSkillController();
	static void DrawSizeController();
	static void DrawSaveCPUController();
	static void DrawSupplyController();
	static void DrawListenerController();
	static void DrawPartyController();
	static void DrawSettingsController();
	
};

#endif

#endif