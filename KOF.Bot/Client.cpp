#include "pch.h"
#include "Client.h"
#include "Bot.h"
#include "Memory.h"
#include "pch.h"

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
	m_fAttackTimeRecent = TimeGet();

	m_fLastGenieStartTime = 0.0f;

	m_iFlashCount = 0;

	m_bVipWarehouseInitialized = false;
	m_bVipWarehouseEnabled = false;

	m_fLastDisconnectTime = 0.0f;

	m_bSkillCasting = false;

	m_bVipWarehouseFull = false;
}

int32_t Client::GetID(DWORD iBase)
{
	if (iBase == 0)
		iBase = m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR")));

	return m_Bot->Read4Byte(iBase + m_Bot->GetAddress(skCryptDec("KO_OFF_ID")));
}

int32_t Client::GetProtoID(DWORD iBase)
{
	if (iBase == 0)
		iBase = m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR")));

	return m_Bot->Read4Byte(iBase + m_Bot->GetAddress(skCryptDec("KO_OFF_PROTO_ID")));
}

std::string Client::GetName(DWORD iBase)
{
	if (iBase == 0)
		iBase = m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR")));

	int iNameLen = m_Bot->Read4Byte(iBase + m_Bot->GetAddress(skCryptDec("KO_OFF_NAME_LEN")));

	if (iNameLen > 15)
		return m_Bot->ReadString(m_Bot->Read4Byte(iBase + m_Bot->GetAddress(skCryptDec("KO_OFF_NAME"))), iNameLen);

	return m_Bot->ReadString(iBase + m_Bot->GetAddress(skCryptDec("KO_OFF_NAME")), iNameLen);
}

int16_t Client::GetHp(DWORD iBase)
{
	if (iBase == 0)
		iBase = m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR")));

	return (int16_t)m_Bot->Read4Byte(iBase + m_Bot->GetAddress(skCryptDec("KO_OFF_HP")));
}

int16_t Client::GetMaxHp(DWORD iBase)
{
	if (iBase == 0)
		iBase = m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR")));

	return (int16_t)m_Bot->Read4Byte(iBase + m_Bot->GetAddress(skCryptDec("KO_OFF_MAXHP")));
}

int16_t Client::GetMp(DWORD iBase)
{
	if (iBase == 0)
		iBase = m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR")));

	return (int16_t)m_Bot->Read4Byte(iBase + m_Bot->GetAddress(skCryptDec("KO_OFF_MP")));
}

int16_t Client::GetMaxMp(DWORD iBase)
{
	if (iBase == 0)
		iBase = m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR")));

	return (int16_t)m_Bot->Read4Byte(iBase + m_Bot->GetAddress(skCryptDec("KO_OFF_MAXMP")));
}

uint8_t Client::GetZone(DWORD iBase)
{
	if (iBase == 0)
		iBase = m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR")));

	return (uint8_t)m_Bot->Read4Byte(iBase + m_Bot->GetAddress(skCryptDec("KO_OFF_ZONE")));
}

uint32_t Client::GetGold(DWORD iBase)
{
	if (iBase == 0)
		iBase = m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR")));

	return m_Bot->Read4Byte(iBase + m_Bot->GetAddress(skCryptDec("KO_OFF_GOLD")));
}

uint8_t Client::GetLevel(DWORD iBase)
{
	if (iBase == 0)
		iBase = m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR")));

	return m_Bot->ReadByte(iBase + m_Bot->GetAddress(skCryptDec("KO_OFF_LEVEL")));
}

uint8_t Client::GetNation(DWORD iBase)
{
	if (iBase == 0)
		iBase = m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR")));

	return m_Bot->ReadByte(iBase + m_Bot->GetAddress(skCryptDec("KO_OFF_NATION")));
}

int32_t Client::GetClass(DWORD iBase)
{
	if (iBase == 0)
		iBase = m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR")));

	return m_Bot->Read4Byte(iBase + m_Bot->GetAddress(skCryptDec("KO_OFF_CLASS")));
}

uint64_t Client::GetExp(DWORD iBase)
{
	if (iBase == 0)
		iBase = m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR")));

	return (uint64_t)m_Bot->Read4Byte(iBase + m_Bot->GetAddress(skCryptDec("KO_OFF_EXP")));
}

uint64_t Client::GetMaxExp(DWORD iBase)
{
	if (iBase == 0)
		iBase = m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR")));

	return (uint64_t)m_Bot->Read4Byte(iBase + m_Bot->GetAddress(skCryptDec("KO_OFF_MAXEXP")));
}

uint8_t Client::GetMoveState(DWORD iBase)
{
	if (iBase == 0)
		iBase = m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR")));

	return m_Bot->ReadByte(iBase + m_Bot->GetAddress(skCryptDec("KO_OFF_MOVE_TYPE")));
}

uint8_t Client::GetActionState(DWORD iBase)
{
	if (iBase == 0)
		iBase = m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR")));

	return m_Bot->ReadByte(iBase + m_Bot->GetAddress(skCryptDec("KO_OFF_ACTION_STATE")));
}

float Client::GetRadius(DWORD iBase)
{
	if (iBase == 0)
		iBase = m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR")));

	return m_Bot->ReadFloat(iBase + m_Bot->GetAddress(skCryptDec("KO_OFF_RADIUS")));
}

void Client::SetScale(DWORD iBase, float fX, float fZ, float fY)
{
	if (iBase == 0)
		iBase = m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR")));

	m_Bot->WriteFloat(iBase + m_Bot->GetAddress(skCryptDec("KO_OFF_SCALE_X")), fX);
	m_Bot->WriteFloat(iBase + m_Bot->GetAddress(skCryptDec("KO_OFF_SCALE_Z")), fZ);
	m_Bot->WriteFloat(iBase + m_Bot->GetAddress(skCryptDec("KO_OFF_SCALE_Y")), fY);
}

float Client::GetScaleX(DWORD iBase)
{
	if (iBase == 0)
		iBase = m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR")));

	return m_Bot->ReadFloat(iBase + m_Bot->GetAddress(skCryptDec("KO_OFF_SCALE_X")));
}

float Client::GetScaleZ(DWORD iBase)
{
	if (iBase == 0)
		iBase = m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR")));

	return m_Bot->ReadFloat(iBase + m_Bot->GetAddress(skCryptDec("KO_OFF_SCALE_Z")));
}

float Client::GetScaleY(DWORD iBase)
{
	if (iBase == 0)
		iBase = m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR")));

	return m_Bot->ReadFloat(iBase + m_Bot->GetAddress(skCryptDec("KO_OFF_SCALE_Y")));
}

bool Client::IsAttackable(DWORD iTargetID)
{
	if (iTargetID == -1)
		return false;

	if (iTargetID >= 5000)
	{
		auto it = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
			[&](const TNpc& a) { return a.iID == iTargetID; });

		if (it != m_vecNpc.end())
		{
			if (it->iMonsterOrNpc == 2 && it->iFamilyType != 171)
				return false;

			if ((it->eState == PSA_DYING || it->eState == PSA_DEATH) || (it->iHPMax != 0 && it->iHP <= 0))
				return false;


			return true;
		}
	}
	else
	{
		auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
			[&](const TPlayer& a) { return a.iID == iTargetID; });

		if (it != m_vecPlayer.end())
		{
			if (it->eNation == m_PlayerMySelf.eNation)
				return false;

			if ((it->eState == PSA_DYING || it->eState == PSA_DEATH) || (it->iHPMax != 0 && it->iHP <= 0))
				return false;


			return true;
		}
	}

	return false;
}

uint8_t Client::GetServerId()
{
	return (uint8_t)m_Bot->Read4Byte(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_INTRO"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_SERVER")));
}

bool Client::IsDisconnect()
{
	return m_Bot->Read4Byte(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_PKT"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_DISCONNECT"))) == 0;
};

