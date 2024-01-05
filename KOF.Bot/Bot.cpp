#include "pch.h"
#include "Bot.h"
#include "ClientHandler.h"
#include "UI.h"
#include "Memory.h"
#include "Remap.h"
#include "HardwareInformation.h"
#include "Drawing.h"
#include "Injector.h"

Bot::Bot(HardwareInformation* pHardwareInformation)
{
	m_pHardwareInformation = pHardwareInformation;
	m_pClientHandler = nullptr;

	m_bTableLoading = false;
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

	m_szClientPath = skCryptDec(INJECT_PATH);
	m_szClientExe = skCryptDec(INJECT_EXE);
	m_ePlatformType = (PlatformType)INJECT_PLATFORM;

	m_bInternalMailslotWorking = false;

	m_hModuleAnyOTP = nullptr;
	m_ClientProcessInfo = PROCESS_INFORMATION();

	m_hInternalMailslot = INVALID_HANDLE_VALUE;

	m_bAuthenticated = false;

	m_bUserSaveRequested = false;

	m_bGameStarting = false;

	m_iniAppConfiguration = nullptr;

	m_bAutoLogin = false;
	m_bAutoStart = false;

	m_szID = "";
	m_szPassword = "";

	m_iServerID = 0;
	m_iChannelID = 0;
	m_iSlotID = 1;

	m_szAnyOTPID.clear();
	m_szAnyOTPPassword.clear();

	m_szToken.clear();

	m_fLastPongTime = TimeGet();

	m_iSubscriptionEndAt = 0;
	m_iCredit = 0;

	m_bUserSaveRequested = false;

	m_isAuthenticationMessage = false;
	m_szAuthenticationMessage = "";

	m_mapAddress.clear();

	m_szOTPHardwareID.clear();

	m_szProxyIP.clear();
	m_iProxyPort = 0;
	m_szProxyUsername.clear();
	m_szProxyPassword.clear();
	m_bConnectWithProxy = 0;

	m_bCheckingProxy = false;
	m_szCheckingProxyResult.clear();
	m_bCheckingProxyResult = false;

	Initialize();
}

Bot::~Bot()
{
	CloseSocket();

	if (m_pClientHandler)
	{
		m_pClientHandler->Clear();
		delete m_pClientHandler;
	}

	m_pClientHandler = nullptr;

	m_pHardwareInformation = nullptr;

	m_bTableLoading = false;
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

	m_hInternalMailslot = nullptr;
	m_bInternalMailslotWorking = false;

	m_hModuleAnyOTP = nullptr;
	m_ClientProcessInfo = PROCESS_INFORMATION();

	m_bAuthenticated = false;

	m_bUserSaveRequested = false;

	m_bGameStarting = false;

	if (m_iniAppConfiguration)
		delete m_iniAppConfiguration;

	m_iniAppConfiguration = nullptr;

	m_bAutoLogin = false;
	m_bAutoStart = false;

	m_szID = "";
	m_szPassword = "";

	m_iServerID = 0;
	m_iChannelID = 0;
	m_iSlotID = 1;

	m_szAnyOTPID.clear();
	m_szAnyOTPPassword.clear();

	m_szToken.clear();

	m_fLastPongTime = TimeGet();

	m_iSubscriptionEndAt = 0;
	m_iCredit = 0;

	m_bUserSaveRequested = false;

	m_isAuthenticationMessage = false;
	m_szAuthenticationMessage = "";

	m_szClientPath = skCryptDec(INJECT_PATH);
	m_szClientExe = skCryptDec(INJECT_EXE);
	m_ePlatformType = (PlatformType)INJECT_PLATFORM;

	m_mapAddress.clear();

	m_szOTPHardwareID.clear();

	m_szProxyIP.clear();
	m_iProxyPort = 0;
	m_szProxyUsername.clear();
	m_szProxyPassword.clear();
	m_bConnectWithProxy = 0;

	m_bCheckingProxy = false;
	m_szCheckingProxyResult.clear();
	m_bCheckingProxyResult = false;

	Release();
}

