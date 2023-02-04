#include "pch.h"
#include "Client.h"
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
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		if (!Client::IsWorking())
			continue;

	}
}