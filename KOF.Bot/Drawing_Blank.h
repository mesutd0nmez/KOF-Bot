#ifndef DRAWING_H
#define DRAWING_H

#include "Bot.h"

#ifdef UI_BLANK

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
};

#endif

#endif