float Client::GetGoX()
{
	return m_Bot->ReadFloat(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_GOX")));
}

float Client::GetGoY()
{
	return m_Bot->ReadFloat(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_GOY")));
}

float Client::GetGoZ()
{
	return m_Bot->ReadFloat(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_GOZ")));
}

float Client::GetX(DWORD iBase)
{
	if (iBase == 0)
		iBase = m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR")));

	return m_Bot->ReadFloat(iBase + m_Bot->GetAddress(skCryptDec("KO_OFF_X")));
}

float Client::GetZ(DWORD iBase)
{
	if (iBase == 0)
		iBase = m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR")));

	return m_Bot->ReadFloat(iBase + m_Bot->GetAddress(skCryptDec("KO_OFF_Z")));
}

float Client::GetY(DWORD iBase)
{
	if (iBase == 0)
		iBase = m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR")));

	return m_Bot->ReadFloat(iBase + m_Bot->GetAddress(skCryptDec("KO_OFF_Y")));
}

uint8_t Client::GetAuthority(DWORD iBase)
{
	if (iBase == 0)
		iBase = m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR")));

	return m_Bot->ReadByte(iBase + m_Bot->GetAddress(skCryptDec("KO_OFF_AUTHORITY")));
}

bool Client::IsDeath(DWORD iBase)
{
	if (iBase == 0)
		iBase = m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR")));

	uint8_t iActionState = GetActionState(iBase);

	if (iActionState == PSA_DYING || iActionState == PSA_DEATH)
		return true;

	return false;
}

bool Client::IsStunned(DWORD iBase)
{
	if (iBase == 0)
		iBase = m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR")));

	return m_Bot->ReadByte(iBase + m_Bot->GetAddress(skCryptDec("KO_OFF_STUN"))) != 0;
}

uint32_t Client::GetTargetBase()
{
	return GetEntityBase(GetTarget());
}

bool Client::IsRogue(int32_t eClass)
{
	if (eClass == CLASS_UNKNOWN)
	{
		eClass = m_PlayerMySelf.eClass;
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
		eClass = m_PlayerMySelf.eClass;
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
		eClass = m_PlayerMySelf.eClass;
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
		eClass = m_PlayerMySelf.eClass;
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

float Client::GetSkillNextUseTime(int32_t iSkillID)
{
	auto it = m_mapSkillUseTime.find(iSkillID);

	if (it != m_mapSkillUseTime.end())
		return it->second;

	return 0.0f;
}

void Client::SetSkillNextUseTime(int32_t iSkillID, float fSkillUseTime)
{
	auto it = m_mapSkillUseTime.find(iSkillID);

	if (it == m_mapSkillUseTime.end())
		m_mapSkillUseTime.insert(std::make_pair(iSkillID, fSkillUseTime));
	else
		it->second = fSkillUseTime;
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

	if (iTargetId == m_PlayerMySelf.iID)
		return m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR")));

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

	DWORD iFldb = m_Bot->GetAddress(skCryptDec("KO_PTR_FLDB"));
	DWORD iFmbs = m_Bot->GetAddress(skCryptDec("KO_PTR_FMBS"));

	memcpy(byCode + 3, &iFldb, sizeof(iFldb));
	memcpy(byCode + 10, &iTargetId, sizeof(iTargetId));
	memcpy(byCode + 15, &iFmbs, sizeof(iFmbs));

	HANDLE hProcess = m_Bot->GetClientProcessHandle();

	LPVOID pAddress = VirtualAllocEx(hProcess, nullptr, sizeof(DWORD), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (!pAddress)
	{
		return 0;
	}

	memcpy(byCode + 22, &pAddress, sizeof(pAddress));

	DWORD iBase = 0;
	SIZE_T bytesRead = 0;

	if (m_Bot->ExecuteRemoteCode(hProcess, byCode, sizeof(byCode)))
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

	if (iTargetID >= 5000)
	{
		auto it = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
			[&](const TNpc& a) { return a.iID == iTargetID; });

		if (it != m_vecNpc.end())
		{
			return Vector3(it->fX, it->fZ, it->fY);
		}
	}
	else
	{
		auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
			[&](const TPlayer& a) { return a.iID == iTargetID; });

		if (it != m_vecPlayer.end())
		{
			return Vector3(it->fX, it->fZ, it->fY);
		}
	}

	return Vector3(0.0f, 0.0f, 0.0f);
}

int32_t Client::GetTarget()
{
	return m_Bot->Read4Byte(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_MOB")));
}

void Client::SetAuthority(uint8_t iAuthority)
{
	m_Bot->WriteByte(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_AUTHORITY")), iAuthority);
}

bool Client::IsBuffActive(int32_t iBuffType)
{ 
	int32_t iSkillBase = m_Bot->Read4Byte(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_DLG"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_SKILL_BASE")));

	int32_t iSkillContainer1 = m_Bot->Read4Byte(iSkillBase + 0x4);
	int32_t iSkillContainer1Slot = m_Bot->Read4Byte(iSkillContainer1 + m_Bot->GetAddress(skCryptDec("KO_OFF_SKILL_SLOT")));

	for (int32_t i = 0; i < 20; i++)
	{
		iSkillContainer1Slot = m_Bot->Read4Byte(iSkillContainer1Slot + 0x0);

		int32_t iAffectedSkill = m_Bot->Read4Byte(iSkillContainer1Slot + 0x8);

		if (iAffectedSkill == 0)
			continue;

		if (m_Bot->Read4Byte(iAffectedSkill + 0x4) == iBuffType)
			return true;
	}

	int32_t iSkillContainer2 = m_Bot->Read4Byte(iSkillBase + 0x8);
	int32_t iSkillContainer2Slot = m_Bot->Read4Byte(iSkillContainer2 + m_Bot->GetAddress(skCryptDec("KO_OFF_SKILL_SLOT")));

	for (int32_t i = 0; i < 20; i++)
	{
		iSkillContainer1Slot = m_Bot->Read4Byte(iSkillContainer1Slot + 0x0);

		int32_t iAffectedSkill = m_Bot->Read4Byte(iSkillContainer1Slot + 0x8);

		if (iAffectedSkill == 0)
			continue;

		if (m_Bot->Read4Byte(iAffectedSkill + 0x4) == iBuffType)
			return true;
	}
		
	return false; 
};

bool Client::IsSkillActive(int32_t iSkillID)
{
	int32_t iSkillBase = m_Bot->Read4Byte(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_DLG"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_SKILL_BASE")));

	int32_t iSkillContainer1 = m_Bot->Read4Byte(iSkillBase + 0x4);
	int32_t iSkillContainer1Slot = m_Bot->Read4Byte(iSkillContainer1 + m_Bot->GetAddress(skCryptDec("KO_OFF_SKILL_SLOT")));

	for (int32_t i = 0; i < 20; i++)
	{
		iSkillContainer1Slot = m_Bot->Read4Byte(iSkillContainer1Slot + 0x0);

		int32_t iAffectedSkill = m_Bot->Read4Byte(iSkillContainer1Slot + 0x8);

		if (iAffectedSkill == 0)
			continue;

		if (m_Bot->Read4Byte(iAffectedSkill + 0x0) == iSkillID)
			return true;
	}

	int32_t iSkillContainer2 = m_Bot->Read4Byte(iSkillBase + 0x8);
	int32_t iSkillContainer2Slot = m_Bot->Read4Byte(iSkillContainer2 + m_Bot->GetAddress(skCryptDec("KO_OFF_SKILL_SLOT")));

	for (int32_t i = 0; i < 20; i++)
	{
		iSkillContainer1Slot = m_Bot->Read4Byte(iSkillContainer1Slot + 0x0);

		int32_t iAffectedSkill = m_Bot->Read4Byte(iSkillContainer1Slot + 0x8);

		if (iAffectedSkill == 0)
			continue;

		if (m_Bot->Read4Byte(iAffectedSkill + 0x0) == iSkillID)
			return true;
	}

	return false;
};

bool Client::IsBlinking(int32_t iTargetID)
{
	if (iTargetID == -1 || iTargetID == m_PlayerMySelf.iID)
	{
		return m_PlayerMySelf.bBlinking;
	}
	else
	{
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
	return m_Bot->ReadByte(m_Bot->Read4Byte(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_DLG"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_SKILL_POINT_BASE"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_SKILL_POINT_START")) + (Slot * 4));
}

int32_t Client::GetInventoryItemCount(uint32_t iItemID)
{
	int32_t iItemCount = 0;

	DWORD iInventoryBase = m_Bot->Read4Byte(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_DLG"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_INVENTORY_BASE")));

	for (size_t i = SLOT_MAX; i < SLOT_MAX + HAVE_MAX; i++)
	{
		DWORD iItemBase = m_Bot->Read4Byte(iInventoryBase + (m_Bot->GetAddress(skCryptDec("KO_OFF_INVENTORY_START")) + (4 * i)));

		if (m_Bot->GetPlatformType() == PlatformType::USKO 
			|| m_Bot->GetPlatformType() == PlatformType::STKO)
		{
			if (m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x60)) + m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x64)) == iItemID)
				iItemCount += m_Bot->Read4Byte(iItemBase + 0x68);
		}
		else if (m_Bot->GetPlatformType() == PlatformType::CNKO)
		{
			if (m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x68)) + m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x6C)) == iItemID)
				iItemCount += m_Bot->Read4Byte(iItemBase + 0x70);
		}
	}

	return iItemCount;
}

TItemData Client::GetInventoryItem(uint32_t iItemID)
{
	TItemData pInventory;
	memset(&pInventory, 0, sizeof(pInventory));

	DWORD iInventoryBase = m_Bot->Read4Byte(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_DLG"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_INVENTORY_BASE")));

	for (size_t i = SLOT_MAX; i < SLOT_MAX + HAVE_MAX; i++)
	{
		DWORD iItemBase = m_Bot->Read4Byte(iInventoryBase + (m_Bot->GetAddress(skCryptDec("KO_OFF_INVENTORY_START")) + (4 * i)));

		if (m_Bot->GetPlatformType() == PlatformType::USKO 
			|| m_Bot->GetPlatformType() == PlatformType::STKO)
		{
			if (m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x60)) + m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x64)) == iItemID)
			{
				pInventory.iPos = i;
				pInventory.iItemID = iItemID;
				pInventory.iCount = (uint16_t)m_Bot->Read4Byte(iItemBase + 0x68);
				pInventory.iDurability = (uint16_t)m_Bot->Read4Byte(iItemBase + 0x6C);

				return pInventory;
			}
		}
		else if (m_Bot->GetPlatformType() == PlatformType::CNKO)
		{
			if (m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x68)) + m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x6C)) == iItemID)
			{
				pInventory.iPos = i;
				pInventory.iItemID = iItemID;
				pInventory.iCount = (uint16_t)m_Bot->Read4Byte(iItemBase + 0x70);
				pInventory.iDurability = (uint16_t)m_Bot->Read4Byte(iItemBase + 0x74);

				return pInventory;
			}
		}
	}

	return pInventory;
}

