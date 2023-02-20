#include "pch.h"
#include "Ini.h"
#include "Client.h"
#include "Memory.h"
#include "AttackHandler.h"
#include "CharacterHandler.h"
#include "ProtectionHandler.h"
#include "AutoLootHandler.h"
#include "Bootstrap.h"

Client::Client()
{
#ifdef _DEBUG
	printf("Client::Initialize\n");
#endif

	m_mapAddressList.clear();

	m_byState = BOOTSTRAP;
	m_bWorking = false;

	m_dwRecvHookAddress = 0;
	m_dwSendHookAddress = 0;

	memset(&m_PlayerMySelf, 0, sizeof(m_PlayerMySelf));

	m_vecNpc.clear();
	m_vecPlayer.clear();

	m_iTargetID = -1;

	m_mapActiveBuffList.clear();
	m_mapSkillUseTime.clear();

	m_bLunarWarDressUp = false;

	m_vecAvailableSkill.clear();

	m_vecLootList.clear();

	m_bIsMovingToLoot = false;

#ifdef _DEBUG
	printf("Client::Initialized\n");
#endif
}

Client::~Client()
{
#ifdef _DEBUG
	printf("Client::Destroy\n");
#endif

	Stop();

	m_mapAddressList.clear();

	m_byState = LOST;
	m_bWorking = false;

	m_dwRecvHookAddress = 0;
	m_dwSendHookAddress = 0;

	memset(&m_PlayerMySelf, 0, sizeof(m_PlayerMySelf));

	m_vecNpc.clear();
	m_vecPlayer.clear();

	m_iTargetID = -1;

	m_mapActiveBuffList.clear();
	m_mapSkillUseTime.clear();

	m_bLunarWarDressUp = false;

	m_vecAvailableSkill.clear();

	m_vecLootList.clear();

#ifdef _DEBUG
	printf("Client::Destroyed\n");
#endif
}

void Client::Start()
{
#ifdef _DEBUG
	printf("Client::Starting\n");
#endif

	m_bWorking = true;

	new std::thread([]() { MainProcess(); });
	new std::thread([]() { HookProcess(); });

#ifdef _DEBUG
	printf("Client::Started\n");
#endif
}

void Client::Stop()
{
#ifdef _DEBUG
	printf("Client::Stoping\n");
#endif

	m_bWorking = false;

	StopHandler();

#ifdef _DEBUG
	printf("Client::Stopped\n");
#endif
}

void Client::StartHandler()
{
#ifdef _DEBUG
	printf("Client::StartHandler\n");
#endif

	AttackHandler::Start();
	CharacterHandler::Start();
	ProtectionHandler::Start();
	AutoLootHandler::Start();
}

void Client::StopHandler()
{
#ifdef _DEBUG
	printf("Client::StopHandler\n");
#endif

	AttackHandler::Stop();
	CharacterHandler::Stop();
	ProtectionHandler::Stop();
	AutoLootHandler::Stop();
}

void Client::LoadUserConfig(std::string strCharacterName)
{
	auto it = m_mapUserConfig.find(strCharacterName);

	if (it == m_mapUserConfig.end())
	{
		std::stringstream strUserConfigFile;
		strUserConfigFile << ".\\" << Client::GetName().c_str() << ".ini";

		Ini* pIni = new Ini(strUserConfigFile.str().c_str());

		m_mapUserConfig.insert(std::pair<std::string, Ini*>(Client::GetName(), pIni));
	}

#ifdef _DEBUG
	printf("LoadUserConfig: %s loaded\n", Client::GetName().c_str());
#endif
}


Ini* Client::GetUserConfig(std::string strCharacterName)
{
	auto it = m_mapUserConfig.find(strCharacterName);

	if (it == m_mapUserConfig.end())
		return NULL;

	return it->second;
}

void Client::HookProcess()
{
#ifdef _DEBUG
	printf("Client::HookProcess\n");
#endif

	while (m_bWorking)
	{
		if (GetState() == State::GAME)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		if (Memory::Read4Byte(Client::GetRecvAddress()) != GetRecvHookAddress())
			HookRecvAddress();
	}
}

void Client::MainProcess()
{
#ifdef _DEBUG
	printf("Client::MainProcess\n");
#endif

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	HookSendAddress();

	while (m_bWorking)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		switch (GetState())
		{
			case State::BOOTSTRAP: BootstrapProcess(); break;
			case State::GAME: GameProcess(); break;
		}
	}
}

void Client::BootstrapProcess()
{
	while (!Client::IsIntroPhase())
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

	Client::PushPhase(Client::GetAddress("KO_PTR_LOGIN"));

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	while (!Client::IsLoginPhase())
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	Client::SetLoginInformation("colinkazim", "ttCnkoSh1993");
	Client::ConnectLoginServer();

	Client::SetState(State::LOGIN);
}

void Client::GameProcess()
{
	std::string strCharacterName = GetName();

	if (Client::IsDisconnect())
	{
		printf("%s Connection lost\n", strCharacterName.c_str());
		Client::SetState(State::LOST);
	}
}

void Client::SetAddress(std::string strAddressName, DWORD dwAddress)
{
	m_mapAddressList.insert({ strAddressName, dwAddress });
}

DWORD Client::GetAddress(std::string strAddressName)
{
	auto it = m_mapAddressList.find(strAddressName);

	if (it == m_mapAddressList.end())
		return 0;

	return it->second;
}

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

bool Client::IsIntroPhase()
{
	return Memory::Read4Byte(GetAddress("KO_PTR_INTRO")) != 0;
};

bool Client::IsLoginPhase()
{
	return Memory::Read4Byte(Memory::Read4Byte(GetAddress("KO_PTR_LOGIN")) + GetAddress("KO_OFF_UI_LOGIN_INTRO")) != 0;
};

bool Client::IsDisconnect()
{
	return Memory::Read4Byte(Memory::Read4Byte(GetAddress("KO_PTR_PKT")) + GetAddress("KO_OFF_DISCONNECT")) == 0;
};

DWORD Client::GetRecvAddress()
{
	return Memory::Read4Byte(Memory::Read4Byte(GetAddress("KO_PTR_DLG"))) + 0x8;
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
	return m_PlayerMySelf.iHP;
}

int16_t Client::GetMaxHp()
{
	return m_PlayerMySelf.iHPMax;
}

int16_t Client::GetMp()
{
	return m_PlayerMySelf.iMSP;
}

int16_t Client::GetMaxMp()
{
	return m_PlayerMySelf.iMSPMax;
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

e_Nation Client::GetNation()
{
	return m_PlayerMySelf.eNation;
}

e_Class Client::GetClass()
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

uint8_t Client::GetSkillPoint(int32_t Slot)
{
	return m_PlayerMySelf.iSkillInfo[Slot];
}

bool Client::IsBlinking() 
{
	return m_PlayerMySelf.bBlinking;
};

void Client::SetTarget(int32_t iTargetID)
{
	m_iTargetID = iTargetID;
}

int32_t Client::GetTarget()
{
	return m_iTargetID;
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
			[](const TNpc& a) { return a.iID == m_iTargetID; });

		if (it != m_vecNpc.end())
			return Vector3(it->fX, it->fZ, it->fY);
	}
	else
	{
		auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
			[](const TPlayer& a) { return a.iID == m_iTargetID; });

		if (it != m_vecPlayer.end())
			return Vector3(it->fX, it->fZ, it->fY);
	}

	return Vector3(0.0f, 0.0f, 0.0f);
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

void Client::LoadSkillData()
{
#ifdef _DEBUG
	printf("Client::LoadSkillData: Start Load Character Skill Data\n");
#endif

	m_vecAvailableSkill.clear();

	auto pSkillList = Bootstrap::GetSkillTable().GetData();

	for (const auto& [key, value] : pSkillList)
	{
		if (0 != std::to_string(value.iNeedSkill).substr(0, 3).compare(std::to_string(Client::GetClass())))
			continue;

		if (value.iTarget != SkillTargetType::TARGET_SELF && value.iTarget != SkillTargetType::TARGET_PARTY_ALL && value.iTarget != SkillTargetType::TARGET_FRIEND_WITHME &&
			value.iTarget != SkillTargetType::TARGET_ENEMY_ONLY && value.iTarget != SkillTargetType::TARGET_AREA_ENEMY)
			continue;

		if ((value.iSelfAnimID1 == 153 || value.iSelfAnimID1 == 154) || (value.iSelfFX1 == 32038 || value.iSelfFX1 == 32039))
			continue;

		switch (value.iNeedSkill % 10)
		{
		case 0:
			if (value.iNeedLevel > Client::GetLevel())
				continue;
			break;
		case 5:
			if (value.iNeedLevel > Client::GetSkillPoint(5))
				continue;
			break;
		case 6:
			if (value.iNeedLevel > Client::GetSkillPoint(6))
				continue;
			break;
		case 7:
			if (value.iNeedLevel > Client::GetSkillPoint(7))
				continue;
			break;
		case 8:
			if (value.iNeedLevel > Client::GetSkillPoint(8))
				continue;
			break;
		}

		m_vecAvailableSkill.push_back(value);
	}
}

bool Client::UseItem(uint32_t iItemID)
{
	auto pItemTable = Bootstrap::GetItemTable().GetData();
	auto pSkillTable = Bootstrap::GetSkillTable().GetData();

	auto pItemData = pItemTable.find(iItemID);

	if (pItemData != pItemTable.end())
	{
		auto pSkillData = pSkillTable.find(pItemData->second.dwEffectID1);

		if (pSkillData != pSkillTable.end())
		{
			std::chrono::milliseconds msNow = duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch()
				);

			std::chrono::milliseconds msLastSkillUseItem = Client::GetSkillUseTime(pSkillData->second.iID);

			if (pSkillData->second.iCooldown > 0 && msLastSkillUseItem.count() > 0)
			{
				int64_t iSkillCooldownTime = static_cast<int64_t>(pSkillData->second.iCooldown) * 100;

				if ((msLastSkillUseItem.count() + iSkillCooldownTime) > msNow.count())
					return false;
			}

			UseSkill(pSkillData->second, GetID());

			return true;
		}
	}

	return false;
}

DWORD Client::GetRecvHookAddress()
{
	return m_dwRecvHookAddress;
};

DWORD Client::GetSendHookAddress()
{
	return m_dwSendHookAddress;
};

