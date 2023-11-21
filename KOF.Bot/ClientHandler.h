#pragma once

#include "Client.h"
#include "Packet.h"
#include "RouteManager.h"
#include "Struct.h"

class Bot;
class ClientHandler : public Client
{
public:
	ClientHandler(Bot* pBot);
	~ClientHandler();

	bool IsWorking() { return m_bWorking; }

	void Clear();

	Client* GetClient() { return static_cast<Client*>(this); }

public:
	void Initialize();
	void StartHandler();
	void StopHandler();
	void Process();
	void PatchSocket();

private:
	TNpc InitializeNpc(Packet& pkt);
	TPlayer InitializePlayer(Packet& pkt);

private:
	virtual void OnReady();

	void PatchRecvAddress(DWORD dwAddress);
	void hookRecv(long patchAddress, std::string MSName);
	void PatchSendAddress();

public:
	void MailSlotRecvProcess();
	void MailSlotSendProcess();

private:
	void RecvProcess(BYTE* byBuffer, DWORD dwLength);
	void SendProcess(BYTE* byBuffer, DWORD dwLength);

	std::function<void(BYTE*, DWORD)> onClientRecvProcess;
	std::function<void(BYTE*, DWORD)> onClientSendProcess;

	HANDLE m_hMailSlotRecv;
	std::string m_szMailSlotRecvName;

	HANDLE m_hMailSlotSend;
	std::string m_szMailSlotSendName;

	LPVOID m_RecvHookAddress;
	LPVOID m_SendHookAddress;

private:
	struct ClientHook
	{
		ClientHook(ClientHandler* pHandler)
		{
			m_ClientHandler = pHandler;
		}

		~ClientHook()
		{
			m_ClientHandler = nullptr;
		}

		static void RecvProcess(BYTE* byBuffer, DWORD dwLength)
		{
			try
			{
				m_ClientHandler->onClientRecvProcess(byBuffer, dwLength);
			}
			catch (const std::exception& e)
			{
#ifdef DEBUG
				printf("OnClientRecvProcess:Exception: %s\n", e.what());
#else
				UNREFERENCED_PARAMETER(e);
#endif
			}
		}

		static void SendProcess(BYTE* byBuffer, DWORD dwLength)
		{
			try
			{
				m_ClientHandler->onClientSendProcess(byBuffer, dwLength);
			}
			catch (const std::exception& e)
			{
#ifdef DEBUG
				printf("OnClientSendProcess:Exception: %s\n", e.what());
#else
				UNREFERENCED_PARAMETER(e);
#endif
			}
		}

		inline static ClientHandler* m_ClientHandler;
	};

	ClientHook* m_ClientHook;

private:
	void SetLoginInformation(std::string szAccountId, std::string szPassword);

public:
	void LoadSkillData();

private:
	void BasicAttackProcess();
	void AttackProcess();
	void MoveToTargetProcess();
	void SearchTargetProcess();

	void AutoLootProcess();

	void MinorProcess();
	void PotionProcess();
	void CharacterProcess();

	void PartySwiftProcess();
	void PriestCharacterProcess(int32_t iTargetID = -1, bool bIsPartyRequest = false, uint16_t iMaxHp = 0, uint16_t iHp = 0);

	bool HealthPotionProcess();
	bool ManaPotionProcess();

	void RouteProcess();

	void SupplyProcess();

	void LevelDownerProcess();

	void MagicHammerProcess();
	void SpeedHackProcess();

	void TransformationProcess();
	void FlashProcess();
	void PartyProcess();

	void VipWarehouseProcess();

	void RegionProcess();

private:
	bool m_bWorking;
	bool m_bMailSlotWorking;

private:
	std::string m_szAccountId;
	std::string m_szPassword;

public:
	void SetRoute(std::vector<Route> vecRoute);
	bool IsRouting() { return m_vecRoute.size() > 0; };
	RouteStepType GetRouteStep() { return m_iRouteStep; };
	void ClearRoute();

protected:
	RouteStepType m_iRouteStep;

private:
	std::vector<Route> m_vecRoute;

private:
	std::chrono::milliseconds m_msLastSupplyTime;

private:
	bool SolveCaptcha(std::vector<uint8_t> vecImageBuffer);

protected:
	std::chrono::milliseconds m_msLastSelectedTargetTime;

public:
	void Test1();
	void Test2();
	void Test3();

public:
	int GetRegionUserCount(bool bExceptPartyMember = false);

protected:
	uint8_t m_iOTPRetryCount;

protected:
	float m_fLastSearchTargetTime;
	float m_fLastAttackTime;
	float m_fLastCharacterProcessTime;
	float m_fLastMinorProcessTime;
	float m_fLastMagicHammerProcessTime;
	float m_fLastAutoLootProcessTime;
	float m_fLastTransformationProcessTime;
	float m_fLastFlashProcessTime;
	float m_fLastSpeedHackProcessTime;
	float m_fLastRegionProcessTime;
	float m_fLastMoveToTargetProcessTime;
	float m_fLastLootRequestTime;
	float m_fLastPotionProcessTime;

protected:
	int32_t PartyMemberNeedSwift();
	int32_t PartyMemberNeedHeal(uint32_t iSkillBaseID);
	int32_t PartyMemberNeedBuff(uint32_t iSkillBaseID);

protected:
	bool IsSkillHasZoneLimit(uint32_t iSkillBaseID);
};