int32_t Client::GetInventoryEmptySlot()
{
	DWORD iInventoryBase = m_Bot->Read4Byte(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_DLG"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_INVENTORY_BASE")));

	for (size_t i = SLOT_MAX; i < SLOT_MAX + HAVE_MAX; i++)
	{
		DWORD iItemBase = m_Bot->Read4Byte(iInventoryBase + (m_Bot->GetAddress(skCryptDec("KO_OFF_INVENTORY_START")) + (4 * i)));

		if (m_Bot->GetPlatformType() == PlatformType::USKO 
			|| m_Bot->GetPlatformType() == PlatformType::STKO)
		{
			if (m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x60)) + m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x64)) == 0)
			{
				return i;
			}
		}
		else if (m_Bot->GetPlatformType() == PlatformType::CNKO)
		{
			if (m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x68)) + m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x6C)) == 0)
			{
				return i;
			}
		}
	}

	return -1;
}

int32_t Client::GetInventoryEmptySlot(std::vector<int32_t> vecExcept)
{
	DWORD iInventoryBase = m_Bot->Read4Byte(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_DLG"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_INVENTORY_BASE")));

	for (int i = SLOT_MAX; i < SLOT_MAX + HAVE_MAX; i++)
	{
		if (std::find(vecExcept.begin(), vecExcept.end(), i)
			!= vecExcept.end())
			continue;

		DWORD iItemBase = m_Bot->Read4Byte(iInventoryBase + (m_Bot->GetAddress(skCryptDec("KO_OFF_INVENTORY_START")) + (4 * i)));

		if (m_Bot->GetPlatformType() == PlatformType::USKO 
			|| m_Bot->GetPlatformType() == PlatformType::STKO)
		{
			if (m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x60)) + m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x64)) == 0)
				return i;
		}
		else if (m_Bot->GetPlatformType() == PlatformType::CNKO)
		{
			if (m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x68)) + m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x6C)) == 0)
				return i;
		}
	}

	return -1;
}

TItemData Client::GetInventoryItemSlot(uint8_t iSlotPosition)
{
	TItemData pInventory;
	memset(&pInventory, 0, sizeof(pInventory));

	DWORD iInventoryBase = m_Bot->Read4Byte(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_DLG"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_INVENTORY_BASE")));
	DWORD iItemBase = m_Bot->Read4Byte(iInventoryBase + (m_Bot->GetAddress(skCryptDec("KO_OFF_INVENTORY_START")) + (4 * iSlotPosition)));

	if (m_Bot->GetPlatformType() == PlatformType::USKO 
		|| m_Bot->GetPlatformType() == PlatformType::STKO)
	{
		uint32_t iItemID = m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x60)) + m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x64));

		if (iItemID != 0)
		{
			pInventory.iPos = iSlotPosition;
			pInventory.iBase = iItemBase;
			pInventory.iItemID = iItemID;
			pInventory.iCount = (uint16_t)m_Bot->Read4Byte(iItemBase + 0x68);
			pInventory.iDurability = (uint16_t)m_Bot->Read4Byte(iItemBase + 0x6C);

			return pInventory;
		}
	}
	else if (m_Bot->GetPlatformType() == PlatformType::CNKO)
	{
		uint32_t iItemID = m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x68)) + m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x6C));

		if (m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x68)) + m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x6C)) != 0)
		{
			pInventory.iPos = iSlotPosition;
			pInventory.iBase = iItemBase;
			pInventory.iItemID = iItemID;
			pInventory.iCount = (uint16_t)m_Bot->Read4Byte(iItemBase + 0x70);
			pInventory.iDurability = (uint16_t)m_Bot->Read4Byte(iItemBase + 0x74);

			return pInventory;
		}
	}

	return pInventory;
}

DWORD Client::GetInventoryItemBase(uint8_t iSlotPosition)
{
	DWORD iInventoryBase = m_Bot->Read4Byte(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_DLG"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_INVENTORY_BASE")));
	return m_Bot->Read4Byte(iInventoryBase + (m_Bot->GetAddress(skCryptDec("KO_OFF_INVENTORY_START")) + (4 * iSlotPosition)));
}

bool Client::GetInventoryItemList(std::vector<TItemData>& vecItemList)
{
	DWORD iInventoryBase = m_Bot->Read4Byte(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_DLG"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_INVENTORY_BASE")));

	for (int i = SLOT_MAX; i < SLOT_MAX + HAVE_MAX; i++)
	{
		TItemData pItem;
		memset(&pItem, 0, sizeof(pItem));

		pItem.iPos = i;

		DWORD iItemBase = m_Bot->Read4Byte(iInventoryBase + (m_Bot->GetAddress(skCryptDec("KO_OFF_INVENTORY_START")) + (4 * i)));

		if (m_Bot->GetPlatformType() == PlatformType::USKO 
			|| m_Bot->GetPlatformType() == PlatformType::STKO)
		{
			if (m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x60)) + m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x64)) != 0)
			{
				pItem.iBase = iItemBase;
				pItem.iItemID = m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x60)) + m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x64));
				pItem.iCount = (uint16_t)m_Bot->Read4Byte(iItemBase + 0x68);
				pItem.iDurability = (uint16_t)m_Bot->Read4Byte(iItemBase + 0x6C);
			}

			vecItemList.push_back(pItem);
		}
		else if (m_Bot->GetPlatformType() == PlatformType::CNKO)
		{
			if (m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x68)) + m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x6C)) != 0)
			{
				pItem.iBase = iItemBase;
				pItem.iItemID = m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x68)) + m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x6C));
				pItem.iCount = (uint16_t)m_Bot->Read4Byte(iItemBase + 0x70);
				pItem.iDurability = (uint16_t)m_Bot->Read4Byte(iItemBase + 0x74);
			}

			vecItemList.push_back(pItem);
		}
	}

	return true;
}

int Client::GetInventoryEmptySlotCount()
{
	DWORD iInventoryBase = m_Bot->Read4Byte(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_DLG"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_INVENTORY_BASE")));

	int iEmptySlotCount = 0;

	for (int i = SLOT_MAX; i < SLOT_MAX + HAVE_MAX; i++)
	{
		DWORD iItemBase = m_Bot->Read4Byte(iInventoryBase + (m_Bot->GetAddress(skCryptDec("KO_OFF_INVENTORY_START")) + (4 * i)));

		if (m_Bot->GetPlatformType() == PlatformType::USKO
			|| m_Bot->GetPlatformType() == PlatformType::STKO)
		{
			if (m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x60)) + m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x64)) == 0)
			{
				iEmptySlotCount++;
			}
		}
		else if (m_Bot->GetPlatformType() == PlatformType::CNKO)
		{
			if (m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x68)) + m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x6C)) == 0)
			{
				iEmptySlotCount++;
			}
		}
	}

	return iEmptySlotCount;
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

	DWORD iDlg = m_Bot->GetAddress(skCryptDec("KO_PTR_DLG"));
	CopyBytes(byCode + 3, iDlg);

	int8_t iStart = bStart ? 1 : 0;
	CopyBytes(byCode + 10, iStart);

	DWORD iWscb = m_Bot->GetAddress(skCryptDec("KO_WSCB"));
	CopyBytes(byCode + 12, iWscb);

	HANDLE hProcess = m_Bot->GetClientProcessHandle();

	m_Bot->ExecuteRemoteCode(hProcess, byCode, sizeof(byCode));
}

void Client::BasicAttack()
{
	Packet pkt = Packet(PIPE_BASIC_ATTACK);

	pkt << uint8_t(1);

	m_Bot->SendInternalMailslot(pkt);

	m_fAttackTimeRecent = TimeGet();
}

void Client::BasicAttackWithPacket(DWORD iTargetID, float fAttackInterval)
{
	/*DWORD iTargetBase = GetEntityBase(iTargetID);

	if (iTargetBase == 0)
		return;

	Vector3 v3TargetPosition = GetTargetPosition();
	float fDistance = GetDistance(v3TargetPosition);

	float fTargetRadius = GetRadius(iTargetBase) * GetScaleZ(iTargetBase);
	float fMySelfRadius = GetRadius() * GetScaleZ();

	float fDistanceExceptRadius = fDistance - ((fMySelfRadius + fTargetRadius) / 2.0f);

	if (fDistanceExceptRadius > 10.0f)
		return;

	SendBasicAttackPacket(iTargetID, fAttackInterval, fDistanceExceptRadius);*/
	SendBasicAttackPacket(iTargetID, fAttackInterval, 0.2f);

	m_fAttackTimeRecent = TimeGet();
}

DWORD Client::GetSkillBase(uint32_t iSkillID)
{
	HANDLE hProcess = m_Bot->GetClientProcessHandle();

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

	DWORD iSbec = m_Bot->GetAddress(skCryptDec("KO_SBEC"));

	CopyBytes(byCode + 3, iSbec);
	CopyBytes(byCode + 8, iSkillID);

	DWORD iSbca = m_Bot->GetAddress(skCryptDec("KO_SBCA"));

	CopyBytes(byCode + 13, iSbca);
	CopyBytes(byCode + 20, pBaseAddress);

	m_Bot->ExecuteRemoteCode(hProcess, byCode, sizeof(byCode));

	DWORD iSkillBase = m_Bot->Read4Byte((DWORD)pBaseAddress);

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

	DWORD iDLG = m_Bot->GetAddress(skCryptDec("KO_PTR_DLG"));
	CopyBytes(byCode + 6, iDLG);

	DWORD i06 = m_Bot->GetAddress(skCryptDec("KO_PTR_06"));
	CopyBytes(byCode + 13, i06);

	HANDLE hProcess = m_Bot->GetClientProcessHandle();

	m_Bot->ExecuteRemoteCode(hProcess, byCode, sizeof(byCode));
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

	DWORD dwPushPhase = m_Bot->GetAddress(skCryptDec("KO_PTR_PUSH_PHASE"));
	CopyBytes(byCode + 8, dwPushPhase);

	HANDLE hProcess = m_Bot->GetClientProcessHandle();

	m_Bot->ExecuteRemoteCode(hProcess, byCode, sizeof(byCode));
}

