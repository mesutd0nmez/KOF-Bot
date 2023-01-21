#include "ClientProcessor.h"
#include "Bot.h"
#include "Client.h"
#include <thread>

void ClientProcessor::Start()
{
	new std::thread([]() { ClientProcessor::MainProcess(); });
	new std::thread([]() { ClientProcessor::ProtectionProcess(); });
	new std::thread([]() { ClientProcessor::AttackProcess(); });
}

void ClientProcessor::MainProcess()
{
	while (Bot::GetState() == Bot::State::GAME)
	{
		fprintf(stdout, "Main Process Running every 1 second\n");
		Sleep(1000);
	}
}

void ClientProcessor::ProtectionProcess()
{
	while (Bot::GetState() == Bot::State::GAME)
	{
		fprintf(stdout, "Protection Process Running every 1 second\n");
		Sleep(1000);
	}
}

void ClientProcessor::AttackProcess()
{
	while (Bot::GetState() == Bot::State::GAME)
	{
		fprintf(stdout, "Attack Process Running every 1 second\n");
		Sleep(1000);
	}
}