void Client::SetPosition(Vector3 v3Position)
{
	Memory::WriteFloat(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_X"), v3Position.m_fX);
	Memory::WriteFloat(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_Y"), v3Position.m_fY);
	Memory::WriteFloat(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_Z"), v3Position.m_fZ);
}

void Client::SetMovePosition(Vector3 v3MovePosition)
{
	Memory::WriteByte(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_MOVE_TYPE"), 2);
	Memory::WriteFloat(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_GOX"), v3MovePosition.m_fX);
	Memory::WriteFloat(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_GOY"), v3MovePosition.m_fY);
	Memory::WriteFloat(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_GOZ"), v3MovePosition.m_fZ);
	Memory::WriteByte(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_MOVE"), 1);
}

void Client::SetAuthority(uint8_t iAuthority)
{
	Memory::WriteByte(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_AUTHORITY"), iAuthority);
}

void Client::PushPhase(DWORD address)
{
	PushPhaseFunction pPushPhaseFunction = (PushPhaseFunction)GetAddress("KO_PTR_PUSH_PHASE");
	pPushPhaseFunction(*reinterpret_cast<int*>(address));
}

void Client::SetLoginInformation(std::string strAccountId, std::string strAccountPassword)
{
	DWORD dwCGameProcIntroLogin = Memory::Read4Byte(GetAddress("KO_PTR_LOGIN"));
	DWORD dwCUILoginIntro = Memory::Read4Byte(dwCGameProcIntroLogin + GetAddress("KO_OFF_UI_LOGIN_INTRO"));

	DWORD dwCN3UIEditIdBase = Memory::Read4Byte(dwCUILoginIntro + GetAddress("KO_OFF_UI_LOGIN_INTRO_ID"));
	DWORD dwCN3UIEditPwBase = Memory::Read4Byte(dwCUILoginIntro + GetAddress("KO_OFF_UI_LOGIN_INTRO_PW"));

	Memory::WriteString(dwCN3UIEditIdBase + GetAddress("KO_OFF_UI_LOGIN_INTRO_ID_INPUT"), strAccountId.c_str());
	Memory::Write4Byte(dwCN3UIEditIdBase + GetAddress("KO_OFF_UI_LOGIN_INTRO_ID_INPUT_LENGTH"), strAccountId.size());
	Memory::WriteString(dwCN3UIEditPwBase + GetAddress("KO_OFF_UI_LOGIN_INTRO_PW_INPUT"), strAccountPassword.c_str());
	Memory::Write4Byte(dwCN3UIEditPwBase + GetAddress("KO_OFF_UI_LOGIN_INTRO_PW_INPUT_LENGTH"), strAccountPassword.size());
}

void Client::ConnectLoginServer(bool bDisconnect)
{
	if (bDisconnect)
	{
		DisconnectFunction pDisconnectFunction = (DisconnectFunction)GetAddress("KO_PTR_LOGIN_DC");
		pDisconnectFunction(*reinterpret_cast<DWORD*>(GetAddress("KO_PTR_LOGIN")));
	}

	Login1Function pLogin1Function = (Login1Function)GetAddress("KO_PTR_LOGIN1");
	pLogin1Function(*reinterpret_cast<DWORD*>(GetAddress("KO_PTR_LOGIN")));

	Login2Function pLogin2Function = (Login2Function)GetAddress("KO_PTR_LOGIN2");
	pLogin2Function(*reinterpret_cast<DWORD*>(GetAddress("KO_PTR_LOGIN")));
}

void Client::ConnectGameServer(BYTE byServerId)
{
	DWORD dwCGameProcIntroLogin = Memory::Read4Byte(GetAddress("KO_PTR_LOGIN"));
	DWORD dwCUILoginIntro = Memory::Read4Byte(dwCGameProcIntroLogin + GetAddress("KO_OFF_UI_LOGIN_INTRO"));

	Memory::Write4Byte(dwCUILoginIntro + GetAddress("KO_OFF_LOGIN_SERVER_INDEX"), byServerId);

	LoginServerFunction pLoginServerFunction = (LoginServerFunction)GetAddress("KO_PTR_SERVER_SELECT");
	pLoginServerFunction(*reinterpret_cast<DWORD*>(GetAddress("KO_PTR_LOGIN")));
}

void Client::SelectCharacterSkip()
{
	CharacterSelectSkipFunction pCharacterSelectEnterFunction = (CharacterSelectSkipFunction)GetAddress("KO_PTR_CHARACTER_SELECT_SKIP");
	pCharacterSelectEnterFunction(*reinterpret_cast<DWORD*>(GetAddress("KO_PTR_CHARACTER_SELECT")));
}

void Client::SelectCharacter(BYTE byCharacterIndex)
{
	CharacterSelectFunction pCharacterSelectFunction = (CharacterSelectFunction)GetAddress("KO_PTR_CHARACTER_SELECT_ENTER");
	pCharacterSelectFunction(*reinterpret_cast<DWORD*>(GetAddress("KO_PTR_CHARACTER_SELECT")));
}

void Client::RouteStart(Vector3 vec3MovePosition)
{
	RouteStartFunction pRouteStartFunction = (RouteStartFunction)GetAddress("KO_PTR_ROUTE_START_CALL");
	pRouteStartFunction(*reinterpret_cast<int*>(GetAddress("KO_PTR_CHR")), &vec3MovePosition);
}

void Client::EquipOreads(int32_t iItemID)
{
	EquipOreadsFunction pEquipOreadsFunction = (EquipOreadsFunction)GetAddress("KO_PTR_EQUIP_ITEM");
	pEquipOreadsFunction(*reinterpret_cast<int*>(GetAddress("KO_PTR_CHR")), iItemID, 0);
}

void Client::SetOreads(bool bValue)
{
	Memory::Write4Byte(Memory::Read4Byte(0xF7F35C) + GetAddress("KO_OFF_LOOT"), bValue ? 1 : 0);
}

void Client::SendPacket(Packet vecBuffer)
{
	SendFunction pSendFunction = (SendFunction)GetAddress("KO_SND_FNC");
	pSendFunction(*reinterpret_cast<DWORD*>(GetAddress("KO_PTR_PKT")), vecBuffer.contents(), vecBuffer.size());
}

void Client::HookRecvAddress()
{
	m_dwRecvHookAddress = (DWORD)VirtualAllocEx(GetCurrentProcess(), (LPVOID*)GetRecvHookAddress(), sizeof(int), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	BYTE byPatch[] =
	{
		0x55,									//push ebp
		0x8B, 0xEC,								//mov ebp,esp
		0x83, 0xC4, 0xF8,						//add esp,-08
		0x53,									//push ebx
		0x8B, 0x45, 0x08,						//mov eax,[ebp+08]
		0x83, 0xC0, 0x04,						//add eax,04
		0x8B, 0x10,								//mov edx,[eax]
		0x89, 0x55, 0xFC,						//mov [ebp-04],edx
		0x8B, 0x4D, 0x08,						//mov ecx,[ebp+08]
		0x83, 0xC1, 0x08,						//add ecx,08
		0x8B, 0x01,								//mov eax,[ecx]
		0x89, 0x45, 0xF8,						//mov [ebp-08],eax
		0xFF, 0x75, 0xFC,						//push [ebp-04]
		0xFF, 0x75, 0xF8,						//push [ebp-08]
		0xB8, 0x00, 0x00, 0x00, 0x00,			//mov eax,00000000 <-- ClientProcessor::RecvProcess()
		0xFF, 0xD0,								//call eax
		0x83, 0xC4, 0x08,						//add esp,08
		0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00,		//mov ecx,[00000000] <-- KO_PTR_DLG
		0xFF, 0x75, 0x0C,						//push [ebp+0C]
		0xFF, 0x75, 0x08,						//push [ebp+08]
		0xB8, 0x00, 0x00, 0x00, 0x00,			//mov eax,00000000 <-- GetRecvCallAddress()
		0xFF, 0xD0,								//call eax
		0x5B,									//pop ebx
		0x59,									//pop ecx
		0x59,									//pop ecx
		0x5D,									//pop ebp
		0xC2, 0x08, 0x00						//ret 0008
	};

	DWORD dwRecvProcessFunction = (DWORD)(LPVOID*)RecvProcess;
	CopyBytes(byPatch + 36, dwRecvProcessFunction);

	DWORD dwDlgAddress = GetAddress("KO_PTR_DLG");
	CopyBytes(byPatch + 47, dwDlgAddress);

	DWORD dwRecvCallAddress = Memory::Read4Byte(GetRecvAddress());
	CopyBytes(byPatch + 58, dwRecvCallAddress);

	std::vector<BYTE> vecPatch(byPatch, byPatch + sizeof(byPatch));

	Memory::WriteBytes(GetRecvHookAddress(), vecPatch);

	DWORD oldProtection;
	VirtualProtect((LPVOID)GetRecvAddress(), 1, PAGE_EXECUTE_READWRITE, &oldProtection);
	Memory::Write4Byte(GetRecvAddress(), GetRecvHookAddress());
	VirtualProtect((LPVOID*)GetRecvAddress(), 1, oldProtection, &oldProtection);

#ifdef _DEBUG
	printf("HookRecvAddress: Patched\n");
#endif
}

void Client::HookSendAddress()
{
	m_dwSendHookAddress = (DWORD)VirtualAllocEx(GetCurrentProcess(), (LPVOID*)GetSendHookAddress(), sizeof(int), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	BYTE byPatch1[] =
	{
		0x55,										//push ebp
		0x8B, 0xEC,									//mov ebp,esp 
		0x60,										//pushad
		0xFF, 0x75, 0x0C,							//push [ebp+0C]
		0xFF, 0x75, 0x08,							//push [ebp+08]
		0xBA, 0x00, 0x00, 0x00, 0x00,				//mov edx,00000000 <-- ClientProcessor::SendProcess()
		0xFF, 0xD2,									//call edx
		0x5E,										//pop esi
		0x5D,										//pop ebp
		0x61,										//popad
		0x6A, 0xFF,									//push-01
		0xBA, 0x00, 0x00, 0x00, 0x00,				//mov edx,00000000 <-- KO_SND_FNC
		0x83, 0xC2, 0x5,							//add edx,05
		0xFF, 0xE2									//jmp edx
	};

	DWORD dwSendProcessFunction = (DWORD)(LPVOID*)SendProcess;
	CopyBytes(byPatch1 + 11, dwSendProcessFunction);

	DWORD dwKoPtrSndFnc = GetAddress("KO_SND_FNC");
	CopyBytes(byPatch1 + 23, dwKoPtrSndFnc);

	std::vector<BYTE> vecPatch1(byPatch1, byPatch1 + sizeof(byPatch1));
	Memory::WriteBytes(GetSendHookAddress(), vecPatch1);

	BYTE byPatch2[] =
	{
		0xE9, 0x00, 0x00, 0x00, 0x00,
	};

	DWORD dwCallDifference = Memory::GetDifference(GetAddress("KO_SND_FNC"), GetSendHookAddress());
	CopyBytes(byPatch2 + 1, dwCallDifference);

	std::vector<BYTE> vecPatch2(byPatch2, byPatch2 + sizeof(byPatch2));
	Memory::WriteBytes(GetAddress("KO_SND_FNC"), vecPatch2);

#ifdef _DEBUG
	printf("HookSendAddress: Patched\n");
#endif
}

TNpc Client::InitializeNpc(Packet& pkt)
{
	TNpc tNpc;
	memset(&tNpc, 0, sizeof(tNpc));

	pkt.SByte();

	tNpc.iID = pkt.read<int32_t>();
	tNpc.iProtoID = pkt.read<uint16_t>();
	tNpc.iMonsterOrNpc = pkt.read<uint8_t>();
	tNpc.iPictureId = pkt.read<uint16_t>();
	tNpc.iUnknown1 = pkt.read<uint32_t>();
	tNpc.iFamilyType = pkt.read<uint8_t>();
	tNpc.iSellingGroup = pkt.read<uint32_t>();
	tNpc.iModelsize = pkt.read<uint16_t>();
	tNpc.iWeapon1 = pkt.read<uint32_t>();
	tNpc.iWeapon2 = pkt.read<uint32_t>();

	if (tNpc.iProtoID == 0)
	{
		int iPetOwnerNameLen = pkt.read<uint8_t>();
		pkt.readString(tNpc.szPetOwnerName, iPetOwnerNameLen);

		int iPetNameLen = pkt.read<uint8_t>();
		pkt.readString(tNpc.szPetName, iPetNameLen);
	}

	tNpc.iModelGroup = pkt.read<uint8_t>();
	tNpc.iLevel = pkt.read<uint8_t>();

	tNpc.fX = (pkt.read<uint16_t>() / 10.0f);
	tNpc.fY = (pkt.read<uint16_t>() / 10.0f);
	tNpc.fZ = (pkt.read<int16_t>() / 10.0f);

	tNpc.iStatus = pkt.read<uint32_t>();

	tNpc.iUnknown2 = pkt.read<uint8_t>();
	tNpc.iUnknown3 = pkt.read<uint32_t>();

	tNpc.fRotation = pkt.read<int16_t>() / 100.0f;

	return tNpc;
}

TPlayer Client::InitializePlayer(Packet& pkt)
{
	TPlayer tPlayer;
	memset(&tPlayer, 0, sizeof(tPlayer));

	pkt.SByte();

	tPlayer.iID = pkt.read<int32_t>();

	int iNameLen = pkt.read<uint8_t>();
	pkt.readString(tPlayer.szName, iNameLen);

	tPlayer.eNation = (e_Nation)pkt.read<uint8_t>();

	uint8_t iUnknown1 = (e_Nation)pkt.read<uint8_t>();
	uint8_t iUnknown2 = (e_Nation)pkt.read<uint8_t>();

	tPlayer.iKnightsID = pkt.read<int16_t>();
	tPlayer.eKnightsDuty = (e_KnightsDuty)pkt.read<uint8_t>();

	int16_t iAllianceID = pkt.read<int16_t>();
	uint8_t iKnightNameLen = pkt.read<uint8_t>();
	pkt.readString(tPlayer.szKnights, iKnightNameLen);

	tPlayer.iKnightsGrade = pkt.read<uint8_t>();
	tPlayer.iKnightsRank = pkt.read<uint8_t>();

	int16_t sMarkVersion = pkt.read<int16_t>();
	int16_t sCapeID = pkt.read<int16_t>();

	uint8_t iR = pkt.read<uint8_t>();
	uint8_t iG = pkt.read<uint8_t>();
	uint8_t iB = pkt.read<uint8_t>();

	uint8_t iUnknown3 = pkt.read<uint8_t>();
	uint8_t iUnknown4 = pkt.read<uint8_t>();

	tPlayer.iLevel = pkt.read<uint8_t>();

	tPlayer.eRace = (e_Race)pkt.read<uint8_t>();
	tPlayer.eClass = (e_Class)pkt.read<int16_t>();

	tPlayer.fX = (pkt.read<uint16_t>()) / 10.0f;
	tPlayer.fY = (pkt.read<uint16_t>()) / 10.0f;
	tPlayer.fZ = (pkt.read<int16_t>()) / 10.0f;

	tPlayer.iFace = pkt.read<uint8_t>();
	tPlayer.iHair = pkt.read<int32_t>();

	uint8_t iResHpType = pkt.read<uint8_t>();

	uint32_t iAbnormalType = pkt.read<uint32_t>();

	switch (iAbnormalType)
	{
		case 4:
			tPlayer.bBlinking = true;
			break;
		case 7:
			tPlayer.bBlinking = false;
			break;
	}

	uint8_t iNeedParty = pkt.read<uint8_t>();

	tPlayer.iAuthority = pkt.read<uint8_t>();

	uint8_t iPartyLeader = pkt.read<uint8_t>();
	uint8_t iInvisibilityType = pkt.read<uint8_t>();
	uint8_t iTeamColor = pkt.read<uint8_t>();
	uint8_t iIsHidingHelmet = pkt.read<uint8_t>();
	uint8_t iIsHidingCospre = pkt.read<uint8_t>();
	uint8_t iIsDevil = pkt.read<uint8_t>();
	uint8_t iIsHidingWings = pkt.read<uint8_t>();

	int16_t iDirection = pkt.read<int16_t>();

	uint8_t iIsChicken = pkt.read<uint8_t>();
	uint8_t iRank = pkt.read<uint8_t>();
	int8_t iKnightsRank = pkt.read<int8_t>();
	int8_t iPersonalRank = pkt.read<int8_t>();

	uint8_t iUnknown5 = pkt.read<uint8_t>(); // 2/8/2023 New

	int32_t iLoop = m_bLunarWarDressUp ? 9 : 15;

	for (int32_t i = 0; i < iLoop; i++)
	{
		tPlayer.tInventory[i].iPos = i;
		tPlayer.tInventory[i].iItemID = pkt.read<uint32_t>();
		tPlayer.tInventory[i].iDurability = pkt.read<uint16_t>();
		tPlayer.tInventory[i].iFlag = pkt.read<uint8_t>();
	}

	tPlayer.iCity = pkt.read<uint8_t>();

	//TODO: Parse remaining bytes
	uint8_t iTempBuffer[16];
	pkt.read(iTempBuffer,16);

	return tPlayer;
}

void Client::RecvProcess(BYTE* byBuffer, DWORD dwLength)
{
	if (!m_bWorking)
		return;

	Packet pkt = Packet(byBuffer[0], (size_t)dwLength);
	pkt.append(&byBuffer[1], dwLength - 1);

	uint8_t byHeader;

	pkt >> byHeader;

	switch (byHeader)
	{
		case LS_LOGIN_REQ:
		{
			int16_t sUnknown;
			int8_t byResult;

			pkt >> sUnknown >> byResult;

			switch (byResult)
			{
				case AUTH_BANNED:
#ifdef _DEBUG
					printf("RecvProcess::LS_LOGIN_REQ: Account Banned\n");
#endif
					break;

				case AUTH_IN_GAME:
				{
#ifdef _DEBUG
					printf("RecvProcess::LS_LOGIN_REQ: Account already in-game\n");
#endif

					new std::thread([]()
					{
#ifdef _DEBUG
						printf("RecvProcess::LS_LOGIN_REQ: Reconnecting login server\n");
#endif
						Client::ConnectLoginServer(true);
					});
				}
				break;

				case AUTH_SUCCESS:
#ifdef _DEBUG
					printf("RecvProcess::LS_LOGIN_REQ: Login Success\n");
#endif
					break;
				default:
#ifdef _DEBUG
					printf("RecvProcess::LS_LOGIN_REQ: %d not implemented!\n", byResult);
#endif
					break;
			}
		}
		break;

		case LS_SERVERLIST:
		{
			int8_t byServerCount;

			pkt >> byServerCount;

			if (byServerCount > 0)
			{
#ifdef _DEBUG
				printf("RecvProcess::LS_SERVERLIST: %d Server loaded\n", byServerCount);
#endif

				new std::thread([]()
				{
#ifdef _DEBUG
					printf("RecvProcess::LS_SERVERLIST: Connecting to server: %d\n", 1);
#endif

					std::this_thread::sleep_for(std::chrono::milliseconds(100));

					ConnectGameServer(1);
				});
			}
		}
		break;

		case WIZ_ALLCHAR_INFO_REQ:
		{
			int8_t byResult;

			pkt >> byResult;

			//Packet End: 0C 02 00
			bool bLoaded = 
				pkt[pkt.size() - 3] == 0x0C && 
				pkt[pkt.size() - 2] == 0x02 && 
				pkt[pkt.size() - 1] == 0x00;

			if (bLoaded)
			{
#ifdef _DEBUG
				printf("RecvProcess::WIZ_ALLCHAR_INFO_REQ: Character list loaded\n");
#endif

				new std::thread([]()
				{
#ifdef _DEBUG
					printf("RecvProcess::WIZ_ALLCHAR_INFO_REQ: Selecting character %d\n", 1);
#endif

					std::this_thread::sleep_for(std::chrono::milliseconds(100));

					SelectCharacterSkip();
					SelectCharacter(1);
				});
			}
		}
		break;

		case WIZ_MYINFO:
		{
			pkt.SByte();

			m_PlayerMySelf.iID = pkt.read<int32_t>();
			
			int iNameLen = pkt.read<uint8_t>();
			pkt.readString(m_PlayerMySelf.szName, iNameLen);

			m_PlayerMySelf.fX = (pkt.read<uint16_t>()) / 10.0f;
			m_PlayerMySelf.fY = (pkt.read<uint16_t>()) / 10.0f;
			m_PlayerMySelf.fZ = (pkt.read<int16_t>()) / 10.0f;

			m_PlayerMySelf.eNation = (e_Nation)pkt.read<uint8_t>();
			m_PlayerMySelf.eRace = (e_Race)pkt.read<uint8_t>();
			m_PlayerMySelf.eClass = (e_Class)pkt.read<int16_t>();

			m_PlayerMySelf.iFace = pkt.read<uint8_t>();
			m_PlayerMySelf.iHair = pkt.read<int32_t>(); 
			m_PlayerMySelf.iRank = pkt.read<uint8_t>();
			m_PlayerMySelf.iTitle = pkt.read<uint8_t>();

			m_PlayerMySelf.iUnknown1 = pkt.read<uint8_t>();
			m_PlayerMySelf.iUnknown2 = pkt.read<uint8_t>();

			m_PlayerMySelf.iLevel = pkt.read<uint8_t>();

			m_PlayerMySelf.iBonusPointRemain = pkt.read<uint16_t>();
			m_PlayerMySelf.iExpNext = pkt.read<uint64_t>();
			m_PlayerMySelf.iExp = pkt.read<uint64_t>();

			m_PlayerMySelf.iRealmPoint = pkt.read<uint32_t>();
			m_PlayerMySelf.iRealmPointMonthly = pkt.read<uint32_t>();

			m_PlayerMySelf.iKnightsID = pkt.read<int16_t>();
			m_PlayerMySelf.eKnightsDuty = (e_KnightsDuty)pkt.read<uint8_t>();

			int16_t iAllianceID = pkt.read<int16_t>();
			uint8_t byFlag = pkt.read<uint8_t>();

			uint8_t iKnightNameLen = pkt.read<uint8_t>();
			pkt.readString(m_PlayerMySelf.szKnights, iKnightNameLen);

			m_PlayerMySelf.iKnightsGrade = pkt.read<uint8_t>();
			m_PlayerMySelf.iKnightsRank = pkt.read<uint8_t>();

			int16_t sMarkVersion = pkt.read<int16_t>();
			int16_t sCapeID = pkt.read<int16_t>();

			uint8_t iR = pkt.read<uint8_t>();
			uint8_t iG = pkt.read<uint8_t>();
			uint8_t iB = pkt.read<uint8_t>();

			uint8_t iUnknown1 = pkt.read<uint8_t>();

			uint8_t iUnknown2 = pkt.read<uint8_t>();
			uint8_t iUnknown3 = pkt.read<uint8_t>();
			uint8_t iUnknown4 = pkt.read<uint8_t>();
			uint8_t iUnknown5 = pkt.read<uint8_t>();

			m_PlayerMySelf.iHPMax = pkt.read<int16_t>();
			m_PlayerMySelf.iHP = pkt.read<int16_t>();

			m_PlayerMySelf.iMSPMax = pkt.read<int16_t>();
			m_PlayerMySelf.iMSP = pkt.read<int16_t>();

			m_PlayerMySelf.iWeightMax = pkt.read<uint32_t>();
			m_PlayerMySelf.iWeight = pkt.read<uint32_t>();

			m_PlayerMySelf.iStrength = pkt.read<uint8_t>();
			m_PlayerMySelf.iStrength_Delta = pkt.read<uint8_t>();
			m_PlayerMySelf.iStamina = pkt.read<uint8_t>();
			m_PlayerMySelf.iStamina_Delta = pkt.read<uint8_t>();
			m_PlayerMySelf.iDexterity = pkt.read<uint8_t>();
			m_PlayerMySelf.iDexterity_Delta = pkt.read<uint8_t>();
			m_PlayerMySelf.iIntelligence = pkt.read<uint8_t>();
			m_PlayerMySelf.iIntelligence_Delta = pkt.read<uint8_t>();
			m_PlayerMySelf.iMagicAttak = pkt.read<uint8_t>();
			m_PlayerMySelf.iMagicAttak_Delta = pkt.read<uint8_t>();

			m_PlayerMySelf.iAttack = pkt.read<int16_t>();
			m_PlayerMySelf.iGuard = pkt.read<int16_t>();

			m_PlayerMySelf.iRegistFire = pkt.read<uint8_t>();
			m_PlayerMySelf.iRegistCold = pkt.read<uint8_t>();
			m_PlayerMySelf.iRegistLight = pkt.read<uint8_t>();
			m_PlayerMySelf.iRegistMagic = pkt.read<uint8_t>();
			m_PlayerMySelf.iRegistCurse = pkt.read<uint8_t>();
			m_PlayerMySelf.iRegistPoison = pkt.read<uint8_t>();

			m_PlayerMySelf.iGold = pkt.read<uint32_t>();
			m_PlayerMySelf.iAuthority = pkt.read<uint8_t>();

			uint8_t bUserRank = pkt.read<uint8_t>();
			uint8_t bPersonalRank = pkt.read<uint8_t>();

			for (int i = 0; i < 9; i++)
			{
				m_PlayerMySelf.iSkillInfo[i] = pkt.read<uint8_t>();
			}

			for (int i = 0; i < INVENTORY_TOTAL; i++)
			{
				m_PlayerMySelf.tInventory[i].iPos = i;
				m_PlayerMySelf.tInventory[i].iItemID = pkt.read<uint32_t>();
				m_PlayerMySelf.tInventory[i].iDurability = pkt.read<uint16_t>();
				m_PlayerMySelf.tInventory[i].iCount = pkt.read<uint16_t>();
				m_PlayerMySelf.tInventory[i].iFlag = pkt.read<uint8_t>();
				m_PlayerMySelf.tInventory[i].iRentalTime = pkt.read<int16_t>();
				m_PlayerMySelf.tInventory[i].iSerial = pkt.read<uint32_t>();
				m_PlayerMySelf.tInventory[i].iExpirationTime = pkt.read<uint32_t>();
			}

			m_PlayerMySelf.bBlinking = true;

#ifdef _DEBUG
			printf("RecvProcess::WIZ_MYINFO: %s loaded\n", m_PlayerMySelf.szName.c_str());
#endif
		}
		break;

		case WIZ_HP_CHANGE:
		{
			m_PlayerMySelf.iHPMax = pkt.read<int16_t>();
			m_PlayerMySelf.iHP = pkt.read<int16_t>();

#ifdef _DEBUG
			printf("RecvProcess::WIZ_HP_CHANGE: %d / %d\n", m_PlayerMySelf.iHP, m_PlayerMySelf.iHPMax);
#endif
		}
		break;

		case WIZ_MSP_CHANGE:
		{
			m_PlayerMySelf.iMSPMax = pkt.read<int16_t>();
			m_PlayerMySelf.iMSP = pkt.read<int16_t>();

#ifdef _DEBUG
			printf("RecvProcess::WIZ_MSP_CHANGE: %d / %d\n", m_PlayerMySelf.iMSP, m_PlayerMySelf.iMSPMax);
#endif
		}
		break;

		case WIZ_EXP_CHANGE:
		{
			uint8_t iUnknown1 = pkt.read<uint8_t>();
			m_PlayerMySelf.iExp = pkt.read<uint64_t>();

#ifdef _DEBUG
			printf("RecvProcess::WIZ_EXP_CHANGE: %llu\n", m_PlayerMySelf.iExp);
#endif
		}
		break;

		case WIZ_LEVEL_CHANGE:
		{
			int32_t iID = pkt.read<int32_t>();
			uint8_t iLevel = pkt.read<uint8_t>();

			if (m_PlayerMySelf.iID == iID)
			{
				m_PlayerMySelf.iLevel = iLevel;
				m_PlayerMySelf.iBonusPointRemain = pkt.read<uint16_t>();
				m_PlayerMySelf.iSkillInfo[0] = pkt.read<uint8_t>();
				m_PlayerMySelf.iExpNext = pkt.read<int64_t>();
				m_PlayerMySelf.iExp = pkt.read<int64_t>();
				m_PlayerMySelf.iHPMax = pkt.read<int16_t>();
				m_PlayerMySelf.iHP = pkt.read<int16_t>();
				m_PlayerMySelf.iMSPMax = pkt.read<int16_t>();
				m_PlayerMySelf.iMSP = pkt.read<int16_t>();
				m_PlayerMySelf.iWeightMax = pkt.read<uint32_t>();
				m_PlayerMySelf.iWeight = pkt.read<uint32_t>();

#ifdef _DEBUG
				printf("RecvProcess::WIZ_LEVEL_CHANGE: %s %d\n", m_PlayerMySelf.szName.c_str(), iLevel);
#endif
			}
			else
			{
				auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
					[iID](const TPlayer& a) { return a.iID == iID; });

				if (it != m_vecPlayer.end())
				{
					it->iLevel = iLevel;

#ifdef _DEBUG
					printf("RecvProcess::WIZ_LEVEL_CHANGE: %s %d\n", it->szName.c_str(), iLevel);
#endif
				}
			}
		}
		break;

		case WIZ_POINT_CHANGE:
		{
			uint8_t iType = pkt.read<uint8_t>();
			int16_t iVal = pkt.read<int16_t>();

			m_PlayerMySelf.iHPMax = pkt.read<int16_t>();
			m_PlayerMySelf.iMSPMax = pkt.read<int16_t>();
			m_PlayerMySelf.iAttack = pkt.read<int16_t>();
			m_PlayerMySelf.iWeightMax = pkt.read<uint32_t>();

			switch (iType)
			{
				case 0x01:
				{
					m_PlayerMySelf.iStrength = (uint8_t)iVal;
#ifdef _DEBUG
					printf("RecvProcess::WIZ_POINT_CHANGE: STR %d\n", iVal);
#endif
				}
				break;

				case 0x02:
				{
					m_PlayerMySelf.iStamina = (uint8_t)iVal;
#ifdef _DEBUG
					printf("RecvProcess::WIZ_POINT_CHANGE: HP %d\n", iVal);
#endif
				}
				break;

				case 0x03:
				{
					m_PlayerMySelf.iDexterity = (uint8_t)iVal;
#ifdef _DEBUG
					printf("RecvProcess::WIZ_POINT_CHANGE: DEX %d\n", iVal);
#endif
				}
				break;

				case 0x04:
				{
					m_PlayerMySelf.iIntelligence = (uint8_t)iVal;
#ifdef _DEBUG
					printf("RecvProcess::WIZ_POINT_CHANGE: INT %d\n", iVal);
#endif
				}
				break;

				case 0x05:
				{
					m_PlayerMySelf.iMagicAttak = (uint8_t)iVal;
#ifdef _DEBUG
					printf("RecvProcess::WIZ_POINT_CHANGE: MP %d\n", iVal);
#endif
				}
				break;
			}

			if (iType >= 1 && iType <= 5)
			{
				m_PlayerMySelf.iBonusPointRemain--;
#ifdef _DEBUG
				printf("RecvProcess::WIZ_POINT_CHANGE: POINT %d\n", m_PlayerMySelf.iBonusPointRemain);
#endif
			}
		}
		break;

		case WIZ_WEIGHT_CHANGE:
		{
			m_PlayerMySelf.iWeight = pkt.read<uint32_t>();
#ifdef _DEBUG
			printf("RecvProcess::WIZ_WEIGHT_CHANGE: %d\n", m_PlayerMySelf.iWeight);
#endif
		}
		break;

		case WIZ_DURATION:
		{
			uint8_t iPos = pkt.read<uint8_t>();
			uint16_t iDurability = pkt.read<uint16_t>();

			m_PlayerMySelf.tInventory[iPos].iDurability = iDurability;

#ifdef _DEBUG
			printf("RecvProcess::WIZ_DURATION: %d,%d\n", iPos, iDurability);
#endif
		}
		break;

		case WIZ_ITEM_REMOVE:
		{
			uint8_t	iResult = pkt.read<uint8_t>();

			switch (iResult)
			{
				case 0x00: 
#ifdef _DEBUG
					printf("RecvProcess::WIZ_ITEM_REMOVE: 0\n");
#endif
					break;

				case 0x01: 
#ifdef _DEBUG
					printf("RecvProcess::WIZ_ITEM_REMOVE: 1\n");
#endif
					break;

				default:
#ifdef _DEBUG
					printf("RecvProcess::WIZ_ITEM_REMOVE: %d Result Not Implemented\n", iResult);
#endif
				break;
			}
		}
		break;

		case WIZ_ITEM_COUNT_CHANGE:
		{
			int16_t iTotalCount = pkt.read<int16_t>();

			for (int i = 0; i < iTotalCount; i++)
			{
				uint8_t iDistrict = pkt.read<uint8_t>();
				uint8_t iIndex = pkt.read<uint8_t>();
				uint32_t iID = pkt.read<uint32_t>();
				uint32_t iCount = pkt.read<uint32_t>();
				uint8_t iNewItem = pkt.read<uint8_t>();
				uint16_t iDurability = pkt.read<uint16_t>();

				uint32_t iSerial = pkt.read<uint32_t>();
				uint32_t iExpirationTime = pkt.read<uint32_t>();

				m_PlayerMySelf.tInventory[14 + iIndex].iItemID = iID;
				m_PlayerMySelf.tInventory[14 + iIndex].iCount = (uint16_t)iCount;
				m_PlayerMySelf.tInventory[14 + iIndex].iDurability = iDurability;
				m_PlayerMySelf.tInventory[14 + iIndex].iSerial = iSerial;
				m_PlayerMySelf.tInventory[14 + iIndex].iExpirationTime = iExpirationTime;

#ifdef _DEBUG
				printf("RecvProcess::WIZ_ITEM_COUNT_CHANGE: %d,%d,%d,%d,%d,%d,%d,%d,%d\n", 
					iDistrict,
					iIndex,
					iID,
					iCount,
					iNewItem,
					iNewItem,
					iDurability,
					iSerial,
					iExpirationTime);
#endif

			}
		}
		break;

		case WIZ_SKILLPT_CHANGE:
		{
			int iType = pkt.read<uint8_t>();
			int iValue = pkt.read<uint8_t>();

			m_PlayerMySelf.iSkillInfo[iType] = iValue;
			m_PlayerMySelf.iSkillInfo[0]++;

#ifdef _DEBUG
			printf("RecvProcess::WIZ_ITEM_REMOVE: %d,%d,%d\n", iType, iValue, m_PlayerMySelf.iSkillInfo[0]);
#endif
		}
		break;

		case WIZ_CLASS_CHANGE:
		{
			uint8_t	iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case N3_SP_CLASS_CHANGE_PURE:
#ifdef _DEBUG
					printf("RecvProcess::WIZ_CLASS_CHANGE: N3_SP_CLASS_CHANGE_PURE\n");
#endif
					break;

				case N3_SP_CLASS_CHANGE_REQ:
				{
					e_SubPacket_ClassChange eSP = (e_SubPacket_ClassChange)pkt.read<uint8_t>();

					switch (eSP)
					{
						case N3_SP_CLASS_CHANGE_SUCCESS:
#ifdef _DEBUG
							printf("RecvProcess::WIZ_CLASS_CHANGE: N3_SP_CLASS_CHANGE_SUCCESS\n");
#endif
							break;

						case N3_SP_CLASS_CHANGE_NOT_YET:
#ifdef _DEBUG
							printf("RecvProcess::WIZ_CLASS_CHANGE: N3_SP_CLASS_CHANGE_NOT_YET\n");
#endif
							break;

						case N3_SP_CLASS_CHANGE_ALREADY:
#ifdef _DEBUG
							printf("RecvProcess::WIZ_CLASS_CHANGE: N3_SP_CLASS_CHANGE_ALREADY\n");
#endif
							break;

						case N3_SP_CLASS_CHANGE_FAILURE:
#ifdef _DEBUG
							printf("RecvProcess::WIZ_CLASS_CHANGE: N3_SP_CLASS_CHANGE_FAILURE\n");
#endif
							break;
					}
				}
				break;

				case N3_SP_CLASS_ALL_POINT:
				{
					uint8_t	iType = pkt.read<uint8_t>();
					uint32_t iGold = pkt.read<uint32_t>();

					switch (iType)
					{
						case 0x00: break;

						case 0x01:
						{
							m_PlayerMySelf.iStrength = (uint8_t)pkt.read<int16_t>();
							m_PlayerMySelf.iStamina = (uint8_t)pkt.read<int16_t>();
							m_PlayerMySelf.iDexterity = (uint8_t)pkt.read<int16_t>();
							m_PlayerMySelf.iIntelligence = (uint8_t)pkt.read<int16_t>();
							m_PlayerMySelf.iMagicAttak = (uint8_t)pkt.read<int16_t>();

							m_PlayerMySelf.iHPMax = pkt.read<int16_t>();
							m_PlayerMySelf.iMSPMax = pkt.read<int16_t>();
							m_PlayerMySelf.iAttack = pkt.read<int16_t>();
							m_PlayerMySelf.iWeightMax = pkt.read<uint32_t>();

							m_PlayerMySelf.iBonusPointRemain = pkt.read<int16_t>();

							m_PlayerMySelf.iGold = iGold;

#ifdef _DEBUG
							printf("RecvProcess::WIZ_CLASS_CHANGE: Stat point reset\n");
							printf("RecvProcess::WIZ_CLASS_CHANGE: %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
								m_PlayerMySelf.iStrength,
								m_PlayerMySelf.iStamina,
								m_PlayerMySelf.iDexterity,
								m_PlayerMySelf.iIntelligence,
								m_PlayerMySelf.iMagicAttak,
								m_PlayerMySelf.iHPMax,
								m_PlayerMySelf.iMSPMax,
								m_PlayerMySelf.iAttack,
								m_PlayerMySelf.iWeightMax,
								m_PlayerMySelf.iBonusPointRemain,
								m_PlayerMySelf.iGold);

#endif
						}
						break;

						case 0x02: break;
					}
				}
				break;

				case N3_SP_CLASS_SKILL_POINT:
				{
					uint8_t	iType = pkt.read<uint8_t>();
					uint32_t iGold = pkt.read<uint32_t>();

					switch (iType)
					{
						case 0x00: break;

						case 0x01:
						{
							m_PlayerMySelf.iSkillInfo[0] = pkt.read<uint8_t>();

							for (int i = 1; i < 9; i++)
								m_PlayerMySelf.iSkillInfo[i] = 0;

#ifdef _DEBUG
							printf("RecvProcess::WIZ_CLASS_CHANGE: Skill point reset, new points: %d\n", 
								m_PlayerMySelf.iSkillInfo[0]);
#endif
						}
						break;

						case 0x02: break;
					}
				}
				break;

				case N3_SP_CLASS_POINT_CHANGE_PRICE_QUERY:
				{
					uint32_t iGold = pkt.read<uint32_t>();
#ifdef _DEBUG
					printf("RecvProcess::WIZ_CLASS_CHANGE: Point change price %d\n", iGold);
#endif
				}
				break;

				case N3_SP_CLASS_PROMOTION:
				{
					uint16_t iClass = pkt.read<uint16_t>();
					uint32_t iID = pkt.read<uint32_t>();

					if (m_PlayerMySelf.iID == iID)
					{
						m_PlayerMySelf.eClass = (e_Class)iClass;
#ifdef _DEBUG
						printf("RecvProcess::WIZ_CLASS_CHANGE: %s class changed to %d\n", 
							m_PlayerMySelf.szName.c_str(), m_PlayerMySelf.eClass);
#endif
					}
					else
					{
						auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
							[iID](const TPlayer& a) { return a.iID == iID; });

						if (it != m_vecPlayer.end())
						{
							it->eClass = (e_Class)iClass;

#ifdef _DEBUG
							printf("RecvProcess::WIZ_CLASS_CHANGE: %s class changed to %d\n",
								it->szName.c_str(), it->eClass);
#endif
						}
					}
				}
				break;
			}
		}
		break;

		case WIZ_GOLD_CHANGE:
		{
			uint8_t iType = pkt.read<uint8_t>();
			uint32_t iGoldOffset = pkt.read<uint32_t>();
			uint32_t iGold = pkt.read<uint32_t>();

			m_PlayerMySelf.iGold = iGold;

#ifdef _DEBUG
			printf("RecvProcess::WIZ_GOLD_CHANGE: %d,%d,%d\n", iType, iGoldOffset, iGold);
#endif
		}
		break;

		case WIZ_ITEM_MOVE:
		{
			uint8_t iType = pkt.read<uint8_t>();

			if (iType != 0)
			{
				uint8_t iSubType = pkt.read<uint8_t>();

				if (iSubType != 0)
				{
					m_PlayerMySelf.iAttack = pkt.read<int16_t>();
					m_PlayerMySelf.iGuard = pkt.read<int16_t>();
					m_PlayerMySelf.iWeightMax = pkt.read<uint32_t>();

					uint16_t iUnknown1 = pkt.read<uint16_t>();

					m_PlayerMySelf.iHPMax = pkt.read<int16_t>();
					m_PlayerMySelf.iMSPMax = pkt.read<int16_t>();

					m_PlayerMySelf.iStrength_Delta = (uint8_t)pkt.read<uint16_t>();
					m_PlayerMySelf.iStamina_Delta = (uint8_t)pkt.read<uint16_t>();
					m_PlayerMySelf.iDexterity_Delta = (uint8_t)pkt.read<uint16_t>();
					m_PlayerMySelf.iIntelligence_Delta = (uint8_t)pkt.read<uint16_t>();
					m_PlayerMySelf.iMagicAttak_Delta = (uint8_t)pkt.read<uint16_t>();

					m_PlayerMySelf.iRegistFire = (uint8_t)pkt.read<uint16_t>();
					m_PlayerMySelf.iRegistCold = (uint8_t)pkt.read<uint16_t>();
					m_PlayerMySelf.iRegistLight = (uint8_t)pkt.read<uint16_t>();
					m_PlayerMySelf.iRegistMagic = (uint8_t)pkt.read<uint16_t>();
					m_PlayerMySelf.iRegistCurse = (uint8_t)pkt.read<uint16_t>();
					m_PlayerMySelf.iRegistPoison = (uint8_t)pkt.read<uint16_t>();

#ifdef _DEBUG
					printf("RecvProcess::WIZ_ITEM_MOVE: %d,%d,%d,Unknown1(%d),%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
						m_PlayerMySelf.iAttack,
						m_PlayerMySelf.iGuard,
						m_PlayerMySelf.iWeightMax,
						iUnknown1,
						m_PlayerMySelf.iHPMax,
						m_PlayerMySelf.iMSPMax,
						m_PlayerMySelf.iStrength_Delta,
						m_PlayerMySelf.iStamina_Delta,
						m_PlayerMySelf.iDexterity_Delta,
						m_PlayerMySelf.iIntelligence_Delta,
						m_PlayerMySelf.iMagicAttak_Delta,
						m_PlayerMySelf.iRegistFire,
						m_PlayerMySelf.iRegistCold,
						m_PlayerMySelf.iRegistLight,
						m_PlayerMySelf.iRegistMagic,
						m_PlayerMySelf.iRegistCurse,
						m_PlayerMySelf.iRegistPoison);
#endif
				}
				else
				{
#ifdef _DEBUG
					printf("RecvProcess::WIZ_ITEM_MOVE: %d SubType Not Implemented\n", iSubType);
#endif
				}
			}
			else
			{
#ifdef _DEBUG
				printf("RecvProcess::WIZ_ITEM_MOVE: %d Type Not Implemented\n", iType);
#endif
			}
		}
		break;

		case WIZ_LOYALTY_CHANGE:
		{
			uint8_t bType = pkt.read<uint8_t>();

			if (bType == 1)
			{
				uint32_t iLoyalty = pkt.read<uint32_t>();
				uint32_t iLoyaltyMonthly = pkt.read<uint32_t>();
				uint32_t iUnknown1 = pkt.read<uint32_t>();
				uint32_t iClanLoyaltyAmount = pkt.read<uint32_t>();

#ifdef _DEBUG
				printf("RecvProcess::WIZ_LOYALTY_CHANGE: %d,%d,%d,Unknown1(%d),%d\n", 
					bType, iLoyalty, iLoyaltyMonthly, iUnknown1, iClanLoyaltyAmount);
#endif
			}
			else
			{
#ifdef _DEBUG
				printf("RecvProcess::WIZ_LOYALTY_CHANGE: %d Type Not Implemented\n", bType);
#endif
			}
		}
		break;

		case WIZ_GAMESTART:
		{
#ifdef _DEBUG
			printf("RecvProcess::WIZ_GAMESTART: Started\n");
#endif

			std::string strCharacterName = GetName();

			if (!IsUserConfigLoaded(strCharacterName))
				LoadUserConfig(strCharacterName);

			auto pUserConfig = Client::GetUserConfig(Client::GetName());

			if (pUserConfig != NULL)
			{
				bool bWallHack = pUserConfig->GetBool("Feature", "WallHack", false);

				if (bWallHack)
					SetAuthority(0);

				bool bOreads = pUserConfig->GetBool("Feature", "Oreads", false);

				if (bOreads)
				{
					Client::EquipOreads(700039000);
					Client::SetOreads(bOreads);
				}
			}

			StartHandler();

			Client::SetState(State::GAME);
		}
		break;

		case WIZ_REQ_NPCIN:
		{
			int16_t iNpcCount = pkt.read<int16_t>();

			if (0 == iNpcCount)
			if (iNpcCount < 0 || iNpcCount >= 1000)
				return;

			for (int16_t i = 0; i < iNpcCount; i++)
			{
				auto pNpc = InitializeNpc(pkt);

				int32_t iNpcID = pNpc.iID;

				auto it = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
					[iNpcID](const TNpc& a) { return a.iID == iNpcID; });

				if (it == m_vecNpc.end())
					m_vecNpc.push_back(pNpc);
				else
					*it = pNpc;
			}

#ifdef _DEBUG
			printf("RecvProcess::WIZ_REQ_NPCIN: Size %d\n", iNpcCount);
#endif
		}
		break;

		case WIZ_REQ_USERIN:
		{
			int16_t iUserCount = pkt.read<int16_t>();

			if (0 == iUserCount)
			if (iUserCount < 0 || iUserCount >= 1000)
				return;

			for (int16_t i = 0; i < iUserCount; i++)
			{
				uint8_t iUnknown0 = pkt.read<uint8_t>();

				auto pUser = InitializePlayer(pkt);

				auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
					[pUser](const TPlayer& a) { return a.iID == pUser.iID; });

				if (it == m_vecPlayer.end())
					m_vecPlayer.push_back(pUser);
				else
					*it = pUser;
			}

#ifdef _DEBUG
			printf("RecvProcess::WIZ_REQ_USERIN: Size %d\n", iUserCount);
#endif
		}
		break;

		case WIZ_NPC_INOUT:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case InOut::INOUT_IN:
				{
					auto pNpc = InitializeNpc(pkt);

					auto it = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
						[pNpc](const TNpc& a) { return a.iID == pNpc.iID; });

					if (it == m_vecNpc.end())
						m_vecNpc.push_back(pNpc);
					else
						*it = pNpc;

#ifdef _DEBUG
					printf("RecvProcess::WIZ_NPC_INOUT: %d,%d\n", iType, pNpc.iID);
#endif
				}
				break;

				case InOut::INOUT_OUT:
				{
					int32_t iNpcID = pkt.read<int32_t>();

					m_vecNpc.erase(
						std::remove_if(m_vecNpc.begin(), m_vecNpc.end(),
							[iNpcID](const TNpc& a) { return a.iID == iNpcID; }),
						m_vecNpc.end());

#ifdef _DEBUG
					printf("RecvProcess::WIZ_NPC_INOUT: %d,%d\n", iType, iNpcID);
#endif
				}
				break;

				default:
#ifdef _DEBUG
					printf("RecvProcess::WIZ_NPC_INOUT: %d not implemented\n", iType);
#endif
					break;
			}
		}
		break;

		case WIZ_USER_INOUT:
		{
			uint8_t iType = pkt.read<uint8_t>();
			uint8_t iUnknown0 = pkt.read<uint8_t>();

			switch (iType)
			{
				case InOut::INOUT_IN:
				case InOut::INOUT_RESPAWN:
				case InOut::INOUT_WARP:
				{
					auto pPlayer = InitializePlayer(pkt);

					auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
						[pPlayer](const TPlayer& a) { return a.iID == pPlayer.iID; });

					if (it == m_vecPlayer.end())
						m_vecPlayer.push_back(pPlayer);
					else
						*it = pPlayer;

#ifdef _DEBUG
					printf("RecvProcess::WIZ_USER_INOUT: %d,%d\n", iType, pPlayer.iID);
#endif
				}
				break;

				case InOut::INOUT_OUT:
				{
					int32_t iPlayerID = pkt.read<int32_t>();

					m_vecPlayer.erase(
						std::remove_if(m_vecPlayer.begin(), m_vecPlayer.end(),
							[iPlayerID](const TPlayer& a) { return a.iID == iPlayerID; }),
						m_vecPlayer.end());

#ifdef _DEBUG
					printf("RecvProcess::WIZ_USER_INOUT: %d,%d\n", iType, iPlayerID);
#endif
				}
				break;

				default:
#ifdef _DEBUG
					printf("RecvProcess::WIZ_USER_INOUT: %d not implemented\n", iType);
#endif
					break;
			}
		}
		break;

		case WIZ_NPC_REGION:
		{
			int16_t iNpcCount = pkt.read<int16_t>();

			if (0 == iNpcCount)
			if (iNpcCount < 0 || iNpcCount >= 1000)
				return;

#ifdef _DEBUG
			printf("RecvProcess::WIZ_NPC_REGION: New npc count %d\n", iNpcCount);
#endif
		}
		break;

		case WIZ_REGIONCHANGE:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case 1:
				{
					int16_t iUserCount = pkt.read<int16_t>();

					if (0 == iUserCount)
						if (iUserCount < 0 || iUserCount >= 1000)
							return;

#ifdef _DEBUG
					printf("RecvProcess::WIZ_REGIONCHANGE: New user count %d\n", iUserCount);
#endif
				}
				break;

				default:
#ifdef _DEBUG
					printf("RecvProcess::WIZ_REGIONCHANGE: Type %d not implemented!\n", iType);
#endif
					break;
			}
		}
		break;

		case WIZ_DEAD:
		{
			int32_t iID = pkt.read<int32_t>();

			if (m_PlayerMySelf.iID == iID)
			{
				m_PlayerMySelf.eState = PSA_DEATH;

#ifdef _DEBUG
				printf("RecvProcess::WIZ_DEAD: MySelf Dead\n");
#endif
			}
			else
			{
				if (iID >= 5000)
				{
					auto it = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
						[iID](const TNpc& a) { return a.iID == iID; });

					if (it != m_vecNpc.end())
					{
						it->eState = PSA_DEATH;

#ifdef _DEBUG
						printf("RecvProcess::WIZ_DEAD: %d Npc Dead\n", iID);
#endif
					}
				}
				else
				{
					auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
						[iID](const TPlayer& a) { return a.iID == iID; });

					if (it != m_vecPlayer.end())
					{
						it->eState = PSA_DEATH;

#ifdef _DEBUG
						printf("RecvProcess::WIZ_DEAD: %d Player Dead\n", iID);
#endif
					}
				}
			}
		}
		break;

		case WIZ_ATTACK:
		{
			uint8_t iType = pkt.read<uint8_t>();
			uint8_t iResult = pkt.read<uint8_t>();

			switch (iResult)
			{
				case ATTACK_FAIL:
				{
					int32_t iAttackID = pkt.read<int32_t>();
					int32_t iTargetID = pkt.read<int32_t>();

#ifdef _DEBUG
					printf("RecvProcess::WIZ_ATTACK: %d,%d FAIL\n", iAttackID, iTargetID);
#endif
				}
				break;

				case ATTACK_SUCCESS:
				{
					int32_t iAttackID = pkt.read<int32_t>();
					int32_t iTargetID = pkt.read<int32_t>();

#ifdef _DEBUG
					printf("RecvProcess::WIZ_ATTACK: %d,%d SUCCESS\n", iAttackID, iTargetID);
#endif
				}
				break;

				case ATTACK_TARGET_DEAD:
				case ATTACK_TARGET_DEAD_OK:
				{
					int32_t iAttackID = pkt.read<int32_t>();
					int32_t iTargetID = pkt.read<int32_t>();

					if (iTargetID >= 5000)
					{
						auto it = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
							[iTargetID](const TNpc& a) { return a.iID == iTargetID; });

						if (it != m_vecNpc.end())
						{
							it->eState = PSA_DEATH;

#ifdef _DEBUG
							printf("RecvProcess::WIZ_ATTACK: TARGET_DEAD | TARGET_DEAD_OK - %d Npc Dead\n", iTargetID);
#endif
						}
					}
					else
					{
						auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
							[iTargetID](const TPlayer& a) { return a.iID == iTargetID; });

						if (it != m_vecPlayer.end())
						{
							it->eState = PSA_DEATH;

#ifdef _DEBUG
							printf("RecvProcess::WIZ_ATTACK: TARGET_DEAD | TARGET_DEAD_OK - %d Player Dead\n", iTargetID);
#endif
						}
					}
				}
				break;

				default:
