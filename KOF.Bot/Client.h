#pragma once

#include "Ini.h"

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

	bool IsDisconnect();

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

	DWORD GetMobBase(int32_t iTargetId);

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
	bool IsBlinking();

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
	Ini* GetConfiguration();

protected:
	Bot* m_Bot;
	TPlayer m_PlayerMySelf;
	
	std::vector<TNpc> m_vecNpc;

public:
	std::recursive_mutex m_mutexNpc;

protected:
	std::map<int32_t, std::chrono::milliseconds> m_mapSkillUseTime;
	std::vector<__TABLE_UPC_SKILL> m_vecAvailableSkill;

public:
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
	void ExecuteRemoteCode(BYTE* codes, size_t psize);
	void ExecuteRemoteCode(LPVOID pAddress);

public:
	std::recursive_mutex m_mutexLootList;

protected:
	std::vector<TLoot> m_vecLootList;
	bool m_bIsMovingToLoot;

protected:
	bool IsMovingToLoot() { return m_bIsMovingToLoot; }
	void SetMovingToLoot(bool bValue) { m_bIsMovingToLoot = bValue; }

public:
	bool IsEnemy(DWORD iBase);
};

