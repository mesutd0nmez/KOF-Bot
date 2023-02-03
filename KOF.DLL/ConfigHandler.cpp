#include "pch.h"
#include "Client.h"
#include "Bot.h"
#include "ConfigHandler.h"

void ConfigHandler::Start()
{
	new std::thread([]() { MainProcess(); });
}

void ConfigHandler::Stop()
{
	m_bWorking = false;
}

void ConfigHandler::MainProcess()
{
	m_bWorking = true;

	while (m_bWorking)
	{
		Sleep(1000);

		if (!Client::IsWorking())
			continue;

		switch (Client::GetState())
		{
			case Client::State::GAME: GameProcess(); break;
		}
	}
}

void ConfigHandler::GameProcess()
{
	if (Client::IsCharacterLoaded())
	{
		std::string strCharacterName = Client::GetName();

		if (!IsUserConfigLoaded(strCharacterName))
			LoadUserConfig(strCharacterName);
	}
}

void ConfigHandler::LoadUserConfig(std::string strCharacterName)
{
	auto it = m_mapUserConfig.find(strCharacterName);

	if (it == m_mapUserConfig.end())
	{
		std::stringstream strUserConfigFile;
		strUserConfigFile << ".\\" << Client::GetName().c_str() << ".ini";

		Ini* pIni = new Ini(strUserConfigFile.str().c_str());

		m_mapUserConfig.insert(std::pair<std::string, Ini*>(Client::GetName(), pIni));
	}

	fprintf(stdout, "%s - Config loaded\n", Client::GetName().c_str());
}

Ini* ConfigHandler::GetUserConfig(std::string strCharacterName)
{
	auto it = m_mapUserConfig.find(strCharacterName);

	if (it == m_mapUserConfig.end())
		return NULL;

	return it->second;
}