#pragma once

#include "Client.h"
#include "Packet.h"
#include "Struct.h"

class Bot;
class ClientHandler : public Client
{
public:
	ClientHandler(Bot* pBot);
	~ClientHandler();

	bool IsWorking() { return m_bWorking; }

	void Clear();
	void ClearUserConfiguration();

	Client* GetClient() { return static_cast<Client*>(this); }

public:
	void Initialize();
	void StartHandler();
	void StopHandler();
	void Process();
	void PatchClient();
	void PatchSocket();
	void OnReady();

private:
	TNpc InitializeNpc(Packet& pkt);
	TPlayer InitializePlayer(Packet& pkt);

private:
	void PatchRecvAddress(DWORD dwAddress);
	void PatchSendAddress();

public:
	void MailSlotRecvProcess();
	void MailSlotSendProcess();

private:
	void RecvProcess(BYTE* byBuffer, DWORD dwLength);
	void SendProcess(BYTE* byBuffer, DWORD dwLength);

	std::function<void(BYTE*, DWORD)> onClientRecvProcess;
	std::function<void(BYTE*, DWORD)> onClientSendProcess;

	HANDLE m_hMailSlotRecv;
	std::string m_szMailSlotRecvName;

	HANDLE m_hMailSlotSend;
	std::string m_szMailSlotSendName;

	LPVOID m_RecvHookAddress;
	LPVOID m_SendHookAddress;

private:
	void SetLoginInformation(std::string szAccountId, std::string szPassword);

public:
	void LoadSkillData();

private:
	void BasicAttackProcess();
	void AttackProcess();
	void MoveToTargetProcess();
	void SearchTargetProcess();

	void AutoLootProcess();

	void MinorProcess();
	void PotionProcess();
	void CharacterProcess();

	bool HealthPotionProcess();
	bool ManaPotionProcess();

	void RouteProcess();
	void SupplyProcess();
	void StatisticsProcess();
	void RemoveItemProcess();

	void VIPStorageSupplyProcess();

	void LevelDownerProcess();

	void MagicHammerProcess();
	void SpeedHackProcess();

	void TransformationProcess();
	void FlashProcess();

	void RegionProcess();

	void RouteRecorderProcess();

private:
	bool m_bReady;
	bool m_bWorking;
	bool m_bMailSlotWorking;

public:
	bool IsReady() { return m_bReady; }

private:
	std::string m_szAccountId;
	std::string m_szPassword;

private:
	float m_fLastSupplyTime;

private:
	void SolveCaptcha(std::vector<uint8_t> vecImageBuffer);

public:
	int GetRegionUserCount(bool bExceptPartyMember = false, float fRangeLimit = 0.0f);

protected:
	uint8_t m_iOTPRetryCount;

protected:
	float m_fLastSearchTargetTime;
	float m_fLastAttackTime;
	float m_fLastCharacterProcessTime;
	float m_fLastMinorProcessTime;
	float m_fLastMagicHammerProcessTime;
	float m_fLastAutoLootProcessTime;
	float m_fLastAutoLootBundleOpenTime;
	float m_fLastTransformationProcessTime;
	float m_fLastFlashProcessTime;
	float m_fLastSpeedHackProcessTime;
	float m_fLastRegionProcessTime;
	float m_fLastMoveToTargetProcessTime;
	float m_fLastLootRequestTime;
	float m_fLastPotionProcessTime;
	float m_fLastStatisticsProcessTime;
	float m_fLastRemoveItemProcessTime;
	float m_fLastLevelDownerProcessTime;
	float m_fLastSupplyProcessTime;
	float m_fLastRouteProcessTime;
	float m_fLastVIPStorageSupplyProcessTime;

protected:
	int32_t PartyMemberNeedSwift();
	int32_t PartyMemberNeedHeal(uint32_t iSkillBaseID);
	int32_t PartyMemberNeedBuff(uint32_t iSkillBaseID);

public:
	bool m_bAttackSpeed;
	int m_iAttackSpeedValue;
	bool m_bAttackStatus;
	std::unordered_set<int> m_vecAttackSkillList;
	bool m_bCharacterStatus;
	bool m_bSearchTargetSpeed;
	int m_iSearchTargetSpeedValue;
	bool m_bClosestTarget;
	bool m_bAutoTarget;
	bool m_bRangeLimit;
	int m_iRangeLimitValue;
	std::unordered_set<int> m_vecSelectedNpcList;
	std::unordered_set<int> m_vecSelectedNpcIDList;
	bool m_bMoveToTarget;
	bool m_bDisableStun;
	bool m_bStartGenieIfUserInRegion;
	int m_iStartGenieIfUserInRegionMeter;

	std::unordered_set<int> m_vecCharacterSkillList;
	bool m_bPartySwift;
	bool m_bPriestPartyHeal;
	bool m_bHealProtection;
	bool m_bPriestPartyBuff;
	int m_iHealProtectionValue;
	bool m_bUseSkillWithPacket;
	bool m_bOnlyAttackSkillUseWithPacket;

