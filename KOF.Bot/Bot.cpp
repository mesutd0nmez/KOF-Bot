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

	m_bTableLoaded = false;
	m_pTbl_Skill = nullptr;
	m_pTbl_Skill_Extension2 = nullptr;
	m_pTbl_Skill_Extension4 = nullptr;
	m_pTbl_Item = nullptr;

	for (size_t i = 0; i < 45; i++)
	{
		m_pTbl_Item_Extension[i] = nullptr;
	}

	m_pTbl_Npc = nullptr;
	m_pTbl_Mob_US = nullptr;
	m_pTbl_Mob_CN = nullptr;
	m_pTbl_ItemSell = nullptr;
	m_pTbl_Disguise_Ring = nullptr;

	m_msLastConfigurationSave = std::chrono::milliseconds(0);

	m_szClientPath.clear();
	m_szClientExe.clear();

	m_bClosed = false;

	m_RouteManager = nullptr;

	m_World = new World();

	m_jSupplyList.clear();

	m_jHealthBuffList.clear();
	m_jDefenceBuffList.clear();
	m_jMindBuffList.clear();
	m_jHealList.clear();

	m_msLastInitializeHandle = std::chrono::milliseconds(0);

	m_hPipe = nullptr;
	m_bPipeWorking = false;

	m_jAccountList.clear();
	m_iSelectedAccount = -1;

	m_jSelectedAccount.clear();

	m_jInventoryFlags.clear();

	m_msLastUserConfigurationSaveTime = std::chrono::milliseconds(0);
}

Bot::~Bot()
{
	Close();

	m_ClientHandler = nullptr;

	m_bTableLoaded = false;

	m_pTbl_Skill = nullptr;
	m_pTbl_Skill_Extension2 = nullptr;
	m_pTbl_Skill_Extension4 = nullptr;
	m_pTbl_Item = nullptr;

	for (size_t i = 0; i < 45; i++)
	{
		m_pTbl_Item_Extension[i] = nullptr;
	}

	m_pTbl_Npc = nullptr;
	m_pTbl_Mob_US = nullptr;
	m_pTbl_Mob_CN = nullptr;
	m_pTbl_ItemSell = nullptr;
	m_pTbl_Disguise_Ring = nullptr;

	m_szClientPath.clear();
	m_szClientExe.clear();

	m_bClosed = true;

	m_RouteManager = nullptr;

	m_World = nullptr;

	m_jSupplyList.clear();
	m_jHealthBuffList.clear();
	m_jDefenceBuffList.clear();
	m_jMindBuffList.clear();
	m_jHealList.clear();

	m_msLastInitializeHandle = std::chrono::milliseconds(0);

	m_hPipe = nullptr;
	m_bPipeWorking = false;

	m_jAccountList.clear();
	m_iSelectedAccount = -1;

	m_jSelectedAccount.clear();

	m_msLastUserConfigurationSaveTime = std::chrono::milliseconds(0);
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
		if (m_bPipeWorking == false)
		{
			if (ConnectPipeServer())
			{
#ifdef DEBUG
				printf("Bot: Connected Pipe server\n");
#endif

				Packet pkt = Packet(PIPE_LOAD_POINTER);

				pkt << int32_t(m_mapAddress.size());

				for (auto &e : m_mapAddress)
				{
					pkt << e.first << e.second;
				}

				SendPipeServer(pkt);

				m_bPipeWorking = true;
			}
		}

		if (m_ClientHandler)
		{
			m_ClientHandler->Process();
		}

		if (m_iniUserConfiguration)
		{
			std::chrono::milliseconds msNow = duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch()
			);

			if ((m_msLastUserConfigurationSaveTime.count() + 30000) < msNow.count())
			{
				if (m_iniUserConfiguration->onSaveEvent)
					m_iniUserConfiguration->onSaveEvent();

				m_msLastUserConfigurationSaveTime = msNow;
			}
		}
	}
}