void Bot::Initialize()
{
	std::string szIniPath = skCryptDec(".\\Config.ini");

	m_iniAppConfiguration = new Ini();
	m_iniAppConfiguration->Load(szIniPath.c_str());

	m_szToken = m_iniAppConfiguration->GetString(skCryptDec("Internal"), skCryptDec("Token"), m_szToken.c_str());

	m_bAutoLogin = m_iniAppConfiguration->GetBool(skCryptDec("Automation"), skCryptDec("Login"), m_bAutoLogin);
	m_bAutoStart = m_iniAppConfiguration->GetBool(skCryptDec("Automation"), skCryptDec("Start"), m_bAutoStart);

	m_szID = m_iniAppConfiguration->GetString(skCryptDec("AutoLogin"), skCryptDec("ID"), m_szID.c_str());
	m_szPassword = m_iniAppConfiguration->GetString(skCryptDec("AutoLogin"), skCryptDec("Password"), m_szPassword.c_str());
	m_iServerID = m_iniAppConfiguration->GetInt(skCryptDec("AutoLogin"), skCryptDec("Server"), m_iServerID);
	m_iChannelID = m_iniAppConfiguration->GetInt(skCryptDec("AutoLogin"), skCryptDec("Channel"), m_iChannelID);
	m_iSlotID = m_iniAppConfiguration->GetInt(skCryptDec("AutoLogin"), skCryptDec("Slot"), m_iSlotID);

	m_szAnyOTPID = m_iniAppConfiguration->GetString(skCryptDec("AnyOTP"), skCryptDec("ID"), m_szAnyOTPID.c_str());
	m_szAnyOTPPassword = m_iniAppConfiguration->GetString(skCryptDec("AnyOTP"), skCryptDec("Password"), m_szAnyOTPPassword.c_str());

	m_szProxyIP = m_iniAppConfiguration->GetString(skCryptDec("Proxy"), skCryptDec("IP"), m_szProxyIP.c_str());
	m_iProxyPort = m_iniAppConfiguration->GetInt(skCryptDec("Proxy"), skCryptDec("Port"), m_iProxyPort);
	m_szProxyUsername = m_iniAppConfiguration->GetString(skCryptDec("Proxy"), skCryptDec("Username"), m_szProxyUsername.c_str());
	m_szProxyPassword = m_iniAppConfiguration->GetString(skCryptDec("Proxy"), skCryptDec("Password"), m_szProxyPassword.c_str());
	m_bConnectWithProxy = m_iniAppConfiguration->GetInt(skCryptDec("Proxy"), skCryptDec("Enable"), m_bConnectWithProxy);

	InitializeAnyOTPService();

	GetService()->Initialize();

	m_startTime = std::chrono::system_clock::now();

	UI::Render(this);
}

void Bot::Process()
{
#ifdef VMPROTECT
	VMProtectBeginMutation("Bot::Process");
#endif

	if (m_pClientHandler)
	{
		if (m_bUserSaveRequested)
		{
			SendSaveUserConfiguration(
				GetClientHandler()->GetServerId(), 
				GetClientHandler()->GetName(), 
				m_pClientHandler->m_iniUserConfiguration->Dump(),
				m_ePlatformType);

			m_bUserSaveRequested = false;
		}

		m_pClientHandler->Process();

		if (m_bAutoLogin)
		{
			if (m_pClientHandler->IsDisconnect())
			{
				if (m_pClientHandler->m_fLastDisconnectTime == 0.0f)
				{
#ifdef DEBUG_LOG
					Print("Disconnected, Auto Login Process Starting In 60 Seconds");
#endif

					m_pClientHandler->m_fLastDisconnectTime = TimeGet();
				}
				else
				{
					if (TimeGet() > (m_pClientHandler->m_fLastDisconnectTime + (60000.0f / 1000.0f)))
					{
						if (m_bTableLoading == true)
						{
#ifdef DEBUG_LOG
							Print("Auto Login Process Waiting Static Data Load");
#endif					
						}
						else
						{
#ifdef DEBUG_LOG
							Print("Auto Login Process Started");
#endif				
							m_pClientHandler->Clear();

							delete m_pClientHandler;
							m_pClientHandler = nullptr;

							m_bInternalMailslotWorking = false;

							Drawing::SetScene(Drawing::LOADER);

							StartGame();
						}
					}				
				}
			}
			else
			{
				if (m_pClientHandler->m_fLastDisconnectTime != 0.0f)
				{
#ifdef DEBUG_LOG
					Print("Auto Login Process Stopped, Connection Return Back");
#endif
				}

				m_pClientHandler->m_fLastDisconnectTime = 0.0f;
			}
		}
	}

#ifdef VMPROTECT
	VMProtectEnd();
#endif
}

void Bot::Release()
{
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
	return m_pClientHandler;
}

