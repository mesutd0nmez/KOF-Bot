#pragma once

#include "Client.h"
#include "Packet.h"

class Bot;
class ClientHandler : public Client
{
public:
	ClientHandler(Bot* pBot);
	~ClientHandler();

public:
	void InitializeHandler();
	void StartHandler();

private:
	TNpc InitializeNpc(Packet& pkt);
	TPlayer InitializePlayer(Packet& pkt);

private:
	virtual void OnReady();

	void PatchClient();

	void PatchRecvAddress(DWORD dwAddress);
	void PatchSendAddress();

	void MailSlotRecvProcess();
	void MailSlotSendProcess();

	void RecvProcess(BYTE* byBuffer, DWORD dwLength);
	void SendProcess(BYTE* byBuffer, DWORD dwLength);

	std::function<void(BYTE*, DWORD)> onClientRecvProcess;
	std::function<void(BYTE*, DWORD)> onClientSendProcess;

private:
	struct ClientHook
	{
		ClientHook(ClientHandler* pHandler)
		{
			m_ClientHandler = pHandler;
		}

		static void RecvProcess(BYTE* byBuffer, DWORD dwLength)
		{
			m_ClientHandler->onClientRecvProcess(byBuffer, dwLength);
		}

		static void SendProcess(BYTE* byBuffer, DWORD dwLength)
		{
			m_ClientHandler->onClientSendProcess(byBuffer, dwLength);
		}

		inline static ClientHandler* m_ClientHandler;
	};

	ClientHook* m_ClientHook;

private:
	typedef void(__thiscall* SendFunction)(DWORD, uint8_t*, uint32_t);
	typedef int(__thiscall* LoginRequestFunction1)(DWORD);
	typedef int(__thiscall* LoginRequestFunction2)(DWORD);
	typedef int(__thiscall* DisconnectFunction)(DWORD);
	typedef int(__thiscall* LoginServerFunction)(DWORD);
	typedef int(__thiscall* CharacterSelectSkipFunction)(DWORD);
	typedef int(__thiscall* CharacterSelectLeftFunction)(DWORD);
	typedef int(__thiscall* CharacterSelectRightFunction)(DWORD);
	typedef int(__thiscall* CharacterSelectFunction)(DWORD);
	typedef int(__cdecl* PushPhaseFunction)(int);
	typedef void(__thiscall* EquipOreadsFunction)(int, int, char);

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

private:
	void LoadSkillData();
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

	bool IsMovingToLoot() { return m_bIsMovingToLoot; }
	void SetMovingToLoot(bool bValue) { m_bIsMovingToLoot = bValue; }

private:
	std::vector<TLoot>* GetLootList() { return &m_vecLootList; }

	std::vector<TLoot> m_vecLootList;
	bool m_bIsMovingToLoot;

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

private:
	bool m_bMailSlotWorking;
	bool m_bWorking;
};



