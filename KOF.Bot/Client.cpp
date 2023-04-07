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
	m_mapSkillUseTime.clear();

	m_vecAvailableSkill.clear();

	m_vecLootList.clear();
	m_bIsMovingToLoot = false;

	m_vecOrigDeathEffectFunction.clear();

	m_msLastBasicAttackTime = std::chrono::milliseconds(0);
}

DWORD Client::GetAddress(std::string szAddressName)
{
	if (!m_Bot) return 0;
	return m_Bot->GetAddress(szAddressName);
}

Ini* Client::GetUserConfiguration()
{
	if (!m_Bot) return 0;
	return m_Bot->GetUserConfiguration();
}

Ini* Client::GetAppConfiguration()
{
	if (!m_Bot) return 0;
	return m_Bot->GetAppConfiguration();
}

int32_t Client::GetID(DWORD iBase)
{
	if (iBase == 0)
		iBase = Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR")));

	return Read4Byte(iBase + GetAddress(skCryptDec("KO_OFF_ID")));
}

int32_t Client::GetProtoID(DWORD iBase)
{
	if (iBase == 0)
		iBase = Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR")));

	return Read4Byte(iBase + GetAddress(skCryptDec("KO_OFF_PROTO_ID")));
}

std::string Client::GetName(DWORD iBase)
{
	if (iBase == 0)
		iBase = Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR")));

	int iNameLen = Read4Byte(iBase + GetAddress(skCryptDec("KO_OFF_NAME_LEN")));

	if (iNameLen > 15)
		return ReadString(Read4Byte(iBase + GetAddress(skCryptDec("KO_OFF_NAME"))), iNameLen);

	return ReadString(iBase + GetAddress(skCryptDec("KO_OFF_NAME")), iNameLen);
}

int16_t Client::GetHp(DWORD iBase)
{
	if (iBase == 0)
		iBase = Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR")));

	return (int16_t)Read4Byte(iBase + GetAddress(skCryptDec("KO_OFF_HP")));
}

int16_t Client::GetMaxHp(DWORD iBase)
{
	if (iBase == 0)
		iBase = Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR")));

	return (int16_t)Read4Byte(iBase + GetAddress(skCryptDec("KO_OFF_MAXHP")));
}

int16_t Client::GetMp(DWORD iBase)
{
	if (iBase == 0)
		iBase = Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR")));

	return (int16_t)Read4Byte(iBase + GetAddress(skCryptDec("KO_OFF_MP")));
}

int16_t Client::GetMaxMp(DWORD iBase)
{
	if (iBase == 0)
		iBase = Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR")));

	return (int16_t)Read4Byte(iBase + GetAddress(skCryptDec("KO_OFF_MAXMP")));
}

uint8_t Client::GetZone(DWORD iBase)
{
	if (iBase == 0)
		iBase = Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR")));

	return (uint8_t)Read4Byte(iBase + GetAddress(skCryptDec("KO_OFF_ZONE")));
}

uint32_t Client::GetGold(DWORD iBase)
{
	if (iBase == 0)
		iBase = Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR")));

	return Read4Byte(iBase + GetAddress(skCryptDec("KO_OFF_GOLD")));
}

uint8_t Client::GetLevel(DWORD iBase)
{
	if (iBase == 0)
		iBase = Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR")));

	return ReadByte(iBase + GetAddress(skCryptDec("KO_OFF_LEVEL")));
}

uint8_t Client::GetNation(DWORD iBase)
{
	if (iBase == 0)
		iBase = Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR")));

	return ReadByte(iBase + GetAddress(skCryptDec("KO_OFF_NATION")));
}

int32_t Client::GetClass(DWORD iBase)
{
	if (iBase == 0)
		iBase = Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR")));

	return Read4Byte(iBase + GetAddress(skCryptDec("KO_OFF_CLASS")));
}

uint64_t Client::GetExp(DWORD iBase)
{
	if (iBase == 0)
		iBase = Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR")));

	return (uint64_t)Read4Byte(iBase + GetAddress(skCryptDec("KO_OFF_EXP")));
}

uint64_t Client::GetMaxExp(DWORD iBase)
{
	if (iBase == 0)
		iBase = Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR")));

	return (uint64_t)Read4Byte(iBase + GetAddress(skCryptDec("KO_OFF_MAXEXP")));
}

uint8_t Client::GetMoveState(DWORD iBase)
{
	if (iBase == 0)
		iBase = Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR")));

	return ReadByte(iBase + GetAddress(skCryptDec("KO_OFF_MOVE_TYPE")));
}

uint8_t Client::GetActionState(DWORD iBase)
{
	if (iBase == 0)
		iBase = Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR")));

	return ReadByte(iBase + GetAddress(skCryptDec("KO_OFF_ACTION_STATE")));
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

float Client::GetX(DWORD iBase)
{
	if (iBase == 0)
		iBase = Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR")));

	return ReadFloat(iBase + GetAddress(skCryptDec("KO_OFF_X")));
}

float Client::GetZ(DWORD iBase)
{
	if (iBase == 0)
		iBase = Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR")));

	return ReadFloat(iBase + GetAddress(skCryptDec("KO_OFF_Z")));
}

float Client::GetY(DWORD iBase)
{
	if (iBase == 0)
		iBase = Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR")));

	return ReadFloat(iBase + GetAddress(skCryptDec("KO_OFF_Y")));
}

uint8_t Client::GetAuthority(DWORD iBase)
{
	if (iBase == 0)
		iBase = Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR")));

	return ReadByte(iBase + GetAddress(skCryptDec("KO_OFF_AUTHORITY")));
}

bool Client::IsDeath(DWORD iBase)
{
	if (iBase == 0)
		iBase = Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR")));

	uint8_t iActionState = GetActionState(iBase);

	if (iActionState == PSA_DYING || iActionState == PSA_DEATH)
		return true;

	return GetHp(iBase) <= 0;
}

int32_t Client::GetClientSelectedTarget()
{
	return Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_MOB")));
}

uint32_t Client::GetClientSelectedTargetBase()
{
	return GetEntityBase(GetClientSelectedTarget());
}

bool Client::IsRogue(int32_t eClass)
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

bool Client::IsMage(int32_t eClass)
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

bool Client::IsWarrior(int32_t eClass)
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

bool Client::IsPriest(int32_t eClass)
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

	if (GetSkillPoint(1) >= 78)
	{
		if (iUndyHpPercent >= 2500)
			return 111654;
		else
			return 112675;
	}
	else if (GetSkillPoint(1) >= 70)
	{
		if (iUndyHpPercent >= 2000)
			return 111654;
		else
			return 112670;
	}
	else if (GetSkillPoint(1) >= 57)
	{
		if (iUndyHpPercent >= 1500)
			return 111654;
		else
			return 111657;
	}
	else if (GetSkillPoint(1) >= 54)
	{
		if (iUndyHpPercent >= 1200)
			return 111654;
		else
			return 111655;
	}
	else if (GetSkillPoint(1) >= 42)
		return 111642;
	else if (GetSkillPoint(1) >= 33)
		return 111633;
	else if (GetSkillPoint(1) >= 24)
		return 111624;
	else if (GetSkillPoint(1) >= 15)
		return 111615;
	else if (GetSkillPoint(1) >= 6)
		return 111606;

	return -1;
}

uint32_t Client::GetProperDefenceBuff()
{
	if (GetSkillPoint(1) >= 76)
		return 112674;
	else if (GetSkillPoint(1) >= 60)
		return 111660;
	else if (GetSkillPoint(1) >= 51)
		return 111651;
	else if (GetSkillPoint(1) >= 39)
		return 111639;
	else if (GetSkillPoint(1) >= 30)
		return 111630;
	else if (GetSkillPoint(1) >= 21)
		return 111621;
	else if (GetSkillPoint(1) >= 12)
		return 111612;
	else if (GetSkillPoint(1) >= 3)
		return 111603;

	return -1;
}

