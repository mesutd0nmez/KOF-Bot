#pragma once

#include "pch.h"

class AttackHandler
{
public:
	static void Start();
	static void Stop();

	static void MainProcess();

protected:
	inline static bool m_bWorking;
};

