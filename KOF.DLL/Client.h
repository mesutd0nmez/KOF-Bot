#pragma once

#include "Ini.h"
#include "World.h"

class Bot;
class Client
{
public:
	Client();
	~Client();

	void Clear();

public:
	int32_t GetID(bool bFromServer = true);
	std::string GetName(bool bFromServer = true);
	int16_t GetHp(bool bFromServer = true);
	int16_t GetMaxHp(bool bFromServer = true);
	int16_t GetMp(bool bFromServer = true);
	int16_t GetMaxMp(bool bFromServer = true);
	uint8_t GetZone(bool bFromServer = true);
	uint32_t GetGold(bool bFromServer = true);
	uint8_t GetLevel(bool bFromServer = true);
	Nation GetNation(bool bFromServer = true);
	Class GetClass(bool bFromServer = true);
	uint64_t GetExp(bool bFromServer = true);
	uint64_t GetMaxExp(bool bFromServer = true);

	bool IsDisconnect();

	float GetX(bool bFromServer = true);
	float GetZ(bool bFromServer = true);
	float GetY(bool bFromServer = true);

	float GetGoX();
	float GetGoY();
	float GetGoZ();

	uint8_t GetAuthority(bool bFromServer = false);
	void SetAuthority(uint8_t iAuthority);

	std::chrono::milliseconds GetSkillUseTime(int32_t iSkillID);
	void SetSkillUseTime(int32_t iSkillID, std::chrono::milliseconds iSkillUseTime);

	Vector3 GetPosition();
	Vector3 GetTargetPosition();
	int32_t GetTarget();

	bool IsBuffActive(int32_t iBuffType);

	bool IsBlinking();

	float GetDistance(Vector3 v3Position);
	float GetDistance(Vector3 v3SourcePosition, Vector3 v3TargetPosition);
	float GetDistance(float fX, float fY);
	float GetDistance(float fX1, float fY1, float fX2, float fY2);

	uint8_t GetSkillPoint(int32_t Slot);

	std::vector<__TABLE_UPC_SKILL> GetAvailableSkill() { return m_vecAvailableSkill; };

	std::vector<TNpc> GetNpcList() { return m_vecNpc; }
	std::vector<TPlayer> GetPlayerList() { return m_vecPlayer; }

	int32_t GetInventoryItemCount(uint32_t iItemID);
	TInventory* GetInventoryItem(uint32_t iItemID);
	TInventory* GetInventoryItemSlot(uint8_t iSlotPosition);

protected:
	DWORD GetAddress(std::string szAddressName);
	Ini* GetConfiguration();

protected:
	Bot* m_Bot;
	TPlayer m_PlayerMySelf;
	
	std::vector<TNpc> m_vecNpc;
	std::vector<TPlayer> m_vecPlayer;

public:
	std::recursive_mutex m_vecNpcLock;
	std::recursive_mutex m_vecPlayerLock;

protected:

	std::map<int32_t, uint32_t> m_mapActiveBuffList;
	std::recursive_mutex m_mapActiveBuffListLock;

	std::map<int32_t, std::chrono::milliseconds> m_mapSkillUseTime;
	std::recursive_mutex m_mapSkillUseTimeLock;

	std::vector<__TABLE_UPC_SKILL> m_vecAvailableSkill;

	int32_t m_iTargetID;

	bool m_bLunarWarDressUp;

public:
	BYTE ReadByte(DWORD dwAddress);
	DWORD Read4Byte(DWORD dwAddress);
	float ReadFloat(DWORD dwAddress);
	std::string ReadString(DWORD dwAddress, size_t nSize);
	std::vector<BYTE> ReadBytes(DWORD dwAddress, size_t nSize);
	void WriteByte(DWORD dwAddress, BYTE byValue);
	void Write4Byte(DWORD dwAddress, DWORD dwValue);
	void WriteFloat(DWORD dwAddress, float fValue);
	void WriteString(DWORD dwAddress, std::string strValue);
	void WriteBytes(DWORD dwAddress, std::vector<BYTE> byValue);

	void ExecuteRemoteCode(BYTE* codes, size_t psize);

private:
	std::vector<TLoot> GetLootList() { return m_vecLootList; }

protected:
	std::vector<TLoot> m_vecLootList;
	std::recursive_mutex m_vecLootListLock;

	bool m_bIsMovingToLoot;

protected:
	bool IsMovingToLoot() { return m_bIsMovingToLoot; }
	void SetMovingToLoot(bool bValue) { m_bIsMovingToLoot = bValue; }

public:
	World* GetWorld() { return m_World; }
protected:
	World* m_World;

};