void Bot::InitializeStaticData()
{
	if (m_bTableLoaded)
		return;

	m_bTableLoading = true;

#ifdef DEBUG_LOG
	Print("Started");
#endif

	std::string szPlatformPrefix = "us";

	switch (m_ePlatformType)
	{
		case PlatformType::CNKO:
		{
			szPlatformPrefix = "nc";
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

	m_pTbl_Item = new Table<__TABLE_ITEM>();
	snprintf(szPath, sizeof(szPath), skCryptDec("%s\\Data\\item_org_%s.tbl"), m_szClientPath.c_str(), szPlatformPrefix.c_str());
	m_pTbl_Item->Load(szPath);

	for (size_t i = 0; i < 45; i++)
	{
		m_pTbl_Item_Extension[i] = new Table<__TABLE_ITEM_EXTENSION>();
		snprintf(szPath, sizeof(szPath), skCryptDec("%s\\Data\\item_ext_%d_%s.tbl"), m_szClientPath.c_str(), i, szPlatformPrefix.c_str());
		m_pTbl_Item_Extension[i]->Load(szPath);
	}

	m_pTbl_Npc = new Table<__TABLE_NPC>();
	snprintf(szPath, sizeof(szPath), skCryptDec("%s\\Data\\npc_%s.tbl"), m_szClientPath.c_str(), szPlatformPrefix.c_str());
	m_pTbl_Npc->Load(szPath);

	m_pTbl_Mob = new Table<__TABLE_MOB>();
	snprintf(szPath, sizeof(szPath), skCryptDec("%s\\Data\\mob_%s.tbl"), m_szClientPath.c_str(), szPlatformPrefix.c_str());
	m_pTbl_Mob->Load(szPath);

	m_pTbl_ItemSell = new Table<__TABLE_ITEM_SELL>();
	m_pTbl_ItemSell->Load(m_szClientPath + skCryptDec("\\Data\\itemsell_table.tbl"));

	m_pTbl_Disguise_Ring = new Table<__TABLE_DISGUISE_RING>();
	snprintf(szPath, sizeof(szPath), skCryptDec("%s\\Data\\disguisering_%s.tbl"), m_szClientPath.c_str(), szPlatformPrefix.c_str());
	m_pTbl_Disguise_Ring->Load(szPath);

#ifdef DEBUG_LOG
	Print("Finished");
#endif

	m_bTableLoading = false;
	m_bTableLoaded = true;
}

void Bot::OnReady()
{
	if (m_szToken.size() > 0)
	{
		SendLogin(m_szToken);
	}
	else
	{
		Drawing::SetScene(Drawing::Scene::LOGIN);
	}
}

void Bot::OnPong(uint32_t iSubscriptionEndAt, int32_t iCredit)
{
#ifdef VMPROTECT
	VMProtectBeginMutation("Bot::OnPong");
#endif

	if (m_pClientHandler
		&& m_pClientHandler->IsWorking())
	{
		SendPong(
			m_pClientHandler->m_PlayerMySelf.szName,
			m_pClientHandler->m_PlayerMySelf.fX,
			m_pClientHandler->m_PlayerMySelf.fY,
			m_pClientHandler->m_PlayerMySelf.iCity);
	}
	else
	{
		SendPong("", 0.0f, 0.0f, 0);
	}

	m_iSubscriptionEndAt = iSubscriptionEndAt;
	m_iCredit = iCredit;
	m_fLastPongTime = TimeGet();

#ifdef VMPROTECT
	VMProtectEnd();
#endif
}

void Bot::OnConnected()
{
#ifdef VMPROTECT
	VMProtectBeginUltra("Bot::OnConnected");
#endif

	char szCurrentProcessFilePath[MAX_PATH + 1];

	GetModuleFileName(NULL, szCurrentProcessFilePath, MAX_PATH);
	szCurrentProcessFilePath[MAX_PATH] = '\0';

	uint32_t iCRC = CalculateCRC32(szCurrentProcessFilePath);

	if (iCRC == 0xFFFFFFFF)
	{
#ifdef DEBUG_LOG
		Print("CRC Calculate failed");
#endif
		exit(0);
		return;
	}

	std::string szProcessFileName = GetFileName(szCurrentProcessFilePath);

	SendReady(szProcessFileName, iCRC, m_pHardwareInformation);

#ifdef VMPROTECT
	VMProtectEnd();
#endif
}

void Bot::OnAuthenticated(uint8_t iStatus)
{
#ifdef VMPROTECT
	VMProtectBeginMutation("Bot::OnAuthenticated");
#endif
	m_bAuthenticated = true;

	switch (iStatus)
	{
		case 1:
			SendPointerRequest(m_ePlatformType);
			Drawing::SetScene(Drawing::Scene::LOADER);
			break;

		case 2:
		{
#ifdef DEBUG_LOG
			SendPointerRequest(m_ePlatformType);
			Drawing::SetScene(Drawing::Scene::LOADER);
#else
			OnUpdate();
			Drawing::SetScene(Drawing::Scene::UPDATE);
#endif
		}
		break;
	}

#ifdef VMPROTECT
	VMProtectEnd();
#endif
}

void Bot::OnUpdate()
{
	SendUpdate();
}

void Bot::OnUpdateDownloaded(bool bStatus)
{
#ifdef DEBUG_LOG
	Print("%d", bStatus);
#endif

	if (bStatus)
	{
		char currentPath[FILENAME_MAX];
		_getcwd(currentPath, FILENAME_MAX);

		PROCESS_INFORMATION updateProcessInfo;
		if (!StartProcess(currentPath, skCryptDec("Updater.exe"), "", updateProcessInfo))
		{
#ifdef DEBUG_LOG
			Print("Update process start failed");
#endif
		}
	}
	else
	{

	}
}

void Bot::OnLoaded(std::string szPointerData)
{
	if (szPointerData.size() == 0)
		return;

	Ini* pIniPointer = new Ini();

	pIniPointer->Load(szPointerData);

	auto pAddressMap = pIniPointer->GetConfigMap();

	if (pAddressMap == nullptr)
		return;

	auto it = pAddressMap->find(skCryptDec("Address"));

	if (it != pAddressMap->end())
	{
		for (auto& e : it->second)
			m_mapAddress.insert(std::make_pair(e.first, std::strtoul(e.second.c_str(), NULL, 16)));
	}

	delete pIniPointer;

	if (m_iniAppConfiguration)
	{
		bool bAutoStart = m_iniAppConfiguration->GetBool(skCryptDec("Automation"), skCryptDec("Start"), false);

		if (bAutoStart)
		{
#ifdef DEBUG_LOG
			Print("Knight Online process starting");
#endif

			StartGame();
		}
	}
}

void Bot::OnSaveToken(std::string szToken, uint32_t iSubscriptionEndAt, int32_t iCredit)
{
	if (m_iniAppConfiguration)
	{
		m_szToken = m_iniAppConfiguration->SetString(skCryptDec("Internal"), skCryptDec("Token"), szToken.c_str());
	}
	
	m_iSubscriptionEndAt = iSubscriptionEndAt;
	m_iCredit = iCredit;
}

void Bot::OnInjection(std::vector<uint8_t> vecBuffer)
{
	VitalCode eInjection = Injection(m_ClientProcessInfo.hProcess, vecBuffer.data(), vecBuffer.size());

	if (eInjection != VITAL_CODE_INJECTION_SUCCESS)
	{
		SendVital(eInjection);
		return;
	}

	SendVital(eInjection);

	new std::thread([&]()
	{
		m_bInternalMailslotWorking = false;

		while (!m_bInternalMailslotWorking)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));

			if (ConnectInternalMailslot())
			{
				Packet pkt1 = Packet(PIPE_LOAD_POINTER);

				pkt1 << int32_t(m_mapAddress.size());

				for (auto& e : m_mapAddress)
				{
					pkt1 << e.first << e.second;
				}

				SendInternalMailslot(pkt1);

				if (m_bConnectWithProxy)
				{
					Packet pkt2 = Packet(PIPE_PROXY);

					pkt2 << m_szProxyIP << m_iProxyPort << m_szProxyUsername << m_szProxyPassword;

					SendInternalMailslot(pkt2);
				}

				m_bInternalMailslotWorking = true;
			}
			else
			{
#ifdef DEBUG_LOG
				Print("Internal Connection Failed");
#endif
			}
		}
	});
}

