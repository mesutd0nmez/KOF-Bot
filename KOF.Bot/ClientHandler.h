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


private:
	TNpc InitializeNpc(Packet& pkt);
	TPlayer InitializePlayer(Packet& pkt);

private:
	virtual void OnReady();

	void PatchClient();

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

#ifdef USE_MAILSLOT
	std::string m_szMailSlotRecvName;
	std::string m_szMailSlotSendName;

	LPVOID m_RecvHookAddress;
	LPVOID m_SendHookAddress;
#endif

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
	void PushPhase(DWORD dwAddress);
	void SetLoginInformation(std::string szAccountId, std::string szPassword);
	void ConnectLoginServer(bool bDisconnect = false);
	void ConnectGameServer(BYTE byServerId);

	void SelectCharacterSkip();
	void SelectCharacterLeft();
	void SelectCharacterRight();
	void SelectCharacter(BYTE byCharacterIndex);

private:
	void SendPacket(Packet byBuffer);

public:
	void LoadSkillData();

private:
	void UseSkill(TABLE_UPC_SKILL pSkillData, int32_t iTargetID);

	void SendStartSkillCastingAtTargetPacket(TABLE_UPC_SKILL pSkillData, int32_t iTargetID);
	void SendStartSkillCastingAtPosPacket(TABLE_UPC_SKILL pSkillData, Vector3 v3TargetPosition);
	void SendStartFlyingAtTargetPacket(TABLE_UPC_SKILL pSkillData, int32_t iTargetID, Vector3 v3TargetPosition, uint16_t arrowIndex = 0);
	void SendStartSkillMagicAtTargetPacket(TABLE_UPC_SKILL pSkillData, int32_t iTargetID, Vector3 v3TargetPosition, uint16_t arrowIndex = 0);
	void SendStartSkillMagicAtPosPacket(TABLE_UPC_SKILL pSkillData, Vector3 v3TargetPosition);
	void SendStartMagicAtTarget(TABLE_UPC_SKILL pSkillData, int32_t iTargetID, Vector3 v3TargetPosition, uint16_t arrowIndex = 0);
	void SendCancelSkillPacket(TABLE_UPC_SKILL pSkillData);

public:
	void SendTownPacket();
	void SetPosition(Vector3 v3Position);
	void SetMovePosition(Vector3 v3MovePosition);
	void SendShoppingMall(ShoppingMallType eType);
	void SendItemMovePacket(uint8_t iType, uint8_t iDirection, uint32_t iItemID, uint8_t iCurrentPosition, uint8_t iTargetPosition);
	void SendTargetHpRequest(int32_t iTargetID, bool bBroadcast);
	void SetTarget(int32_t iTargetID);

	void EquipOreads(int32_t iItemID);
	void SetOreads(bool bValue);

	bool UseItem(uint32_t iItemID);

private:
	void SendBasicAttackPacket(int32_t iTargetID, float fInterval = 1.0f, float fDistance = 2.0f);
	void SendMovePacket(Vector3 vecStartPosition, Vector3 vecTargetPosition, int16_t iMoveSpeed, uint8_t iMoveType);
	void SendRotation(float fRotation);
	void SendRequestBundleOpen(uint32_t iBundleID);
	void SendBundleItemGet(uint32_t iBundleID, uint32_t iItemID, int16_t iIndex);

private:
	bool IsConfigurationLoaded() { return m_bConfigurationLoaded; };

public:
	void SetConfigurationLoaded(bool bValue) { m_bConfigurationLoaded = bValue; };

private:
	bool m_bConfigurationLoaded;

private:
	void BasicAttackProcess();
	void AttackProcess();
	void SearchTargetProcess();

	void AutoLootProcess();
	void CharacterProcess();

	void ProtectionProcess();
	void GodModeProcess();
	void MinorProcess();

	void HealthPotionProcess();
	void ManaPotionProcess();

	void RouteProcess();

	void SupplyProcess();

	void PriestBuffProcess();
	void PriestHealProcess();

private:
	bool m_bWorking;

#ifdef USE_MAILSLOT
private:
	bool m_bMailSlotWorking;
#endif

private:
	std::string m_szAccountId;
	std::string m_szPassword;

public:
	void PatchDeathEffect(bool bValue);

private:
	std::vector<uint8_t> m_vecOrigDeathEffectFunction;

public:
	void SetRoute(std::vector<Route> vecRoute);
	bool IsRouting() { return m_vecRoute.size() > 0; };
	void ClearRoute();

private:
	std::vector<Route> m_vecRoute;
	std::recursive_mutex m_vecRouteLock;

private:
	void SendNpcEvent(int32_t iTargetID);
	void SendItemTradeBuy(uint32_t iSellingGroup, int32_t iNpcId, int32_t iItemId, uint8_t iInventoryPosition, int16_t iCount, uint8_t iShopPage, uint8_t iShopPosition);
	void SendItemTradeBuy(uint32_t iSellingGroup, int32_t iNpcId, std::vector<SSItemBuy> vecItemList);
	void SendItemTradeSell(uint32_t iSellingGroup, int32_t iNpcId, int32_t iItemId, uint8_t iInventoryPosition, int16_t iCount);
	void SendItemTradeSell(uint32_t iSellingGroup, int32_t iNpcId, std::vector<SSItemSell> vecItemList);
	void SendItemRepair(uint8_t iDirection, uint8_t iInventoryPosition, int32_t iNpcId, int32_t iItemId);

private:
	bool IsNeedRepair();
	bool IsNeedSupply();

private:
	std::recursive_mutex m_UseSkillLock;

private:
	std::chrono::milliseconds m_msLastSupplyTime;
};



