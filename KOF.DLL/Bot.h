#pragma once

#include "Service.h"
#include "Table.h"

class ClientHandler;
class Client;
class Bot : public Service
{
public:
	Bot();
	~Bot();

	ClientHandler* GetClientHandler();

public:
	void Initialize();
	void InitializeStaticData();

	void Process();

	DWORD GetAddress(std::string szAddressName);
	Ini* GetConfiguration();

	Table<__TABLE_UPC_SKILL> GetSkillTable() { return m_pTbl_Skill; };
	Table<__TABLE_UPC_SKILL_EXTENSION1> GetSkillExtension1Table() { return m_pTbl_Skill_Extension1; };
	Table<__TABLE_UPC_SKILL_EXTENSION2> GetSkillExtension2Table() { return m_pTbl_Skill_Extension2; };
	Table<__TABLE_UPC_SKILL_EXTENSION3> GetSkillExtension3Table() { return m_pTbl_Skill_Extension3; };
	Table<__TABLE_UPC_SKILL_EXTENSION4> GetSkillExtension4Table() { return m_pTbl_Skill_Extension4; };
	Table<__TABLE_UPC_SKILL_EXTENSION5> GetSkillExtension5Table() { return m_pTbl_Skill_Extension5; };
	Table<__TABLE_UPC_SKILL_EXTENSION6> GetSkillExtension6Table() { return m_pTbl_Skill_Extension6; };
	Table<__TABLE_UPC_SKILL_EXTENSION7> GetSkillExtension7Table() { return m_pTbl_Skill_Extension7; };
	Table<__TABLE_UPC_SKILL_EXTENSION8> GetSkillExtension8Table() { return m_pTbl_Skill_Extension8; };
	Table<__TABLE_UPC_SKILL_EXTENSION9> GetSkillExtension9Table() { return m_pTbl_Skill_Extension9; };

	Table<__TABLE_ITEM> GetItemTable() { return m_pTbl_Item; };

	Table<__TABLE_NPC> GetNpcTable() { return m_pTbl_Npc; };
	Table<__TABLE_MOB_USKO> GetMobTable() { return m_pTbl_Mob; };

private:
	void OnReady();
	void OnPong();
	void OnAuthenticated();
	void OnLoaded();
	void OnConfigurationLoaded();

private:
	ClientHandler* m_ClientHandler;

	Table<__TABLE_UPC_SKILL> m_pTbl_Skill;
	Table<__TABLE_UPC_SKILL_EXTENSION1> m_pTbl_Skill_Extension1;
	Table<__TABLE_UPC_SKILL_EXTENSION2> m_pTbl_Skill_Extension2;
	Table<__TABLE_UPC_SKILL_EXTENSION3> m_pTbl_Skill_Extension3;
	Table<__TABLE_UPC_SKILL_EXTENSION4> m_pTbl_Skill_Extension4;
	Table<__TABLE_UPC_SKILL_EXTENSION5> m_pTbl_Skill_Extension5;
	Table<__TABLE_UPC_SKILL_EXTENSION6> m_pTbl_Skill_Extension6;
	Table<__TABLE_UPC_SKILL_EXTENSION7> m_pTbl_Skill_Extension7;
	Table<__TABLE_UPC_SKILL_EXTENSION8> m_pTbl_Skill_Extension8;
	Table<__TABLE_UPC_SKILL_EXTENSION9> m_pTbl_Skill_Extension9;
	Table<__TABLE_ITEM> m_pTbl_Item;
	Table<__TABLE_NPC> m_pTbl_Npc;
	Table<__TABLE_MOB_USKO> m_pTbl_Mob;

public:
	HANDLE GetInjectedProcess() { return m_hInjectedProcess; };
	DWORD GetInjectedProcessId() { return m_dwInjectedProcessID; };

	BYTE ReadByte(DWORD dwAddress);
	DWORD Read4Byte(DWORD dwAddress);
	float ReadFloat(DWORD dwAddress);
	std::string ReadString(DWORD dwAddress, size_t nSize);
	std::vector<BYTE> ReadBytes(DWORD dwAddress, size_t nSize);
	void WriteByte(DWORD dwAddress, DWORD dwValue);
	void Write4Byte(DWORD dwAddress, DWORD dwValue);
	void WriteFloat(DWORD dwAddress, float fValue);
	void WriteString(DWORD dwAddress, std::string strValue);
	void WriteBytes(DWORD dwAddress, std::vector<BYTE> byValue);

	void ExecuteRemoteCode(BYTE* codes, size_t psize);

	bool IsInjectedProcessLost();

private:
	HANDLE m_hInjectedProcess;
	DWORD m_dwInjectedProcessID;
};