void Bot::OnPurchase(std::string szPurchaseUrl)
{
	OpenURLInDefaultBrowser(szPurchaseUrl.c_str());
}

void Bot::OnCaptchaResponse(bool bStatus, std::string szResult)
{
	if (!bStatus)
	{
		m_pClientHandler->RefreshCaptcha();
	}
	else
	{
		m_pClientHandler->SendCaptcha(szResult);
	}
}

void Bot::OnAuthenticationMessage(bool bStatus, std::string szMessage)
{
	if (bStatus == true 
		&& Drawing::GetScene() != Drawing::Scene::LOGIN)
	{
		Drawing::SetScene(Drawing::Scene::LOGIN);
	}

	m_isAuthenticationMessage = bStatus;
	m_szAuthenticationMessage = szMessage;
}

void Bot::OnRouteLoaded(std::vector<uint8_t> vecBuffer)
{
	try
	{
		JSON jRouteData = JSON::parse(vecBuffer);

		std::vector<Route> vecRoute;

		std::string szStepListAttribute = skCryptDec("steplist");

		for (size_t i = 0; i < jRouteData[szStepListAttribute.c_str()].size(); i++)
		{
			Route pRoute{};

			std::string szXAttribute = skCryptDec("x");
			std::string szYAttribute = skCryptDec("y");
			std::string szStepTypeAttribute = skCryptDec("steptype");
			std::string szPacketAttribute = skCryptDec("packet");

			pRoute.fX = jRouteData[szStepListAttribute.c_str()][i][szXAttribute.c_str()].get<float>();
			pRoute.fY = jRouteData[szStepListAttribute.c_str()][i][szYAttribute.c_str()].get<float>();
			pRoute.eStepType = (RouteStepType)jRouteData[szStepListAttribute.c_str()][i][szStepTypeAttribute.c_str()].get<int>();
			pRoute.szPacket = jRouteData[szStepListAttribute.c_str()][i][szPacketAttribute.c_str()].get<std::string>();

			vecRoute.push_back(pRoute);
		}

		std::string szNameAttribute = skCryptDec("name");
		std::string szIndexAttribute = skCryptDec("index");

		uint8_t iIndex = jRouteData[szIndexAttribute.c_str()].get<uint8_t>();

		auto pRouteData = m_pClientHandler->m_mapRouteList.find(iIndex);

		if (pRouteData != m_pClientHandler->m_mapRouteList.end())
		{
			pRouteData->second.insert(std::make_pair(jRouteData[szNameAttribute.c_str()].get<std::string>(), vecRoute));
		}
		else
		{
			m_pClientHandler->m_mapRouteList.insert(std::make_pair(iIndex, std::map<std::string, std::vector<Route>> {
				std::make_pair(jRouteData[szNameAttribute.c_str()].get<std::string>(), vecRoute)
			}));
		}
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG_LOG
		Print("%s", e.what());
#else
		DBG_UNREFERENCED_PARAMETER(e);
#endif
	}
}

