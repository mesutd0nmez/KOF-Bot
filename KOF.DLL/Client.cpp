#include "pch.h"
#include "Ini.h"
#include "Client.h"
#include "Memory.h"
#include "AttackHandler.h"
#include "ProtectionHandler.h"

Client::Client()
{
	printf("Client::Initialize\n");

	m_mapAddressList.clear();

	m_byState = BOOTSTRAP;
	m_bWorking = false;

	m_dwRecvHookAddress = 0;
	m_dwSendHookAddress = 0;

	memset(&m_PlayerMySelf, 0, sizeof(m_PlayerMySelf));

	m_bCharacterLoaded = false;

	m_mapNpc.clear();
	m_mapPlayer.clear();
}

Client::~Client()
{
	printf("Client::Destroy\n");

	m_mapAddressList.clear();

	m_byState = BOOTSTRAP;
	m_bWorking = false;

	m_dwRecvHookAddress = 0;
	m_dwSendHookAddress = 0;

	memset(&m_PlayerMySelf, 0, sizeof(m_PlayerMySelf));

	m_bCharacterLoaded = false;

	m_mapNpc.clear();
	m_mapPlayer.clear();
}

void Client::Start()
{
	printf("Client::Start\n");

	m_bWorking = true;

	new std::thread([]() { MainProcess(); });
	new std::thread([]() { HookProcess(); });
}

void Client::Stop()
{
	printf("Client::Stop\n");

	m_bWorking = false;
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

	printf("LoadUserConfig: %s loaded\n", Client::GetName().c_str());
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
	printf("Client::HookProcess\n");

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
	printf("Client::MainProcess\n");

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	HookSendAddress();

	if (Client::IsCharacterLoaded())
		SetState(State::GAME);

	AttackHandler::Start();
	ProtectionHandler::Start();

	while (m_bWorking)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		switch (GetState())
		{
			case State::BOOTSTRAP: BootstrapProcess(); break;
			case State::LOST: LostProcess(); break;
			case State::GAME: GameProcess(); break;
		}
	}
}

void Client::BootstrapProcess()
{
	while (!Client::IsIntroPhase())
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

	Client::PushPhase(Client::GetAddress("KO_PTR_LOGIN"));

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	while (!Client::IsLoginPhase())
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	Client::SetLoginInformation("colinkazim", "ttCnkoSh1993");
	Client::ConnectLoginServer();

	Client::SetState(State::LOGIN);
}

void Client::LostProcess()
{
}