#ifdef _DEBUG
					printf("RecvProcess::WIZ_ATTACK: %d not implemented\n", iResult);
#endif
					break;

			}
		}
		break;

		case WIZ_TARGET_HP:
		{
			int32_t iID = pkt.read<int32_t>();
			uint8_t iUpdateImmediately = pkt.read<uint8_t>();

			int32_t iTargetHPMax = pkt.read<int32_t>();
			int32_t iTargetHPCur = pkt.read<int32_t>();

			int16_t iTargetHPChange = pkt.read<int16_t>();

			if (m_PlayerMySelf.iID == iID)
			{
				m_PlayerMySelf.iHPMax = iTargetHPMax;
				m_PlayerMySelf.iHP = iTargetHPCur;

				if (m_PlayerMySelf.iHPMax > 0 && m_PlayerMySelf.iHP <= 0)
					m_PlayerMySelf.eState = PSA_DEATH;

#ifdef _DEBUG
				printf("RecvProcess::WIZ_TARGET_HP: %s, %d / %d\n", 
					m_PlayerMySelf.szName.c_str(), m_PlayerMySelf.iHP, m_PlayerMySelf.iHPMax);
#endif
			}
			else
			{
				if (iID >= 5000)
				{
					auto it = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
						[iID](const TNpc& a) { return a.iID == iID; });

					if (it != m_vecNpc.end())
					{
						it->iHPMax = iTargetHPMax;
						it->iHP = iTargetHPCur;

						if (it->iHPMax > 0 && it->iHP <= 0)
							it->eState = PSA_DEATH;

#ifdef _DEBUG
						printf("RecvProcess::WIZ_TARGET_HP: %d, %d / %d\n", iID, it->iHP, it->iHPMax);
#endif
					}
				}
				else
				{
					auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
						[iID](const TPlayer& a) { return a.iID == iID; });

					if (it != m_vecPlayer.end())
					{
						it->iHPMax = iTargetHPMax;
						it->iHP = iTargetHPCur;

						if (it->iHPMax > 0 && it->iHP <= 0)
							it->eState = PSA_DEATH;

#ifdef _DEBUG
						printf("RecvProcess::WIZ_TARGET_HP: %s, %d / %d\n", 
							it->szName.c_str(), it->iHP, it->iHPMax);
#endif
					}
				}
			}
		}
		break;

		case WIZ_MOVE:
		{
			int32_t iID = pkt.read<int32_t>();

			float fX = pkt.read<uint16_t>() / 10.0f;
			float fY = pkt.read<uint16_t>() / 10.0f;
			float fZ = pkt.read<int16_t>() / 10.0f;

			int16_t iSpeed = pkt.read<int16_t>();
			uint8_t iMoveType = pkt.read<uint8_t>();

			if (m_PlayerMySelf.iID == iID)
			{
				m_PlayerMySelf.fX = fX;
				m_PlayerMySelf.fY = fY;
				m_PlayerMySelf.fZ = fZ;

				m_PlayerMySelf.iMoveSpeed = iSpeed;
				m_PlayerMySelf.iMoveType = iMoveType;
			}
			else
			{
				auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
					[iID](const TPlayer& a) { return a.iID == iID; });

				if (it != m_vecPlayer.end())
				{
					it->fX = fX;
					it->fY = fY;
					it->fZ = fZ;

					it->iMoveSpeed = iSpeed;
					it->iMoveType = iMoveType;
				}
#ifdef _DEBUG
				else
					printf("RecvProcess::WIZ_MOVE: %d not in m_vecPlayer list, is ghost player\n", iID);
#endif
			}

