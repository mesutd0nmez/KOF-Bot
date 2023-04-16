#pragma once

#include "Ini.h"
#include "Packet.h"

class Bot;
class Client
{
public:
	Client();
	~Client();

	void Clear();

public:
	int32_t GetID(DWORD iBase = 0);
	int32_t GetProtoID(DWORD iBase = 0);
	std::string GetName(DWORD iBase = 0);
	int16_t GetHp(DWORD iBase = 0);
	int16_t GetMaxHp(DWORD iBase = 0);
	int16_t GetMp(DWORD iBase = 0);
	int16_t GetMaxMp(DWORD iBase = 0);
	uint8_t GetZone(DWORD iBase = 0);
	uint32_t GetGold(DWORD iBase = 0);
	uint8_t GetLevel(DWORD iBase = 0);
	uint8_t GetNation(DWORD iBase = 0);
	int32_t GetClass(DWORD iBase = 0);
	uint64_t GetExp(DWORD iBase = 0);
	uint64_t GetMaxExp(DWORD iBase = 0);

	uint8_t GetMoveState(DWORD iBase = 0);
	uint8_t GetActionState(DWORD iBase = 0);

	int32_t GetClientSelectedTarget();
	uint32_t GetClientSelectedTargetBase();

	float GetRadius(DWORD iBase = 0);
	float GetScale(DWORD iBase = 0);

	bool IsDisconnect();

	bool IsDeath(DWORD iBase = 0);

	float GetX(DWORD iBase = 0);
	float GetZ(DWORD iBase = 0);
	float GetY(DWORD iBase = 0);

	float GetGoX();
	float GetGoY();
	float GetGoZ();

	uint8_t GetAuthority(DWORD iBase = 0);
	void SetAuthority(uint8_t iAuthority);

	Vector3 GetPosition();
	Vector3 GetTargetPosition();
	int32_t GetTarget();

	DWORD GetEntityBase(int32_t iTargetId);

	bool IsRogue(int32_t eClass = CLASS_UNKNOWN);
	bool IsMage(int32_t eClass = CLASS_UNKNOWN);
	bool IsWarrior(int32_t eClass = CLASS_UNKNOWN);
	bool IsPriest(int32_t eClass = CLASS_UNKNOWN);

	uint32_t GetProperHealthBuff(int MaxHp);
	uint32_t GetProperDefenceBuff();
	uint32_t GetProperMindBuff();
	uint32_t GetProperHeal();

	std::chrono::milliseconds GetSkillUseTime(int32_t iSkillID);
	void SetSkillUseTime(int32_t iSkillID, std::chrono::milliseconds iSkillUseTime);

	bool IsBuffActive(int32_t iBuffType);
	bool IsSkillActive(int32_t iSkillID);

	bool IsBlinking(DWORD iBase = 0);

	float GetDistance(Vector3 v3Position);
	float GetDistance(Vector3 v3SourcePosition, Vector3 v3TargetPosition);
	float GetDistance(float fX, float fY);
	float GetDistance(float fX1, float fY1, float fX2, float fY2);

	uint8_t GetSkillPoint(int32_t Slot);

	bool GetAvailableSkill(std::vector<__TABLE_UPC_SKILL>** vecAvailableSkills);

	int32_t GetInventoryItemCount(uint32_t iItemID);
	TInventory GetInventoryItem(uint32_t iItemID);
	TInventory GetInventoryItemSlot(uint8_t iSlotPosition);
	int32_t GetInventoryEmptySlot();
	int32_t GetInventoryEmptySlot(std::vector<int32_t> vecExcept);

	int SearchMob(std::vector<EntityInfo>& vecOutMobList);
	int SearchPlayer(std::vector<EntityInfo>& vecOutPlayerList);

protected:
	DWORD GetAddress(std::string szAddressName);
	Ini* GetUserConfiguration();
	Ini* GetAppConfiguration();

protected:
	Bot* m_Bot;
	TPlayer m_PlayerMySelf;
	
	std::vector<TNpc> m_vecNpc;

protected:
	std::map<int32_t, std::chrono::milliseconds> m_mapSkillUseTime;
	std::vector<__TABLE_UPC_SKILL> m_vecAvailableSkill;

public:
	std::recursive_mutex m_mutexLootList;

protected:
	std::vector<TLoot> m_vecLootList;
	bool m_bIsMovingToLoot;

protected:
	bool IsMovingToLoot() { return m_bIsMovingToLoot; }
	void SetMovingToLoot(bool bValue) { m_bIsMovingToLoot = bValue; }

public:
	void StopMove();

protected:
	bool IsEnemy(DWORD iBase);
	void StepCharacterForward(bool bStart);
	void BasicAttack();

	void PushPhase(DWORD dwAddress);
	void WriteLoginInformation(std::string szAccountId, std::string szPassword);
	void ConnectLoginServer(bool bDisconnect = false);
	void ConnectGameServer(BYTE byServerId);

	void SelectCharacterSkip();
	void SelectCharacterLeft();
	void SelectCharacterRight();
	void SelectCharacter();

	void SendPacket(Packet byBuffer);


	void UseSkillWithPacket(TABLE_UPC_SKILL pSkillData, int32_t iTargetID);

