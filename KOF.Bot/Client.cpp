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

int32_t Client::GetID()
{
	return Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_ID")));
}

std::string Client::GetName()
{
	int iNameLen = Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_NAME_LEN")));

	if (iNameLen > 15)
		return ReadString(Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_NAME"))), iNameLen);

	return ReadString(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_NAME")), iNameLen);
}

int16_t Client::GetHp()
{
	return (int16_t)Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_HP")));
}

int16_t Client::GetMaxHp()
{
	return (int16_t)Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_MAXHP")));
}

int16_t Client::GetMp()
{
	return (int16_t)Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_MP")));
}

int16_t Client::GetMaxMp()
{
	return (int16_t)Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_MAXMP")));
}

uint8_t Client::GetZone()
{
	return (uint8_t)Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_ZONE")));
}

uint32_t Client::GetGold()
{
	return Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_GOLD")));
}

uint8_t Client::GetLevel()
{
	return ReadByte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_LEVEL")));
}

Nation Client::GetNation()
{
	return (Nation)ReadByte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_NATION")));
}

Class Client::GetClass()
{
	return (Class)Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_CLASS")));
}

uint64_t Client::GetExp()
{
	return (uint64_t)Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_EXP")));
}

uint64_t Client::GetMaxExp()
{
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

float Client::GetX()
{
	return ReadFloat(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_X")));
}

float Client::GetZ()
{
	return ReadFloat(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_Z")));
}

float Client::GetY()
{
	return ReadFloat(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_Y")));
}

uint8_t Client::GetAuthority()
{
	return ReadByte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_AUTHORITY")));
}

bool Client::IsRogue(Class eClass)
{
	if (eClass == CLASS_UNKNOWN)
	{
		eClass = GetClass();
	}

	switch (eClass)
	{
		case CLASS_KA_ROGUE:
		case CLASS_KA_HUNTER:
		case CLASS_KA_PENETRATOR:
		case CLASS_EL_ROGUE:
		case CLASS_EL_RANGER:
		case CLASS_EL_ASSASIN:
			return true;
	}

	return false;
}

bool Client::IsMage(Class eClass)
{
	if (eClass == CLASS_UNKNOWN)
	{
		eClass = GetClass();
	}

	switch (eClass)
	{
		case CLASS_KA_WIZARD:
		case CLASS_KA_SORCERER:
		case CLASS_KA_NECROMANCER:
		case CLASS_EL_WIZARD:
		case CLASS_EL_MAGE:
		case CLASS_EL_ENCHANTER:
			return true;
	}

	return false;
}

bool Client::IsWarrior(Class eClass)
{
	if (eClass == CLASS_UNKNOWN)
	{
		eClass = GetClass();
	}

	switch (eClass)
	{
		case CLASS_KA_WARRIOR:
		case CLASS_KA_BERSERKER:
		case CLASS_KA_GUARDIAN:
		case CLASS_EL_WARRIOR:
		case CLASS_EL_BLADE:
		case CLASS_EL_PROTECTOR:
			return true;
	}

	return false;
}

bool Client::IsPriest(Class eClass)
{
	if (eClass == CLASS_UNKNOWN)
	{
		eClass = GetClass();
	}

	switch (eClass)
	{
		case CLASS_KA_PRIEST:
		case CLASS_KA_SHAMAN:
		case CLASS_KA_DARKPRIEST:
		case CLASS_EL_PRIEST:
		case CLASS_EL_CLERIC:
		case CLASS_EL_DRUID:
			return true;
	}

	return false;
}

uint32_t Client::GetProperHealthBuff(int MaxHp)
{
	int32_t iUndyHpPercent = (int32_t)std::ceil((MaxHp * 100) / 100.0f);

	if (GetSkillPoint(6) >= 78)
	{
		if (iUndyHpPercent >= 2500)
			return 111654;
		else
			return 112675;
	}
	else if (GetSkillPoint(6) >= 70)
	{
		if (iUndyHpPercent >= 2000)
			return 111654;
		else
			return 112670;
	}
	else if (GetSkillPoint(6) >= 57)
	{
		if (iUndyHpPercent >= 1500)
			return 111654;
		else
			return 111657;
	}
	else if (GetSkillPoint(6) >= 54)
	{
		if (iUndyHpPercent >= 1200)
			return 111654;
		else
			return 111655;
	}
	else if (GetSkillPoint(6) >= 42)
		return 111642;
	else if (GetSkillPoint(6) >= 33)
		return 111633;
	else if (GetSkillPoint(6) >= 24)
		return 111624;
	else if (GetSkillPoint(6) >= 15)
		return 111615;
	else if (GetSkillPoint(6) >= 6)
		return 111606;

	return -1;
}

uint32_t Client::GetProperDefenceBuff()
{
	if (GetSkillPoint(6) >= 76)
		return 112674;
	else if (GetSkillPoint(6) >= 60)
		return 111660;
	else if (GetSkillPoint(6) >= 51)
		return 111651;
	else if (GetSkillPoint(6) >= 39)
		return 111639;
	else if (GetSkillPoint(6) >= 30)
		return 111630;
	else if (GetSkillPoint(6) >= 21)
		return 111621;
	else if (GetSkillPoint(6) >= 12)
		return 111612;
	else if (GetSkillPoint(6) >= 3)
		return 111603;

	return -1;
}

uint32_t Client::GetProperMindBuff()
{
	if (GetSkillPoint(6) >= 45 && GetSkillPoint(6) <= 80)
		return 111645;
	else if (GetSkillPoint(6) >= 36 && GetSkillPoint(6) <= 44)
		return 111636;
	else if (GetSkillPoint(6) >= 27 && GetSkillPoint(6) <= 35)
		return 111627;
	else if (GetSkillPoint(6) >= 9 && GetSkillPoint(6) <= 26)
		return 111609;

	return -1;
}

uint32_t Client::GetProperHeal()
{
	if (GetSkillPoint(5) >= 45)
		return 111545;
	else if (GetSkillPoint(5) >= 36)
		return 111536;
	else if (GetSkillPoint(5) >= 27)
		return 111527;
	else if (GetSkillPoint(5) >= 18)
		return 111518;
	else if (GetSkillPoint(5) >= 9)
		return 111509;
	else if (GetSkillPoint(5) >= 0)
		return 107705;

	return -1;
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

DWORD Client::GetMobBase(int32_t iTargetId)
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_Bot->GetInjectedProcessId());

	if (hProcess == nullptr)
		return 0;

	LPVOID pAddress = VirtualAllocEx(hProcess, 0, 1, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (pAddress == 0)
	{
		CloseHandle(hProcess);
		return 0;
	}

	BYTE byCode[] =
	{
		0x60,
		0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00,
		0x6A, 0x01,
		0x68, 0x00, 0x00, 0x00, 0x00,
		0xBF, 0x00, 0x00, 0x00, 0x00,
		0xFF, 0xD7,
		0xA3, 0x00, 0x00, 0x00, 0x00,
		0x61,
		0xC3,
	};

	DWORD iFldb = GetAddress(skCryptDec("KO_PTR_FLDB"));

	CopyBytes(byCode + 3, iFldb);
	CopyBytes(byCode + 10, iTargetId);

	DWORD iFmbs = GetAddress(skCryptDec("KO_PTR_FMBS"));
	CopyBytes(byCode + 15, iFmbs);
	CopyBytes(byCode + 22, pAddress);

	ExecuteRemoteCode(byCode, sizeof(byCode));

	DWORD iBase = Read4Byte((DWORD)pAddress);

	VirtualFreeEx(hProcess, pAddress, 0, MEM_RELEASE);
	CloseHandle(hProcess);

	return iBase;
}

Vector3 Client::GetTargetPosition()
{
	if (m_iTargetID >= 5000)
	{
		DWORD iBase = GetMobBase(m_iTargetID);

		if (iBase > 0)
		{
			return Vector3(
				ReadFloat(iBase + GetAddress("KO_OFF_X")), 
				ReadFloat(iBase + GetAddress("KO_OFF_Z")), 
				ReadFloat(iBase + GetAddress("KO_OFF_Y")));
		}
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

bool Client::GetAvailableSkill(std::vector<__TABLE_UPC_SKILL>** vecAvailableSkills)
{
	if (m_vecAvailableSkill.size() == 0)
		return false;

	*vecAvailableSkills = &m_vecAvailableSkill;

	return true;
}

bool Client::GetNpcList(std::vector<TNpc>** vecNpcList)
{ 
	if (m_vecNpc.size() == 0)
		return false;

	*vecNpcList = &m_vecNpc;

	return true; 
}

bool Client::GetPlayerList(std::vector<TPlayer>** vecPlayerList)
{ 
	if(m_vecPlayer.size() == 0)
		return false;

	*vecPlayerList = &m_vecPlayer;

	return true;
}

int32_t Client::GetInventoryItemCount(uint32_t iItemID)
{
	int32_t iItemCount = 0;

	DWORD iInventoryBase = Read4Byte(Read4Byte(GetAddress("KO_PTR_DLG")) + GetAddress("KO_OFF_INVENTORY_BASE"));

	for (size_t i = SLOT_MAX; i < SLOT_MAX + HAVE_MAX; i++)
	{
		DWORD iItemBase = Read4Byte(iInventoryBase + (GetAddress("KO_OFF_INVENTORY_START") + (4 * i)));

		if (m_Bot->GetPlatformType() == PlatformType::USKO)
		{
			if (Read4Byte(Read4Byte(iItemBase + 0x60)) + Read4Byte(Read4Byte(iItemBase + 0x64)) == iItemID)
				iItemCount += Read4Byte(iItemBase + 0x68);
		}
		else
		{
			if (Read4Byte(Read4Byte(iItemBase + 0x68)) + Read4Byte(Read4Byte(iItemBase + 0x6C)) == iItemID)
				iItemCount += Read4Byte(iItemBase + 0x70);
		}
	}

	return iItemCount;
}

TInventory Client::GetInventoryItem(uint32_t iItemID)
{
	TInventory pInventory;
	memset(&pInventory, 0, sizeof(pInventory));

	DWORD iInventoryBase = Read4Byte(Read4Byte(GetAddress("KO_PTR_DLG")) + GetAddress("KO_OFF_INVENTORY_BASE"));

	for (size_t i = SLOT_MAX; i < SLOT_MAX + HAVE_MAX; i++)
	{
		DWORD iItemBase = Read4Byte(iInventoryBase + (GetAddress("KO_OFF_INVENTORY_START") + (4 * i)));

		if (m_Bot->GetPlatformType() == PlatformType::USKO)
		{
			if (Read4Byte(Read4Byte(iItemBase + 0x60)) + Read4Byte(Read4Byte(iItemBase + 0x64)) == iItemID)
			{
				pInventory.iPos = i;
				pInventory.iItemID = iItemID;
				pInventory.iCount = (uint16_t)Read4Byte(iItemBase + 0x68);
				pInventory.iDurability = (uint16_t)Read4Byte(iItemBase + 0x6C);

				return pInventory;
			}
		}
		else
		{
			if (Read4Byte(Read4Byte(iItemBase + 0x68)) + Read4Byte(Read4Byte(iItemBase + 0x6C)) == iItemID)
			{
				pInventory.iPos = i;
				pInventory.iItemID = iItemID;
				pInventory.iCount = (uint16_t)Read4Byte(iItemBase + 0x70);
				pInventory.iDurability = (uint16_t)Read4Byte(iItemBase + 0x74);

				return pInventory;
			}
		}
	}

	return pInventory;
}

int32_t Client::GetInventoryEmptySlot()
{
	DWORD iInventoryBase = Read4Byte(Read4Byte(GetAddress("KO_PTR_DLG")) + GetAddress("KO_OFF_INVENTORY_BASE"));

	for (size_t i = SLOT_MAX; i < SLOT_MAX + HAVE_MAX; i++)
	{
		DWORD iItemBase = Read4Byte(iInventoryBase + (GetAddress("KO_OFF_INVENTORY_START") + (4 * i)));

		if (m_Bot->GetPlatformType() == PlatformType::USKO)
		{
			if (Read4Byte(Read4Byte(iItemBase + 0x60)) + Read4Byte(Read4Byte(iItemBase + 0x64)) == 0)
			{
				return i;
			}
		}
		else
		{
			if (Read4Byte(Read4Byte(iItemBase + 0x68)) + Read4Byte(Read4Byte(iItemBase + 0x6C)) == 0)
			{
				return i;
			}
		}
	}

	return -1;
}

int32_t Client::GetInventoryEmptySlot(std::vector<int32_t> vecExcept)
{
	DWORD iInventoryBase = Read4Byte(Read4Byte(GetAddress("KO_PTR_DLG")) + GetAddress("KO_OFF_INVENTORY_BASE"));

	for (int i = SLOT_MAX; i < SLOT_MAX + HAVE_MAX; i++)
	{
		if (std::find(vecExcept.begin(), vecExcept.end(), i)
			!= vecExcept.end())
			continue;

		DWORD iItemBase = Read4Byte(iInventoryBase + (GetAddress("KO_OFF_INVENTORY_START") + (4 * i)));

		if (m_Bot->GetPlatformType() == PlatformType::USKO)
		{
			if (Read4Byte(Read4Byte(iItemBase + 0x60)) + Read4Byte(Read4Byte(iItemBase + 0x64)) == 0)
				return i;
		}
		else
		{
			if (Read4Byte(Read4Byte(iItemBase + 0x68)) + Read4Byte(Read4Byte(iItemBase + 0x6C)) == 0)
				return i;
		}
	}

	return -1;
}

TInventory Client::GetInventoryItemSlot(uint8_t iSlotPosition)
{
	TInventory pInventory;
	memset(&pInventory, 0, sizeof(pInventory));

	DWORD iInventoryBase = Read4Byte(Read4Byte(GetAddress("KO_PTR_DLG")) + GetAddress("KO_OFF_INVENTORY_BASE"));
	DWORD iItemBase = Read4Byte(iInventoryBase + (GetAddress("KO_OFF_INVENTORY_START") + (4 * iSlotPosition)));

	if (m_Bot->GetPlatformType() == PlatformType::USKO)
	{
		if (Read4Byte(Read4Byte(iItemBase + 0x60)) + Read4Byte(Read4Byte(iItemBase + 0x64)) != 0)
		{
			pInventory.iPos = iSlotPosition;
			pInventory.iItemID = Read4Byte(Read4Byte(iItemBase + 0x60)) + Read4Byte(Read4Byte(iItemBase + 0x64));
			pInventory.iCount = (uint16_t)Read4Byte(iItemBase + 0x68);
			pInventory.iDurability = (uint16_t)Read4Byte(iItemBase + 0x6C);

			return pInventory;
		}
	}
	else
	{
		if (Read4Byte(Read4Byte(iItemBase + 0x68)) + Read4Byte(Read4Byte(iItemBase + 0x6C)) != 0)
		{
			pInventory.iPos = iSlotPosition;
			pInventory.iItemID = Read4Byte(Read4Byte(iItemBase + 0x68)) + Read4Byte(Read4Byte(iItemBase + 0x6C));
			pInventory.iCount = (uint16_t)Read4Byte(iItemBase + 0x70);
			pInventory.iDurability = (uint16_t)Read4Byte(iItemBase + 0x74);

			return pInventory;
		}
	}

	return pInventory;
}

void Client::ReadInventory()
{
	const int SLOT_MAX = 14; // 14 equipped item slots
	const int HAVE_MAX = 28; // 28 inventory slots

	for (int i = 0; i < SLOT_MAX + HAVE_MAX; i++)
	{
		DWORD KO_PTR_DLG = 0x1086148; // CGameProcMain
		DWORD KO_OFF_INVENTORY_BASE = 0x1B0; //CUIInventory
		DWORD KO_OFF_INVENTORY_START = 0x230;

		int InventoryBase = Read4Byte(Read4Byte(KO_PTR_DLG) + KO_OFF_INVENTORY_BASE);
		int Length = Read4Byte(InventoryBase + (KO_OFF_INVENTORY_START + (4 * i)));

		int NameBase = Read4Byte(Length + 0x60);
		int NameLengthBase = Read4Byte(NameBase + 0x1C);

		std::string ItemName;

		if (NameLengthBase > 15)
			ItemName = ReadString(Read4Byte(NameBase + 0xC), NameLengthBase);
		else
			ItemName = ReadString(NameBase + 0xC, NameLengthBase);

		int ItemId = Read4Byte(Read4Byte(Length + 0x60)) + Read4Byte(Read4Byte(Length + 0x64));
		int ItemCount = Read4Byte(Length + 0x68);
		int ItemDurability = Read4Byte(Length + 0x6C);

		printf("Name: %s -  ID: %d - Count: %d - Durability: %d\n", ItemName.c_str(), ItemId, ItemCount, ItemDurability);
	}
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