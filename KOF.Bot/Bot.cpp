#include "pch.h"
#include "Bot.h"
#include "ClientHandler.h"
#include "UI.h"
#include "Memory.h"
#include "Remap.h"
#include "HardwareInformation.h"

Bot::Bot()
{
	m_ClientHandler = nullptr;

	m_bTableLoaded = false;
	m_pTbl_Skill = nullptr;
	m_pTbl_Skill_Extension2 = nullptr;
	m_pTbl_Skill_Extension3 = nullptr;
	m_pTbl_Skill_Extension4 = nullptr;
	m_pTbl_Item = nullptr;

	for (size_t i = 0; i < 45; i++)
	{
		m_pTbl_Item_Extension[i] = nullptr;
	}

	m_pTbl_Npc = nullptr;
	m_pTbl_Mob = nullptr;
	m_pTbl_ItemSell = nullptr;
	m_pTbl_Disguise_Ring = nullptr;

	m_szClientPath = skCryptDec(DEVELOPMENT_PATH);
	m_szClientExe = skCryptDec(DEVELOPMENT_EXE);

	m_bClosed = false;

	m_msLastInitializeHandle = std::chrono::milliseconds(0);

	m_bInternalMailslotWorking = false;

	m_hModuleAnyOTP = nullptr;
	m_InjectedProcessInfo = PROCESS_INFORMATION();

	m_hInternalMailslot = INVALID_HANDLE_VALUE;

	m_bAuthenticated = false;

	m_bUpdate = false;

	m_bUserSaveRequested = false;

	m_bOnReady = false;

	m_bStarted = false;
	m_bForceClosed = false;

	Initialize();
}

Bot::~Bot()
{
	m_ClientHandler = nullptr;

	m_bTableLoaded = false;

	m_pTbl_Skill = nullptr;
	m_pTbl_Skill_Extension2 = nullptr;
	m_pTbl_Skill_Extension3 = nullptr;
	m_pTbl_Skill_Extension4 = nullptr;
	m_pTbl_Item = nullptr;

	for (size_t i = 0; i < 45; i++)
	{
		m_pTbl_Item_Extension[i] = nullptr;
	}

	m_pTbl_Npc = nullptr;
	m_pTbl_Mob = nullptr;
	m_pTbl_ItemSell = nullptr;
	m_pTbl_Disguise_Ring = nullptr;

	m_szClientPath.clear();
	m_szClientExe.clear();

	m_bClosed = true;

	m_msLastInitializeHandle = std::chrono::milliseconds(0);

	m_hInternalMailslot = nullptr;
	m_bInternalMailslotWorking = false;

	m_hModuleAnyOTP = nullptr;
	m_InjectedProcessInfo = PROCESS_INFORMATION();

	m_bAuthenticated = false;
	m_bUpdate = false;

	m_bUserSaveRequested = false;

	m_bOnReady = false;

	m_bStarted = false;
	m_bForceClosed = false;

	Close();
}

void Bot::Initialize()
{
#ifdef DEBUG
	printf("Bot: Initialize\n");
#endif

	m_hardwareInfo.LoadHardwareInformation();

	InitializeAnyOTPService();

	m_ePlatformType = (PlatformType)DEVELOPMENT_PLATFORM;

	GetService()->Initialize();

	m_startTime = std::chrono::system_clock::now();

	UI::Render(this);
}