void Bot::LoadAccountList()
{
	try
	{
		m_szAccountListFilePath = skCryptDec("data\\accounts.json");

		std::ifstream i(m_szAccountListFilePath.c_str());
		m_jAccountList = JSON::parse(i);
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("%s\n", e.what());
#else
		DBG_UNREFERENCED_PARAMETER(e);
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

	if (m_pTbl_Skill_Extension2)
		m_pTbl_Skill_Extension2->Release();

	if (m_pTbl_Skill_Extension4)
		m_pTbl_Skill_Extension4->Release();

	if (m_pTbl_Item)
		m_pTbl_Item->Release();

	if (m_pTbl_Npc)
		m_pTbl_Npc->Release();

	if (m_pTbl_Mob_US)
		m_pTbl_Mob_US->Release();

	if (m_pTbl_Mob_CN)
		m_pTbl_Mob_CN->Release();

	if (m_pTbl_ItemSell)
		m_pTbl_ItemSell->Release();

	if(m_pTbl_Disguise_Ring)
		m_pTbl_Disguise_Ring->Release();
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

	std::string szPlatformPrefix = "us";

	switch (m_ePlatformType)
	{
		case PlatformType::CNKO:
		{
			szPlatformPrefix = "nc";
		}
		break;

		case PlatformType::JPKO:
		{
			szPlatformPrefix = "jp";
		}
		break;
	}

	char szPath[255];

	m_pTbl_Skill = new Table<__TABLE_UPC_SKILL>();
	snprintf(szPath, sizeof(szPath), skCryptDec("%s\\Data\\skill_magic_main_%s.tbl"), m_szClientPath.c_str(), szPlatformPrefix.c_str());
	m_pTbl_Skill->Load(szPath);

	m_pTbl_Skill_Extension2 = new Table<__TABLE_UPC_SKILL_EXTENSION2>();
	m_pTbl_Skill_Extension2->Load(m_szClientPath + skCryptDec("\\Data\\skill_magic_2.tbl"));

	m_pTbl_Skill_Extension4 = new Table<__TABLE_UPC_SKILL_EXTENSION4>();
	m_pTbl_Skill_Extension4->Load(m_szClientPath + skCryptDec("\\Data\\skill_magic_4.tbl"));

#ifdef DEBUG
	printf("InitializeStaticData: Loaded %d skills\n", m_pTbl_Skill->GetDataSize());
#endif

	m_pTbl_Item = new Table<__TABLE_ITEM>();
	snprintf(szPath, sizeof(szPath), skCryptDec("%s\\Data\\item_org_%s.tbl"), m_szClientPath.c_str(), szPlatformPrefix.c_str());
	m_pTbl_Item->Load(szPath);

#ifdef DEBUG
	printf("InitializeStaticData: Loaded %d items\n", m_pTbl_Item->GetDataSize());
#endif

	for (size_t i = 0; i < 45; i++)
	{
		m_pTbl_Item_Extension[i] = new Table<__TABLE_ITEM_EXTENSION>();
		snprintf(szPath, sizeof(szPath), skCryptDec("%s\\Data\\item_ext_%d_%s.tbl"), m_szClientPath.c_str(), i, szPlatformPrefix.c_str());
		m_pTbl_Item_Extension[i]->Load(szPath);
#ifdef DEBUG
		printf("InitializeStaticData: Loaded item extension %d, size %d\n", i, m_pTbl_Item_Extension[i]->GetDataSize());
#endif
	}

	m_pTbl_Npc = new Table<__TABLE_NPC>();
	snprintf(szPath, sizeof(szPath), skCryptDec("%s\\Data\\npc_%s.tbl"), m_szClientPath.c_str(), szPlatformPrefix.c_str());
	m_pTbl_Npc->Load(szPath);

#ifdef DEBUG
	printf("InitializeStaticData: Loaded %d npcs\n", m_pTbl_Npc->GetDataSize());
#endif

	if (m_ePlatformType == PlatformType::USKO)
	{
		m_pTbl_Mob_US = new Table<__TABLE_MOB_US>();
		snprintf(szPath, sizeof(szPath), skCryptDec("%s\\Data\\mob_%s.tbl"), m_szClientPath.c_str(), szPlatformPrefix.c_str());
		m_pTbl_Mob_US->Load(szPath);

#ifdef DEBUG
		printf("InitializeStaticData: Loaded %d mobs\n", m_pTbl_Mob_US->GetDataSize());
#endif
	}
	else if (m_ePlatformType == PlatformType::CNKO)
	{
		m_pTbl_Mob_CN = new Table<__TABLE_MOB_CN>();
		snprintf(szPath, sizeof(szPath), skCryptDec("%s\\Data\\mob_%s.tbl"), m_szClientPath.c_str(), szPlatformPrefix.c_str());
		m_pTbl_Mob_CN->Load(szPath);

#ifdef DEBUG
		printf("InitializeStaticData: Loaded %d mobs\n", m_pTbl_Mob_CN->GetDataSize());
#endif
	}

	m_pTbl_ItemSell = new Table<__TABLE_ITEM_SELL>();
	m_pTbl_ItemSell->Load(m_szClientPath + skCryptDec("\\Data\\itemsell_table.tbl"));

#ifdef DEBUG
	printf("InitializeStaticData: Loaded %d selling item\n", m_pTbl_ItemSell->GetDataSize());
#endif

	m_pTbl_Disguise_Ring = new Table<__TABLE_DISGUISE_RING>();
	snprintf(szPath, sizeof(szPath), skCryptDec("%s\\Data\\disguisering_%s.tbl"), m_szClientPath.c_str(), szPlatformPrefix.c_str());
	m_pTbl_Disguise_Ring->Load(szPath);

#ifdef DEBUG
	printf("InitializeStaticData: Loaded %d disquise mob\n", m_pTbl_Disguise_Ring->GetDataSize());
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

void Bot::InitializeSupplyData()
{
#ifdef DEBUG
	printf("InitializeSupplyData: Started\n");
#endif

	try
	{
		std::ifstream ifSupply(
			skCryptDec("data\\supply.json"));

		m_jSupplyList = JSON::parse(ifSupply);

		std::ifstream ifInventoryFlags(
			skCryptDec("data\\inventoryflags.json"));

		m_jInventoryFlags = JSON::parse(ifInventoryFlags);
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("%s\n", e.what());
#else
		DBG_UNREFERENCED_PARAMETER(e);
#endif
	}

#ifdef DEBUG
	printf("InitializeSupplyData: Finished\n");
#endif
}

void Bot::InitializePriestData()
{
#ifdef DEBUG
	printf("InitializePriestData: Started\n");
#endif

	try
	{
		std::ifstream iHealthBuffList(
			skCryptDec("data\\health.json"));

		m_jHealthBuffList = JSON::parse(iHealthBuffList);

		std::ifstream iDefenceBuffList(
			skCryptDec("data\\defence.json"));

		m_jDefenceBuffList = JSON::parse(iDefenceBuffList);

		std::ifstream iMindBuffList(
			skCryptDec("data\\mind.json"));

		m_jMindBuffList = JSON::parse(iMindBuffList);

		std::ifstream iHealList(
			skCryptDec("data\\heal.json"));

		m_jHealList = JSON::parse(iHealList);
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("%s\n", e.what());
#else
		DBG_UNREFERENCED_PARAMETER(e);
#endif
	}

#ifdef DEBUG
	printf("InitializePriestData: Finished\n");
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

	BuildAdress();

#ifdef DEBUG
	printf("Bot: Knight Online process starting\n");
#endif

#ifdef ENABLE_FIREWALL_RULES
	if (m_ePlatformType == PlatformType::USKO)
	{
		std::string strConsoleInfo;
		ConsoleCommand(skCryptDec("netsh advfirewall firewall show rule name=\"KOF Firewall\""), strConsoleInfo);

		if (strConsoleInfo.find(skCryptDec("No rules match")) == std::string::npos)
		{
			ConsoleCommand(skCryptDec("netsh advfirewall firewall set rule name=\"KOF Firewall\" new enable=no"), strConsoleInfo);
		}
	}
#endif

#ifndef DISABLE_XIGNCODE
	if (m_ePlatformType == PlatformType::USKO)
	{
		PROCESS_INFORMATION loaderProcessInfo;
		std::ostringstream strLoaderCommandLine;
		strLoaderCommandLine << m_szClientPath + "\\"  + m_szClientExe;

		if (!StartProcess(
			m_szClientPath + "\\XIGNCODE\\", "xldr_KnightOnline_NA_loader_win32.exe", strLoaderCommandLine.str(), loaderProcessInfo))
		{
#ifdef DEBUG
			printf("Bot: Loader cannot started\n");
#endif
			Close();
			return;
		}

		WaitForSingleObject(loaderProcessInfo.hProcess, INFINITE);
	}
#endif

	std::ostringstream strCommandLine;

	if (m_ePlatformType == PlatformType::USKO || m_ePlatformType == PlatformType::CNKO)
	{
		strCommandLine << GetCurrentProcessId();
	}
	else if (m_ePlatformType == PlatformType::KOKO)
	{
		strCommandLine << "ongate MVGHONG4 NDAYOK1EPZFQT1P6TIQA0YE7ZTD0IWN8LS1V10JLT1V185JX00OMLNQ0 2330316151 15100 0";
	}

	if (!StartProcess(m_szClientPath, m_szClientExe, strCommandLine.str(), m_InjectedProcessInfo))
	{
#ifdef DEBUG
		printf("Bot: Process cannot started\n");
#endif
		Close();
		return;
	}

#ifdef DEBUG
	printf("Bot: Knight Online process started\n");
#endif

	if (m_ePlatformType == PlatformType::USKO 
		|| m_ePlatformType == PlatformType::KOKO)
	{
		DWORD dwXignCodeEntryPoint = 0;

#ifdef DEBUG
		printf("Bot: Waiting entry point\n");
#endif

		while (dwXignCodeEntryPoint == 0)
			ReadProcessMemory(m_InjectedProcessInfo.hProcess, (LPVOID)GetAddress(skCryptDec("KO_XIGNCODE_ENTRY_POINT")), &dwXignCodeEntryPoint, 4, 0);

#ifdef DEBUG
		printf("Bot: Entry point ready, Knight Online process suspending\n");
#endif

		SuspendProcess(m_InjectedProcessInfo.hProcess);

#ifdef DEBUG
		printf("Bot: Knight Online process suspended, bypass started\n");
#endif

		if (GetAddress(skCryptDec("KO_PATCH_ADDRESS1")) > 0)
		{
			Remap::PatchSection(
				m_InjectedProcessInfo.hProcess,
				(LPVOID*)GetAddress(skCryptDec("KO_PATCH_ADDRESS1")), 
				GetAddress(skCryptDec("KO_PATCH_ADDRESS1_SIZE")), PAGE_EXECUTE_READWRITE);
		}

		if (GetAddress(skCryptDec("KO_PATCH_ADDRESS2")) > 0)
		{
			Remap::PatchSection(
				m_InjectedProcessInfo.hProcess,
				(LPVOID*)GetAddress(skCryptDec("KO_PATCH_ADDRESS2")),
				GetAddress(skCryptDec("KO_PATCH_ADDRESS2_SIZE")), PAGE_EXECUTE_READWRITE);
		}
		
		if (GetAddress(skCryptDec("KO_PATCH_ADDRESS3")) > 0)
		{
			Remap::PatchSection(
				m_InjectedProcessInfo.hProcess,
				(LPVOID*)GetAddress(skCryptDec("KO_PATCH_ADDRESS3")),
				GetAddress(skCryptDec("KO_PATCH_ADDRESS3_SIZE")), PAGE_EXECUTE_READWRITE);
		}

#ifdef DEBUG
		printf("Bot: Knight Online Patched\n");
#endif

#ifdef DISABLE_XIGNCODE
		if (m_ePlatformType == PlatformType::USKO)
		{
			BYTE byPatch2[] = { 0xE9, 0xE5, 0x02, 0x00, 0x00, 0x90 };
			WriteProcessMemory(m_InjectedProcessInfo.hProcess, (LPVOID*)GetAddress(skCryptDec("KO_XIGNCODE_ENTRY_POINT")), byPatch2, sizeof(byPatch2), 0);
		}
		else if (m_ePlatformType == PlatformType::KOKO)
		{
			BYTE byPatch2[] = { 0xE9, 0x50, 0x07, 0x00, 0x00, 0x90 };
			WriteProcessMemory(m_InjectedProcessInfo.hProcess, (LPVOID*)GetAddress(skCryptDec("KO_XIGNCODE_ENTRY_POINT")), byPatch2, sizeof(byPatch2), 0);
		}
#endif

#ifdef DEBUG
		printf("Bot: Bypass finished, Knight Online process resuming\n");
#endif

		ResumeProcess(m_InjectedProcessInfo.hProcess);
	}

	Patch(m_InjectedProcessInfo.hProcess);

	std::this_thread::sleep_for(std::chrono::milliseconds(3000));

	Injection(m_InjectedProcessInfo.dwProcessId, skCryptDec("KOF.dll"));

	//Injection(m_InjectedProcessInfo.dwProcessId, skCryptDec("C:\\Users\\Administrator\\Documents\\Github\\Pipeline\\Debug\\Pipeline.dll"));

#ifndef NO_INITIALIZE_CLIENT_HANDLER
	m_ClientHandler = new ClientHandler(this);
	m_ClientHandler->Initialize();
#endif
}

void Bot::Patch(HANDLE hProcess)
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	DWORD iPatchEntryPoint1 = 0;
	while (iPatchEntryPoint1 == 0)
		ReadProcessMemory(hProcess, (LPVOID)GetAddress(skCryptDec("KO_OFF_WIZ_HACKTOOL_NOP1")), &iPatchEntryPoint1, 4, 0);

	BYTE byPatch1[] = 
	{ 
		0x90, 
		0x90, 
		0x90, 
		0x90, 
		0x90 
	};

	WriteProcessMemory(hProcess, (LPVOID*)GetAddress(skCryptDec("KO_OFF_WIZ_HACKTOOL_NOP1")), byPatch1, sizeof(byPatch1), 0);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	DWORD iPatchEntryPoint2 = 0;
	while (iPatchEntryPoint2 == 0)
		ReadProcessMemory(hProcess, (LPVOID)GetAddress(skCryptDec("KO_LEGAL_ATTACK_FIX1")), &iPatchEntryPoint2, 4, 0);

	BYTE byPatch2[] =
	{ 
		0xE9, 0xC5, 0x00, 0x00, 0x00,
		0x90 
	};

	WriteProcessMemory(hProcess, (LPVOID*)GetAddress(skCryptDec("KO_LEGAL_ATTACK_FIX1")), byPatch2, sizeof(byPatch2), 0);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

	new std::thread([this]() { UI::Render(this); });

	m_iniUserConfiguration->onSaveEvent = [=]()
	{
#ifdef DEBUG
		printf("User configuration saving\n");
#endif

		std::chrono::milliseconds msCurrentTime = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

		if ((msCurrentTime - m_msLastConfigurationSave) > std::chrono::milliseconds(1000))
		{
			SendSaveUserConfiguration(1, GetClientHandler()->GetName());
			m_msLastConfigurationSave = msCurrentTime;
		}
	};
}

Ini* Bot::GetAppConfiguration()
{
	return m_iniAppConfiguration;
}

Ini* Bot::GetUserConfiguration()
{
	return m_iniUserConfiguration;
}

bool Bot::GetSkillTable(std::map<uint32_t, __TABLE_UPC_SKILL>** mapDataOut)
{
	if (m_pTbl_Skill == nullptr)
		return false;

	return m_pTbl_Skill->GetData(mapDataOut);
}

bool Bot::GetSkillExtension2Table(std::map<uint32_t, __TABLE_UPC_SKILL_EXTENSION2>** mapDataOut)
{
	if (m_pTbl_Skill_Extension2 == nullptr)
		return false;

	return m_pTbl_Skill_Extension2->GetData(mapDataOut);
}

bool Bot::GetSkillExtension4Table(std::map<uint32_t, __TABLE_UPC_SKILL_EXTENSION4>** mapDataOut)
{
	if (m_pTbl_Skill_Extension4 == nullptr)
		return false;

	return m_pTbl_Skill_Extension4->GetData(mapDataOut);
}

bool Bot::GetItemTable(std::map<uint32_t, __TABLE_ITEM>** mapDataOut)
{
	if (m_pTbl_Item == nullptr)
		return false;

	return m_pTbl_Item->GetData(mapDataOut);
}

bool Bot::GetItemData(uint32_t iItemID, __TABLE_ITEM*& pOutItemData)
{
	if (m_pTbl_Item == nullptr)
		return false;

	std::map<uint32_t, __TABLE_ITEM>* pItemTable;
	if (!GetItemTable(&pItemTable))
		return false;

	uint32_t iItemBaseID = iItemID / 1000 * 1000;

	auto pItemData = pItemTable->find(iItemBaseID);

	if (pItemData == pItemTable->end())
		return false;

	pOutItemData = &(pItemData->second);

	return true;
}

bool Bot::GetItemExtensionTable(uint8_t iExtensionID, std::map<uint32_t, __TABLE_ITEM_EXTENSION>** mapDataOut)
{
	if (m_pTbl_Item_Extension[iExtensionID] == nullptr)
		return false;

	return m_pTbl_Item_Extension[iExtensionID]->GetData(mapDataOut);
}

bool Bot::GetItemExtensionData(uint32_t iItemID, uint8_t iExtensionID, __TABLE_ITEM_EXTENSION*& pOutItemExtensionData)
{
	if (m_pTbl_Item_Extension[iExtensionID] == nullptr)
		return false;

	std::map<uint32_t, __TABLE_ITEM_EXTENSION>* pItemExtensionTable;
	if (!GetItemExtensionTable(iExtensionID, &pItemExtensionTable))
		return false;

	auto pItemExtensionData = pItemExtensionTable->find(iItemID % 1000);

	if (pItemExtensionData == pItemExtensionTable->end())
		return false;

	pOutItemExtensionData = &(pItemExtensionData->second);

	return true;
}

bool Bot::GetNpcTable(std::map<uint32_t, __TABLE_NPC>** mapDataOut)
{
	if (m_pTbl_Npc == nullptr)
		return false;

	return m_pTbl_Npc->GetData(mapDataOut);
}

bool Bot::GetMobTable(std::map<uint32_t, __TABLE_MOB_US>** mapDataOut)
{
	if (m_pTbl_Mob_US == nullptr)
		return false;

	return m_pTbl_Mob_US->GetData(mapDataOut);
}

bool Bot::GetMobTable(std::map<uint32_t, __TABLE_MOB_CN>** mapDataOut)
{
	if (m_pTbl_Mob_CN == nullptr)
		return false;

	return m_pTbl_Mob_CN->GetData(mapDataOut);
}

bool Bot::GetItemSellTable(std::map<uint32_t, __TABLE_ITEM_SELL>** mapDataOut)
{
	if (m_pTbl_ItemSell == nullptr)
		return false;

	return m_pTbl_ItemSell->GetData(mapDataOut);
}

bool Bot::GetDisguiseRingTable(std::map<uint32_t, __TABLE_DISGUISE_RING>** mapDataOut)
{
	if (m_pTbl_Disguise_Ring == nullptr)
		return false;

	return m_pTbl_Disguise_Ring->GetData(mapDataOut);
}

BYTE Bot::ReadByte(DWORD dwAddress)
{
	/*HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetInjectedProcessId());

	if (hProcess == nullptr)
		return 0;*/

	HANDLE hProcess = GetInjectedProcessHandle();

	BYTE nRet = Memory::ReadByte(hProcess, dwAddress);

	//CloseHandle(hProcess);
	return nRet;
}

DWORD Bot::Read4Byte(DWORD dwAddress)
{
	//HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetInjectedProcessId());

	//if (hProcess == nullptr)
	//	return 0;

	HANDLE hProcess = GetInjectedProcessHandle();

	DWORD nRet = Memory::Read4Byte(hProcess, dwAddress);

	//CloseHandle(hProcess);
	return nRet;
}

float Bot::ReadFloat(DWORD dwAddress)
{
	//HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetInjectedProcessId());

	//if (hProcess == nullptr)
	//	return 0.0f;

	HANDLE hProcess = GetInjectedProcessHandle();

	float nRet = Memory::ReadFloat(hProcess, dwAddress);

	//CloseHandle(hProcess);
	return nRet;
}

std::string Bot::ReadString(DWORD dwAddress, size_t nSize)
{
	//HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetInjectedProcessId());

	//if (hProcess == nullptr)
	//	return "";

	HANDLE hProcess = GetInjectedProcessHandle();

	std::string nRet = Memory::ReadString(hProcess, dwAddress, nSize);

	//CloseHandle(hProcess);
	return nRet;
}

std::vector<BYTE> Bot::ReadBytes(DWORD dwAddress, size_t nSize)
{
	//HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetInjectedProcessId());

	std::vector<BYTE> nRet;
	/*if (hProcess == nullptr)
		return nRet;*/

	HANDLE hProcess = GetInjectedProcessHandle();

	nRet = Memory::ReadBytes(hProcess, dwAddress, nSize);

	//CloseHandle(hProcess);
	return nRet;
}

void Bot::WriteByte(DWORD dwAddress, BYTE byValue)
{
	//HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetInjectedProcessId());

	//if (hProcess == nullptr)
	//	return;

	HANDLE hProcess = GetInjectedProcessHandle();

	Memory::WriteByte(hProcess, dwAddress, byValue);
	//CloseHandle(hProcess);
}

void Bot::Write4Byte(DWORD dwAddress, int iValue)
{
	//HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetInjectedProcessId());

	//if (hProcess == nullptr)
	//	return;

	HANDLE hProcess = GetInjectedProcessHandle();

	Memory::Write4Byte(hProcess, dwAddress, iValue);
	//CloseHandle(hProcess);
}

void Bot::WriteFloat(DWORD dwAddress, float fValue)
{
	//HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetInjectedProcessId());

	//if (hProcess == nullptr)
	//	return;

	HANDLE hProcess = GetInjectedProcessHandle();

	Memory::WriteFloat(hProcess, dwAddress, fValue);
	//CloseHandle(hProcess);
}

void Bot::WriteString(DWORD dwAddress, std::string strValue)
{
	//HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetInjectedProcessId());

	//if (hProcess == nullptr)
	//	return;

	HANDLE hProcess = GetInjectedProcessHandle();

	Memory::WriteString(hProcess, dwAddress, strValue);
	//CloseHandle(hProcess);
}

void Bot::WriteBytes(DWORD dwAddress, std::vector<BYTE> byValue)
{
	//HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetInjectedProcessId());

	//if (hProcess == nullptr)
	//	return;

	HANDLE hProcess = GetInjectedProcessHandle();

	Memory::WriteBytes(hProcess, dwAddress, byValue);
	//CloseHandle(hProcess);
}

bool Bot::ExecuteRemoteCode(HANDLE hProcess, BYTE* codes, size_t psize)
{
	return Memory::ExecuteRemoteCode(hProcess, codes, psize);
}

bool Bot::ExecuteRemoteCode(HANDLE hProcess, LPVOID pAddress)
{
	return Memory::ExecuteRemoteCode(hProcess, pAddress);
}

bool Bot::GetShopItemTable(int32_t iSellingGroup, std::vector<SShopItem>& vecShopWindow)
{
	std::map<uint32_t, __TABLE_ITEM_SELL>* pSellTable;
	if (!GetItemSellTable(&pSellTable))
		return false;

	if (pSellTable->size() == 0)
		return false;

	std::vector<SShopItem> vecTmpShopWindow;

	uint8_t iPage = 0;
	for (auto& e : *pSellTable)
	{
		if (e.second.iSellingGroup == iSellingGroup)
		{
			vecTmpShopWindow.push_back(SShopItem(iPage, 0, e.second.iItem0));
			vecTmpShopWindow.push_back(SShopItem(iPage, 1, e.second.iItem1));
			vecTmpShopWindow.push_back(SShopItem(iPage, 2, e.second.iItem2));
			vecTmpShopWindow.push_back(SShopItem(iPage, 3, e.second.iItem3));
			vecTmpShopWindow.push_back(SShopItem(iPage, 4, e.second.iItem4));
			vecTmpShopWindow.push_back(SShopItem(iPage, 5, e.second.iItem5));
			vecTmpShopWindow.push_back(SShopItem(iPage, 6, e.second.iItem6));
			vecTmpShopWindow.push_back(SShopItem(iPage, 7, e.second.iItem7));
			vecTmpShopWindow.push_back(SShopItem(iPage, 8, e.second.iItem8));
			vecTmpShopWindow.push_back(SShopItem(iPage, 9, e.second.iItem9));
			vecTmpShopWindow.push_back(SShopItem(iPage, 10, e.second.iItem10));
			vecTmpShopWindow.push_back(SShopItem(iPage, 11, e.second.iItem11));
			vecTmpShopWindow.push_back(SShopItem(iPage, 12, e.second.iItem12));
			vecTmpShopWindow.push_back(SShopItem(iPage, 13, e.second.iItem13));
			vecTmpShopWindow.push_back(SShopItem(iPage, 14, e.second.iItem14));
			vecTmpShopWindow.push_back(SShopItem(iPage, 15, e.second.iItem15));
			vecTmpShopWindow.push_back(SShopItem(iPage, 16, e.second.iItem16));
			vecTmpShopWindow.push_back(SShopItem(iPage, 17, e.second.iItem17));
			vecTmpShopWindow.push_back(SShopItem(iPage, 18, e.second.iItem18));
			vecTmpShopWindow.push_back(SShopItem(iPage, 19, e.second.iItem19));
			vecTmpShopWindow.push_back(SShopItem(iPage, 20, e.second.iItem20));
			vecTmpShopWindow.push_back(SShopItem(iPage, 21, e.second.iItem21));
			vecTmpShopWindow.push_back(SShopItem(iPage, 22, e.second.iItem22));
			vecTmpShopWindow.push_back(SShopItem(iPage, 23, e.second.iItem23));

			iPage += 1;
		}
	}

	if (vecTmpShopWindow.size() == 0)
		return false;

	vecShopWindow = vecTmpShopWindow;

	return true;
}

DWORD Bot::GetAddress(std::string szAddressName)
{
	auto it = m_mapAddress.find(szAddressName);

	if (it != m_mapAddress.end())
		return it->second;

	return 0;
}

void Bot::BuildAdress()
{
#ifdef DEBUG
	printf("Bot: Address build started\n");
#endif
	auto pAddressMap = m_iniPointer->GetConfigMap();

	if (pAddressMap == nullptr)
		return;

	auto it = pAddressMap->find(skCryptDec("Address"));

	if (it != pAddressMap->end())
	{
		for (auto &e : it->second)
			m_mapAddress.insert(std::make_pair(e.first, std::strtoul(e.second.c_str(), NULL, 16)));
	}

	delete m_iniPointer;
	m_iniPointer = nullptr;

#ifdef DEBUG
	printf("Bot: Address build completed\n");
#endif
}

bool Bot::ConnectPipeServer()
{
	m_hPipe = CreateFileA(skCryptDec("\\\\.\\pipe\\pipeline"),
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (m_hPipe != INVALID_HANDLE_VALUE)
	{
		return true;
	}

	return false;
}

void Bot::SendPipeServer(Packet pkt)
{
	if (m_hPipe != INVALID_HANDLE_VALUE)
	{
		DWORD iNumberOfBytesWritten;

		WriteFile(m_hPipe,
			pkt.contents(),
			pkt.size(),
			&iNumberOfBytesWritten,
			NULL);
	}
}

float Bot::TimeGet()
{
	static bool bInit = false;
	static bool bUseHWTimer = FALSE;
	static LARGE_INTEGER nTime, nFrequency;

	if (bInit == false)
	{
		if (TRUE == ::QueryPerformanceCounter(&nTime))
		{
			::QueryPerformanceFrequency(&nFrequency);
			bUseHWTimer = TRUE;
		}
		else
		{
			bUseHWTimer = FALSE;
		}

		bInit = true;
	}

	if (bUseHWTimer)
	{
		::QueryPerformanceCounter(&nTime);
		return (float)((double)(nTime.QuadPart) / (double)nFrequency.QuadPart);
	}

	return (float)timeGetTime();
}

uint8_t Bot::GetInventoryItemFlag(uint32_t iItemID)
{
	for (size_t i = 0; i < m_jInventoryFlags.size(); i++)
	{
		if (m_jInventoryFlags[i]["id"].get<uint32_t>() == iItemID)
		{
			return m_jInventoryFlags[i]["flag"].get<uint8_t>();
		}
	}

	return INVENTORY_ITEM_FLAG_NONE;
};

void Bot::UpdateInventoryItemFlag(JSON pInventoryFlags)
{
	try
	{
		m_jInventoryFlags = pInventoryFlags;

		std::ofstream o(
			std::filesystem::current_path().string()
			+ skCryptDec("\\data\\")
			+ skCryptDec("inventoryflags.json"));

		o << std::setw(4) << m_jInventoryFlags << std::endl;
	}
	catch (const std::exception& e)
	{
		DBG_UNREFERENCED_PARAMETER(e);

#ifdef _DEBUG
		printf("%s\n", e.what());
#endif
	}
}

bool Bot::IsInjectedProcessLost()
{
	if (GetInjectedProcessId() == 0)
	{
		return true;
	}

	DWORD iExitCode = 0;
	if (GetExitCodeProcess(GetInjectedProcessHandle(), &iExitCode) == FALSE)
	{
		return true;
	}

	if (iExitCode != STILL_ACTIVE)
	{
		return true;
	}

	return false;
}

HANDLE Bot::GetInjectedProcessHandle()
{
	DWORD dwFlags;

	if (GetHandleInformation(m_InjectedProcessInfo.hProcess, &dwFlags))
	{
		return m_InjectedProcessInfo.hProcess;
	}

	m_InjectedProcessInfo.hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetInjectedProcessId());

	return m_InjectedProcessInfo.hProcess;
}