#ifndef UI_H
#define UI_H

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

	static bool CreateDeviceD3D(HWND hWnd);
	static void CleanupDeviceD3D();
	static void ResetDevice();
	static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

public:
	static void Render(Bot* pBot);

	static bool LoadTextureFromFile(std::string filename, PDIRECT3DTEXTURE9* out_texture, int* out_width, int* out_height);
	static bool LoadTextureFromMemory(std::vector<uint8_t> pImageRawData, PDIRECT3DTEXTURE9* out_texture);
};

#endif