uint32_t Client::GetProperMindBuff()
{
	if (GetSkillPoint(1) >= 45 && GetSkillPoint(1) <= 80)
		return 111645;
	else if (GetSkillPoint(1) >= 36 && GetSkillPoint(1) <= 44)
		return 111636;
	else if (GetSkillPoint(1) >= 27 && GetSkillPoint(1) <= 35)
		return 111627;
	else if (GetSkillPoint(1) >= 9 && GetSkillPoint(1) <= 26)
		return 111609;

	return -1;
}

uint32_t Client::GetProperHeal()
{
	if (GetSkillPoint(0) >= 45)
		return 111545;
	else if (GetSkillPoint(0) >= 36)
		return 111536;
	else if (GetSkillPoint(0) >= 27)
		return 111527;
	else if (GetSkillPoint(0) >= 18)
		return 111518;
	else if (GetSkillPoint(0) >= 9)
		return 111509;
	else if (GetSkillPoint(0) >= 0)
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

DWORD Client::GetEntityBase(int32_t iTargetId)
{
	if(iTargetId == -1)
		return 0;

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
	DWORD iFmbs = GetAddress(skCryptDec("KO_PTR_FMBS"));

	memcpy(byCode + 3, &iFldb, sizeof(iFldb));
	memcpy(byCode + 10, &iTargetId, sizeof(iTargetId));
	memcpy(byCode + 15, &iFmbs, sizeof(iFmbs));

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_Bot->GetInjectedProcessId());

	if (!hProcess)
		return 0;

	LPVOID pAddress = VirtualAllocEx(hProcess, nullptr, sizeof(DWORD), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (!pAddress)
	{
		CloseHandle(hProcess);
		return 0;
	}

	memcpy(byCode + 22, &pAddress, sizeof(pAddress));

	DWORD iBase = 0;
	SIZE_T bytesRead = 0;

	if (ExecuteRemoteCode(hProcess, byCode, sizeof(byCode)))
	{
		if (ReadProcessMemory(hProcess, pAddress, &iBase, sizeof(iBase), &bytesRead) && bytesRead == sizeof(iBase))
		{
			VirtualFreeEx(hProcess, pAddress, 0, MEM_RELEASE);
			CloseHandle(hProcess);
			return iBase;
		}
	}

	VirtualFreeEx(hProcess, pAddress, 0, MEM_RELEASE);
	CloseHandle(hProcess);

	return 0;
}

Vector3 Client::GetTargetPosition()
{
	int32_t iTargetID = GetTarget();

	if(iTargetID == -1)
		return Vector3(0.0f, 0.0f, 0.0f);

	DWORD iBase = GetEntityBase(iTargetID);

	if (iBase > 0)
	{
		return Vector3(
			ReadFloat(iBase + GetAddress("KO_OFF_X")),
			ReadFloat(iBase + GetAddress("KO_OFF_Z")),
			ReadFloat(iBase + GetAddress("KO_OFF_Y")));
	}

	return Vector3(0.0f, 0.0f, 0.0f);
}

int32_t Client::GetTarget()
{
	return Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_MOB")));
}

void Client::SetAuthority(uint8_t iAuthority)
{
	WriteByte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_AUTHORITY")), iAuthority);
}

bool Client::IsBuffActive(int32_t iBuffType)
{ 
	int32_t iSkillBase = Read4Byte(Read4Byte(GetAddress("KO_PTR_DLG")) + GetAddress("KO_OFF_SKILL_BASE"));

	int32_t iSkillContainer1 = Read4Byte(iSkillBase + 0x4);
	int32_t iSkillContainer1Slot = Read4Byte(iSkillContainer1 + GetAddress("KO_OFF_SKILL_SLOT"));

	for (int32_t i = 0; i < 20; i++)
	{
		iSkillContainer1Slot = Read4Byte(iSkillContainer1Slot + 0x0);

		int32_t iAffectedSkill = Read4Byte(iSkillContainer1Slot + 0x8);

		if (iAffectedSkill == 0)
			continue;

		if (Read4Byte(iAffectedSkill + 0x4) == iBuffType)
			return true;
	}

	int32_t iSkillContainer2 = Read4Byte(iSkillBase + 0x8);
	int32_t iSkillContainer2Slot = Read4Byte(iSkillContainer2 + GetAddress("KO_OFF_SKILL_SLOT"));

	for (int32_t i = 0; i < 20; i++)
	{
		iSkillContainer1Slot = Read4Byte(iSkillContainer1Slot + 0x0);

		int32_t iAffectedSkill = Read4Byte(iSkillContainer1Slot + 0x8);

		if (iAffectedSkill == 0)
			continue;

		if (Read4Byte(iAffectedSkill + 0x4) == iBuffType)
			return true;
	}
		
	return false; 
};

bool Client::IsSkillActive(int32_t iSkillID)
{
	int32_t iSkillBase = Read4Byte(Read4Byte(GetAddress("KO_PTR_DLG")) + GetAddress("KO_OFF_SKILL_BASE"));

	int32_t iSkillContainer1 = Read4Byte(iSkillBase + 0x4);
	int32_t iSkillContainer1Slot = Read4Byte(iSkillContainer1 + GetAddress("KO_OFF_SKILL_SLOT"));

	for (int32_t i = 0; i < 20; i++)
	{
		iSkillContainer1Slot = Read4Byte(iSkillContainer1Slot + 0x0);

		int32_t iAffectedSkill = Read4Byte(iSkillContainer1Slot + 0x8);

		if (iAffectedSkill == 0)
			continue;

		if (Read4Byte(iAffectedSkill + 0x0) == iSkillID)
			return true;
	}

	int32_t iSkillContainer2 = Read4Byte(iSkillBase + 0x8);
	int32_t iSkillContainer2Slot = Read4Byte(iSkillContainer2 + GetAddress("KO_OFF_SKILL_SLOT"));

	for (int32_t i = 0; i < 20; i++)
	{
		iSkillContainer1Slot = Read4Byte(iSkillContainer1Slot + 0x0);

		int32_t iAffectedSkill = Read4Byte(iSkillContainer1Slot + 0x8);

		if (iAffectedSkill == 0)
			continue;

		if (Read4Byte(iAffectedSkill + 0x0) == iSkillID)
			return true;
	}

	return false;
};

bool Client::IsBlinking(DWORD iBase)
{
	DWORD iMySelfBase = Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR")));

	if (iBase == 0)
		iBase = iMySelfBase;

	bool bIsMySelf = false;

	if (iBase == iMySelfBase)
		bIsMySelf = true;

	return Read4Byte(iBase + GetAddress(skCryptDec("KO_OFF_BLINKING"))) > 0 || (bIsMySelf && m_PlayerMySelf.bBlinking);
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
	return std::sqrtf((fX1 - fX2) * (fX1 - fX2) +
		(fY1 - fY2) * (fY1 - fY2));
}

uint8_t Client::GetSkillPoint(int32_t Slot)
{
	return ReadByte(Read4Byte(Read4Byte(GetAddress("KO_PTR_DLG")) + GetAddress("KO_OFF_SKILL_POINT_BASE")) + GetAddress("KO_OFF_SKILL_POINT_START") + (Slot * 4));
}

bool Client::GetAvailableSkill(std::vector<__TABLE_UPC_SKILL>** vecAvailableSkills)
{
	if (m_vecAvailableSkill.size() == 0)
		return false;

	*vecAvailableSkills = &m_vecAvailableSkill;

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
		else if (m_Bot->GetPlatformType() == PlatformType::CNKO)
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
		else if (m_Bot->GetPlatformType() == PlatformType::CNKO)
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
		else if (m_Bot->GetPlatformType() == PlatformType::CNKO)
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
		else if (m_Bot->GetPlatformType() == PlatformType::CNKO)
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
	else if (m_Bot->GetPlatformType() == PlatformType::CNKO)
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

int Client::SearchMob(std::vector<EntityInfo>& vecOutMobList)
{
	int Ebp = Read4Byte(Read4Byte(GetAddress("KO_PTR_FLDB")) + 0x28);
	int iMobSize = Read4Byte(Read4Byte(GetAddress("KO_PTR_FLDB")) + 0x2C);
	int Fend = Read4Byte(Read4Byte(Ebp + 0x4) + 0x4);
	int Esi = Read4Byte(Ebp);
	auto Tick = std::chrono::steady_clock::now();

	auto AddEntityInfo = [&](DWORD Base)
	{
		DWORD id = GetID(Base);
		DWORD proto_id = GetProtoID(Base);
		DWORD max_hp = GetMaxHp(Base);
		DWORD hp = GetHp(Base);
		DWORD state = GetActionState(Base);
		DWORD nation = GetNation(Base);
		Vector3 pos = Vector3(GetX(Base), GetZ(Base), GetY(Base));
		float distance = GetDistance(pos);
		bool enemy = IsEnemy(Base);

		vecOutMobList.emplace_back(EntityInfo(Base, id, proto_id, max_hp, hp, state, nation, pos, distance, enemy));
	};

	while (Esi != Ebp 
		&& std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - Tick).count() < 50)
	{
		if (vecOutMobList.size() == iMobSize)
			break;

		DWORD base = Read4Byte(Esi + 0x14);

		if (base == 0) break;

		auto found = std::find_if(vecOutMobList.begin(), vecOutMobList.end(), [&base](const EntityInfo& target) { return target.m_iBase == base; });

		if (found == vecOutMobList.end())
		{
			AddEntityInfo(base);
		}

		DWORD Eax = Read4Byte(Esi + 0x8);

		if (Eax != Fend)
		{
			while (Read4Byte(Eax) != Fend 
				&& std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - Tick).count() < 50)
			{
				Eax = Read4Byte(Eax);
			}

			Esi = Eax;
		}
		else
		{
			DWORD Ebx = Read4Byte(Esi + 0x4);

			while (Esi == Read4Byte(Ebx + 0x8) 
				&& std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - Tick).count() < 50)
			{
				Esi = Ebx;
				Ebx = Read4Byte(Ebx + 0x4);
			}

			if (Read4Byte(Esi + 0x8) != Ebx)
				Esi = Ebx;
		}
	}

	return static_cast<int>(vecOutMobList.size());
}

