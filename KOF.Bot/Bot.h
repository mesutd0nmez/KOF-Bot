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
	Service* GetService() { return static_cast<Service*>(this); }

public:
	void Initialize();
	void InitializeStaticData();

	void Process();

	void Close();
	void Release();

	DWORD GetAddress(std::string szAddressName);

	Ini* GetAppConfiguration();
	Ini* GetUserConfiguration();

	bool GetSkillTable(std::map<uint32_t, __TABLE_UPC_SKILL>** mapDataOut);
	bool GetSkillExtension2Table(std::map<uint32_t, __TABLE_UPC_SKILL_EXTENSION2>** mapDataOut);
	bool GetSkillExtension3Table(std::map<uint32_t, __TABLE_UPC_SKILL_EXTENSION3>** mapDataOut);
	bool GetSkillExtension4Table(std::map<uint32_t, __TABLE_UPC_SKILL_EXTENSION4>** mapDataOut);

	bool GetItemTable(std::map<uint32_t, __TABLE_ITEM>** mapDataOut);
	bool GetItemData(uint32_t iItemID, __TABLE_ITEM*& pItemData);
	bool GetItemExtensionTable(uint8_t iExtensionID, std::map<uint32_t, __TABLE_ITEM_EXTENSION>** mapDataOut);
	bool GetItemExtensionData(uint32_t iItemID, uint8_t iExtensionID, __TABLE_ITEM_EXTENSION*& pOutItemExtensionData);
	bool GetNpcTable(std::map<uint32_t, __TABLE_NPC>** mapDataOut);

	bool GetMobTable(std::map<uint32_t, __TABLE_MOB>** mapDataOut);

	bool GetItemSellTable(std::map<uint32_t, __TABLE_ITEM_SELL>** mapDataOut);
	bool GetShopItemTable(int32_t iSellingGroup, std::vector<SShopItem>& vecShopWindow);

	bool GetDisguiseRingTable(std::map<uint32_t, __TABLE_DISGUISE_RING>** mapDataOut);

private:
	void OnConnected();
	void OnReady();
	void OnPong();
	void OnAuthenticated();
	void OnUpdate();
	void OnUpdateDownloaded(bool bStatus);
	void OnLoaded();
	void OnConfigurationLoaded();
	void OnCaptchaResponse(bool bStatus, std::string szResult);

private:
	ClientHandler* m_ClientHandler;

	Table<__TABLE_UPC_SKILL>* m_pTbl_Skill;
	Table<__TABLE_UPC_SKILL_EXTENSION2>* m_pTbl_Skill_Extension2;
	Table<__TABLE_UPC_SKILL_EXTENSION3>* m_pTbl_Skill_Extension3;
	Table<__TABLE_UPC_SKILL_EXTENSION4>* m_pTbl_Skill_Extension4;
	Table<__TABLE_ITEM>* m_pTbl_Item;
	Table<__TABLE_ITEM_EXTENSION>* m_pTbl_Item_Extension[45];
	Table<__TABLE_NPC>* m_pTbl_Npc;
	Table<__TABLE_MOB>* m_pTbl_Mob;

	Table<__TABLE_ITEM_SELL>* m_pTbl_ItemSell;
	Table<__TABLE_DISGUISE_RING>* m_pTbl_Disguise_Ring;

	std::string m_szAccountListFilePath;

public:
	bool IsClosed() { return m_bClosed; }
	bool IsInjectedProcessLost();
	bool IsTableLoaded() { return m_bTableLoaded; }

	PlatformType GetPlatformType() { return m_ePlatformType; }

public:
	bool m_bClosed;
	std::string m_szClientPath;
	std::string m_szClientExe;

	bool m_bTableLoaded;

public:
	void BuildAdress();

private:
	std::unordered_map<std::string, DWORD> m_mapAddress;

private:
	std::chrono::milliseconds m_msLastInitializeHandle;

public:
	HANDLE m_hInternalMailslot;
	bool m_bInternalMailslotWorking;

	bool ConnectInternalMailslot();
	void SendInternalMailslot(Packet pkt);

public:
	BYTE ReadByte(DWORD dwAddress);
	WORD Read2Byte(DWORD dwAddress);
	DWORD Read4Byte(DWORD dwAddress);
	float ReadFloat(DWORD dwAddress);
	std::string ReadString(DWORD dwAddress, size_t nSize);
	std::vector<BYTE> ReadBytes(DWORD dwAddress, size_t nSize);
	void WriteByte(DWORD dwAddress, BYTE byValue);
	void Write4Byte(DWORD dwAddress, int iValue);
	void WriteFloat(DWORD dwAddress, float fValue);
	void WriteString(DWORD dwAddress, std::string strValue);
	void WriteBytes(DWORD dwAddress, std::vector<BYTE> byValue);
	bool ExecuteRemoteCode(HANDLE hProcess, BYTE* codes, size_t psize);
	bool ExecuteRemoteCode(HANDLE hProcess, LPVOID pAddress);

public:
	static float TimeGet();

public:
	void Patch(HANDLE hProcess);

public:
	DWORD GetInjectedProcessId() { return m_InjectedProcessInfo.dwProcessId; };
	HANDLE GetInjectedProcessHandle();

private:
	PROCESS_INFORMATION m_InjectedProcessInfo;

public:
	HMODULE m_hModuleAnyOTP;
	std::wstring GetAnyOTPHardwareID();
private:

	void InitializeAnyOTPService();
	typedef int(__stdcall* GenerateOTP)(int, LPCWSTR, LPCWSTR, int*);

public:
	std::wstring ReadAnyOTPCode(std::string szOTPPassword, std::string szHardwareID);

private:
	bool m_bAuthenticated;

public:
	bool IsAuthenticated() { return m_bAuthenticated; };

private:
	bool m_bUpdate;

public:
	bool IsUpdating() { return m_bUpdate; };

public:
	std::chrono::time_point<std::chrono::system_clock> m_startTime;

public:
	std::map<uint32_t, __TABLE_ITEM>* m_mapItemTable;

public:
	void StartGame();
	void StopStartGameProcess();

public:
	bool m_bStarted;
	bool m_bForceClosed;
public:
	bool IsStarted() { return m_bStarted; };
	bool IsForceClosed() { return m_bForceClosed; };
};

