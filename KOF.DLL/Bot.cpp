#include "pch.h"
#include "Bot.h"
#include "ClientHandler.h"
#include "UI.h"
#include "Memory.h"
#include "Remap.h"

Bot::Bot()
{
	m_ClientHandler = nullptr;

	m_dwInjectedProcessID = 0;

	m_bTableLoaded = false;
	m_pTbl_Skill = nullptr;
	m_pTbl_Skill_Extension1 = nullptr;
	m_pTbl_Skill_Extension2 = nullptr;
	m_pTbl_Skill_Extension3 = nullptr;
	m_pTbl_Skill_Extension4 = nullptr;
	m_pTbl_Skill_Extension5 = nullptr;
	m_pTbl_Skill_Extension6 = nullptr;
	m_pTbl_Skill_Extension7 = nullptr;
	m_pTbl_Skill_Extension8 = nullptr;
	m_pTbl_Skill_Extension9 = nullptr;
	m_pTbl_Item = nullptr;
	m_pTbl_Npc = nullptr;
	m_pTbl_Mob = nullptr;

	msLastConfigurationSave = std::chrono::milliseconds(0);
}

Bot::~Bot()
{
	Close();

	m_ClientHandler = nullptr;

	m_dwInjectedProcessID = 0;

	m_bTableLoaded = false;

	m_pTbl_Skill = nullptr;
	m_pTbl_Skill_Extension1 = nullptr;
	m_pTbl_Skill_Extension2 = nullptr;
	m_pTbl_Skill_Extension3 = nullptr;
	m_pTbl_Skill_Extension4 = nullptr;
	m_pTbl_Skill_Extension5 = nullptr;
	m_pTbl_Skill_Extension6 = nullptr;
	m_pTbl_Skill_Extension7 = nullptr;
	m_pTbl_Skill_Extension8 = nullptr;
	m_pTbl_Skill_Extension9 = nullptr;
	m_pTbl_Item = nullptr;
	m_pTbl_Npc = nullptr;
	m_pTbl_Mob = nullptr;

	msLastConfigurationSave = std::chrono::milliseconds(0);
}

void Bot::Initialize(PlatformType ePlatformType, int32_t iSelectedAccount)
{
#ifdef DEBUG
	printf("Bot: Initialize\n");
#endif

#ifdef DEBUG
	printf("Bot: Current process adjusting privileges\n");
#endif

	HANDLE hToken = NULL;
	TOKEN_PRIVILEGES priv = { 0 };
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		priv.PrivilegeCount = 1;
		priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		if (LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &priv.Privileges[0].Luid))
			AdjustTokenPrivileges(hToken, FALSE, &priv, 0, NULL, NULL);

		CloseHandle(hToken);
	}

#ifdef _WINDLL
	m_dwInjectedProcessID = GetCurrentProcessId();
#endif

	m_ePlatformType = ePlatformType;
	m_iSelectedAccount = iSelectedAccount;

	new std::thread([this]() { GetService()->Initialize(); });
}

void Bot::Process()
{
	if (m_ClientHandler)
		m_ClientHandler->Process();
}

void Bot::Close()
{
#ifdef DEBUG
	printf("Bot: Closing\n");
#endif

	if (m_ClientHandler)
		m_ClientHandler->StopHandler();

	Release();
}