void Client::WriteLoginInformation(std::string szAccountId, std::string szPassword)
{
	DWORD dwCGameProcIntroLogin = m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_INTRO")));
	DWORD dwCUILoginIntro = m_Bot->Read4Byte(dwCGameProcIntroLogin + m_Bot->GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO")));

	DWORD dwCN3UIEditIdBase = m_Bot->Read4Byte(dwCUILoginIntro + m_Bot->GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO_ID")));
	DWORD dwCN3UIEditPwBase = m_Bot->Read4Byte(dwCUILoginIntro + m_Bot->GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO_PW")));

	m_Bot->WriteString(dwCN3UIEditIdBase + m_Bot->GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO_ID_INPUT")), szAccountId.c_str());
	m_Bot->Write4Byte(dwCN3UIEditIdBase + m_Bot->GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO_ID_INPUT_LENGTH")), szAccountId.size());

	m_Bot->WriteString(dwCN3UIEditPwBase + m_Bot->GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO_PW_INPUT")), szPassword.c_str());
	m_Bot->Write4Byte(dwCN3UIEditPwBase + m_Bot->GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO_PW_INPUT_LENGTH")), szPassword.size());
}

void Client::ConnectLoginServer(bool bDisconnect)
{
	Packet pkt = Packet(PIPE_LOGIN);
	pkt << uint8_t(1) << uint8_t(bDisconnect);
	m_Bot->SendInternalMailslot(pkt);
}

void Client::LoadServerList()
{
	Packet pkt = Packet(PIPE_LOAD_SERVER_LIST);
	pkt << uint8_t(1);
	m_Bot->SendInternalMailslot(pkt);
}

void Client::SelectServer(uint8_t iIndex)
{
	Packet pkt = Packet(PIPE_SELECT_SERVER);
	pkt << uint8_t(iIndex);
	m_Bot->SendInternalMailslot(pkt);
}

void Client::ShowChannel()
{
	Packet pkt = Packet(PIPE_SHOW_CHANNEL);
	pkt << uint8_t(1);
	m_Bot->SendInternalMailslot(pkt);
}

void Client::SelectChannel(uint8_t iIndex)
{
	Packet pkt = Packet(PIPE_SELECT_CHANNEL);
	pkt << uint8_t(iIndex);
	m_Bot->SendInternalMailslot(pkt);
}

void Client::ConnectServer()
{
	Packet pkt = Packet(PIPE_CONNECT_SERVER);
	pkt << uint8_t(1);
	m_Bot->SendInternalMailslot(pkt);
}

void Client::SetSaveCPUSleepTime(int32_t iValue)
{
	Packet pkt = Packet(PIPE_SAVE_CPU);
	pkt << int32_t(iValue);
	m_Bot->SendInternalMailslot(pkt);
}

void Client::SelectCharacterSkip()
{
	Packet pkt = Packet(PIPE_SELECT_CHARACTER_SKIP);

	pkt << uint8_t(1);

	m_Bot->SendInternalMailslot(pkt);
}

void Client::SelectCharacterLeft()
{
	Packet pkt = Packet(PIPE_SELECT_CHARACTER_LEFT);

	pkt << uint8_t(1);

	m_Bot->SendInternalMailslot(pkt);
}

void Client::SelectCharacterRight()
{
	Packet pkt = Packet(PIPE_SELECT_CHARACTER_RIGHT);

	pkt << uint8_t(1);

	m_Bot->SendInternalMailslot(pkt);
}

void Client::SelectCharacter()
{
	Packet pkt = Packet(PIPE_SELECT_CHARACTER_ENTER);

	pkt << uint8_t(1);

	m_Bot->SendInternalMailslot(pkt);
}

void Client::SendPacket(Packet vecBuffer)
{
	HANDLE hProcess = m_Bot->GetClientProcessHandle();

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

	DWORD dwPtrPkt = m_Bot->GetAddress(skCryptDec("KO_PTR_PKT"));

	CopyBytes(byCode + 3, dwPtrPkt);

	size_t dwPacketSize = vecBuffer.size();

	CopyBytes(byCode + 8, dwPacketSize);
	CopyBytes(byCode + 13, pPacketAddress);

	DWORD dwPtrSndFnc = m_Bot->GetAddress(skCryptDec("KO_SND_FNC"));
	CopyBytes(byCode + 18, dwPtrSndFnc);

	m_Bot->ExecuteRemoteCode(hProcess, byCode, sizeof(byCode));
	VirtualFreeEx(hProcess, pPacketAddress, 0, MEM_RELEASE);
}

void Client::SendPacket(std::string szPacket)
{
	HANDLE hProcess = m_Bot->GetClientProcessHandle();

	std::vector<uint8_t> vecPacketByte = FromHexString(szPacket);

	LPVOID pPacketAddress = VirtualAllocEx(hProcess, nullptr, vecPacketByte.size(), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (pPacketAddress == nullptr)
	{
		return;
	}

	WriteProcessMemory(hProcess, pPacketAddress, vecPacketByte.data(), vecPacketByte.size(), 0);

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

	DWORD dwPtrPkt = m_Bot->GetAddress(skCryptDec("KO_PTR_PKT"));

	CopyBytes(byCode + 3, dwPtrPkt);

	size_t dwPacketSize = szPacket.size();

	CopyBytes(byCode + 8, dwPacketSize);
	CopyBytes(byCode + 13, pPacketAddress);

	DWORD dwPtrSndFnc = m_Bot->GetAddress(skCryptDec("KO_SND_FNC"));
	CopyBytes(byCode + 18, dwPtrSndFnc);

	m_Bot->ExecuteRemoteCode(hProcess, byCode, sizeof(byCode));
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
			if (pSkillData.iReCastTime > 0)
			{
				m_bSkillCasting = true;

				if (m_PlayerMySelf.iMoveType != 0)
				{
					SendMovePacket(v3MyPosition, v3MyPosition, 0, 0);
				}

				SendStartSkillCastingAtTargetPacket(pSkillData, iTargetID);

				SetSkillNextUseTime(pSkillData.iID, TimeGet() + ((pSkillData.iReCastTime * 100.0f) / 1000.0f));

				if (bWaitCastTime || (pSkillData.dw1stTableType == 3 || pSkillData.dw1stTableType == 4))
				{
					std::this_thread::sleep_for(std::chrono::milliseconds((((pSkillData.iReCastTime * 100) * 75) / 100)));
				}

				m_bSkillCasting = false;
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

			if (pSkillData.iCooldown > 0)
			{
				SetSkillNextUseTime(pSkillData.iID, TimeGet() + ((pSkillData.iCooldown * 100.0f) / 1000.0f));
			}
			else
			{
				//SetSkillNextUseTime(pSkillData.iID, Bot::TimeGet() + (100.0f / 1000.0f));
			}
		}
		break;

		case SkillTargetType::TARGET_AREA:
		case SkillTargetType::TARGET_PARTY_ALL:
		case SkillTargetType::TARGET_AREA_ENEMY:
		{
			if (pSkillData.iReCastTime > 0)
			{
				m_bSkillCasting = true;

				if (m_PlayerMySelf.iMoveType != 0)
				{
					SendMovePacket(v3MyPosition, v3MyPosition, 0, 0);
				}

				SendStartSkillCastingAtPosPacket(pSkillData, v3TargetPosition);

				SetSkillNextUseTime(pSkillData.iID, TimeGet() + ((pSkillData.iReCastTime * 100.0f) / 1000.0f));	

				if (bWaitCastTime || (pSkillData.dw1stTableType == 3 || pSkillData.dw1stTableType == 4))
				{
					std::this_thread::sleep_for(std::chrono::milliseconds((((pSkillData.iReCastTime * 100) * 75) / 100)));
				}

				m_bSkillCasting = false;
			}

			if (pSkillData.iFlyingFX != 0)
				SendStartFlyingAtTargetPacket(pSkillData, -1, v3TargetPosition);

			SendStartSkillMagicAtPosPacket(pSkillData, v3TargetPosition);
			SendStartMagicAtTarget(pSkillData, -1, v3TargetPosition);

			if (pSkillData.iCooldown > 0)
			{
				SetSkillNextUseTime(pSkillData.iID, TimeGet() + ((pSkillData.iCooldown * 100.0f) / 1000.0f));
			}
			else
			{
				//SetSkillNextUseTime(pSkillData.iID, Bot::TimeGet() + (100.0f / 1000.0f));
			}
		}
		break;
	}
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

	/*int iX, iY = 0;
	GetChildAreaByiOrder(1, 6, iX, iY);

	DWORD iInventoryBase = m_Bot->Read4Byte(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_DLG"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_INVENTORY_BASE")));
	DWORD iItemBase = m_Bot->Read4Byte(iInventoryBase + (m_Bot->GetAddress(skCryptDec("KO_OFF_INVENTORY_START")) + (4 * 14)));

	m_Bot->Write4Byte(0x010E5B9C, 0);
	m_Bot->Write4Byte(0x010E5BA0, 1);
	m_Bot->Write4Byte(0x010E5BA4, 0);
	m_Bot->Write4Byte(0x010E5BA8, iItemBase);

	ReceiveIconDrop(iItemBase, iX, iY);*/

	//VipGetInTest();
	//VipGetOutTest();

	//GetChildAreaByiOrder(1, 6); //ITEM Sol
	//GetChildAreaByiOrder(1, 8); //ITEM Sag
}

void Client::SetPosition(Vector3 v3Position)
{
	m_Bot->WriteFloat(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_X")), v3Position.m_fX);
	m_Bot->WriteFloat(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_Y")), v3Position.m_fY);
	m_Bot->WriteFloat(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_Z")), v3Position.m_fZ);
}

void Client::SetMovePosition(Vector3 v3MovePosition)
{
	if (v3MovePosition.m_fX == 0.0f && v3MovePosition.m_fY == 0.0f && v3MovePosition.m_fZ == 0.0f)
	{
		m_Bot->WriteByte(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_MOVE_TYPE")), 0);
		m_Bot->WriteFloat(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_GOX")), v3MovePosition.m_fX);
		m_Bot->WriteFloat(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_GOY")), v3MovePosition.m_fY);
		m_Bot->WriteFloat(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_GOZ")), v3MovePosition.m_fZ);
		m_Bot->WriteByte(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_MOVE")), 0);
	}
	else
	{
		m_Bot->WriteByte(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_MOVE_TYPE")), 2);
		m_Bot->WriteFloat(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_GOX")), v3MovePosition.m_fX);
		m_Bot->WriteFloat(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_GOY")), v3MovePosition.m_fY);
		m_Bot->WriteFloat(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_GOZ")), v3MovePosition.m_fZ);
		m_Bot->WriteByte(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_MOVE")), 1);
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

void Client::SetTarget(uint32_t iTargetID)
{
	DWORD iTargetBase = GetEntityBase(iTargetID);

	if (iTargetBase == 0)
		return;

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

	DWORD iDlg = m_Bot->GetAddress(skCryptDec("KO_PTR_DLG"));
	CopyBytes(byCode + 2, iDlg);
	CopyBytes(byCode + 9, iTargetBase); // Target Base

	DWORD iSelectMob = m_Bot->GetAddress(skCryptDec("KO_SELECT_MOB"));
	CopyBytes(byCode + 15, iSelectMob);

	HANDLE hProcess = m_Bot->GetClientProcessHandle();

	m_Bot->ExecuteRemoteCode(hProcess, byCode, sizeof(byCode));
}

void Client::PatchDeathEffect(bool bValue)
{
	if (bValue)
	{
		if (m_vecOrigDeathEffectFunction.size() == 0)
			m_vecOrigDeathEffectFunction = m_Bot->ReadBytes(m_Bot->GetAddress(skCryptDec("KO_DEATH_EFFECT")), 2);

		std::vector<uint8_t> vecPatch = { 0x90, 0x90 };
		m_Bot->WriteBytes(m_Bot->GetAddress(skCryptDec("KO_DEATH_EFFECT")), vecPatch);
	}
	else
	{
		if (m_vecOrigDeathEffectFunction.size() > 0)
			m_Bot->WriteBytes(m_Bot->GetAddress(skCryptDec("KO_DEATH_EFFECT")), m_vecOrigDeathEffectFunction);
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
	uint8_t iZoneID = GetRepresentZone(m_PlayerMySelf.iCity);

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

	for (auto it = mapDisguiseTable->begin(); it != mapDisguiseTable->end(); ++it) 
	{
		const auto& kvPair = *it;
		const auto& v = kvPair.second;

		if (IsSkillActive(v.iSkillID)) 
		{
			return false;
		}
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

void Client::UpdateSkillSuccessRate(bool bDisableCasting)
{
	for (auto& e : m_vecAvailableSkill)
	{
		DWORD iSkillBase = GetSkillBase(e.iID);

		if (iSkillBase == 0 || m_Bot->Read4Byte(iSkillBase + 0xA8) == 0)
			continue;

		if (bDisableCasting)
			m_Bot->Write4Byte(iSkillBase + 0xA8, 100);
		else
			m_Bot->Write4Byte(iSkillBase + 0xA8, e.iPercentSuccess);
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

void Client::VipWarehouseGetIn(DWORD iItemBase, int32_t iSourcePosition, int32_t iTargetPosition)
{
	if (iItemBase == 0)
		return;

	HANDLE hProcess = m_Bot->GetClientProcessHandle();

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

	DWORD iDlg = m_Bot->GetAddress(skCryptDec("KO_PTR_DLG"));
	memcpy(byAreaPositionCode + 3, &iDlg, sizeof(iDlg));

	DWORD iUIVipWarehouse = m_Bot->GetAddress(skCryptDec("KO_OFF_VIP"));
	memcpy(byAreaPositionCode + 9, &iUIVipWarehouse, sizeof(iUIVipWarehouse));

	memcpy(byAreaPositionCode + 14, &iTargetPosition, sizeof(iTargetPosition));

	DWORD iAreaType = 0x00000003;
	memcpy(byAreaPositionCode + 19, &iAreaType, sizeof(iAreaType));

	DWORD iAreaPositionCallAddress = m_Bot->GetAddress(skCryptDec("KO_PTR_AREA_POSITION"));
	memcpy(byAreaPositionCode + 24, &iAreaPositionCallAddress, sizeof(iAreaPositionCallAddress));

	LPVOID pCallReturnAddress = VirtualAllocEx(hProcess, nullptr, sizeof(DWORD), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (!pCallReturnAddress)
		return;

	memcpy(byAreaPositionCode + 31, &pCallReturnAddress, sizeof(pCallReturnAddress));

	DWORD iAreaPositionBase = 0;
	SIZE_T bytesRead = 0;

	if (m_Bot->ExecuteRemoteCode(hProcess, byAreaPositionCode, sizeof(byAreaPositionCode)))
	{
		if (!ReadProcessMemory(hProcess, pCallReturnAddress, &iAreaPositionBase, sizeof(iAreaPositionBase), &bytesRead) && bytesRead == sizeof(iAreaPositionBase))
		{
			VirtualFreeEx(hProcess, pCallReturnAddress, 0, MEM_RELEASE);
			return;
		}
	}

	VirtualFreeEx(hProcess, pCallReturnAddress, 0, MEM_RELEASE);

	if (iAreaPositionBase == 0)
		return;

	DWORD iAreaPositionX = m_Bot->Read4Byte(iAreaPositionBase + 0xC8) + 1;
	DWORD iAreaPositionY = m_Bot->Read4Byte(iAreaPositionBase + 0xC4) + 1;

	m_Bot->Write4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_UI_WND")), 14);
	m_Bot->Write4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_UI_SLOT_ORDER")), iSourcePosition - 14);
	m_Bot->Write4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_UI_WND_DISTRICT")), 10);
	m_Bot->Write4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_SELECTED_ITEM_BASE")), iItemBase);

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

	DWORD iItemBaseAddress = m_Bot->GetAddress(skCryptDec("KO_PTR_SELECTED_ITEM_BASE"));

	if (m_Bot->Read4Byte(iItemBaseAddress) == 0
		|| m_Bot->Read4Byte(iItemBaseAddress) != m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_SELECTED_ITEM_BASE"))))
		return;

	memcpy(byVipWarehouseGetInCode + 25, &iItemBaseAddress, sizeof(iItemBaseAddress));

	DWORD iVipWarehouseGetInCallAddress = m_Bot->GetAddress(skCryptDec("KO_PTR_VIP_ITEM_MOVE"));
	memcpy(byVipWarehouseGetInCode + 30, &iVipWarehouseGetInCallAddress, sizeof(iVipWarehouseGetInCallAddress));

	m_Bot->ExecuteRemoteCode(hProcess, byVipWarehouseGetInCode, sizeof(byVipWarehouseGetInCode));
}

void Client::VipWarehouseGetOut(DWORD iItemBase, int32_t iSourcePosition, int32_t iTargetPosition)
{
	if (iItemBase == 0)
		return;

	HANDLE hProcess = m_Bot->GetClientProcessHandle();

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

	DWORD iDlg = m_Bot->GetAddress(skCryptDec("KO_PTR_DLG"));
	memcpy(byAreaPositionCode + 3, &iDlg, sizeof(iDlg));

	DWORD iUIVipWarehouse = m_Bot->GetAddress(skCryptDec("KO_OFF_VIP"));
	memcpy(byAreaPositionCode + 9, &iUIVipWarehouse, sizeof(iUIVipWarehouse));

	memcpy(byAreaPositionCode + 14, &iTargetPosition, sizeof(iTargetPosition));

	DWORD iAreaType = 0x0000000B;
	memcpy(byAreaPositionCode + 19, &iAreaType, sizeof(iAreaType));

	DWORD iAreaPositionCallAddress = m_Bot->GetAddress(skCryptDec("KO_PTR_AREA_POSITION"));
	memcpy(byAreaPositionCode + 24, &iAreaPositionCallAddress, sizeof(iAreaPositionCallAddress));

	LPVOID pCallReturnAddress = VirtualAllocEx(hProcess, nullptr, sizeof(DWORD), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (!pCallReturnAddress)
		return;

	memcpy(byAreaPositionCode + 31, &pCallReturnAddress, sizeof(pCallReturnAddress));

	DWORD iAreaPositionBase = 0;
	SIZE_T bytesRead = 0;

	if (m_Bot->ExecuteRemoteCode(hProcess, byAreaPositionCode, sizeof(byAreaPositionCode)))
	{
		if (!ReadProcessMemory(hProcess, pCallReturnAddress, &iAreaPositionBase, sizeof(iAreaPositionBase), &bytesRead) && bytesRead == sizeof(iAreaPositionBase))
		{
			VirtualFreeEx(hProcess, pCallReturnAddress, 0, MEM_RELEASE);
			return;
		}
	}

	VirtualFreeEx(hProcess, pCallReturnAddress, 0, MEM_RELEASE);

	if (iAreaPositionBase == 0)
		return;

	DWORD iAreaPositionX = m_Bot->Read4Byte(iAreaPositionBase + 0xC8) + 1;
	DWORD iAreaPositionY = m_Bot->Read4Byte(iAreaPositionBase + 0xC4) + 1;

	m_Bot->Write4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_UI_WND")), 14);
	m_Bot->Write4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_UI_SLOT_ORDER")), iSourcePosition);
	m_Bot->Write4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_UI_WND_DISTRICT")), 2);
	m_Bot->Write4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_SELECTED_ITEM_BASE")), iItemBase);

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

	DWORD iItemBaseAddress = m_Bot->GetAddress(skCryptDec("KO_PTR_SELECTED_ITEM_BASE"));

	if (m_Bot->Read4Byte(iItemBaseAddress) == 0
		|| m_Bot->Read4Byte(iItemBaseAddress) != m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_SELECTED_ITEM_BASE"))))
		return;

	memcpy(byVipWarehouseGetOutCode + 25, &iItemBaseAddress, sizeof(iItemBaseAddress));

	DWORD iVipWarehouseGetOutCallAddress = m_Bot->GetAddress(skCryptDec("KO_PTR_VIP_ITEM_MOVE"));
	memcpy(byVipWarehouseGetOutCode + 30, &iVipWarehouseGetOutCallAddress, sizeof(iVipWarehouseGetOutCallAddress));

	m_Bot->ExecuteRemoteCode(hProcess, byVipWarehouseGetOutCode, sizeof(byVipWarehouseGetOutCode));
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

	DWORD iCountableItemDLG = m_Bot->GetAddress(skCryptDec("KO_PTR_COUNTABLE_DLG"));
	CopyBytes(byCode + 3, iCountableItemDLG);
	CopyBytes(byCode + 8, iCount);

	DWORD iCountableItemCallAddress = m_Bot->GetAddress(skCryptDec("KO_PTR_COUNTABLE_CHANGE"));
	CopyBytes(byCode + 13, iCountableItemCallAddress);

	HANDLE hProcess = m_Bot->GetClientProcessHandle();

	m_Bot->ExecuteRemoteCode(hProcess, byCode, sizeof(byCode));
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

	DWORD iDLG = m_Bot->GetAddress(skCryptDec("KO_PTR_DLG"));
	CopyBytes(byCode + 3, iDLG);

	DWORD iCUIVipWareHouse = m_Bot->GetAddress(skCryptDec("KO_OFF_VIP"));
	CopyBytes(byCode + 9, iCUIVipWareHouse);

	DWORD iCallAddress = m_Bot->GetAddress(skCryptDec("KO_PTR_COUNTABLE_ACCEPT"));
	CopyBytes(byCode + 14, iCallAddress);

	HANDLE hProcess = m_Bot->GetClientProcessHandle();

	m_Bot->ExecuteRemoteCode(hProcess, byCode, sizeof(byCode));
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
	DWORD iVipWarehouseBase = m_Bot->Read4Byte(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_DLG"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_VIP")));

	for (size_t i = 0; i < VIP_HAVE_MAX; i++)
	{
		TItemData pItem;
		memset(&pItem, 0, sizeof(pItem));

		pItem.iPos = i;

		DWORD iItemBase = m_Bot->Read4Byte(iVipWarehouseBase + (m_Bot->GetAddress(skCryptDec("KO_OFF_VIP_START")) + (4 * i)));

		if (m_Bot->GetPlatformType() == PlatformType::USKO 
			|| m_Bot->GetPlatformType() == PlatformType::STKO)
		{
			if (m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x60)) + m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x64)) != 0)
			{
				pItem.iBase = iItemBase;
				pItem.iItemID = m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x60)) + m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x64));
				pItem.iCount = (uint16_t)m_Bot->Read4Byte(iItemBase + 0x68);
				pItem.iDurability = (uint16_t)m_Bot->Read4Byte(iItemBase + 0x6C);
			}

			vecItemList.push_back(pItem);
		}
		else if (m_Bot->GetPlatformType() == PlatformType::CNKO)
		{
			if (m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x68)) + m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x6C)) != 0)
			{
				pItem.iBase = iItemBase;
				pItem.iItemID = m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x68)) + m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x6C));
				pItem.iCount = (uint16_t)m_Bot->Read4Byte(iItemBase + 0x70);
				pItem.iDurability = (uint16_t)m_Bot->Read4Byte(iItemBase + 0x74);
			}

			vecItemList.push_back(pItem);
		}
	}

	return vecItemList.size() > 0;
}

bool Client::GetVipWarehouseInventoryItemList(std::vector<TItemData>& vecItemList)
{
	DWORD iVipWarehouseBase = m_Bot->Read4Byte(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_DLG"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_VIP")));

	for (size_t i = 0; i < HAVE_MAX; i++)
	{
		TItemData pItem;
		memset(&pItem, 0, sizeof(pItem));

		pItem.iPos = i;

		DWORD iItemBase = m_Bot->Read4Byte(iVipWarehouseBase + (m_Bot->GetAddress(skCryptDec("KO_OFF_VIP_INVENTORY_START")) + (4 * i)));

		if (m_Bot->GetPlatformType() == PlatformType::USKO 
			|| m_Bot->GetPlatformType() == PlatformType::STKO)
		{
			if (m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x60)) + m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x64)) != 0)
			{
				pItem.iBase = iItemBase;
				pItem.iItemID = m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x60)) + m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x64));
				pItem.iCount = (uint16_t)m_Bot->Read4Byte(iItemBase + 0x68);
				pItem.iDurability = (uint16_t)m_Bot->Read4Byte(iItemBase + 0x6C);
			}

			vecItemList.push_back(pItem);
		}
		else if (m_Bot->GetPlatformType() == PlatformType::CNKO)
		{
			if (m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x68)) + m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x6C)) != 0)
			{
				pItem.iBase = iItemBase;
				pItem.iItemID = m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x68)) + m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x6C));
				pItem.iCount = (uint16_t)m_Bot->Read4Byte(iItemBase + 0x70);
				pItem.iDurability = (uint16_t)m_Bot->Read4Byte(iItemBase + 0x74);
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
	HANDLE hProcess = m_Bot->GetClientProcessHandle();

	if (bEnable)
	{
		BYTE byPatch[] =
		{
			0x75
		};

		WriteProcessMemory(hProcess, (LPVOID*)m_Bot->GetAddress(skCryptDec("KO_OBJECT_COLLISION_CHECK")), byPatch, sizeof(byPatch), 0);
	}
	else
	{
		BYTE byPatch[] =
		{
			0x74
		};

		WriteProcessMemory(hProcess, (LPVOID*)m_Bot->GetAddress(skCryptDec("KO_OBJECT_COLLISION_CHECK")), byPatch, sizeof(byPatch), 0);
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
	DWORD iVipWarehouseBase = m_Bot->Read4Byte(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_DLG"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_VIP")));

	for (size_t i = 0; i < VIP_HAVE_MAX; i++)
	{
		DWORD iItemBase = m_Bot->Read4Byte(iVipWarehouseBase + (m_Bot->GetAddress(skCryptDec("KO_OFF_VIP_START")) + (4 * i)));

		if (m_Bot->GetPlatformType() == PlatformType::USKO
			|| m_Bot->GetPlatformType() == PlatformType::STKO)
		{
			if (m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x60)) + m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x64)) == 0)
				return false;
		}
		else if (m_Bot->GetPlatformType() == PlatformType::CNKO)
		{
			if (m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x68)) + m_Bot->Read4Byte(m_Bot->Read4Byte(iItemBase + 0x6C)) == 0)
				return false;
		}
	}

	return true;
}

