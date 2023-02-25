#include "pch.h"
#include "Client.h"
#include "Bot.h"
#include "Memory.h"

Client::Client()
{
	m_Bot = nullptr;

	memset(&m_PlayerMySelf, 0, sizeof(m_PlayerMySelf));

	m_vecNpc.clear();
	m_vecPlayer.clear();

	m_mapActiveBuffList.clear();
	m_mapSkillUseTime.clear();

	m_iTargetID = -1;

	m_bLunarWarDressUp = false;

	m_vecAvailableSkill.clear();
}

Client::~Client()
{
	m_Bot = nullptr;

	memset(&m_PlayerMySelf, 0, sizeof(m_PlayerMySelf));

	m_vecNpc.clear();
	m_vecPlayer.clear();

	m_mapActiveBuffList.clear();
	m_mapSkillUseTime.clear();

	m_iTargetID = -1;

	m_bLunarWarDressUp = false;

	m_vecAvailableSkill.clear();
}

DWORD Client::GetAddress(std::string szAddressName)
{
	if (!m_Bot) return 0;
	return m_Bot->GetAddress(szAddressName);
}

Ini* Client::GetConfiguration()
{
	if (!m_Bot) return 0;
	return m_Bot->GetConfiguration();
}

int32_t Client::GetID()
{
	return m_PlayerMySelf.iID;
}

std::string Client::GetName()
{
	return m_PlayerMySelf.szName;
}

int16_t Client::GetHp()
{
	return (int16_t)m_PlayerMySelf.iHP;
}

int16_t Client::GetMaxHp()
{
	return (int16_t)m_PlayerMySelf.iHPMax;
}

int16_t Client::GetMp()
{
	return (int16_t)m_PlayerMySelf.iMSP;
}

int16_t Client::GetMaxMp()
{
	return (int16_t)m_PlayerMySelf.iMSPMax;
}

uint8_t Client::GetZone()
{
	return m_PlayerMySelf.iCity;
}

uint32_t Client::GetGold()
{
	return m_PlayerMySelf.iGold;
}

uint8_t Client::GetLevel()
{
	return m_PlayerMySelf.iLevel;
}

Nation Client::GetNation()
{
	return m_PlayerMySelf.eNation;
}

Class Client::GetClass()
{
	return m_PlayerMySelf.eClass;
}

uint64_t Client::GetExp()
{
	return m_PlayerMySelf.iExp;
}

uint64_t Client::GetMaxExp()
{
	return m_PlayerMySelf.iExpNext;
}

float Client::GetGoX()
{
	return Memory::ReadFloat(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_GOX"));
}

float Client::GetGoY()
{
	return Memory::ReadFloat(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_GOY"));
}

float Client::GetGoZ()
{
	return Memory::ReadFloat(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_GOZ"));
}

float Client::GetX()
{
	return Memory::ReadFloat(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_X"));
}

float Client::GetZ()
{
	return Memory::ReadFloat(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_Z"));
}

float Client::GetY()
{
	return Memory::ReadFloat(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_Y"));
}

uint8_t Client::GetAuthority()
{
	return Memory::ReadByte(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_AUTHORITY"));
}

std::chrono::milliseconds Client::GetSkillUseTime(int32_t iSkillID)
{
	auto it = m_mapSkillUseTime.find(iSkillID);

	if (it != m_mapSkillUseTime.end())
		return it->second;

	return (std::chrono::milliseconds)0;
}

void Client::SetSkillUseTime(int32_t iSkillID, std::chrono::milliseconds iSkillUseTime)
{
	auto it = m_mapSkillUseTime.find(iSkillID);

	if (it == m_mapSkillUseTime.end())
		m_mapSkillUseTime.insert(std::pair(iSkillID, iSkillUseTime));
	else
		it->second = iSkillUseTime;
}

Vector3 Client::GetPosition()
{
	return Vector3(GetX(), GetZ(), GetY());
}

Vector3 Client::GetTargetPosition()
{
	if (m_iTargetID >= 5000)
	{
		auto it = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
			[this](const TNpc& a) { return a.iID == m_iTargetID; });

		if (it != m_vecNpc.end())
			return Vector3(it->fX, it->fZ, it->fY);
	}
	else
	{
		auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
			[this](const TPlayer& a) { return a.iID == m_iTargetID; });

		if (it != m_vecPlayer.end())
			return Vector3(it->fX, it->fZ, it->fY);
	}

	return Vector3(0.0f, 0.0f, 0.0f);
}

void Client::SetTarget(int32_t iTargetID)
{
	m_iTargetID = iTargetID;
}

int32_t Client::GetTarget()
{
	return m_iTargetID;
}

void Client::SetAuthority(uint8_t iAuthority)
{
	Memory::WriteByte(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_AUTHORITY"), iAuthority);
}

bool Client::IsBuffActive(int32_t iBuffType)
{ 
	return m_mapActiveBuffList.find(iBuffType) != m_mapActiveBuffList.end(); 
};

bool Client::IsBlinking()
{
	return m_PlayerMySelf.bBlinking;
};

float Client::GetDistance(Vector3 v3Position)
{
	Vector3 v3MyPosition = GetPosition();
	return GetDistance(v3MyPosition.m_fX, v3MyPosition.m_fY, v3Position.m_fX, v3Position.m_fY);
}

float Client::GetDistance(Vector3 v3SourcePosition, Vector3 v3TargetPosition)
{
	return GetDistance(v3SourcePosition.m_fX, v3SourcePosition.m_fY, v3TargetPosition.m_fX, v3TargetPosition.m_fY);
}

float Client::GetDistance(float fX, float fY)
{
	Vector3 v3MyPosition = GetPosition();
	return GetDistance(v3MyPosition.m_fX, v3MyPosition.m_fY, fX, fY);
}

float Client::GetDistance(float fX1, float fY1, float fX2, float fY2)
{
	return (float)sqrt(pow(fX2 - fX1, 2.0f) + pow(fY2 - fY1, 2.0f) * 1.0);
}

uint8_t Client::GetSkillPoint(int32_t Slot)
{
	return m_PlayerMySelf.iSkillInfo[Slot];
}

int32_t Client::GetInventoryItemCount(uint32_t iItemID)
{
	int32_t iItemCount = 0;

	for (int i = SLOT_MAX; i < SLOT_MAX + HAVE_MAX; i++)
	{
		if (m_PlayerMySelf.tInventory[i].iItemID == iItemID)
			iItemCount += m_PlayerMySelf.tInventory[i].iCount;
	}

	return iItemCount;
}

TInventory* Client::GetInventoryItem(uint32_t iItemID)
{
	for (int i = SLOT_MAX; i < SLOT_MAX + HAVE_MAX; i++)
	{
		if (m_PlayerMySelf.tInventory[i].iItemID == iItemID)
			return &m_PlayerMySelf.tInventory[i];
	}

	return NULL;
}

TInventory* Client::GetInventoryItemSlot(uint8_t iSlotPosition)
{
	if (m_PlayerMySelf.tInventory[iSlotPosition].iItemID != 0)
		return &m_PlayerMySelf.tInventory[iSlotPosition];

	return NULL;
}