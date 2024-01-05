#pragma once

#include "Service.h"
#include "Table.h"

class ClientHandler;
class Client;
class Bot : public Service
{
public:
	Bot(HardwareInformation* pHardwareInformation);
	~Bot();

	ClientHandler* GetClientHandler();
	Service* GetService() { return static_cast<Service*>(this); }

public:
	void Initialize();
	void InitializeStaticData();

	void Process();
	void Release();

	DWORD GetAddress(std::string szAddressName);

	Ini* GetAppConfiguration();

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
	void OnPong(uint32_t iSubscriptionEndAt, int32_t iCredit);
	void OnAuthenticated(uint8_t iStatus);
	void OnUpdate();
	void OnUpdateDownloaded(bool bStatus);
	void OnLoaded(std::string szPointerData);
	void OnSaveToken(std::string szToken, uint32_t iSubscriptionEndAt, int32_t iCredit);
	void OnInjection(std::vector<uint8_t> vecBuffer);
	void OnPurchase(std::string szPurchaseUrl);
	void OnConfigurationLoaded(std::string szConfiguration);
	void OnCaptchaResponse(bool bStatus, std::string szResult);
	void OnRouteLoaded(std::vector<uint8_t> vecBuffer);
	void OnAuthenticationMessage(bool bStatus, std::string szMessage);

private:
	ClientHandler* m_pClientHandler;

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

	PlatformType GetPlatformType() { return m_ePlatformType; }

public:
	std::string m_szClientPath;
	std::string m_szClientExe;

	bool m_bTableLoading;
	bool m_bTableLoaded;

public:
	std::unordered_map<std::string, DWORD> m_mapAddress;

public:
	HANDLE m_hInternalMailslot;
	bool m_bInternalMailslotWorking;

	bool ConnectInternalMailslot();
	bool SendInternalMailslot(Packet pkt);

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
	void Patch(HANDLE hProcess);

public:
	DWORD GetClientProcessId() { return m_ClientProcessInfo.dwProcessId; };
	HANDLE GetClientProcessHandle();
	bool IsClientProcessLost();

private:
	PROCESS_INFORMATION m_ClientProcessInfo;

public:
	std::wstring m_szOTPHardwareID;
	HMODULE m_hModuleAnyOTP;
	std::wstring GetAnyOTPHardwareID();
private:

	void InitializeAnyOTPService();

public:
	std::wstring ReadAnyOTPCode(std::string szOTPPassword, std::string szHardwareID);

private:
	bool m_bAuthenticated;

public:
	bool IsAuthenticated() { return m_bAuthenticated; };

public:
	std::chrono::time_point<std::chrono::system_clock> m_startTime;

public:
	std::map<uint32_t, __TABLE_ITEM>* m_mapItemTable;

public:
	void StartGame();
	void StopStartGameProcess();

public:
	bool m_bGameStarting;

public:
	bool IsGameStarting() { return m_bGameStarting; };

public:
	Ini* m_iniAppConfiguration;

public:
	bool m_bAutoLogin;
	bool m_bAutoStart;

	std::string m_szID;
	std::string m_szPassword;

	int m_iServerID;
	int m_iChannelID;
	int m_iSlotID;

	std::string m_szAnyOTPID;
	std::string m_szAnyOTPPassword;

	int32_t m_iCredit;
	uint32_t m_iSubscriptionEndAt;
	float m_fLastPongTime;

	bool m_isAuthenticationMessage;
	std::string m_szAuthenticationMessage;

protected:
	std::string m_szToken;
	bool m_bUserSaveRequested;

public:
	PlatformType m_ePlatformType;

private:
	HardwareInformation* m_pHardwareInformation;

public:
	std::string m_szProxyIP;
	int32_t m_iProxyPort;
	std::string m_szProxyUsername;
	std::string m_szProxyPassword;
	bool m_bConnectWithProxy;

public:
	bool m_bCheckingProxyResult;
	std::string m_szCheckingProxyResult;
	bool m_bCheckingProxy;
	void CheckProxy(const std::string& szProxyIP, uint16_t iProxyPort, const std::string& szUsername, const std::string& szPassword);
};

