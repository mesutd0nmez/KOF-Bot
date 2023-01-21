#pragma once
#include "Define.h"
#include "Memory.h"

class Client
{

public:
	static bool IsIntroPhase() 
	{ 
		return Memory::Read4Byte(KO_PTR_INTRO) != 0; 
	};

	static bool IsLoginPhase() 
	{ 
		return Memory::Read4Byte(Memory::Read4Byte(KO_PTR_LOGIN) + KO_OFF_UI_LOGIN_INTRO) != 0; 
	};

	static bool IsServerSelectPhase() 
	{ 
		return Memory::Read4Byte(Memory::Read4Byte(Memory::Read4Byte(KO_PTR_LOGIN) + KO_OFF_UI_LOGIN_INTRO) + KO_OFF_LOGIN_SERVER_LIST_READY) != -1;
	};

	static bool IsCharacterSelectPhase() 
	{ 
		return Memory::Read4Byte(Memory::Read4Byte(KO_PTR_CHARACTER_SELECT) + KO_OFF_UI_CHARACTER_SELECT_TIMER) != 0;
	};

	static bool IsDisconnect()
	{
		return Memory::Read4Byte(Memory::Read4Byte(KO_PTR_PKT) + KO_OFF_DISCONNECT) == 0;
	};

	static bool IsCharacterLoaded()
	{
		return GetID() != 0;
	};

	static DWORD GetID()
	{
		return Memory::Read4Byte(Memory::Read4Byte(KO_PTR_CHR) + KO_OFF_ID);
	}

	static DWORD GetHp()
	{
		return Memory::Read4Byte(Memory::Read4Byte(KO_PTR_CHR) + KO_OFF_HP);
	}

	static DWORD GetMaxHp()
	{
		return Memory::Read4Byte(Memory::Read4Byte(KO_PTR_CHR) + KO_OFF_MAX_HP);
	}

	static DWORD GetMp()
	{
		return Memory::Read4Byte(Memory::Read4Byte(KO_PTR_CHR) + KO_OFF_MP);
	}

	static DWORD GetMaxMp()
	{
		return Memory::Read4Byte(Memory::Read4Byte(KO_PTR_CHR) + KO_OFF_MAX_MP);
	}

public:
	static void SetLoginPhase();

private:
	static void SetPhase(DWORD address);

public:
	static void SetLoginInformation(std::string strAccountId, std::string strAccountPassword);
	static void ConnectLoginServer();
	static void SetServerIndex(BYTE byServerId);
	static void ConnectGameServer();
	static void SelectCharacterEnter();
	static void SelectCharacter(BYTE byCharacterIndex);
};