void Bot::Release()
{
#ifdef DEBUG
	printf("Bot: Release\n");
#endif

	if (m_pTbl_Skill)
		m_pTbl_Skill->Release();

	if (m_pTbl_Skill_Extension1)
		m_pTbl_Skill_Extension1->Release();

	if (m_pTbl_Skill_Extension2)
		m_pTbl_Skill_Extension2->Release();

	if (m_pTbl_Skill_Extension3)
		m_pTbl_Skill_Extension3->Release();

	if (m_pTbl_Skill_Extension4)
		m_pTbl_Skill_Extension4->Release();

	if (m_pTbl_Skill_Extension5)
		m_pTbl_Skill_Extension5->Release();

	if (m_pTbl_Skill_Extension6)
		m_pTbl_Skill_Extension6->Release();

	if (m_pTbl_Skill_Extension7)
		m_pTbl_Skill_Extension7->Release();

	if (m_pTbl_Skill_Extension8)
		m_pTbl_Skill_Extension8->Release();

	if (m_pTbl_Skill_Extension9)
		m_pTbl_Skill_Extension9->Release();

	if (m_pTbl_Item)
		m_pTbl_Item->Release();

	if (m_pTbl_Npc)
		m_pTbl_Npc->Release();

	if (m_pTbl_Mob)
		m_pTbl_Mob->Release();
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

	std::string szDevelopmentPath = DEVELOPMENT_PATH;

	m_pTbl_Skill = new Table<__TABLE_UPC_SKILL>();
	m_pTbl_Skill->Load(szDevelopmentPath + ".\\Data\\skill_magic_main_us.tbl");

	m_pTbl_Skill_Extension1 = new Table<__TABLE_UPC_SKILL_EXTENSION1>();
	m_pTbl_Skill_Extension1->Load(szDevelopmentPath + ".\\Data\\skill_magic_1.tbl");

	m_pTbl_Skill_Extension2 = new Table<__TABLE_UPC_SKILL_EXTENSION2>();
	m_pTbl_Skill_Extension2->Load(szDevelopmentPath + ".\\Data\\skill_magic_2.tbl");

	m_pTbl_Skill_Extension3 = new Table<__TABLE_UPC_SKILL_EXTENSION3>();
	m_pTbl_Skill_Extension3->Load(szDevelopmentPath + ".\\Data\\skill_magic_3.tbl");

	m_pTbl_Skill_Extension4 = new Table<__TABLE_UPC_SKILL_EXTENSION4>();
	m_pTbl_Skill_Extension4->Load(szDevelopmentPath + ".\\Data\\skill_magic_4.tbl");

	m_pTbl_Skill_Extension5 = new Table<__TABLE_UPC_SKILL_EXTENSION5>();
	m_pTbl_Skill_Extension5->Load(szDevelopmentPath + ".\\Data\\skill_magic_5.tbl");

	m_pTbl_Skill_Extension6 = new Table<__TABLE_UPC_SKILL_EXTENSION6>();
	m_pTbl_Skill_Extension6->Load(szDevelopmentPath + ".\\Data\\skill_magic_6.tbl");

	m_pTbl_Skill_Extension7 = new Table<__TABLE_UPC_SKILL_EXTENSION7>();
	m_pTbl_Skill_Extension7->Load(szDevelopmentPath + ".\\Data\\skill_magic_7.tbl");

	m_pTbl_Skill_Extension8 = new Table<__TABLE_UPC_SKILL_EXTENSION8>();
	m_pTbl_Skill_Extension8->Load(szDevelopmentPath + ".\\Data\\skill_magic_8.tbl");

	m_pTbl_Skill_Extension9 = new Table<__TABLE_UPC_SKILL_EXTENSION9>();
	m_pTbl_Skill_Extension9->Load(szDevelopmentPath + ".\\Data\\skill_magic_9.tbl");

#ifdef DEBUG
	printf("InitializeStaticData: Loaded %d skills\n", m_pTbl_Skill->GetDataSize());
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

	m_pTbl_Skill->Insert(pGodMode.iID, pGodMode);

	TABLE_UPC_SKILL_EXTENSION4 pGodModeExtension4;
	memset(&pGodModeExtension4, 0, sizeof(pGodModeExtension4));

	pGodModeExtension4.iID = pGodMode.iID;
	pGodModeExtension4.iBuffType = 1;
	pGodModeExtension4.iAreaRadius = 0;
	pGodModeExtension4.iBuffDuration = 3600;

	m_pTbl_Skill_Extension4->Insert(pGodModeExtension4.iID, pGodModeExtension4);

#ifdef DEBUG
	printf("InitializeStaticData: Loaded custom skills\n");
#endif

	m_pTbl_Item = new Table<__TABLE_ITEM>();
	m_pTbl_Item->Load(szDevelopmentPath + ".\\Data\\item_org_us.tbl");

#ifdef DEBUG
	printf("InitializeStaticData: Loaded %d items\n", m_pTbl_Item->GetDataSize());
#endif

	m_pTbl_Npc = new Table<__TABLE_NPC>();
	m_pTbl_Npc->Load(szDevelopmentPath + ".\\Data\\npc_us.tbl");

#ifdef DEBUG
	printf("InitializeStaticData: Loaded %d npcs\n", m_pTbl_Npc->GetDataSize());
#endif

	m_pTbl_Mob = new Table<__TABLE_MOB_USKO>();
	m_pTbl_Mob->Load(szDevelopmentPath + ".\\Data\\mob_us.tbl");

#ifdef DEBUG
	printf("InitializeStaticData: Loaded %d mobs\n", m_pTbl_Mob->GetDataSize());
#endif

#ifdef DEBUG
	printf("InitializeStaticData: Finished\n");
#endif

	m_bTableLoaded = true;
}

void Bot::OnReady()
{
#ifdef DEBUG
	printf("Bot: OnReady\n");
#endif

	if (m_szToken.size() > 0)
	{
		SendLogin(m_szToken);
	}
}

void Bot::OnPong()
{
#ifdef DEBUG
	printf("Bot: OnPong\n");
#endif
}

void Bot::OnAuthenticated()
{
#ifdef DEBUG
	printf("Bot: OnAuthenticated\n");
#endif
}

