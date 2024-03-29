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
	float GetRadius(DWORD iBase = 0);
	void SetScale(DWORD iBase, float fX, float fZ, float fY);
	float GetScaleX(DWORD iBase = 0);
	float GetScaleZ(DWORD iBase = 0);
	float GetScaleY(DWORD iBase = 0);
	bool IsAttackable(DWORD iTargetID);
	uint8_t GetServerId();
	bool IsDisconnect();
	bool IsDeath(DWORD iBase = 0);
	bool IsStunned(DWORD iBase = 0);
	float GetX(DWORD iBase = 0);
	float GetZ(DWORD iBase = 0);
	float GetY(DWORD iBase = 0);
	float GetGoX();
	float GetGoY();
	float GetGoZ();
	uint8_t GetAuthority(DWORD iBase = 0);
	void SetAuthority(uint8_t iAuthority);

	Vector3 GetPosition();
	Vector3 GetMovePosition();
	Vector3 GetTargetPosition();

	int32_t GetTarget();
	uint32_t GetTargetBase();

	DWORD GetEntityBase(int32_t iTargetId);

	bool IsRogue(int32_t eClass = CLASS_UNKNOWN);
	bool IsMage(int32_t eClass = CLASS_UNKNOWN);
	bool IsWarrior(int32_t eClass = CLASS_UNKNOWN);
	bool IsPriest(int32_t eClass = CLASS_UNKNOWN);

	float GetSkillNextUseTime(int32_t iSkillID);
	void SetSkillNextUseTime(int32_t iSkillID, float fSkillNextUseTime);

	bool IsBuffActive(int32_t iBuffType);
	bool IsSkillActive(int32_t iSkillID);

	bool IsBlinking(int32_t iTargetID = -1);

	float GetDistance(Vector3 v3Position);
	float GetDistance(Vector3 v3SourcePosition, Vector3 v3TargetPosition);
	float GetDistance(float fX, float fY);
	float GetDistance(float fX1, float fY1, float fX2, float fY2);

	uint8_t GetSkillPoint(int32_t Slot);

	int32_t GetInventoryItemCount(uint32_t iItemID);
	TItemData GetInventoryItem(uint32_t iItemID);
	TItemData GetInventoryItemSlot(uint8_t iSlotPosition);
	bool GetInventoryItemList(std::vector<TItemData>& vecItemList);

	DWORD GetInventoryItemBase(uint8_t iSlotPosition);

	int32_t GetInventoryEmptySlot();
	int32_t GetInventoryEmptySlot(std::vector<int32_t> vecExcept);

protected:
	Bot* m_Bot;

public:
	TPlayer m_PlayerMySelf;

	std::vector<__TABLE_UPC_SKILL> m_vecAvailableSkill;

protected:
	std::map<int32_t, float> m_mapSkillUseTime;

public:
	std::vector<TNpc> m_vecNpc;
	std::vector<TPlayer> m_vecPlayer;

	std::vector<TLoot> m_vecLootList;
	bool m_bIsMovingToLoot;

	bool IsMovingToLoot() { return m_bIsMovingToLoot; }
	void SetMovingToLoot(bool bValue) { m_bIsMovingToLoot = bValue; }

protected:
	bool m_bIsZoneChanging;
	bool IsZoneChanging() { return m_bIsZoneChanging; }
	void SetZoneChange(bool bValue) { m_bIsZoneChanging = bValue; }

public:
	void StopMove();

	void UseSkillWithPacket(TABLE_UPC_SKILL pSkillData, int32_t iTargetID, bool bWaitCastTime = true);

protected:
	void StepCharacterForward(bool bStart);
	void BasicAttack();
	void BasicAttackWithPacket(DWORD iTargetBase, float fBasicAttackInterval);

	void PushPhase(DWORD dwAddress);
	void WriteLoginInformation(std::string szAccountId, std::string szPassword);
	void ConnectLoginServer(bool bDisconnect = false);
	void LoadServerList();
	void SelectServer(uint8_t iIndex);
	void ShowChannel();
	void SelectChannel(uint8_t iIndex);
	void ConnectServer();

	void SelectCharacterSkip();
	void SelectCharacterLeft();
	void SelectCharacterRight();
	void SelectCharacter();

	void SendPacket(Packet byBuffer);


	DWORD GetSkillBase(uint32_t iSkillID);

public:
	void SetSaveCPUSleepTime(int32_t iValue);
	void SendPacket(std::string szPacket);

protected:
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
	void SendShoppingMall(ShoppingMallType eType);
protected:
	void SetPosition(Vector3 v3Position);

	void SendTargetHpRequest(int32_t iTargetID, bool bBroadcast);
	void SetTarget(uint32_t iTargetID);

protected:
	void SendBasicAttackPacket(int32_t iTargetID, float fInterval = 1.0f, float fDistance = 2.0f);
	void SendMovePacket(Vector3 vecStartPosition, Vector3 vecTargetPosition, int16_t iMoveSpeed, uint8_t iMoveType);
	void SendRotation(float fRotation);
	void SendRequestBundleOpen(uint32_t iBundleID);
	void SendBundleItemGet(uint32_t iBundleID, uint32_t iItemID, int16_t iIndex);