#ifdef _DEBUG
			//printf("RecvProcess::WIZ_MOVE: %d,%f,%f,%f,%d,%d\n", 
			//	iID, fX, fY, fZ, iSpeed, iMoveType);
#endif
		}
		break;

		case WIZ_NPC_MOVE:
		{
			uint8_t iMoveType = pkt.read<uint8_t>();

			int32_t iID = pkt.read<int32_t>();

			float fX = pkt.read<uint16_t>() / 10.0f;
			float fY = pkt.read<uint16_t>() / 10.0f;
			float fZ = pkt.read<int16_t>() / 10.0f;

			uint16_t iSpeed = pkt.read<uint16_t>();

			auto it = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
				[iID](const TNpc& a) { return a.iID == iID; });

			if (it != m_vecNpc.end())
			{
				it->fX = fX;
				it->fY = fY;
				it->fZ = fZ;

				it->iMoveSpeed = iSpeed;
				it->iMoveType = iMoveType;
			}
#ifdef _DEBUG
			else
				printf("RecvProcess::WIZ_NPC_MOVE: %d not in m_vecNpc list, is ghost npc\n", iID);
#endif

#ifdef _DEBUG
			//printf("RecvProcess::WIZ_NPC_MOVE: %d,%d,%f,%f,%f,%d\n",
			//	iMoveType, iID, fX, fY, fZ, iSpeed);
