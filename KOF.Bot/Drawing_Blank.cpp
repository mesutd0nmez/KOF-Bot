#include "pch.h"
#include "Drawing_Blank.h"
#include "Bot.h"
#include "Client.h"
#include "ClientHandler.h"
#include "UI.h"
#include "RouteManager.h"

#ifdef UI_BLANK

Bot* Drawing::Bot = nullptr;
bool Drawing::bDraw = true;
bool Drawing::Done = false;

ClientHandler* m_pClient = nullptr;
Ini* m_pUserConfiguration = nullptr;
Ini* m_pAppConfiguration = nullptr;

PDIRECT3DTEXTURE9 m_pMapTexture = nullptr;
PDIRECT3DTEXTURE9 m_pMinimapTexture = nullptr;

float m_fScreenWidth = 0.0f;
float m_fScreenHeight = 0.0f;

bool m_bInitPos = false;

void Drawing::Active()
{
	bDraw = true;
}

bool Drawing::isActive()
{
	return bDraw == true;
}

void Drawing::Initialize()
{
    m_fScreenWidth = (GetSystemMetrics(SM_CXSCREEN)) / 2.0f;
    m_fScreenHeight = (GetSystemMetrics(SM_CYSCREEN)) / 2.0f;
}

void Drawing::InitializeSceneData()
{
    m_pUserConfiguration = Drawing::Bot->GetUserConfiguration();
    m_pAppConfiguration = Drawing::Bot->GetAppConfiguration();
    m_pClient = Drawing::Bot->GetClientHandler();
}

void Drawing::Draw()
{
    bool bTableLoaded = Drawing::Bot->IsTableLoaded();

    if (!bTableLoaded)
        ImGui::BeginDisabled();

	if (isActive())
	{
        InitializeSceneData();

        ImVec2 ScreenRes { 0, 0 };
        ImVec2 WindowPos { 0, 0 };
        ImVec2 WindowSize { 658, 700 };

        if (m_bInitPos == false)
        {
            RECT ScreenRect;
            GetWindowRect(GetDesktopWindow(), &ScreenRect);
            ScreenRes = ImVec2(float(ScreenRect.right), float(ScreenRect.bottom));
            WindowPos.x = (ScreenRes.x - WindowSize.x) * 0.5f;
            WindowPos.y = (ScreenRes.y - WindowSize.y) * 0.5f;
            m_bInitPos = true;
        }

        ImGui::SetNextWindowPos(ImVec2(WindowPos.x, WindowPos.y), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(WindowSize.x, WindowSize.y));
        ImGui::SetNextWindowBgAlpha(1.0f);

        ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize;
		ImGui::Begin(UI::m_szMainWindowName.c_str(), &bDraw, WindowFlags);
		{
            DrawScene();
		}

        ImGui::End();
	}

    if (!bTableLoaded)
        ImGui::EndDisabled();
}

void Drawing::DrawScene()
{
    ImGui::Text(skCryptDec("Hello World!"));
}

#endif