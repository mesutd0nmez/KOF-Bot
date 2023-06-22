#pragma once

#include "Client.h"
#include "Packet.h"
#include "RouteManager.h"

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
	void AttackProcess();
	void MoveToTargetProcess();
	void SearchTargetProcess();

	void AutoLootProcess();

	void MinorProcess();
	void PotionProcess();
	void CharacterProcess();

	void TransformationProcess();
	void RogueCharacterProcess(int32_t iTargetID = -1, bool bIsPartyRequest = false);
	void PriestCharacterProcess(int32_t iTargetID = -1, bool bIsPartyRequest = false, uint16_t iMaxHp = 0, uint16_t iHp = 0);

	bool HealthPotionProcess();
	bool ManaPotionProcess();

	void RouteProcess();

	void SupplyProcess();

	void LevelDownerProcess();

	void MagicHammerProcess();

private:
	bool m_bWorking;
	bool m_bMailSlotWorking;

private:
	std::string m_szAccountId;
	std::string m_szPassword;

public:
	void SetRoute(std::vector<Route> vecRoute);
	bool IsRouting() { return m_vecRoute.size() > 0; };
	RouteStepType GetRouteStep() { return iRouteStep; };
	void ClearRoute();

protected:
	RouteStepType iRouteStep;

private:
	std::vector<Route> m_vecRoute;

private:
	std::chrono::milliseconds m_msLastSupplyTime;

private:
	bool SolveCaptcha(std::vector<uint8_t> vecImageBuffer);

protected:
	std::chrono::milliseconds m_msLastSelectedTargetTime;

public:
	void Test();

public:
	int GetRegionUserCount(bool bExceptPartyMember = false);
};