void Bot::Process()
{
	if (m_bUserSaveRequested)
	{
		SendSaveUserConfiguration(GetClientHandler()->GetServerId(), GetClientHandler()->GetName());
		m_bUserSaveRequested = false;
	}

	if (m_bInternalMailslotWorking == false)
	{
		if (ConnectInternalMailslot())
		{
#ifdef DEBUG
			printf("Bot: Internal Connection Ready\n");
#endif
			Packet pkt = Packet(PIPE_LOAD_POINTER);

			pkt << int32_t(m_mapAddress.size());

			for (auto& e : m_mapAddress)
			{
				pkt << e.first << e.second;
			}

			SendInternalMailslot(pkt);

			m_bInternalMailslotWorking = true;
		}
	}

	if (m_bAutoLogin && !m_bForceClosed)
	{
		if (GetInjectedProcessId() != 0 && IsInjectedProcessLost())
		{
#ifdef DEBUG
			printf("Bot: Auto Login Process Started\n");
#endif

			PROCESS_INFORMATION botProcessInfo;
			StartProcess(std::filesystem::current_path().string(), skCryptDec("\\Discord.exe"), "", botProcessInfo);

			Close();

			exit(0);
		}
	}

	if (m_ClientHandler)
	{
		m_ClientHandler->Process();

		if (m_bAutoLogin)
		{
			if (m_ClientHandler->IsDisconnect())
			{
				if (m_ClientHandler->m_msLastDisconnectTime == std::chrono::milliseconds(0))
				{
#ifdef DEBUG
					printf("Bot: Disconnected, Auto Login Process Starting In 60 Seconds\n");
#endif

					m_ClientHandler->m_msLastDisconnectTime = duration_cast<std::chrono::milliseconds>(
						std::chrono::system_clock::now().time_since_epoch()
					);
				}
				else
				{
					std::chrono::milliseconds msNow = duration_cast<std::chrono::milliseconds>(
						std::chrono::system_clock::now().time_since_epoch()
					);

					if ((m_ClientHandler->m_msLastDisconnectTime.count() + 60000) < msNow.count())
					{
#ifdef DEBUG
						printf("Bot: Auto Login Process Started\n");
#endif

						PROCESS_INFORMATION botProcessInfo;
						StartProcess(std::filesystem::current_path().string(), skCryptDec("\\Discord.exe"), "", botProcessInfo);

						Close();

						exit(0);
					}
				}
			}
			else
			{
				if (m_ClientHandler->m_msLastDisconnectTime != std::chrono::milliseconds(0))
				{
#ifdef DEBUG
					printf("Bot: Auto Login Process Stopped, Connection Return Back\n");
#endif
				}

				m_ClientHandler->m_msLastDisconnectTime = std::chrono::milliseconds(0);
			}
		}
	}
}

void Bot::Close()
{
#ifdef DEBUG
	printf("Bot: Closing\n");
#endif

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

	if (m_pTbl_Skill_Extension3)
		m_pTbl_Skill_Extension3->Release();

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
			szPlatformPrefix = "nc";
		}
		break;

		case PlatformType::JPKO:
		{
			szPlatformPrefix = "jp";
		}
		break;

		case PlatformType::STKO:
		{
			szPlatformPrefix = "st";
		}
		break;
	}

	char szPath[255];

	m_pTbl_Skill = new Table<__TABLE_UPC_SKILL>();
	snprintf(szPath, sizeof(szPath), skCryptDec("%s\\Data\\skill_magic_main_%s.tbl"), m_szClientPath.c_str(), szPlatformPrefix.c_str());
	m_pTbl_Skill->Load(szPath);

	m_pTbl_Skill_Extension2 = new Table<__TABLE_UPC_SKILL_EXTENSION2>();
	m_pTbl_Skill_Extension2->Load(m_szClientPath + skCryptDec("\\Data\\skill_magic_2.tbl"));

	m_pTbl_Skill_Extension3 = new Table<__TABLE_UPC_SKILL_EXTENSION3>();
	m_pTbl_Skill_Extension3->Load(m_szClientPath + skCryptDec("\\Data\\skill_magic_3.tbl"));

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

	m_pTbl_Mob = new Table<__TABLE_MOB>();
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

void Bot::OnReady()
{
#ifdef DEBUG
	printf("Bot: OnReady\n");
#endif

	if (m_szToken.size() > 0)
	{
		SendLogin(m_szToken);
	}

	m_bOnReady = true;
}

void Bot::OnPong()
{
#ifdef DEBUG
	printf("Bot: OnPong\n");
#endif

	if (m_ClientHandler 
		&& m_ClientHandler->IsWorking())
	{
		SendPong(
			m_ClientHandler->m_PlayerMySelf.szName, 
			m_ClientHandler->m_PlayerMySelf.fX, 
			m_ClientHandler->m_PlayerMySelf.fY, 
			m_ClientHandler->m_PlayerMySelf.iCity);
	}
	else
	{
		SendPong("", 0.0f, 0.0f, 0);
	}
}

