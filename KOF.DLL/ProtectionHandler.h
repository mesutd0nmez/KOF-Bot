#pragma once

#include "pch.h"

class ProtectionHandler
{
public:
	static void Start();
	static void Stop();

	static void ProtectionProcess();
	static void GodModeProcess();
	static void MinorProcess();

	static void HealthPotionProcess();
	static void ManaPotionProcess();

protected:
	inline static bool m_bWorking;
};

