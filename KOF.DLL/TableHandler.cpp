#include "pch.h"
#include "Bot.h"
#include "TableHandler.h"

void TableHandler::Start()
{
	new std::thread([]() { MainProcess(); });
}

void TableHandler::Stop()
{
	m_bWorking = false;
}

void TableHandler::MainProcess()
{
	m_bWorking = true;
	m_bTableLoaded = false;

	//m_pTbl_Texts.Load(".\\Data\\texts_nc.tbl");
	//m_pTbl_Skill.Load(".\\Data\\skill_magic_main_us.tbl");
	//m_pTbl_Items_Basic.Load(".\\Data\\item_org_nc.tbl");

	m_bTableLoaded = true;
	m_bWorking = false;
}