#endif
		}
		break;

		case WIZ_MAGIC_PROCESS:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case SkillMagicType::SKILL_MAGIC_TYPE_EFFECTING:
				{
					uint32_t iSkillID = pkt.read<uint32_t>();
					int32_t iSourceID = pkt.read<int32_t>();
					int32_t iTargetID = pkt.read<int32_t>();

					auto pSkillTable = Bootstrap::GetSkillTable().GetData();
					auto pSkillData = pSkillTable.find(iSkillID);

					if (pSkillData != pSkillTable.end())
					{
						if (pSkillData->second.dw1stTableType == 4)
						{
							if (iTargetID == GetID())
							{
								auto pSkillExtension4 = Bootstrap::GetSkillExtension4Table().GetData();
								auto pSkillExtension4Data = pSkillExtension4.find(pSkillData->second.iID);

								if (pSkillExtension4Data != pSkillExtension4.end())
								{
									auto it = m_mapActiveBuffList.find(pSkillExtension4Data->second.iBuffType);

									if (it != m_mapActiveBuffList.end())
										m_mapActiveBuffList.erase(pSkillExtension4Data->second.iBuffType);

									m_mapActiveBuffList.insert(std::pair(pSkillExtension4Data->second.iBuffType, pSkillData->second.iID));

									switch (pSkillData->second.iBaseId)
									{
										case 101001:
										case 107010:
											m_PlayerMySelf.iMoveSpeed = 67;
											break;

										case 107725:
											m_PlayerMySelf.iMoveSpeed = 90;
											break;
									}

#ifdef _DEBUG
									printf("RecvProcess::WIZ_MAGIC_PROCESS: %s %s Buff added\n", GetName().c_str(), pSkillData->second.szEngName.c_str());
#endif
								}
#ifdef _DEBUG
								else
								{
									printf("RecvProcess::WIZ_MAGIC_PROCESS: %s %s Buff added but extension not		exist\n", GetName().c_str(), pSkillData->second.szEngName.c_str());
								}	
#endif
							}
							else
							{
								if (iTargetID < 5000)
								{
									auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
										[iTargetID](const TPlayer& a) { return a.iID == iTargetID; });

									if (it != m_vecPlayer.end())
									{
										switch (pSkillData->second.iBaseId)
										{
											case 101001:
											case 107010:
												it->iMoveSpeed = 67;
												break;

											case 107725:
												it->iMoveSpeed = 90;
												break;
										}

#ifdef _DEBUG
										printf("RecvProcess::WIZ_MAGIC_PROCESS: %s %s Buff added\n", it->szName.c_str(), pSkillData->second.szEngName.c_str());
#endif
									}
								}
							}
						}
					}
				}
				break;

				case SkillMagicType::SKILL_MAGIC_TYPE_FAIL:
				{
					uint32_t iSkillID = pkt.read<uint32_t>();
					int32_t iSourceID = pkt.read<int32_t>();
					int32_t iTargetID = pkt.read<int32_t>();

					int32_t iData[6];

					for (size_t i = 0; i < 6; i++)
						iData[i] = pkt.read<int32_t>();

					if (iData[3] == -100 || iData[3] == -103)
					{
#ifdef _DEBUG
						printf("RecvProcess::WIZ_MAGIC_PROCESS: %d - Skill failed %d\n", iSkillID, iData[3]);
#endif
						Client::SetSkillUseTime(iSkillID, (std::chrono::milliseconds)0);
					}
				}
				break;

				case SkillMagicType::SKILL_MAGIC_TYPE_BUFF:
				{
					uint8_t iBuffType = pkt.read<uint8_t>();

					auto it = m_mapActiveBuffList.find(iBuffType);

					if (it != m_mapActiveBuffList.end())
					{
						auto pSkillTable = Bootstrap::GetSkillTable().GetData();
						auto pSkillData = pSkillTable.find(it->second);

#ifdef _DEBUG
						if (pSkillData != pSkillTable.end())
							printf("RecvProcess::WIZ_MAGIC_PROCESS: %s %s buff removed\n", GetName().c_str(), pSkillData->second.szEngName.c_str());
						else
							printf("RecvProcess::WIZ_MAGIC_PROCESS: %s %d buff removed\n", GetName().c_str(), it->second);
#endif
					}

					switch (iBuffType)
					{
						case (byte)BuffType::BUFF_TYPE_SPEED:
							m_PlayerMySelf.iMoveSpeed = 45;
						break;
					}

					m_mapActiveBuffList.erase(iBuffType);
				}
				break;
			default:
#ifdef _DEBUG
				printf("RecvProcess::WIZ_MAGIC_PROCESS: %d Type Not Implemented!\n", iType);
#endif
				break;
			}
		}
		break;

		case WIZ_STATE_CHANGE:
		{
			int32_t iID = pkt.read<int32_t>();
			uint8_t iType = pkt.read<uint8_t>();
			uint64_t iBuff = pkt.read<uint64_t>();

			switch (iType)
			{
				case 3:
				{
					switch (iBuff)
					{
						case 4:
						{
							if (m_PlayerMySelf.iID == iID)
							{
#ifdef _DEBUG
								print("RecvProcess::WIZ_STATE_CHANGE: %s blinking start\n", m_PlayerMySelf.szName.c_str());
#endif

								m_PlayerMySelf.bBlinking = true;
							}
							else
							{

								auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
									[iID](const TPlayer& a) { return a.iID == iID; });

								if (it != m_vecPlayer.end())
								{
#ifdef _DEBUG
									printf("RecvProcess::WIZ_STATE_CHANGE: %s blinking start\n", it->szName.c_str());
#endif

									it->bBlinking = true;
								}
							}
						}
						break;

						case 7:
						{
							if (m_PlayerMySelf.iID == iID)
							{
#ifdef _DEBUG
								printf("RecvProcess::WIZ_STATE_CHANGE: %s blinking end\n", m_PlayerMySelf.szName.c_str());
#endif

								m_PlayerMySelf.bBlinking = false;
							}
							else
							{
								auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
									[iID](const TPlayer& a) { return a.iID == iID; });

								if (it != m_vecPlayer.end())
								{
#ifdef _DEBUG
									printf( "RecvProcess::WIZ_STATE_CHANGE: %s blinking end\n", it->szName.c_str());
#endif

									it->bBlinking = false;
								}
							}
						}
						break;

						default:
#ifdef _DEBUG
							printf("RecvProcess::WIZ_STATE_CHANGE: Abnormal Type - %llu Buff not implemented\n", iBuff);
#endif
							break;
					}
				}
				break;

				default:
