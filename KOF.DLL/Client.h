#pragma once
#include "pch.h"
#include "Packet.h"
#include "Structures.h"
#include "Table.h"
#include "Define.h"
#include "Ini.h"
#include "Vector3.h"

typedef struct SInventory
{
	int32_t				iPos;
	uint32_t			iItemID;
	uint16_t			iDurability;
	uint16_t			iCount;
	uint8_t				iFlag;
	int16_t				iRentalTime;
	uint32_t			iSerial;
	uint32_t			iExpirationTime;
} TInventory;

typedef struct SPlayer
{
	int32_t			iID;
	std::string		szName;
	float			fX;
	float			fY;
	float			fZ;
	e_Nation		eNation;
	e_Race			eRace;
	e_Class			eClass;
	uint8_t			iLevel;
	int32_t			iHPMax;
	int32_t			iHP;
	int32_t			iMSPMax;
	int32_t			iMSP;
	int32_t			iAuthority;
	uint16_t		iBonusPointRemain;

	uint8_t			iFace;
	int32_t			iHair;

	uint8_t			iRank;
	uint8_t			iTitle;

	uint8_t			iUnknown1;
	uint8_t			iUnknown2;

	uint8_t			iCity;
	int16_t			iKnightsID;
	std::string		szKnights;
	uint8_t			iKnightsGrade;
	uint8_t			iKnightsRank;

	uint32_t		iGold;
	uint64_t		iExpNext;
	uint64_t		iExp;
	uint32_t		iRealmPoint;
	uint32_t		iRealmPointMonthly;
	e_KnightsDuty	eKnightsDuty;
	uint32_t		iWeightMax;
	uint32_t		iWeight;

	uint8_t			iStrength;
	uint8_t			iStrength_Delta;
	uint8_t			iStamina;
	uint8_t			iStamina_Delta;
	uint8_t			iDexterity;
	uint8_t			iDexterity_Delta;
	uint8_t			iIntelligence;
	uint8_t			iIntelligence_Delta;
	uint8_t 		iMagicAttak;
	uint8_t 		iMagicAttak_Delta;

	int16_t 		iAttack;
	int16_t 		iGuard;

	uint8_t 		iRegistFire;
	uint8_t 		iRegistCold;
	uint8_t 		iRegistLight;
	uint8_t 		iRegistMagic;
	uint8_t 		iRegistCurse;
	uint8_t 		iRegistPoison;

	uint8_t			iSkillInfo[9];

	e_StateAction	eState;

	int16_t			iMoveSpeed;
	uint8_t			iMoveType;

	TInventory		tInventory[INVENTORY_TOTAL];

	bool			bBlinking;
	float			fRotation;
} TPlayer;

typedef struct SNpc
{
	int32_t			iID;
	uint16_t		iProtoID;
	uint8_t			iMonsterOrNpc;
	uint16_t		iPictureId;
	uint32_t		iUnknown1;
	uint8_t			iFamilyType;
	uint32_t		iSellingGroup;
	uint16_t		iModelsize;
	uint32_t		iWeapon1;
	uint32_t		iWeapon2;

	std::string		szPetOwnerName;
	std::string		szPetName;

	uint8_t			iModelGroup;
	uint8_t			iLevel;

	float			fX;
	float			fY;
	float			fZ;

	uint32_t		iStatus;

	uint8_t			iUnknown2;
	uint32_t		iUnknown3;
	float			fRotation;

	e_StateAction	eState;

	int16_t			iMoveSpeed;
	uint8_t			iMoveType;

	int32_t			iHPMax;
	int32_t			iHP;
} TNpc;

typedef struct SLoot
{
	int32_t			iNpcID;
	uint32_t		iBundleID;
	uint8_t			iItemCount;
	std::chrono::milliseconds	msDropTime;
	bool			iRequestedOpen;
} TLoot;

class Client
{
public:
	Client();
	virtual ~Client();