void Client::GameProcess()
{
	std::string strCharacterName = GetName();

	if (Client::IsDisconnect())
	{
		fprintf(stdout, "%s Connection lost\n", strCharacterName.c_str());
		Client::SetState(State::LOST);
	}
	else
	{
		if (Client::IsCharacterLoaded())
		{
			std::string strCharacterName = GetName();

			if (!IsUserConfigLoaded(strCharacterName))
				LoadUserConfig(strCharacterName);
		}
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

bool Client::IsCharacterLoaded()
{
	return m_bCharacterLoaded;
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

DWORD Client::GetGoX()
{
	return Memory::Read4Byte(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_GOX"));
}

DWORD Client::GetGoY()
{
	return Memory::Read4Byte(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_GOY"));
}

float Client::GetX()
{
	return m_PlayerMySelf.fX;
}

float Client::GetY()
{
	return m_PlayerMySelf.fY;
}

float Client::GetZ()
{
	return m_PlayerMySelf.fZ;
}

uint8_t Client::GetSkillPoint(int32_t Slot)
{
	return m_PlayerMySelf.iSkillInfo[Slot];
}

void Client::PushPhase(DWORD address)
{
	PushPhaseCall pPushPhaseCallFnc = (PushPhaseCall)GetAddress("KO_PTR_PUSH_PHASE");
	pPushPhaseCallFnc(*reinterpret_cast<int*>(address));
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
		LoginCall1 pLoginCallFnc1 = (LoginCall1)GetAddress("KO_PTR_LOGIN_DC");
		pLoginCallFnc1(*reinterpret_cast<DWORD*>(GetAddress("KO_PTR_LOGIN")));
	}

	LoginCall1 pLoginCallFnc1 = (LoginCall1)GetAddress("KO_PTR_LOGIN1");
	pLoginCallFnc1(*reinterpret_cast<DWORD*>(GetAddress("KO_PTR_LOGIN")));

	LoginCall2 pLoginCallFnc2 = (LoginCall2)GetAddress("KO_PTR_LOGIN2");
	pLoginCallFnc2(*reinterpret_cast<DWORD*>(GetAddress("KO_PTR_LOGIN")));
}

void Client::ConnectGameServer(BYTE byServerId)
{
	DWORD dwCGameProcIntroLogin = Memory::Read4Byte(GetAddress("KO_PTR_LOGIN"));
	DWORD dwCUILoginIntro = Memory::Read4Byte(dwCGameProcIntroLogin + GetAddress("KO_OFF_UI_LOGIN_INTRO"));

	Memory::Write4Byte(dwCUILoginIntro + GetAddress("KO_OFF_LOGIN_SERVER_INDEX"), byServerId);

	LoginServerCall pLoginServerCallFnc = (LoginServerCall)GetAddress("KO_PTR_SERVER_SELECT");
	pLoginServerCallFnc(*reinterpret_cast<DWORD*>(GetAddress("KO_PTR_LOGIN")));
}

void Client::SelectCharacterSkip()
{
	CharacterSelectSkipCall pCharacterSelectEnterCallFnc = (CharacterSelectSkipCall)GetAddress("KO_PTR_CHARACTER_SELECT_SKIP");
	pCharacterSelectEnterCallFnc(*reinterpret_cast<DWORD*>(GetAddress("KO_PTR_CHARACTER_SELECT")));
}

void Client::SelectCharacter(BYTE byCharacterIndex)
{
	CharacterSelectCall pCharacterSelectCallFnc = (CharacterSelectCall)GetAddress("KO_PTR_CHARACTER_SELECT_ENTER");
	pCharacterSelectCallFnc(*reinterpret_cast<DWORD*>(GetAddress("KO_PTR_CHARACTER_SELECT")));
}

void Client::SendPacket(Packet vecBuffer)
{
	Send pSendFnc = (Send)GetAddress("KO_SND_FNC");
	pSendFnc(*reinterpret_cast<DWORD*>(GetAddress("KO_PTR_PKT")), vecBuffer.contents(), vecBuffer.size());
}

void Client::RouteStart(float fX, float fY, float fZ)
{
	Vector3 *pVector3 = new Vector3(fX, fZ, fY);
	RouteStartCall pRouteStartCallFnc = (RouteStartCall)GetAddress("KO_PTR_ROUTE_START_CALL");
	pRouteStartCallFnc(*reinterpret_cast<int*>(GetAddress("KO_PTR_CHR")), pVector3);
}

void Client::Move(int32_t iX, int32_t iY)
{
	MoveCall pMoveCallFnc = (MoveCall)0x654A10;
	pMoveCallFnc(*reinterpret_cast<int*>(GetAddress("KO_PTR_CHR")), iX, iY);
}

void Client::Town()
{
	SendPacket(Packet(WIZ_HOME));
}

DWORD Client::GetRecvHookAddress()
{
	return m_dwRecvHookAddress;
};

DWORD Client::GetSendHookAddress()
{
	return m_dwSendHookAddress;
};

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

	fprintf(stdout, "HookRecvAddress: Patched\n");
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

	printf("HookSendAddress: Patched\n");
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
	tNpc.fZ = (pkt.read<uint16_t>() / 10.0f);

	tNpc.iStatus = pkt.read<uint32_t>();

	tNpc.iUnknown2 = pkt.read<uint8_t>();
	tNpc.iUnknown3 = pkt.read<uint32_t>();

	tNpc.iRotation = pkt.read<uint16_t>();

	return tNpc;
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
					printf("RecvProcess::LS_LOGIN_REQ: Account Banned\n");
					break;

				case AUTH_IN_GAME:
				{
					printf("RecvProcess::LS_LOGIN_REQ: Account already in-game\n");

					new std::thread([]()
					{
						printf("RecvProcess::LS_LOGIN_REQ: Reconnecting login server\n");
						Client::ConnectLoginServer(true);
					});
				}
				break;

				case AUTH_SUCCESS:
					printf("RecvProcess::LS_LOGIN_REQ: Login Success\n");
					break;
				default:
					printf("RecvProcess::LS_LOGIN_REQ: %d not implemented!\n", byResult);
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
				printf("RecvProcess::LS_SERVERLIST: %d Server loaded\n", byServerCount);

				new std::thread([]()
				{
					printf("RecvProcess::LS_SERVERLIST: Connecting to server: %d\n", 1);

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
				printf("RecvProcess::WIZ_ALLCHAR_INFO_REQ: Character list loaded\n");

				new std::thread([]()
				{
					printf("RecvProcess::WIZ_ALLCHAR_INFO_REQ: Selecting character %d\n", 1);

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

			// KNIGHT DATA START

			m_PlayerMySelf.iKnightsID = pkt.read<int16_t>();
			e_KnightsDuty eKnightsDuty = (e_KnightsDuty)pkt.read<uint8_t>();

			int16_t iAllianceID = pkt.read<int16_t>();
			uint8_t byFlag = pkt.read<uint8_t>();

			uint8_t iKnightNameLen = pkt.read<uint8_t>();
			std::string szKnightsName;
			pkt.readString(szKnightsName, iKnightNameLen);

			m_PlayerMySelf.iKnightsGrade = pkt.read<uint8_t>();
			m_PlayerMySelf.iKnightsRank = pkt.read<uint8_t>();

			int16_t sMarkVersion = pkt.read<int16_t>();
			int16_t sCapeID = pkt.read<int16_t>();

			uint8_t iR = pkt.read<uint8_t>();
			uint8_t iG = pkt.read<uint8_t>();
			uint8_t iB = pkt.read<uint8_t>();

			uint8_t iUnknown1 = pkt.read<uint8_t>();

			// KNIGHT DATA END

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

			m_bCharacterLoaded = true;

			printf("RecvProcess::WIZ_MYINFO: %s loaded\n", m_PlayerMySelf.szName.c_str());

			Client::SetState(State::GAME);
		}
		break;

		case WIZ_HP_CHANGE:
		{
			m_PlayerMySelf.iHPMax = pkt.read<int16_t>();
			m_PlayerMySelf.iHP = pkt.read<int16_t>();

			printf("RecvProcess::WIZ_HP_CHANGE: %d / %d\n", m_PlayerMySelf.iHP, m_PlayerMySelf.iHPMax);
		}
		break;

		case WIZ_MSP_CHANGE:
		{
			m_PlayerMySelf.iMSPMax = pkt.read<int16_t>();
			m_PlayerMySelf.iMSP = pkt.read<int16_t>();

			printf("RecvProcess::WIZ_MSP_CHANGE: %d / %d\n", m_PlayerMySelf.iMSP, m_PlayerMySelf.iMSPMax);
		}
		break;

		case WIZ_EXP_CHANGE:
		{
			uint8_t iUnknown1 = pkt.read<uint8_t>();
			m_PlayerMySelf.iExp = pkt.read<uint64_t>();

			printf("RecvProcess::WIZ_EXP_CHANGE: %llu\n", m_PlayerMySelf.iExp);
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

				printf("RecvProcess::WIZ_LEVEL_CHANGE: %s %d\n", m_PlayerMySelf.szName.c_str(), iLevel);
			}
			else
			{
				auto it = m_mapPlayer.find(iID);

				if (it != m_mapPlayer.end())
				{
					it->second.iLevel = iLevel;

					printf("RecvProcess::WIZ_LEVEL_CHANGE: %s %d\n", it->second.szName.c_str(), iLevel);
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
					printf("RecvProcess::WIZ_POINT_CHANGE: STR %d\n", iVal);
				}
				break;

				case 0x02:
				{
					m_PlayerMySelf.iStamina = (uint8_t)iVal;
					printf("RecvProcess::WIZ_POINT_CHANGE: HP %d\n", iVal);
				}
				break;

				case 0x03:
				{
					m_PlayerMySelf.iDexterity = (uint8_t)iVal;
					printf("RecvProcess::WIZ_POINT_CHANGE: DEX %d\n", iVal);
				}
				break;

				case 0x04:
				{
					m_PlayerMySelf.iIntelligence = (uint8_t)iVal;
					printf("RecvProcess::WIZ_POINT_CHANGE: INT %d\n", iVal);
				}
				break;

				case 0x05:
				{
					m_PlayerMySelf.iMagicAttak = (uint8_t)iVal;
					printf("RecvProcess::WIZ_POINT_CHANGE: MP %d\n", iVal);
				}
				break;
			}

			if (iType >= 1 && iType <= 5)
			{
				m_PlayerMySelf.iBonusPointRemain--;
				printf("RecvProcess::WIZ_POINT_CHANGE: POINT %d\n", m_PlayerMySelf.iBonusPointRemain);
			}
		}
		break;

		case WIZ_WEIGHT_CHANGE:
		{
			m_PlayerMySelf.iWeight = pkt.read<uint32_t>();
			printf("RecvProcess::WIZ_WEIGHT_CHANGE: %d\n", m_PlayerMySelf.iWeight);
		}
		break;

		case WIZ_DURATION:
		{
			uint8_t iPos = pkt.read<uint8_t>();
			uint16_t iDurability = pkt.read<uint16_t>();

			m_PlayerMySelf.tInventory[iPos].iDurability = iDurability;

			printf("RecvProcess::WIZ_DURATION: %d,%d\n", iPos, iDurability);
		}
		break;

		case WIZ_ITEM_REMOVE:
		{
			uint8_t	iResult = pkt.read<uint8_t>();

			switch (iResult)
			{
				case 0x00: 
					printf("RecvProcess::WIZ_ITEM_REMOVE: 0\n");
					break;

				case 0x01: 
					printf("RecvProcess::WIZ_ITEM_REMOVE: 1\n");
					break;

				default:
					printf("RecvProcess::WIZ_ITEM_REMOVE: %d Result Not Implemented\n", iResult);
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

			}
		}
		break;

		case WIZ_SKILLPT_CHANGE:
		{
			int iType = pkt.read<uint8_t>();
			int iValue = pkt.read<uint8_t>();

			m_PlayerMySelf.iSkillInfo[iType] = iValue;
			m_PlayerMySelf.iSkillInfo[0]++;

			printf("RecvProcess::WIZ_ITEM_REMOVE: %d,%d,%d\n", iType, iValue, m_PlayerMySelf.iSkillInfo[0]);
		}
		break;

		case WIZ_CLASS_CHANGE:
		{
			uint8_t	iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case N3_SP_CLASS_CHANGE_PURE:
					printf("RecvProcess::WIZ_CLASS_CHANGE: N3_SP_CLASS_CHANGE_PURE\n");
					break;

				case N3_SP_CLASS_CHANGE_REQ:
				{
					e_SubPacket_ClassChange eSP = (e_SubPacket_ClassChange)pkt.read<uint8_t>();

					switch (eSP)
					{
						case N3_SP_CLASS_CHANGE_SUCCESS:
							printf("RecvProcess::WIZ_CLASS_CHANGE: N3_SP_CLASS_CHANGE_SUCCESS\n");
							break;

						case N3_SP_CLASS_CHANGE_NOT_YET:
							printf("RecvProcess::WIZ_CLASS_CHANGE: N3_SP_CLASS_CHANGE_NOT_YET\n");
							break;

						case N3_SP_CLASS_CHANGE_ALREADY:
							printf("RecvProcess::WIZ_CLASS_CHANGE: N3_SP_CLASS_CHANGE_ALREADY\n");
							break;

						case N3_SP_CLASS_CHANGE_FAILURE:
							printf("RecvProcess::WIZ_CLASS_CHANGE: N3_SP_CLASS_CHANGE_FAILURE\n");
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

							printf("RecvProcess::WIZ_CLASS_CHANGE: Skill point reset, new points: %d\n", 
								m_PlayerMySelf.iSkillInfo[0]);
						}
						break;

						case 0x02: break;
					}
				}
				break;

				case N3_SP_CLASS_POINT_CHANGE_PRICE_QUERY:
				{
					uint32_t iGold = pkt.read<uint32_t>();
					printf("RecvProcess::WIZ_CLASS_CHANGE: Point change price %d\n", iGold);
				}
				break;

				case N3_SP_CLASS_PROMOTION:
				{
					uint16_t iClass = pkt.read<uint16_t>();
					uint32_t iID = pkt.read<uint32_t>();

					if (m_PlayerMySelf.iID == iID)
					{
						m_PlayerMySelf.eClass = (e_Class)iClass;
						printf("RecvProcess::WIZ_CLASS_CHANGE: %s class changed to %d\n", 
							m_PlayerMySelf.szName.c_str(), m_PlayerMySelf.eClass);
					}
					else
					{
						auto it = m_mapPlayer.find(iID);

						if (it != m_mapPlayer.end())
						{
							it->second.eClass = (e_Class)iClass;

							printf("RecvProcess::WIZ_CLASS_CHANGE: %s class changed to %d\n",
								it->second.szName.c_str(), it->second.eClass);
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

			printf("RecvProcess::WIZ_GOLD_CHANGE: %d,%d,%d\n", iType, iGoldOffset, iGold);
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
				}
				else
				{
					printf("RecvProcess::WIZ_ITEM_MOVE: %d SubType Not Implemented\n", iSubType);
				}
			}
			else
			{
				printf("RecvProcess::WIZ_ITEM_MOVE: %d Type Not Implemented\n", iType);
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

				printf("RecvProcess::WIZ_LOYALTY_CHANGE: %d,%d,%d,Unknown1(%d),%d\n", 
					bType, iLoyalty, iLoyaltyMonthly, iUnknown1, iClanLoyaltyAmount);
			}
			else
			{
				printf("RecvProcess::WIZ_LOYALTY_CHANGE: %d Type Not Implemented\n", bType);
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
					printf("RecvProcess::WIZ_ITEM_GET: %d,%d\n", iType, iBundleID);
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

					printf("RecvProcess::WIZ_ITEM_GET: %d,%d,%d,%d,%d,%d\n", iType, iBundleID, iPos, iItemID, iItemCount, iGold);
				}
				break;

				case 0x03:
				{
					uint32_t iBundleID = pkt.read<uint32_t>();
					printf("RecvProcess::WIZ_ITEM_GET: %d,%d\n", iType, iBundleID);
				}
				break;

				case 0x06:
				{
					printf("RecvProcess::WIZ_ITEM_GET: Inventory Full\n");
				}
				break;
			default:
				printf("RecvProcess::WIZ_ITEM_GET: %d Type Not Implemented\n", iType);
				break;
			}
		}
		break;

		case WIZ_GAMESTART:
		{
			printf("RecvProcess::WIZ_GAMESTART: Started\n");

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

				auto it = m_mapNpc.find(pNpc.iID);

				if (it == m_mapNpc.end())
					m_mapNpc.insert(std::pair(pNpc.iID, pNpc));
				else
					it->second = pNpc;
			}

			printf("RecvProcess::WIZ_REQ_NPCIN: Size %d\n", iNpcCount);
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

					auto it = m_mapNpc.find(pNpc.iID);

					if (it == m_mapNpc.end())
						m_mapNpc.insert(std::pair(pNpc.iID, pNpc));
					else
						it->second = pNpc;

					printf("RecvProcess::WIZ_NPC_INOUT: IN %d\n", pNpc.iID);
				}
				break;

				case InOut::INOUT_OUT:
				{
					int32_t iNpcId = pkt.read<int32_t>();

					auto it = m_mapNpc.find(iNpcId);

					if (it != m_mapNpc.end())
						m_mapNpc.erase(it->first);

					printf("RecvProcess::WIZ_NPC_INOUT: OUT %d\n", iNpcId);
				}
				break;

				default:
					printf("WIZ_NPC_INOUT: %d not implemented\n", iType);
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

			std::set<int32_t> iSetNpcID;

			for (int16_t i = 0; i < iNpcCount; i++)
			{
				int32_t iID = pkt.read<int32_t>();
				iSetNpcID.insert(iID);
			}

			if (iSetNpcID.empty())
			{
				printf("RecvProcess::WIZ_NPC_REGION: All npc cleared\n");
				m_mapNpc.clear();
				return;
			}

			for (auto it = m_mapNpc.begin(); it != m_mapNpc.end();)
			{
				auto iNpcId = iSetNpcID.find(it->first);

				if (iNpcId != iSetNpcID.end())
				{
					iSetNpcID.erase(iNpcId);

					++it;
				}
				else
				{
					if ((PSA_DYING == it->second.eState || PSA_DEATH == it->second.eState))
						++it; 
					else
						m_mapNpc.erase(it++);
				}
			}

			printf("RecvProcess::WIZ_NPC_REGION: New npc count %d\n", iSetNpcID.size());
		}
		break;

		case WIZ_DEAD:
		{
			int32_t iID = pkt.read<int32_t>();

			if (m_PlayerMySelf.iID == iID)
			{
				m_PlayerMySelf.eState = PSA_DEATH;

				printf("RecvProcess::WIZ_DEAD: MySelf Dead\n");
			}
			else
			{
				if (iID >= 5000)
				{
					auto it = m_mapNpc.find(iID);

					if (it != m_mapNpc.end())
					{
						it->second.eState = PSA_DEATH;

						printf("RecvProcess::WIZ_DEAD: %d Npc Dead\n", iID);
					}
				}
				else
				{
					auto it = m_mapPlayer.find(iID);

					if (it != m_mapPlayer.end())
					{
						it->second.eState = PSA_DEATH;

						printf("RecvProcess::WIZ_DEAD: %d Player Dead\n", iID);
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
					printf("RecvProcess::WIZ_ATTACK: FAIL\n");
				}
				break;

				case ATTACK_SUCCESS:
				{
					printf("RecvProcess::WIZ_ATTACK: SUCCESS\n");
				}
				break;

				case ATTACK_TARGET_DEAD:
				case ATTACK_TARGET_DEAD_OK:
				{
					printf("RecvProcess::WIZ_ATTACK: TARGET_DEAD | TARGET_DEAD_OK\n");
				}
				break;

				default:
					printf("RecvProcess::WIZ_ATTACK: %d not implemented\n", iResult);
					break;

			}
		}
		break;

		case WIZ_TARGET_HP:
		{
			int32_t iID = pkt.read<int32_t>();
			uint8_t iUpdateImmediately = pkt.read<uint8_t>();
			uint32_t iTargetHPMax = pkt.read<uint32_t>();
			uint32_t iTargetHPCur = pkt.read<uint32_t>();
			int16_t iTargetHPChange = pkt.read<int16_t>();

			if (m_PlayerMySelf.iID == iID)
			{
				m_PlayerMySelf.iHPMax = (int16_t)iTargetHPMax;
				m_PlayerMySelf.iHP = (int16_t)iTargetHPCur;

				printf("RecvProcess::WIZ_TARGET_HP: %s, %d / %d\n", 
					m_PlayerMySelf.szName.c_str(), m_PlayerMySelf.iHP, m_PlayerMySelf.iHPMax);
			}
			else
			{
				if (iID >= 5000)
				{
					auto it = m_mapNpc.find(iID);

					if (it != m_mapNpc.end())
					{
						it->second.iHPMax = (int16_t)iTargetHPMax;
						it->second.iHP = (int16_t)iTargetHPCur;

						printf("RecvProcess::WIZ_TARGET_HP: %d, %d / %d\n", iID, it->second.iHP, it->second.iHPMax);
					}
				}
				else
				{
					auto it = m_mapPlayer.find(iID);

					if (it != m_mapPlayer.end())
					{
						it->second.iHPMax = (int16_t)iTargetHPMax;
						it->second.iHP = (int16_t)iTargetHPCur;

						printf("RecvProcess::WIZ_TARGET_HP: %s, %d / %d\n", 
							it->second.szName.c_str(), it->second.iHP, it->second.iHPMax);
					}
				}
			}
		}
		break;
	}

	//fprintf(stdout, "Recv Packet: %s\n", pkt.toHex().c_str());
}

void Client::SendProcess(BYTE* byBuffer, DWORD dwLength)
{
	if (!m_bWorking)
		return;

	auto pkt = Packet(byBuffer[0], (size_t)dwLength);
	pkt.append(&byBuffer[1], dwLength - 1);

	uint8_t byHeader = pkt.GetOpcode();

	//switch (byHeader)
	//{
	//	default:
	//		break;
	//}

	//fprintf(stdout, "Send Packet: %s\n", pkt.toHex().c_str());


}