int Client::SearchPlayer(std::vector<EntityInfo>& vecOutPlayerList)
{
	int Ebp = Read4Byte(Read4Byte(GetAddress("KO_PTR_FLDB")) + 0x30);
	int iPlayerSize = Read4Byte(Read4Byte(GetAddress("KO_PTR_FLDB")) + 0x34);
	int Fend = Read4Byte(Read4Byte(Ebp + 0x4) + 0x4);
	int Esi = Read4Byte(Ebp);
	auto Tick = std::chrono::steady_clock::now();

	auto AddEntityInfo = [&](DWORD Base)
	{
		DWORD id = GetID(Base);
		DWORD proto_id = GetProtoID(Base);
		DWORD max_hp = GetMaxHp(Base);
		DWORD hp = GetHp(Base);
		DWORD state = GetActionState(Base);
		DWORD nation = GetNation(Base);
		Vector3 pos = Vector3(GetX(Base), GetZ(Base), GetY(Base));
		float distance = GetDistance(pos);
		bool enemy = IsEnemy(Base);

		vecOutPlayerList.emplace_back(EntityInfo(Base, id, proto_id, max_hp, hp, state, nation, pos, distance, enemy));
	};

	while (Esi != Ebp 
		&& std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - Tick).count() < 50)
	{
		if (vecOutPlayerList.size() == iPlayerSize)
			break;

		DWORD base = Read4Byte(Esi + 0x14);

		if (base == 0) break;

		auto found = std::find_if(vecOutPlayerList.begin(), vecOutPlayerList.end(), [&base](const EntityInfo& target) { return target.m_iBase == base; });

		if (found == vecOutPlayerList.end())
		{
			AddEntityInfo(base);
		}

		DWORD Eax = Read4Byte(Esi + 0x8);

		if (Eax != Fend)
		{
			while (Read4Byte(Eax) != Fend
				&& std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - Tick).count() < 50)
			{
				Eax = Read4Byte(Eax);
			}

			Esi = Eax;
		}
		else
		{
			DWORD Ebx = Read4Byte(Esi + 0x4);

			while (Esi == Read4Byte(Ebx + 0x8)
				&& std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - Tick).count() < 50)
			{
				Esi = Ebx;
				Ebx = Read4Byte(Ebx + 0x4);
			}

			if (Read4Byte(Esi + 0x8) != Ebx)
				Esi = Ebx;
		}
	}

	return static_cast<int>(vecOutPlayerList.size());
}

bool Client::IsEnemy(DWORD iBase)
{
	return true;


	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_Bot->GetInjectedProcessId());

	if (!hProcess)
		return false;

	BYTE byCode[] =
	{
		0x60,
		0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00,
		0x68, 0x00, 0x00, 0x00, 0x00,
		0xBF, 0x00, 0x00, 0x00, 0x00,
		0xFF, 0xD7,
		0xA3, 0x00, 0x00, 0x00, 0x00,
		0x61,
		0xC3,
	};

	DWORD iChr = GetAddress(skCryptDec("KO_PTR_CHR"));
	DWORD iIsEnemy = GetAddress(skCryptDec("KO_PTR_ENEMY"));

	memcpy(byCode + 3, &iChr, sizeof(iChr));
	memcpy(byCode + 8, &iBase, sizeof(iBase));
	memcpy(byCode + 13, &iIsEnemy, sizeof(iIsEnemy));

	LPVOID pBaseAddress = VirtualAllocEx(hProcess, nullptr, sizeof(DWORD), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (!pBaseAddress)
	{
		CloseHandle(hProcess);
		return false;
	}

	memcpy(byCode + 20, &pBaseAddress, sizeof(pBaseAddress));

	DWORD iEnemyBase = 0;
	SIZE_T bytesRead = 0;

	if (ExecuteRemoteCode(hProcess, byCode, sizeof(byCode)))
	{
		if (ReadProcessMemory(hProcess, pBaseAddress, &iEnemyBase, sizeof(iEnemyBase), &bytesRead) && bytesRead == sizeof(iEnemyBase))
		{
			VirtualFreeEx(hProcess, pBaseAddress, 0, MEM_RELEASE);
			CloseHandle(hProcess);
			return iEnemyBase > 0;
		}
	}

	VirtualFreeEx(hProcess, pBaseAddress, 0, MEM_RELEASE);
	CloseHandle(hProcess);
	return false;
}

void Client::StepCharacterForward(bool bStart)
{
	BYTE byCode[] =
	{
		0x60,
		0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00,
		0x6A, 0x01,
		0x6A, 0x00,
		0xB8, 0x00, 0x00, 0x00, 0x00,
		0xFF, 0xD0,
		0x61,
		0xC3,
	};

	DWORD iDlg = GetAddress(skCryptDec("KO_PTR_DLG"));
	CopyBytes(byCode + 3, iDlg);

	int8_t iStart = bStart ? 1 : 0;
	CopyBytes(byCode + 10, iStart);

	DWORD iWscb = GetAddress(skCryptDec("KO_WSCB"));
	CopyBytes(byCode + 12, iWscb);

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_Bot->GetInjectedProcessId());

	if (!hProcess)
		return;

	ExecuteRemoteCode(hProcess, byCode, sizeof(byCode));
	CloseHandle(hProcess);
}

