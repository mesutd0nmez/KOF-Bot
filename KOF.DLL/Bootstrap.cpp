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

	printf("Bootstrap::InitializeStaticData: Starting Table Load Thread\n");

	new std::thread([]() 
	{ 
		m_bTableLoaded = false;

		m_pTbl_Texts.Load(".\\Data\\texts_nc.tbl");
		m_pTbl_Skill.Load(".\\Data\\skill_magic_main_us.tbl");
		m_pTbl_Items_Basic.Load(".\\Data\\item_org_nc.tbl");

		m_bTableLoaded = true;

		printf("Bootstrap::InitializeStaticData: Ended Table Load Thread\n");
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