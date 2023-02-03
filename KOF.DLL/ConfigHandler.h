#pragma once
#include ".\pch.h"

class ConfigHandler
{
public:
	static void Start();
	static void Stop();

	static void MainProcess();
	static void GameProcess();

	static void LoadUserConfig(std::string strCharacterName);
	static Ini* GetUserConfig(std::string strCharacterName);
	static bool IsUserConfigLoaded(std::string strCharacterName) { return m_mapUserConfig.find(strCharacterName) != m_mapUserConfig.end(); };

protected:
	inline static bool m_bWorking;
	inline static std::map<std::string, Ini*> m_mapUserConfig;
};