#ifdef _DEBUG
					printf("RecvProcess::WIZ_STATE_CHANGE: Type %d not implemented\n", iType);
#endif
					break;
			}		
		}
		break;

		case WIZ_SHOPPING_MALL:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case ShoppingMallType::STORE_CLOSE:
				{
					for (int i = SLOT_MAX; i < SLOT_MAX + HAVE_MAX; i++)
					{
						m_PlayerMySelf.tInventory[i].iPos = i;
						m_PlayerMySelf.tInventory[i].iItemID = pkt.read<uint32_t>();
						m_PlayerMySelf.tInventory[i].iDurability = pkt.read<uint16_t>();
						m_PlayerMySelf.tInventory[i].iCount = pkt.read<uint16_t>();
						m_PlayerMySelf.tInventory[i].iFlag = pkt.read<uint8_t>();
						m_PlayerMySelf.tInventory[i].iRentalTime = pkt.read<int16_t>();
						m_PlayerMySelf.tInventory[i].iSerial = pkt.read<uint32_t>();
					}

#ifdef _DEBUG
					printf("RecvProcess::WIZ_SHOPPING_MALL: STORE_CLOSE\n");
#endif
				}
				break;
				default:
#ifdef _DEBUG
					printf("RecvProcess::WIZ_SHOPPING_MALL: Type %d not implemented\n", iType);
#endif
					break;
			}
		}
		break;

		case WIZ_ITEM_TRADE:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case 0:
				{
					uint8_t iErrorCode = pkt.read<uint8_t>();
#ifdef _DEBUG
					printf("RecvProcess::WIZ_ITEM_TRADE: Error Code: %d\n", iErrorCode);
#endif
				}
				break;

				case 1:
				{
					uint32_t iGold = pkt.read<uint32_t>();
					uint32_t iTransactionFee = pkt.read<uint32_t>();
					uint8_t iSellingGroup = pkt.read<uint8_t>();

					SendShoppingMall(ShoppingMallType::STORE_CLOSE);

#ifdef _DEBUG
					printf("RecvProcess::WIZ_ITEM_TRADE: %d,%d,%d\n", iGold, iTransactionFee, iSellingGroup);
#endif
				}
				break;

				default:
#ifdef _DEBUG
					printf("RecvProcess::WIZ_ITEM_TRADE: Type %d not implemented\n", iType);
#endif
					break;
			}
		}
		break;

		case WIZ_MAP_EVENT:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case 9:
				{
					m_bLunarWarDressUp = pkt.read<uint8_t>();
				}
				break;

				default:
#ifdef _DEBUG
					printf("RecvProcess::WIZ_MAP_EVENT: Type %d not implemented\n", iType);
#endif
					break;
			}
		}
		break;

		case WIZ_EXCHANGE:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case TradeSubPacket::TRADE_REQUEST:
				{
				}
				break;

				case TradeSubPacket::TRADE_AGREE:
				{
				}
				break;

				case TradeSubPacket::TRADE_ADD:
				{
				}
				break;

				case TradeSubPacket::TRADE_OTHER_ADD:
				{
				}
				break;

				case TradeSubPacket::TRADE_DECIDE:
				{
				}
				break;

				case TradeSubPacket::TRADE_OTHER_DECIDE:
				{
				}
				break;

				case TradeSubPacket::TRADE_DONE:
				{
					uint8_t iResult = pkt.read<uint8_t>();

					if (iResult == 1)
					{
						m_PlayerMySelf.iGold = pkt.read<uint32_t>();

						int16_t iItemCount = pkt.read<int16_t>();

						for (int32_t i = 0; i < iItemCount; i++)
						{
							uint8_t iItemPos = pkt.read<uint8_t>();

							m_PlayerMySelf.tInventory[14 + iItemPos].iItemID = pkt.read<uint32_t>();
							m_PlayerMySelf.tInventory[14 + iItemPos].iCount = pkt.read<int16_t>();
							m_PlayerMySelf.tInventory[14 + iItemPos].iDurability = pkt.read<int16_t>();

#ifdef _DEBUG
							printf("RecvProcess::WIZ_EXCHANGE: TRADE_DONE - Item - %d,%d,%d \n", 
								m_PlayerMySelf.tInventory[14 + iItemPos].iItemID, 
								m_PlayerMySelf.tInventory[14 + iItemPos].iCount,
								m_PlayerMySelf.tInventory[14 + iItemPos].iDurability);
#endif
						}

#ifdef _DEBUG
						printf("RecvProcess::WIZ_EXCHANGE: TRADE_DONE - Success - %d,%d \n", m_PlayerMySelf.iGold, iItemCount);
#endif
					}
#ifdef _DEBUG
					else
						printf("RecvProcess::WIZ_EXCHANGE: TRADE_DONE - Failed\n");
#endif
				}
				break;

				case TradeSubPacket::TRADE_CANCEL:
				{
				}
				break;

				default:
#ifdef _DEBUG
					printf( "RecvProcess::WIZ_EXCHANGE: Type %d not implemented\n", iType);
