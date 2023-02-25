#include "pch.h"
#include "Bot.h"
#include "ClientHandler.h"
#include "UI.h"

Bot::Bot()
{
	m_ClientHandler = nullptr;
}

Bot::~Bot()
{
	m_ClientHandler = nullptr;
}

void Bot::Initialize()
{
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

	m_pTbl_Skill.Load(".\\Data\\skill_magic_main_us.tbl");
	m_pTbl_Skill_Extension1.Load(".\\Data\\skill_magic_1.tbl");
	m_pTbl_Skill_Extension2.Load(".\\Data\\skill_magic_2.tbl");
	m_pTbl_Skill_Extension3.Load(".\\Data\\skill_magic_3.tbl");
	m_pTbl_Skill_Extension4.Load(".\\Data\\skill_magic_4.tbl");
	m_pTbl_Skill_Extension5.Load(".\\Data\\skill_magic_5.tbl");
	m_pTbl_Skill_Extension6.Load(".\\Data\\skill_magic_6.tbl");
	m_pTbl_Skill_Extension7.Load(".\\Data\\skill_magic_7.tbl");
	m_pTbl_Skill_Extension8.Load(".\\Data\\skill_magic_8.tbl");
	m_pTbl_Skill_Extension9.Load(".\\Data\\skill_magic_9.tbl");

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

	m_pTbl_Item.Load(".\\Data\\item_org_nc.tbl");

#ifdef DEBUG
	printf("InitializeStaticData: Loaded %d items\n", m_pTbl_Item.GetDataSize());
#endif

#ifdef DEBUG
	printf("InitializeStaticData: Loading npc data\n");
#endif

	m_pTbl_Npc.Load(".\\Data\\npc_us.tbl");

#ifdef DEBUG
	printf("InitializeStaticData: Loaded %d npcs\n", m_pTbl_Npc.GetDataSize());
#endif

#ifdef DEBUG
	printf("InitializeStaticData: Loading mob data\n");
#endif

	m_pTbl_Mob.Load(".\\Data\\mob_us.tbl");

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
	printf("Bot: Pong!\n");

	if(GetClientHandler()->GetName().size() > 0)
		SendSaveUserConfiguration(1, GetClientHandler()->GetName());
}

void Bot::OnAuthenticated()
{
	InitializeStaticData();
}

void Bot::OnLoaded()
{
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
