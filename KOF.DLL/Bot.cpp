#include "pch.h"
#include "Bot.h"
#include "ClientHandler.h"
#include "UI.h"
#include "Memory.h"

Bot::Bot()
{
	m_ClientHandler = nullptr;

	m_hInjectedProcess = nullptr;
	m_dwInjectedProcessID = 0;
}

Bot::~Bot()
{
	m_ClientHandler = nullptr;

	m_hInjectedProcess = nullptr;
	m_dwInjectedProcessID = 0;
}

void Bot::Initialize()
{
#ifdef _WINDLL
	m_hInjectedProcess = GetCurrentProcess();
	m_dwInjectedProcessID = GetCurrentProcessId();
#endif

	InitializeService();
}

ClientHandler* Bot::GetClientHandler()
{
	return m_ClientHandler;
}

void Bot::InitializeStaticData()
{
#ifdef DEBUG
	printf("InitializeStaticData: Started\n");
#endif

#ifdef DEBUG
	printf("InitializeStaticData: Loading skill & skill extension data\n");
#endif

	std::string szDevelopmentPath = DEVELOPMENT_PATH;

	m_pTbl_Skill.Load(szDevelopmentPath + ".\\Data\\skill_magic_main_us.tbl");
	m_pTbl_Skill_Extension1.Load(szDevelopmentPath + ".\\Data\\skill_magic_1.tbl");
	m_pTbl_Skill_Extension2.Load(szDevelopmentPath + ".\\Data\\skill_magic_2.tbl");
	m_pTbl_Skill_Extension3.Load(szDevelopmentPath + ".\\Data\\skill_magic_3.tbl");
	m_pTbl_Skill_Extension4.Load(szDevelopmentPath + ".\\Data\\skill_magic_4.tbl");
	m_pTbl_Skill_Extension5.Load(szDevelopmentPath + ".\\Data\\skill_magic_5.tbl");
	m_pTbl_Skill_Extension6.Load(szDevelopmentPath + ".\\Data\\skill_magic_6.tbl");
	m_pTbl_Skill_Extension7.Load(szDevelopmentPath + ".\\Data\\skill_magic_7.tbl");
	m_pTbl_Skill_Extension8.Load(szDevelopmentPath + ".\\Data\\skill_magic_8.tbl");
	m_pTbl_Skill_Extension9.Load(szDevelopmentPath + ".\\Data\\skill_magic_9.tbl");

#ifdef DEBUG
	printf("InitializeStaticData: Loaded %d skills\n", m_pTbl_Skill.GetDataSize());
#endif

#ifdef DEBUG
	printf("InitializeStaticData: Loading custom skills\n");
#endif

	TABLE_UPC_SKILL pGodMode;
	memset(&pGodMode, 0, sizeof(pGodMode));

	pGodMode.iID = 500344;
	pGodMode.szName = "God Mode";
	pGodMode.szEngName = "God Mode";
	pGodMode.iSelfAnimID1 = -1;
	pGodMode.iSelfFX1 = 2401;
	pGodMode.iSelfPart1 = -1;
	pGodMode.iTarget = 9;
	pGodMode.iCooldown = 10;
	pGodMode.dw1stTableType = 4;
	pGodMode.dw2ndTableType = 0;
	pGodMode.iBaseId = 490006;

	m_pTbl_Skill.Insert(pGodMode.iID, pGodMode);

	TABLE_UPC_SKILL_EXTENSION4 pGodModeExtension4;
	memset(&pGodModeExtension4, 0, sizeof(pGodModeExtension4));

	pGodModeExtension4.iID = pGodMode.iID;
	pGodModeExtension4.iBuffType = 1;
	pGodModeExtension4.iAreaRadius = 0;
	pGodModeExtension4.iBuffDuration = 3600;

	m_pTbl_Skill_Extension4.Insert(pGodModeExtension4.iID, pGodModeExtension4);

#ifdef DEBUG
	printf("InitializeStaticData: Loaded custom skills\n");
#endif

#ifdef DEBUG
	printf("InitializeStaticData: Loading item data\n");
#endif

	m_pTbl_Item.Load(szDevelopmentPath + ".\\Data\\item_org_nc.tbl");

#ifdef DEBUG
	printf("InitializeStaticData: Loaded %d items\n", m_pTbl_Item.GetDataSize());
#endif

#ifdef DEBUG
	printf("InitializeStaticData: Loading npc data\n");
#endif

	m_pTbl_Npc.Load(szDevelopmentPath + ".\\Data\\npc_us.tbl");

#ifdef DEBUG
	printf("InitializeStaticData: Loaded %d npcs\n", m_pTbl_Npc.GetDataSize());
#endif

#ifdef DEBUG
	printf("InitializeStaticData: Loading mob data\n");
#endif

	m_pTbl_Mob.Load(szDevelopmentPath + ".\\Data\\mob_us.tbl");

#ifdef DEBUG
	printf("InitializeStaticData: Loaded %d mobs\n", m_pTbl_Mob.GetDataSize());
#endif

#ifdef DEBUG
	printf("InitializeStaticData: finished\n");
#endif
}