	static void Start();
	static void StartHandler();
	static void Stop();
	static void StopHandler();
	static bool IsWorking() { return m_bWorking; }
	static void MainProcess();
	static void HookProcess();
	static void BootstrapProcess();
	static void GameProcess();

	static State GetState() { return m_byState; }
	static void SetState(State eState) { m_byState = eState; }

	static TNpc InitializeNpc(Packet& pkt);
	static TPlayer InitializePlayer(Packet& pkt);

	static void RecvProcess(BYTE* byBuffer, DWORD dwLength);
	static void SendProcess(BYTE* byBuffer, DWORD dwLength);

public:
	static DWORD GetRecvAddress();
	static int32_t GetID();
	static std::string GetName();
	static int16_t GetHp();
	static int16_t GetMaxHp();
	static int16_t GetMp();
	static int16_t GetMaxMp();
	static uint8_t GetZone();
	static uint32_t GetGold();
	static uint8_t GetLevel();
	static e_Nation GetNation();
	static e_Class GetClass();
	static uint64_t GetExp();
	static uint64_t GetMaxExp();
	static uint8_t GetAuthority();
	static void SetAuthority(uint8_t iAuthority);

	static uint8_t GetSkillPoint(int32_t Slot);

	static void SetPosition(Vector3 v3Position);
	static void SetMovePosition(Vector3 v3MovePosition);

	static float GetGoX();
	static float GetGoY();
	static float GetGoZ();

	static float GetX();
	static float GetZ();
	static float GetY();

	static bool IsIntroPhase();
	static bool IsLoginPhase();
	static bool IsDisconnect();

	static void PushPhase(DWORD address);
	static void SetLoginInformation(std::string strAccountId, std::string strAccountPassword);
	static void ConnectLoginServer(bool bDisconnect = false);
	static void ConnectGameServer(BYTE byServerId);
	static void SelectCharacterSkip();
	static void SelectCharacter(BYTE byCharacterIndex);

	static void RouteStart(Vector3 vec3MovePosition);
	static void SetAddress(std::string strAddressName, DWORD dwAddress);
	static DWORD GetAddress(std::string strAddressName);

	static void HookRecvAddress();
	static void HookSendAddress();

	static DWORD GetRecvHookAddress();
	static DWORD GetSendHookAddress();

	static void LoadUserConfig(std::string strCharacterName);
	static Ini* GetUserConfig(std::string strCharacterName);
	static bool IsUserConfigLoaded(std::string strCharacterName) { return m_mapUserConfig.find(strCharacterName) != m_mapUserConfig.end(); };

	static std::vector<TNpc> GetNpcList() { return m_vecNpc; }
	static std::vector<TPlayer> GetPlayerList() { return m_vecPlayer; }

	static float GetDistance(Vector3 v3Position);
	static float GetDistance(Vector3 v3SourcePosition, Vector3 v3TargetPosition);
	static float GetDistance(float fX, float fY);
	static float GetDistance(float fX1, float fY1, float fX2, float fY2);

	static int32_t GetInventoryItemCount(uint32_t iItemID);
	static TInventory* GetInventoryItem(uint32_t iItemID);
	static TInventory* GetInventoryItemSlot(uint8_t iSlotPosition);

	static void UseSkill(TABLE_UPC_SKILL pSkillData, int32_t iTargetID);

	static void SendPacket(Packet byBuffer);
	static void SendTownPacket();
	static void SendBasicAttackPacket(int32_t iTargetID, float fInterval = 1.0f, float fDistance = 2.0f);

	static void SendStartSkillCastingAtTargetPacket(TABLE_UPC_SKILL pSkillData, int32_t iTargetID);
	static void SendStartSkillCastingAtPosPacket(TABLE_UPC_SKILL pSkillData, Vector3 v3TargetPosition);

	static void SendStartFlyingAtTargetPacket(TABLE_UPC_SKILL pSkillData, int32_t iTargetID, Vector3 v3TargetPosition, uint16_t arrowIndex = 0);

