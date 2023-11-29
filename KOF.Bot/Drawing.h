#ifndef DRAWING_H
#define DRAWING_H

#include "pch.h"
#include "Bot.h"
#include "UI.h"

class Drawing
{
private:
	static ImVec2 vWindowSize;
	static ImGuiWindowFlags WindowFlags;

	static UI::WindowItem lpSelectedWindow;
	static LPDIRECT3DDEVICE9 pD3DDevice;

	static void DrawFilledRectangle(int x, int y, int w, int h, unsigned char r, unsigned char g, unsigned char b);

	static void DrawStatisticsController();
	static void DrawMainController();
	static void DrawProtectionController();
	static void DrawAutoLootController();
	static void DrawItemListController();
	static void DrawTransformationController();
	static void DrawSpeedController();
	static void DrawDistanceController();
	static void DrawAttackController();
	static void DrawTargetListController();
	static void DrawSkillController();
	static void DrawSizeController();
	static void DrawSaveCPUController();
	static void DrawSupplyController();
	static void DrawSupplyListController();
	static void DrawListenerController();
	static void DrawPartyController();
	static void DrawSettingsController();
	static void DrawModeController();
	static void DrawFlashController();

	static void DrawRoutePlannerController();
	static void DrawRouteListController();
	static void DrawMainSettingsArea();

	static void DrawExtraController();
	static void DrawLevelDownerController();

public:
	static bool isActive();
	static void Draw();
	static void DXDraw(LPDIRECT3DDEVICE9 pCurrentD3DDevice);
	static void SetLegalModeSettings(bool bMode);
	static void SetSpeedModeSettings(bool bMode);

	static Bot* Bot;
	static LPCSTR lpWindowName;
	static bool bDraw;
};

#endif