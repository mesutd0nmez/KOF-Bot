#ifndef UI_H
#define UI_H

#include "pch.h"
#include "Bot.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class UI
{
private:
	static LPDIRECT3D9 pD3D;
	static LPDIRECT3DDEVICE9 pD3DDevice;
	static D3DPRESENT_PARAMETERS D3Dpp;
	static UINT g_ResizeWidth, g_ResizeHeight;
	static DWORD dTargetPID;

	static bool CreateDeviceD3D(HWND hWnd);
	static void CleanupDeviceD3D();
	static void ResetDevice();
	static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK EnumWind(const HWND hWindow, const LPARAM lPrams);
	static void GetWindow();
	static void MoveWindow(HWND hCurrentProcessWindow);
	static BOOL IsWindowFocus(HWND hCurrentProcessWindow);
	static BOOL CALLBACK EnumAllWind(HWND hWindow, LPARAM lPrams);
	static void GetProcessName(LPSTR lpProcessName, DWORD dPID);
	static BOOL IsWindowValid(HWND hCurrentWindow);
	static BOOL IsWindowCloaked(HWND hCurrentWindow);

public:
	static HMODULE hCurrentModule;
	static BOOL IsWindowAlive();

	struct WindowItem
	{
		HWND CurrentWindow;
		char CurrentWindowTitle[125];
		char CurrentProcessName[125];
	};

	static HWND hWindow;
	static HWND hTargetWindow;
	static BOOL bTargetSet;

	static void Render(Bot* pBot);
	static BOOL IsWindowTargeted();
	static void GetAllWindow(std::vector<WindowItem>* vWindowList);
	static void SetTargetWindow(HWND hWindow);
	static HWND GetProcessWindowHandle(DWORD targetProcessId);
	static void ClearTargetWindow();
};

#endif