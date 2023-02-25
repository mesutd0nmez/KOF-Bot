#pragma once

#include "Ini.h"

class Bot;
class Client
{
public:
	Client();
	~Client();

public:
	int32_t GetID();
	std::string GetName();
	int16_t GetHp();
	int16_t GetMaxHp();
	int16_t GetMp();
	int16_t GetMaxMp();
	uint8_t GetZone();
	uint32_t GetGold();
	uint8_t GetLevel();
	Nation GetNation();
	Class GetClass();
	uint64_t GetExp();
	uint64_t GetMaxExp();
	float GetGoX();
	float GetGoY();
	float GetGoZ();

	float GetX();
	float GetZ();
	float GetY();
	uint8_t GetAuthority();
	void SetAuthority(uint8_t iAuthority);

	std::chrono::milliseconds GetSkillUseTime(int32_t iSkillID);
	void SetSkillUseTime(int32_t iSkillID, std::chrono::milliseconds iSkillUseTime);

	Vector3 GetPosition();
	Vector3 GetTargetPosition();

	void SetTarget(int32_t iTargetID);
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

	std::map<int32_t, uint32_t> m_mapActiveBuffList;
	std::map<int32_t, std::chrono::milliseconds> m_mapSkillUseTime;

	std::vector<__TABLE_UPC_SKILL> m_vecAvailableSkill;

	int32_t m_iTargetID;

	bool m_bLunarWarDressUp;
};

