#include "pch.h"
#include "Bot.h"
#include "ProtectionHandler.h"

void ProtectionHandler::Start()
{
	new std::thread([]() { MainProcess(); });
}

void ProtectionHandler::Stop()
{
	m_bWorking = false;
}

void ProtectionHandler::MainProcess()
{
	m_bWorking = true;

	while (m_bWorking)
	{
		Sleep(1000);

		if (!Client::IsWorking())
			continue;

	}
}