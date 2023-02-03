#include "pch.h"
#include "Bot.h"
#include "LoginHandler.h"

void LoginHandler::Start()
{
	new std::thread([]() { MainProcess(); });
}

void LoginHandler::Stop()
{
	m_bWorking = false;
}

void LoginHandler::MainProcess()
{
	m_bWorking = true;

	while (m_bWorking)
	{
		Sleep(1000);

		if (!Client::IsWorking())
			continue;

		switch (Client::GetState())
		{
			case Client::State::INTRO: IntroSkipProcess(); break;
			case Client::State::LOGIN: LoginProcess(); break;
			case Client::State::SERVER_SELECT: ServerSelectProcess(); break;
			case Client::State::CHARACTER_SELECT: CharacterSelectProcess(); break;
		}
	}
}

void LoginHandler::IntroSkipProcess()
{
	while (!Client::IsIntroPhase())
		Sleep(1000);

	Client::PushPhase(Client::GetAddress("KO_PTR_LOGIN"));

	Sleep(1000);

	Client::SetState(Client::State::LOGIN);
}

void LoginHandler::LoginProcess()
{
	while (!Client::IsLoginPhase())
		Sleep(1000);

	Client::SetLoginInformation("colinkazim", "ttCnkoSh1993");
	Client::ConnectLoginServer();

	Sleep(1000);

	Client::SetState(Client::State::SERVER_SELECT);
}

void LoginHandler::ServerSelectProcess()
{
	std::time_t tStartTime = std::time(0);

	while (!Client::IsServerSelectPhase())
	{
		Sleep(1000);

		if (std::time(0) - tStartTime >= 15)
		{
			Client::SetState(Client::State::LOGIN);
			LoginProcess();
			return;
		}
	}

	Client::SetServerIndex(1);
	Sleep(1000);
	Client::ConnectGameServer();

	Sleep(1000);

	Client::SetState(Client::State::CHARACTER_SELECT);
}

void LoginHandler::CharacterSelectProcess()
{
	while (!Client::IsCharacterSelectPhase())
		Sleep(1000);

	Client::SelectCharacterEnter();
	Sleep(1000);
	Client::SelectCharacter(1);

	while (!Client::IsCharacterLoaded())
		Sleep(1000);

	Client::SetState(Client::State::GAME);
}