#include "Drawing.h"
#include "Memory.h"
#include "Bot.h"
#include "Client.h"

LPCSTR Drawing::lpWindowName = "Discord";
ImVec2 Drawing::vWindowSize = { 500, 500 };
ImGuiWindowFlags Drawing::WindowFlags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize;
bool Drawing::m_bDraw = true;
ImVec2 initial_pos(1600, 0);

void Drawing::Draw()
{
	if (IsDrawable()/* && Bot::GetState() == Bot::State::GAME*/)
	{
		ImGui::SetNextWindowPos(initial_pos, ImGuiCond_Once);
		ImGui::SetNextWindowSize(vWindowSize);
		ImGui::SetNextWindowBgAlpha(1.0f);
		ImGui::Begin(lpWindowName, &m_bDraw, WindowFlags);
		{
			DrawGameController();
		}

		ImGui::End();
	}

#ifdef _WINDLL
	if (GetAsyncKeyState(VK_INSERT) & 1)
		m_bDraw = !m_bDraw;
#endif
}

void Drawing::DrawGameController()
{
	if (ImGui::BeginTabBar("Controller", ImGuiTabBarFlags_None))
	{

	}

	ImGui::EndTabBar();
}
