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

	m_dwInjectedProcessId = 0;

	m_bTableLoaded = false;
	m_pTbl_Skill = nullptr;
	m_pTbl_Skill_Extension2 = nullptr;
	m_pTbl_Skill_Extension4 = nullptr;
	m_pTbl_Item = nullptr;
	m_pTbl_Npc = nullptr;
	m_pTbl_Mob = nullptr;
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
}

Bot::~Bot()
{
	Close();

	m_ClientHandler = nullptr;

	m_dwInjectedProcessId = 0;

	m_bTableLoaded = false;

	m_pTbl_Skill = nullptr;
	m_pTbl_Skill_Extension2 = nullptr;
	m_pTbl_Skill_Extension4 = nullptr;
	m_pTbl_Item = nullptr;
	m_pTbl_Npc = nullptr;
	m_pTbl_Mob = nullptr;
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
		if (m_ClientHandler)
			m_ClientHandler->Process();

		if (m_bPipeWorking == false)
		{
			if (ConnectPipeServer())
			{
				m_bPipeWorking = true;

				Packet pkt = Packet(PIPE_LOAD_POINTER);

				pkt << int32_t(m_mapAddress.size());

				for (auto e : m_mapAddress)
				{
					pkt << e.first << e.second;
				}

				SendPipeServer(pkt);
			}
		}
	}
}

void Bot::LoadAccountList()
{
	try
	{
		m_szAccountListFilePath = skCryptDec(".\\data\\accounts.json");

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

	if (m_pTbl_Skill_Extension2)
		m_pTbl_Skill_Extension2->Release();

	if (m_pTbl_Skill_Extension4)
		m_pTbl_Skill_Extension4->Release();

	if (m_pTbl_Item)
		m_pTbl_Item->Release();

	if (m_pTbl_Npc)
		m_pTbl_Npc->Release();

	if (m_pTbl_Mob)
		m_pTbl_Mob->Release();

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
			szPlatformPrefix = "us";
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
	snprintf(szPath, sizeof(szPath), skCryptDec("%s\\Data\\item_org_%s.tbl"), m_szClientPath.c_str(), szPlatformPrefix.c_str());
	m_pTbl_Item->Load(szPath);

#ifdef DEBUG
	printf("InitializeStaticData: Loaded %d items\n", m_pTbl_Item->GetDataSize());
#endif

	m_pTbl_Npc = new Table<__TABLE_NPC>();
	snprintf(szPath, sizeof(szPath), skCryptDec("%s\\Data\\npc_%s.tbl"), m_szClientPath.c_str(), szPlatformPrefix.c_str());
	m_pTbl_Npc->Load(szPath);

#ifdef DEBUG
	printf("InitializeStaticData: Loaded %d npcs\n", m_pTbl_Npc->GetDataSize());
#endif

	m_pTbl_Mob = new Table<__TABLE_MOB_USKO>();
	snprintf(szPath, sizeof(szPath), skCryptDec("%s\\Data\\mob_%s.tbl"), m_szClientPath.c_str(), szPlatformPrefix.c_str());
	m_pTbl_Mob->Load(szPath);

#ifdef DEBUG
	printf("InitializeStaticData: Loaded %d mobs\n", m_pTbl_Mob->GetDataSize());
#endif

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
		std::ifstream i(
			std::filesystem::current_path().string() 
			+ skCryptDec("\\data\\") 
			+ skCryptDec("supply.json"));

		m_jSupplyList = JSON::parse(i);
	}
	catch (const std::exception& e)
	{
		DBG_UNREFERENCED_PARAMETER(e);

#ifdef _DEBUG
		printf("%s\n", e.what());
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
			std::filesystem::current_path().string()
			+ skCryptDec("\\data\\")
			+ skCryptDec("health.json"));

		m_jHealthBuffList = JSON::parse(iHealthBuffList);

		std::ifstream iDefenceBuffList(
			std::filesystem::current_path().string()
			+ skCryptDec("\\data\\")
			+ skCryptDec("defence.json"));

		m_jDefenceBuffList = JSON::parse(iDefenceBuffList);

		std::ifstream iMindBuffList(
			std::filesystem::current_path().string()
			+ skCryptDec("\\data\\")
			+ skCryptDec("mind.json"));

		m_jMindBuffList = JSON::parse(iMindBuffList);

		std::ifstream iHealList(
			std::filesystem::current_path().string()
			+ skCryptDec("\\data\\")
			+ skCryptDec("heal.json"));

		m_jHealList = JSON::parse(iHealList);
	}
	catch (const std::exception& e)
	{
		DBG_UNREFERENCED_PARAMETER(e);

#ifdef _DEBUG
		printf("%s\n", e.what());
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

void Bot::OnPong()
{
#ifdef DEBUG
	printf("Bot: OnPong\n");
#endif

	if (m_iniUserConfiguration)
	{
		if (m_iniUserConfiguration->onSaveEvent)
			m_iniUserConfiguration->onSaveEvent();
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

	SendInjectionRequest(injectedProcessInfo.dwProcessId);

	//Injection(injectedProcessInfo.dwProcessId, "C:\\Users\\Administrator\\Documents\\Github\\Pipeline\\Debug\\Pipeline.dll");

#ifndef NO_INITIALIZE_CLIENT_HANDLER
	m_ClientHandler = new ClientHandler(this);
	m_ClientHandler->Initialize();
#endif
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

	new std::thread([this]() 
	{ 
		WaitCondition(IsTableLoaded() == false);

		m_ClientHandler->SetConfigurationLoaded(true);
		m_ClientHandler->StartHandler();

		new std::thread([this]() { UI::Render(this); });
	});
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

bool Bot::GetNpcTable(std::map<uint32_t, __TABLE_NPC>** mapDataOut)
{
	if (m_pTbl_Npc == nullptr)
		return false;

	return m_pTbl_Npc->GetData(mapDataOut);
}

bool Bot::GetMobTable(std::map<uint32_t, __TABLE_MOB_USKO>** mapDataOut)
{
	if (m_pTbl_Mob == nullptr)
		return false;

	return m_pTbl_Mob->GetData(mapDataOut);
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
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetInjectedProcessId());

	if (hProcess == nullptr)
		return 0;

	BYTE nRet = Memory::ReadByte(hProcess, dwAddress);

	CloseHandle(hProcess);
	return nRet;
}

DWORD Bot::Read4Byte(DWORD dwAddress)
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetInjectedProcessId());

	if (hProcess == nullptr)
		return 0;

	DWORD nRet = Memory::Read4Byte(hProcess, dwAddress);

	CloseHandle(hProcess);
	return nRet;
}

