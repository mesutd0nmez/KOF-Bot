#include "pch.h"
#include "Client.h"
#include "Bot.h"
#include "Memory.h"
#include "Guard.h"

Client::Client()
{
	m_Bot = nullptr;

	Clear();
}

Client::~Client()
{
	m_Bot = nullptr;

	Clear();
}

void Client::Clear()
{
	memset(&m_PlayerMySelf, 0, sizeof(m_PlayerMySelf));

	m_vecNpc.clear();
	m_vecPlayer.clear();

	m_mapActiveBuffList.clear();
	m_mapSkillUseTime.clear();

	m_iTargetID = -1;

	m_bLunarWarDressUp = false;

	m_vecAvailableSkill.clear();

	m_vecLootList.clear();
	m_bIsMovingToLoot = false;
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

int32_t Client::GetID(bool bFromServer)
{
	if(bFromServer)
		return m_PlayerMySelf.iID;

	return Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_ID")));
}

std::string Client::GetName(bool bFromServer)
{
	if(bFromServer)
		return m_PlayerMySelf.szName;
	else
	{
		int iNameLen = Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_NAME_LEN")));

		if (iNameLen > 15)
			return ReadString(Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_NAME"))), iNameLen);

		return ReadString(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_NAME")), iNameLen);
	}
}

int16_t Client::GetHp(bool bFromServer)
{
	if (bFromServer)
		return (int16_t)m_PlayerMySelf.iHP;

	return (int16_t)Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_HP")));
}

int16_t Client::GetMaxHp(bool bFromServer)
{
	if (bFromServer)
		return (int16_t)m_PlayerMySelf.iHPMax;

	return (int16_t)Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_MAXHP")));
}

int16_t Client::GetMp(bool bFromServer)
{
	if (bFromServer)
		return (int16_t)m_PlayerMySelf.iMSP;

	return (int16_t)Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_MP")));
}

int16_t Client::GetMaxMp(bool bFromServer)
{
	if (bFromServer)
		return (int16_t)m_PlayerMySelf.iMSPMax;

	return (int16_t)Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_MAXMP")));
}

uint8_t Client::GetZone(bool bFromServer)
{
	if (bFromServer)
		return m_PlayerMySelf.iCity;

	return (uint8_t)Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_ZONE")));
}

uint32_t Client::GetGold(bool bFromServer)
{
	if (bFromServer)
		return m_PlayerMySelf.iGold;

	return Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_GOLD")));
}

uint8_t Client::GetLevel(bool bFromServer)
{
	if (bFromServer)
		return m_PlayerMySelf.iLevel;

	return ReadByte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_LEVEL")));
}

Nation Client::GetNation(bool bFromServer)
{
	if (bFromServer)
		return m_PlayerMySelf.eNation;

	return (Nation)ReadByte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_NATION")));
}

Class Client::GetClass(bool bFromServer)
{
	if (bFromServer)
		return m_PlayerMySelf.eClass;

	return (Class)Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_CLASS")));
}

uint64_t Client::GetExp(bool bFromServer)
{
	if (bFromServer)
		return m_PlayerMySelf.iExp;

	return (uint64_t)Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_EXP")));
}

uint64_t Client::GetMaxExp(bool bFromServer)
{
	if (bFromServer)
		return m_PlayerMySelf.iExpNext;

	return (uint64_t)Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_MAXEXP")));
}

bool Client::IsDisconnect()
{
	return Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_PKT"))) + GetAddress(skCryptDec("KO_OFF_DISCONNECT"))) == 0;
};

float Client::GetGoX()
{
	return ReadFloat(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_GOX")));
}

float Client::GetGoY()
{
	return ReadFloat(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_GOY")));
}

float Client::GetGoZ()
{
	return ReadFloat(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_GOZ")));
}

float Client::GetX(bool bFromServer)
{
	if (bFromServer)
		return m_PlayerMySelf.fX;

	return ReadFloat(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_X")));
}

float Client::GetZ(bool bFromServer)
{
	if (bFromServer)
		return m_PlayerMySelf.fZ;

	return ReadFloat(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_Z")));
}

float Client::GetY(bool bFromServer)
{
	if (bFromServer)
		return m_PlayerMySelf.fY;

	return ReadFloat(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_Y")));
}

uint8_t Client::GetAuthority(bool bFromServer)
{
	if (bFromServer)
		return (uint8_t)m_PlayerMySelf.iAuthority;

	return ReadByte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_AUTHORITY")));
}

std::chrono::milliseconds Client::GetSkillUseTime(int32_t iSkillID)
{
	Guard lock(m_mapSkillUseTimeLock);
	auto it = m_mapSkillUseTime.find(iSkillID);

	if (it != m_mapSkillUseTime.end())
		return it->second;

	return (std::chrono::milliseconds)0;
}

void Client::SetSkillUseTime(int32_t iSkillID, std::chrono::milliseconds iSkillUseTime)
{
	Guard lock(m_mapSkillUseTimeLock);
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

int32_t Client::GetTarget()
{
	return m_iTargetID;
}

void Client::SetAuthority(uint8_t iAuthority)
{
	WriteByte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_AUTHORITY")), iAuthority);
}

bool Client::IsBuffActive(int32_t iBuffType)
{ 
	Guard lock(m_mapActiveBuffListLock);
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
	return std::sqrt((fX1 - fX2) * (fX1 - fX2) +
		(fY1 - fY2) * (fY1 - fY2));
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

int32_t Client::GetInventoryEmptySlot()
{
	for (int i = SLOT_MAX; i < SLOT_MAX + HAVE_MAX; i++)
	{
		if (m_PlayerMySelf.tInventory[i].iItemID == 0)
			return i;
	}

	return -1;
}

TInventory* Client::GetInventoryItemSlot(uint8_t iSlotPosition)
{
	if (m_PlayerMySelf.tInventory[iSlotPosition].iItemID != 0)
		return &m_PlayerMySelf.tInventory[iSlotPosition];

	return NULL;
}

BYTE Client::ReadByte(DWORD dwAddress)
{
	return m_Bot->ReadByte(dwAddress);
}

DWORD Client::Read4Byte(DWORD dwAddress)
{
	return m_Bot->Read4Byte(dwAddress);
}

float Client::ReadFloat(DWORD dwAddress)
{
	return m_Bot->ReadFloat(dwAddress);
}

std::string Client::ReadString(DWORD dwAddress, size_t nSize)
{
	return m_Bot->ReadString(dwAddress, nSize);
}

std::vector<BYTE> Client::ReadBytes(DWORD dwAddress, size_t nSize)
{
	return m_Bot->ReadBytes(dwAddress, nSize);
}

void Client::WriteByte(DWORD dwAddress, BYTE byValue)
{
	m_Bot->WriteByte(dwAddress, byValue);
}

void Client::Write4Byte(DWORD dwAddress, DWORD dwValue)
{
	m_Bot->Write4Byte(dwAddress, dwValue);
}

void Client::WriteFloat(DWORD dwAddress, float fValue)
{
	m_Bot->WriteFloat(dwAddress, fValue);
}

void Client::WriteString(DWORD dwAddress, std::string strValue)
{
	m_Bot->WriteString(dwAddress, strValue);
}

void Client::WriteBytes(DWORD dwAddress, std::vector<BYTE> byValue)
{
	m_Bot->WriteBytes(dwAddress, byValue);
}

void Client::ExecuteRemoteCode(BYTE* codes, size_t psize)
{
	m_Bot->ExecuteRemoteCode(codes, psize);
}