void Bot::OnReady()
{
	
}

void Bot::OnPong()
{
	if(GetClientHandler()->GetName().size() > 0)
		SendSaveUserConfiguration(1, GetClientHandler()->GetName());
}

void Bot::OnAuthenticated()
{

	new std::thread([this]() { InitializeStaticData(); });
}

void Bot::OnLoaded()
{
#ifndef _WINDLL
	std::ostringstream strCommandLine;
	strCommandLine << GetCurrentProcessId();

	PROCESS_INFORMATION processInfo;

	StartProcess(DEVELOPMENT_PATH, "KnightOnLine.exe", strCommandLine.str(), processInfo);

	std::this_thread::sleep_for(std::chrono::milliseconds(2000));

	m_hInjectedProcess = processInfo.hProcess;
	m_dwInjectedProcessID = processInfo.dwProcessId;
#endif

	m_ClientHandler = new ClientHandler(this);
	m_ClientHandler->InitializeHandler();
}

void Bot::OnConfigurationLoaded()
{
	if (m_ClientHandler == nullptr)
	{
#ifdef DEBUG
		printf("m_ClientHandler == nullptr\n");
#endif
		return;
	}

#ifdef DEBUG
	printf("User configuration loaded\n");
#endif

	m_ClientHandler->SetConfigurationLoaded(true);
	m_ClientHandler->StartHandler();

	new std::thread([this]() { UI::Render(this); });
}

DWORD Bot::GetAddress(std::string szAddressName)
{
	std::string szAddress = m_iniPointer.GetString("Address", szAddressName.c_str(), "0x000000");
	return std::strtoul(szAddress.c_str(), NULL, 16);
}

Ini* Bot::GetConfiguration()
{
	return &m_iniUserConfiguration;
}

BYTE Bot::ReadByte(DWORD dwAddress)
{
	return Memory::ReadByte(m_hInjectedProcess, dwAddress);
}

DWORD Bot::Read4Byte(DWORD dwAddress)
{
	return Memory::Read4Byte(m_hInjectedProcess, dwAddress);
}

float Bot::ReadFloat(DWORD dwAddress)
{
	return Memory::ReadFloat(m_hInjectedProcess, dwAddress);
}

std::string Bot::ReadString(DWORD dwAddress, size_t nSize)
{
	return Memory::ReadString(m_hInjectedProcess, dwAddress, nSize);
}

std::vector<BYTE> Bot::ReadBytes(DWORD dwAddress, size_t nSize)
{
	return Memory::ReadBytes(m_hInjectedProcess, dwAddress, nSize);
}

void Bot::WriteByte(DWORD dwAddress, DWORD dwValue)
{
	Memory::WriteByte(m_hInjectedProcess, dwAddress, dwValue);
}

void Bot::Write4Byte(DWORD dwAddress, DWORD dwValue)
{
	Memory::Write4Byte(m_hInjectedProcess, dwAddress, dwValue);
}

void Bot::WriteFloat(DWORD dwAddress, float fValue)
{
	Memory::WriteFloat(m_hInjectedProcess, dwAddress, fValue);
}

void Bot::WriteString(DWORD dwAddress, std::string strValue)
{
	Memory::WriteString(m_hInjectedProcess, dwAddress, strValue);
}

void Bot::WriteBytes(DWORD dwAddress, std::vector<BYTE> byValue)
{
	Memory::WriteBytes(m_hInjectedProcess, dwAddress, byValue);
}

void Bot::ExecuteRemoteCode(BYTE* codes, size_t psize)
{
	Memory::ExecuteRemoteCode(m_hInjectedProcess, codes, psize);
}
