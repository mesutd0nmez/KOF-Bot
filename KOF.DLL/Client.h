#pragma once
#include "pch.h"
#include "Packet.h"
#include "Structures.h"
#include "Table.h"
#include "Define.h"
#include "Ini.h"

class Client
{

public:
	Client();
	virtual ~Client();

	static void Start();
	static void Stop();
	static bool IsWorking() { return m_bWorking; }
	static void MainProcess();
	static void HookProcess();
	static void BootstrapProcess();
	static void LostProcess();
	static void GameProcess();

	static State GetState() { return m_byState; }
	static void SetState(State eState) { m_byState = eState; }

	static TPlayer InitializeMySelf(Packet& pkt);
	static TPlayer InitializePlayer(Packet& pkt);
	static TNpc InitializeNpc(Packet& pkt);

	static void RecvProcess(BYTE* byBuffer, DWORD dwLength);
	static void SendProcess(BYTE* byBuffer, DWORD dwLength);

public:
	static DWORD GetRecvAddress();
	static int32_t GetID();
	static std::string GetName();
	static int16_t GetHp();
	static int16_t GetMaxHp();
	static int16_t GetMp();
	static int16_t GetMaxMp();
	static uint8_t GetZone();
	static uint32_t GetGold();
	static uint8_t GetLevel();
	static e_Nation GetNation();
	static e_Class GetClass();
	static uint64_t GetExp();
	static uint64_t GetMaxExp();
	static DWORD GetGoX();
	static DWORD GetGoY();
	static float GetX();
	static float GetY();
	static float GetZ();
	static uint8_t GetSkillPoint(int32_t Slot);

	static bool IsIntroPhase();
	static bool IsLoginPhase();
	static bool IsServerSelectPhase();
	static bool IsCharacterSelectPhase();
	static bool IsDisconnect();
	static bool IsCharacterLoaded();

	static void PushPhase(DWORD address);
	static void SetLoginInformation(std::string strAccountId, std::string strAccountPassword);
	static void ConnectLoginServer(bool bDisconnect = false);
	static void ConnectGameServer(BYTE byServerId);
	static void SelectCharacterSkip();
	static void SelectCharacter(BYTE byCharacterIndex);

	static void RouteStart(float fX, float fY, float fZ = 0.0f);

	static void SendPacket(Packet byBuffer);

	static void SetAddress(std::string strAddressName, DWORD dwAddress);
	static DWORD GetAddress(std::string strAddressName);

	static void Town();

	static void Move(int32_t iX, int32_t iY);

	static void HookRecvAddress();
	static void HookSendAddress();

	static DWORD GetRecvHookAddress();
	static DWORD GetSendHookAddress();

	static void LoadUserConfig(std::string strCharacterName);
	static Ini* GetUserConfig(std::string strCharacterName);
	static bool IsUserConfigLoaded(std::string strCharacterName) { return m_mapUserConfig.find(strCharacterName) != m_mapUserConfig.end(); };

protected:
	inline static std::map<std::string, DWORD> m_mapAddressList;

	inline static bool m_bWorking;
	inline static State m_byState;

	inline static DWORD m_dwRecvHookAddress;
	inline static DWORD m_dwSendHookAddress;

	inline static std::map<std::string, Ini*> m_mapUserConfig;

	inline static TPlayer m_PlayerMySelf;

	inline static bool m_bCharacterLoaded;

	inline static std::map<int32_t, TNpc> m_mapNpc;
	inline static std::map<int32_t, TPlayer> m_mapPlayer;

private:
	typedef void(__thiscall* Send)(DWORD, uint8_t*, uint32_t);
	typedef int(__thiscall* LoginCall1)(DWORD);
	typedef int(__thiscall* LoginCall2)(DWORD);
	typedef int(__thiscall* LoginServerCall)(DWORD);
	typedef int(__thiscall* CharacterSelectSkipCall)(DWORD);
	typedef int(__thiscall* CharacterSelectCall)(DWORD);
	typedef int(__thiscall* RouteStartCall)(int, Vector3*); //Rota2
	typedef int(__thiscall* MoveCall)(DWORD, int, int); //Rota2

	typedef int(__cdecl* PushPhaseCall)(int);
};