void Client::OpenVipWarehouse()
{
	m_bVipWarehouseLoaded = false;

	m_Bot->Write4Byte(m_Bot->Read4Byte(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_DLG"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_VIP"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_VIP_OPEN")), 1);

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

	DWORD iDLG = m_Bot->GetAddress(skCryptDec("KO_PTR_DLG"));
	CopyBytes(byCode + 3, iDLG);

	DWORD iCUIVipWareHouse = m_Bot->GetAddress(skCryptDec("KO_OFF_VIP"));
	CopyBytes(byCode + 9, iCUIVipWareHouse);

	DWORD iCallAddress = m_Bot->GetAddress(skCryptDec("KO_PTR_VIP_OPEN"));
	CopyBytes(byCode + 14, iCallAddress);

	HANDLE hProcess = m_Bot->GetClientProcessHandle();

	m_Bot->ExecuteRemoteCode(hProcess, byCode, sizeof(byCode));
}

void Client::CloseVipWarehouse()
{
	m_bVipWarehouseLoaded = false;

	m_Bot->Write4Byte(m_Bot->Read4Byte(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_DLG"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_VIP"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_VIP_OPEN")), 0);

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

	DWORD iDLG = m_Bot->GetAddress(skCryptDec("KO_PTR_DLG"));
	CopyBytes(byCode + 3, iDLG);

	DWORD iCUIVipWareHouse = m_Bot->GetAddress(skCryptDec("KO_OFF_VIP"));
	CopyBytes(byCode + 9, iCUIVipWareHouse);

	DWORD iCallAddress = m_Bot->GetAddress(skCryptDec("KO_PTR_VIP_CLOSE"));
	CopyBytes(byCode + 14, iCallAddress);

	HANDLE hProcess = m_Bot->GetClientProcessHandle();

	m_Bot->ExecuteRemoteCode(hProcess, byCode, sizeof(byCode));
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
	m_Bot->WriteFloat(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_SWIFT")), fSpeed);
}

float Client::GetCharacterSpeed()
{
	return m_Bot->ReadFloat(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_CHR"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_SWIFT")));
}

void Client::PatchSpeedHack(bool bEnable)
{
	HANDLE hProcess = m_Bot->GetClientProcessHandle();

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

		DWORD iCallDifference = Memory::GetDifference(m_Bot->GetAddress(skCryptDec("KO_PTR_MOVE_HOOK")), (DWORD)pPatchAddress);
		CopyBytes(byPatch2 + 1, iCallDifference);

		WriteProcessMemory(hProcess, (LPVOID*)m_Bot->GetAddress(skCryptDec("KO_PTR_MOVE_HOOK")), byPatch2, sizeof(byPatch2), 0);
	}
	else
	{
		BYTE byPatch[] =
		{
			0xE8, 0x00, 0x00, 0x00, 0x00
		};

		DWORD iCallDifference = Memory::GetDifference(m_Bot->GetAddress(skCryptDec("KO_PTR_MOVE_HOOK")), (DWORD)m_Bot->GetAddress(skCryptDec("KO_SND_FNC")));
		CopyBytes(byPatch + 1, iCallDifference);

		WriteProcessMemory(hProcess, (LPVOID*)m_Bot->GetAddress(skCryptDec("KO_PTR_MOVE_HOOK")), byPatch, sizeof(byPatch), 0);
	}
}

bool Client::IsVipWarehouseOpen()
{
	DWORD iVipWarehouseBase = m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_DLG"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_VIP"));
	DWORD iVipWarehouseOpenAddress = m_Bot->Read4Byte(iVipWarehouseBase) + 0xE4;
	return m_Bot->Read2Byte(iVipWarehouseOpenAddress) != 0;
}