void Bot::OnConnected()
{
#ifdef DEBUG
	printf("Bot: OnConnected\n");
#endif

	uint32_t iCRC = CalculateCRC32(skCryptDec("Discord.exe"));

	if (iCRC == 0xFFFFFFFF)
	{
#ifdef DEBUG
		printf("Bot: CRC Calculate failed\n");
		return;
#endif
	}

	SendReady(iCRC);
}

void Bot::OnAuthenticated()
{
#ifdef DEBUG
	printf("Bot: OnAuthenticated\n");
#endif

	SendRouteLoadRequest();

	m_bAuthenticated = true;
}

void Bot::OnUpdate()
{
#ifdef DEBUG
	printf("Bot: OnUpdate\n");
#endif

	SendUpdate();

	m_bUpdate = true;
}

void Bot::OnUpdateDownloaded(bool bStatus)
{
#ifdef DEBUG
	printf("Bot: OnUpdateDownloaded: %d\n", bStatus);
#endif

	if (bStatus)
	{
		std::filesystem::path currentPath = std::filesystem::current_path();

		PROCESS_INFORMATION updateProcessInfo;
		if (!StartProcess(currentPath.string(), skCryptDec("Updater.exe"), "", updateProcessInfo))
		{
#ifdef DEBUG
			printf("OnUpdateDownloaded: Update process start failed\n");
#endif
		}
	}
	else
	{

	}
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

	bool bAutoStart = GetAppConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Start"), false);

	if (bAutoStart)
	{
		StartGame();
	}
}

void Bot::OnCaptchaResponse(bool bStatus, std::string szResult)
{
#ifdef DEBUG
	printf("Bot: OnCaptchaResponse: %d - %s\n", bStatus ? 1 : 0, szResult.c_str());
#endif

	if (!bStatus)
	{
		m_ClientHandler->RefreshCaptcha();
	}
	else
	{
		m_ClientHandler->SendCaptcha(szResult);
	}
}

void Bot::Patch(HANDLE hProcess)
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	WaitCondition(Read4Byte(GetAddress(skCryptDec("KO_OFF_WIZ_HACKTOOL_NOP1"))) == 0);

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

	m_ClientHandler->InitializeUserConfiguration();

#ifdef DEBUG
	printf("User configuration loaded\n");