	bool m_bAttackRangeLimit;
	int m_iAttackRangeLimitValue;
	bool m_bBasicAttack;
	bool m_bBasicAttackWithPacket;

	bool m_bVIPSellSupply;

	bool m_bAutoLoot;
	bool m_bMoveToLoot;
	bool m_bMinorProtection;
	int m_iMinorProtectionValue;
	bool m_bHpProtectionEnable;
	int32_t m_iHpProtectionValue;
	bool m_bMpProtectionEnable;
	int32_t m_iMpProtectionValue;
	bool m_bAutoRepairMagicHammer;
	bool m_bSpeedHack;
	bool m_bAutoTransformation;
	int m_iTransformationItem;
	int m_iTransformationSkill;
	bool m_bAutoDCFlash;
	int m_iAutoDCFlashCount;
	bool m_bAutoWarFlash;
	int m_iAutoWarFlashCount;
	bool m_bAutoExpFlash;
	int m_iAutoExpFlashCount;

	int m_iLootMinPrice;
	bool m_bDeathEffect;
	bool m_bDisableCasting;

	bool m_bTargetSizeEnable;
	int m_iTargetSize;
	bool m_bCharacterSizeEnable;
	int m_iCharacterSize;

	bool m_bSaveCPUEnable;
	int m_iSaveCPUValue;

	bool m_bPartyRequest;
	std::string m_szPartyRequestMessage;
	bool m_bTeleportRequest;
	std::string m_szTeleportRequestMessage;

	bool m_bTownStopBot;
	bool m_bTownOrTeleportStopBot;
	bool m_bSyncWithGenie;

	bool m_bLegalMode;
	bool m_bSpeedMode;

	bool m_bStopBotIfDead;

	bool m_bSendTownIfBanNotice;
	bool m_bPlayBeepfIfBanNotice;

	bool m_bWallHack;

	bool m_bArcherCombo;

	std::unordered_set<std::string> m_setSelectedSupplyRouteList;
	std::unordered_set<std::string> m_setSelectedDeathRouteList;
	std::unordered_set<std::string> m_setSelectedLoginRouteList;

	bool m_bAutoRepair;

	std::unordered_set<int> m_vecSupplyList;

	bool m_bAutoSupply;
	bool m_bAutoRPRChangeWeapon;

	std::unordered_set<int> m_vecAutoRPRChangeWeaponLeft;
	std::unordered_set<int> m_vecAutoRPRChangeWeaponRight;

	int m_iSlotExpLimit;
	bool m_bSlotExpLimitEnable;
	bool m_bPartyLeaderSelect;

	std::unordered_set<int> m_vecLootItemList;
	std::unordered_set<int> m_vecSellItemList;
	std::unordered_set<int> m_vecInnItemList;
	std::unordered_set<int> m_vecDeleteItemList;

	int m_iLootType;
	bool m_bMinPriceLootEnable;

	uint32_t m_iStartCoin;
	uint64_t m_iCoinCounter;
	uint64_t m_iExpCounter;

	uint64_t m_iEveryMinuteCoinPrevCounter;
	uint64_t m_iEveryMinuteCoinCounter;
	uint64_t m_iEveryMinuteExpPrevCounter;
	uint64_t m_iEveryMinuteExpCounter;

	bool m_bHidePlayer;

	bool m_bLevelDownerEnable;
	int m_iLevelDownerNpcId;
	bool m_bLevelDownerLevelLimitEnable;
	int m_iLevelDownerLevelLimit;
	bool m_bLevelDownerStopNearbyPlayer;

public:
	void InitializeUserConfiguration();

protected:
	bool UseSkill(TABLE_UPC_SKILL pSkillData, int32_t iTargetID, int32_t iPriority = 0, bool bWaitCastTime = true);
	bool UseItem(uint32_t iItemID);

protected:
	bool IsNeedRepair();
	bool IsNeedSupply();
	bool IsNeedSell();

protected:
	std::queue<TABLE_UPC_SKILL> m_qAttackSkillQueue;

public:
	void SetRoute(std::vector<Route> vecRoute);
	bool IsRouting() { return m_vecRouteActive.size() > 0; };
	void ClearRoute();
	bool m_bIsRoutePlanning;

public:
	std::vector<Route> m_vecRouteActive;
	std::vector<Route> m_vecRoutePlan;
	Route* m_pCurrentRunningRoute;

protected:
	bool m_bRouteWarpListLoaded;

public:
	Ini* GetUserConfiguration();
	Ini* m_iniUserConfiguration;

public:
	typedef std::map<std::string, std::vector<Route>> RouteList;
	typedef std::map<uint8_t, RouteList> RouteData;

	RouteData m_mapRouteList;

public:
	bool GetRouteList(uint8_t iMapIndex, RouteList& pRouteList);
	void SaveRoute(std::string szRouteName, uint8_t iMapIndex, std::vector<Route> vecRoute);
	void DeleteRoute(std::string szRouteName, uint8_t iMapIndex);
};