bool Client::IsTransactionDialogOpen()
{
	DWORD iTransactionDlgBase = m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_DLG"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_TRANSACTION_DLG"));
	DWORD iiTransactionDlgOpenAddress = m_Bot->Read4Byte(iTransactionDlgBase) + 0xE4;
	return m_Bot->Read2Byte(iiTransactionDlgOpenAddress) != 0;
}

bool Client::IsWarehouseOpen()
{
	DWORD iTransactionDlgBase = m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_DLG"))) + 0x1F8;
	DWORD iiTransactionDlgOpenAddress = m_Bot->Read4Byte(iTransactionDlgBase) + 0xE4;
	return m_Bot->Read2Byte(iiTransactionDlgOpenAddress) != 0;
}

void Client::RemoveItem(int32_t iItemSlot)
{
	DWORD iInventoryBase = m_Bot->Read4Byte(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_DLG"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_INVENTORY_BASE")));
	DWORD iItemBase = m_Bot->Read4Byte(iInventoryBase + (m_Bot->GetAddress(skCryptDec("KO_OFF_INVENTORY_START")) + (4 * (iItemSlot + 14))));

	m_Bot->WriteByte(m_Bot->GetAddress(skCryptDec("KO_REMOVE_BASE1")), 1);
	m_Bot->Write4Byte(m_Bot->GetAddress(skCryptDec("KO_REMOVE_BASE1")) + 0x4, iItemSlot);
	m_Bot->Write4Byte(m_Bot->GetAddress(skCryptDec("KO_REMOVE_BASE1")) + 0x8, iItemBase);

	m_Bot->WriteByte(m_Bot->GetAddress(skCryptDec("KO_REMOVE_BASE2")), 1);
	m_Bot->Write4Byte(m_Bot->GetAddress(skCryptDec("KO_REMOVE_BASE2")) + 0x4, iItemBase);
	m_Bot->WriteByte(m_Bot->GetAddress(skCryptDec("KO_REMOVE_BASE2")) + 0x0C, 1);
	m_Bot->Write4Byte(m_Bot->GetAddress(skCryptDec("KO_REMOVE_BASE2")) + 0x10, iItemSlot);
	m_Bot->WriteByte(m_Bot->GetAddress(skCryptDec("KO_REMOVE_BASE2")) + 0x14, 1);

	HANDLE hProcess = m_Bot->GetClientProcessHandle();

	BYTE byPatch[] =
	{
		0x60,
		0xB9, 0x00, 0x00, 0x00, 0x00,
		0xBF, 0x00, 0x00, 0x00, 0x00,
		0xFF, 0xD7,
		0x61,
		0xC3,
	};

	memcpy(byPatch + 2, &iInventoryBase, sizeof(iInventoryBase));

	DWORD iCallAddress = m_Bot->GetAddress(skCryptDec("KO_REMOVE_CALL"));
	memcpy(byPatch + 7, &iCallAddress, sizeof(iCallAddress));

	m_Bot->ExecuteRemoteCode(hProcess, byPatch, sizeof(byPatch));
}