#endif

	m_iniUserConfiguration->onSaveEvent = [=]()
	{
		m_bUserSaveRequested = true;
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

bool Bot::GetSkillExtension3Table(std::map<uint32_t, __TABLE_UPC_SKILL_EXTENSION3>** mapDataOut)
{
	if (m_pTbl_Skill_Extension3 == nullptr)
		return false;

	return m_pTbl_Skill_Extension3->GetData(mapDataOut);
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

bool Bot::GetMobTable(std::map<uint32_t, __TABLE_MOB>** mapDataOut)
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
	HANDLE hProcess = GetInjectedProcessHandle();
	BYTE nRet = Memory::ReadByte(hProcess, dwAddress);
	return nRet;
}

WORD Bot::Read2Byte(DWORD dwAddress)
{
	HANDLE hProcess = GetInjectedProcessHandle();
	WORD nRet = Memory::ReadByte(hProcess, dwAddress);
	return nRet;
}

DWORD Bot::Read4Byte(DWORD dwAddress)
{
	HANDLE hProcess = GetInjectedProcessHandle();
	DWORD nRet = Memory::Read4Byte(hProcess, dwAddress);
	return nRet;
}

float Bot::ReadFloat(DWORD dwAddress)
{
	HANDLE hProcess = GetInjectedProcessHandle();
	float nRet = Memory::ReadFloat(hProcess, dwAddress);
	return nRet;
}

std::string Bot::ReadString(DWORD dwAddress, size_t nSize)
{
	HANDLE hProcess = GetInjectedProcessHandle();
	std::string nRet = Memory::ReadString(hProcess, dwAddress, nSize);
	return nRet;
}

std::vector<BYTE> Bot::ReadBytes(DWORD dwAddress, size_t nSize)
{
	std::vector<BYTE> nRet;
	HANDLE hProcess = GetInjectedProcessHandle();
	nRet = Memory::ReadBytes(hProcess, dwAddress, nSize);
	return nRet;
}

void Bot::WriteByte(DWORD dwAddress, BYTE byValue)
{
	HANDLE hProcess = GetInjectedProcessHandle();
	Memory::WriteByte(hProcess, dwAddress, byValue);
}

void Bot::Write4Byte(DWORD dwAddress, int iValue)
{
	HANDLE hProcess = GetInjectedProcessHandle();
	Memory::Write4Byte(hProcess, dwAddress, iValue);
}

void Bot::WriteFloat(DWORD dwAddress, float fValue)
{
	HANDLE hProcess = GetInjectedProcessHandle();
	Memory::WriteFloat(hProcess, dwAddress, fValue);
}

void Bot::WriteString(DWORD dwAddress, std::string strValue)
{
	HANDLE hProcess = GetInjectedProcessHandle();
	Memory::WriteString(hProcess, dwAddress, strValue);
}

void Bot::WriteBytes(DWORD dwAddress, std::vector<BYTE> byValue)
{
	HANDLE hProcess = GetInjectedProcessHandle();
	Memory::WriteBytes(hProcess, dwAddress, byValue);
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
	return m_mapAddress[szAddressName];
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

bool Bot::ConnectInternalMailslot()
{
	m_hInternalMailslot = CreateFile(skCryptDec("\\\\.\\mailslot\\Internal"),
		GENERIC_WRITE,
		0,
		NULL,
		OPEN_ALWAYS,
		0,
		NULL);

	if (m_hInternalMailslot != INVALID_HANDLE_VALUE)
	{
		return true;
	}

	return false;
}

void Bot::SendInternalMailslot(Packet pkt)
{
	if (m_hInternalMailslot != INVALID_HANDLE_VALUE)
	{
		DWORD bytesWritten;

		if (!WriteFile(m_hInternalMailslot, pkt.contents(), pkt.size(), &bytesWritten, nullptr))
		{
#ifdef DEBUG
			printf("SendInternalMailslot: Failed to write to mailslot. Error code: %d\n", GetLastError());
#endif
		}
	}
	else
	{
#ifdef DEBUG
		printf("SendInternalMailslot: m_hInternalMailslot == INVALID_HANDLE_VALUE\n");
#endif
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
	if (m_InjectedProcessInfo.hProcess != NULL 
		&& m_InjectedProcessInfo.hProcess != INVALID_HANDLE_VALUE)
	{
		DWORD dwFlags;

		if (GetHandleInformation(m_InjectedProcessInfo.hProcess, &dwFlags))
		{
			return m_InjectedProcessInfo.hProcess;
		}
		else
		{
			//CloseHandle(m_InjectedProcessInfo.hProcess);
			m_InjectedProcessInfo.hProcess = NULL;
		}
	}

	m_InjectedProcessInfo.hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetInjectedProcessId());
	return m_InjectedProcessInfo.hProcess;
}

std::wstring Bot::GetAnyOTPHardwareID()
{
	std::wstring szOTPHardwareID = L"";

	for (size_t i = 0; i < m_hardwareInfo.Disk.size(); i++)
	{
		HardwareInformation::DiskObject& Disk{ m_hardwareInfo.Disk.at(i) };

		if (!Disk.IsBootDrive)
			continue;

		std::wstring szPNPDeviceID = Disk.PNPDeviceID;

		if (Disk.Signature == 0)
		{
			szOTPHardwareID = L"0000" + szPNPDeviceID.substr(szPNPDeviceID.length() - 16);
		}
		else
		{
			std::wstringstream sstream;
			sstream << std::hex << Disk.Signature;
			sstream << std::hex << std::uppercase << Disk.Signature;
			std::wstring szSignature = sstream.str();

			szOTPHardwareID = szSignature.substr(0, 4) + szPNPDeviceID.substr(szPNPDeviceID.length() - 16);
		}
	}

	return szOTPHardwareID;
}

void Bot::InitializeAnyOTPService()
{
	std::string filePath;

	if (std::filesystem::exists(skCryptDec("C:\\Program Files\\AnyOTPSetup\\AnyOTPBiz.dll"))) 
	{
		m_hModuleAnyOTP = LoadLibraryExW(skCryptDec(L"C:\\Program Files\\AnyOTPSetup\\AnyOTPBiz.dll"), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	}
	else if (std::filesystem::exists(skCryptDec("C:\\Program Files (x86)\\AnyOTPSetup\\AnyOTPBiz.dll")))
	{
		m_hModuleAnyOTP = LoadLibraryExW(skCryptDec(L"C:\\Program Files (x86)\\AnyOTPSetup\\AnyOTPBiz.dll"), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	}

	if (m_hModuleAnyOTP == NULL)
	{
#ifdef _DEBUG
		printf("Bot:: DLL Not Loaded\n");
#endif
		return;
	}

#ifdef _DEBUG
	printf("Bot: AnyOTP Library Loaded\n");
#endif

	m_szAnyOTPID = to_string(GetAnyOTPHardwareID().c_str());
}

std::wstring Bot::ReadAnyOTPCode(std::string szPassword, std::string szHardwareID)
{
	std::wstring szCode = L"";
	std::wstring szOTPPassword(szPassword.begin(), szPassword.end());
	std::wstring szOTPHardwareID = GetAnyOTPHardwareID();

	if (szHardwareID.size() > 0)
	{
		std::wstring szCustomOTPHardwareID(szHardwareID.begin(), szHardwareID.end());
		szOTPHardwareID = szCustomOTPHardwareID;
	}

	auto pGenerateOTPAddress = (LPVOID)((DWORD)(m_hModuleAnyOTP)+0x6327);
	GenerateOTP pGenerateOTP = reinterpret_cast<GenerateOTP>(pGenerateOTPAddress);

	int iCodeAddress;
	pGenerateOTP(0, szOTPHardwareID.c_str(), szOTPPassword.c_str(), &iCodeAddress);

	const int bufferSize = 6 * 2;
	WCHAR buffer[bufferSize] = { 0 };

	SIZE_T bytesRead;
	if (!ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(iCodeAddress), buffer, bufferSize, &bytesRead))
	{
#ifdef _DEBUG
		printf("Bot: ReadProcessMemory Failed\n");
#endif
		return szCode;
	}

	szCode = buffer;

	return szCode;
}

void Bot::StartGame()
{
#ifdef VMPROTECT
	VMProtectBeginUltra("Bypass Function");
#endif

	m_bStarted = true;

	new std::thread([&]()
	{
		std::vector<const char*> fileNames = {
		#ifdef DEBUG
			skCryptDec("KOF.exe"),
		#endif
			skCryptDec("Updater.exe"),
			skCryptDec("Updater2.exe"),
			skCryptDec("KnightOnLine.exe"),
			skCryptDec("xldr_KnightOnline_NA.exe"),
			skCryptDec("xldr_KnightOnline_NA_loader_win32.exe"),
			skCryptDec("xldr_KnightOnline_GB.exe"),
			skCryptDec("xldr_KnightOnline_GB_loader_win32.exe") 
		};

		KillProcessesByFileNames(fileNames);

		while (IsProcessRunning(skCryptDec("Updater.exe")))
		{
			Sleep(1000);
			KillProcessesByFileName(skCryptDec("Updater.exe"));
			Sleep(1000);
		}

#ifndef DISABLE_XIGNCODE
		if (m_ePlatformType == PlatformType::USKO)
		{
			PROCESS_INFORMATION loaderProcessInfo;
			std::ostringstream strLoaderCommandLine;
			strLoaderCommandLine << m_szClientPath + "\\" + m_szClientExe;

			if (!StartProcess(
				m_szClientPath + skCryptDec("\\XIGNCODE\\"), skCryptDec("xldr_KnightOnline_NA_loader_win32.exe"), strLoaderCommandLine.str(), loaderProcessInfo))
			{
#ifdef DEBUG
				printf("Bot: Loader cannot started\n");
#endif
				Close();
				exit(0);
				return;
			}
		}
		else if (m_ePlatformType == PlatformType::STKO)
		{
			PROCESS_INFORMATION loaderProcessInfo;
			std::ostringstream strLoaderCommandLine;
			strLoaderCommandLine << m_szClientPath + "\\" + m_szClientExe;

			if (!StartProcess(
				m_szClientPath + skCryptDec("\\XIGNCODE\\"), skCryptDec("xldr_KnightOnline_GB_loader_win32.exe"), strLoaderCommandLine.str(), loaderProcessInfo))
			{
#ifdef DEBUG
				printf("Bot: Loader cannot started\n");
#endif
				Close();
				exit(0);
				return;
			}
		}
#endif

		std::ostringstream strCommandLine;

		if (m_ePlatformType == PlatformType::USKO
			|| m_ePlatformType == PlatformType::CNKO
			|| m_ePlatformType == PlatformType::STKO)
		{
			strCommandLine << GetCurrentProcessId();
		}
		else if (m_ePlatformType == PlatformType::KOKO)
		{
			strCommandLine << skCryptDec("ongate MVGHONG4 NDAYOK1EPZFQT1P6TIQA0YE7ZTD0IWN8LS1V10JLT1V185JX00OMLNQ0 2330316151 15100 0");
		}

		if (!StartProcess(m_szClientPath, m_szClientExe, strCommandLine.str(), m_InjectedProcessInfo))
		{
#ifdef DEBUG
			printf("Bot: Process cannot started\n");
#endif
			Close();
			exit(0);
			return;
		}

#ifdef DEBUG
		printf("Bot: Knight Online process started\n");
#endif

		if (m_ePlatformType == PlatformType::USKO
			|| m_ePlatformType == PlatformType::KOKO
			|| m_ePlatformType == PlatformType::STKO)
		{

#ifdef DEBUG
			printf("Bot: Waiting entry point\n");
#endif

			WaitCondition(Read4Byte(GetAddress(skCryptDec("KO_XIGNCODE_ENTRY_POINT"))) == 0);

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
				BYTE byPatch2[] = { 0xE9, 0x39, 0x03, 0x00, 0x00, 0x90 };
				WriteProcessMemory(m_InjectedProcessInfo.hProcess, (LPVOID*)GetAddress(skCryptDec("KO_XIGNCODE_ENTRY_POINT")), byPatch2, sizeof(byPatch2), 0);
			}
			else if (m_ePlatformType == PlatformType::KOKO)
			{
				BYTE byPatch2[] = { 0xE9, 0x50, 0x07, 0x00, 0x00, 0x90 };
				WriteProcessMemory(m_InjectedProcessInfo.hProcess, (LPVOID*)GetAddress(skCryptDec("KO_XIGNCODE_ENTRY_POINT")), byPatch2, sizeof(byPatch2), 0);
			}
			else if (m_ePlatformType == PlatformType::STKO)
			{
				BYTE byPatch2[] = { 0xE9, 0x09, 0x03, 0x00, 0x00, 0x90 };
				WriteProcessMemory(m_InjectedProcessInfo.hProcess, (LPVOID*)GetAddress(skCryptDec("KO_XIGNCODE_ENTRY_POINT")), byPatch2, sizeof(byPatch2), 0);
			}
#endif

#ifdef DEBUG
			printf("Bot: Bypass finished, Knight Online process resuming\n");
#endif

			ResumeProcess(m_InjectedProcessInfo.hProcess);
		}

		SendInjectionRequest(m_InjectedProcessInfo.dwProcessId);

		Patch(m_InjectedProcessInfo.hProcess);

		m_ClientHandler = new ClientHandler(this);
		m_ClientHandler->Initialize();

		m_bForceClosed = false;
	});

#ifdef VMPROTECT
	VMProtectEnd();
#endif
}

void Bot::StopStartGameProcess()
{
	if (!m_bStarted)
		return;

	m_bForceClosed = true;

	std::vector<const char*> fileNames = 
	{
	#ifdef DEBUG
		skCryptDec("KOF.exe"),
	#endif
		skCryptDec("Updater.exe"),
		skCryptDec("Updater2.exe"),
		skCryptDec("KnightOnLine.exe"),
		skCryptDec("xldr_KnightOnline_NA.exe"),
		skCryptDec("xldr_KnightOnline_NA_loader_win32.exe"),
		skCryptDec("xldr_KnightOnline_GB.exe"),
		skCryptDec("xldr_KnightOnline_GB_loader_win32.exe")
	};

	KillProcessesByFileNames(fileNames);

	if (m_ClientHandler) 
	{
		m_ClientHandler->Clear();

		delete m_ClientHandler;
		m_ClientHandler = nullptr;
	}

	m_bStarted = false;
}