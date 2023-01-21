#ifndef DRAWING_H
#define DRAWING_H

#include <Windows.h>

#include "ImGui/imgui.h"

class Drawing
{
private:
	static LPCSTR lpWindowName;
	static ImVec2 vWindowSize;
	static ImGuiWindowFlags WindowFlags;
	static bool m_bDraw;

public:
	static void SetDraw(bool bDraw) { m_bDraw = bDraw;}
	static bool IsDrawable() { return m_bDraw; }
	static void Draw();
	static void DrawGameController();
};

#endif
