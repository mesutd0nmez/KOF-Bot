#pragma once
#include "Define.h"
#include <Windows.h>

class Bot
{
public:
	enum State
	{
		LOST = 0,
		INTRO = 1,
		LOGIN = 2,
		SERVER_SELECT = 3,
		CHARACTER_SELECT = 4,
		GAME = 5
	};

	static State GetState() { return m_byState; }
private:
	static void IntroSkipProcess();
	static void LoginProcess();
	static void ServerSelectProcess();
	static void CharacterSelectProcess();
	static void BotLostProcess();
	static void BotGameProcess();
	static void SetState(State eState) { m_byState = eState; }

protected:
	static State m_byState;

public:
	static void Process();
};

