#include "pch.h"
#include "Bot.h"
#include "ClientHandler.h"
#include "UI.h"
#include "Memory.h"
#include "Remap.h"
#include "Drawing.h"

Bot::Bot()
{
	m_ClientHandler = nullptr;

#ifdef _WINDLL
	m_dwInjectedProcessId = GetCurrentProcessId();
#else
	m_dwInjectedProcessId = 0;
#endif

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

	m_szClientPath.clear();
	m_szClientExe.clear();

	m_bClosed = false;

	m_RouteManager = nullptr;

	m_World = new World();
}

Bot::~Bot()
{
	Close();

	m_ClientHandler = nullptr;

	m_dwInjectedProcessId = 0;

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

	m_szClientPath.clear();
	m_szClientExe.clear();

	m_bClosed = true;

	m_RouteManager = nullptr;

	m_World = nullptr;
}

void Bot::Initialize(std::string szClientPath, std::string szClientExe, PlatformType ePlatformType, int32_t iSelectedAccount)
{
	m_szClientPath = szClientPath;
	m_szClientExe = szClientExe;

	Initialize(ePlatformType, iSelectedAccount);
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

		if (LookupPrivilegeValue(NULL, skCryptDec(SE_DEBUG_NAME), &priv.Privileges[0].Luid))
			AdjustTokenPrivileges(hToken, FALSE, &priv, 0, NULL, NULL);

		CloseHandle(hToken);
	}

	wchar_t* wszAppDataFolder;
	SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, NULL, &wszAppDataFolder);
	m_szAppDataFolder = to_string(wszAppDataFolder);

	m_ePlatformType = ePlatformType;
	m_iSelectedAccount = iSelectedAccount;

	LoadAccountList();

	GetService()->Initialize();
}

void Bot::Process()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(1));

	if (IsServiceClosed())
	{
		Close();
	}
	else
	{
		if (m_ClientHandler)
			m_ClientHandler->Process();
	}
}

void Bot::LoadAccountList()
{
	try
	{
		m_szAccountListFilePath = m_szAppDataFolder + skCryptDec("\\KOF.Accounts.json");

		std::ifstream i(m_szAccountListFilePath.c_str());
		m_AccountList = JSON::parse(i);
	}
	catch (const std::exception& e)
	{
		DBG_UNREFERENCED_PARAMETER(e);

#ifdef _DEBUG
		printf("%s\n", e.what());
#endif
	}
}