void Client::BasicAttack()
{
	DWORD iMobBase = GetEntityBase(GetTarget());

	if (iMobBase == 0)
		return;

	if (!IsEnemy(iMobBase))
		return;

	DWORD iHp = GetHp(iMobBase);
	DWORD iMaxHp = GetMaxHp(iMobBase);
	DWORD iState = GetActionState(iMobBase);

	if ((iState == PSA_DYING || iState == PSA_DEATH) || (iMaxHp != 0 && iHp == 0))
		return;

	std::chrono::milliseconds msCurrentTime = duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()
	);

	bool bBasicAttack = GetUserConfiguration()->GetBool(skCryptDec("Attack"), skCryptDec("BasicAttack"), true);
	bool bAttackSpeed = GetUserConfiguration()->GetBool("Attack", "AttackSpeed", false);
	int iAttackSpeedValue = GetUserConfiguration()->GetInt("Attack", "AttackSpeedValue", 1000);

	if (bAttackSpeed)
	{
		if (iAttackSpeedValue == 0)
			iAttackSpeedValue = 1;
	}
	else
		iAttackSpeedValue = 1000;

	if (m_msLastBasicAttackTime > std::chrono::milliseconds(0) 
		&& (msCurrentTime - m_msLastBasicAttackTime) < std::chrono::milliseconds(iAttackSpeedValue))
		return;

	Packet pkt = Packet(PIPE_BASIC_ATTACK);

	pkt << uint8_t(1);

	m_Bot->SendPipeServer(pkt);

	m_msLastBasicAttackTime = duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()
	);
}

DWORD Client::GetSkillBase(uint32_t iSkillID)
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_Bot->GetInjectedProcessId());

	if (!hProcess)
		return false;

	LPVOID pBaseAddress = VirtualAllocEx(hProcess, 0, sizeof(DWORD), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (pBaseAddress == 0)
	{
		CloseHandle(hProcess);
		return 0;
	}

	BYTE byCode[] =
	{
		0x60,
		0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00,
		0x68, 0x00, 0x00, 0x00, 0x00,
		0xBF, 0x00, 0x00, 0x00, 0x00,
		0xFF, 0xD7,
		0xA3, 0x00, 0x00, 0x00, 0x00,
		0x61,
		0xC3,
	};

	DWORD iSbec = GetAddress(skCryptDec("KO_SBEC"));

	CopyBytes(byCode + 3, iSbec);
	CopyBytes(byCode + 8, iSkillID);

	DWORD iSbca = GetAddress(skCryptDec("KO_SBCA"));

	CopyBytes(byCode + 13, iSbca);
	CopyBytes(byCode + 20, pBaseAddress);

	ExecuteRemoteCode(hProcess, byCode, sizeof(byCode));

	DWORD iSkillBase = Read4Byte((DWORD)pBaseAddress);

	VirtualFreeEx(hProcess, pBaseAddress, 0, MEM_RELEASE);
	CloseHandle(hProcess);

	return iSkillBase;
}

void Client::StopMove()
{
	if (GetMoveState() == PSM_STOP)
		return;

	BYTE byCode[] =
	{
		0x60,
		0x6A, 0x00,
		0x6A, 0x00,
		0xB9, 0x00, 0x00, 0x00, 0x00,
		0x8B, 0x09,
		0xB8, 0x00, 0x00, 0x00, 0x00,
		0xFF, 0xD0,
		0x61,
		0xC3,
	};

	DWORD iDLG = GetAddress(skCryptDec("KO_PTR_DLG"));
	CopyBytes(byCode + 6, iDLG);

	DWORD i06 = GetAddress(skCryptDec("KO_PTR_06"));
	CopyBytes(byCode + 13, i06);

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_Bot->GetInjectedProcessId());

	if (!hProcess)
		return;

	ExecuteRemoteCode(hProcess, byCode, sizeof(byCode));
	CloseHandle(hProcess);
}

void Client::PushPhase(DWORD iAddress)
{
	Packet pkt = Packet(PIPE_PUSH_PHASE);
	pkt << DWORD(iAddress);
	m_Bot->SendPipeServer(pkt);
}