	DWORD GetSkillBase(uint32_t iSkillID);

protected:
	void UseSkill(TABLE_UPC_SKILL pSkillData, int32_t iTargetID, int32_t iPriority = 0, bool bAttacking = false, bool bBasicAttack = false);

	void SendStartSkillCastingAtTargetPacket(TABLE_UPC_SKILL pSkillData, int32_t iTargetID);
	void SendStartSkillCastingAtPosPacket(TABLE_UPC_SKILL pSkillData, Vector3 v3TargetPosition);
	void SendStartFlyingAtTargetPacket(TABLE_UPC_SKILL pSkillData, int32_t iTargetID, Vector3 v3TargetPosition, uint16_t arrowIndex = 0);
	void SendStartSkillMagicAtTargetPacket(TABLE_UPC_SKILL pSkillData, int32_t iTargetID, Vector3 v3TargetPosition, uint16_t arrowIndex = 0);
	void SendStartSkillMagicAtPosPacket(TABLE_UPC_SKILL pSkillData, Vector3 v3TargetPosition);
	void SendStartMagicAtTarget(TABLE_UPC_SKILL pSkillData, int32_t iTargetID, Vector3 v3TargetPosition, uint16_t arrowIndex = 0);
	void SendCancelSkillPacket(TABLE_UPC_SKILL pSkillData);

public:
	void PatchDeathEffect(bool bValue);
	void SendTownPacket();
	void SetMovePosition(Vector3 v3MovePosition);

	void SendItemMovePacket(uint8_t iType, uint8_t iDirection, uint32_t iItemID, uint8_t iCurrentPosition, uint8_t iTargetPosition);

protected:
	void SetPosition(Vector3 v3Position);
	void SendShoppingMall(ShoppingMallType eType);
	void SendTargetHpRequest(int32_t iTargetID, bool bBroadcast);
	void SetTarget(uint32_t iTargetBase);
	bool UseItem(uint32_t iItemID);

protected:
	void SendBasicAttackPacket(int32_t iTargetID, float fInterval = 1.0f, float fDistance = 2.0f);
	void SendMovePacket(Vector3 vecStartPosition, Vector3 vecTargetPosition, int16_t iMoveSpeed, uint8_t iMoveType);
	void SendRotation(float fRotation);
	void SendRequestBundleOpen(uint32_t iBundleID);
	void SendBundleItemGet(uint32_t iBundleID, uint32_t iItemID, int16_t iIndex);

private:
	std::vector<uint8_t> m_vecOrigDeathEffectFunction;

protected:
	void SendNpcEvent(int32_t iTargetID);
	void SendItemTradeBuy(uint32_t iSellingGroup, int32_t iNpcId, int32_t iItemId, uint8_t iInventoryPosition, int16_t iCount, uint8_t iShopPage, uint8_t iShopPosition);
	void SendItemTradeBuy(uint32_t iSellingGroup, int32_t iNpcId, std::vector<SSItemBuy> vecItemList);
	void SendItemTradeSell(uint32_t iSellingGroup, int32_t iNpcId, int32_t iItemId, uint8_t iInventoryPosition, int16_t iCount);
	void SendItemTradeSell(uint32_t iSellingGroup, int32_t iNpcId, std::vector<SSItemSell> vecItemList);
	void SendItemRepair(uint8_t iDirection, uint8_t iInventoryPosition, int32_t iNpcId, int32_t iItemId);

protected:
	void RefreshCaptcha();
	void SendCaptcha(std::string szCode);

protected:
	bool IsNeedRepair();
	bool IsNeedSupply();

public:
	int32_t GetPartyMemberCount();
	bool GetPartyList(std::vector<Party>& vecParty);
	bool GetPartyMember(int32_t iID, Party& pPartyMember);

public:
	bool GetPartyMemberBuffInfo(int32_t iMemberID, PartyBuffInfo& pPartyBuffInfo);
	void SetPartyMemberBuffInfo(int32_t iMemberID, PartyBuffInfo pPartyBuffInfo);

protected:
	std::map<int32_t, PartyBuffInfo> m_mapPartyBuffInfo;

protected:
	BYTE ReadByte(DWORD dwAddress);
	DWORD Read4Byte(DWORD dwAddress);
	float ReadFloat(DWORD dwAddress);
	std::string ReadString(DWORD dwAddress, size_t nSize);
	std::vector<BYTE> ReadBytes(DWORD dwAddress, size_t nSize);
	void WriteByte(DWORD dwAddress, BYTE byValue);
	void Write4Byte(DWORD dwAddress, int iValue);
	void WriteFloat(DWORD dwAddress, float fValue);
	void WriteString(DWORD dwAddress, std::string strValue);
	void WriteBytes(DWORD dwAddress, std::vector<BYTE> byValue);
	bool ExecuteRemoteCode(HANDLE hProcess, BYTE* codes, size_t psize);
	bool ExecuteRemoteCode(HANDLE hProcess, LPVOID pAddress);

public:
	bool IsTransformationAvailableZone();
	bool IsTransformationAvailable();

public:
	uint8_t GetRepresentZone(uint8_t iZone);

private:
	std::chrono::milliseconds m_msLastBasicAttackTime;

public:
	void UpdateSkillSuccessRate(bool bDisableCasting);
};