void Bot::OnLoaded()
{
#ifdef DEBUG
	printf("Bot: OnLoaded\n");
#endif

#ifndef _WINDLL
	std::ostringstream strCommandLine;
	strCommandLine << GetCurrentProcessId();

	StartProcess(DEVELOPMENT_PATH, "KnightOnLine.exe", strCommandLine.str(), m_injectedProcessInfo);

	if (m_injectedProcessInfo.hProcess != 0)
	{
		

		DWORD dwXignCodeEntryPoint = 0;

		while (dwXignCodeEntryPoint == 0)
			ReadProcessMemory(m_injectedProcessInfo.hProcess, (LPVOID)0xCEB282, &dwXignCodeEntryPoint, 4, 0);

		SuspendProcess(m_injectedProcessInfo.hProcess);

		Remap::PatchSection(m_injectedProcessInfo.hProcess, (LPVOID*)0x00400000, 0x00A30000);
		Remap::PatchSection(m_injectedProcessInfo.hProcess, (LPVOID*)0x00E30000, 0x00140000);

		HMODULE hModuleAdvapi = GetModuleHandle("advapi32");

		if (hModuleAdvapi != 0)
		{
			LPVOID* pOpenServicePtr = (LPVOID*)GetProcAddress(hModuleAdvapi, "OpenServiceW");

			if (pOpenServicePtr != 0)
			{
				BYTE byPatch[] =
				{
					0xC2, 0x0C, 0x00
				};

				WriteProcessMemory(m_injectedProcessInfo.hProcess, pOpenServicePtr, byPatch, sizeof(byPatch), 0);
			}
		}

#ifdef DISABLE_XIGNCODE
		BYTE byPatch22[] = { 0xE9, 0xE5, 0x02, 0x00, 0x00, 0x90 };
		WriteProcessMemory(processInfo.hProcess, (LPVOID*)0xCEB282, byPatch22, sizeof(byPatch22), 0);
#endif

		ResumeProcess(m_injectedProcessInfo.hProcess);

		m_dwInjectedProcessID = m_injectedProcessInfo.dwProcessId;
	}
#endif

	m_ClientHandler = new ClientHandler(this);
	m_ClientHandler->Initialize();
}

void Bot::OnConfigurationLoaded()
{
#ifdef DEBUG
	printf("Bot: OnConfigurationLoaded\n");
#endif

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

	m_iniUserConfiguration->onSaveEvent = [=]()
	{
		std::chrono::milliseconds msCurrentTime = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

		if ((msCurrentTime - msLastConfigurationSave) > std::chrono::milliseconds(3000))
		{
			SendSaveUserConfiguration(1, GetClientHandler()->GetName());
			msLastConfigurationSave = msCurrentTime;
		}
	};


	m_ClientHandler->SetConfigurationLoaded(true);
	m_ClientHandler->StartHandler();

	new std::thread([this]() { UI::Render(this); });
}

DWORD Bot::GetAddress(std::string szAddressName)
{
	std::string szAddress = m_iniPointer->GetString("Address", szAddressName.c_str(), "0x000000");
	return std::strtoul(szAddress.c_str(), NULL, 16);
}

Ini* Bot::GetConfiguration()
{
	return m_iniUserConfiguration;
}

BYTE Bot::ReadByte(DWORD dwAddress)
{
	return Memory::ReadByte(m_dwInjectedProcessID, dwAddress);
}

DWORD Bot::Read4Byte(DWORD dwAddress)
{
	return Memory::Read4Byte(m_dwInjectedProcessID, dwAddress);
}

float Bot::ReadFloat(DWORD dwAddress)
{
	return Memory::ReadFloat(m_dwInjectedProcessID, dwAddress);
}

std::string Bot::ReadString(DWORD dwAddress, size_t nSize)
{
	return Memory::ReadString(m_dwInjectedProcessID, dwAddress, nSize);
}

std::vector<BYTE> Bot::ReadBytes(DWORD dwAddress, size_t nSize)
{
	return Memory::ReadBytes(m_dwInjectedProcessID, dwAddress, nSize);
}

void Bot::WriteByte(DWORD dwAddress, BYTE byValue)
{
	Memory::WriteByte(m_dwInjectedProcessID, dwAddress, byValue);
}

void Bot::Write4Byte(DWORD dwAddress, DWORD dwValue)
{
	Memory::Write4Byte(m_dwInjectedProcessID, dwAddress, dwValue);
}

void Bot::WriteFloat(DWORD dwAddress, float fValue)
{
	Memory::WriteFloat(m_dwInjectedProcessID, dwAddress, fValue);
}

void Bot::WriteString(DWORD dwAddress, std::string strValue)
{
	Memory::WriteString(m_dwInjectedProcessID, dwAddress, strValue);
}

void Bot::WriteBytes(DWORD dwAddress, std::vector<BYTE> byValue)
{
	Memory::WriteBytes(m_dwInjectedProcessID, dwAddress, byValue);
}

void Bot::ExecuteRemoteCode(BYTE* codes, size_t psize)
{
	Memory::ExecuteRemoteCode(m_dwInjectedProcessID, codes, psize);
}

bool Bot::IsInjectedProcessLost()
{
	if (GetInjectedProcessId() == 0)
		return true;

#ifdef _WINDLL
	HANDLE hProcess = GetCurrentProcess();
#else
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetInjectedProcessId());
#endif

	if (hProcess == 0)
		return true;

	DWORD dwExitCode = 0;

	if (GetExitCodeProcess(hProcess, &dwExitCode) == FALSE)
		return true;

	if (dwExitCode != STILL_ACTIVE)
		return true;

	return false;
}
