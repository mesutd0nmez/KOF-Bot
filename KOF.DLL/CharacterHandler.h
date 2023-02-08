#pragma once

#include "pch.h"

class CharacterHandler
{
public:
	static void Start();
	static void Stop();

	static void CharacterProcess();

protected:
	inline static bool m_bWorking;
};

