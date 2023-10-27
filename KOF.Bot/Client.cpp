#include "pch.h"
#include "Client.h"
#include "Bot.h"
#include "Memory.h"

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

	m_vecPartyMembers.clear();

	m_bLunarWarDressUp = false;

	m_vecRegionUserList.clear();

	m_fAttackDelta = 1.0f;
	m_fAttackTimeRecent = Bot::TimeGet();

	m_msLastGenieStartTime = std::chrono::milliseconds(0);

	m_iFlashCount = 0;

	m_bVipWarehouseInitialized = false;
	m_bVipWarehouseEnabled = false;
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

float Client::GetRadius(DWORD iBase)
{
	if (iBase == 0)
		iBase = Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR")));

	return ReadFloat(iBase + GetAddress(skCryptDec("KO_OFF_RADIUS")));
}

void Client::SetScale(DWORD iBase, float fX, float fZ, float fY)
{
	if (iBase == 0)
		iBase = Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR")));

	WriteFloat(iBase + GetAddress(skCryptDec("KO_OFF_SCALE_X")), fX);
	WriteFloat(iBase + GetAddress(skCryptDec("KO_OFF_SCALE_Z")), fZ);
	WriteFloat(iBase + GetAddress(skCryptDec("KO_OFF_SCALE_Y")), fY);
}

float Client::GetScaleX(DWORD iBase)
{
	if (iBase == 0)
		iBase = Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR")));

	return ReadFloat(iBase + GetAddress(skCryptDec("KO_OFF_SCALE_X")));
}

float Client::GetScaleZ(DWORD iBase)
{
	if (iBase == 0)
		iBase = Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR")));

	return ReadFloat(iBase + GetAddress(skCryptDec("KO_OFF_SCALE_Z")));
}

float Client::GetScaleY(DWORD iBase)
{
	if (iBase == 0)
		iBase = Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR")));

	return ReadFloat(iBase + GetAddress(skCryptDec("KO_OFF_SCALE_Y")));
}