private:
	std::vector<uint8_t> m_vecOrigDeathEffectFunction;

public:
	void SendNpcEvent(int32_t iTargetID);

protected:

	void SendItemTradeBuy(uint32_t iSellingGroup, int32_t iNpcId, int32_t iItemId, uint8_t iInventoryPosition, int16_t iCount, uint8_t iShopPage, uint8_t iShopPosition);
	void SendItemTradeBuy(uint32_t iSellingGroup, int32_t iNpcId, std::vector<SSItemBuy> vecItemList);
	void SendItemTradeSell(uint32_t iSellingGroup, int32_t iNpcId, int32_t iItemId, uint8_t iInventoryPosition, int16_t iCount);
	void SendItemTradeSell(uint32_t iSellingGroup, int32_t iNpcId, std::vector<SSItemSell> vecItemList);
	void SendItemRepair(uint8_t iDirection, uint8_t iInventoryPosition, int32_t iNpcId, int32_t iItemId);

public:
	void RefreshCaptcha();
	void SendCaptcha(std::string szCode);

public:
	void SendWarehouseOpen(uint32_t iNpcID);
	void SendWarehouseGetIn(int32_t iNpcID, uint32_t iItemID, uint8_t iPage, uint8_t iCurrentPosition, uint8_t iTargetPosition, uint32_t iCount);
	void SendWarehouseGetOut(int32_t iNpcID, uint32_t iItemID, uint8_t iPage, uint8_t iCurrentPosition, uint8_t iTargetPosition, uint32_t iCount);
	int32_t GetWarehouseAvailableSlot(uint32_t iItemID, uint8_t iContable);

public:
	void SendUseGeniePotion(uint32_t iItemID);
	void SendStartGenie();
	void SendStopGenie();

public:
	bool IsTransformationAvailableZone();
	bool IsTransformationAvailable();

public:
	uint8_t GetRepresentZone(uint8_t iZone);

private:
	std::chrono::milliseconds m_msLastBasicAttackTime;

public:
	void UpdateSkillSuccessRate(bool bDisableCasting);

protected:
	std::vector<PartyMember> m_vecPartyMembers;

protected:
	bool m_bLunarWarDressUp;

protected:
	std::unordered_set<int32_t> m_vecRegionUserList;

protected:
	float m_fAttackDelta;
	float m_fAttackTimeRecent;

public:
	void SendPartyCreate(std::string szName);
	void SendPartyInsert(std::string szName);

protected:
	float m_fLastGenieStartTime;

public:
	void SendRearrangeInventory();

public:
	void SendOpenVipWarehouse(uint32_t iItemID = 0);
	void SendVipWarehouseGetIn(int32_t iNpcID, uint32_t iItemID, uint8_t iPage, uint8_t iCurrentPosition, uint8_t iTargetPosition, uint16_t iCount);

	bool GetVipWarehouseItemList(std::vector<TItemData>& vecItemList);
	bool GetVipWarehouseInventoryItemList(std::vector<TItemData>& vecItemList);

	void VipWarehouseGetIn(DWORD iItemBase, int32_t iSourcePosition, int32_t iTargetPosition);
	void VipWarehouseGetOut(DWORD iItemBase, int32_t iSourcePosition, int32_t iTargetPosition);

	void CountableDialogChangeCount(uint32_t iCount);
	void AcceptCountableDialog();
	
public:
	void SendQuestCompleted(uint32_t iQuestID);
	void SendQuestUnknown1(uint32_t iQuestID);
	void SendQuestUnknown2(uint32_t iQuestID);

public:
	void OpenVipWarehouse();
	void CloseVipWarehouse();
	bool IsVipWarehouseOpen();
	bool IsTransactionDialogOpen();
	bool IsWarehouseOpen();

public:
	void SendRegenePacket();

public:
	void SendOTPPacket(std::string szAccountId, std::string szPassword, std::string szCode);

public:
	void SetCharacterSpeed(float fSpeed);
	float GetCharacterSpeed();
	void PatchSpeedHack(bool bEnable);

protected:
	uint8_t m_iFlashCount;

protected:
	bool m_bVipWarehouseInitialized;
	bool m_bVipWarehouseEnabled;
	bool m_bVipWarehouseLoaded;
	bool m_bVipWarehouseFull;

public:
	bool IsVipWarehouseFull();

public:
	void SendSelectMessage(uint8_t iMenuIndex, std::string szLuaName, bool bAccept = false);

public:
	float m_fLastDisconnectTime;

protected:
	bool m_bSkillCasting;

public:
	void RemoveItem(int32_t iItemSlot);

	void VipGetInTest();
	void VipGetOutTest();
	void Legalize(DWORD iItemBase, int32_t iSourcePosition, int32_t iTargetPosition);

	void EquipItem(DWORD iItemBase, int32_t iSourcePosition, int32_t iTargetPosition);

public:
	bool IsInventoryFull() { return GetInventoryEmptySlotCount() == 0; };
	int GetInventoryEmptySlotCount();

protected:
	bool IsSkillHasZoneLimit(uint32_t iSkillBaseID);
};