void Client::HidePlayer(bool bHide)
{
	m_Bot->WriteByte(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_DLG"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_HIDE")), bHide ? 0 : 1);
}

void Client::VipGetInTest()
{
	new std::thread([=]()
	{
		if (m_bVipWarehouseInitialized
			&& !m_bVipWarehouseEnabled)
			return;

		std::vector<TItemData> vecInventoryItemList;
		std::vector<TItemData> vecFlaggedItemList;

		GetInventoryItemList(vecInventoryItemList);

		for (const TItemData& pItem : vecInventoryItemList)
		{
			if (pItem.iItemID == 0)
				continue;

			vecFlaggedItemList.push_back(pItem);
		}

		OpenVipWarehouse();

		std::this_thread::sleep_for(std::chrono::milliseconds(1500));

		WaitConditionWithTimeout(m_bVipWarehouseLoaded == false, 3000);

		if (!m_bVipWarehouseLoaded)
			return;

		for (const TItemData& pItem : vecFlaggedItemList)
		{
			__TABLE_ITEM* pItemData;
			if (!m_Bot->GetItemData(pItem.iItemID, pItemData))
				continue;

			if (pItemData->byNeedRace == RACE_NO_TRADE_SOLD_STORE)
				continue;

			WaitConditionWithTimeout(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_UI_LOCK"))) == 1, 3000);

			std::vector<TItemData> vecVipWarehouseItemList;
			GetVipWarehouseItemList(vecVipWarehouseItemList);

			int iTargetPosition = -1;

			if (pItemData->byContable)
			{
				auto pWarehouseItem = std::find_if(vecVipWarehouseItemList.begin(), vecVipWarehouseItemList.end(),
					[&](const TItemData& a)
					{
						return a.iItemID == pItem.iItemID;
					});

				if (pWarehouseItem != vecVipWarehouseItemList.end())
				{
					if ((pItem.iCount + pWarehouseItem->iCount) > 9999)
						continue;

					iTargetPosition = pWarehouseItem->iPos;
				}
				else
				{
					auto pWarehouseEmptySlot = std::find_if(vecVipWarehouseItemList.begin(), vecVipWarehouseItemList.end(),
						[&](const TItemData& a)
						{
							return a.iItemID == 0;
						});

					if (pWarehouseEmptySlot != vecVipWarehouseItemList.end())
					{
						iTargetPosition = pWarehouseEmptySlot->iPos;
					}
				}
			}
			else
			{
				auto pWarehouseEmptySlot = std::find_if(vecVipWarehouseItemList.begin(), vecVipWarehouseItemList.end(),
					[&](const TItemData& a)
					{
						return a.iItemID == 0;
					});

				if (pWarehouseEmptySlot != vecVipWarehouseItemList.end())
				{
					iTargetPosition = pWarehouseEmptySlot->iPos;
				}
			}

			if (iTargetPosition == -1)
				break;

			WaitConditionWithTimeout(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_UI_LOCK"))) == 1, 3000);

			VipWarehouseGetIn(pItem.iBase, pItem.iPos, iTargetPosition);

			if (pItemData->byContable)
			{
				CountableDialogChangeCount(1);
				CountableDialogChangeCount(31);
				AcceptCountableDialog();
			}
		}

		WaitConditionWithTimeout(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_UI_LOCK"))) == 1, 3000);

		std::this_thread::sleep_for(std::chrono::milliseconds(1500));

		CloseVipWarehouse();

		std::this_thread::sleep_for(std::chrono::milliseconds(1500));
	});
}

void Client::VipGetOutTest()
{
	new std::thread([=]()
	{ 
		if (m_bVipWarehouseInitialized
			&& !m_bVipWarehouseEnabled)
			return;

		OpenVipWarehouse();

		std::this_thread::sleep_for(std::chrono::milliseconds(1500));

		WaitConditionWithTimeout(m_bVipWarehouseLoaded == false, 3000);

		if (!m_bVipWarehouseLoaded)
			return;

		std::vector<TItemData> vecVipWarehouseItemList;
		std::vector<TItemData> vecFlaggedItemList;

		GetVipWarehouseItemList(vecVipWarehouseItemList);

		for (const TItemData& pItem : vecVipWarehouseItemList)
		{
			if (pItem.iItemID == 0)
				continue;

			vecFlaggedItemList.push_back(pItem);
		}

		if (vecFlaggedItemList.size() == 0)
			return;

		for (const TItemData& pItem : vecFlaggedItemList)
		{
			__TABLE_ITEM* pItemData;
			if (!m_Bot->GetItemData(pItem.iItemID, pItemData))
				continue;

			WaitConditionWithTimeout(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_UI_LOCK"))) == 1, 3000);

			std::vector<TItemData> vecVipWarehouseInventoryItemList;
			GetVipWarehouseInventoryItemList(vecVipWarehouseInventoryItemList);

			int iTargetPosition = -1;

			if (pItemData->byContable)
			{
				auto pInventoryItem = std::find_if(vecVipWarehouseInventoryItemList.begin(), vecVipWarehouseInventoryItemList.end(),
					[&](const TItemData& a)
					{
						return a.iItemID == pItem.iItemID;
					});

				if (pInventoryItem != vecVipWarehouseInventoryItemList.end())
				{
					if ((pItem.iCount + pInventoryItem->iCount) > 9999)
						continue;

					iTargetPosition = pInventoryItem->iPos;
				}
				else
				{
					auto pInventoryEmptySlot = std::find_if(vecVipWarehouseInventoryItemList.begin(), vecVipWarehouseInventoryItemList.end(),
						[&](const TItemData& a)
						{
							return a.iItemID == 0;
						});

					if (pInventoryEmptySlot != vecVipWarehouseInventoryItemList.end())
					{
						iTargetPosition = pInventoryEmptySlot->iPos;
					}
				}
			}
			else
			{
				auto pInventoryEmptySlot = std::find_if(vecVipWarehouseInventoryItemList.begin(), vecVipWarehouseInventoryItemList.end(),
					[&](const TItemData& a)
					{
						return a.iItemID == 0;
					});

				if (pInventoryEmptySlot != vecVipWarehouseInventoryItemList.end())
				{
					iTargetPosition = pInventoryEmptySlot->iPos;
				}
			}

			if (iTargetPosition == -1)
			{
				return;
			}

			WaitConditionWithTimeout(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_UI_LOCK"))) == 1, 3000);

			VipWarehouseGetOut(pItem.iBase, pItem.iPos, iTargetPosition);

			if (pItemData->byContable)
			{
				CountableDialogChangeCount(1);
				CountableDialogChangeCount(31);
				AcceptCountableDialog();
			}
		}

		WaitConditionWithTimeout(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_UI_LOCK"))) == 1, 3000);

		std::this_thread::sleep_for(std::chrono::milliseconds(1500));

		CloseVipWarehouse();

		std::this_thread::sleep_for(std::chrono::milliseconds(1500));
	});
}

void Client::Legalize(DWORD iItemBase, int32_t iSourcePosition, int32_t iTargetPosition)
{
	if (iItemBase == 0)
		return;

	EquipItem(iItemBase, iSourcePosition, iTargetPosition);
}

void Client::EquipItem(DWORD iItemBase, int32_t iSourcePosition, int32_t iTargetPosition)
{
	if (iItemBase == 0)
		return;

	HANDLE hProcess = m_Bot->GetClientProcessHandle();

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

	DWORD iDlg = m_Bot->GetAddress(skCryptDec("KO_PTR_DLG"));
	memcpy(byAreaPositionCode + 3, &iDlg, sizeof(iDlg));

	DWORD iInventoryBase = m_Bot->GetAddress(skCryptDec("KO_OFF_INVENTORY_BASE"));
	memcpy(byAreaPositionCode + 9, &iInventoryBase, sizeof(iInventoryBase));

	memcpy(byAreaPositionCode + 14, &iTargetPosition, sizeof(iTargetPosition));

	DWORD iAreaType = 0x00000001; //UI_AREA_TYPE_SLOT
	memcpy(byAreaPositionCode + 19, &iAreaType, sizeof(iAreaType));

	DWORD iAreaPositionCallAddress = m_Bot->GetAddress(skCryptDec("KO_PTR_AREA_POSITION"));
	memcpy(byAreaPositionCode + 24, &iAreaPositionCallAddress, sizeof(iAreaPositionCallAddress));

	LPVOID pCallReturnAddress = VirtualAllocEx(hProcess, nullptr, sizeof(DWORD), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (!pCallReturnAddress)
		return;

	memcpy(byAreaPositionCode + 31, &pCallReturnAddress, sizeof(pCallReturnAddress));

	DWORD iAreaPositionBase = 0;
	SIZE_T bytesRead = 0;

	if (m_Bot->ExecuteRemoteCode(hProcess, byAreaPositionCode, sizeof(byAreaPositionCode)))
	{
		if (!ReadProcessMemory(hProcess, pCallReturnAddress, &iAreaPositionBase, sizeof(iAreaPositionBase), &bytesRead) && bytesRead == sizeof(iAreaPositionBase))
		{
			VirtualFreeEx(hProcess, pCallReturnAddress, 0, MEM_RELEASE);
			return;
		}
	}

	if (iAreaPositionBase == 0)
	{
		VirtualFreeEx(hProcess, pCallReturnAddress, 0, MEM_RELEASE);
		return;
	}

	LPVOID pMouseInputPatchAddress = VirtualAllocEx(hProcess, nullptr, sizeof(DWORD), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (!pMouseInputPatchAddress)
		return;

	BYTE byMouseInputPatch[] =
	{
		0xB8, 0x00, 0x00, 0x00, 0x00,
		0x8B, 0x88, 0xC4, 0x00, 0x00, 0x00,
		0x8B, 0x80, 0xC8, 0x00, 0x00, 0x00,
		0xE9, 0x00, 0x00, 0x00, 0x00,
	};

	memcpy(byMouseInputPatch + 1, &iAreaPositionBase, sizeof(iAreaPositionBase));

	DWORD iMouseInputHookAddress = Memory::GetDifference((DWORD)pMouseInputPatchAddress + 17, m_Bot->GetAddress(skCryptDec("KO_PTR_INVENTORY_MOUSE_HOOK")) + 11);
	memcpy(byMouseInputPatch + 18, &iMouseInputHookAddress, sizeof(iMouseInputHookAddress));

	WriteProcessMemory(hProcess, pMouseInputPatchAddress, &byMouseInputPatch[0], sizeof(byMouseInputPatch), 0);

	BYTE byMouseInputHookPatch[] =
	{
		0xE9, 0x00, 0x00, 0x00, 0x00
	};

	DWORD iCallDifference = Memory::GetDifference(m_Bot->GetAddress(skCryptDec("KO_PTR_INVENTORY_MOUSE_HOOK")), (DWORD)pMouseInputPatchAddress);
	memcpy(byMouseInputHookPatch + 1, &iCallDifference, sizeof(iCallDifference));

	std::vector<uint8_t> vecBackupOriginalFunction;
	vecBackupOriginalFunction = m_Bot->ReadBytes(m_Bot->GetAddress(skCryptDec("KO_PTR_INVENTORY_MOUSE_HOOK")), 5);

	WriteProcessMemory(hProcess, (LPVOID*)m_Bot->GetAddress(skCryptDec("KO_PTR_INVENTORY_MOUSE_HOOK")), byMouseInputHookPatch, sizeof(byMouseInputHookPatch), 0);

	m_Bot->Write4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_UI_WND")), 0);
	m_Bot->Write4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_UI_SLOT_ORDER")), iSourcePosition);
	m_Bot->Write4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_UI_WND_DISTRICT")), 1);
	m_Bot->Write4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_SELECTED_ITEM_BASE")), iItemBase);

	BYTE byCallPatch[] =
	{
		0x60,
		0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00,
		0x8B, 0x89, 0x00, 0x00, 0x00, 0x00,
		0xFF, 0x35, 0x00, 0x00, 0x00, 0x00,
		0xBF, 0x00, 0x00, 0x00, 0x00,
		0xFF, 0xD7,
		0x61,
		0xC3,
	};

	memcpy(byCallPatch + 3, &iDlg, sizeof(iDlg));
	memcpy(byCallPatch + 9, &iInventoryBase, sizeof(iInventoryBase));

	DWORD iItemBaseAddress = m_Bot->GetAddress(skCryptDec("KO_PTR_SELECTED_ITEM_BASE"));

	if (m_Bot->Read4Byte(iItemBaseAddress) == 0
		|| m_Bot->Read4Byte(iItemBaseAddress) != m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_SELECTED_ITEM_BASE"))))
		return;

	memcpy(byCallPatch + 15, &iItemBaseAddress, sizeof(iItemBaseAddress));

	DWORD iCallAddress = m_Bot->GetAddress(skCryptDec("KO_OFF_INVENTORY_ITEM_MOVE"));
	memcpy(byCallPatch + 20, &iCallAddress, sizeof(iCallAddress));

	m_Bot->ExecuteRemoteCode(hProcess, byCallPatch, sizeof(byCallPatch));

	WriteProcessMemory(hProcess, (LPVOID*)m_Bot->GetAddress(skCryptDec("KO_PTR_INVENTORY_MOUSE_HOOK")), &vecBackupOriginalFunction[0], vecBackupOriginalFunction.size(), 0);

	VirtualFreeEx(hProcess, pMouseInputPatchAddress, 0, MEM_RELEASE);
	VirtualFreeEx(hProcess, pCallReturnAddress, 0, MEM_RELEASE);
}

bool Client::IsSkillHasZoneLimit(uint32_t iSkillBaseID)
{
	int iZoneIndex = m_PlayerMySelf.iCity;

	switch (iSkillBaseID)
	{
	case 490803:
	case 490811:
	case 490808:
	case 490809:
	case 490810:
	case 490800:
	case 490801:
	case 490817:
	{
		if (iZoneIndex == ZONE_DRAKI_TOWER ||
			iZoneIndex == ZONE_MONSTER_STONE1 ||
			iZoneIndex == ZONE_MONSTER_STONE2 ||
			iZoneIndex == ZONE_MONSTER_STONE3)
			return true;
	}
	break;
	}

	return false;
}