void Client::WriteLoginInformation(std::string szAccountId, std::string szPassword)
{
	DWORD dwCGameProcIntroLogin = Read4Byte(GetAddress(skCryptDec("KO_PTR_INTRO")));
	DWORD dwCUILoginIntro = Read4Byte(dwCGameProcIntroLogin + GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO")));

	DWORD dwCN3UIEditIdBase = Read4Byte(dwCUILoginIntro + GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO_ID")));
	DWORD dwCN3UIEditPwBase = Read4Byte(dwCUILoginIntro + GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO_PW")));

	WriteString(dwCN3UIEditIdBase + GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO_ID_INPUT")), szAccountId.c_str());
	Write4Byte(dwCN3UIEditIdBase + GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO_ID_INPUT_LENGTH")), szAccountId.size());
	WriteString(dwCN3UIEditPwBase + GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO_PW_INPUT")), szPassword.c_str());
	Write4Byte(dwCN3UIEditPwBase + GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO_PW_INPUT_LENGTH")), szPassword.size());
}

void Client::ConnectLoginServer(bool bDisconnect)
{
	Packet pkt = Packet(PIPE_LOGIN);
	pkt << uint8_t(1) << uint8_t(bDisconnect);
	m_Bot->SendPipeServer(pkt);
}

void Client::ConnectGameServer(BYTE byServerId)
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_Bot->GetInjectedProcessId());

	if (!hProcess)
		return;

	DWORD dwCGameProcIntroLogin = Read4Byte(GetAddress(skCryptDec("KO_PTR_INTRO")));
	DWORD dwCUILoginIntro = Read4Byte(dwCGameProcIntroLogin + GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO")));

	Write4Byte(dwCUILoginIntro + GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO_SERVER_INDEX")), byServerId);

	BYTE byCode[] =
	{
		0x60,
		0x8B, 0x0D, 0x0, 0x0, 0x0, 0x0,
		0xBF, 0x0, 0x0, 0x0, 0x0,
		0xFF, 0xD7,
		0x61,
		0xC3,
	};

	DWORD dwPtrIntro = GetAddress(skCryptDec("KO_PTR_INTRO"));
	CopyBytes(byCode + 3, dwPtrIntro);

	DWORD dwPtrServerSelect = GetAddress(skCryptDec("KO_PTR_SERVER_SELECT"));
	CopyBytes(byCode + 8, dwPtrServerSelect);

	ExecuteRemoteCode(hProcess, byCode, sizeof(byCode));
	CloseHandle(hProcess);
}

void Client::SelectCharacterSkip()
{
	Packet pkt = Packet(PIPE_SELECT_CHARACTER_SKIP);

	pkt << uint8_t(1);

	m_Bot->SendPipeServer(pkt);
}

void Client::SelectCharacterLeft()
{
	Packet pkt = Packet(PIPE_SELECT_CHARACTER_LEFT);

	pkt << uint8_t(1);

	m_Bot->SendPipeServer(pkt);
}

void Client::SelectCharacterRight()
{
	Packet pkt = Packet(PIPE_SELECT_CHARACTER_RIGHT);

	pkt << uint8_t(1);

	m_Bot->SendPipeServer(pkt);
}

void Client::SelectCharacter()
{
	Packet pkt = Packet(PIPE_SELECT_CHARACTER_ENTER);

	pkt << uint8_t(1);

	m_Bot->SendPipeServer(pkt);
}

void Client::SendPacket(Packet vecBuffer)
{
	Packet pkt = Packet(PIPE_SEND_PACKET);

	pkt << vecBuffer.size();
	pkt.append(vecBuffer.contents(), vecBuffer.size());

	m_Bot->SendPipeServer(pkt);
}

void Client::UseSkillWithPacket(TABLE_UPC_SKILL pSkillData, int32_t iTargetID, bool iAttacking)
{
	Vector3 v3MyPosition = GetPosition();
	Vector3 v3TargetPosition = GetTargetPosition();

	switch (pSkillData.iTarget)
	{
	case SkillTargetType::TARGET_SELF:
	case SkillTargetType::TARGET_FRIEND_WITHME:
	case SkillTargetType::TARGET_FRIEND_ONLY:
	case SkillTargetType::TARGET_PARTY:
	case SkillTargetType::TARGET_NPC_ONLY:
	case SkillTargetType::TARGET_ENEMY_ONLY:
	case SkillTargetType::TARGET_ALL:
	case 9: // For God Mode
	case SkillTargetType::TARGET_AREA_FRIEND:
	case SkillTargetType::TARGET_AREA_ALL:
	case SkillTargetType::TARGET_DEAD_FRIEND_ONLY:
	{
		if (pSkillData.iReCastTime != 0)
		{
			if (GetMoveState() != PSM_STOP)
			{
				SendMovePacket(v3MyPosition, v3MyPosition, 0, 0);
			}

			SendStartSkillCastingAtTargetPacket(pSkillData, iTargetID);
			std::this_thread::sleep_for(std::chrono::milliseconds(pSkillData.iReCastTime * 100));
		}

		uint32_t iArrowCount = 0;

		std::map<uint32_t, __TABLE_UPC_SKILL_EXTENSION2>* pSkillExtension2;
		if (m_Bot->GetSkillExtension2Table(&pSkillExtension2))
		{
			auto pSkillExtension2Data = pSkillExtension2->find(pSkillData.iID);

			if (pSkillExtension2Data != pSkillExtension2->end())
				iArrowCount = pSkillExtension2Data->second.iArrowCount;
		}

		if ((iArrowCount == 0 && pSkillData.iFlyingFX != 0) || iArrowCount == 1)
			SendStartFlyingAtTargetPacket(pSkillData, iTargetID, v3TargetPosition);

		if (iArrowCount > 1)
		{
			SendStartFlyingAtTargetPacket(pSkillData, iTargetID, Vector3(0.0f, 0.0f, 0.0f), 1);

			float fDistance = GetDistance(v3TargetPosition);

			switch (iArrowCount)
			{
			case 3:
			{
				iArrowCount = 1;

				if (fDistance <= 5.0f)
					iArrowCount = 3;
				else if (fDistance < 16.0f)
					iArrowCount = 2;
			};
			break;

			case 5:
			{
				iArrowCount = 1;

				if (fDistance <= 5.0f)
					iArrowCount = 5;
				else if (fDistance <= 6.0f)
					iArrowCount = 4;
				else if (fDistance <= 8.0f)
					iArrowCount = 3;
				else if (fDistance < 16.0f)
					iArrowCount = 2;
			}
			break;
			}

			for (uint32_t i = 0; i < iArrowCount; i++)
			{
				SendStartSkillMagicAtTargetPacket(pSkillData, iTargetID, Vector3(0.0f, 0.0f, 0.0f), (i + 1));
				SendStartMagicAtTarget(pSkillData, iTargetID, v3TargetPosition, (i + 1));
			}
		}
		else
			SendStartSkillMagicAtTargetPacket(pSkillData, iTargetID, v3TargetPosition);

	}
	break;

	case SkillTargetType::TARGET_AREA:
	case SkillTargetType::TARGET_PARTY_ALL:
	case SkillTargetType::TARGET_AREA_ENEMY:
	{
		if (pSkillData.iReCastTime != 0)
		{
			if (GetMoveState() != PSM_STOP)
			{
				SendMovePacket(v3MyPosition, v3MyPosition, 0, 0);
			}

			SendStartSkillCastingAtPosPacket(pSkillData, v3TargetPosition);
			std::this_thread::sleep_for(std::chrono::milliseconds(pSkillData.iReCastTime * 100));
		}

		if (pSkillData.iFlyingFX != 0)
			SendStartFlyingAtTargetPacket(pSkillData, iTargetID, v3TargetPosition);

		SendStartSkillMagicAtPosPacket(pSkillData, v3TargetPosition);
	}
	break;
	}
}

void Client::UseSkill(TABLE_UPC_SKILL pSkillData, int32_t iTargetID, int32_t iPriority, bool iAttacking)
{
	if (pSkillData.iReCastTime != 0)
	{
		if (GetActionState() == PSA_SPELLMAGIC)
			return;
	}

	Packet pkt = Packet(PIPE_USE_SKILL);

	pkt << iTargetID << pSkillData.iID << iPriority << uint8_t(iAttacking);

	m_Bot->SendPipeServer(pkt);

	SetSkillUseTime(pSkillData.iID,
		duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
	);
}

void Client::SendStartSkillCastingAtTargetPacket(TABLE_UPC_SKILL pSkillData, int32_t iTargetID)
{
	Packet pkt = Packet(WIZ_MAGIC_PROCESS);

	pkt
		<< uint8_t(SkillMagicType::SKILL_MAGIC_TYPE_CASTING)
		<< pSkillData.iID
		<< m_PlayerMySelf.iID
		<< iTargetID
		<< uint32_t(0) << uint32_t(0) << uint32_t(0) << uint32_t(0) << uint32_t(0) << uint32_t(0) << uint32_t(0)
		<< int16_t(pSkillData.iReCastTime);

	if (pSkillData.iTarget == SkillTargetType::TARGET_PARTY)
	{
		pkt << uint32_t(0);
	}

	SendPacket(pkt);
}

void Client::SendStartSkillCastingAtPosPacket(TABLE_UPC_SKILL pSkillData, Vector3 v3TargetPosition)
{
	Packet pkt = Packet(WIZ_MAGIC_PROCESS);

	pkt
		<< uint8_t(SkillMagicType::SKILL_MAGIC_TYPE_CASTING)
		<< pSkillData.iID
		<< m_PlayerMySelf.iID
		<< int32_t(-1)
		<< uint32_t(v3TargetPosition.m_fX * 10.0f)
		<< int32_t(v3TargetPosition.m_fZ * 10.0f)
		<< uint32_t(v3TargetPosition.m_fY * 10.0f)
		<< uint32_t(0) << uint32_t(0) << uint32_t(0) << uint32_t(0)
		<< int16_t(pSkillData.iReCastTime);

	SendPacket(pkt);
}

void Client::SendStartFlyingAtTargetPacket(TABLE_UPC_SKILL pSkillData, int32_t iTargetID, Vector3 v3TargetPosition, uint16_t arrowIndex)
{
	Packet pkt = Packet(WIZ_MAGIC_PROCESS);

	pkt
		<< uint8_t(SkillMagicType::SKILL_MAGIC_TYPE_FLYING)
		<< pSkillData.iID
		<< m_PlayerMySelf.iID
		<< iTargetID
		<< uint32_t(v3TargetPosition.m_fX * 10.0f)
		<< int32_t(v3TargetPosition.m_fZ * 10.0f)
		<< uint32_t(v3TargetPosition.m_fY * 10.0f)
		<< arrowIndex
		<< uint32_t(0) << uint32_t(0) << int16_t(0);

	SendPacket(pkt);
}

void Client::SendStartSkillMagicAtTargetPacket(TABLE_UPC_SKILL pSkillData, int32_t iTargetID, Vector3 v3TargetPosition, uint16_t arrowIndex)
{
	Packet pkt = Packet(WIZ_MAGIC_PROCESS);

	pkt
		<< uint8_t(SkillMagicType::SKILL_MAGIC_TYPE_EFFECTING)
		<< pSkillData.iID
		<< m_PlayerMySelf.iID
		<< iTargetID;

	if (pSkillData.iCastTime == 0)
		pkt << uint32_t(1) << uint32_t(1) << uint32_t(0);
	else
	{
		pkt
			<< uint32_t(v3TargetPosition.m_fX * 10.0f)
			<< int32_t(v3TargetPosition.m_fZ * 10.0f)
			<< uint32_t(v3TargetPosition.m_fY * 10.0f);
	}

	if (pSkillData.iTarget == SkillTargetType::TARGET_PARTY)
	{
		pkt << arrowIndex
			<< uint32_t(0) << uint32_t(0) << int16_t(0);
	}
	else
	{
		pkt << arrowIndex
			<< uint32_t(0) << uint32_t(0) << int16_t(0) << int16_t(0) << int16_t(0);
	}

	SendPacket(pkt);
}

void Client::SendStartSkillMagicAtPosPacket(TABLE_UPC_SKILL pSkillData, Vector3 v3TargetPosition)
{
	Packet pkt = Packet(WIZ_MAGIC_PROCESS);

	pkt
		<< uint8_t(SkillMagicType::SKILL_MAGIC_TYPE_EFFECTING)
		<< pSkillData.iID
		<< m_PlayerMySelf.iID
		<< int32_t(-1)
		<< uint32_t(v3TargetPosition.m_fX * 10.0f)
		<< int32_t(v3TargetPosition.m_fZ * 10.0f)
		<< uint32_t(v3TargetPosition.m_fY * 10.0f)
		<< uint32_t(0) << uint32_t(0) << uint32_t(0);

	SendPacket(pkt);
}

void Client::SendStartMagicAtTarget(TABLE_UPC_SKILL pSkillData, int32_t iTargetID, Vector3 v3TargetPosition, uint16_t arrowIndex)
{
	Packet pkt = Packet(WIZ_MAGIC_PROCESS);

	pkt
		<< uint8_t(SkillMagicType::SKILL_MAGIC_TYPE_FAIL)
		<< pSkillData.iID
		<< m_PlayerMySelf.iID
		<< iTargetID
		<< uint32_t(v3TargetPosition.m_fX * 10.0f)
		<< int32_t(v3TargetPosition.m_fZ * 10.0f)
		<< uint32_t(v3TargetPosition.m_fY * 10.0f)
		<< int32_t(-101)
		<< arrowIndex
		<< uint32_t(0) << uint32_t(0) << int16_t(0) << int16_t(0) << int16_t(0);

	SendPacket(pkt);
}

void Client::SendCancelSkillPacket(TABLE_UPC_SKILL pSkillData)
{
	Packet pkt = Packet(WIZ_MAGIC_PROCESS);

	pkt
		<< uint8_t(SkillMagicType::SKILL_MAGIC_TYPE_CANCEL)
		<< pSkillData.iID
		<< m_PlayerMySelf.iID
		<< m_PlayerMySelf.iID
		<< uint32_t(0) << uint32_t(0) << uint32_t(0)
		<< uint32_t(0) << uint32_t(0) << uint32_t(0);

	SendPacket(pkt);
}

void Client::SendMovePacket(Vector3 vecStartPosition, Vector3 vecTargetPosition, int16_t iMoveSpeed, uint8_t iMoveType)
{
	Packet pkt = Packet(WIZ_MOVE);

	pkt
		<< uint16_t(vecStartPosition.m_fX * 10.0f)
		<< uint16_t(vecStartPosition.m_fY * 10.0f)
		<< int16_t(vecStartPosition.m_fZ * 10.0f)
		<< iMoveSpeed
		<< iMoveType
		<< uint16_t(vecTargetPosition.m_fX * 10.0f)
		<< uint16_t(vecTargetPosition.m_fY * 10.0f)
		<< int16_t(vecTargetPosition.m_fZ * 10.0f);

	SendPacket(pkt);
}

void Client::SendTownPacket()
{
	Packet pkt = Packet(WIZ_HOME);

	SendPacket(pkt);
}

void Client::SetPosition(Vector3 v3Position)
{
	WriteFloat(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_X")), v3Position.m_fX);
	WriteFloat(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_Y")), v3Position.m_fY);
	WriteFloat(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_Z")), v3Position.m_fZ);
}

void Client::SetMovePosition(Vector3 v3MovePosition)
{
	if (v3MovePosition.m_fX == 0.0f && v3MovePosition.m_fY == 0.0f && v3MovePosition.m_fZ == 0.0f)
	{
		WriteByte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_MOVE_TYPE")), 0);
		WriteFloat(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_GOX")), v3MovePosition.m_fX);
		WriteFloat(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_GOY")), v3MovePosition.m_fY);
		WriteFloat(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_GOZ")), v3MovePosition.m_fZ);
		WriteByte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_MOVE")), 0);
	}
	else
	{
		WriteByte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_MOVE_TYPE")), 2);
		WriteFloat(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_GOX")), v3MovePosition.m_fX);
		WriteFloat(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_GOY")), v3MovePosition.m_fY);
		WriteFloat(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_GOZ")), v3MovePosition.m_fZ);
		WriteByte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_MOVE")), 1);

	}
}

void Client::SendBasicAttackPacket(int32_t iTargetID, float fInterval, float fDistance)
{
	Packet pkt = Packet(WIZ_ATTACK);

	pkt
		<< uint8_t(1) << uint8_t(1)
		<< iTargetID
		<< uint16_t(fInterval)
		<< uint16_t(fDistance)
		<< uint8_t(0) << uint8_t(1);

	SendPacket(pkt);
}

void Client::SendShoppingMall(ShoppingMallType eType)
{
	Packet pkt = Packet(WIZ_SHOPPING_MALL);

	switch (eType)
	{
	case ShoppingMallType::STORE_CLOSE:
		pkt << uint8_t(eType);
		break;

	case ShoppingMallType::STORE_PROCESS:
	case ShoppingMallType::STORE_LETTER:
		pkt << uint8_t(eType) << uint8_t(1);
		break;
	}

	SendPacket(pkt);
}

void Client::SendRotation(float fRotation)
{
	Packet pkt = Packet(WIZ_ROTATE);

	pkt << int16_t(fRotation * 100.f);

	SendPacket(pkt);
}

void Client::SendRequestBundleOpen(uint32_t iBundleID)
{
	Packet pkt = Packet(WIZ_BUNDLE_OPEN_REQ);

	pkt << uint32_t(iBundleID);

	SendPacket(pkt);
}

void Client::SendBundleItemGet(uint32_t iBundleID, uint32_t iItemID, int16_t iIndex)
{
	Packet pkt = Packet(WIZ_ITEM_GET);

	pkt << uint32_t(iBundleID) << uint32_t(iItemID) << int16_t(iIndex);

	SendPacket(pkt);
}

void Client::SendItemMovePacket(uint8_t iType, uint8_t iDirection, uint32_t iItemID, uint8_t iCurrentPosition, uint8_t iTargetPosition)
{
	Packet pkt = Packet(WIZ_ITEM_MOVE);

	pkt << uint8_t(iType) << uint8_t(iDirection) << uint32_t(iItemID) << uint8_t(iCurrentPosition) << uint8_t(iTargetPosition);

	SendPacket(pkt);
}

void Client::SendTargetHpRequest(int32_t bTargetID, bool bBroadcast)
{
	Packet pkt = Packet(WIZ_TARGET_HP);

	pkt << int32_t(bTargetID) << uint8_t(bBroadcast ? 1 : 0);

	SendPacket(pkt);
}

void Client::SetTarget(uint32_t iTargetBase)
{
	BYTE byCode[] =
	{
		0x60,
		0xB9, 0x00, 0x00, 0x00, 0x00,
		0x8B, 0x09,
		0xB8, 0x00, 0x00, 0x00, 0x00,
		0x50,
		0xBE, 0x00, 0x00, 0x00, 0x00,
		0xFF, 0xD6,
		0x61,
		0xC3,
	};

	DWORD iDlg = GetAddress(skCryptDec("KO_PTR_DLG"));
	CopyBytes(byCode + 2, iDlg);
	CopyBytes(byCode + 9, iTargetBase);

	DWORD iSelectMob = GetAddress(skCryptDec("KO_SELECT_MOB"));
	CopyBytes(byCode + 15, iSelectMob);

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_Bot->GetInjectedProcessId());

	if (!hProcess)
		return;

	ExecuteRemoteCode(hProcess, byCode, sizeof(byCode));
	CloseHandle(hProcess);
}

bool Client::UseItem(uint32_t iItemID)
{
	std::map<uint32_t, __TABLE_ITEM>* pItemTable;
	if (!m_Bot->GetItemTable(&pItemTable))
		return false;

	std::map<uint32_t, __TABLE_UPC_SKILL>* pSkillTable;
	if (!m_Bot->GetSkillTable(&pSkillTable))
		return false;

	auto pItemData = pItemTable->find(iItemID);

	if (pItemData != pItemTable->end())
	{
		auto pSkillData = pSkillTable->find(pItemData->second.dwEffectID1);

		if (pSkillData != pSkillTable->end())
		{
			/*std::chrono::milliseconds msNow = duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch()
			);

			std::chrono::milliseconds msLastSkillUseItem = Client::GetSkillUseTime(pSkillData->second.iID);

			if (pSkillData->second.iCooldown > 0 && msLastSkillUseItem.count() > 0)
			{
				int64_t iSkillCooldownTime = static_cast<int64_t>(pSkillData->second.iCooldown) * 100;

				if ((msLastSkillUseItem.count() + iSkillCooldownTime) > msNow.count())
					return false;
			}*/

			UseSkill(pSkillData->second, GetID(), 1);

			return true;
		}
	}

	return false;
}

void Client::PatchDeathEffect(bool bValue)
{
	if (bValue)
	{
		if (m_vecOrigDeathEffectFunction.size() == 0)
			m_vecOrigDeathEffectFunction = ReadBytes(GetAddress(skCryptDec("KO_DEATH_EFFECT")), 2);

		std::vector<uint8_t> vecPatch = { 0x90, 0x90 };
		WriteBytes(GetAddress(skCryptDec("KO_DEATH_EFFECT")), vecPatch);
	}
	else
	{
		if (m_vecOrigDeathEffectFunction.size() > 0)
			WriteBytes(GetAddress(skCryptDec("KO_DEATH_EFFECT")), m_vecOrigDeathEffectFunction);
	}
}

void Client::SendNpcEvent(int32_t iTargetID)
{
	Packet pkt = Packet(WIZ_NPC_EVENT);

	pkt << uint8_t(1) << int32_t(iTargetID) << int32_t(-1);

	SendPacket(pkt);
}

void Client::SendItemTradeBuy(uint32_t iSellingGroup, int32_t iNpcId, int32_t iItemId, uint8_t iInventoryPosition, int16_t iCount, uint8_t iShopPage, uint8_t iShopPosition)
{
	Packet pkt = Packet(WIZ_ITEM_TRADE);

	pkt
		<< uint8_t(1)
		<< uint32_t(iSellingGroup)
		<< uint32_t(iNpcId)
		<< uint8_t(1)
		<< int32_t(iItemId)
		<< uint8_t(iInventoryPosition)
		<< int16_t(iCount)
		<< uint8_t(iShopPage)
		<< uint8_t(iShopPosition);

	SendPacket(pkt);
}

void Client::SendItemTradeBuy(uint32_t iSellingGroup, int32_t iNpcId, std::vector<SSItemBuy> vecItemList)
{
	Packet pkt = Packet(WIZ_ITEM_TRADE);

	pkt
		<< uint8_t(1)
		<< uint32_t(iSellingGroup)
		<< uint32_t(iNpcId)
		<< uint8_t(vecItemList.size());

	for (auto& e : vecItemList)
	{
		pkt
			<< int32_t(e.m_iItemId)
			<< uint8_t(e.m_iInventoryPosition)
			<< int16_t(e.m_iCount)
			<< uint8_t(e.m_iShopPage)
			<< uint8_t(e.m_iShopPosition);
	}

	SendPacket(pkt);
}

void Client::SendItemTradeSell(uint32_t iSellingGroup, int32_t iNpcId, int32_t iItemId, uint8_t iInventoryPosition, int16_t iCount)
{
	Packet pkt = Packet(WIZ_ITEM_TRADE);

	pkt
		<< uint8_t(2)
		<< uint32_t(iSellingGroup)
		<< uint32_t(iNpcId)
		<< uint8_t(1)
		<< int32_t(iItemId)
		<< uint8_t(iInventoryPosition)
		<< int16_t(iCount);

	SendPacket(pkt);
}

void Client::SendItemTradeSell(uint32_t iSellingGroup, int32_t iNpcId, std::vector<SSItemSell> vecItemList)
{
	Packet pkt = Packet(WIZ_ITEM_TRADE);

	pkt
		<< uint8_t(2)
		<< uint32_t(iSellingGroup)
		<< uint32_t(iNpcId)
		<< uint8_t(vecItemList.size());

	for (auto& e : vecItemList)
	{
		pkt
			<< int32_t(e.m_iItemId)
			<< uint8_t(e.m_iInventoryPosition)
			<< int16_t(e.m_iCount);
	}

	SendPacket(pkt);
}

void Client::SendItemRepair(uint8_t iDirection, uint8_t iInventoryPosition, int32_t iNpcId, int32_t iItemId)
{
	Packet pkt = Packet(WIZ_ITEM_REPAIR);

	pkt
		<< uint8_t(iDirection)
		<< uint8_t(iInventoryPosition)
		<< int32_t(iNpcId)
		<< int32_t(iItemId);

	SendPacket(pkt);
}

bool Client::IsNeedRepair()
{
	for (uint8_t i = 0; i < SLOT_MAX; i++)
	{
		switch (i)
		{
		case 1:
		case 4:
		case 6:
		case 8:
		case 10:
		case 12:
		case 13:
		{
			TInventory pInventory = GetInventoryItemSlot(i);

			if (pInventory.iItemID == 0)
				continue;

			if (pInventory.iDurability != 0)
				continue;

			return true;
		}
		break;
		}
	}

	return false;
}

bool Client::IsNeedSupply()
{
	auto jSupplyList = m_Bot->GetSupplyList();

	if (jSupplyList.size() > 0)
	{
		for (size_t i = 0; i < jSupplyList.size(); i++)
		{
			std::string szItemIdAttribute = skCryptDec("itemid");
			std::string szSellingGroupAttribute = skCryptDec("sellinggroup");
			std::string szCountAttribute = skCryptDec("count");

			int32_t iSupplyItemId = jSupplyList[i][szItemIdAttribute.c_str()].get<int32_t>();
			int32_t iSupplyItemCount = jSupplyList[i][szCountAttribute.c_str()].get<int32_t>();

			std::vector<int> vecSupplyList = GetUserConfiguration()->GetInt(skCryptDec("Supply"), skCryptDec("Enable"), std::vector<int>());

			bool bSelected = std::find(vecSupplyList.begin(), vecSupplyList.end(), iSupplyItemId) != vecSupplyList.end();

			if (!bSelected)
				continue;

			TInventory pInventoryItem = GetInventoryItem(iSupplyItemId);

			if (pInventoryItem.iItemID == 0)
				return true;

			if (pInventoryItem.iItemID != 0)
			{
				if (pInventoryItem.iCount <= 3 && pInventoryItem.iCount < iSupplyItemCount)
					return true;
			}
		}
	}

	return false;
}

uint8_t Client::GetRepresentZone(uint8_t iZone)
{
	switch (iZone)
	{
		case ZoneInfo::ZONE_KARUS:
		case ZoneInfo::ZONE_KARUS2:
		{
			return ZoneInfo::ZONE_KARUS;
		}
		break;

		case ZoneInfo::ZONE_ELMORAD:
		case ZoneInfo::ZONE_ELMORAD2:
		{
			return ZoneInfo::ZONE_ELMORAD;
		}
		break;

		case ZoneInfo::ZONE_KARUS_ESLANT:
		case ZoneInfo::ZONE_KARUS_ESLANT2:
		{
			return ZoneInfo::ZONE_KARUS_ESLANT;
		}
		break;

		case ZoneInfo::ZONE_ELMORAD_ESLANT:
		case ZoneInfo::ZONE_ELMORAD_ESLANT2:
		{
			return ZoneInfo::ZONE_ELMORAD_ESLANT;
		}
		break;

		case ZoneInfo::ZONE_MORADON:
		case ZoneInfo::ZONE_MORADON2:
		case ZoneInfo::ZONE_MORADON3:
		case ZoneInfo::ZONE_MORADON4:
		case ZoneInfo::ZONE_MORADON5:
		{
			return ZoneInfo::ZONE_MORADON;
		}
		break;

		default:
		{
			return iZone;
		}
		break;	
	}
}

bool Client::IsTransformationAvailableZone()
{
	uint8_t iZoneID = GetRepresentZone(GetZone());

	switch (iZoneID)
	{
		case ZONE_DELOS:
		case ZONE_BIFROST:
		case ZONE_ARENA:
		case ZONE_BATTLE:
		case ZONE_BATTLE2:
		case ZONE_BATTLE3:
		case ZONE_BATTLE4:
		case ZONE_BATTLE5:
		case ZONE_BATTLE6:
		case ZONE_SNOW_BATTLE:
		case ZONE_RONARK_LAND:
		case ZONE_ARDREAM:
		case ZONE_RONARK_LAND_BASE:
		case ZONE_KROWAZ_DOMINION:
		case ZONE_MONSTER_STONE1:
		case ZONE_MONSTER_STONE2:
		case ZONE_MONSTER_STONE3:
		case ZONE_BORDER_DEFENSE_WAR:
		case ZONE_CHAOS_DUNGEON:
		case ZONE_UNDER_CASTLE:
		case ZONE_JURAD_MOUNTAIN:
		case ZONE_PRISON:
		case ZONE_DRAKI_TOWER:
		{
			return false;
		}
		break;

		default:
		{
			return true;
		}
		break;
	}
}

bool Client::IsTransformationAvailable()
{
	if (!IsTransformationAvailableZone())
		return false;

	std::map<uint32_t, __TABLE_DISGUISE_RING>* mapDisguiseTable;
	if (!m_Bot->GetDisguiseRingTable(&mapDisguiseTable))
		return false;

	for (auto& [k, v] : *mapDisguiseTable)
	{
		if (IsSkillActive(v.iSkillID))
			return false;
	}
	
	return true;
}

void Client::RefreshCaptcha()
{
	Packet pkt = Packet(WIZ_CAPTCHA);

	pkt
		<< uint8_t(1)
		<< uint8_t(1);

	SendPacket(pkt);
}

void Client::SendCaptcha(std::string szCode)
{
	Packet pkt = Packet(WIZ_CAPTCHA);

	pkt.DByte();

	pkt
		<< uint8_t(1)
		<< uint8_t(2)
		<< szCode;

	SendPacket(pkt);
}

int32_t Client::GetPartyMemberCount()
{
	return Read4Byte(Read4Byte(Read4Byte(GetAddress("KO_PTR_DLG")) + GetAddress("KO_OFF_PARTY_BASE")) + GetAddress("KO_OFF_PARTY_LIST_COUNT"));
}

bool Client::GetPartyList(std::vector<Party>& vecParty)
{
	return false;
	int32_t iBase = Read4Byte(Read4Byte(Read4Byte(Read4Byte(GetAddress("KO_PTR_DLG")) + GetAddress("KO_OFF_PARTY_BASE")) + GetAddress("KO_OFF_PARTY_LIST")));
	int32_t iPartyMemberCount = GetPartyMemberCount();

	if (iPartyMemberCount == 0)
		return false;

	for (int i = 0; i <= iPartyMemberCount - 1; i++)
	{
		Party pParty;
		memset(&pParty, 0, sizeof(pParty));

		pParty.iID = Read4Byte(iBase + GetAddress("KO_OFF_PARTY_ID"));

		pParty.iHP = (int16_t)Read4Byte(iBase + GetAddress("KO_OFF_PARTY_HP"));
		pParty.iMaxHP = (int16_t)Read4Byte(iBase + GetAddress("KO_OFF_PARTY_MAXHP"));
		pParty.iMP = (int16_t)Read4Byte(iBase + GetAddress("KO_OFF_PARTY_MP"));
		pParty.iMaxMP = (int16_t)Read4Byte(iBase + GetAddress("KO_OFF_PARTY_MAXMP"));
		pParty.iCure = Read4Byte(iBase + GetAddress("KO_OFF_PARTY_CURE"));

		int MemberNameLen = Read4Byte(iBase + GetAddress("KO_OFF_PARTY_NAME_LENGTH"));

		if (MemberNameLen > 15)
			pParty.szName = ReadString(Read4Byte(iBase + GetAddress("KO_OFF_PARTY_NAME")), MemberNameLen);
		else
			pParty.szName = ReadString(iBase + GetAddress("KO_OFF_PARTY_NAME"), MemberNameLen);

		vecParty.push_back(pParty);

		iBase = Read4Byte(iBase);
	}

	return vecParty.size() > 0;
}

bool Client::GetPartyMember(int32_t iID, Party& pPartyMember)
{
	int32_t iBase = Read4Byte(Read4Byte(Read4Byte(Read4Byte(GetAddress("KO_PTR_DLG")) + GetAddress("KO_OFF_PARTY_BASE")) + GetAddress("KO_OFF_PARTY_LIST")));
	int32_t iPartyMemberCount = GetPartyMemberCount();

	if (iPartyMemberCount == 0)
		return false;

	for (int i = 0; i <= iPartyMemberCount - 1; i++)
	{
		Party pParty;
		memset(&pParty, 0, sizeof(pParty));

		pParty.iID = Read4Byte(iBase + GetAddress("KO_OFF_PARTY_ID"));

		if (pParty.iID != iID)
		{
			iBase = Read4Byte(iBase);
			continue;
		}

		pParty.iHP = (int16_t)Read4Byte(iBase + GetAddress("KO_OFF_PARTY_HP"));
		pParty.iMaxHP = (int16_t)Read4Byte(iBase + GetAddress("KO_OFF_PARTY_MAXHP"));
		pParty.iMP = (int16_t)Read4Byte(iBase + GetAddress("KO_OFF_PARTY_MP"));
		pParty.iMaxMP = (int16_t)Read4Byte(iBase + GetAddress("KO_OFF_PARTY_MAXMP"));
		pParty.iCure = Read4Byte(iBase + GetAddress("KO_OFF_PARTY_CURE"));

		int MemberNameLen = Read4Byte(iBase + GetAddress("KO_OFF_PARTY_NAME_LENGTH"));

		if (MemberNameLen > 15)
			pParty.szName = ReadString(Read4Byte(iBase + GetAddress("KO_OFF_PARTY_NAME")), MemberNameLen);
		else
			pParty.szName = ReadString(iBase + GetAddress("KO_OFF_PARTY_NAME"), MemberNameLen);

		pPartyMember = pParty;
		return true;
	}

	return false;
}

bool Client::GetPartyMemberBuffInfo(int32_t iMemberID, PartyBuffInfo & pPartyBuffInfo)
{
	auto it = m_mapPartyBuffInfo.find(iMemberID);

	if (it != m_mapPartyBuffInfo.end())
	{
		pPartyBuffInfo = it->second;
		return true;
	}

	return false;
}

void Client::SetPartyMemberBuffInfo(int32_t iMemberID, PartyBuffInfo pPartyBuffInfo)
{
	auto it = m_mapPartyBuffInfo.find(iMemberID);

	if (it == m_mapPartyBuffInfo.end())
		m_mapPartyBuffInfo.insert(std::pair(iMemberID, pPartyBuffInfo));
	else
		it->second = pPartyBuffInfo;
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

void Client::Write4Byte(DWORD dwAddress, int iValue)
{
	m_Bot->Write4Byte(dwAddress, iValue);
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

bool Client::ExecuteRemoteCode(HANDLE hProcess, BYTE* codes, size_t psize)
{
	return m_Bot->ExecuteRemoteCode(hProcess, codes, psize);
}

bool Client::ExecuteRemoteCode(HANDLE hProcess, LPVOID pAddress)
{
	return m_Bot->ExecuteRemoteCode(hProcess, pAddress);
}