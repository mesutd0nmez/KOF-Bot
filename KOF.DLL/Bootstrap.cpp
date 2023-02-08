#include "pch.h"
#include "Bootstrap.h"
#include "Client.h"

Bootstrap::Bootstrap()
{
	printf("Bootstrap::Initialize\n");

	InitializeStaticData();
}

Bootstrap::~Bootstrap()
{
	printf("Bootstrap::Destroy\n");
}

void Bootstrap::InitializeStaticData()
{
	printf("Bootstrap::InitializeStaticData\n");

	InitializeAddress();
	InitializeTable();
}

void Bootstrap::InitializeTable()
{
	printf("Bootstrap::InitializeTable: Starting Table Load Thread\n");

	new std::thread([]()
	{
		m_bTableLoaded = false;

		printf("Bootstrap::InitializeTable: Waiting state Game\n");

		while(m_Client.GetState() != State::GAME)
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		printf("Bootstrap::InitializeTable: Started\n");

		/*
		* Load Skill Table
		*/
		m_pTbl_Skill.Load(".\\Data\\skill_magic_main_us.tbl");

		for (const auto& x : m_pTbl_Skill.GetData())
		{
			printf("%d - %s - %s\n", x.second.iID, x.second.szName.c_str(), x.second.szEngName.c_str());
		}
		m_pTbl_Skill_Extension1.Load(".\\Data\\skill_magic_1.tbl");
		m_pTbl_Skill_Extension2.Load(".\\Data\\skill_magic_2.tbl");
		m_pTbl_Skill_Extension3.Load(".\\Data\\skill_magic_3.tbl");
		m_pTbl_Skill_Extension4.Load(".\\Data\\skill_magic_4.tbl");
		m_pTbl_Skill_Extension5.Load(".\\Data\\skill_magic_5.tbl");
		m_pTbl_Skill_Extension6.Load(".\\Data\\skill_magic_6.tbl");
		m_pTbl_Skill_Extension7.Load(".\\Data\\skill_magic_7.tbl");
		m_pTbl_Skill_Extension8.Load(".\\Data\\skill_magic_8.tbl");
		m_pTbl_Skill_Extension9.Load(".\\Data\\skill_magic_9.tbl");

		/*
		* Load Custom Skills
		*/
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

		m_pTbl_Item.Load(".\\Data\\item_org_nc.tbl");
		m_pTbl_Npc.Load(".\\Data\\npc_us.tbl");
		m_pTbl_Mob.Load(".\\Data\\mob_us.tbl");

		m_bTableLoaded = true;

		printf("Bootstrap::InitializeTable: Ended Table Load Thread\n");
	});
}