void Bot::Close()
{
#ifdef DEBUG
	printf("Bot: Closing\n");
#endif

	Drawing::Done = true;

	m_bClosed = true;

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

	m_pTbl_Skill = new Table<__TABLE_UPC_SKILL>();
	m_pTbl_Skill->Load(m_szClientPath + skCryptDec("\\Data\\skill_magic_main_us.tbl"));

	m_pTbl_Skill_Extension1 = new Table<__TABLE_UPC_SKILL_EXTENSION1>();
	m_pTbl_Skill_Extension1->Load(m_szClientPath + skCryptDec("\\Data\\skill_magic_1.tbl"));

	m_pTbl_Skill_Extension2 = new Table<__TABLE_UPC_SKILL_EXTENSION2>();
	m_pTbl_Skill_Extension2->Load(m_szClientPath + skCryptDec("\\Data\\skill_magic_2.tbl"));

	m_pTbl_Skill_Extension3 = new Table<__TABLE_UPC_SKILL_EXTENSION3>();
	m_pTbl_Skill_Extension3->Load(m_szClientPath + skCryptDec("\\Data\\skill_magic_3.tbl"));

	m_pTbl_Skill_Extension4 = new Table<__TABLE_UPC_SKILL_EXTENSION4>();
	m_pTbl_Skill_Extension4->Load(m_szClientPath + skCryptDec("\\Data\\skill_magic_4.tbl"));

	m_pTbl_Skill_Extension5 = new Table<__TABLE_UPC_SKILL_EXTENSION5>();
	m_pTbl_Skill_Extension5->Load(m_szClientPath + skCryptDec("\\Data\\skill_magic_5.tbl"));

	m_pTbl_Skill_Extension6 = new Table<__TABLE_UPC_SKILL_EXTENSION6>();
	m_pTbl_Skill_Extension6->Load(m_szClientPath + skCryptDec("\\Data\\skill_magic_6.tbl"));

	m_pTbl_Skill_Extension7 = new Table<__TABLE_UPC_SKILL_EXTENSION7>();
	m_pTbl_Skill_Extension7->Load(m_szClientPath + skCryptDec("\\Data\\skill_magic_7.tbl"));

	m_pTbl_Skill_Extension8 = new Table<__TABLE_UPC_SKILL_EXTENSION8>();
	m_pTbl_Skill_Extension8->Load(m_szClientPath + skCryptDec("\\Data\\skill_magic_8.tbl"));

	m_pTbl_Skill_Extension9 = new Table<__TABLE_UPC_SKILL_EXTENSION9>();
	m_pTbl_Skill_Extension9->Load(m_szClientPath + skCryptDec("\\Data\\skill_magic_9.tbl"));

#ifdef DEBUG
	printf("InitializeStaticData: Loaded %d skills\n", m_pTbl_Skill->GetDataSize());
#endif

	TABLE_UPC_SKILL pGodMode;
	memset(&pGodMode, 0, sizeof(pGodMode));

	pGodMode.iID = 500344;
	pGodMode.szName = skCryptDec("God Mode");
	pGodMode.szEngName = skCryptDec("God Mode");
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
	m_pTbl_Item->Load(m_szClientPath + skCryptDec("\\Data\\item_org_us.tbl"));

#ifdef DEBUG
	printf("InitializeStaticData: Loaded %d items\n", m_pTbl_Item->GetDataSize());
#endif

	m_pTbl_Npc = new Table<__TABLE_NPC>();
	m_pTbl_Npc->Load(m_szClientPath + skCryptDec("\\Data\\npc_us.tbl"));

#ifdef DEBUG
	printf("InitializeStaticData: Loaded %d npcs\n", m_pTbl_Npc->GetDataSize());
#endif

	m_pTbl_Mob = new Table<__TABLE_MOB_USKO>();
	m_pTbl_Mob->Load(m_szClientPath + skCryptDec("\\Data\\mob_us.tbl"));

#ifdef DEBUG
	printf("InitializeStaticData: Loaded %d mobs\n", m_pTbl_Mob->GetDataSize());
#endif

#ifdef DEBUG
	printf("InitializeStaticData: Finished\n");
#endif

	m_bTableLoaded = true;
}

void Bot::InitializeRouteData()
{
#ifdef DEBUG
	printf("InitializeRouteData: Started\n");
#endif

	m_RouteManager = new RouteManager();
	m_RouteManager->Load();

#ifdef DEBUG
	printf("InitializeRouteData: Finished\n");
#endif
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
	if(m_iniUserConfiguration->onSaveEvent)
		m_iniUserConfiguration->onSaveEvent();
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

#if !defined(_WINDLL)
	PROCESS_INFORMATION injectedProcessInfo;
	std::ostringstream strCommandLine;
	strCommandLine << GetCurrentProcessId();

	if (!StartProcess(m_szClientPath, m_szClientExe, strCommandLine.str(), injectedProcessInfo))
	{
		Close();
		return;
	}

	if (m_ePlatformType == PlatformType::USKO)
	{
		DWORD dwXignCodeEntryPoint = 0;

		while (dwXignCodeEntryPoint == 0)
			ReadProcessMemory(injectedProcessInfo.hProcess, (LPVOID)GetAddress(skCryptDec("KO_XIGNCODE_ENTRY_POINT")), &dwXignCodeEntryPoint, 4, 0);

		SuspendProcess(injectedProcessInfo.hProcess);

		Remap::PatchSection(injectedProcessInfo.hProcess, (LPVOID*)0x400000, 0xA30000);
		Remap::PatchSection(injectedProcessInfo.hProcess, (LPVOID*)0xE30000, 0x010000);
		Remap::PatchSection(injectedProcessInfo.hProcess, (LPVOID*)0xE40000, 0x130000);

		HMODULE hModuleAdvapi = GetModuleHandle(skCryptDec("advapi32"));

		if (hModuleAdvapi != 0)
		{
			LPVOID* pOpenServicePtr = (LPVOID*)GetProcAddress(hModuleAdvapi, skCryptDec("OpenServiceW"));

			if (pOpenServicePtr != 0)
			{
				BYTE byPatch1[] =
				{
					0xC2, 0x0C, 0x00
				};

				WriteProcessMemory(injectedProcessInfo.hProcess, pOpenServicePtr, byPatch1, sizeof(byPatch1), 0);
			}
		}

#ifdef DISABLE_XIGNCODE
		BYTE byPatch2[] = { 0xE9, 0xE5, 0x02, 0x00, 0x00, 0x90 };
		WriteProcessMemory(injectedProcessInfo.hProcess, (LPVOID*)GetAddress(skCryptDec("KO_XIGNCODE_ENTRY_POINT")), byPatch2, sizeof(byPatch2), 0);
#endif
	}

	m_dwInjectedProcessId = injectedProcessInfo.dwProcessId;

	ResumeProcess(injectedProcessInfo.hProcess);
	CloseHandle(injectedProcessInfo.hProcess);

	m_ClientHandler = new ClientHandler(this);
	m_ClientHandler->Initialize();
#endif
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
		printf("User configuration saving\n");

		std::chrono::milliseconds msCurrentTime = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

		if ((msCurrentTime - msLastConfigurationSave) > std::chrono::milliseconds(1000))
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
	std::string szAddress = m_iniPointer->GetString(skCryptDec("Address"), szAddressName.c_str(), skCryptDec("0x000000"));
	return std::strtoul(szAddress.c_str(), NULL, 16);
}

