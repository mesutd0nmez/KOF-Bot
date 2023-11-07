#ifndef UI_H
#define UI_H

#ifdef UI_HAZAR
#include "Drawing_Hazar.h"
#endif
#ifdef UI_COMMON
#include "Drawing_Common.h"
#endif
#ifdef UI_DEFAULT
#include "Drawing.h"
#endif

#include "Bot.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class UI
{
private:
	static LPDIRECT3D9              g_pD3D;
	static LPDIRECT3DDEVICE9        g_pd3dDevice;
	static D3DPRESENT_PARAMETERS    g_d3dpp;

	static DWORD g_iLastFrameTime;
	static DWORD g_iFPSLimit;

public:
	static std::string m_szMainWindowName;
	static std::string m_szRoutePlannerWindowName;
	static std::string m_szInventoryWindowName;

	static bool CreateDeviceD3D(HWND hWnd);
	static void CleanupDeviceD3D();
	static void ResetDevice();
	static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

public:
	static void Render(Bot* pBot);

	static bool LoadTextureFromFile(std::string filename, PDIRECT3DTEXTURE9* out_texture, int* out_width, int* out_height);
	static bool LoadTextureFromMemory(std::vector<uint8_t> pImageRawData, PDIRECT3DTEXTURE9* out_texture);

private:
	static void StyleColorsHazar();
	static void StyleColorsDarkGreenBlue();
	static void StyleColorsWhite();

};

#endif