void Bot::Patch(HANDLE hProcess)
{
	BYTE byPatch1[] = 
	{ 
		0x90, 
		0x90, 
		0x90, 
		0x90, 
		0x90 
	};

	WriteProcessMemory(hProcess, (LPVOID*)GetAddress(skCryptDec("KO_OFF_WIZ_HACKTOOL_NOP1")), byPatch1, sizeof(byPatch1), 0);
}

void Bot::OnConfigurationLoaded(std::string szConfiguration)
{
	if (m_pClientHandler == nullptr)
	{
#ifdef DEBUG_LOG
		Print("m_pClientHandler == nullptr");
#endif
		return;
	}

	m_pClientHandler->m_iniUserConfiguration = new Ini();

	if (szConfiguration.size() > 0)
	{
		m_pClientHandler->m_iniUserConfiguration->Load(szConfiguration);
	}

	m_pClientHandler->InitializeUserConfiguration();

	m_pClientHandler->m_iniUserConfiguration->onSaveEvent = [=]()
	{
		m_bUserSaveRequested = true;
	};

	SendRouteLoadRequest();
}

Ini* Bot::GetAppConfiguration()
{
	return m_iniAppConfiguration;
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
	HANDLE hProcess = GetClientProcessHandle();
	BYTE nRet = Memory::ReadByte(hProcess, dwAddress);
	return nRet;
}

WORD Bot::Read2Byte(DWORD dwAddress)
{
	HANDLE hProcess = GetClientProcessHandle();
	WORD nRet = Memory::ReadByte(hProcess, dwAddress);
	return nRet;
}

DWORD Bot::Read4Byte(DWORD dwAddress)
{
	HANDLE hProcess = GetClientProcessHandle();
	DWORD nRet = Memory::Read4Byte(hProcess, dwAddress);
	return nRet;
}

float Bot::ReadFloat(DWORD dwAddress)
{
	HANDLE hProcess = GetClientProcessHandle();
	float nRet = Memory::ReadFloat(hProcess, dwAddress);
	return nRet;
}

std::string Bot::ReadString(DWORD dwAddress, size_t nSize)
{
	HANDLE hProcess = GetClientProcessHandle();
	std::string nRet = Memory::ReadString(hProcess, dwAddress, nSize);
	return nRet;
}