bool Client::IsAttackable(DWORD iBase)
{
	DWORD iTargetID = GetID(iBase);

	if (iTargetID == -1)
		return false;

	DWORD iNation = GetNation(iBase);
	DWORD iMeNation = GetNation();

	if (iTargetID >= 5000)
	{
		if (iNation != 0 || iNation == GetNation())
			return false;
	}
	else
	{
		if (iNation == GetNation())
			return false;
	}

	DWORD iHp = GetHp(iBase);
	DWORD iMaxHp = GetMaxHp(iBase);
	DWORD iState = GetActionState(iBase);

	if ((iState == PSA_DYING || iState == PSA_DEATH) || (iMaxHp != 0 && iHp <= 0))
		return false;

	return true;
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

bool Client::IsStunned(DWORD iBase)
{
	return false;

	if (iBase == 0)
		iBase = Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR")));

	return ReadByte(iBase + GetAddress(skCryptDec("KO_OFF_STUN"))) != 0;
}

uint32_t Client::GetTargetBase()
{
	return GetEntityBase(GetTarget());
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
	int32_t iUndyHpPercent = (int32_t)std::ceil((MaxHp / 100) * 60);

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

Vector3 Client::GetMovePosition()
{
	return Vector3(GetGoX(), GetGoZ(), GetGoY());
}

DWORD Client::GetEntityBase(int32_t iTargetId)
{
	if(iTargetId == -1)
		return 0;

	if (iTargetId == GetID())
		return Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR")));

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

	HANDLE hProcess = m_Bot->GetInjectedProcessHandle();

	LPVOID pAddress = VirtualAllocEx(hProcess, nullptr, sizeof(DWORD), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (!pAddress)
	{
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
			return iBase;
		}
	}

	VirtualFreeEx(hProcess, pAddress, 0, MEM_RELEASE);

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

bool Client::IsBlinking(int32_t iTargetID)
{
	if (iTargetID == -1 || iTargetID == GetID())
	{
		return m_PlayerMySelf.bBlinking;
	}
	else
	{
		std::shared_lock<std::shared_mutex> lock(m_mutexPlayer);
		auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
			[&](const TPlayer& a) { return a.iID == iTargetID; });

		if (it != m_vecPlayer.end())
		{
			return it->bBlinking;
		}
	}

	return false;
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

		if (m_Bot->GetPlatformType() == PlatformType::USKO 
			|| m_Bot->GetPlatformType() == PlatformType::STKO)
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

TItemData Client::GetInventoryItem(uint32_t iItemID)
{
	TItemData pInventory;
	memset(&pInventory, 0, sizeof(pInventory));

	DWORD iInventoryBase = Read4Byte(Read4Byte(GetAddress("KO_PTR_DLG")) + GetAddress("KO_OFF_INVENTORY_BASE"));

	for (size_t i = SLOT_MAX; i < SLOT_MAX + HAVE_MAX; i++)
	{
		DWORD iItemBase = Read4Byte(iInventoryBase + (GetAddress("KO_OFF_INVENTORY_START") + (4 * i)));

		if (m_Bot->GetPlatformType() == PlatformType::USKO 
			|| m_Bot->GetPlatformType() == PlatformType::STKO)
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

		if (m_Bot->GetPlatformType() == PlatformType::USKO 
			|| m_Bot->GetPlatformType() == PlatformType::STKO)
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

		if (m_Bot->GetPlatformType() == PlatformType::USKO 
			|| m_Bot->GetPlatformType() == PlatformType::STKO)
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

TItemData Client::GetInventoryItemSlot(uint8_t iSlotPosition)
{
	TItemData pInventory;
	memset(&pInventory, 0, sizeof(pInventory));

	DWORD iInventoryBase = Read4Byte(Read4Byte(GetAddress("KO_PTR_DLG")) + GetAddress("KO_OFF_INVENTORY_BASE"));
	DWORD iItemBase = Read4Byte(iInventoryBase + (GetAddress("KO_OFF_INVENTORY_START") + (4 * iSlotPosition)));

	if (m_Bot->GetPlatformType() == PlatformType::USKO 
		|| m_Bot->GetPlatformType() == PlatformType::STKO)
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

bool Client::GetInventoryItemList(std::vector<TItemData>& vecItemList)
{
	DWORD iInventoryBase = Read4Byte(Read4Byte(GetAddress("KO_PTR_DLG")) + GetAddress("KO_OFF_INVENTORY_BASE"));

	for (int i = SLOT_MAX; i < SLOT_MAX + HAVE_MAX; i++)
	{
		TItemData pItem;
		memset(&pItem, 0, sizeof(pItem));

		pItem.iPos = i;

		DWORD iItemBase = Read4Byte(iInventoryBase + (GetAddress("KO_OFF_INVENTORY_START") + (4 * i)));

		if (m_Bot->GetPlatformType() == PlatformType::USKO 
			|| m_Bot->GetPlatformType() == PlatformType::STKO)
		{
			if (Read4Byte(Read4Byte(iItemBase + 0x60)) + Read4Byte(Read4Byte(iItemBase + 0x64)) != 0)
			{
				pItem.iBase = iItemBase;
				pItem.iItemID = Read4Byte(Read4Byte(iItemBase + 0x60)) + Read4Byte(Read4Byte(iItemBase + 0x64));
				pItem.iCount = (uint16_t)Read4Byte(iItemBase + 0x68);
				pItem.iDurability = (uint16_t)Read4Byte(iItemBase + 0x6C);
			}

			vecItemList.push_back(pItem);
		}
		else if (m_Bot->GetPlatformType() == PlatformType::CNKO)
		{
			if (Read4Byte(Read4Byte(iItemBase + 0x68)) + Read4Byte(Read4Byte(iItemBase + 0x6C)) != 0)
			{
				pItem.iBase = iItemBase;
				pItem.iItemID = Read4Byte(Read4Byte(iItemBase + 0x68)) + Read4Byte(Read4Byte(iItemBase + 0x6C));
				pItem.iCount = (uint16_t)Read4Byte(iItemBase + 0x70);
				pItem.iDurability = (uint16_t)Read4Byte(iItemBase + 0x74);
			}

			vecItemList.push_back(pItem);
		}
	}

	return true;
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
		//bool enemy = IsEnemy(Base);
		bool enemy = true;
		float radius = GetRadius(Base) * GetScaleZ(Base);

		vecOutMobList.emplace_back(EntityInfo(Base, id, proto_id, max_hp, hp, state, nation, pos, distance, enemy, radius));
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
		//bool enemy = IsEnemy(Base);
		bool enemy = true;
		float radius = GetRadius(Base) * GetScaleZ(Base);

		vecOutPlayerList.emplace_back(EntityInfo(Base, id, proto_id, max_hp, hp, state, nation, pos, distance, enemy, radius));
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
	HANDLE hProcess = m_Bot->GetInjectedProcessHandle();

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
			return iEnemyBase > 0;
		}
	}

	VirtualFreeEx(hProcess, pBaseAddress, 0, MEM_RELEASE);
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

	HANDLE hProcess = m_Bot->GetInjectedProcessHandle();

	ExecuteRemoteCode(hProcess, byCode, sizeof(byCode));
}

void Client::BasicAttack()
{
	DWORD iMobBase = GetEntityBase(GetTarget());

	if (iMobBase == 0)
		return;

	if (!IsAttackable(iMobBase))
		return;

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

	std::chrono::milliseconds msCurrentTime = duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()
	);

	if (m_msLastBasicAttackTime > std::chrono::milliseconds(0) 
		&& (msCurrentTime - m_msLastBasicAttackTime) < std::chrono::milliseconds(iAttackSpeedValue))
		return;

	Packet pkt = Packet(PIPE_BASIC_ATTACK);

	pkt << uint8_t(1);

	m_Bot->SendPipeServer(pkt);
	m_Bot->SendPipeServer(pkt);

	m_msLastBasicAttackTime = msCurrentTime;
}

void Client::BasicAttackWithPacket(DWORD iTargetBase, float fBasicAttackInterval)
{
	float fTime = Bot::TimeGet();
	float fAttackInterval = fBasicAttackInterval;

	Vector3 v3TargetPosition = GetTargetPosition();
	float fDistance = GetDistance(v3TargetPosition);

	float fTargetRadius = GetRadius(iTargetBase) * GetScaleZ(iTargetBase);
	float fMySelfRadius = GetRadius() * GetScaleZ();

	float fDistanceExceptRadius = fDistance - ((fMySelfRadius + fTargetRadius) / 2.0f);

	if (fDistanceExceptRadius > 8.0f)
		return;

	if (fTime > m_fAttackTimeRecent + fAttackInterval)
	{
		SendBasicAttackPacket(GetTarget(), fAttackInterval, fDistanceExceptRadius);
		m_fAttackTimeRecent = fTime;
	}
}

DWORD Client::GetSkillBase(uint32_t iSkillID)
{
	HANDLE hProcess = m_Bot->GetInjectedProcessHandle();

	LPVOID pBaseAddress = VirtualAllocEx(hProcess, 0, sizeof(DWORD), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (pBaseAddress == 0)
	{
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

	HANDLE hProcess = m_Bot->GetInjectedProcessHandle();

	ExecuteRemoteCode(hProcess, byCode, sizeof(byCode));
}

void Client::PushPhase(DWORD iAddress)
{
	BYTE byCode[] =
	{
		0x60,
		0xFF,0x35,0x0,0x0,0x0,0x0,
		0xBF,0,0,0,0,
		0xFF,0xD7,
		0x83,0xC4,0x04,
		0xB0,0x01,
		0x61,
		0xC2,0x04,0x0,
	};

	CopyBytes(byCode + 3, iAddress);

	DWORD dwPushPhase = GetAddress(skCryptDec("KO_PTR_PUSH_PHASE"));
	CopyBytes(byCode + 8, dwPushPhase);

	HANDLE hProcess = m_Bot->GetInjectedProcessHandle();

	ExecuteRemoteCode(hProcess, byCode, sizeof(byCode));
}

void Client::WriteLoginInformation(std::string szAccountId, std::string szPassword)
{
	DWORD dwCGameProcIntroLogin = Read4Byte(GetAddress(skCryptDec("KO_PTR_INTRO")));
	DWORD dwCUILoginIntro = Read4Byte(dwCGameProcIntroLogin + GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO")));

	DWORD dwCN3UIEditIdBase = Read4Byte(dwCUILoginIntro + GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO_ID")));
	DWORD dwCN3UIEditPwBase = Read4Byte(dwCUILoginIntro + GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO_PW")));

	WriteString(dwCN3UIEditIdBase + GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO_ID_INPUT")), szAccountId.c_str());
	Write4Byte(dwCN3UIEditIdBase + GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO_ID_INPUT_LENGTH")), szAccountId.size());

	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	WriteString(dwCN3UIEditPwBase + GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO_PW_INPUT")), szPassword.c_str());
	Write4Byte(dwCN3UIEditPwBase + GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO_PW_INPUT_LENGTH")), szPassword.size());
}

void Client::ConnectLoginServer(bool bDisconnect)
{
	Packet pkt = Packet(PIPE_LOGIN);
	pkt << uint8_t(1) << uint8_t(bDisconnect);
	m_Bot->SendPipeServer(pkt);
}

void Client::LoadServerList()
{
	Packet pkt = Packet(PIPE_LOAD_SERVER_LIST);
	pkt << uint8_t(1);
	m_Bot->SendPipeServer(pkt);
}

void Client::SelectServer(uint8_t iIndex)
{
	Packet pkt = Packet(PIPE_SELECT_SERVER);
	pkt << uint8_t(iIndex);
	m_Bot->SendPipeServer(pkt);
}

void Client::ShowChannel()
{
	Packet pkt = Packet(PIPE_SHOW_CHANNEL);
	pkt << uint8_t(1);
	m_Bot->SendPipeServer(pkt);
}

void Client::SelectChannel(uint8_t iIndex)
{
	Packet pkt = Packet(PIPE_SELECT_CHANNEL);
	pkt << uint8_t(iIndex);
	m_Bot->SendPipeServer(pkt);
}

void Client::ConnectServer()
{
	Packet pkt = Packet(PIPE_CONNECT_SERVER);
	pkt << uint8_t(1);
	m_Bot->SendPipeServer(pkt);
}

void Client::SetSaveCPUSleepTime(int32_t iValue)
{
	Packet pkt = Packet(PIPE_SAVE_CPU);
	pkt << int32_t(iValue);
	m_Bot->SendPipeServer(pkt);
}

void Client::ConnectGameServer(BYTE byServerId)
{
	HANDLE hProcess = m_Bot->GetInjectedProcessHandle();

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
	HANDLE hProcess = m_Bot->GetInjectedProcessHandle();

	LPVOID pPacketAddress = VirtualAllocEx(hProcess, nullptr, vecBuffer.size(), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (pPacketAddress == nullptr)
	{
		return;
	}

	WriteProcessMemory(hProcess, pPacketAddress, vecBuffer.contents(), vecBuffer.size(), 0);

	BYTE byCode[] =
	{
		0x60,
		0x8B, 0x0D, 0x0, 0x0, 0x0, 0x0,
		0x68, 0x0, 0x0, 0x0, 0x0,
		0x68, 0x0, 0x0, 0x0, 0x0,
		0xBF, 0x0, 0x0, 0x0, 0x0,
		0xFF, 0xD7,
		0x61,
		0xC3,
	};

	DWORD dwPtrPkt = GetAddress(skCryptDec("KO_PTR_PKT"));

	CopyBytes(byCode + 3, dwPtrPkt);

	size_t dwPacketSize = vecBuffer.size();

	CopyBytes(byCode + 8, dwPacketSize);
	CopyBytes(byCode + 13, pPacketAddress);

	DWORD dwPtrSndFnc = GetAddress(skCryptDec("KO_SND_FNC"));
	CopyBytes(byCode + 18, dwPtrSndFnc);

	ExecuteRemoteCode(hProcess, byCode, sizeof(byCode));
	VirtualFreeEx(hProcess, pPacketAddress, 0, MEM_RELEASE);
}

void Client::UseSkillWithPacket(TABLE_UPC_SKILL pSkillData, int32_t iTargetID, bool bWaitCastTime)
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
			if (m_PlayerMySelf.iMoveType != 0)
			{
				SendMovePacket(v3MyPosition, v3MyPosition, 0, 0);
				//StopMove();
			}

			SendStartSkillCastingAtTargetPacket(pSkillData, iTargetID);

			if (bWaitCastTime || (pSkillData.dw1stTableType == 3 || pSkillData.dw1stTableType == 4))
			{
				std::this_thread::sleep_for(std::chrono::milliseconds((pSkillData.iReCastTime * 100)));
			}
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
			if (m_PlayerMySelf.iMoveType != 0)
			{
				SendMovePacket(v3MyPosition, v3MyPosition, 0, 0);
				//StopMove();
			}

			SendStartSkillCastingAtPosPacket(pSkillData, v3TargetPosition);

			if (bWaitCastTime || (pSkillData.dw1stTableType == 3 || pSkillData.dw1stTableType == 4))
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(pSkillData.iReCastTime * 100));
			}
		}

		if (pSkillData.iFlyingFX != 0)
			SendStartFlyingAtTargetPacket(pSkillData, -1, v3TargetPosition);

		SendStartSkillMagicAtPosPacket(pSkillData, v3TargetPosition);
		SendStartMagicAtTarget(pSkillData, -1, v3TargetPosition);
	}
	break;
	}

	Client::SetSkillUseTime(pSkillData.iID, duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()
	));
}

bool Client::UseSkill(TABLE_UPC_SKILL pSkillData, int32_t iTargetID, int32_t iPriority, bool bWaitCastTime)
{
	Vector3 v3MyPosition = GetPosition();

	if (pSkillData.iReCastTime != 0)
	{
		/*if (GetActionState() == PSA_SPELLMAGIC)
			return false;*/

		bool bSpeedHack = GetUserConfiguration()->GetBool(skCryptDec("Feature"), skCryptDec("SpeedHack"), false);

		if (bSpeedHack
			&& m_PlayerMySelf.iMoveType != 0)
		{
			SendMovePacket(v3MyPosition, v3MyPosition, 0, 0);
			//StopMove();
		}
	}

	Packet pkt = Packet(PIPE_USE_SKILL);

	pkt << iTargetID << pSkillData.iID << iPriority << uint8_t(0) << uint8_t(0);

	m_Bot->SendPipeServer(pkt);

	if (pSkillData.iReCastTime != 0)
	{
		if (bWaitCastTime || (pSkillData.dw1stTableType == 3 || pSkillData.dw1stTableType == 4))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(pSkillData.iReCastTime * 100));
		}
	}

	//SetSkillUseTime(pSkillData.iID,
	//	duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
	//);

	return true;
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
		<< uint32_t(v3TargetPosition.m_fX)
		<< int32_t(v3TargetPosition.m_fZ)
		<< uint32_t(v3TargetPosition.m_fY)
		<< uint32_t(0) << uint32_t(0) << uint32_t(0) << uint32_t(0)
		<< int16_t(pSkillData.iReCastTime)
		<< uint32_t(0);

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
		<< uint32_t(v3TargetPosition.m_fX)
		<< int32_t(v3TargetPosition.m_fZ)
		<< uint32_t(v3TargetPosition.m_fY)
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
			<< uint32_t(v3TargetPosition.m_fX)
			<< int32_t(v3TargetPosition.m_fZ)
			<< uint32_t(v3TargetPosition.m_fY);
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
		<< uint32_t(v3TargetPosition.m_fX)
		<< int32_t(v3TargetPosition.m_fZ)
		<< uint32_t(v3TargetPosition.m_fY)
		<< uint32_t(0) << uint32_t(0) << uint32_t(0)
		<< uint32_t(0) << uint32_t(0) << int16_t(0);

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
		<< uint32_t(v3TargetPosition.m_fX)
		<< int32_t(v3TargetPosition.m_fZ)
		<< uint32_t(v3TargetPosition.m_fY)
		<< int32_t(-101)
		<< arrowIndex
		<< uint32_t(0) << uint32_t(0) << int16_t(0) << int16_t(0) << int16_t(0) << int16_t(0);

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
		<< uint16_t((fInterval * 100))
		<< int16_t((fDistance))
		<< uint8_t(0) << uint8_t(0);

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

void Client::SetTarget(uint32_t iTarget)
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
	CopyBytes(byCode + 9, iTarget); // Target Base

	DWORD iSelectMob = GetAddress(skCryptDec("KO_SELECT_MOB"));
	CopyBytes(byCode + 15, iSelectMob);

	HANDLE hProcess = m_Bot->GetInjectedProcessHandle();

	ExecuteRemoteCode(hProcess, byCode, sizeof(byCode));
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
			UseSkill(pSkillData->second, GetID(), 2);
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

void Client::SendSelectMessage(uint8_t iMenuIndex, std::string szLuaName, bool bAccept)
{
	Packet pkt = Packet(WIZ_SELECT_MSG);

	pkt.SByte();

	pkt
		<< uint8_t(iMenuIndex)
		<< szLuaName;

	if (bAccept)
	{
		pkt << bAccept;
	}

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
				TItemData pInventory = GetInventoryItemSlot(i);

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

			TItemData pInventoryItem = GetInventoryItem(iSupplyItemId);

			if (pInventoryItem.iItemID == 0)
				return true;

			if (pInventoryItem.iItemID != 0)
			{
				if (pInventoryItem.iCount <= 5 && pInventoryItem.iCount < iSupplyItemCount)
					return true;
			}
		}
	}

	return false;
}

bool Client::IsNeedSell()
{
	bool bAutoSellSlotRange = GetUserConfiguration()->GetBool(skCryptDec("Supply"), skCryptDec("AutoSellSlotRange"), false);

	if (bAutoSellSlotRange)
	{
		int iAutoSellSlotRangeStart = GetUserConfiguration()->GetInt(skCryptDec("Supply"), skCryptDec("AutoSellSlotRangeStart"), 1);
		int iAutoSellSlotRangeEnd = GetUserConfiguration()->GetInt(skCryptDec("Supply"), skCryptDec("AutoSellSlotRangeEnd"), 14);

		for (int i = iAutoSellSlotRangeStart; i <= iAutoSellSlotRangeEnd; i++)
		{
			int iPosition = 14 + (i - 1);

			TItemData pInventory = GetInventoryItemSlot((uint8_t)iPosition);

			if (pInventory.iItemID == 0)
				return false;
		}
	}

	bool bAutoSellByFlag = GetUserConfiguration()->GetBool(skCryptDec("Supply"), skCryptDec("AutoSellByFlag"), false);

	if (bAutoSellByFlag)
	{
		for (size_t i = SLOT_MAX; i < SLOT_MAX + HAVE_MAX; i++)
		{
			TItemData pInventory = GetInventoryItemSlot((uint8_t)i);

			if (pInventory.iItemID == 0)
				return false;
		}
	}

	bool bAutoSellVipSlotRange = GetUserConfiguration()->GetBool(skCryptDec("Supply"), skCryptDec("AutoSellVipSlotRange"), false);

	if (bAutoSellVipSlotRange)
	{
		if (!m_bVipWarehouseInitialized)
			return false;

		if (m_bVipWarehouseEnabled)
		{
			int iAutoSellVipSlotRangeStart = GetUserConfiguration()->GetInt(skCryptDec("Supply"), skCryptDec("AutoSellVipSlotRangeStart"), 1);
			int iAutoSellVipSlotRangeEnd = GetUserConfiguration()->GetInt(skCryptDec("Supply"), skCryptDec("AutoSellVipSlotRangeEnd"), 48);

			for (int i = iAutoSellVipSlotRangeStart; i <= iAutoSellVipSlotRangeEnd; i++)
			{
				int iPosition = (i - 1);

				TItemData* pItem = &m_PlayerMySelf.tVipWarehouse[iPosition];

				if (pItem->iItemID == 0)
					return false;
			}
		}
	}

	return true;
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

void Client::UpdateSkillSuccessRate(bool bDisableCasting)
{
	std::shared_lock<std::shared_mutex> lock(m_mutexAvailableSkill);
	for (auto& e : m_vecAvailableSkill)
	{
		DWORD iSkillBase = GetSkillBase(e.iID);

		if (iSkillBase == 0 || Read4Byte(iSkillBase + 0xA8) == 0)
			continue;

		if (bDisableCasting)
			Write4Byte(iSkillBase + 0xA8, 100);
		else
			Write4Byte(iSkillBase + 0xA8, e.iPercentSuccess);
	}
}

void Client::SendWarehouseOpen(uint32_t iNpcID)
{
	Packet pkt = Packet(WIZ_WAREHOUSE);

	pkt
		<< uint8_t(1)
		<< iNpcID;

	SendPacket(pkt);
}

void Client::SendWarehouseGetIn(int32_t iNpcID, uint32_t iItemID, uint8_t iPage, uint8_t iCurrentPosition, uint8_t iTargetPosition, uint32_t iCount)
{
	Packet pkt = Packet(WIZ_WAREHOUSE);

	pkt
		<< uint8_t(2)
		<< int32_t(iNpcID)
		<< uint32_t(iItemID)
		<< uint8_t(iPage)
		<< uint8_t(iCurrentPosition)
		<< uint8_t(iTargetPosition)
		<< uint32_t(iCount);

	SendPacket(pkt);
}

void Client::VipWarehouseGetIn(int32_t iTargetPosition)
{
	HANDLE hProcess = m_Bot->GetInjectedProcessHandle();

	BYTE byAreaPositionCode[] =
	{
		0x60,
		0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00,
		0x8B, 0x89, 0x00, 0x00, 0x00, 0x00,
		0x68, 0x00, 0x00, 0x00, 0x00,
		0x68, 0x00, 0x00, 0x00, 0x00,
		0xBF, 0x00, 0x00, 0x00, 0x00,
		0xFF, 0xD7,
		0xA3, 0x00, 0x00, 0x00, 0x00,
		0x61,
		0xC3,
	};

	DWORD iDlg = GetAddress(skCryptDec("KO_PTR_DLG"));
	memcpy(byAreaPositionCode + 3, &iDlg, sizeof(iDlg));

	DWORD iUIVipWarehouse = GetAddress(skCryptDec("KO_OFF_VIP"));
	memcpy(byAreaPositionCode + 9, &iUIVipWarehouse, sizeof(iUIVipWarehouse));

	memcpy(byAreaPositionCode + 14, &iTargetPosition, sizeof(iTargetPosition));

	DWORD iAreaType = 0x00000003;
	memcpy(byAreaPositionCode + 19, &iAreaType, sizeof(iAreaType));

	DWORD iAreaPositionCallAddress = GetAddress(skCryptDec("KO_PTR_AREA_POSITION"));
	memcpy(byAreaPositionCode + 24, &iAreaPositionCallAddress, sizeof(iAreaPositionCallAddress));

	LPVOID pCallReturnAddress = VirtualAllocEx(hProcess, nullptr, sizeof(DWORD), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (!pCallReturnAddress)
		return;

	memcpy(byAreaPositionCode + 31, &pCallReturnAddress, sizeof(pCallReturnAddress));

	DWORD iAreaPositionBase = 0;
	DWORD iAreaPositionX = 0;
	DWORD iAreaPositionY = 0;
	SIZE_T bytesRead = 0;

	if (ExecuteRemoteCode(hProcess, byAreaPositionCode, sizeof(byAreaPositionCode)))
	{
		if (ReadProcessMemory(hProcess, pCallReturnAddress, &iAreaPositionBase, sizeof(iAreaPositionBase), &bytesRead) && bytesRead == sizeof(iAreaPositionBase))
		{
			iAreaPositionX = Read4Byte(iAreaPositionBase + 0xC8) + 1;
			iAreaPositionY = Read4Byte(iAreaPositionBase + 0xC4) + 1;
		}
	}

	VirtualFreeEx(hProcess, pCallReturnAddress, 0, MEM_RELEASE);

	if (iAreaPositionBase == 0)
		return;

	BYTE byVipWarehouseGetInCode[] =
	{
		0x60,
		0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00,
		0x8B, 0x89, 0x00, 0x00, 0x00, 0x00,
		0x68, 0x00, 0x00, 0x00, 0x00,
		0x68, 0x00, 0x00, 0x00, 0x00,
		0xFF, 0x35, 0x00, 0x00, 0x00, 0x00,
		0xBF, 0x00, 0x00, 0x00, 0x00,
		0xFF, 0xD7,
		0x61,
		0xC3,
	};

	memcpy(byVipWarehouseGetInCode + 3, &iDlg, sizeof(iDlg));
	memcpy(byVipWarehouseGetInCode + 9, &iUIVipWarehouse, sizeof(iUIVipWarehouse));
	memcpy(byVipWarehouseGetInCode + 14, &iAreaPositionX, sizeof(iAreaPositionX));
	memcpy(byVipWarehouseGetInCode + 19, &iAreaPositionY, sizeof(iAreaPositionY));

	DWORD iItemBaseAddress = GetAddress(skCryptDec("KO_PTR_SELECTED_ITEM_BASE"));
	memcpy(byVipWarehouseGetInCode + 25, &iItemBaseAddress, sizeof(iItemBaseAddress));

	DWORD iVipWarehouseGetInCallAddress = GetAddress(skCryptDec("KO_PTR_VIP_GET_IN"));
	memcpy(byVipWarehouseGetInCode + 30, &iVipWarehouseGetInCallAddress, sizeof(iVipWarehouseGetInCallAddress));

	ExecuteRemoteCode(hProcess, byVipWarehouseGetInCode, sizeof(byVipWarehouseGetInCode));
}

void Client::VipWarehouseGetOut(int32_t iTargetPosition)
{
	HANDLE hProcess = m_Bot->GetInjectedProcessHandle();

	BYTE byAreaPositionCode[] =
	{
		0x60,
		0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00,
		0x8B, 0x89, 0x00, 0x00, 0x00, 0x00,
		0x68, 0x00, 0x00, 0x00, 0x00,
		0x68, 0x00, 0x00, 0x00, 0x00,
		0xBF, 0x00, 0x00, 0x00, 0x00,
		0xFF, 0xD7,
		0xA3, 0x00, 0x00, 0x00, 0x00,
		0x61,
		0xC3,
	};

	DWORD iDlg = GetAddress(skCryptDec("KO_PTR_DLG"));
	memcpy(byAreaPositionCode + 3, &iDlg, sizeof(iDlg));

	DWORD iUIVipWarehouse = GetAddress(skCryptDec("KO_OFF_VIP"));
	memcpy(byAreaPositionCode + 9, &iUIVipWarehouse, sizeof(iUIVipWarehouse));

	memcpy(byAreaPositionCode + 14, &iTargetPosition, sizeof(iTargetPosition));

	DWORD iAreaType = 0x0000000B;
	memcpy(byAreaPositionCode + 19, &iAreaType, sizeof(iAreaType));

	DWORD iAreaPositionCallAddress = GetAddress(skCryptDec("KO_PTR_AREA_POSITION"));
	memcpy(byAreaPositionCode + 24, &iAreaPositionCallAddress, sizeof(iAreaPositionCallAddress));

	LPVOID pCallReturnAddress = VirtualAllocEx(hProcess, nullptr, sizeof(DWORD), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (!pCallReturnAddress)
		return;

	memcpy(byAreaPositionCode + 31, &pCallReturnAddress, sizeof(pCallReturnAddress));

	DWORD iAreaPositionBase = 0;
	DWORD iAreaPositionX = 0;
	DWORD iAreaPositionY = 0;
	SIZE_T bytesRead = 0;

	if (ExecuteRemoteCode(hProcess, byAreaPositionCode, sizeof(byAreaPositionCode)))
	{
		if (ReadProcessMemory(hProcess, pCallReturnAddress, &iAreaPositionBase, sizeof(iAreaPositionBase), &bytesRead) && bytesRead == sizeof(iAreaPositionBase))
		{
			iAreaPositionX = Read4Byte(iAreaPositionBase + 0xC8) + 1;
			iAreaPositionY = Read4Byte(iAreaPositionBase + 0xC4) + 1;
		}
	}

	VirtualFreeEx(hProcess, pCallReturnAddress, 0, MEM_RELEASE);

	if (iAreaPositionBase == 0)
		return;

	BYTE byVipWarehouseGetOutCode[] =
	{
		0x60,
		0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00,
		0x8B, 0x89, 0x00, 0x00, 0x00, 0x00,
		0x68, 0x00, 0x00, 0x00, 0x00,
		0x68, 0x00, 0x00, 0x00, 0x00,
		0xFF, 0x35, 0x00, 0x00, 0x00, 0x00,
		0xBF, 0x00, 0x00, 0x00, 0x00,
		0xFF, 0xD7,
		0x61,
		0xC3,
	};

	memcpy(byVipWarehouseGetOutCode + 3, &iDlg, sizeof(iDlg));
	memcpy(byVipWarehouseGetOutCode + 9, &iUIVipWarehouse, sizeof(iUIVipWarehouse));
	memcpy(byVipWarehouseGetOutCode + 14, &iAreaPositionX, sizeof(iAreaPositionX));
	memcpy(byVipWarehouseGetOutCode + 19, &iAreaPositionY, sizeof(iAreaPositionY));

	DWORD iItemBaseAddress = GetAddress(skCryptDec("KO_PTR_SELECTED_ITEM_BASE"));
	memcpy(byVipWarehouseGetOutCode + 25, &iItemBaseAddress, sizeof(iItemBaseAddress));

	DWORD iVipWarehouseGetOutCallAddress = GetAddress(skCryptDec("KO_PTR_VIP_GET_OUT"));
	memcpy(byVipWarehouseGetOutCode + 30, &iVipWarehouseGetOutCallAddress, sizeof(iVipWarehouseGetOutCallAddress));

	ExecuteRemoteCode(hProcess, byVipWarehouseGetOutCode, sizeof(byVipWarehouseGetOutCode));
}

void Client::CountableDialogChangeCount(uint32_t iCount)
{
	BYTE byCode[] =
	{
		0x60,
		0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00,
		0x68, 0x00, 0x00, 0x00, 0x00,
		0xBF, 0x00, 0x00, 0x00, 0x00,
		0xFF, 0xD7,
		0x61,
		0xC3,
	};

	DWORD iCountableItemDLG = GetAddress(skCryptDec("KO_PTR_COUNTABLE_DLG"));
	CopyBytes(byCode + 3, iCountableItemDLG);
	CopyBytes(byCode + 8, iCount);

	DWORD iCountableItemCallAddress = GetAddress(skCryptDec("KO_PTR_COUNTABLE_CHANGE"));
	CopyBytes(byCode + 13, iCountableItemCallAddress);

	HANDLE hProcess = m_Bot->GetInjectedProcessHandle();

	ExecuteRemoteCode(hProcess, byCode, sizeof(byCode));
}

void Client::AcceptCountableDialog()
{
	BYTE byCode[] =
	{
		0x60,
		0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00,
		0x8B, 0x89, 0x00, 0x00, 0x00, 0x00,
		0xBF, 0x00, 0x00, 0x00, 0x00,
		0xFF, 0xD7,
		0x61,
		0xC3,
	};

	DWORD iDLG = GetAddress(skCryptDec("KO_PTR_DLG"));
	CopyBytes(byCode + 3, iDLG);

	DWORD iCUIVipWareHouse = GetAddress(skCryptDec("KO_OFF_VIP"));
	CopyBytes(byCode + 9, iCUIVipWareHouse);

	DWORD iCallAddress = GetAddress(skCryptDec("KO_PTR_COUNTABLE_ACCEPT"));
	CopyBytes(byCode + 14, iCallAddress);

	HANDLE hProcess = m_Bot->GetInjectedProcessHandle();

	ExecuteRemoteCode(hProcess, byCode, sizeof(byCode));
}

void Client::SendWarehouseGetOut(int32_t iNpcID, uint32_t iItemID, uint8_t iPage, uint8_t iCurrentPosition, uint8_t iTargetPosition, uint32_t iCount)
{
	Packet pkt = Packet(WIZ_WAREHOUSE);

	pkt 
		<< uint8_t(3)
		<< int32_t(iNpcID)
		<< uint32_t(iItemID)
		<< uint8_t(iPage)
		<< uint8_t(iCurrentPosition)
		<< uint8_t(iTargetPosition)
		<< uint32_t(iCount);

	SendPacket(pkt);
}

bool Client::GetVipWarehouseItemList(std::vector<TItemData>& vecItemList)
{
	DWORD iVipWarehouseBase = Read4Byte(Read4Byte(GetAddress("KO_PTR_DLG")) + GetAddress(skCryptDec("KO_OFF_VIP")));

	for (size_t i = 0; i < VIP_HAVE_MAX; i++)
	{
		TItemData pItem;
		memset(&pItem, 0, sizeof(pItem));

		pItem.iPos = i;

		DWORD iItemBase = Read4Byte(iVipWarehouseBase + (GetAddress(skCryptDec("KO_OFF_VIP_START")) + (4 * i)));

		if (m_Bot->GetPlatformType() == PlatformType::USKO 
			|| m_Bot->GetPlatformType() == PlatformType::STKO)
		{
			if (Read4Byte(Read4Byte(iItemBase + 0x60)) + Read4Byte(Read4Byte(iItemBase + 0x64)) != 0)
			{
				pItem.iBase = iItemBase;
				pItem.iItemID = Read4Byte(Read4Byte(iItemBase + 0x60)) + Read4Byte(Read4Byte(iItemBase + 0x64));
				pItem.iCount = (uint16_t)Read4Byte(iItemBase + 0x68);
				pItem.iDurability = (uint16_t)Read4Byte(iItemBase + 0x6C);
			}

			vecItemList.push_back(pItem);
		}
		else if (m_Bot->GetPlatformType() == PlatformType::CNKO)
		{
			if (Read4Byte(Read4Byte(iItemBase + 0x68)) + Read4Byte(Read4Byte(iItemBase + 0x6C)) != 0)
			{
				pItem.iBase = iItemBase;
				pItem.iItemID = Read4Byte(Read4Byte(iItemBase + 0x68)) + Read4Byte(Read4Byte(iItemBase + 0x6C));
				pItem.iCount = (uint16_t)Read4Byte(iItemBase + 0x70);
				pItem.iDurability = (uint16_t)Read4Byte(iItemBase + 0x74);
			}

			vecItemList.push_back(pItem);
		}
	}

	return vecItemList.size() > 0;
}

bool Client::GetVipWarehouseInventoryItemList(std::vector<TItemData>& vecItemList)
{
	DWORD iVipWarehouseBase = Read4Byte(Read4Byte(GetAddress("KO_PTR_DLG")) + GetAddress(skCryptDec("KO_OFF_VIP")));

	for (size_t i = 0; i < HAVE_MAX; i++)
	{
		TItemData pItem;
		memset(&pItem, 0, sizeof(pItem));

		pItem.iPos = i;

		DWORD iItemBase = Read4Byte(iVipWarehouseBase + (GetAddress(skCryptDec("KO_OFF_VIP_INVENTORY_START")) + (4 * i)));

		if (m_Bot->GetPlatformType() == PlatformType::USKO 
			|| m_Bot->GetPlatformType() == PlatformType::STKO)
		{
			if (Read4Byte(Read4Byte(iItemBase + 0x60)) + Read4Byte(Read4Byte(iItemBase + 0x64)) != 0)
			{
				pItem.iBase = iItemBase;
				pItem.iItemID = Read4Byte(Read4Byte(iItemBase + 0x60)) + Read4Byte(Read4Byte(iItemBase + 0x64));
				pItem.iCount = (uint16_t)Read4Byte(iItemBase + 0x68);
				pItem.iDurability = (uint16_t)Read4Byte(iItemBase + 0x6C);
			}

			vecItemList.push_back(pItem);
		}
		else if (m_Bot->GetPlatformType() == PlatformType::CNKO)
		{
			if (Read4Byte(Read4Byte(iItemBase + 0x68)) + Read4Byte(Read4Byte(iItemBase + 0x6C)) != 0)
			{
				pItem.iBase = iItemBase;
				pItem.iItemID = Read4Byte(Read4Byte(iItemBase + 0x68)) + Read4Byte(Read4Byte(iItemBase + 0x6C));
				pItem.iCount = (uint16_t)Read4Byte(iItemBase + 0x70);
				pItem.iDurability = (uint16_t)Read4Byte(iItemBase + 0x74);
			}

			vecItemList.push_back(pItem);
		}
	}

	return vecItemList.size() > 0;
}

int32_t Client::GetWarehouseAvailableSlot(uint32_t iItemID, uint8_t iContable)
{
	int iAvailableSlot = -1;

	for (size_t i = 0; i < WAREHOUSE_MAX; i++)
	{
		TItemData* pItem = &m_PlayerMySelf.tWarehouse[i];

		if (iAvailableSlot == -1 
			&& pItem->iItemID == 0)
		{
			iAvailableSlot = pItem->iPos;
		}

		if (pItem->iItemID == iItemID && iContable)
		{
			iAvailableSlot = pItem->iPos;
		}
	}

	return iAvailableSlot;
}

void Client::SendUseGeniePotion(uint32_t iItemID)
{
	Packet pkt = Packet(WIZ_GENIE);

	pkt
		<< uint8_t(1)
		<< uint8_t(1)
		<< uint32_t(iItemID);

	SendPacket(pkt);
}

void Client::SendStartGenie()
{
	Packet pkt = Packet(WIZ_GENIE);

	pkt
		<< uint8_t(1)
		<< uint8_t(4);

	SendPacket(pkt);
}

void Client::SendStopGenie()
{
	Packet pkt = Packet(WIZ_GENIE);

	pkt
		<< uint8_t(1)
		<< uint8_t(5);

	SendPacket(pkt);
}

void Client::SendPartyCreate(std::string szName)
{
	Packet pkt = Packet(WIZ_PARTY);

	pkt
		<< uint8_t(1)
		<< szName
		<< uint8_t(0);

	SendPacket(pkt);
}

void Client::SendPartyInsert(std::string szName)
{
	Packet pkt = Packet(WIZ_PARTY);

	pkt
		<< uint8_t(3)
		<< szName
		<< uint8_t(0);

	SendPacket(pkt);
}

void Client::PatchObjectCollision(bool bEnable)
{
	HANDLE hProcess = m_Bot->GetInjectedProcessHandle();

	if (bEnable)
	{
		BYTE byPatch[] =
		{
			0x75
		};

		WriteProcessMemory(hProcess, (LPVOID*)GetAddress("KO_OBJECT_COLLISION_CHECK"), byPatch, sizeof(byPatch), 0);
	}
	else
	{
		BYTE byPatch[] =
		{
			0x74
		};

		WriteProcessMemory(hProcess, (LPVOID*)GetAddress("KO_OBJECT_COLLISION_CHECK"), byPatch, sizeof(byPatch), 0);
	}
}

void Client::SendRearrangeInventory()
{
	Packet pkt = Packet(WIZ_ITEM_MOVE);

	pkt << uint8_t(2);

	SendPacket(pkt);
}

void Client::SendQuestCompleted(uint32_t iQuestID)
{
	Packet pkt = Packet(WIZ_QUEST);

	pkt
		<< uint8_t(4)
		<< int32_t(iQuestID);

	SendPacket(pkt);
}

void Client::SendQuestUnknown1(uint32_t iQuestID)
{
	Packet pkt = Packet(WIZ_QUEST);

	pkt
		<< uint8_t(6)
		<< int32_t(iQuestID);

	SendPacket(pkt);
}

void Client::SendQuestUnknown2(uint32_t iQuestID)
{
	Packet pkt = Packet(WIZ_QUEST);

	pkt
		<< uint8_t(7)
		<< int32_t(iQuestID);

	SendPacket(pkt);
}

void Client::SendOpenVipWarehouse(uint32_t iItemID)
{
	Packet pkt = Packet(WIZ_VIPWAREHOUSE);

	pkt << uint8_t(1) << iItemID;

	SendPacket(pkt);
}

void Client::SendVipWarehouseGetIn(int32_t iNpcID, uint32_t iItemID, uint8_t iPage, uint8_t iCurrentPosition, uint8_t iTargetPosition, uint16_t iCount)
{
	Packet pkt = Packet(WIZ_VIPWAREHOUSE);

	pkt
		<< uint8_t(2)
		<< int32_t(iNpcID)
		<< uint32_t(iItemID)
		<< uint8_t(iPage)
		<< uint8_t(iCurrentPosition)
		<< uint8_t(iTargetPosition)
		<< uint16_t(iCount);

	SendPacket(pkt);
}

bool Client::IsVipWarehouseFull()
{
	DWORD iVipWarehouseBase = Read4Byte(Read4Byte(GetAddress("KO_PTR_DLG")) + GetAddress(skCryptDec("KO_OFF_VIP")));

	for (size_t i = 0; i < VIP_HAVE_MAX; i++)
	{
		DWORD iItemBase = Read4Byte(iVipWarehouseBase + (GetAddress(skCryptDec("KO_OFF_VIP_START")) + (4 * i)));

		if (m_Bot->GetPlatformType() == PlatformType::USKO
			|| m_Bot->GetPlatformType() == PlatformType::STKO)
		{
			if (Read4Byte(Read4Byte(iItemBase + 0x60)) + Read4Byte(Read4Byte(iItemBase + 0x64)) == 0)
				return false;
		}
		else if (m_Bot->GetPlatformType() == PlatformType::CNKO)
		{
			if (Read4Byte(Read4Byte(iItemBase + 0x68)) + Read4Byte(Read4Byte(iItemBase + 0x6C)) == 0)
				return false;
		}
	}

	return true;
}

void Client::OpenVipWarehouse()
{
	m_bVipWarehouseLoaded = false;

	Write4Byte(Read4Byte(Read4Byte(GetAddress("KO_PTR_DLG")) + GetAddress(skCryptDec("KO_OFF_VIP"))) + GetAddress(skCryptDec("KO_OFF_VIP_OPEN")), 1);

	BYTE byCode[] =
	{
		0x60,
		0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00,
		0x8B, 0x89, 0x00, 0x00, 0x00, 0x00,
		0xBF, 0x00, 0x00, 0x00, 0x00,
		0xFF, 0xD7,
		0x61,
		0xC3,
	};

	DWORD iDLG = GetAddress(skCryptDec("KO_PTR_DLG"));
	CopyBytes(byCode + 3, iDLG);

	DWORD iCUIVipWareHouse = GetAddress(skCryptDec("KO_OFF_VIP"));
	CopyBytes(byCode + 9, iCUIVipWareHouse);

	DWORD iCallAddress = GetAddress(skCryptDec("KO_PTR_VIP_OPEN"));
	CopyBytes(byCode + 14, iCallAddress);

	HANDLE hProcess = m_Bot->GetInjectedProcessHandle();

	ExecuteRemoteCode(hProcess, byCode, sizeof(byCode));
}

void Client::CloseVipWarehouse()
{
	m_bVipWarehouseLoaded = false;

	Write4Byte(Read4Byte(Read4Byte(GetAddress("KO_PTR_DLG")) + GetAddress(skCryptDec("KO_OFF_VIP"))) + GetAddress(skCryptDec("KO_OFF_VIP_OPEN")), 0);

	BYTE byCode[] =
	{
		0x60,
		0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00,
		0x8B, 0x89, 0x00, 0x00, 0x00, 0x00,
		0xBF, 0x00, 0x00, 0x00, 0x00,
		0xFF, 0xD7,
		0x61,
		0xC3,
	};

	DWORD iDLG = GetAddress(skCryptDec("KO_PTR_DLG"));
	CopyBytes(byCode + 3, iDLG);

	DWORD iCUIVipWareHouse = GetAddress(skCryptDec("KO_OFF_VIP"));
	CopyBytes(byCode + 9, iCUIVipWareHouse);

	DWORD iCallAddress = GetAddress(skCryptDec("KO_PTR_VIP_CLOSE"));
	CopyBytes(byCode + 14, iCallAddress);

	HANDLE hProcess = m_Bot->GetInjectedProcessHandle();

	ExecuteRemoteCode(hProcess, byCode, sizeof(byCode));
}

void Client::SendRegenePacket()
{
	Packet pkt = Packet(WIZ_REGENE);

	pkt
		<< uint8_t(1);

	SendPacket(pkt);
}

void Client::SendOTPPacket(std::string szAccountId, std::string szPassword, std::string szCode)
{
	Packet pkt = Packet(LS_OTP);

	pkt
		<< szAccountId
		<< szPassword
		<< szCode;

	SendPacket(pkt);
}

void Client::SetCharacterSpeed(float fSpeed)
{
	WriteFloat(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_SWIFT")), fSpeed);
}

float Client::GetCharacterSpeed()
{
	return ReadFloat(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_SWIFT")));
}

void Client::PatchSpeedHack(bool bEnable)
{
	HANDLE hProcess = m_Bot->GetInjectedProcessHandle();

	if (bEnable)
	{
		BYTE byPatch1[] =
		{
			0xC2, 0x08, 0x00
		};

		LPVOID pPatchAddress = VirtualAllocEx(hProcess, nullptr, sizeof(byPatch1), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

		if (pPatchAddress == 0)
			return;

		WriteProcessMemory(hProcess, pPatchAddress, byPatch1, sizeof(byPatch1), 0);

		BYTE byPatch2[] =
		{
			0xE8, 0x00, 0x00, 0x00, 0x00
		};

		DWORD iCallDifference = Memory::GetDifference(GetAddress(skCryptDec("KO_PTR_MOVE_HOOK")), (DWORD)pPatchAddress);
		CopyBytes(byPatch2 + 1, iCallDifference);

		WriteProcessMemory(hProcess, (LPVOID*)GetAddress(skCryptDec("KO_PTR_MOVE_HOOK")), byPatch2, sizeof(byPatch2), 0);
	}
	else
	{
		BYTE byPatch[] =
		{
			0xE8, 0x00, 0x00, 0x00, 0x00
		};

		DWORD iCallDifference = Memory::GetDifference(GetAddress(skCryptDec("KO_PTR_MOVE_HOOK")), (DWORD)GetAddress(skCryptDec("KO_SND_FNC")));
		CopyBytes(byPatch + 1, iCallDifference);

		WriteProcessMemory(hProcess, (LPVOID*)GetAddress(skCryptDec("KO_PTR_MOVE_HOOK")), byPatch, sizeof(byPatch), 0);
	}
}

bool Client::IsVipWarehouseOpen()
{
	DWORD iVipWarehouseBase = Read4Byte(GetAddress("KO_PTR_DLG")) + GetAddress(skCryptDec("KO_OFF_VIP"));
	DWORD iVipWarehouseOpenAddress = Read4Byte(iVipWarehouseBase) + 0xE4;
	return Read2Byte(iVipWarehouseOpenAddress) != 0;
}

bool Client::IsTransactionDialogOpen()
{
	DWORD iTransactionDlgBase = Read4Byte(GetAddress("KO_PTR_DLG")) + GetAddress(skCryptDec("KO_OFF_TRANSACTION_DLG"));
	DWORD iiTransactionDlgOpenAddress = Read4Byte(iTransactionDlgBase) + 0xE4;
	return Read2Byte(iiTransactionDlgOpenAddress) != 0;
}

bool Client::IsWarehouseOpen()
{
	DWORD iTransactionDlgBase = Read4Byte(GetAddress("KO_PTR_DLG")) + 0x1F8;
	DWORD iiTransactionDlgOpenAddress = Read4Byte(iTransactionDlgBase) + 0xE4;
	return Read2Byte(iiTransactionDlgOpenAddress) != 0;
}

BYTE Client::ReadByte(DWORD dwAddress)
{
	return m_Bot->ReadByte(dwAddress);
}

WORD Client::Read2Byte(DWORD dwAddress)
{
	return m_Bot->Read2Byte(dwAddress);
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