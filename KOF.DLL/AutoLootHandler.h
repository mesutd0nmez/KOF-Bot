#pragma once

#include "pch.h"

class AutoLootHandler
{
public:
	static void Start();
	static void Stop();

	static void AutoLootProcess();

protected:
	inline static bool m_bWorking;
};