std::vector<BYTE> Bot::ReadBytes(DWORD dwAddress, size_t nSize)
{
	std::vector<BYTE> nRet;
	HANDLE hProcess = GetClientProcessHandle();
	nRet = Memory::ReadBytes(hProcess, dwAddress, nSize);
	return nRet;
}

void Bot::WriteByte(DWORD dwAddress, BYTE byValue)
{
	HANDLE hProcess = GetClientProcessHandle();
	Memory::WriteByte(hProcess, dwAddress, byValue);
}

void Bot::Write4Byte(DWORD dwAddress, int iValue)
{
	HANDLE hProcess = GetClientProcessHandle();
	Memory::Write4Byte(hProcess, dwAddress, iValue);
}

void Bot::WriteFloat(DWORD dwAddress, float fValue)
{
	HANDLE hProcess = GetClientProcessHandle();
	Memory::WriteFloat(hProcess, dwAddress, fValue);
}

void Bot::WriteString(DWORD dwAddress, std::string strValue)
{
	HANDLE hProcess = GetClientProcessHandle();
	Memory::WriteString(hProcess, dwAddress, strValue);
}

void Bot::WriteBytes(DWORD dwAddress, std::vector<BYTE> byValue)
{
	HANDLE hProcess = GetClientProcessHandle();
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

bool Bot::ConnectInternalMailslot()
{
	std::string szMailslotName = skCryptDec("\\\\.\\mailslot\\Internal\\") + std::to_string(GetClientProcessId());

	m_hInternalMailslot = CreateFile(szMailslotName.c_str(),
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

bool Bot::SendInternalMailslot(Packet pkt)
{
	if (m_hInternalMailslot != INVALID_HANDLE_VALUE)
	{
		DWORD bytesWritten;

		if (!WriteFile(m_hInternalMailslot, pkt.contents(), pkt.size(), &bytesWritten, nullptr))
		{
#ifdef DEBUG_LOG
			Print("Failed to write to mailslot. Error code: %d", GetLastError());
#endif
			return false;
		}
	}
	else
	{
#ifdef DEBUG_LOG
		Print("m_hInternalMailslot == INVALID_HANDLE_VALUE");
#endif
		return false;
	}

	return true;
}

bool Bot::IsClientProcessLost()
{
	if (GetClientProcessId() == 0)
	{
		return true;
	}

	DWORD iExitCode = 0;
	if (GetExitCodeProcess(GetClientProcessHandle(), &iExitCode) == FALSE)
	{
		return true;
	}

	if (iExitCode != STILL_ACTIVE)
	{
		return true;
	}

	return false;
}

HANDLE Bot::GetClientProcessHandle()
{
	if (m_ClientProcessInfo.hProcess != NULL
		&& m_ClientProcessInfo.hProcess != INVALID_HANDLE_VALUE)
	{
		DWORD dwFlags;

		if (GetHandleInformation(m_ClientProcessInfo.hProcess, &dwFlags))
		{
			return m_ClientProcessInfo.hProcess;
		}
		else
		{
			m_ClientProcessInfo.hProcess = NULL;
		}
	}

	m_ClientProcessInfo.hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetClientProcessId());
	return m_ClientProcessInfo.hProcess;
}

std::wstring Bot::GetAnyOTPHardwareID()
{
	std::wstring szOTPHardwareID = L"";

	for (size_t i = 0; i < m_pHardwareInformation->Disk.size(); i++)
	{
		HardwareInformation::DiskObject& Disk{ m_pHardwareInformation->Disk.at(i) };

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
			sstream << std::hex << std::uppercase << Disk.Signature;
			std::wstring szSignature = sstream.str();

			szOTPHardwareID = szSignature.substr(0, 4) + szPNPDeviceID.substr(szPNPDeviceID.length() - 16);
		}
	}

	return szOTPHardwareID;
}

void Bot::InitializeAnyOTPService()
{
	if (FileExists(skCryptDec("C:\\Program Files\\AnyOTPSetup\\AnyOTPBiz.dll")))
	{
		m_hModuleAnyOTP = LoadLibraryExW(skCryptDec(L"C:\\Program Files\\AnyOTPSetup\\AnyOTPBiz.dll"), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	}
	else if (FileExists(skCryptDec("C:\\Program Files (x86)\\AnyOTPSetup\\AnyOTPBiz.dll")))
	{
		m_hModuleAnyOTP = LoadLibraryExW(skCryptDec(L"C:\\Program Files (x86)\\AnyOTPSetup\\AnyOTPBiz.dll"), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	}

	if (m_hModuleAnyOTP == NULL)
	{
#ifdef _DEBUG
		Print("DLL Not Loaded");
#endif
		return;
	}

#ifdef _DEBUG
	Print("AnyOTP Library Loaded");
#endif

	if (m_szOTPHardwareID.size() == 0)
	{
		m_szOTPHardwareID = GetAnyOTPHardwareID();
	}
}

std::wstring Bot::ReadAnyOTPCode(std::string szPassword, std::string szHardwareID)
{
	std::wstring szCode = L"";
	std::wstring szOTPPassword(szPassword.begin(), szPassword.end());
	std::wstring szOTPHardwareID = m_szOTPHardwareID;

	if (szHardwareID.size() > 0)
	{
		std::wstring szCustomOTPHardwareID(szHardwareID.begin(), szHardwareID.end());
		szOTPHardwareID = szCustomOTPHardwareID;
	}

	typedef int(__stdcall* GenerateOTP)(int, LPCWSTR, LPCWSTR, int*);
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
		Print("ReadProcessMemory Failed");
#endif
		return szCode;
	}

	szCode = buffer;

	return szCode;
}

void Bot::StartGame()
{
#ifdef VMPROTECT
	VMProtectBeginMutation("Bot::StartGame");
#endif

	if (!IsConnected())
		return;

	std::time_t iCurrentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	if (m_iSubscriptionEndAt < iCurrentTime)
		return;

	m_bGameStarting = true;

	new std::thread([&]()
	{
		std::vector<const char*> vecProcessNames = 
		{
			skCryptDec("KnightOnLine.exe"),
			skCryptDec("xldr_KnightOnline_NA.exe"),
			skCryptDec("xldr_KnightOnline_NA_loader_win32.exe"),
			skCryptDec("xldr_KnightOnline_GB.exe"),
			skCryptDec("xldr_KnightOnline_GB_loader_win32.exe"),
			skCryptDec("xxd-0.xem"),
		};

		KillProcessesByFileName(vecProcessNames);

		m_ClientProcessInfo = PROCESS_INFORMATION();

#ifndef DISABLE_XIGNCODE
		DWORD iLoaderExitCode = 0;
		PROCESS_INFORMATION loaderProcessInfo;

		std::ostringstream strLoaderCommandLine;
		strLoaderCommandLine << m_szClientPath + "\\" + m_szClientExe;

		if (!StartProcess(
			m_szClientPath + skCryptDec("\\XIGNCODE\\"), skCryptDec("xldr_KnightOnline_NA_loader_win32.exe"), strLoaderCommandLine.str(), loaderProcessInfo))
		{
			SendVital(VITAL_CODE_XIGN_LOADER_START_PROCESS_ERROR);
			return;
		}

		SendVital(VITAL_CODE_XIGN_LOADER_STARTED);

		if (WaitForSingleObject(loaderProcessInfo.hProcess, -1))
		{
			SendVital(VITAL_CODE_XIGN_LOADER_NOT_EXITED_NORMALLY);
			return;
		}

		if (!GetExitCodeProcess(loaderProcessInfo.hProcess, &iLoaderExitCode))
		{
			SendVital(VITAL_CODE_XIGN_LOADER_NOT_EXITED_NORMALLY);
			return;
		}

		if (iLoaderExitCode > 0)
		{
			SendVital(VITAL_CODE_XIGN_LOADER_EXIT_CODE_NOT_ZERO);
			return;
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

		if (!StartProcess(m_szClientPath, m_szClientExe, strCommandLine.str(), m_ClientProcessInfo))
		{
			SendVital(VITAL_CODE_CLIENT_START_PROCESS_ERROR);
			return;
		}

		SendVital(VITAL_CODE_CLIENT_PROCESS_STARTED);

#ifndef DISABLE_XIGNCODE
		if (!WaitForSingleObject(m_ClientProcessInfo.hProcess, 3000))
		{
			SendVital(VITAL_CODE_CLIENT_PROCESS_EXITED_UNKNOWN_REASON);
			return;
		}
#endif

		while (Read4Byte(GetAddress(skCryptDec("KO_XIGNCODE_ENTRY_POINT"))) == 0)
			continue;

		if (m_ePlatformType == PlatformType::USKO
			|| m_ePlatformType == PlatformType::KOKO
			|| m_ePlatformType == PlatformType::STKO)
		{
			NtSuspendProcess(m_ClientProcessInfo.hProcess);

			if (GetAddress(skCryptDec("KO_PATCH_ADDRESS1")) > 0)
			{
				Remap::PatchSection(
					m_ClientProcessInfo.hProcess,
					(LPVOID*)GetAddress(skCryptDec("KO_PATCH_ADDRESS1")),
					GetAddress(skCryptDec("KO_PATCH_ADDRESS1_SIZE")), PAGE_EXECUTE_READWRITE);
			}

			if (GetAddress(skCryptDec("KO_PATCH_ADDRESS2")) > 0)
			{
				Remap::PatchSection(
					m_ClientProcessInfo.hProcess,
					(LPVOID*)GetAddress(skCryptDec("KO_PATCH_ADDRESS2")),
					GetAddress(skCryptDec("KO_PATCH_ADDRESS2_SIZE")), PAGE_EXECUTE_READWRITE);
			}

			if (GetAddress(skCryptDec("KO_PATCH_ADDRESS3")) > 0)
			{
				Remap::PatchSection(
					m_ClientProcessInfo.hProcess,
					(LPVOID*)GetAddress(skCryptDec("KO_PATCH_ADDRESS3")),
					GetAddress(skCryptDec("KO_PATCH_ADDRESS3_SIZE")), PAGE_EXECUTE_READWRITE);
			}

#ifdef DISABLE_XIGNCODE
			if (m_ePlatformType == PlatformType::USKO)
			{
				BYTE byPatch2[] = { 0xE9, 0x01, 0x03, 0x00, 0x00, 0x90 };
				WriteProcessMemory(m_ClientProcessInfo.hProcess, (LPVOID*)GetAddress(skCryptDec("KO_XIGNCODE_ENTRY_POINT")), byPatch2, sizeof(byPatch2), 0);
			}
			else if (m_ePlatformType == PlatformType::KOKO)
			{
				BYTE byPatch2[] = { 0xE9, 0x50, 0x07, 0x00, 0x00, 0x90 };
				WriteProcessMemory(m_ClientProcessInfo.hProcess, (LPVOID*)GetAddress(skCryptDec("KO_XIGNCODE_ENTRY_POINT")), byPatch2, sizeof(byPatch2), 0);
			}
			else if (m_ePlatformType == PlatformType::STKO)
			{
				BYTE byPatch2[] = { 0xE9, 0x09, 0x03, 0x00, 0x00, 0x90 };
				WriteProcessMemory(m_ClientProcessInfo.hProcess, (LPVOID*)GetAddress(skCryptDec("KO_XIGNCODE_ENTRY_POINT")), byPatch2, sizeof(byPatch2), 0);
			}
#endif
		}

		NtResumeProcess(m_ClientProcessInfo.hProcess);

		m_pClientHandler = new ClientHandler(this);
		m_pClientHandler->Initialize();

		SendInjectionRequest();

		m_bGameStarting = false;
	});

#ifdef VMPROTECT
	VMProtectEnd();
#endif
}

void Bot::StopStartGameProcess()
{
	m_bGameStarting = false;

	std::vector<const char*> vecFileNames = 
	{
		skCryptDec("KnightOnLine.exe"),
		skCryptDec("xldr_KnightOnline_NA.exe"),
		skCryptDec("xldr_KnightOnline_NA_loader_win32.exe"),
		skCryptDec("xldr_KnightOnline_GB.exe"),
		skCryptDec("xldr_KnightOnline_GB_loader_win32.exe"),
		skCryptDec("xxd-0.xem")
	};

	KillProcessesByFileName(vecFileNames);

	m_ClientProcessInfo = PROCESS_INFORMATION();

	if (m_pClientHandler)
	{
		m_pClientHandler->Clear();

		delete m_pClientHandler;
		m_pClientHandler = nullptr;
	}
}

void Bot::CheckProxy(const std::string& szProxyIP, uint16_t iProxyPort, const std::string& szUsername, const std::string& szPassword)
{
	new std::thread([=]()
	{
		m_bCheckingProxy = true;

		m_bCheckingProxyResult = ::CheckProxy(szProxyIP, iProxyPort, szUsername, szPassword);

		if (m_bCheckingProxyResult)
			m_szCheckingProxyResult = skCryptDec("Proxy test baglantisi basarili");
		else
			m_szCheckingProxyResult = skCryptDec("Proxy test baglantisi basarisiz");

		std::this_thread::sleep_for(std::chrono::milliseconds(5000));

		m_szCheckingProxyResult.clear();

		m_bCheckingProxy = false;
	});
}