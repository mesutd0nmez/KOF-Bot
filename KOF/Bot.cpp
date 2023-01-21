#include "Bot.h"
#include "Memory.h"
#include "Client.h"
#include "ClientProcessor.h"

Bot::State Bot::m_byState = State::INTRO;

void Bot::BotLostProcess()
{
	if (Client::IsDisconnect() && Client::IsServerSelectPhase())
		SetState(State::SERVER_SELECT);

	if (!Client::IsDisconnect() && Client::IsCharacterSelectPhase())
		SetState(State::CHARACTER_SELECT);
}

void Bot::IntroSkipProcess()
{
	while (!Client::IsIntroPhase())
		Sleep(1);

	Sleep(1000);

	Client::SetLoginPhase();

	Sleep(1000);

	SetState(State::LOGIN);
}

void Bot::LoginProcess()
{
	while (!Client::IsLoginPhase())
		Sleep(1);

	Sleep(1000);

	Client::SetLoginInformation("colinkazim", "ttCnkoSh1993");
	Sleep(1000);
	Client::ConnectLoginServer();

	Sleep(1000);

	SetState(State::SERVER_SELECT);
}

void Bot::ServerSelectProcess()
{
	std::time_t tStartTime = std::time(0);

	while (!Client::IsServerSelectPhase())
	{
		if (std::time(0) - tStartTime >= 15)
		{
			SetState(State::LOGIN);
			LoginProcess();
			return;
		}

		Sleep(1);
	}

	Sleep(1000);

	Client::SetServerIndex(1);
	Sleep(1000);
	Client::ConnectGameServer();

	Sleep(1000);

	SetState(State::CHARACTER_SELECT);
}

void Bot::CharacterSelectProcess()
{
	while (!Client::IsCharacterSelectPhase())
		Sleep(1);

	Sleep(1000);

	Client::SelectCharacterEnter();
	Sleep(1000);
	Client::SelectCharacter(1);

	Sleep(1000);

	while (!Client::IsCharacterLoaded())
		Sleep(1);

	SetState(State::GAME);

	ClientProcessor::Start();
}

void Bot::BotGameProcess()
{
	if (Client::IsDisconnect())
	{
		fprintf(stdout, "Client connection lost\n");
		SetState(State::LOST);
	}
}

void Bot::Process()
{
	if (Client::IsCharacterLoaded())
	{
		SetState(State::GAME);
		ClientProcessor::Start();
	}

	bool bDone = false;

	while (!bDone)
	{
		switch (GetState())
		{
			case State::LOST: BotLostProcess(); break;
			case State::INTRO: IntroSkipProcess(); break;
			case State::LOGIN: LoginProcess(); break;
			case State::SERVER_SELECT: ServerSelectProcess(); break;
			case State::CHARACTER_SELECT: CharacterSelectProcess(); break;
			case State::GAME: BotGameProcess();break;
		}

		Sleep(1);
	}
}