#pragma once
#include "pch.h"
#include "Ini.h"
#include "AttackHandler.h"
#include "ProtectionHandler.h"
#include "Client.h"

class Bootstrap
{
public:
	Bootstrap();
	virtual ~Bootstrap();

	static void Start();
	static void Stop();

	static Table<__TABLE_UPC_SKILL> GetSkillTable() { return m_pTbl_Skill; };

	static bool IsTableLoaded() { return m_bTableLoaded; };

private:
	static void InitializeStaticData();
	static void InitializeAddress();

protected:
	inline static Client m_Client;

	inline static bool m_bTableLoaded;

	inline static Table<__TABLE_TEXTS> m_pTbl_Texts;
	inline static Table<__TABLE_UPC_SKILL> m_pTbl_Skill;
	inline static Table<__TABLE_ITEM_BASIC> m_pTbl_Items_Basic;

	inline static Ini* m_AddressConfig;
};

