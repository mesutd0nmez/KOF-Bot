#include "pch.h"
#include "Bot.h"
#include "Define.h"
#include "Client.h"

Bot::Bot()
{
	InitializeStaticData();
}

Bot::~Bot()
{
}

void Bot::InitializeStaticData()
{
	InitializeAddress();
}

void Bot::InitializeAddress()
{
	m_AddressConfig = new Ini(ADDRESS_INI_FILE);

	Ini *ini = m_AddressConfig;

	ini->GetString("Address", "KO_PTR_PUSH_PHASE_CALL", "0x6A29C0");
	ini->GetString("Address", "KO_PTR_INTRO", "0xF7E365");
	ini->GetString("Address", "KO_PTR_LOGIN", "0xF7E344");
	ini->GetString("Address", "KO_PTR_LOGIN_CALL1", "0x6B0BB0");
	ini->GetString("Address", "KO_PTR_LOGIN_CALL2", "0x6AE6E0");
	ini->GetString("Address", "KO_PTR_LOGIN_SERVER_CALL", "0x6B14A0");
	ini->GetString("Address", "KO_PTR_CHARACTER_SELECT", "0xF7E350");
	ini->GetString("Address", "KO_PTR_CHARACTER_SELECT_CALL", "0x6B2890");
	ini->GetString("Address", "KO_PTR_CHARACTER_SELECT_ENTER_CALL", "0x6B73A0");
	ini->GetString("Address", "KO_PTR_DLG", "0xF7E36C");
	ini->GetString("Address", "KO_PTR_CHR", "0xF7E2F4");
	ini->GetString("Address", "KO_PTR_PKT", "0xF7E32C");
	ini->GetString("Address", "KO_SND_FNC", "0x5ED620");
	ini->GetString("Address", "KO_PTR_PLAYER_OTHER", "0xF7E2F0");
	ini->GetString("Address", "KO_PTR_ROUTE_START_CALL", "0x7D8440");

	ini->GetString("Address", "KO_OFF_DISCONNECT", "0xA0");
	ini->GetString("Address", "KO_OFF_LOGIN_SERVER_INDEX", "0x410");
	ini->GetString("Address", "KO_OFF_LOGIN_SERVER_LIST_READY", "0x268");
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

void Bot::Start()
{
	m_Client.Start();

	m_ConfigHandler.Start();
	m_TableHandler.Start();
	m_AttackHandler.Start();
	m_HookHandler.Start();
	//m_LoginHandler.Start();
	m_ProtectionHandler.Start();
}

void Bot::Stop()
{
	m_Client.Stop();

	m_ConfigHandler.Stop();
	m_TableHandler.Stop();
	m_AttackHandler.Stop();
	m_HookHandler.Stop();
	//m_LoginHandler.Stop();
	m_ProtectionHandler.Stop();
}