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

	m_mapActiveBuffList.clear();
	m_mapSkillUseTime.clear();

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

uint8_t Client::GetNation()
{
	return ReadByte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_NATION")));
}

int32_t Client::GetClass()
{
	return Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_CLASS")));
}

uint64_t Client::GetExp()
{
	return (uint64_t)Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_EXP")));
}

uint64_t Client::GetMaxExp()
{
	return (uint64_t)Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_MAXEXP")));
}

uint8_t Client::GetMoveState()
{
	return ReadByte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_MOVE_TYPE")));
}

uint8_t Client::GetActionState()
{
	return ReadByte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_ACTION_STATE")));
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

int32_t Client::GetClientSelectedTarget()
{
	return Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_MOB")));
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

DWORD Client::GetMobBase(int32_t iTargetId)
{
	if (iTargetId == -1)
		return 0;

	LPVOID pAddress = VirtualAllocEx(m_Bot->GetInjectedProcessHandle(), 0, 1, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (pAddress == 0)
	{
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

	VirtualFreeEx(m_Bot->GetInjectedProcessHandle(), pAddress, 0, MEM_RELEASE);

	return iBase;
}

Vector3 Client::GetTargetPosition()
{
	int32_t iTargetID = GetTarget();

	if (iTargetID >= 5000)
	{
		DWORD iBase = GetMobBase(iTargetID);

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
	return Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_MOB")));
}

void Client::SetAuthority(uint8_t iAuthority)
{
	WriteByte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_AUTHORITY")), iAuthority);
}

bool Client::IsBuffActive(int32_t iBuffType)
{ 
	Guard guard(m_mutexActiveBuffList);
	return m_mapActiveBuffList.find(iBuffType) != m_mapActiveBuffList.end(); 
};

bool Client::IsBlinking()
{
	return m_PlayerMySelf.bBlinking;
};

double Client::GetDistance(Vector3 v3Position)
{
	Vector3 v3MyPosition = GetPosition();
	return GetDistance(v3MyPosition.m_fX, v3MyPosition.m_fY, v3Position.m_fX, v3Position.m_fY);
}

double Client::GetDistance(Vector3 v3SourcePosition, Vector3 v3TargetPosition)
{
	return GetDistance(v3SourcePosition.m_fX, v3SourcePosition.m_fY, v3TargetPosition.m_fX, v3TargetPosition.m_fY);
}

double Client::GetDistance(float fX, float fY)
{
	Vector3 v3MyPosition = GetPosition();
	return GetDistance(v3MyPosition.m_fX, v3MyPosition.m_fY, fX, fY);
}

double Client::GetDistance(float fX1, float fY1, float fX2, float fY2)
{
	struct Point { double x; double y; };

	Point a{ fX1, fY1 };
	Point b{ fX2, fY2 };

	return std::hypot(a.x - b.x, a.y - b.y);
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

bool Client::GetNpcList(std::vector<TNpc>** vecNpcList)
{ 
	if (m_vecNpc.size() == 0)
		return false;

	*vecNpcList = &m_vecNpc;

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
		DWORD id = Read4Byte(Base + GetAddress("KO_OFF_ID"));
		DWORD proto_id = Read4Byte(Base + GetAddress("KO_OFF_PROTO_ID"));
		DWORD max_hp = Read4Byte(Base + GetAddress("KO_OFF_MAXHP"));
		DWORD hp = Read4Byte(Base + GetAddress("KO_OFF_HP"));
		DWORD state = Read4Byte(Base + GetAddress("KO_OFF_ACTION_STATE"));
		DWORD nation = Read4Byte(Base + GetAddress("KO_OFF_NATION"));
		Vector3 pos = Vector3(
			ReadFloat(Base + GetAddress("KO_OFF_X")),
			ReadFloat(Base + GetAddress("KO_OFF_Z")),
			ReadFloat(Base + GetAddress("KO_OFF_Y"))
		);
		double distance = GetDistance(pos);

		vecOutMobList.emplace_back(EntityInfo(Base, id, proto_id, max_hp, hp, state, nation, pos, distance));
	};

	while (Esi != Ebp && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - Tick).count() < 50)
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
			while (Read4Byte(Eax) != Fend && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - Tick).count() < 50)
			{
				Eax = Read4Byte(Eax);
			}

			Esi = Eax;
		}
		else
		{
			DWORD Ebx = Read4Byte(Esi + 0x4);

			while (Esi == Read4Byte(Ebx + 0x8) && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - Tick).count() < 50)
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
	int Fend = Read4Byte(Read4Byte(Ebp + 0x4) + 0x4);
	int Esi = Read4Byte(Ebp);
	auto Tick = std::chrono::steady_clock::now();

	auto AddEntityInfo = [&](DWORD Base)
	{
		DWORD id = Read4Byte(Base + GetAddress("KO_OFF_ID"));
		DWORD proto_id = Read4Byte(Base + GetAddress("KO_OFF_PROTO_ID"));
		DWORD max_hp = Read4Byte(Base + GetAddress("KO_OFF_MAXHP"));
		DWORD hp = Read4Byte(Base + GetAddress("KO_OFF_HP"));
		DWORD state = Read4Byte(Base + GetAddress("KO_OFF_ACTION_STATE"));
		DWORD nation = Read4Byte(Base + GetAddress("KO_OFF_NATION"));
		Vector3 pos = Vector3(
			ReadFloat(Base + GetAddress("KO_OFF_X")),
			ReadFloat(Base + GetAddress("KO_OFF_Z")),
			ReadFloat(Base + GetAddress("KO_OFF_Y"))
		);
		double distance = GetDistance(pos);

		vecOutPlayerList.emplace_back(EntityInfo(Base, id, proto_id, max_hp, hp, state, nation, pos, distance));
	};

	while (Esi != Ebp
		&& std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - Tick).count() < 50)
	{
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

void Client::ExecuteRemoteCode(BYTE* codes, size_t psize)
{
	m_Bot->ExecuteRemoteCode(codes, psize);
}

void Client::ExecuteRemoteCode(LPVOID pAddress)
{
	m_Bot->ExecuteRemoteCode(pAddress);
}