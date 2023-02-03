#pragma once
#include "pch.h"
#include "Packet.h"
#include "Structures.h"
#include "Table.h"
#include <map>

class Client
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

	Client();
	virtual ~Client();

	static void Start();
	static void Stop();
	static bool IsWorking() { return m_bWorking; }
	static void MainProcess();
	static void LostProcess();
	static void GameProcess();

	static State GetState() { return m_byState; }
	static void SetState(State eState) { m_byState = eState; }

public:
	static DWORD GetRecvAddress();
	static DWORD GetRecvCallAddress();
	static DWORD GetID();
	static std::string GetName();
	static DWORD GetHp();
	static DWORD GetMaxHp();
	static DWORD GetMp();
	static DWORD GetMaxMp();
	static DWORD GetZone();
	static DWORD GetGold();
	static DWORD GetLevel();
	static DWORD GetNation();
	static DWORD GetClass();
	static DWORD GetExp();
	static DWORD GetMaxExp();
	static DWORD GetGoX();
	static DWORD GetGoY();
	static DWORD GetX();
	static DWORD GetY();
	static DWORD GetZ();
	static DWORD GetSkillPoint(int Slot);

	static bool IsIntroPhase();
	static bool IsLoginPhase();
	static bool IsServerSelectPhase();
	static bool IsCharacterSelectPhase();
	static bool IsDisconnect();
	static bool IsCharacterLoaded();

	static void PushPhase(DWORD address);
	static void SetLoginInformation(std::string strAccountId, std::string strAccountPassword);
	static void ConnectLoginServer();
	static void SetServerIndex(BYTE byServerId);
	static void ConnectGameServer();
	static void SelectCharacterEnter();
	static void SelectCharacter(BYTE byCharacterIndex);

	static void RouteStart(float fX, float fY, float fZ = 0.0f);

	static void SendPacket(Packet byBuffer);

	static void SetAddress(std::string strAddressName, DWORD dwAddress);
	static DWORD GetAddress(std::string strAddressName);

	static void Town();

	static void Move(int iX, int iY);

protected:
	inline static std::map<std::string, DWORD> m_mapAddressList;

	inline static bool m_bWorking;
	inline static State m_byState;

private:
	typedef void(__thiscall* Send)(DWORD, uint8_t*, uint32_t);
	typedef int(__thiscall* LoginCall1)(DWORD);
	typedef int(__thiscall* LoginCall2)(DWORD);
	typedef int(__thiscall* LoginServerCall)(DWORD);
	typedef int(__thiscall* CharacterSelectEnterCall)(DWORD);
	typedef int(__thiscall* CharacterSelectCall)(DWORD);
	typedef int(__thiscall* RouteStartCall)(int, Vector3*); //Rota2
	typedef int(__thiscall* MoveCall)(DWORD, int, int); //Rota2

	typedef int(__cdecl* PushPhaseCall)(int);
};