float Bot::ReadFloat(DWORD dwAddress)
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetInjectedProcessId());

	if (hProcess == nullptr)
		return 0.0f;

	float nRet = Memory::ReadFloat(hProcess, dwAddress);

	CloseHandle(hProcess);
	return nRet;
}

std::string Bot::ReadString(DWORD dwAddress, size_t nSize)
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetInjectedProcessId());

	if (hProcess == nullptr)
		return "";

	std::string nRet = Memory::ReadString(hProcess, dwAddress, nSize);

	CloseHandle(hProcess);
	return nRet;
}

std::vector<BYTE> Bot::ReadBytes(DWORD dwAddress, size_t nSize)
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetInjectedProcessId());

	std::vector<BYTE> nRet;
	if (hProcess == nullptr)
		return nRet;

	nRet = Memory::ReadBytes(hProcess, dwAddress, nSize);

	CloseHandle(hProcess);
	return nRet;
}

void Bot::WriteByte(DWORD dwAddress, BYTE byValue)
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetInjectedProcessId());

	if (hProcess == nullptr)
		return;

	Memory::WriteByte(hProcess, dwAddress, byValue);
	CloseHandle(hProcess);
}

void Bot::Write4Byte(DWORD dwAddress, int iValue)
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetInjectedProcessId());

	if (hProcess == nullptr)
		return;

	Memory::Write4Byte(hProcess, dwAddress, iValue);
	CloseHandle(hProcess);
}

void Bot::WriteFloat(DWORD dwAddress, float fValue)
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetInjectedProcessId());

	if (hProcess == nullptr)
		return;

	Memory::WriteFloat(hProcess, dwAddress, fValue);
	CloseHandle(hProcess);
}

void Bot::WriteString(DWORD dwAddress, std::string strValue)
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetInjectedProcessId());

	if (hProcess == nullptr)
		return;

	Memory::WriteString(hProcess, dwAddress, strValue);
	CloseHandle(hProcess);
}

void Bot::WriteBytes(DWORD dwAddress, std::vector<BYTE> byValue)
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetInjectedProcessId());

	if (hProcess == nullptr)
		return;

	Memory::WriteBytes(hProcess, dwAddress, byValue);
	CloseHandle(hProcess);
}

bool Bot::ExecuteRemoteCode(HANDLE hProcess, BYTE* codes, size_t psize)
{
	return Memory::ExecuteRemoteCode(hProcess, codes, psize);
}

bool Bot::ExecuteRemoteCode(HANDLE hProcess, LPVOID pAddress)
{
	return Memory::ExecuteRemoteCode(hProcess, pAddress);
}

bool Bot::IsInjectedProcessLost()
{
	if (GetInjectedProcessId() == 0)
		return true;

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetInjectedProcessId());

	if (hProcess == nullptr)
		return true;

	DWORD iExitCode = 0;
	if (GetExitCodeProcess(hProcess, &iExitCode) == FALSE)
	{
		CloseHandle(hProcess);
		return true;
	}

	if (iExitCode != STILL_ACTIVE)
	{
		CloseHandle(hProcess);
		return true;
	}

	CloseHandle(hProcess);
	return false;
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
	auto pAddressMap = m_iniPointer->GetConfigMap();

	if (pAddressMap == nullptr)
		return;

	auto it = pAddressMap->find(skCryptDec("Address"));

	if (it != pAddressMap->end())
	{
		for (auto e : it->second)
			m_mapAddress.insert(std::make_pair(e.first, std::strtoul(e.second.c_str(), NULL, 16)));
	}

	delete m_iniPointer;
	m_iniPointer = nullptr;
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
		WriteFile(m_hPipe,
			pkt.contents(),
			pkt.size(),
			0,
			NULL);
	}
}