void Bootstrap::InitializeAddress()
{
	printf("Bootstrap::InitializeAddress\n");

	m_AddressConfig = new Ini(ADDRESS_INI_FILE);

	Ini *ini = m_AddressConfig;

	ini->GetString("Address", "KO_PTR_PUSH_PHASE", "0x6A3750");
	ini->GetString("Address", "KO_PTR_INTRO", "0xF7F370");
	ini->GetString("Address", "KO_PTR_LOGIN", "0xF7F34C");
	ini->GetString("Address", "KO_PTR_LOGIN1", "0x6B1940");
	ini->GetString("Address", "KO_PTR_LOGIN2", "0x6AF480");
	ini->GetString("Address", "KO_PTR_LOGIN_DC", "0x6AF8D0");
	ini->GetString("Address", "KO_PTR_SERVER_SELECT", "0x6B2230");
	ini->GetString("Address", "KO_PTR_CHARACTER_SELECT", "0xF7F358");
	ini->GetString("Address", "KO_PTR_CHARACTER_SELECT_SKIP", "0x6B8120");
	ini->GetString("Address", "KO_PTR_CHARACTER_SELECT_ENTER", "0x6B3610");
	ini->GetString("Address", "KO_PTR_CHARACTER_SELECT_RIGHT", "0x6BB9D0");
	ini->GetString("Address", "KO_PTR_CHARACTER_SELECT_LEFT", "0x6BB920");
	ini->GetString("Address", "KO_PTR_DLG", "0xF7F374");
	ini->GetString("Address", "KO_PTR_CHR", "0xF7F2FC");
	ini->GetString("Address", "KO_PTR_PKT", "0xF7F334");
	ini->GetString("Address", "KO_SND_FNC", "0x5EE3D0");
	ini->GetString("Address", "KO_PTR_FLDB", "0xF7F2F8");
	ini->GetString("Address", "KO_PTR_ROUTE_START_CALL", "0x7D8440");

	ini->GetString("Address", "KO_OFF_DISCONNECT", "0xA0");
	ini->GetString("Address", "KO_OFF_LOGIN_SERVER_INDEX", "0x410");
	ini->GetString("Address", "KO_OFF_UI_LOGIN_INTRO", "0x2C");
	ini->GetString("Address", "KO_OFF_UI_LOGIN_INTRO_ID", "0x10C");
	ini->GetString("Address", "KO_OFF_UI_LOGIN_INTRO_ID_INPUT", "0x140");
	ini->GetString("Address", "KO_OFF_UI_LOGIN_INTRO_ID_INPUT_LENGTH", "0x150");
	ini->GetString("Address", "KO_OFF_UI_LOGIN_INTRO_PW", "0x110");
	ini->GetString("Address", "KO_OFF_UI_LOGIN_INTRO_PW_INPUT", "0x128");
	ini->GetString("Address", "KO_OFF_UI_LOGIN_INTRO_PW_INPUT_LENGTH", "0x138");
	ini->GetString("Address", "KO_OFF_UI_CHARACTER_SELECT_TIMER", "0x2C");
	ini->GetString("Address", "KO_OFF_ID", "0x648");
	ini->GetString("Address", "KO_OFF_MAX_HP", "0x67C");
	ini->GetString("Address", "KO_OFF_MAX_MP", "0xB24");
	ini->GetString("Address", "KO_OFF_HP", "0x680");
	ini->GetString("Address", "KO_OFF_MP", "0xB28");
	ini->GetString("Address", "KO_OFF_NAME_LEN", "0x65C");
	ini->GetString("Address", "KO_OFF_NAME", "0x64C");
	ini->GetString("Address", "KO_OFF_ZONE", "0xBD0");
	ini->GetString("Address", "KO_OFF_GOLD", "0xB38");
	ini->GetString("Address", "KO_OFF_LEVEL", "0x678");
	ini->GetString("Address", "KO_OFF_NATION", "0x684");
	ini->GetString("Address", "KO_OFF_CLASS", "0x674");
	ini->GetString("Address", "KO_OFF_MAXEXP", "0xB40");
	ini->GetString("Address", "KO_OFF_EXP", "0xB48");
	ini->GetString("Address", "KO_OFF_GOX", "0xF3C");
	ini->GetString("Address", "KO_OFF_GOY", "0xF44");
	ini->GetString("Address", "KO_OFF_X", "0xD0");
	ini->GetString("Address", "KO_OFF_Y", "0xD8");
	ini->GetString("Address", "KO_OFF_Z", "0xD4");
	ini->GetString("Address", "KO_OFF_SKILL_TREE_BASE", "0x1DC");
	ini->GetString("Address", "KO_OFF_SKILL_TREE_POINT_BASE", "0x158");

	auto configMap = ini->GetConfigMap();

	auto it = configMap->find("Address");

	for (const auto& address : it->second)
		m_Client.SetAddress(address.first, std::strtoul(address.second.c_str(), NULL, 16));
}

void Bootstrap::Start()
{
	printf("Bootstrap::Start\n");

	m_Client.Start();
}

void Bootstrap::Stop()
{
	printf("Bootstrap::Stop\n");

	m_Client.Stop();
}