#endif
					break;
			}
		}
		break;

		case WIZ_ROTATE:
		{
			int32_t iID = pkt.read<int32_t>();
			float fRotation = pkt.read<int16_t>() / 100.0f;

			if (m_PlayerMySelf.iID == iID)
			{
				m_PlayerMySelf.fRotation = fRotation;

#ifdef _DEBUG
				printf("RecvProcess::WIZ_ROTATE: %s MySelf Rotate %f\n", m_PlayerMySelf.szName.c_str(), fRotation);
#endif
			}
			else
			{
				if (iID >= 5000)
				{
					auto it = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
						[iID](const TNpc& a) { return a.iID == iID; });

					if (it != m_vecNpc.end())
					{
						it->fRotation = fRotation;

#ifdef _DEBUG
						printf("RecvProcess::WIZ_ROTATE: %d Npc Rotate %f\n", iID, fRotation);
#endif
					}
				}
				else
				{
					auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
						[iID](const TPlayer& a) { return a.iID == iID; });

					if (it != m_vecPlayer.end())
					{
						it->fRotation = fRotation;

#ifdef _DEBUG
						printf("RecvProcess::WIZ_ROTATE: %s Player Rotate %f\n", it->szName.c_str(), fRotation);
#endif
					}
				}
			}
		}
		break;

		case WIZ_ITEM_DROP:
		{
			TLoot tLoot;
			memset(&tLoot, 0, sizeof(tLoot));

			tLoot.iNpcID = pkt.read<int32_t>();
			tLoot.iBundleID = pkt.read<uint32_t>();
			tLoot.iItemCount = pkt.read<uint8_t>();
	
			tLoot.msDropTime = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

			tLoot.iRequestedOpen = false;

			auto pLoot = std::find_if(m_vecLootList.begin(), m_vecLootList.end(),
				[tLoot](const TLoot a) { return a.iBundleID == tLoot.iBundleID; });

			if (pLoot == m_vecLootList.end())
				m_vecLootList.push_back(tLoot);
			else
				*pLoot = tLoot;

#ifdef _DEBUG
			printf("RecvProcess::WIZ_ITEM_DROP: %d,%d,%d,%lld\n", 
				tLoot.iNpcID, 
				tLoot.iBundleID, 
				tLoot.iItemCount, 
				tLoot.msDropTime.count());
#endif
		}
		break;

		case WIZ_BUNDLE_OPEN_REQ:
		{
			uint32_t iBundleID = pkt.read<uint32_t>();
			uint8_t iResult = pkt.read<uint8_t>();

			auto pLoot = std::find_if(m_vecLootList.begin(), m_vecLootList.end(),
				[iBundleID](const TLoot a) { return a.iBundleID == iBundleID; });

			if (pLoot != m_vecLootList.end())
			{
				switch (iResult)
				{
					case 0:
					{
#ifdef _DEBUG
						printf("RecvProcess::WIZ_BUNDLE_OPEN_REQ: Bundle open req failed\n");
#endif
					}
					break;

					case 1:
					{
						for (size_t i = 0; i < pLoot->iItemCount; i++)
						{
							uint32_t iItemID = pkt.read<uint32_t>();
							uint32_t iItemCount = pkt.read<int16_t>();

							SendBundleItemGet(iBundleID, iItemID, (int16_t)i);

#ifdef _DEBUG
							printf("RecvProcess::WIZ_BUNDLE_OPEN_REQ: %d,%d,%d,%d\n", iBundleID, iItemID, iItemCount, (int16_t)i);
#endif
						}
					}
					break;

					default:
#ifdef _DEBUG
						printf("RecvProcess::WIZ_BUNDLE_OPEN_REQ: Result %d not implemented\n", iResult);
#endif
					break;
				}

				m_vecLootList.erase(
					std::remove_if(m_vecLootList.begin(), m_vecLootList.end(),
						[iBundleID](const TLoot& a) { return a.iBundleID == iBundleID; }),
					m_vecLootList.end());
			}
		}
		break;

		case WIZ_ITEM_GET:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case 0x00:
				{
					uint32_t iBundleID = pkt.read<uint32_t>();

					m_vecLootList.erase(
						std::remove_if(m_vecLootList.begin(), m_vecLootList.end(),
							[iBundleID](const TLoot& a) { return a.iBundleID == iBundleID; }),
						m_vecLootList.end());

#ifdef _DEBUG
					printf("RecvProcess::WIZ_ITEM_GET: %d,%d\n", iType, iBundleID);
#endif
				}
				break;

				case 0x01:
				case 0x02:
				case 0x05:
				{
					uint32_t iBundleID = pkt.read<uint32_t>();
					uint8_t iPos = pkt.read<uint8_t>();
					uint32_t iItemID = pkt.read<uint32_t>();

					uint16_t iItemCount = 0;

					if (iType == 1 || iType == 5)
						iItemCount = pkt.read<uint16_t>();

					uint32_t iGold = pkt.read<uint32_t>();

					m_PlayerMySelf.tInventory[14 + iPos].iItemID = iItemID;
					m_PlayerMySelf.tInventory[14 + iPos].iCount = iItemCount;

					m_PlayerMySelf.iGold = iGold;

					m_vecLootList.erase(
						std::remove_if(m_vecLootList.begin(), m_vecLootList.end(),
							[iBundleID](const TLoot& a) { return a.iBundleID == iBundleID; }),
						m_vecLootList.end());

#ifdef _DEBUG
					printf("RecvProcess::WIZ_ITEM_GET: %d,%d,%d,%d,%d,%d\n", iType, iBundleID, iPos, iItemID, iItemCount, iGold);
#endif
				}
				break;

				case 0x03:
				{
					uint32_t iBundleID = pkt.read<uint32_t>();

					m_vecLootList.erase(
						std::remove_if(m_vecLootList.begin(), m_vecLootList.end(),
							[iBundleID](const TLoot& a) { return a.iBundleID == iBundleID; }),
						m_vecLootList.end());

#ifdef _DEBUG
					printf("RecvProcess::WIZ_ITEM_GET: %d,%d\n", iType, iBundleID);
#endif
				}
				break;

				case 0x06:
				{
#ifdef _DEBUG
					printf("RecvProcess::WIZ_ITEM_GET: Inventory Full\n");
#endif
				}
				break;

				default:
#ifdef _DEBUG
					printf("RecvProcess::WIZ_ITEM_GET: %d Type Not Implemented\n", iType);
#endif
					break;
			}
		}
		break;
	}

	//printf("Recv Packet: %s\n", pkt.toHex().c_str());
}

void Client::SendProcess(BYTE* byBuffer, DWORD dwLength)
{
	if (!m_bWorking)
		return;

	Packet pkt = Packet(byBuffer[0], (size_t)dwLength);
	pkt.append(&byBuffer[1], dwLength - 1);

	uint8_t byHeader;

	pkt >> byHeader;

	switch (byHeader)
	{
		case WIZ_HOME:
		{
#ifdef _DEBUG
			printf("SendProcess::WIZ_HOME\n");
#endif
			SetTarget(-1);		
		}
		break;

		case WIZ_ITEM_MOVE:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case 1:
				{
					uint8_t iDirection = pkt.read<uint8_t>();

					uint32_t iItemID = pkt.read<uint32_t>();

					uint8_t iCurrentPosition = pkt.read<uint8_t>();
					uint8_t iTargetPosition = pkt.read<uint8_t>();

					switch (iDirection)
					{
						case ITEM_INVEN_SLOT:
							break;
						case ITEM_SLOT_INVEN:
							break;
						case ITEM_INVEN_INVEN:
							break;
						case ITEM_SLOT_SLOT:
							break;
						case ITEM_INVEN_ZONE:
							break;
						case ITEM_ZONE_INVEN:
							break;
						case ITEM_INVEN_TO_COSP:
							break;
						case ITEM_COSP_TO_INVEN:
							break;
						case ITEM_INVEN_TO_MBAG:
							break;
						case ITEM_MBAG_TO_INVEN:
							break;
						case ITEM_MBAG_TO_MBAG:
							break;
						default:
#ifdef _DEBUG
							printf("SendProcess::WIZ_ITEM_MOVE: Direction %d not implemented\n", iDirection);
#endif
							break;
					}

#ifdef _DEBUG
					printf( "SendProcess::WIZ_ITEM_MOVE: iDirection(%d), iItemID(%d), iCurrentPosition(%d), iTargetPosition(%d)\n",
						iDirection,
						iItemID,
						iCurrentPosition,
						iTargetPosition
					);
#endif
				}
				break;

				default:
#ifdef _DEBUG
					printf("SendProcess::WIZ_ITEM_MOVE: Type %d not implemented\n", iType);
#endif
				break;
			}
		}
		break;
	}

	//printf("Send Packet: %s\n", pkt.toHex().c_str());
}

void Client::UseSkill(TABLE_UPC_SKILL pSkillData, int32_t iTargetID)
 {
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
					SendMovePacket(GetPosition(), GetPosition(), 0, 0);
				}

				SendStartSkillCastingAtTargetPacket(pSkillData, iTargetID);
				std::this_thread::sleep_for(std::chrono::milliseconds(pSkillData.iCastTime * 100));
			}

			auto pSkillExtension2 = Bootstrap::GetSkillExtension2Table().GetData();
			auto pSkillExtension2Data = pSkillExtension2.find(pSkillData.iID);

			uint32_t iArrowCount = 0;

			if (pSkillExtension2Data != pSkillExtension2.end())
				iArrowCount = pSkillExtension2Data->second.iArrowCount;

			Vector3 v3TargetPosition = GetTargetPosition();

			if ((iArrowCount == 0 && pSkillData.iFlyingFX != 0) || iArrowCount == 1)
				SendStartFlyingAtTargetPacket(pSkillData, iTargetID, v3TargetPosition);

			if (iArrowCount > 1)
			{
				SendStartFlyingAtTargetPacket(pSkillData, iTargetID, Vector3(0.0f, 0.0f, 0.0f), 1);

				float fDistance = GetDistance(GetTargetPosition());

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
					SendMovePacket(GetPosition(), GetPosition(), 0, 0);
				}

				SendStartSkillCastingAtPosPacket(pSkillData, GetTargetPosition());
				std::this_thread::sleep_for(std::chrono::milliseconds(pSkillData.iCastTime * 100));
			}

			if (pSkillData.iFlyingFX != 0)
				SendStartFlyingAtTargetPacket(pSkillData, iTargetID, GetTargetPosition());

			SendStartSkillMagicAtPosPacket(pSkillData, GetTargetPosition());
		}
		break;
	}

	Client::SetSkillUseTime(pSkillData.iID, 
		duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
	);
 }

void Client::SendTownPacket()
{
	Packet pkt = Packet(WIZ_HOME);

	SendPacket(pkt);
}

void Client::SendBasicAttackPacket(int32_t iTargetID, float fInterval, float fDistance)
{
	Packet pkt = Packet(WIZ_ATTACK);

	pkt
		<< uint8_t(1) << uint8_t(1)
		<< iTargetID
		<< uint16_t(fInterval * 100.0f)
		<< uint16_t(fDistance * 10.0f)
		<< uint8_t(0) << uint8_t(0);

	SendPacket(pkt);
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

	pkt << arrowIndex 
		<< uint32_t(0) << uint32_t(0) << uint32_t(0) << uint32_t(0);

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
		<< uint32_t(0) << uint32_t(0) << uint32_t(0);

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

void Client::SendMovePacket(Vector3 vecStartPosition, Vector3 vecTaragetPosition, int16_t iMoveSpeed, uint8_t iMoveType)
{
	Packet pkt = Packet(WIZ_MOVE);

	pkt
		<< uint16_t(vecStartPosition.m_fX * 10.0f)
		<< uint16_t(vecStartPosition.m_fY * 10.0f)
		<< int16_t(vecStartPosition.m_fZ * 10.0f)
		<< iMoveSpeed
		<< iMoveType
		<< uint16_t(vecTaragetPosition.m_fX * 10.0f)
		<< uint16_t(vecTaragetPosition.m_fY * 10.0f)
		<< int16_t(vecTaragetPosition.m_fZ * 10.0f);

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