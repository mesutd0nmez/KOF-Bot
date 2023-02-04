#pragma once

#include "pch.h"

class ProtectionHandler
{
public:
	static void Start();
	static void Stop();

	static void MainProcess();

protected:
	inline static bool m_bWorking;
};

