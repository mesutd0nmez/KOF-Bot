#pragma once

#include "Structures.h"
#include "Table.h"

class TableHandler
{
public:
	static void Start();
	static void Stop();

	static void MainProcess();

	static Table<__TABLE_UPC_SKILL> GetSkillTable() { return m_pTbl_Skill; };

	static bool IsTableLoaded() { return m_bTableLoaded; };

protected:
	inline static bool m_bWorking;
	inline static bool m_bTableLoaded;

	inline static Table<__TABLE_TEXTS> m_pTbl_Texts;
	inline static Table<__TABLE_UPC_SKILL> m_pTbl_Skill;
	inline static Table<__TABLE_ITEM_BASIC> m_pTbl_Items_Basic;
};