Ini* Bot::GetConfiguration()
{
	return m_iniUserConfiguration;
}

BYTE Bot::ReadByte(DWORD dwAddress)
{
	return Memory::ReadByte(m_dwInjectedProcessId, dwAddress);
}

DWORD Bot::Read4Byte(DWORD dwAddress)
{
	return Memory::Read4Byte(m_dwInjectedProcessId, dwAddress);
}

float Bot::ReadFloat(DWORD dwAddress)
{
	return Memory::ReadFloat(m_dwInjectedProcessId, dwAddress);
}

std::string Bot::ReadString(DWORD dwAddress, size_t nSize)
{
	return Memory::ReadString(m_dwInjectedProcessId, dwAddress, nSize);
}

std::vector<BYTE> Bot::ReadBytes(DWORD dwAddress, size_t nSize)
{
	return Memory::ReadBytes(m_dwInjectedProcessId, dwAddress, nSize);
}

void Bot::WriteByte(DWORD dwAddress, BYTE byValue)
{
	Memory::WriteByte(m_dwInjectedProcessId, dwAddress, byValue);
}

void Bot::Write4Byte(DWORD dwAddress, DWORD dwValue)
{
	Memory::Write4Byte(m_dwInjectedProcessId, dwAddress, dwValue);
}

void Bot::WriteFloat(DWORD dwAddress, float fValue)
{
	Memory::WriteFloat(m_dwInjectedProcessId, dwAddress, fValue);
}

void Bot::WriteString(DWORD dwAddress, std::string strValue)
{
	Memory::WriteString(m_dwInjectedProcessId, dwAddress, strValue);
}

void Bot::WriteBytes(DWORD dwAddress, std::vector<BYTE> byValue)
{
	Memory::WriteBytes(m_dwInjectedProcessId, dwAddress, byValue);
}

void Bot::ExecuteRemoteCode(BYTE* codes, size_t psize)
{
	Memory::ExecuteRemoteCode(m_dwInjectedProcessId, codes, psize);
}

bool Bot::IsInjectedProcessLost()
{
	if (GetInjectedProcessId() == 0)
		return true;

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetInjectedProcessId());

	if (hProcess == nullptr)
		return true;

	DWORD dwExitCode = 0;

	if (GetExitCodeProcess(hProcess, &dwExitCode) == FALSE)
	{
		CloseHandle(hProcess);
		return true;
	}

	if (dwExitCode != STILL_ACTIVE)
	{
		CloseHandle(hProcess);
		return true;
	}

	CloseHandle(hProcess);

	return false;
}