	static void SendStartSkillMagicAtTargetPacket(TABLE_UPC_SKILL pSkillData, int32_t iTargetID, Vector3 v3TargetPosition, uint16_t arrowIndex = 0);
	static void SendStartSkillMagicAtPosPacket(TABLE_UPC_SKILL pSkillData, Vector3 v3TargetPosition);

	static void SendStartMagicAtTarget(TABLE_UPC_SKILL pSkillData, int32_t iTargetID, Vector3 v3TargetPosition, uint16_t arrowIndex = 0);

	static void SendCancelSkillPacket(TABLE_UPC_SKILL pSkillData);

	static void SendMovePacket(Vector3 vecStartPosition, Vector3 vecTaragetPosition, int16_t iMoveSpeed, uint8_t iMoveType);

	static void SendRotation(float fRotation);

	static void SendShoppingMall(ShoppingMallType eType);

	static void SendRequestBundleOpen(uint32_t iBundleID);
	static void SendBundleItemGet(uint32_t iBundleID, uint32_t iItemID, int16_t iIndex);

	static void SendItemMovePacket(uint8_t iType, uint8_t iDirection, uint32_t iItemID, uint8_t iCurrentPosition, uint8_t iTargetPosition);

	static Vector3 GetPosition();
	static Vector3 GetTargetPosition();

	static void SetTarget(int32_t iTargetID);
	static int32_t GetTarget();

	static bool IsBuffActive(int32_t iBuffType) { return m_mapActiveBuffList.find(iBuffType) != m_mapActiveBuffList.end(); };

	static bool IsBlinking();

	static std::chrono::milliseconds GetSkillUseTime(int32_t iSkillID);
	static void SetSkillUseTime(int32_t iSkillID, std::chrono::milliseconds iSkillUseTime);

	static bool UseItem(uint32_t iItemID);

	static void LoadSkillData();

	static std::vector<__TABLE_UPC_SKILL> GetAvailableSkill() { return m_vecAvailableSkill; };

	static std::vector<TLoot>* GetLootList() { return &m_vecLootList; }

	static bool IsMovingToLoot() { return m_bIsMovingToLoot; }
	static void SetMovingToLoot(bool bValue) { m_bIsMovingToLoot = bValue; }

	static void EquipOreads(int32_t iItemID);
	static void SetOreads(bool bValue);

protected:
	inline static int32_t m_iTargetID;

	inline static std::map<std::string, DWORD> m_mapAddressList;

	inline static bool m_bWorking;
	inline static State m_byState;

	inline static DWORD m_dwRecvHookAddress;
	inline static DWORD m_dwSendHookAddress;

	inline static std::map<std::string, Ini*> m_mapUserConfig;

	inline static TPlayer m_PlayerMySelf;

	inline static std::vector<TNpc> m_vecNpc;
	inline static std::vector<TPlayer> m_vecPlayer;

	inline static std::map<int32_t, uint32_t> m_mapActiveBuffList;
	inline static std::map<int32_t, std::chrono::milliseconds> m_mapSkillUseTime;

	inline static bool m_bLunarWarDressUp;

	inline static std::vector<__TABLE_UPC_SKILL> m_vecAvailableSkill;

	inline static std::vector<TLoot> m_vecLootList;
	inline static bool m_bIsMovingToLoot;

private:
	typedef void(__thiscall* SendFunction)(DWORD, uint8_t*, uint32_t);
	typedef int(__thiscall* Login1Function)(DWORD);
	typedef int(__thiscall* Login2Function)(DWORD);
	typedef int(__thiscall* DisconnectFunction)(DWORD);
	typedef int(__thiscall* LoginServerFunction)(DWORD);
	typedef int(__thiscall* CharacterSelectSkipFunction)(DWORD);
	typedef int(__thiscall* CharacterSelectFunction)(DWORD);
	typedef int(__thiscall* RouteStartFunction)(int, Vector3*);
	typedef int(__cdecl* PushPhaseFunction)(int);
	typedef void(__thiscall* EquipOreadsFunction)(int, int, char);
};

