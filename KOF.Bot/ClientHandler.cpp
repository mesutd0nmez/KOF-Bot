#include "pch.h"
#include "ClientHandler.h"
#include "Client.h"
#include "Memory.h"
#include "Packet.h"
#include "Bot.h"
#include "Service.h"
#include "UI.h"
#include "Password.h"
#include "Drawing.h"

ClientHandler::ClientHandler(Bot* pBot)
{
	m_Bot = pBot;

	Clear();
}

ClientHandler::~ClientHandler()
{
	m_Bot = nullptr;

	Clear();
}

void ClientHandler::Clear()
{
	m_bWorking = false;

	m_bMailSlotWorking = false;
	m_hMailSlotRecv = nullptr;
	m_szMailSlotRecvName.clear();
	m_hMailSlotSend = nullptr;
	m_szMailSlotSendName.clear();

	m_RecvHookAddress = 0;
	m_SendHookAddress = 0;

	m_szAccountId.clear();
	m_szPassword.clear();

	m_vecRouteActive.clear();
	m_vecRoutePlan.clear();

	m_fLastSupplyTime = 0;

	m_iOTPRetryCount = 0;

	m_fLastSearchTargetTime = 0.0f;
	m_fLastAttackTime = 0.0f;
	m_fLastCharacterProcessTime = 0.0f;
	m_fLastMinorProcessTime = 0.0f;
	m_fLastMagicHammerProcessTime = 0.0f;
	m_fLastAutoLootProcessTime = 0.0f;
	m_fLastAutoLootBundleOpenTime = 0.0f;
	m_fLastTransformationProcessTime = 0.0f;
	m_fLastRegionProcessTime = 0.0f;
	m_fLastLootRequestTime = 0.0f;
	m_fLastPotionProcessTime = 0.0f;
	m_fLastStatisticsProcessTime = 0.0f;
	m_fLastRemoveItemProcessTime = 0.0f;
	m_fLastLevelDownerProcessTime = 0.0f;
	m_fLastSupplyProcessTime = 0.0f;
	m_fLastRouteProcessTime = 0.0f;
	m_fLastVIPStorageSupplyProcessTime = 0.0f;

	m_bIsRoutePlanning = false;

	m_bRouteWarpListLoaded = false;

	m_pCurrentRunningRoute = nullptr;

	m_iStartCoin = 0;
	m_iCoinCounter = 0;
	m_iExpCounter = 0;

	m_iEveryMinuteCoinPrevCounter = 0;
	m_iEveryMinuteCoinCounter = 0;
	m_iEveryMinuteExpPrevCounter = 0;
	m_iEveryMinuteExpCounter = 0;

	m_iStartGenieIfUserInRegionMeter = 100;

	m_setSelectedSupplyRouteList.clear();
	m_setSelectedDeathRouteList.clear();
	m_setSelectedLoginRouteList.clear();

	ClearUserConfiguration();
}

void ClientHandler::ClearUserConfiguration()
{
	m_bAttackSpeed = false;
	m_iAttackSpeedValue = 100;
	m_bAttackStatus = false;
	m_vecAttackSkillList.clear();
	m_bCharacterStatus = false;

	m_bSearchTargetSpeed = false;
	m_iSearchTargetSpeedValue = 100;
	m_bClosestTarget = false;
	m_bAutoTarget = false;
	m_bRangeLimit = false;
	m_iRangeLimitValue = 100;
	m_vecSelectedNpcList.clear();
	m_vecSelectedNpcIDList.clear();

	m_bMoveToTarget = false;
	m_bDisableStun = false;

	m_bStartGenieIfUserInRegion = false;

	m_vecCharacterSkillList.clear();
	m_bPartySwift = false;
	m_bPriestPartyHeal = false;
	m_bHealProtection = false;
	m_bPriestPartyBuff = false;
	m_iHealProtectionValue = 75;
	m_bUseSkillWithPacket = false;
	m_bOnlyAttackSkillUseWithPacket = false;

	m_bAttackRangeLimit = false;
	m_iAttackRangeLimitValue = 50;
	m_bBasicAttack = false;
	m_bBasicAttackWithPacket = false;

	m_bAutoLoot = false;
	m_bMoveToLoot = false;
	m_bMinorProtection = false;
	m_iMinorProtectionValue = 30;
	m_bHpProtectionEnable = false;
	m_iHpProtectionValue = 50;
	m_bMpProtectionEnable = false;
	m_iMpProtectionValue = 50;
	m_bAutoRepairMagicHammer = false;
	m_bSpeedHack = false;
	m_bAutoTransformation = false;
	m_iTransformationItem = 381001000;
	m_iTransformationSkill = 472020;
	m_bAutoDCFlash = false;
	m_iAutoDCFlashCount = 10;
	m_bAutoWarFlash = false;
	m_iAutoWarFlashCount = 10;
	m_bAutoExpFlash = false;
	m_iAutoExpFlashCount = 10;

	m_iLootMinPrice = 0;

	m_bDeathEffect = false;

	m_bDisableCasting = false;

	m_bTargetSizeEnable = false;
	m_iTargetSize = 1;
	m_bCharacterSizeEnable = false;
	m_iCharacterSize = 1;

	m_bSaveCPUEnable = false;
	m_iSaveCPUValue = 1;

	m_bPartyRequest = false;
	m_szPartyRequestMessage = skCryptDec("add");
	m_bTeleportRequest = false;
	m_szTeleportRequestMessage = skCryptDec("tptp");

	m_bTownStopBot = true;
	m_bTownOrTeleportStopBot = false;
	m_bSyncWithGenie = false;

	m_bLegalMode = true;
	m_bSpeedMode = true;

	m_bStopBotIfDead = true;

	m_bSendTownIfBanNotice = false;
	m_bPlayBeepfIfBanNotice = false;

	m_bWallHack = false;
	m_bLegalWallHack = false;

	m_bArcherCombo = true;
	m_bRouteWarpListLoaded = false;

	m_bAutoRepair = false;

	m_vecSupplyList.clear();

	m_bAutoSupply = false;

	m_iSlotExpLimit = 35000;
	m_bSlotExpLimitEnable = false;

	m_bPartyLeaderSelect = false;

	m_vecLootItemList.clear();
	m_vecSellItemList.clear();
	m_vecInnItemList.clear();
	m_vecDeleteItemList.clear();

	m_iLootType = 0;

	m_bMinPriceLootEnable = false;

	m_bHidePlayer = false;

	m_bLevelDownerEnable = false;
	m_iLevelDownerNpcId = -1;
	m_bLevelDownerLevelLimitEnable = true;
	m_iLevelDownerLevelLimit = 35;
	m_bLevelDownerStopNearbyPlayer = true;

	m_iStartGenieIfUserInRegionMeter = 100;

	m_setSelectedSupplyRouteList.clear();
	m_setSelectedDeathRouteList.clear();
	m_setSelectedLoginRouteList.clear();

	m_bVIPSellSupply = false;
}

void ClientHandler::Initialize()
{
#ifdef DEBUG
	printf("Client handler initializing\n");
#endif

	PatchSocket();
}

void ClientHandler::InitializeUserConfiguration()
{
	m_bAttackSpeed = GetUserConfiguration()->GetBool("Attack", "AttackSpeed", m_bAttackSpeed);
	m_iAttackSpeedValue = GetUserConfiguration()->GetInt("Attack", "AttackSpeedValue", m_iAttackSpeedValue);
	m_bAttackStatus = GetUserConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Attack"), m_bAttackStatus);
	m_vecAttackSkillList = GetUserConfiguration()->GetInt(skCryptDec("Automation"), skCryptDec("AttackSkillList"), m_vecAttackSkillList);
	m_bCharacterStatus = GetUserConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Character"), m_bCharacterStatus);
	m_bSearchTargetSpeed = GetUserConfiguration()->GetBool("Attack", "SearchTargetSpeed", m_bSearchTargetSpeed);
	m_iSearchTargetSpeedValue = GetUserConfiguration()->GetInt("Attack", "SearchTargetSpeedValue", m_iSearchTargetSpeedValue);
	m_bClosestTarget = GetUserConfiguration()->GetInt(skCryptDec("Attack"), skCryptDec("ClosestTarget"), m_bClosestTarget);
	m_bAutoTarget = GetUserConfiguration()->GetInt(skCryptDec("Attack"), skCryptDec("AutoTarget"), m_bAutoTarget);
	m_bRangeLimit = GetUserConfiguration()->GetBool(skCryptDec("Attack"), skCryptDec("RangeLimit"), m_bRangeLimit);
	m_iRangeLimitValue = GetUserConfiguration()->GetInt(skCryptDec("Attack"), skCryptDec("RangeLimitValue"), m_iRangeLimitValue);
	m_vecSelectedNpcList = GetUserConfiguration()->GetInt(skCryptDec("Attack"), skCryptDec("NpcList"), m_vecSelectedNpcList);
	m_vecSelectedNpcIDList = GetUserConfiguration()->GetInt(skCryptDec("Attack"), skCryptDec("NpcIDList"), m_vecSelectedNpcIDList);
	m_bMoveToTarget = GetUserConfiguration()->GetBool(skCryptDec("Attack"), skCryptDec("MoveToTarget"), m_bMoveToTarget);
	m_bDisableStun = GetUserConfiguration()->GetBool(skCryptDec("Attack"), skCryptDec("DisableStun"), m_bDisableStun);
	m_bStartGenieIfUserInRegion = GetUserConfiguration()->GetBool(skCryptDec("Settings"), skCryptDec("StartGenieIfUserInRegion"), m_bStartGenieIfUserInRegion);
	m_iStartGenieIfUserInRegionMeter = GetUserConfiguration()->GetInt(skCryptDec("Settings"), skCryptDec("StartGenieIfUserInRegionMeter"), m_iStartGenieIfUserInRegionMeter);
	m_vecCharacterSkillList = GetUserConfiguration()->GetInt(skCryptDec("Automation"), skCryptDec("CharacterSkillList"), m_vecCharacterSkillList);
	m_bPartySwift = GetUserConfiguration()->GetBool(skCryptDec("Rogue"), skCryptDec("PartySwift"), m_bPartySwift);
	m_bPriestPartyHeal = GetUserConfiguration()->GetBool(skCryptDec("Priest"), skCryptDec("PartyHeal"), m_bPriestPartyHeal);
	m_bHealProtection = GetUserConfiguration()->GetBool(skCryptDec("Protection"), skCryptDec("Heal"), m_bHealProtection);
	m_bPriestPartyBuff = GetUserConfiguration()->GetBool(skCryptDec("Priest"), skCryptDec("PartyBuff"), m_bPriestPartyBuff);
	m_iHealProtectionValue = GetUserConfiguration()->GetInt(skCryptDec("Protection"), skCryptDec("HealValue"), m_iHealProtectionValue);
	m_bUseSkillWithPacket = GetUserConfiguration()->GetBool(skCryptDec("Skill"), skCryptDec("UseSkillWithPacket"), m_bUseSkillWithPacket);
	m_bOnlyAttackSkillUseWithPacket = GetUserConfiguration()->GetBool(skCryptDec("Skill"), skCryptDec("OnlyAttackSkillUseWithPacket"), m_bOnlyAttackSkillUseWithPacket);
	m_bAttackRangeLimit = GetUserConfiguration()->GetBool(skCryptDec("Attack"), skCryptDec("AttackRangeLimit"), m_bAttackRangeLimit);
	m_iAttackRangeLimitValue = GetUserConfiguration()->GetInt(skCryptDec("Attack"), skCryptDec("AttackRangeLimitValue"), m_iAttackRangeLimitValue);
	m_bBasicAttack = GetUserConfiguration()->GetBool(skCryptDec("Attack"), skCryptDec("BasicAttack"), m_bBasicAttack);
	m_bBasicAttackWithPacket = GetUserConfiguration()->GetBool(skCryptDec("Attack"), skCryptDec("BasicAttackWithPacket"), m_bBasicAttackWithPacket);
	m_bAutoLoot = GetUserConfiguration()->GetBool(skCryptDec("AutoLoot"), skCryptDec("Enable"), m_bAutoLoot);
	m_bMoveToLoot = GetUserConfiguration()->GetBool(skCryptDec("AutoLoot"), skCryptDec("MoveToLoot"), m_bMoveToLoot);
	m_bMinorProtection = GetUserConfiguration()->GetBool(skCryptDec("Protection"), skCryptDec("Minor"), m_bMinorProtection);
	m_iMinorProtectionValue = GetUserConfiguration()->GetInt(skCryptDec("Protection"), skCryptDec("MinorValue"), m_iMinorProtectionValue);
	m_bHpProtectionEnable = GetUserConfiguration()->GetBool(skCryptDec("Protection"), skCryptDec("Hp"), m_bHpProtectionEnable);
	m_iHpProtectionValue = GetUserConfiguration()->GetInt(skCryptDec("Protection"), skCryptDec("HpValue"), m_iHpProtectionValue);
	m_bMpProtectionEnable = GetUserConfiguration()->GetBool(skCryptDec("Protection"), skCryptDec("Mp"), m_bMpProtectionEnable);
	m_iMpProtectionValue = GetUserConfiguration()->GetInt(skCryptDec("Protection"), skCryptDec("MpValue"), m_iMpProtectionValue);
	m_bAutoRepairMagicHammer = GetUserConfiguration()->GetBool(skCryptDec("Supply"), skCryptDec("AutoRepairMagicHammer"), m_bAutoRepairMagicHammer);
	m_bSpeedHack = GetUserConfiguration()->GetBool(skCryptDec("Feature"), skCryptDec("SpeedHack"), m_bSpeedHack);
	m_bAutoTransformation = GetUserConfiguration()->GetBool(skCryptDec("Transformation"), skCryptDec("Auto"), m_bAutoTransformation);
	m_iTransformationItem = GetUserConfiguration()->GetInt(skCryptDec("Transformation"), skCryptDec("Item"), m_iTransformationItem);
	m_iTransformationSkill = GetUserConfiguration()->GetInt(skCryptDec("Transformation"), skCryptDec("Skill"), m_iTransformationSkill);
	m_bAutoDCFlash = GetUserConfiguration()->GetBool(skCryptDec("Settings"), skCryptDec("AutoDCFlash"), m_bAutoDCFlash);
	m_iAutoDCFlashCount = GetUserConfiguration()->GetInt(skCryptDec("Settings"), skCryptDec("AutoDCFlashCount"), m_iAutoDCFlashCount);
	m_bAutoWarFlash = GetUserConfiguration()->GetBool(skCryptDec("Settings"), skCryptDec("AutoWarFlash"), m_bAutoWarFlash);
	m_iAutoWarFlashCount = GetUserConfiguration()->GetInt(skCryptDec("Settings"), skCryptDec("AutoWarFlashCount"), m_iAutoWarFlashCount);
	m_bAutoExpFlash = GetUserConfiguration()->GetBool(skCryptDec("Settings"), skCryptDec("AutoExpFlash"), m_bAutoExpFlash);
	m_iAutoExpFlashCount = GetUserConfiguration()->GetInt(skCryptDec("Settings"), skCryptDec("AutoExpFlashCount"), m_iAutoExpFlashCount);
	m_iLootMinPrice = GetUserConfiguration()->GetInt(skCryptDec("AutoLoot"), skCryptDec("MinPrice"), m_iLootMinPrice);
	m_bDeathEffect = GetUserConfiguration()->GetBool(skCryptDec("Feature"), skCryptDec("DeathEffect"), m_bDeathEffect);
	m_bDisableCasting = GetUserConfiguration()->GetBool(skCryptDec("Feature"), skCryptDec("DisableCasting"), m_bDisableCasting);
	m_bTargetSizeEnable = GetUserConfiguration()->GetBool(skCryptDec("Target"), skCryptDec("SizeEnable"), m_bTargetSizeEnable);
	m_iTargetSize = GetUserConfiguration()->GetInt(skCryptDec("Target"), skCryptDec("Size"), m_iTargetSize);
	m_bCharacterSizeEnable = GetUserConfiguration()->GetBool(skCryptDec("Character"), skCryptDec("SizeEnable"), m_bCharacterSizeEnable);
	m_iCharacterSize = GetUserConfiguration()->GetInt(skCryptDec("Character"), skCryptDec("Size"), m_iCharacterSize);
	m_bSaveCPUEnable = GetUserConfiguration()->GetBool(skCryptDec("Feature"), skCryptDec("SaveCPU"), m_bSaveCPUEnable);
	m_iSaveCPUValue = GetUserConfiguration()->GetInt(skCryptDec("Feature"), skCryptDec("SaveCPUValue"), m_iSaveCPUValue);
	m_bPartyRequest = GetUserConfiguration()->GetBool(skCryptDec("Listener"), skCryptDec("PartyRequest"), m_bPartyRequest);
	m_szPartyRequestMessage = GetUserConfiguration()->GetString(skCryptDec("Listener"), skCryptDec("PartyRequestMessage"), m_szPartyRequestMessage.c_str());
	m_bTeleportRequest = GetUserConfiguration()->GetBool(skCryptDec("Listener"), skCryptDec("TeleportRequest"), m_bTeleportRequest);
	m_szTeleportRequestMessage = GetUserConfiguration()->GetString(skCryptDec("Listener"), skCryptDec("TeleportRequestMessage"), m_szTeleportRequestMessage.c_str());
	m_bTownStopBot = GetUserConfiguration()->GetBool(skCryptDec("Bot"), skCryptDec("TownStopBot"), m_bTownStopBot);
	m_bTownOrTeleportStopBot = GetUserConfiguration()->GetBool(skCryptDec("Bot"), skCryptDec("TownOrTeleportStopBot"), m_bTownOrTeleportStopBot);
	m_bSyncWithGenie = GetUserConfiguration()->GetBool(skCryptDec("Bot"), skCryptDec("SyncWithGenie"), m_bSyncWithGenie);
	m_bLegalMode = GetUserConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("LegalMode"), m_bLegalMode);
	m_bSpeedMode = GetUserConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("SpeedMode"), m_bSpeedMode);
	m_bStopBotIfDead = GetUserConfiguration()->GetBool(skCryptDec("Settings"), skCryptDec("StopBotIfDead"), m_bStopBotIfDead);
	m_bSendTownIfBanNotice = GetUserConfiguration()->GetBool(skCryptDec("Settings"), skCryptDec("SendTownIfBanNotice"), m_bSendTownIfBanNotice);
	m_bPlayBeepfIfBanNotice = GetUserConfiguration()->GetBool(skCryptDec("Settings"), skCryptDec("PlayBeepfIfBanNotice"), m_bPlayBeepfIfBanNotice);
	m_bWallHack = GetUserConfiguration()->GetBool(skCryptDec("Feature"), skCryptDec("WallHack"), m_bWallHack);
	m_bLegalWallHack = GetUserConfiguration()->GetBool(skCryptDec("Feature"), skCryptDec("LegalWallHack"), m_bLegalWallHack);
	m_bArcherCombo = GetUserConfiguration()->GetBool(skCryptDec("Attack"), skCryptDec("ArcherCombo"), m_bArcherCombo);
	m_bAutoRepair = GetUserConfiguration()->GetBool(skCryptDec("Supply"), skCryptDec("AutoRepair"), m_bAutoRepair);
	m_vecSupplyList = GetUserConfiguration()->GetInt(skCryptDec("Supply"), skCryptDec("Enable"), m_vecSupplyList);
	m_bAutoSupply = GetUserConfiguration()->GetBool(skCryptDec("Supply"), skCryptDec("AutoSupply"), m_bAutoSupply);
	m_iSlotExpLimit = GetUserConfiguration()->GetInt(skCryptDec("Bot"), skCryptDec("SlotExpLimit"), m_iSlotExpLimit);
	m_bSlotExpLimitEnable = GetUserConfiguration()->GetBool(skCryptDec("Bot"), skCryptDec("SlotExpLimitEnable"), m_bSlotExpLimitEnable);
	m_bPartyLeaderSelect = GetUserConfiguration()->GetBool(skCryptDec("Bot"), skCryptDec("PartyLeaderSelect"), m_bPartyLeaderSelect);
	m_vecLootItemList = GetUserConfiguration()->GetInt(skCryptDec("Automation"), skCryptDec("LootItemList"), m_vecLootItemList);
	m_vecSellItemList = GetUserConfiguration()->GetInt(skCryptDec("Automation"), skCryptDec("SellItemList"), m_vecSellItemList);
	m_vecInnItemList = GetUserConfiguration()->GetInt(skCryptDec("Automation"), skCryptDec("InnItemList"), m_vecInnItemList);
	m_vecDeleteItemList = GetUserConfiguration()->GetInt(skCryptDec("Automation"), skCryptDec("DeleteItemList"), m_vecDeleteItemList);
	m_iLootType = GetUserConfiguration()->GetInt(skCryptDec("AutoLoot"), skCryptDec("LootType"), m_iLootType);
	m_bMinPriceLootEnable = GetUserConfiguration()->GetBool(skCryptDec("AutoLoot"), skCryptDec("MinPriceLootEnable"), m_bMinPriceLootEnable);
	m_bHidePlayer = GetUserConfiguration()->GetBool(skCryptDec("Feature"), skCryptDec("HidePlayer"), m_bHidePlayer);
	m_bLevelDownerEnable = GetUserConfiguration()->GetInt(skCryptDec("LevelDowner"), skCryptDec("Enable"), m_bLevelDownerEnable);
	m_iLevelDownerNpcId = GetUserConfiguration()->GetInt(skCryptDec("LevelDowner"), skCryptDec("NpcId"), m_iLevelDownerNpcId);
	m_bLevelDownerLevelLimitEnable = GetUserConfiguration()->GetInt(skCryptDec("LevelDowner"), skCryptDec("LevelLimitEnable"), m_bLevelDownerLevelLimitEnable);
	m_iLevelDownerLevelLimit = GetUserConfiguration()->GetInt(skCryptDec("LevelDowner"), skCryptDec("LevelLimit"), m_iLevelDownerLevelLimit);
	m_bLevelDownerStopNearbyPlayer = GetUserConfiguration()->GetInt(skCryptDec("LevelDowner"), skCryptDec("StopIfNearbyPlayer"), m_bLevelDownerStopNearbyPlayer);

	m_setSelectedSupplyRouteList = GetUserConfiguration()->GetString(skCryptDec("Bot"), skCryptDec("SelectedSupplyRouteList"), m_setSelectedSupplyRouteList);
	m_setSelectedDeathRouteList = GetUserConfiguration()->GetString(skCryptDec("Bot"), skCryptDec("SelectedDeathRouteList"), m_setSelectedDeathRouteList);
	m_setSelectedLoginRouteList = GetUserConfiguration()->GetString(skCryptDec("Bot"), skCryptDec("SelectedLoginRouteList"), m_setSelectedLoginRouteList);

	m_bVIPSellSupply = GetUserConfiguration()->GetBool(skCryptDec("VIPStorage"), skCryptDec("SellSupply"), m_bVIPSellSupply);
}

void ClientHandler::StartHandler()
{
#ifdef DEBUG
	printf("Client handler starting\n");
#endif

	m_bWorking = true;
}

void ClientHandler::StopHandler()
{
#ifdef DEBUG
	printf("Client handler stopped\n");
#endif

	m_bWorking = false;
	m_bMailSlotWorking = false;

	GetClient()->Clear();
}

void ClientHandler::Process()
{
	if (m_bMailSlotWorking)
	{
		MailSlotRecvProcess();
		MailSlotSendProcess();
	}

	if (m_bWorking)
	{
		if (IsDisconnect())
		{
#ifdef DEBUG
			printf("Client connection closed\n");
#endif
			StopHandler();
		}
		else
		{
			RegionProcess();
			PotionProcess();

			if (IsRogue())
			{
				MinorProcess();
			}

			CharacterProcess();

			SearchTargetProcess();

			MoveToTargetProcess();
			BasicAttackProcess();
			AttackProcess();

			MagicHammerProcess();
			AutoLootProcess();
			TransformationProcess();
			FlashProcess();
			SpeedHackProcess();

			RouteProcess();
			RouteRecorderProcess();
			SupplyProcess();

			StatisticsProcess();
			RemoveItemProcess();
			LevelDownerProcess();

			VIPStorageSupplyProcess();
		}
	}
}

void ClientHandler::OnReady()
{
#ifdef DEBUG
	printf("Client handler ready\n");
#endif

	new std::thread([&]()
	{
		WaitCondition(m_Bot->m_bInternalMailslotWorking == false);

		PushPhase(m_Bot->GetAddress(skCryptDec("KO_PTR_INTRO")));

		WaitCondition(m_Bot->Read4Byte(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_INTRO"))) + m_Bot->GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO"))) == 0);

		if (m_Bot->m_bAutoLogin
			&& m_Bot->m_szID.size() > 0 && m_Bot->m_szPassword.size())
		{
			if (m_Bot->m_ePlatformType != PlatformType::STKO)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(500));

				SetLoginInformation(m_Bot->m_szID, m_Bot->m_szPassword);
				WriteLoginInformation(m_szAccountId, m_szPassword);
				ConnectLoginServer();
			}
		}

		m_Bot->InitializeStaticData();
	});
}

void ClientHandler::PatchSocket()
{
	onClientSendProcess = [=](BYTE* iStream, DWORD iStreamLength)
	{
		SendProcess(iStream, iStreamLength);
	};

	onClientRecvProcess = [=](BYTE* iStream, DWORD iStreamLength)
	{
		RecvProcess(iStream, iStreamLength);
	};

	for (size_t i = 0; i < 12; i++)
		PatchRecvAddress(m_Bot->GetAddress(skCryptDec("KO_PTR_INTRO")) + (4 * i));

	PatchSendAddress();

	m_bMailSlotWorking = true;

	OnReady();
}

void ClientHandler::PatchRecvAddress(DWORD iAddress)
{
	HANDLE hProcess = m_Bot->GetInjectedProcessHandle();

	WaitCondition(m_Bot->Read4Byte(iAddress) == 0);

	HMODULE hModuleKernel32 = GetModuleHandle(skCryptDec("kernel32.dll"));

	if (hModuleKernel32 == nullptr)
	{
#ifdef DEBUG
		printf("hModuleKernel32 == nullptr\n");
#endif
		return;
	}

	LPVOID pCreateFilePtr = GetProcAddress(hModuleKernel32, skCryptDec("CreateFileA"));
	LPVOID pWriteFilePtr = GetProcAddress(hModuleKernel32, skCryptDec("WriteFile"));
	LPVOID pCloseHandlePtr = GetProcAddress(hModuleKernel32, skCryptDec("CloseHandle"));

	m_szMailSlotRecvName = skCryptDec("\\\\.\\mailslot\\KOF1\\") + std::to_string(m_Bot->GetInjectedProcessId());
	std::vector<BYTE> vecMailSlotName(m_szMailSlotRecvName.begin(), m_szMailSlotRecvName.end());

	if (m_hMailSlotRecv == nullptr)
	{
		m_hMailSlotRecv = CreateMailslotA(m_szMailSlotRecvName.c_str(), 0, MAILSLOT_WAIT_FOREVER, NULL);

		if (m_hMailSlotRecv == INVALID_HANDLE_VALUE)
		{
#ifdef DEBUG
			printf("CreateMailslot recv failed with %d\n", GetLastError());
#endif
			return;
		}
	}

	LPVOID pMailSlotNameAddress = VirtualAllocEx(hProcess, nullptr, vecMailSlotName.size(), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (pMailSlotNameAddress == 0)
	{
		return;
	}

	m_Bot->WriteBytes((DWORD)pMailSlotNameAddress, vecMailSlotName);

	if (m_RecvHookAddress == 0)
	{
		BYTE byHookPatch[] =
		{
			0x55,
			0x8B, 0xEC,
			0x83, 0xC4, 0xF4,
			0x33, 0xC0,
			0x89, 0x45, 0xFC,
			0x33, 0xD2,
			0x89, 0x55, 0xF8,
			0x6A, 0x00,
			0x68, 0x80, 0x00, 0x00, 0x00,
			0x6A, 0x03,
			0x6A, 0x00,
			0x6A, 0x01,
			0x68, 0x00, 0x00, 0x00, 0x40,
			0x68, 0x00, 0x00, 0x00, 0x00,
			0xE8, 0x00, 0x00, 0x00, 0x00,
			0x89, 0x45, 0xF8,
			0x6A, 0x00,
			0x8D, 0x4D, 0xFC,
			0x51,
			0xFF, 0x75, 0x0C,
			0xFF, 0x75, 0x08,
			0xFF, 0x75, 0xF8,
			0xE8, 0x00, 0x00, 0x00, 0x00,
			0x89, 0x45, 0xF4,
			0xFF, 0x75, 0xF8,
			0xE8, 0x00, 0x00, 0x00, 0x00,
			0x8B, 0xE5,
			0x5D,
			0xC3
		};

		m_RecvHookAddress = VirtualAllocEx(hProcess, nullptr, sizeof(byHookPatch), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

		if (m_RecvHookAddress == 0)
		{
			return;
		}

		CopyBytes(byHookPatch + 35, pMailSlotNameAddress);

		DWORD iCreateFileDifference = Memory::GetDifference((DWORD)m_RecvHookAddress + 39, (DWORD)pCreateFilePtr);
		CopyBytes(byHookPatch + 40, iCreateFileDifference);

		DWORD iWriteFileDifference = Memory::GetDifference((DWORD)m_RecvHookAddress + 62, (DWORD)pWriteFilePtr);
		CopyBytes(byHookPatch + 63, iWriteFileDifference);

		DWORD iCloseHandlePtrDifference = Memory::GetDifference((DWORD)m_RecvHookAddress + 73, (DWORD)pCloseHandlePtr);
		CopyBytes(byHookPatch + 74, iCloseHandlePtrDifference);

		std::vector<BYTE> vecHookPatch(byHookPatch, byHookPatch + sizeof(byHookPatch));
		m_Bot->WriteBytes((DWORD)m_RecvHookAddress, vecHookPatch);
	}

	DWORD iRecvAddress = m_Bot->Read4Byte(m_Bot->Read4Byte(iAddress)) + 0x8;

	BYTE byPatch[] =
	{
		0x55,									//push ebp
		0x8B, 0xEC,								//mov ebp,esp
		0x83, 0xC4, 0xF8,						//add esp,-08
		0x53,									//push ebx
		0x8B, 0x45, 0x08,						//mov eax,[ebp+08]
		0x83, 0xC0, 0x04,						//add eax,04
		0x8B, 0x10,								//mov edx,[eax]
		0x89, 0x55, 0xFC,						//mov [ebp-04],edx
		0x8B, 0x4D, 0x08,						//mov ecx,[ebp+08]
		0x83, 0xC1, 0x08,						//add ecx,08
		0x8B, 0x01,								//mov eax,[ecx]
		0x89, 0x45, 0xF8,						//mov [ebp-08],eax
		0xFF, 0x75, 0xFC,						//push [ebp-04]
		0xFF, 0x75, 0xF8,						//push [ebp-08]
		0xB8, 0x00, 0x00, 0x00, 0x00,			//mov eax,00000000 <-- ClientHook::RecvProcess()
		0xFF, 0xD0,								//call eax
		0x83, 0xC4, 0x08,						//add esp,08
		0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00,		//mov ecx,[00000000] <-- KO_PTR_DLG
		0xFF, 0x75, 0x0C,						//push [ebp+0C]
		0xFF, 0x75, 0x08,						//push [ebp+08]
		0xB8, 0x00, 0x00, 0x00, 0x00,			//mov eax,00000000 <-- GetRecvCallAddress()
		0xFF, 0xD0,								//call eax
		0x5B,									//pop ebx
		0x59,									//pop ecx
		0x59,									//pop ecx
		0x5D,									//pop ebp
		0xC2, 0x08, 0x00						//ret 0008
	};

	DWORD iRecvProcessFunction = (DWORD)(LPVOID*)m_RecvHookAddress;
	CopyBytes(byPatch + 36, iRecvProcessFunction);

	DWORD iDlgAddress = iAddress;
	CopyBytes(byPatch + 47, iDlgAddress);

	DWORD iRecvCallAddress = m_Bot->Read4Byte(iRecvAddress);
	CopyBytes(byPatch + 58, iRecvCallAddress);

	std::vector<BYTE> vecPatch(byPatch, byPatch + sizeof(byPatch));

	LPVOID pPatchAddress = VirtualAllocEx(hProcess, nullptr, sizeof(byPatch), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (pPatchAddress == nullptr)
	{
		return;
	}

	m_Bot->WriteBytes((DWORD)pPatchAddress, vecPatch);

	DWORD dwOldProtection;
	VirtualProtectEx(hProcess, (LPVOID)iRecvAddress, 1, PAGE_EXECUTE_READWRITE, &dwOldProtection);
	m_Bot->Write4Byte(m_Bot->Read4Byte(m_Bot->Read4Byte(iAddress)) + 0x8, (DWORD)pPatchAddress);
	VirtualProtectEx(hProcess, (LPVOID)iRecvAddress, 1, dwOldProtection, &dwOldProtection);

#ifdef DEBUG
	printf("PatchRecvAddress: 0x%x patched\n", iRecvAddress);
#endif
}

void ClientHandler::PatchSendAddress()
{
	HANDLE hProcess = m_Bot->GetInjectedProcessHandle();

	WaitCondition(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_SND_FNC"))) == 0);

	HMODULE hModuleKernel32 = GetModuleHandle(skCryptDec("kernel32.dll"));

	if (hModuleKernel32 == nullptr)
	{
#ifdef DEBUG
		printf("hModuleKernel32 == nullptr\n");
#endif
		return;
	}

	LPVOID pCreateFilePtr = GetProcAddress(hModuleKernel32, skCryptDec("CreateFileA"));
	LPVOID pWriteFilePtr = GetProcAddress(hModuleKernel32, skCryptDec("WriteFile"));
	LPVOID pCloseHandlePtr = GetProcAddress(hModuleKernel32, skCryptDec("CloseHandle"));

	m_szMailSlotSendName = skCryptDec("\\\\.\\mailslot\\KOF2\\") + std::to_string(m_Bot->GetInjectedProcessId());
	std::vector<BYTE> vecMailSlotName(m_szMailSlotSendName.begin(), m_szMailSlotSendName.end());

	if (m_hMailSlotSend == nullptr)
	{
		m_hMailSlotSend = CreateMailslotA(m_szMailSlotSendName.c_str(), 0, MAILSLOT_WAIT_FOREVER, NULL);

		if (m_hMailSlotSend == INVALID_HANDLE_VALUE)
		{
#ifdef DEBUG
			printf("CreateMailslot send failed with %d\n", GetLastError());
#endif
			return;
		}
	}

	LPVOID pMailSlotNameAddress = VirtualAllocEx(hProcess, nullptr, vecMailSlotName.size(), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (pMailSlotNameAddress == 0)
	{
		return;
	}

	m_Bot->WriteBytes((DWORD)pMailSlotNameAddress, vecMailSlotName);

	if (m_SendHookAddress == 0)
	{
		BYTE byHookPatch[] =
		{
			0x55,
			0x8B, 0xEC,
			0x83, 0xC4, 0xF4,
			0x33, 0xC0,
			0x89, 0x45, 0xFC,
			0x33, 0xD2,
			0x89, 0x55, 0xF8,
			0x6A, 0x00,
			0x68, 0x80, 0x00, 0x00, 0x00,
			0x6A, 0x03,
			0x6A, 0x00,
			0x6A, 0x01,
			0x68, 0x00, 0x00, 0x00, 0x40,
			0x68, 0x00, 0x00, 0x00, 0x00,
			0xE8, 0x00, 0x00, 0x00, 0x00,
			0x89, 0x45, 0xF8,
			0x6A, 0x00,
			0x8D, 0x4D, 0xFC,
			0x51,
			0xFF, 0x75, 0x0C,
			0xFF, 0x75, 0x08,
			0xFF, 0x75, 0xF8,
			0xE8, 0x00, 0x00, 0x00, 0x00,
			0x89, 0x45, 0xF4,
			0xFF, 0x75, 0xF8,
			0xE8, 0x00, 0x00, 0x00, 0x00,
			0x8B, 0xE5,
			0x5D,
			0xC3
};

		m_SendHookAddress = VirtualAllocEx(hProcess, nullptr, sizeof(byHookPatch), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

		if (m_SendHookAddress == 0)
		{
			return;
		}

		CopyBytes(byHookPatch + 35, pMailSlotNameAddress);

		DWORD iCreateFileDifference = Memory::GetDifference((DWORD)m_SendHookAddress + 39, (DWORD)pCreateFilePtr);
		CopyBytes(byHookPatch + 40, iCreateFileDifference);

		DWORD iWriteFileDifference = Memory::GetDifference((DWORD)m_SendHookAddress + 62, (DWORD)pWriteFilePtr);
		CopyBytes(byHookPatch + 63, iWriteFileDifference);

		DWORD iCloseHandlePtrDifference = Memory::GetDifference((DWORD)m_SendHookAddress + 73, (DWORD)pCloseHandlePtr);
		CopyBytes(byHookPatch + 74, iCloseHandlePtrDifference);

		std::vector<BYTE> vecHookPatch(byHookPatch, byHookPatch + sizeof(byHookPatch));
		m_Bot->WriteBytes((DWORD)m_SendHookAddress, vecHookPatch);
	}

	BYTE byPatch1[] =
	{
		0x55,										//push ebp
		0x8B, 0xEC,									//mov ebp,esp 
		0x60,										//pushad
		0xFF, 0x75, 0x0C,							//push [ebp+0C]
		0xFF, 0x75, 0x08,							//push [ebp+08]
		0xBA, 0x00, 0x00, 0x00, 0x00,				//mov edx,00000000 <-- ClientHook::SendProcess()
		0xFF, 0xD2,									//call edx
		0x5E,										//pop esi
		0x5D,										//pop ebp
		0x61,										//popad
		0x6A, 0xFF,									//push-01
		0xBA, 0x00, 0x00, 0x00, 0x00,				//mov edx,00000000 <-- KO_SND_FNC
		0x83, 0xC2, 0x5,							//add edx,05
		0xFF, 0xE2									//jmp edx
	};

	CopyBytes(byPatch1 + 11, m_SendHookAddress);

	DWORD iKoPtrSndFnc = m_Bot->GetAddress(skCryptDec("KO_SND_FNC"));
	CopyBytes(byPatch1 + 23, iKoPtrSndFnc);

	std::vector<BYTE> vecPatch1(byPatch1, byPatch1 + sizeof(byPatch1));

	LPVOID pPatchAddress = VirtualAllocEx(hProcess, nullptr, vecPatch1.size(), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (pPatchAddress == 0)
	{
		return;
	}

	m_Bot->WriteBytes((DWORD)pPatchAddress, vecPatch1);

	BYTE byPatch2[] =
	{
		0xE9, 0x00, 0x00, 0x00, 0x00
	};

	DWORD iCallDifference = Memory::GetDifference(m_Bot->GetAddress(skCryptDec("KO_SND_FNC")), (DWORD)pPatchAddress);
	CopyBytes(byPatch2 + 1, iCallDifference);

	std::vector<BYTE> vecPatch2(byPatch2, byPatch2 + sizeof(byPatch2));

	DWORD iOldProtection;
	VirtualProtectEx(hProcess, (LPVOID)m_Bot->GetAddress(skCryptDec("KO_SND_FNC")), 1, PAGE_EXECUTE_READWRITE, &iOldProtection);
	m_Bot->WriteBytes(m_Bot->GetAddress(skCryptDec("KO_SND_FNC")), vecPatch2);
	VirtualProtectEx(hProcess, (LPVOID)m_Bot->GetAddress(skCryptDec("KO_SND_FNC")), 1, iOldProtection, &iOldProtection);

#ifdef DEBUG
	printf("PatchSendAddress: 0x%x patched\n", m_Bot->GetAddress("KO_SND_FNC"));
#endif
}

void ClientHandler::MailSlotRecvProcess()
{
	try
	{
		DWORD iCurrentMesageSize, iMesageLeft, iMessageReadSize;

		while (true)
		{
			BOOL bResult = GetMailslotInfo(m_hMailSlotRecv, NULL, &iCurrentMesageSize, &iMesageLeft, NULL);

			if (!bResult)
				return;

			if (iCurrentMesageSize == MAILSLOT_NO_MESSAGE)
				break;

			std::vector<uint8_t> vecMessageBuffer;

			vecMessageBuffer.resize(iCurrentMesageSize);

			OVERLAPPED ov{};
			ov.Offset = 0;
			ov.OffsetHigh = 0;
			ov.hEvent = NULL;

			bResult = ReadFile(m_hMailSlotRecv, &vecMessageBuffer[0], iCurrentMesageSize, &iMessageReadSize, &ov);

			if (!bResult)
				return;

			vecMessageBuffer.resize(iMessageReadSize);

			onClientRecvProcess(vecMessageBuffer.data(), vecMessageBuffer.size());
		}
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("MailSlotRecvProcess: Exception: %s\n", e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}
}

void ClientHandler::MailSlotSendProcess()
{
	try
	{
		DWORD iCurrentMesageSize, iMesageLeft, iMessageReadSize;

		while (true)
		{
			BOOL bResult = GetMailslotInfo(m_hMailSlotSend, NULL, &iCurrentMesageSize, &iMesageLeft, NULL);

			if (!bResult)
				return;

			if (iCurrentMesageSize == MAILSLOT_NO_MESSAGE)
				break;

			std::vector<uint8_t> vecMessageBuffer;

			vecMessageBuffer.resize(iCurrentMesageSize);

			OVERLAPPED ov{};
			ov.Offset = 0;
			ov.OffsetHigh = 0;
			ov.hEvent = NULL;

			bResult = ReadFile(m_hMailSlotSend, &vecMessageBuffer[0], iCurrentMesageSize, &iMessageReadSize, &ov);

			if (!bResult)
				return;

			vecMessageBuffer.resize(iMessageReadSize);

			onClientSendProcess(vecMessageBuffer.data(), vecMessageBuffer.size());
		}
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("MailSlotSendProcess: Exception: %s\n", e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}
}

void ClientHandler::RecvProcess(BYTE* byBuffer, DWORD iLength)
{
	Packet pkt = Packet(byBuffer[0], (size_t)iLength);
	pkt.append(&byBuffer[1], iLength - 1);

#ifdef DEBUG
#ifdef PRINT_RECV_PACKET
	printf("RecvProcess: %s\n", pkt.convertToHex().c_str());
#endif
#endif

	uint8_t iHeader;

	pkt >> iHeader;

	switch (iHeader)
	{
		case LS_LOGIN_REQ:
		{
			int16_t sUnknown;
			int8_t byResult;

			pkt >> sUnknown >> byResult;

			switch (byResult)
			{
				case AUTH_BANNED:
				{
#ifdef DEBUG
					printf("RecvProcess::LS_LOGIN_REQ: Account Banned\n");
#endif
				}
				break;

				case AUTH_IN_GAME:
				{
#ifdef DEBUG
					printf("RecvProcess::LS_LOGIN_REQ: Account already in-game\n");
#endif
					if (m_Bot->GetPlatformType() == PlatformType::CNKO)
					{
						new std::thread([this]()
						{
#ifdef DEBUG
							printf("RecvProcess::LS_LOGIN_REQ: Reconnecting login server\n");
#endif
							std::this_thread::sleep_for(std::chrono::milliseconds(500));

							WriteLoginInformation(m_szAccountId, m_szPassword);
							std::this_thread::sleep_for(std::chrono::milliseconds(100));
							ConnectLoginServer(true);
						});
					}
				}
				break;

				case AUTH_SUCCESS:
				{
#ifdef DEBUG
					printf("RecvProcess::LS_LOGIN_REQ: Login Success\n");
#endif
				}
				break;

				case AUTH_OTP:
				{
#ifdef DEBUG
					printf("RecvProcess::LS_LOGIN_REQ: OTP Validation\n");
#endif
					if (m_Bot->m_hModuleAnyOTP != NULL  
						&& m_Bot->m_bAutoLogin
						&& m_Bot->m_szID.size() > 0 && m_Bot->m_szPassword.size() > 0
						&& m_Bot->m_szAnyOTPPassword.size() > 0)
					{
						std::string szHashedPassword = Password::PasswordHash(m_szPassword);
						std::string szCode = to_string(m_Bot->ReadAnyOTPCode(m_Bot->m_szAnyOTPPassword, m_Bot->m_szAnyOTPID).c_str());

						if (szCode != skCryptDec("000000"))
						{
							SendOTPPacket(m_szAccountId, szHashedPassword, szCode);
							m_iOTPRetryCount++;
						}
						else
						{
#ifdef DEBUG
							printf("RecvProcess::LS_OTP: Otp code generation failed! Response: 000000\n");
#endif
						}
					}
				}
				break;

				default:
				{
#ifdef DEBUG
					printf("RecvProcess::LS_LOGIN_REQ: %d not implemented!\n", byResult);
#endif
				}
				break;
			}
		}
		break;

		case LS_OTP:
		{
			int8_t iResult;
			int8_t iUnknown;

			pkt >> iResult >> iUnknown;

			switch (iResult)
			{
				case 1:
				{
#ifdef DEBUG
					printf("RecvProcess::LS_OTP: OTP Success\n");
#endif
				}
				break;

				case 2:
				{
#ifdef DEBUG
					printf("RecvProcess::LS_OTP: OTP Failed, Retry Count: %d\n", m_iOTPRetryCount);
#endif

					if (m_iOTPRetryCount <= 3)
					{
#ifdef DEBUG
						printf("RecvProcess::LS_OTP: Retrying OTP\n");
#endif

						if (m_Bot->m_hModuleAnyOTP != NULL
							&& m_Bot->m_bAutoLogin
							&& m_Bot->m_szID.size() > 0 && m_Bot->m_szPassword.size() > 0
							&& m_Bot->m_szAnyOTPPassword.size() > 0)
						{
							std::string szHashedPassword = Password::PasswordHash(m_szPassword);
							std::string szCode = to_string(m_Bot->ReadAnyOTPCode(m_Bot->m_szAnyOTPPassword, m_Bot->m_szAnyOTPID).c_str());

							if (szCode != skCryptDec("000000"))
							{
								SendOTPPacket(m_szAccountId, szHashedPassword, szCode);
								m_iOTPRetryCount++;
							}
							else
							{
#ifdef DEBUG
								printf("RecvProcess::LS_OTP: Otp code generation failed! Response: 000000\n");
#endif
							}
						}
					}
				}
				break;

				case 3:
				{
#ifdef DEBUG
					printf("RecvProcess::LS_OTP: OTP code cannot be reused\n");
#endif

					if (m_Bot->m_hModuleAnyOTP != NULL
						&& m_Bot->m_bAutoLogin
						&& m_Bot->m_szID.size() > 0 && m_Bot->m_szPassword.size() > 0
						&& m_Bot->m_szAnyOTPPassword.size() > 0)
					{
						new std::thread([this]()
						{
							std::this_thread::sleep_for(std::chrono::milliseconds(60000));
							std::string szHashedPassword = Password::PasswordHash(m_szPassword);
							std::string szCode = to_string(m_Bot->ReadAnyOTPCode(m_Bot->m_szAnyOTPPassword, m_Bot->m_szAnyOTPID).c_str());

							if (szCode != skCryptDec("000000"))
							{
								SendOTPPacket(m_szAccountId, szHashedPassword, szCode);
							}
							else
							{
#ifdef DEBUG
								printf("RecvProcess::LS_OTP: Otp code generation failed! Response: 000000\n");
#endif
							}

						});
					}
				}
				break;

				case 10:
				{
#ifdef DEBUG
					printf("OTP Login failed with unknown issue. Please Contact customer service\n");
#endif
				}
				break;

				default:
				{
#ifdef DEBUG
					printf("RecvProcess::LS_OTP: %d not implemented!\n", iResult);
#endif
				}
				break;
			}
		}
		break;

		case LS_SERVERLIST:
		{
			int8_t byServerCount;

			pkt >> byServerCount;

			if (byServerCount > 0)
			{
#ifdef DEBUG
				printf("RecvProcess::LS_SERVERLIST: %d Server loaded\n", byServerCount);
#endif

				if (m_Bot->m_bAutoLogin
					&& m_Bot->m_szID.size() > 0 && m_Bot->m_szPassword.size() > 0)
				{
					new std::thread([this]()
					{
						if (m_Bot->GetPlatformType() != PlatformType::CNKO)
						{
							LoadServerList();
						}

						SelectServer((uint8_t)m_Bot->m_iServerID);

						if (m_Bot->GetPlatformType() != PlatformType::CNKO)
						{
							ShowChannel();
							SelectChannel((uint8_t)m_Bot->m_iChannelID);
						}

						std::this_thread::sleep_for(std::chrono::milliseconds(100));

						ConnectServer();
					});
				}

			}
		}
		break;

		case WIZ_ALLCHAR_INFO_REQ:
		{
			int8_t byResult;

			pkt >> byResult;

			bool bLoaded =
				pkt[pkt.size() - 3] == 0x0C &&
				pkt[pkt.size() - 2] == 0x02 &&
				pkt[pkt.size() - 1] == 0x00;

			if (bLoaded)
			{
#ifdef DEBUG
				printf("RecvProcess::WIZ_ALLCHAR_INFO_REQ: Character list loaded\n");
#endif

				if (m_Bot->m_bAutoLogin
					&& m_Bot->m_szID.size() > 0 && m_Bot->m_szPassword.size() > 0)
				{

					new std::thread([this]()
					{
#ifdef DEBUG
						printf("RecvProcess::WIZ_ALLCHAR_INFO_REQ: Selecting character %d\n", 1);
#endif

						SelectCharacterSkip();

						if (m_Bot->m_iSlotID - 1 > 0)
						{
							for (int32_t i = 0; i < m_Bot->m_iSlotID - 1; i++)
							{
								std::this_thread::sleep_for(std::chrono::milliseconds(100));
								SelectCharacterLeft();
								std::this_thread::sleep_for(std::chrono::milliseconds(100));
								SelectCharacterSkip();

							}
						}

						std::this_thread::sleep_for(std::chrono::milliseconds(100));

						SelectCharacter();
					});
				}
			}
		}
		break;

		case WIZ_SEL_CHAR:
		{
			int iRet = pkt.read<uint8_t>();


			if (iRet == 1)
			{
				m_PlayerMySelf.iCity = pkt.read<uint8_t>();

				m_PlayerMySelf.fX = (pkt.read<uint16_t>()) / 10.0f;
				m_PlayerMySelf.fY = (pkt.read<uint16_t>()) / 10.0f;
				m_PlayerMySelf.fZ = (pkt.read<int16_t>()) / 10.0f;

				uint8_t iVictoryNation = pkt.read<uint8_t>();

#ifdef DEBUG
				printf("RecvProcess::WIZ_SEL_CHAR Zone: [%d] Coordinate: [%f - %f - %f] VictoryNation: [%d]\n", 
					m_PlayerMySelf.iCity, m_PlayerMySelf.fX, m_PlayerMySelf.fY, m_PlayerMySelf.fZ, iVictoryNation);
#endif
			}
		}
		break;

		case WIZ_MYINFO:
		{
			pkt.SByte();

			m_PlayerMySelf.iID = pkt.read<int32_t>();

			pkt.readString(m_PlayerMySelf.szName);

			m_PlayerMySelf.fX = (pkt.read<uint16_t>()) / 10.0f;
			m_PlayerMySelf.fY = (pkt.read<uint16_t>()) / 10.0f;
			m_PlayerMySelf.fZ = (pkt.read<int16_t>()) / 10.0f;

			m_PlayerMySelf.eNation = (Nation)pkt.read<uint8_t>();
			m_PlayerMySelf.eRace = (Race)pkt.read<uint8_t>();
			m_PlayerMySelf.eClass = (Class)pkt.read<int16_t>();

			m_PlayerMySelf.iFace = pkt.read<uint8_t>();
			m_PlayerMySelf.iHair = pkt.read<int32_t>();
			m_PlayerMySelf.iRank = pkt.read<uint8_t>();
			m_PlayerMySelf.iTitle = pkt.read<uint8_t>();

			m_PlayerMySelf.iUnknown1 = pkt.read<uint8_t>();
			m_PlayerMySelf.iUnknown2 = pkt.read<uint8_t>();

			m_PlayerMySelf.iLevel = pkt.read<uint8_t>();

			m_PlayerMySelf.iBonusPointRemain = pkt.read<uint16_t>();
			m_PlayerMySelf.iExpNext = pkt.read<uint64_t>();
			m_PlayerMySelf.iExp = pkt.read<uint64_t>();

			m_PlayerMySelf.iRealmPoint = pkt.read<uint32_t>();
			m_PlayerMySelf.iRealmPointMonthly = pkt.read<uint32_t>();

			m_PlayerMySelf.iKnightsID = pkt.read<int16_t>();
			m_PlayerMySelf.eKnightsDuty = (KnightsDuty)pkt.read<uint8_t>();

			int16_t iAllianceID = pkt.read<int16_t>();
			uint8_t byFlag = pkt.read<uint8_t>();

			pkt.readString(m_PlayerMySelf.szKnights);

			m_PlayerMySelf.iKnightsGrade = pkt.read<uint8_t>();
			m_PlayerMySelf.iKnightsRank = pkt.read<uint8_t>();

			int16_t sMarkVersion = pkt.read<int16_t>();
			int16_t sCapeID = pkt.read<int16_t>();

			uint8_t iR = pkt.read<uint8_t>();
			uint8_t iG = pkt.read<uint8_t>();
			uint8_t iB = pkt.read<uint8_t>();

			uint8_t iUnknown1 = pkt.read<uint8_t>();

			uint32_t iUnknown2 = pkt.read<uint32_t>();
			uint32_t iUnknown3 = pkt.read<uint32_t>();

			m_PlayerMySelf.iHPMax = pkt.read<int16_t>();
			m_PlayerMySelf.iHP = pkt.read<int16_t>();

			m_PlayerMySelf.iMSPMax = pkt.read<int16_t>();
			m_PlayerMySelf.iMSP = pkt.read<int16_t>();

			m_PlayerMySelf.iWeightMax = pkt.read<uint32_t>();
			m_PlayerMySelf.iWeight = pkt.read<uint32_t>();

			m_PlayerMySelf.iStrength = pkt.read<uint8_t>();
			m_PlayerMySelf.iStrength_Delta = pkt.read<uint8_t>();
			m_PlayerMySelf.iStamina = pkt.read<uint8_t>();
			m_PlayerMySelf.iStamina_Delta = pkt.read<uint8_t>();
			m_PlayerMySelf.iDexterity = pkt.read<uint8_t>();
			m_PlayerMySelf.iDexterity_Delta = pkt.read<uint8_t>();
			m_PlayerMySelf.iIntelligence = pkt.read<uint8_t>();
			m_PlayerMySelf.iIntelligence_Delta = pkt.read<uint8_t>();
			m_PlayerMySelf.iMagicAttak = pkt.read<uint8_t>();
			m_PlayerMySelf.iMagicAttak_Delta = pkt.read<uint8_t>();

			m_PlayerMySelf.iAttack = pkt.read<int16_t>();
			m_PlayerMySelf.iGuard = pkt.read<int16_t>();

			m_PlayerMySelf.iRegistFire = pkt.read<uint8_t>();
			m_PlayerMySelf.iRegistCold = pkt.read<uint8_t>();
			m_PlayerMySelf.iRegistLight = pkt.read<uint8_t>();
			m_PlayerMySelf.iRegistMagic = pkt.read<uint8_t>();
			m_PlayerMySelf.iRegistCurse = pkt.read<uint8_t>();
			m_PlayerMySelf.iRegistPoison = pkt.read<uint8_t>();

			m_PlayerMySelf.iGold = pkt.read<uint32_t>();
			m_PlayerMySelf.iAuthority = pkt.read<uint8_t>();

			uint8_t bUserRank = pkt.read<uint8_t>();
			uint8_t bPersonalRank = pkt.read<uint8_t>();

			for (int i = 0; i < 9; i++)
			{
				m_PlayerMySelf.iSkillInfo[i] = pkt.read<uint8_t>();
			}

			for (int i = 0; i < INVENTORY_TOTAL; i++)
			{
				m_PlayerMySelf.tInventory[i].iPos = i;
				m_PlayerMySelf.tInventory[i].iItemID = pkt.read<uint32_t>();
				m_PlayerMySelf.tInventory[i].iDurability = pkt.read<uint16_t>();
				m_PlayerMySelf.tInventory[i].iCount = pkt.read<uint16_t>();
				m_PlayerMySelf.tInventory[i].iFlag = pkt.read<uint8_t>();
				m_PlayerMySelf.tInventory[i].iRentalTime = pkt.read<int16_t>();
				m_PlayerMySelf.tInventory[i].iSerial = pkt.read<uint32_t>();
				m_PlayerMySelf.tInventory[i].iExpirationTime = pkt.read<uint32_t>();
			}

			m_Bot->SendLoadUserConfiguration(GetServerId(), m_PlayerMySelf.szName);

#ifdef DEBUG
			printf("RecvProcess::WIZ_MYINFO: %s loaded\n", m_PlayerMySelf.szName.c_str());
#endif
		}
		break;

		case WIZ_LEVEL_CHANGE:
		{
			int32_t iID = pkt.read<int32_t>();
			uint8_t iLevel = pkt.read<uint8_t>();

			if (m_PlayerMySelf.iID == iID)
			{
				m_PlayerMySelf.iLevel = iLevel;
				m_PlayerMySelf.iBonusPointRemain = pkt.read<uint16_t>();
				m_PlayerMySelf.iSkillInfo[0] = pkt.read<uint8_t>();
				m_PlayerMySelf.iExpNext = pkt.read<int64_t>();
				m_PlayerMySelf.iExp = pkt.read<int64_t>();
				m_PlayerMySelf.iHPMax = pkt.read<int16_t>();
				m_PlayerMySelf.iHP = pkt.read<int16_t>();
				m_PlayerMySelf.iMSPMax = pkt.read<int16_t>();
				m_PlayerMySelf.iMSP = pkt.read<int16_t>();
				m_PlayerMySelf.iWeightMax = pkt.read<uint32_t>();
				m_PlayerMySelf.iWeight = pkt.read<uint32_t>();

				if (iLevel <= 10)
				{
					LoadSkillData();
				}

#ifdef DEBUG
				printf("RecvProcess::WIZ_LEVEL_CHANGE: %s %d\n", m_PlayerMySelf.szName.c_str(), iLevel);
#endif
			}
			else
			{
				auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
					[iID](const TPlayer& a) { return a.iID == iID; });

				if (it != m_vecPlayer.end())
				{
					it->iLevel = iLevel;

#ifdef DEBUG
					printf("RecvProcess::WIZ_LEVEL_CHANGE: %s %d\n", it->szName.c_str(), iLevel);
#endif
				}
			}
		}
		break;

		case WIZ_SKILLPT_CHANGE:
		{
			int iType = pkt.read<uint8_t>();
			int iValue = pkt.read<uint8_t>();

			m_PlayerMySelf.iSkillInfo[iType] = (uint8_t)iValue;
			m_PlayerMySelf.iSkillInfo[0]++;

#ifdef DEBUG
			printf("RecvProcess::WIZ_SKILLPT_CHANGE: %d,%d,%d\n", iType, iValue, m_PlayerMySelf.iSkillInfo[0]);
#endif
		}
		break;

		case WIZ_CLASS_CHANGE:
		{
			uint8_t	iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case N3_SP_CLASS_CHANGE_PURE:
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_CLASS_CHANGE: N3_SP_CLASS_CHANGE_PURE\n");
#endif
				}
				break;

				case N3_SP_CLASS_CHANGE_REQ:
				{
					SubPacketClassChange eSP = (SubPacketClassChange)pkt.read<uint8_t>();

					switch (eSP)
					{
						case N3_SP_CLASS_CHANGE_SUCCESS:
						{
#ifdef DEBUG
							printf("RecvProcess::WIZ_CLASS_CHANGE: N3_SP_CLASS_CHANGE_SUCCESS\n");
#endif
						}
						break;

						case N3_SP_CLASS_CHANGE_NOT_YET:
						{
#ifdef DEBUG
							printf("RecvProcess::WIZ_CLASS_CHANGE: N3_SP_CLASS_CHANGE_NOT_YET\n");
#endif
						}
						break;

						case N3_SP_CLASS_CHANGE_ALREADY:
						{
#ifdef DEBUG
							printf("RecvProcess::WIZ_CLASS_CHANGE: N3_SP_CLASS_CHANGE_ALREADY\n");
#endif
						}
						break;

						case N3_SP_CLASS_CHANGE_FAILURE:
						{
#ifdef DEBUG
							printf("RecvProcess::WIZ_CLASS_CHANGE: N3_SP_CLASS_CHANGE_FAILURE\n");
#endif
						}
						break;
					}
				}
				break;

				case N3_SP_CLASS_ALL_POINT:
				{
					uint8_t	iType = pkt.read<uint8_t>();
					uint32_t iGold = pkt.read<uint32_t>();

					switch (iType)
					{
						case 0x00: 
						{
						}
						break;

						case 0x01:
						{
							m_vecAttackSkillList = GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("AttackSkillList"), std::unordered_set<int>());
							m_vecCharacterSkillList = GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("CharacterSkillList"), std::unordered_set<int>());
							LoadSkillData();
						}
						break;

						case 0x02: 
						{
						}
						break;
					}
				}
				break;

				case N3_SP_CLASS_SKILL_POINT:
				{
					uint8_t	iType = pkt.read<uint8_t>();
					uint32_t iGold = pkt.read<uint32_t>();

					switch (iType)
					{
						case 0x00: 
						{
						}
						break;

						case 0x01:
						{
							m_vecAttackSkillList = GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("AttackSkillList"), std::unordered_set<int>());
							m_vecCharacterSkillList = GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("CharacterSkillList"), std::unordered_set<int>());
							LoadSkillData();
						}
						break;

						case 0x02: 
						{
						}
						break;
					}
				}
				break;

				case N3_SP_CLASS_POINT_CHANGE_PRICE_QUERY:
				{
					uint32_t iGold = pkt.read<uint32_t>();
#ifdef DEBUG
					printf("RecvProcess::WIZ_CLASS_CHANGE: Point change price %d\n", iGold);
#endif
				}
				break;

				case N3_SP_CLASS_PROMOTION:
				{
					uint16_t iClass = pkt.read<uint16_t>();
					uint32_t iID = pkt.read<uint32_t>();

					if (m_PlayerMySelf.iID == iID)
					{
						m_PlayerMySelf.eClass = (Class)iClass;

						m_vecAttackSkillList = GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("AttackSkillList"), std::unordered_set<int>());
						m_vecCharacterSkillList = GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("CharacterSkillList"), std::unordered_set<int>());

						LoadSkillData();
					}					
				}
				break;
			}
		}
		break;

		case WIZ_GAMESTART:
		{
#ifdef DEBUG
			printf("RecvProcess::WIZ_GAMESTART: Started\n");
#endif
			new std::thread([this]()
			{
				WaitCondition(GetUserConfiguration() == nullptr)
				WaitCondition(GetUserConfiguration()->GetConfigMap()->size() == 0)

				LoadSkillData();

				m_iStartCoin = m_PlayerMySelf.iGold;

				SetAuthority(m_bWallHack ? 0 : 1);
				PatchObjectCollision(m_bLegalWallHack);
				PatchDeathEffect(m_bDeathEffect);
				UpdateSkillSuccessRate(m_bDisableCasting);
				HidePlayer(m_bHidePlayer);

				if (m_bCharacterSizeEnable)
				{
					DWORD iMyBase = Drawing::Bot->Read4Byte(Drawing::Bot->GetAddress(skCryptDec("KO_PTR_CHR")));
					SetScale(iMyBase, (float)m_iCharacterSize, (float)m_iCharacterSize, (float)m_iCharacterSize);
				}

				m_bAttackStatus = GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Attack"), 0);
				m_bCharacterStatus = GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Character"), 0);

				if (m_bSaveCPUEnable)
				{
					SetSaveCPUSleepTime(m_iSaveCPUValue);
				}

				if (m_bSpeedHack)
				{
					SetCharacterSpeed(1.5);
					PatchSpeedHack(true);
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(5000));

				StartHandler();
			});
		}
		break;

		case WIZ_REQ_NPCIN:
		{
			int16_t iNpcCount = pkt.read<int16_t>();

			if (iNpcCount > 0)
			{
				for (int16_t i = 0; i < iNpcCount; i++)
				{
					auto pNpc = InitializeNpc(pkt);

					int32_t iNpcID = pNpc.iID;
					
					auto it = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
						[&](const TNpc& a) { return a.iID == iNpcID; });

					if (it == m_vecNpc.end())
						m_vecNpc.push_back(pNpc);
					else
						*it = pNpc;
				}
			}

#ifdef DEBUG
			printf("RecvProcess::WIZ_REQ_NPCIN: Size %d\n", iNpcCount);
#endif
		}
		break;

		case WIZ_NPC_INOUT:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case InOut::INOUT_IN:
				{
					auto pNpc = InitializeNpc(pkt);

					auto it = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
						[pNpc](const TNpc& a) { return a.iID == pNpc.iID; });

					if (it == m_vecNpc.end())
					{
						m_vecNpc.push_back(pNpc);
					}
					else
						*it = pNpc;

#ifdef DEBUG
					printf("RecvProcess::WIZ_NPC_INOUT: INOUT_IN - %d,%d\n", iType, pNpc.iID);
#endif
				}
				break;

				case InOut::INOUT_OUT:
				{
					int32_t iNpcID = pkt.read<int32_t>();

					m_vecNpc.erase(
						std::remove_if(m_vecNpc.begin(), m_vecNpc.end(),
							[&](const TNpc& a) { return a.iID == iNpcID; }),
						m_vecNpc.end());

#ifdef DEBUG
					printf("RecvProcess::WIZ_NPC_INOUT: INOUT_OUT - %d,%d\n", iType, iNpcID);
#endif
				}
				break;

				default:
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_NPC_INOUT: %d not implemented\n", iType);
#endif
				}
				break;
			}
		}
		break;

		case WIZ_NPC_REGION:
		{
			int16_t iNpcCount = pkt.read<int16_t>();

			if (iNpcCount > 0)
			{
				std::vector<int32_t> vecRegionNpcList;

				for (int16_t i = 0; i < iNpcCount; i++)
				{
					int32_t iNpcID = pkt.read<int32_t>();
					vecRegionNpcList.push_back(iNpcID);
				}

				for (const auto& pNpc : m_vecNpc)
				{
					if (std::find(vecRegionNpcList.begin(), vecRegionNpcList.end(), pNpc.iID) == vecRegionNpcList.end())
					{
						m_vecNpc.erase(
							std::remove_if(m_vecNpc.begin(), m_vecNpc.end(),
								[&](const TNpc& a) { return a.iID == pNpc.iID; }),
							m_vecNpc.end());
					}
				}

#ifdef DEBUG
				printf("RecvProcess::WIZ_NPC_REGION: Npc region change completed! Region list size %d, current npc list %d\n", vecRegionNpcList.size(), m_vecNpc.size());
#endif
			}
		}
		break;

		case WIZ_REQ_USERIN:
		{
			int16_t iUserCount = pkt.read<int16_t>();

			if (iUserCount > 0)
			{
				for (int16_t i = 0; i < iUserCount; i++)
				{
					uint8_t iUnknown0 = pkt.read<uint8_t>();

					auto pPlayer = InitializePlayer(pkt);

					auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
						[&](const TPlayer& a) { return a.iID == pPlayer.iID; });

					if (it == m_vecPlayer.end())
						m_vecPlayer.push_back(pPlayer);
					else
						*it = pPlayer;

					/*bool bSendTownIfThereIsGMNearby = GetUserConfiguration()->GetBool(skCryptDec("Settings"), skCryptDec("bSendTownIfThereIsGMNearby"), false);
					bool bAttackStatus = GetUserConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Attack"), false);
					bool bCharacterStatus = GetUserConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Character"), false);

					if ((bAttackStatus || bCharacterStatus)
						&& bSendTownIfThereIsGMNearby && pPlayer.iAuthority == 0)
					{
						SendTownPacket();
#ifdef DEBUG
						printf("RecvProcess::WIZ_REQ_USERIN: !! GM !!, Gamemaster: %s\n", pPlayer.szName.c_str());
#endif

					}*/
				}

			}

#ifdef DEBUG
			printf("RecvProcess::WIZ_REQ_USERIN: Size %d\n", iUserCount);
#endif
		}
		break;

		case WIZ_USER_INOUT:
		{
			uint8_t iType = pkt.read<uint8_t>();
			uint8_t iUnknown0 = pkt.read<uint8_t>();

			switch (iType)
			{
			case InOut::INOUT_IN:
			case InOut::INOUT_RESPAWN:
			case InOut::INOUT_WARP:
			{
				auto pPlayer = InitializePlayer(pkt);

				auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
					[&](const TPlayer& a) { return a.iID == pPlayer.iID; });

				if (it == m_vecPlayer.end())
					m_vecPlayer.push_back(pPlayer);
				else
					*it = pPlayer;

				/*bool bSendTownIfThereIsGMNearby = GetUserConfiguration()->GetBool(skCryptDec("Settings"), skCryptDec("bSendTownIfThereIsGMNearby"), false);
				bool bAttackStatus = GetUserConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Attack"), false);
				bool bCharacterStatus = GetUserConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Character"), false);

				if ((bAttackStatus || bCharacterStatus) 
					&& bSendTownIfThereIsGMNearby && pPlayer.iAuthority == 0)
				{
					SendTownPacket();
#ifdef DEBUG
					printf("RecvProcess::WIZ_USER_INOUT: !! GM !!, Gamemaster: %s\n", pPlayer.szName.c_str());
#endif
				}*/
#ifdef DEBUG
				printf("RecvProcess::WIZ_USER_INOUT: INOUT_IN | INOUT_RESPAWN | INOUT_WARP - %d,%d\n", iType, pPlayer.iID);
#endif
			}
			break;

			case InOut::INOUT_OUT:
			{
				int32_t iPlayerID = pkt.read<int32_t>();

				m_vecPlayer.erase(
					std::remove_if(m_vecPlayer.begin(), m_vecPlayer.end(),
						[&](const TPlayer& a) { return a.iID == iPlayerID; }),
					m_vecPlayer.end());

#ifdef DEBUG
				printf("RecvProcess::WIZ_USER_INOUT: INOUT_OUT - %d,%d\n", iType, iPlayerID);
#endif
			}
			break;

			default:
			{
#ifdef DEBUG
				printf("RecvProcess::WIZ_USER_INOUT: %d not implemented\n", iType);
#endif
			}

			break;
			}
		}
		break;

		case WIZ_REGIONCHANGE:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case 0:
				{
					m_vecRegionUserList.clear();

#ifdef DEBUG
					printf("RecvProcess::WIZ_REGIONCHANGE: Region change activity start, current user count: %d\n", m_vecPlayer.size());
#endif
				}
				break;

				case 1:
				{
					int16_t iUserCount = pkt.read<int16_t>();

					for (int16_t i = 0; i < iUserCount; i++)
					{
						int32_t iUserID = pkt.read<int32_t>();

						if (iUserID == m_PlayerMySelf.iID)
							continue;

						m_vecRegionUserList.insert(iUserID);
					}

#ifdef DEBUG
					printf("RecvProcess::WIZ_REGIONCHANGE: Region change activity user count: %d\n", m_vecRegionUserList.size());
#endif
				}
				break;

				case 2:
				{			
					std::vector<int> indicesToRemove;

					for (size_t i = 0; i < m_vecPlayer.size(); i++)
					{
						const auto& pPlayer = m_vecPlayer[i];

						if (std::find(m_vecRegionUserList.begin(), m_vecRegionUserList.end(), pPlayer.iID) == m_vecRegionUserList.end())
							indicesToRemove.push_back(i);
					}

					for (int i = indicesToRemove.size() - 1; i >= 0; i--)
					{
						m_vecPlayer.erase(m_vecPlayer.begin() + indicesToRemove[i]);
					}

#ifdef DEBUG
					printf("RecvProcess::WIZ_REGIONCHANGE: Region change activity end region user count: %d, current user list: %d\n", m_vecRegionUserList.size(), m_vecPlayer.size());
#endif

					m_vecRegionUserList.clear();
				}
				break;

				default:
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_REGIONCHANGE: Type %d not implemented! [%s]\n", iType, pkt.convertToHex().c_str());
#endif
				}
				break;
			}
		}
		break;

		case WIZ_DEAD:
		{
			int32_t iID = pkt.read<int32_t>();

			if (m_PlayerMySelf.iID == iID)
			{
				m_PlayerMySelf.eState = PSA_DEATH;

				if (m_bStopBotIfDead)
				{
					m_bAttackStatus = GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Attack"), 0);
					m_bCharacterStatus = GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Character"), 0);
				}

				auto pMember = std::find_if(m_vecPartyMembers.begin(), m_vecPartyMembers.end(),
					[&](const PartyMember& a)
					{
						return a.iMemberID == iID;
					});

				if (pMember != m_vecPartyMembers.end())
				{
					pMember->fHpBuffTime = 0.0f;
					pMember->iHpBuffAttemptCount = 0;
					pMember->fACBuffTime = 0.0f;
					pMember->iACBuffAttemptCount = 0;
					pMember->fMindBuffTime = 0.0f;
					pMember->iMindBuffAttemptCount = 0;
				}
#ifdef DEBUG
				printf("RecvProcess::WIZ_DEAD: MySelf Dead\n");
#endif
			}
			else
			{
				if (iID >= 5000)
				{
					auto it = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
						[&](const TNpc& a) { return a.iID == iID; });

					if (it != m_vecNpc.end())
					{
						it->eState = PSA_DEATH;

						m_vecNpc.erase(
							std::remove_if(m_vecNpc.begin(), m_vecNpc.end(),
								[&](const TNpc& a) { return a.iID == it->iID; }),
							m_vecNpc.end());

#ifdef DEBUG
						printf("RecvProcess::WIZ_DEAD: %d Npc Dead\n", iID);
#endif
					}
				}
				else
				{
					auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
						[&](const TPlayer& a) { return a.iID == iID; });

					if (it != m_vecPlayer.end())
					{
						it->eState = PSA_DEATH;

						auto pMember = std::find_if(m_vecPartyMembers.begin(), m_vecPartyMembers.end(),
							[&](const PartyMember& a)
							{
								return a.iMemberID == iID;
							});

						if (pMember != m_vecPartyMembers.end())
						{
							pMember->fHpBuffTime = 0.0f;
							pMember->iHpBuffAttemptCount = 0;
							pMember->fACBuffTime = 0.0f;
							pMember->iACBuffAttemptCount = 0;
							pMember->fMindBuffTime = 0.0f;
							pMember->iMindBuffAttemptCount = 0;
						}

#ifdef DEBUG
						printf("RecvProcess::WIZ_DEAD: %d Player Dead\n", iID);
#endif
					}
				}
			}
		}
		break;

		case WIZ_ATTACK:
		{
			uint8_t iType = pkt.read<uint8_t>();
			uint8_t iResult = pkt.read<uint8_t>();

			switch (iResult)
			{
				case ATTACK_FAIL:
				{
					int32_t iAttackID = pkt.read<int32_t>();
					int32_t iTargetID = pkt.read<int32_t>();

	#ifdef DEBUG
					printf("RecvProcess::WIZ_ATTACK: %d,%d FAIL\n", iAttackID, iTargetID);
	#endif
				}
				break;

				case ATTACK_SUCCESS:
				{
					int32_t iAttackID = pkt.read<int32_t>();
					int32_t iTargetID = pkt.read<int32_t>();

					if (m_bPartyLeaderSelect 
						&& m_vecPartyMembers.size() > 0
						&& iAttackID != m_PlayerMySelf.iID
						&& iAttackID < 5000
						&& iTargetID >= 5000)
					{
						auto pMember = std::find_if(m_vecPartyMembers.begin(), m_vecPartyMembers.end(),
							[&](const PartyMember& a)
							{
								return a.iMemberID == iAttackID;
							});

						if (pMember != m_vecPartyMembers.end())
						{
							if (pMember->iIndex == 100)
							{
								SetTarget(iTargetID);
							}
						}
					}

	#ifdef DEBUG
					printf("RecvProcess::WIZ_ATTACK: %d,%d SUCCESS\n", iAttackID, iTargetID);
	#endif
				}
				break;

				case ATTACK_TARGET_DEAD:
				case ATTACK_TARGET_DEAD_OK:
				{
					int32_t iAttackID = pkt.read<int32_t>();
					int32_t iTargetID = pkt.read<int32_t>();

					if (iTargetID >= 5000)
					{
						auto it = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
							[&](const TNpc& a) { return a.iID == iTargetID; });

						if (it != m_vecNpc.end())
						{
							it->eState = PSA_DEATH;
	#ifdef DEBUG
							printf("RecvProcess::WIZ_ATTACK: TARGET_DEAD | TARGET_DEAD_OK - %d Npc Dead\n", iTargetID);
	#endif
						}
					}
					else
					{
						auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
							[&](const TPlayer& a) { return a.iID == iTargetID; });

						if (it != m_vecPlayer.end())
						{
							it->eState = PSA_DEATH;

#ifdef DEBUG
							printf("RecvProcess::WIZ_ATTACK: TARGET_DEAD | TARGET_DEAD_OK - %d Player Dead\n", iTargetID);
#endif
						}
					}
				}
				break;

				default:
				{
	#ifdef DEBUG
					printf("RecvProcess::WIZ_ATTACK: %d not implemented\n", iResult);
	#endif
				}
				break;
			}
		}
		break;

		case WIZ_TARGET_HP:
		{
			int32_t iID = pkt.read<int32_t>();
			uint8_t iUpdateImmediately = pkt.read<uint8_t>();

			int32_t iTargetHPMax = pkt.read<int32_t>();
			int32_t iTargetHPCur = pkt.read<int32_t>();

			int16_t iTargetHPChange = pkt.read<int16_t>();

			if (m_PlayerMySelf.iID == iID)
			{
				m_PlayerMySelf.iHPMax = iTargetHPMax;
				m_PlayerMySelf.iHP = iTargetHPCur;

				//if (/*m_PlayerMySelf.iHPMax > 0 && */m_PlayerMySelf.iHP <= 0)
					//m_PlayerMySelf.eState = PSA_DEATH;

#ifdef DEBUG
				printf("RecvProcess::WIZ_TARGET_HP: %s, %d / %d\n",
					m_PlayerMySelf.szName.c_str(), m_PlayerMySelf.iHP, m_PlayerMySelf.iHPMax);
#endif
			}
			else
			{
				if (iID >= 5000)
				{
					auto it = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
						[&](const TNpc& a) { return a.iID == iID; });

					if (it != m_vecNpc.end())
					{
						it->iHPMax = iTargetHPMax;
						it->iHP = iTargetHPCur;

						//if (/*it->iHPMax > 0 && */it->iHP <= 0)
							//it->eState = PSA_DEATH;

#ifdef DEBUG
						printf("RecvProcess::WIZ_TARGET_HP: %d, %d / %d\n", iID, it->iHP, it->iHPMax);
#endif
					}
				}
				else
				{
					auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
						[iID](const TPlayer& a) { return a.iID == iID; });

					if (it != m_vecPlayer.end())
					{
						it->iHPMax = iTargetHPMax;
						it->iHP = iTargetHPCur;

						//if (it->iHPMax > 0 && it->iHP <= 0)
							//it->eState = PSA_DEATH;

#ifdef DEBUG
						printf("RecvProcess::WIZ_TARGET_HP: %s, %d / %d\n",
							it->szName.c_str(), it->iHP, it->iHPMax);
#endif
					}
				}
			}
		}
		break;

		case WIZ_MOVE:
		{
			int32_t iID = pkt.read<int32_t>();

			float fX = pkt.read<uint16_t>() / 10.0f;
			float fY = pkt.read<uint16_t>() / 10.0f;
			float fZ = pkt.read<int16_t>() / 10.0f;

			int16_t iSpeed = pkt.read<int16_t>();
			uint8_t iMoveType = pkt.read<uint8_t>();

			if (m_PlayerMySelf.iID == iID)
			{
				m_PlayerMySelf.fX = fX;
				m_PlayerMySelf.fY = fY;
				m_PlayerMySelf.fZ = fZ;

				m_PlayerMySelf.iMoveSpeed = iSpeed;
				m_PlayerMySelf.iMoveType = iMoveType;
			}
			else
			{
				auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
					[&](const TPlayer& a) { return a.iID == iID; });

				if (it != m_vecPlayer.end())
				{
					it->fX = fX;
					it->fY = fY;
					it->fZ = fZ;

					it->iMoveSpeed = iSpeed;
					it->iMoveType = iMoveType;
				}
#ifdef DEBUG
				else
					printf("RecvProcess::WIZ_MOVE: %d not in m_vecPlayer list, is ghost player\n", iID);
#endif
			}
		}
		break;

		case WIZ_NPC_MOVE:
		{
			uint8_t iMoveType = pkt.read<uint8_t>();

			int32_t iID = pkt.read<int32_t>();

			float fX = pkt.read<uint16_t>() / 10.0f;
			float fY = pkt.read<uint16_t>() / 10.0f;
			float fZ = pkt.read<int16_t>() / 10.0f;

			uint16_t iSpeed = pkt.read<uint16_t>();

			auto it = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
				[&](const TNpc& a) { return a.iID == iID; });

			if (it != m_vecNpc.end())
			{
				it->fX = fX;
				it->fY = fY;
				it->fZ = fZ;

				it->iMoveSpeed = iSpeed;
				it->iMoveType = iMoveType;
			}
#ifdef DEBUG
			else
				printf("RecvProcess::WIZ_NPC_MOVE: %d not in m_vecNpc list, is ghost npc\n", iID);
#endif

		}
		break;

		case WIZ_MAGIC_PROCESS:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case SkillMagicType::SKILL_MAGIC_TYPE_FLYING:
				{
					uint32_t iSkillID = pkt.read<uint32_t>();
					int32_t iSourceID = pkt.read<int32_t>();
					int32_t iTargetID = pkt.read<int32_t>();
				}
				break;

				case SkillMagicType::SKILL_MAGIC_TYPE_EFFECTING:
				{
					uint32_t iSkillID = pkt.read<uint32_t>();
					int32_t iSourceID = pkt.read<int32_t>();
					int32_t iTargetID = pkt.read<int32_t>();

					if (iSourceID == m_PlayerMySelf.iID)
					{
						if (m_vecPartyMembers.size() > 0 && (IsPriest() || IsRogue()))
						{
							auto pPartyMember = std::find_if(m_vecPartyMembers.begin(), m_vecPartyMembers.end(),
								[&](const PartyMember& a)
								{
									return a.iMemberID == iTargetID;
								});

							if (pPartyMember != m_vecPartyMembers.end())
							{
								auto it = std::find_if(m_vecAvailableSkill.begin(), m_vecAvailableSkill.end(),
									[&](const TABLE_UPC_SKILL& a) { return a.iID == iSkillID; });

								if (it != m_vecAvailableSkill.end())
								{
									switch (it->iBaseId)
									{
									case 111654: case 112675: case 112670: case 111657:
									case 111655: case 111642: case 111633: case 111624:
									case 111615: case 111606:
									{
										pPartyMember->fHpBuffTime = Bot::TimeGet();
										pPartyMember->iHpBuffAttemptCount = 0;
									}
									break;

									case 112674: case 111660: case 111651: case 111639:
									case 111630: case 111621: case 111612: case 111603:
									{
										pPartyMember->fACBuffTime = Bot::TimeGet();
										pPartyMember->iACBuffAttemptCount = 0;
									}
									break;

									case 111645: case 111636: case 111627: case 111609:
									{
										pPartyMember->fMindBuffTime = Bot::TimeGet();
										pPartyMember->iMindBuffAttemptCount = 0;
									}
									break;

									case 107010:
									{
									}
									break;
									}
								}
							}
						}
					}

					if (m_bPartyLeaderSelect
						&& m_vecPartyMembers.size() > 0
						&& iSourceID != m_PlayerMySelf.iID
						&& iSourceID < 5000
						&& iTargetID >= 5000)
					{
						auto pMember = std::find_if(m_vecPartyMembers.begin(), m_vecPartyMembers.end(),
							[&](const PartyMember& a)
							{
								return a.iMemberID == iSourceID;
							});

						if (pMember != m_vecPartyMembers.end())
						{
							if (pMember->iIndex == 100)
							{
								SetTarget(iTargetID);
							}
						}
					}
				}
				break;

				case SkillMagicType::SKILL_MAGIC_TYPE_FAIL:
				{
					uint32_t iSkillID = pkt.read<uint32_t>();
					int32_t iSourceID = pkt.read<int32_t>();
					int32_t iTargetID = pkt.read<int32_t>();

					int32_t iData[6];
					memset(iData, 0, sizeof(iData));

					for (size_t i = 0; i < 6; i++)
						iData[i] = pkt.read<int32_t>();

					if (iData[3] == SKILLMAGIC_FAIL_CASTING 
						|| iData[3] == SKILLMAGIC_FAIL_NOEFFECT
						|| iData[3] == SKILLMAGIC_FAIL_ATTACKZERO
						|| iData[3] == SKILLMAGIC_FAIL_UNKNOWN)
					{
#ifdef DEBUG
						printf("RecvProcess::WIZ_MAGIC_PROCESS: %d - %d - Skill failed %d\n", iSourceID, iSkillID, iData[3]);
#endif
						if (iSourceID == m_PlayerMySelf.iID)
						{
							Client::SetSkillNextUseTime(iSkillID, 0.0f);

							if (m_vecPartyMembers.size() > 0 && (IsPriest() || IsRogue()))
							{
								auto pPartyMember = std::find_if(m_vecPartyMembers.begin(), m_vecPartyMembers.end(),
									[&](const PartyMember& a)
									{
										return a.iMemberID == iTargetID;
									});

								if (pPartyMember != m_vecPartyMembers.end())
								{
									auto it = std::find_if(m_vecAvailableSkill.begin(), m_vecAvailableSkill.end(),
										[&](const TABLE_UPC_SKILL& a) { return a.iID == iSkillID; });

									if (it != m_vecAvailableSkill.end())
									{
										switch (it->iBaseId)
										{
										case 111654: case 112675: case 112670: case 111657:
										case 111655: case 111642: case 111633: case 111624:
										case 111615: case 111606:
										{
											if (iData[3] == SKILLMAGIC_FAIL_CASTING)
											{
												pPartyMember->fHpBuffTime = 0.0f;
												pPartyMember->iHpBuffAttemptCount = 0;
											}
											else
											{
												if (pPartyMember->iHpBuffAttemptCount < 2)
												{
													pPartyMember->iHpBuffAttemptCount++;
												}
												else
												{
													pPartyMember->fHpBuffTime = Bot::TimeGet();
												}
											}
										}
										break;

										case 112674: case 111660: case 111651: case 111639:
										case 111630: case 111621: case 111612: case 111603:
										{
											if (iData[3] == SKILLMAGIC_FAIL_CASTING)
											{
												pPartyMember->fACBuffTime = 0;
												pPartyMember->iACBuffAttemptCount = 0;
											}
											else
											{
												if (pPartyMember->iACBuffAttemptCount < 2)
												{
													pPartyMember->iACBuffAttemptCount++;
												}
												else
												{
													pPartyMember->fACBuffTime = Bot::TimeGet();
												}
											}
										}
										break;

										case 111645: case 111636: case 111627: case 111609:
										{
											if (iData[3] == SKILLMAGIC_FAIL_CASTING)
											{
												pPartyMember->fMindBuffTime = 0;
												pPartyMember->iMindBuffAttemptCount = 0;
											}
											else
											{
												if (pPartyMember->iMindBuffAttemptCount < 2)
												{
													pPartyMember->iMindBuffAttemptCount++;
												}
												else
												{
													pPartyMember->fMindBuffTime = Bot::TimeGet();
												}
											}
										}
										break;

										case 107010:
										{
										}
										break;
										}
									}
								}
							}
						}
						
					}
				}
				break;

				case SkillMagicType::SKILL_MAGIC_TYPE_BUFF:
				{
					uint8_t iBuffType = pkt.read<uint8_t>();
				}
				break;

				default:
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_MAGIC_PROCESS: %d Type Not Implemented!\n", iType);
#endif
				}
				break;
			}
		}
		break;

		case WIZ_STATE_CHANGE:
		{
			int32_t iID = pkt.read<int32_t>();
			uint8_t iType = pkt.read<uint8_t>();
			uint64_t iBuff = pkt.read<uint64_t>();

			switch (iType)
			{
				case N3_SP_STATE_CHANGE_SIZE:
				{
					switch (iBuff)
					{
						case ABNORMAL_NORMAL:
						{
							if (m_PlayerMySelf.iID == iID)
							{
	#ifdef DEBUG
								printf("RecvProcess::WIZ_STATE_CHANGE: %s - ABNORMAL_NORMAL\n", m_PlayerMySelf.szName.c_str());
	#endif

								m_PlayerMySelf.bBlinking = false;
							}
							else
							{
								auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
									[&](const TPlayer& a) { return a.iID == iID; });

								if (it != m_vecPlayer.end())
								{
#ifdef DEBUG
									printf("RecvProcess::WIZ_STATE_CHANGE: %s - ABNORMAL_NORMAL\n", it->szName.c_str());
#endif

									it->bBlinking = false;
								}
							}
						}
						break;

						case ABNORMAL_BLINKING:
						{
							if (m_PlayerMySelf.iID == iID)
							{
#ifdef DEBUG
								printf("RecvProcess::WIZ_STATE_CHANGE: %s - ABNORMAL_BLINKING\n", m_PlayerMySelf.szName.c_str());
#endif

								m_PlayerMySelf.bBlinking = true;
							}
							else
							{
								auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
									[&](const TPlayer& a) { return a.iID == iID; });

								if (it != m_vecPlayer.end())
								{
#ifdef DEBUG
									printf("RecvProcess::WIZ_STATE_CHANGE: %s - ABNORMAL_BLINKING\n", it->szName.c_str());
#endif

									it->bBlinking = true;
								}
							}
						}
						break;

						case ABNORMAL_BLINKING_END:
						{
							if (m_PlayerMySelf.iID == iID)
							{
#ifdef DEBUG
								printf("RecvProcess::WIZ_STATE_CHANGE: %s - ABNORMAL_BLINKING_END\n", m_PlayerMySelf.szName.c_str());
#endif

								m_PlayerMySelf.bBlinking = false;
							}
							else
							{		
								auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
									[&](const TPlayer& a) { return a.iID == iID; });

								if (it != m_vecPlayer.end())
								{
#ifdef DEBUG
									printf("RecvProcess::WIZ_STATE_CHANGE: %s - ABNORMAL_BLINKING_END\n", it->szName.c_str());
#endif

									it->bBlinking = false;
								}
							}
						}
						break;

						default:
						{
#ifdef DEBUG
							printf("RecvProcess::WIZ_STATE_CHANGE: Abnormal Type - %llu Buff not implemented\n", iBuff);
#endif
						}
						break;
					}
				}
				break;

				default:
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_STATE_CHANGE: Type %d not implemented\n", iType);
#endif
				}
				break;
			}
		}
		break;

		case WIZ_MAP_EVENT:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
			case 9:
			{
				m_bLunarWarDressUp = pkt.read<uint8_t>();
			}
			break;

			default:
			{
#ifdef DEBUG
				printf("RecvProcess::WIZ_MAP_EVENT: Type %d not implemented\n", iType);
#endif
			}
			break;
			}
		}
		break;

		case WIZ_EXCHANGE:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case TradeSubPacket::TRADE_DONE:
				{
					uint8_t iResult = pkt.read<uint8_t>();

					if (iResult == 1)
					{
						uint32_t iGold = pkt.read<uint32_t>();
						int16_t iItemCount = pkt.read<int16_t>();

#ifdef DEBUG
						printf("RecvProcess::WIZ_EXCHANGE: TRADE_DONE - Success - %d,%d \n", iGold, iItemCount);
#endif
					}
#ifdef DEBUG
					else
						printf("RecvProcess::WIZ_EXCHANGE: TRADE_DONE - Failed\n");
#endif
				}
				break;

				case TradeSubPacket::TRADE_CANCEL:
				{
				}
				break;

				default:
				{
	#ifdef DEBUG
					printf("RecvProcess::WIZ_EXCHANGE: Type %d not implemented\n", iType);
	#endif
				}
				break;
			}
		}
		break;

		case WIZ_ITEM_DROP:
		{
			TLoot tLoot;
			memset(&tLoot, 0, sizeof(tLoot));

			tLoot.iNpcID = pkt.read<int32_t>();
			tLoot.iBundleID = pkt.read<uint32_t>();
			tLoot.iItemCount = pkt.read<uint8_t>();

			if (tLoot.iItemCount == 0)
			{
#ifdef DEBUG
				printf("RecvProcess::WIZ_ITEM_DROP: Box empty!\n");
#endif
				if (IsMovingToLoot())
				{
					SetMovingToLoot(false);
				}

				return;
			}

			if (m_bMoveToLoot)
			{
				auto pNpc = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
					[&](const TNpc& a)
					{
						return a.iID == tLoot.iNpcID;
					});

				if (pNpc == m_vecNpc.end())
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_ITEM_DROP: Move to loot NPC Position not exist!\n");
#endif
					return;
				}

				tLoot.fNpcX = pNpc->fX;
				tLoot.fNpcZ = pNpc->fZ;
				tLoot.fNpcY = pNpc->fY;
			}

			tLoot.fDropTime = Bot::TimeGet();

			auto pLoot = std::find_if(m_vecLootList.begin(), m_vecLootList.end(),
				[&](const TLoot a) { return a.iBundleID == tLoot.iBundleID; });

			if (pLoot == m_vecLootList.end())
				m_vecLootList.push_back(tLoot);
			else
				*pLoot = tLoot;

#ifdef DEBUG
			printf("RecvProcess::WIZ_ITEM_DROP: %d,%d,%d,%f\n",
				tLoot.iNpcID,
				tLoot.iBundleID,
				tLoot.iItemCount,
				tLoot.fDropTime);
#endif
		}
		break;

		case WIZ_BUNDLE_OPEN_REQ:
		{
			uint32_t iBundleID = pkt.read<uint32_t>();
			uint8_t iResult = pkt.read<uint8_t>();

			switch (iResult)
			{
				case 0:
				{
	#ifdef DEBUG
					printf("RecvProcess::WIZ_BUNDLE_OPEN_REQ: Bundle open req failed\n");
	#endif
				}
				break;

				case 1:
				{
					if (m_bAutoLoot)
					{
						auto pLoot = std::find_if(m_vecLootList.begin(), m_vecLootList.end(),
							[&](const TLoot a) { return a.iBundleID == iBundleID; });

						if (pLoot != m_vecLootList.end())
						{
							for (size_t i = 0; i < pLoot->iItemCount; i++)
							{
								uint32_t iItemID = pkt.read<uint32_t>();
								uint32_t iItemCount = pkt.read<int16_t>();

								switch (m_iLootType)
								{
									// Loot All
									case 0:
									{
										if (iItemID == 900000000)
										{
											SendBundleItemGet(iBundleID, iItemID, (int16_t)i);
										}
										else
										{
											__TABLE_ITEM* pItemData;
											__TABLE_ITEM_EXTENSION* pItemExtensionData;
											if (m_Bot->GetItemData(iItemID, pItemData)
												&& m_Bot->GetItemExtensionData(iItemID, pItemData->byExtIndex, pItemExtensionData))
											{
												if (!m_bMinPriceLootEnable
													|| (m_bMinPriceLootEnable && pItemData->iPriceRepair >= m_iLootMinPrice)
													|| (pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UNIQUE
														|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_RED
														|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UPGRADE_REVERSE
														|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UNIQUE_REVERSE
														|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_PET
														|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_EVENT
														|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_COSPRE
														|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_MINERVA))
												{

													SendBundleItemGet(iBundleID, iItemID, (int16_t)i);
												}
											}
											else
											{
												SendBundleItemGet(iBundleID, iItemID, (int16_t)i);
											}
										}
									}
									break;

									// Loot Only Coin
									case 1:
									{
										if (iItemID == 900000000)
										{
											SendBundleItemGet(iBundleID, iItemID, (int16_t)i);
										}
									}
									break;

									// Loot Only Item List
									case 2:
									{
										if (iItemID == 900000000)
										{
											SendBundleItemGet(iBundleID, iItemID, (int16_t)i);
										}
										else
										{
											uint32_t iItemBaseID = iItemID / 1000 * 1000;
											bool bLootSelected = m_vecLootItemList.count(iItemBaseID) > 0;

											__TABLE_ITEM* pItemData;
											__TABLE_ITEM_EXTENSION* pItemExtensionData;
											if (m_Bot->GetItemData(iItemID, pItemData)
												&& m_Bot->GetItemExtensionData(iItemID, pItemData->byExtIndex, pItemExtensionData))
											{
												if (bLootSelected)
												{
													if (!m_bMinPriceLootEnable
														|| (m_bMinPriceLootEnable && pItemData->iPriceRepair >= m_iLootMinPrice)
														|| (pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UNIQUE
															|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_RED
															|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UPGRADE_REVERSE
															|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UNIQUE_REVERSE
															|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_PET
															|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_EVENT
															|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_COSPRE
															|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_MINERVA))
													{

														SendBundleItemGet(iBundleID, iItemID, (int16_t)i);
													}
												}
											}
											else
											{
												SendBundleItemGet(iBundleID, iItemID, (int16_t)i);
											}
										}
									}
									break;
								}
							}

							m_vecLootList.erase(
								std::remove_if(m_vecLootList.begin(), m_vecLootList.end(),
									[&](const TLoot& a) { return a.iBundleID == iBundleID; }),
								m_vecLootList.end());
						}
					}
				}
				break;

				case 2:
				{
					m_vecLootList.erase(
						std::remove_if(m_vecLootList.begin(), m_vecLootList.end(),
							[&](const TLoot& a) { return a.iBundleID == iBundleID; }),
						m_vecLootList.end());
				}
				break;

				default:
				{
	#ifdef DEBUG
					printf("RecvProcess::WIZ_BUNDLE_OPEN_REQ: Result %d not implemented\n", iResult);
	#endif
				}
				break;
			}
		}
		break;

		case WIZ_ITEM_GET:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case 0x00:
				{
					//uint32_t iBundleID = pkt.read<uint32_t>();

					//// Unknown1: -15, -7 = No space in the inventory for this item
					//int16_t Unknown1 = pkt.read<int16_t>();
					//int16_t Unknown2 = pkt.read<int16_t>();		

					//m_vecLootList.erase(
					//	std::remove_if(m_vecLootList.begin(), m_vecLootList.end(),
					//		[&](const TLoot& a) { return a.iBundleID == iBundleID; }),
					//	m_vecLootList.end());

//#ifdef DEBUG
//					printf("RecvProcess::WIZ_ITEM_GET: Failed - %d,%d,%d,%d\n", iType, iBundleID, Unknown1, Unknown2);
//#endif
				}
				break;

				case 0x01:
				case 0x02:
				case 0x05:
				{
					uint32_t iBundleID = pkt.read<uint32_t>();
					uint8_t iPos = pkt.read<uint8_t>();
					uint32_t iItemID = pkt.read<uint32_t>();

					uint16_t iItemCount = 0;

					if (iType == 1 || iType == 5)
						iItemCount = pkt.read<uint16_t>();

					uint32_t iGold = pkt.read<uint32_t>();

					if (iItemID == 900000000)
					{
						m_iCoinCounter += iItemCount;
					}

#ifdef DEBUG
					printf("RecvProcess::WIZ_ITEM_GET: Success - %d,%d,%d,%d,%d,%d\n", iType, iBundleID, iPos, iItemID, iItemCount, iGold);
#endif
				}
				break;

				case 0x03:
				{
					uint32_t iBundleID = pkt.read<uint32_t>();

#ifdef DEBUG
					printf("RecvProcess::WIZ_ITEM_GET: %d,%d\n", iType, iBundleID);
#endif
				}
				break;

				case 0x06:
				{
					uint32_t iBundleID = pkt.read<uint32_t>();

					m_vecLootList.erase(
						std::remove_if(m_vecLootList.begin(), m_vecLootList.end(),
							[&](const TLoot& a) { return a.iBundleID == iBundleID; }),
						m_vecLootList.end());

#ifdef DEBUG
					printf("RecvProcess::WIZ_ITEM_GET: Inventory Full\n");
#endif

				}
				break;


				default:
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_ITEM_GET: %d Type Not Implemented\n", iType);
#endif
				}
				break;
			}
		}
		break;

		case WIZ_PARTY:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case PartyUpdateType::PARTY_UPDATE_CREATE:
				{
					pkt.DByte();

					uint8_t iCommandType = pkt.read<uint8_t>();

					std::string szName;
					pkt.readString(szName);
				}
				break;

				case PartyUpdateType::PARTY_UPDATE_INSERT:
				{
					pkt.DByte();

					int32_t iMemberID = pkt.read<int32_t>();

					std::string szName;
					pkt.readString(szName);
				}
				break;

				case PartyUpdateType::PARTY_UPDATE_JOINED:
				{
					pkt.DByte();

					int16_t iJoinResponse = pkt.read<int16_t>();

					switch (iJoinResponse)
					{
						case -1:
						{
							if (m_vecPartyMembers.size() == 1)
							{
								m_vecPartyMembers.clear();
							}
						}
						break;
						case 1:
						{
							PartyMember pPartyMember;
							memset(&pPartyMember, 0, sizeof(pPartyMember));

							pPartyMember.iMemberID = pkt.read<int32_t>();
							pPartyMember.iIndex = pkt.read<uint8_t>();

							pkt.readString(pPartyMember.szName);

							pPartyMember.iMaxHP = pkt.read<uint16_t>();
							pPartyMember.iHP = pkt.read<uint16_t>();
							pPartyMember.iLevel = pkt.read<uint8_t>();
							pPartyMember.iClass = pkt.read<uint16_t>();
							pPartyMember.iMaxMP = pkt.read<uint16_t>();
							pPartyMember.iMP = pkt.read<uint16_t>();

							pPartyMember.iNation = pkt.read<uint8_t>();

							uint8_t iUnknown1 = pkt.read<uint8_t>();
							int32_t iUnknown2 = pkt.read<int32_t>();

							uint8_t iUnknown3 = pkt.read<uint8_t>();
							int8_t iUnknown4 = pkt.read<int8_t>();

							if (m_vecPartyMembers.size() > 0 
								&& pPartyMember.iIndex == 100)
							{
								auto pOldLeader = std::find_if(m_vecPartyMembers.begin(), m_vecPartyMembers.end(),
									[&](const PartyMember& a)
									{
										return a.iIndex == 100;
									});

								auto pNewLeader = std::find_if(m_vecPartyMembers.begin(), m_vecPartyMembers.end(),
									[&](const PartyMember& a)
									{
										return a.iMemberID == pPartyMember.iMemberID;
									});

								if (pOldLeader != m_vecPartyMembers.end() 
									&& pNewLeader != m_vecPartyMembers.end())
								{
									pOldLeader->iIndex = pNewLeader->iIndex;
								}

								if (pNewLeader != m_vecPartyMembers.end())
								{
									pNewLeader->iIndex = pPartyMember.iIndex;
								}
							}
							else
							{
								m_vecPartyMembers.push_back(pPartyMember);
							}	
						}
						break;
					}
				}
				break;

				case PartyUpdateType::PARTY_UPDATE_LEAVE:
				{
					int32_t iMemberID = pkt.read<int32_t>();

					m_vecPartyMembers.erase(
						std::remove_if(m_vecPartyMembers.begin(), m_vecPartyMembers.end(),
							[&](const PartyMember& a) { return a.iMemberID == iMemberID; }),
						m_vecPartyMembers.end());
				}
				break;

				case PartyUpdateType::PARTY_UPDATE_DISMISSED:
				{
					m_vecPartyMembers.clear();
				}
				break;

				case PartyUpdateType::PARTY_UPDATE_HEALTH_MANA_CHANGE:
				{
					int32_t iMemberID = pkt.read<int32_t>();

					auto pMember = std::find_if(m_vecPartyMembers.begin(), m_vecPartyMembers.end(),
						[&](const PartyMember& a)
						{
							return a.iMemberID == iMemberID;
						});

					if (pMember != m_vecPartyMembers.end())
					{
						pMember->iMaxHP = pkt.read<uint16_t>();
						pMember->iHP = pkt.read<uint16_t>();
						pMember->iMaxMP = pkt.read<uint16_t>();
						pMember->iMP = pkt.read<uint16_t>();
					}
				}
				break;

				case PartyUpdateType::PARTY_UPDATE_LEVEL_CHANGE:
				{
					int32_t iMemberID = pkt.read<int32_t>();

					auto pMember = std::find_if(m_vecPartyMembers.begin(), m_vecPartyMembers.end(),
						[&](const PartyMember& a)
						{
							return a.iMemberID == iMemberID;
						});

					if (pMember != m_vecPartyMembers.end())
					{
						pMember->iLevel = pkt.read<uint8_t>();
					}
				}
				break;

				case PartyUpdateType::PARTY_UPDATE_CLASS_CHANGE:
				{
					int32_t iMemberID = pkt.read<int32_t>();

					auto pMember = std::find_if(m_vecPartyMembers.begin(), m_vecPartyMembers.end(),
						[&](const PartyMember& a)
						{
							return a.iMemberID == iMemberID;
						});

					if (pMember != m_vecPartyMembers.end())
					{
						pMember->iClass = pkt.read<uint16_t>();
					}
				}
				break;

				case PartyUpdateType::PARTY_UPDATE_STATUS_CHANGE:
				{
					// TODO: !!!
				}
				break;
			}
		}
		break;

		case WIZ_ZONE_CHANGE:
		{
			uint8_t iOpCode = pkt.read<uint8_t>();

			switch (iOpCode)
			{
				case ZoneChangeOpcode::ZoneChangeLoading:
				{
				}
				break;

				case ZoneChangeOpcode::ZoneChangeLoaded:
				{
					SetZoneChange(false);
				}
				break;

				case ZoneChangeOpcode::ZoneChangeTeleport:
				{
					SetZoneChange(true);

					uint8_t iCity = (uint8_t)pkt.read<int16_t>();

					if (m_PlayerMySelf.iCity != iCity)
					{
						m_PlayerMySelf.iCity = iCity;

						m_mapSkillUseTime.clear();

						m_bIsMovingToLoot = false;

						m_vecLootList.clear();

						m_vecRouteActive.clear();
						m_vecRoutePlan.clear();

						m_fLastSupplyTime = 0;

						m_PlayerMySelf.bBlinking = false;

						m_bAttackStatus = GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Attack"), 0);
						m_bCharacterStatus = GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Character"), 0);
					}

					m_PlayerMySelf.fZ = (pkt.read<int16_t>() / 10.0f);
					m_PlayerMySelf.fX = (pkt.read<uint16_t>() / 10.0f);
					m_PlayerMySelf.fY = (pkt.read<uint16_t>() / 10.0f);

					uint8_t iVictoryNation = pkt.read<uint8_t>();

					m_vecNpc.clear();
					m_vecPlayer.clear();

#ifdef DEBUG
					printf("RecvProcess::WIZ_ZONE_CHANGE: Teleport Zone: [%d] Coordinate: [%f - %f - %f] VictoryNation: [%d]\n",
						m_PlayerMySelf.iCity, m_PlayerMySelf.fX, m_PlayerMySelf.fY, m_PlayerMySelf.fZ, iVictoryNation);
#endif
				}
				break;

				case ZoneChangeOpcode::ZoneChangeMilitaryCamp:
				{
					m_mapSkillUseTime.clear();

					m_bIsMovingToLoot = false;

					m_vecLootList.clear();

					m_vecRouteActive.clear();
					m_vecRoutePlan.clear();

					m_fLastSupplyTime = 0;

					m_vecNpc.clear();
					m_vecPlayer.clear();

					m_bAttackStatus = GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Attack"), 0);
					m_bCharacterStatus = GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Character"), 0);
				}
				break;
			}
		}
		break;

		case WIZ_WARP:
		{
#ifdef DEBUG
			printf("RecvProcess::WIZ_WARP\n");
#endif

			if (!IsRouting())
			{
				if (m_bTownOrTeleportStopBot)
				{
					m_bAttackStatus = GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Attack"), 0);
					m_bCharacterStatus = GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Character"), 0);
				}
			}

			SetMovePosition(Vector3(0.0f, 0.0f, 0.0f));

			m_PlayerMySelf.fX = (pkt.read<uint16_t>() / 10.0f);
			m_PlayerMySelf.fY = (pkt.read<uint16_t>() / 10.0f);
			m_PlayerMySelf.fZ = (pkt.read<int16_t>() / 10.0f);

			m_vecNpc.clear();
			m_vecPlayer.clear();
			m_vecLootList.clear();
		}
		break;	

		case WIZ_CAPTCHA:
		{
			uint8_t iUnknown = pkt.read<uint8_t>();
			uint8_t iOpCode = pkt.read<uint8_t>();

			switch (iOpCode)
			{
				case 1:
				{
					uint8_t iResult = pkt.read<uint8_t>();

					if (iResult == 1)
					{
#ifdef DEBUG
						printf("RecvProcess::WIZ_CAPTCHA: Image loaded\n");
#endif
						int iBufferLength;
						pkt >> iBufferLength;

						std::vector<uint8_t> vecBuffer(iBufferLength);
						pkt.read(&vecBuffer[0], iBufferLength);

						m_Bot->SendCaptcha(vecBuffer);
					}
					else
					{
#ifdef DEBUG
						printf("RecvProcess::WIZ_CAPTCHA: Image loading failed(%d)\n", iResult);
#endif	
						new std::thread([&]()
						{
							std::this_thread::sleep_for(std::chrono::milliseconds(3000));

							RefreshCaptcha();
#ifdef DEBUG
							printf("RecvProcess::WIZ_CAPTCHA: Refreshed\n");
#endif
						});
					}
				}
				break;

				case 2:
				{
					uint8_t iResult = pkt.read<uint8_t>();

					if (iResult == 1)
					{
#ifdef DEBUG
						printf("RecvProcess::WIZ_CAPTCHA: Success\n");
#endif
					}
					else
					{	
#ifdef DEBUG
						printf("RecvProcess::WIZ_CAPTCHA: Failed(%d)\n", iResult);
#endif
						new std::thread([&]()
						{
							std::this_thread::sleep_for(std::chrono::milliseconds(3000));

							RefreshCaptcha();
#ifdef DEBUG
							printf("RecvProcess::WIZ_CAPTCHA: Refreshed\n");
#endif
						});
					}
				}
				break;

				default:
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_CAPTCHA: Not implemented opcode(%d)\n", iOpCode);
#endif
				}
				break;
			}
		}
		break;

		case WIZ_CHAT:
		{
			uint8_t iChatMode = pkt.read<uint8_t>();

			switch (iChatMode)
			{
				case GENERAL_CHAT:
				case PRIVATE_CHAT:
				case PARTY_CHAT:
				{
					uint8_t iNation = pkt.read<uint8_t>();
					int32_t iSenderID = pkt.read<int32_t>();

					pkt.SByte();
					std::string szSenderName;
					pkt.readString(szSenderName);

					pkt.DByte();
					std::string szMessage;
					pkt.readString(szMessage);

					uint8_t iRank = pkt.read<uint8_t>();
					uint8_t iAuthority = pkt.read<uint8_t>();

					if (iSenderID != m_PlayerMySelf.iID)
					{
						if (m_bPartyRequest
							&& szMessage.rfind(m_szPartyRequestMessage.c_str(), 0) == 0)
						{
							if (m_vecPartyMembers.size() == 0)
								SendPartyCreate(szSenderName);
							else
								SendPartyInsert(szSenderName);
						}

						if (IsMage()
							&& m_vecPartyMembers.size() > 0
							&& m_bTeleportRequest
							&& szMessage.rfind(m_szTeleportRequestMessage.c_str(), 0) == 0)
						{
							auto pMember = std::find_if(m_vecPartyMembers.begin(), m_vecPartyMembers.end(),
								[&](const PartyMember& a)
								{
									return a.iMemberID == iSenderID;
								});

							if (pMember != m_vecPartyMembers.end())
							{
								auto it = std::find_if(m_vecAvailableSkill.begin(), m_vecAvailableSkill.end(),
									[](const TABLE_UPC_SKILL& a) { return a.iBaseId == 109004; });

								if (it != m_vecAvailableSkill.end())
								{
									new std::thread([=]() { UseSkillWithPacket(*it, iSenderID); });
								}
							}
						}
					}
				}
				break;

				case PUBLIC_CHAT:
				case ANNOUNCEMENT_CHAT:
				{
					uint8_t iNation = pkt.read<uint8_t>();
					int32_t iSenderID = pkt.read<int32_t>();

					pkt.SByte();
					std::string szSenderName;
					pkt.readString(szSenderName);

					pkt.DByte();
					std::string szMessage;
					pkt.readString(szMessage);

					uint8_t iRank = pkt.read<uint8_t>();
					uint8_t iAuthority = pkt.read<uint8_t>();

					std::string searchString = skCryptDec("is currently blocked for using illegal software.");
					size_t found = szMessage.find(searchString);

					if (found != std::string::npos)
					{
#ifdef DEBUG
						printf("RecvProcess::WIZ_CHAT: !! GM !!, ban notification received! \n");

						if (m_bPlayBeepfIfBanNotice)
						{
							Beep(1000, 500);
							Beep(1000, 500);
							Beep(1000, 500);
						}

						if ((m_bAttackStatus || m_bCharacterStatus) && m_bSendTownIfBanNotice)
						{
							SendTownPacket();
						}
#endif
					}

						
#ifdef DEBUG
					printf("RecvProcess::WIZ_CHAT: PUBLIC_CHAT | ANNOUNCEMENT_CHAT : %s \n", szMessage.c_str());
#endif
				}
				break;

				default:
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_CHAT: Not implemented chatmode(%d)\n", iChatMode);
#endif
				}
				break;
			}
		}
		break;

		case WIZ_WAREHOUSE:
		{
			uint8_t iOpCode = pkt.read<uint8_t>();

			switch (iOpCode)
			{
				case 0x01:
				{
					uint8_t iUnknown = pkt.read<uint8_t>();
					uint32_t iCoins = pkt.read<uint32_t>();

					for (size_t i = 0; i < WAREHOUSE_MAX; i++)
					{
						TItemData* pItem = &m_PlayerMySelf.tWarehouse[i];

						pItem->iPos = i;
						pkt >> pItem->iItemID >> pItem->iDurability >> pItem->iCount >> pItem->iFlag >> pItem->iSerial >> pItem->iExpirationTime;
					}

					if (m_pCurrentRunningRoute != nullptr 
						&& m_pCurrentRunningRoute->eStepType == STEP_INN
						&& m_pCurrentRunningRoute->bSubProcessRequested)
					{
						if (m_vecInnItemList.size() > 0)
						{
							for (const TItemData& pItem : m_pCurrentRunningRoute->vecInventoryItemList)
							{
								if (pItem.iItemID == 0)
									continue;

								uint32_t iItemBaseID = pItem.iItemID / 1000 * 1000;
								bool bInnSelected = m_vecInnItemList.count(iItemBaseID) > 0;

								if (!bInnSelected)
									continue;

								uint8_t iContable = 0;

								__TABLE_ITEM* pItemData;

								if (m_Bot->GetItemData(pItem.iItemID, pItemData))
								{
									iContable = pItemData->byContable;
								}

								int iRealSlot = GetWarehouseAvailableSlot(pItem.iItemID, iContable);
								int iPage = iRealSlot / 24;
								int iSlot = iRealSlot % 24;

								SendWarehouseGetIn(m_pCurrentRunningRoute->pNpcID, pItem.iItemID, (uint8_t)iPage, pItem.iPos - 14, (uint8_t)iSlot, pItem.iCount);

								m_PlayerMySelf.tWarehouse[iRealSlot] = pItem;
								m_PlayerMySelf.tWarehouse[iRealSlot].iPos = iRealSlot;

								if (iContable)
								{
									m_PlayerMySelf.tWarehouse[iRealSlot].iCount += pItem.iCount;
								}
							}

							SendNpcEvent(m_pCurrentRunningRoute->pNpcID);
							SendShoppingMall(ShoppingMallType::STORE_CLOSE);
						}

						m_pCurrentRunningRoute->bSubProcessFinished = true;
					}
				}
				break;

				default:
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_WAREHOUSE: Not implemented opcode(%d)\n", iOpCode);
#endif
				}
				break;
			}
		}
		break;

		case WIZ_VIPWAREHOUSE:
		{
			uint8_t iOpCode = pkt.read<uint8_t>();

			switch (iOpCode)
			{
				case 0x01:
				{
					uint8_t iResult = pkt.read<uint8_t>();

					switch (iResult)
					{
						case 0x01:
						{
							memset(m_PlayerMySelf.tVipWarehouse, 0, sizeof(m_PlayerMySelf.tVipWarehouse));

							uint8_t iIsPasswordUsed = pkt.read<uint8_t>();
							uint8_t iUnknown2 = pkt.read<uint8_t>();
							uint32_t iExpirationTime = pkt.read<uint32_t>();
							uint32_t iVaultKey = pkt.read<uint32_t>();
							uint16_t iUnknown3 = pkt.read<uint16_t>();
							uint16_t iUnknown4 = pkt.read<uint16_t>();
							uint8_t iUnknown5 = pkt.read<uint8_t>();
							uint32_t iUnknown6 = pkt.read<uint32_t>();
							uint16_t iUnknown7 = pkt.read<uint16_t>();

							bool bIsFull = true;

							for (size_t i = 0; i < VIP_HAVE_MAX; i++)
							{
								TItemData* pItem = &m_PlayerMySelf.tVipWarehouse[i];

								pItem->iPos = i;

								if (pItem->iItemID == 0)
									bIsFull = false;

								pkt >> pItem->iItemID >> pItem->iDurability >> pItem->iCount >> pItem->iFlag >> pItem->iSerial >> pItem->iExpirationTime;
							}

							m_bVipWarehouseFull = bIsFull;

							m_bVipWarehouseInitialized = true;
							m_bVipWarehouseEnabled = true;
							m_bVipWarehouseLoaded = true;

#ifdef DEBUG
							printf("RecvProcess::WIZ_VIPWAREHOUSE: VIP Warehouse initialized\n");
#endif
						}
						break;

						case 0x15:
						{
							m_bVipWarehouseInitialized = true;
							m_bVipWarehouseEnabled = false;
							m_bVipWarehouseLoaded = false;

#ifdef DEBUG
							printf("RecvProcess::WIZ_VIPWAREHOUSE: VIP Key doesn't exist\n");
#endif
						}
						break;

						default:
						{
#ifdef DEBUG
							printf("RecvProcess::WIZ_VIPWAREHOUSE: Opcode(%d), Not implemented result(%d)\n", iOpCode, iResult);
#endif
						}
						break;
					}
				}
				break;

				default:
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_VIPWAREHOUSE: Not implemented opcode(%d)\n", iOpCode);
#endif
				}
				break;
			}
		}
		break;

		case WIZ_STORY:
		{
			uint8_t iOpCode = pkt.read<uint8_t>();

			switch (iOpCode)
			{
				case 4: // Flash Item Operation
				{
					uint8_t iResult = pkt.read<uint8_t>();

					switch (iResult)
					{
						case 1: //Use
						{
							m_iFlashCount++;
						}
						break;

						case 2: //Delete
						{
							m_iFlashCount = 0;
						}
						break;

						default:
						{
#ifdef DEBUG
							printf("RecvProcess::WIZ_STORY: Opcode(%d), Not implemented result(%d)\n", iOpCode, iResult);
#endif						
						}
						break;
					}

				}
				break;

				default:
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_STORY: Not implemented opcode(%d)\n", iOpCode);
#endif
					break;
				}
			}
		}
		break;

		case WIZ_GENIE:
		{
			uint8_t iOpCode = pkt.read<uint8_t>();

			switch (iOpCode)
			{
				//Genie Information
				case 0x01: 
				{
					uint8_t iState = pkt.read<uint8_t>();

					switch (iState)
					{
						//Genie Start
						case 0x04: 
						{
							if (!IsRouting())
							{
								if (m_bSyncWithGenie && !m_bStartGenieIfUserInRegion)
								{
									m_bAttackStatus = GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Attack"), 1);
									m_bCharacterStatus = GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Character"), 1);
								}
							}
						}
						break;

						//Genie Stop
						case 0x05: 
						{
							if (!IsRouting())
							{
								if (m_bSyncWithGenie && !m_bStartGenieIfUserInRegion)
								{
									m_bAttackStatus = GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Attack"), 0);
									m_bCharacterStatus = GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Character"), 0);
								}
							}
						}
						break;
					}
				}
				break;
			}

		}
		break;

		/*case WIZ_COMPRESS_PACKET:
		{
			uint32_t compressedLength = pkt.read<uint32_t>();
			uint32_t originalLength = pkt.read<uint32_t>();
			uint32_t crc = pkt.read<uint32_t>();

			uint8_t* decompressedBuffer = Compression::DecompressWithCRC32(pkt.contents() + pkt.rpos(), compressedLength, originalLength, crc);

			if (decompressedBuffer != nullptr)
			{
				RecvProcess(decompressedBuffer, originalLength);
			}
		}
		break;*/

		case WIZ_REGENE:
		{
			m_PlayerMySelf.fX = (pkt.read<uint16_t>() / 10.0f);
			m_PlayerMySelf.fY = (pkt.read<uint16_t>() / 10.0f);
			m_PlayerMySelf.fZ = (pkt.read<int16_t>() / 10.0f);

			m_PlayerMySelf.eState = PSA_BASIC;
		}
		break;

		case WIZ_HP_CHANGE:
		{
			m_PlayerMySelf.iHPMax = pkt.read<uint16_t>();
			m_PlayerMySelf.iHP = pkt.read<uint16_t>();
		}
		break;

		case WIZ_MSP_CHANGE:
		{
			m_PlayerMySelf.iMSPMax = pkt.read<uint16_t>();
			m_PlayerMySelf.iMSP = pkt.read<uint16_t>();
		}
		break;

		case WIZ_ITEM_MOVE:
		{
			uint8_t iType = pkt.read<uint8_t>();

			if (iType != 0)
			{
				uint8_t iSubType = pkt.read<uint8_t>();

				if (iSubType != 0)
				{
					m_PlayerMySelf.iAttack = pkt.read<int16_t>();
					m_PlayerMySelf.iGuard = pkt.read<int16_t>();
					m_PlayerMySelf.iWeightMax = pkt.read<uint32_t>();

					uint16_t iUnknown1 = pkt.read<uint16_t>();

					m_PlayerMySelf.iHPMax = pkt.read<int16_t>();
					m_PlayerMySelf.iMSPMax = pkt.read<int16_t>();

					m_PlayerMySelf.iStrength_Delta = (uint8_t)pkt.read<uint16_t>();
					m_PlayerMySelf.iStamina_Delta = (uint8_t)pkt.read<uint16_t>();
					m_PlayerMySelf.iDexterity_Delta = (uint8_t)pkt.read<uint16_t>();
					m_PlayerMySelf.iIntelligence_Delta = (uint8_t)pkt.read<uint16_t>();
					m_PlayerMySelf.iMagicAttak_Delta = (uint8_t)pkt.read<uint16_t>();

					m_PlayerMySelf.iRegistFire = (uint8_t)pkt.read<uint16_t>();
					m_PlayerMySelf.iRegistCold = (uint8_t)pkt.read<uint16_t>();
					m_PlayerMySelf.iRegistLight = (uint8_t)pkt.read<uint16_t>();
					m_PlayerMySelf.iRegistMagic = (uint8_t)pkt.read<uint16_t>();
					m_PlayerMySelf.iRegistCurse = (uint8_t)pkt.read<uint16_t>();
					m_PlayerMySelf.iRegistPoison = (uint8_t)pkt.read<uint16_t>();

#ifdef DEBUG
					printf("RecvProcess::WIZ_ITEM_MOVE: %d,%d,%d,Unknown1(%d),%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
						m_PlayerMySelf.iAttack,
						m_PlayerMySelf.iGuard,
						m_PlayerMySelf.iWeightMax,
						iUnknown1,
						m_PlayerMySelf.iHPMax,
						m_PlayerMySelf.iMSPMax,
						m_PlayerMySelf.iStrength_Delta,
						m_PlayerMySelf.iStamina_Delta,
						m_PlayerMySelf.iDexterity_Delta,
						m_PlayerMySelf.iIntelligence_Delta,
						m_PlayerMySelf.iMagicAttak_Delta,
						m_PlayerMySelf.iRegistFire,
						m_PlayerMySelf.iRegistCold,
						m_PlayerMySelf.iRegistLight,
						m_PlayerMySelf.iRegistMagic,
						m_PlayerMySelf.iRegistCurse,
						m_PlayerMySelf.iRegistPoison);
#endif
				}
				else
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_ITEM_MOVE: %d SubType Not Implemented\n", iSubType);
#endif
				}
			}
			else
			{
#ifdef DEBUG
				printf("RecvProcess::WIZ_ITEM_MOVE: %d Type Not Implemented\n", iType);
#endif
			}
		}
		break;

		case WIZ_WEIGHT_CHANGE:
		{
			m_PlayerMySelf.iWeight = pkt.read<uint32_t>();
#ifdef DEBUG
			printf("RecvProcess::WIZ_WEIGHT_CHANGE: %d\n", m_PlayerMySelf.iWeight);
#endif
		}
		break;

		case WIZ_DURATION:
		{
			uint8_t iPos = pkt.read<uint8_t>();
			uint16_t iDurability = pkt.read<uint16_t>();

			m_PlayerMySelf.tInventory[iPos].iDurability = iDurability;

#ifdef DEBUG
			printf("RecvProcess::WIZ_DURATION: %d,%d\n", iPos, iDurability);
#endif
		}
		break;

		case WIZ_POINT_CHANGE:
		{
			uint8_t iType = pkt.read<uint8_t>();
			int16_t iVal = pkt.read<int16_t>();

			m_PlayerMySelf.iHPMax = pkt.read<int16_t>();
			m_PlayerMySelf.iMSPMax = pkt.read<int16_t>();
			m_PlayerMySelf.iAttack = pkt.read<int16_t>();
			m_PlayerMySelf.iWeightMax = pkt.read<uint32_t>();

			switch (iType)
			{
			case 0x01:
			{
				m_PlayerMySelf.iStrength = (uint8_t)iVal;
#ifdef DEBUG
				printf("RecvProcess::WIZ_POINT_CHANGE: STR %d\n", iVal);
#endif
			}
			break;

			case 0x02:
			{
				m_PlayerMySelf.iStamina = (uint8_t)iVal;
#ifdef DEBUG
				printf("RecvProcess::WIZ_POINT_CHANGE: HP %d\n", iVal);
#endif
			}
			break;

			case 0x03:
			{
				m_PlayerMySelf.iDexterity = (uint8_t)iVal;
#ifdef DEBUG
				printf("RecvProcess::WIZ_POINT_CHANGE: DEX %d\n", iVal);
#endif
			}
			break;

			case 0x04:
			{
				m_PlayerMySelf.iIntelligence = (uint8_t)iVal;
#ifdef DEBUG
				printf("RecvProcess::WIZ_POINT_CHANGE: INT %d\n", iVal);
#endif
			}
			break;

			case 0x05:
			{
				m_PlayerMySelf.iMagicAttak = (uint8_t)iVal;
#ifdef DEBUG
				printf("RecvProcess::WIZ_POINT_CHANGE: MP %d\n", iVal);
#endif
			}
			break;
			}

			if (iType >= 1 && iType <= 5)
			{
				m_PlayerMySelf.iBonusPointRemain--;
#ifdef DEBUG
				printf("RecvProcess::WIZ_POINT_CHANGE: POINT %d\n", m_PlayerMySelf.iBonusPointRemain);
#endif
			}
		}
		break;

		case WIZ_ITEM_REMOVE:
		{
			uint8_t	iResult = pkt.read<uint8_t>();

			switch (iResult)
			{
			case 0x00:
			{
#ifdef DEBUG
				printf("RecvProcess::WIZ_ITEM_REMOVE: 0\n");
#endif
			}
			break;

			case 0x01:
			{
#ifdef DEBUG
				printf("RecvProcess::WIZ_ITEM_REMOVE: 1\n");
#endif
			}
			break;

			default:
			{
#ifdef DEBUG
				printf("RecvProcess::WIZ_ITEM_REMOVE: %d Result Not Implemented\n", iResult);
#endif
			}
			break;
			}
		}
		break;

		case WIZ_ITEM_COUNT_CHANGE:
		{
			int16_t iTotalCount = pkt.read<int16_t>();

			for (int i = 0; i < iTotalCount; i++)
			{
				uint8_t iDistrict = pkt.read<uint8_t>();
				uint8_t iIndex = pkt.read<uint8_t>();
				uint32_t iID = pkt.read<uint32_t>();
				uint32_t iCount = pkt.read<uint32_t>();
				uint8_t iNewItem = pkt.read<uint8_t>();
				uint16_t iDurability = pkt.read<uint16_t>();

				uint32_t iSerial = pkt.read<uint32_t>();
				uint32_t iExpirationTime = pkt.read<uint32_t>();

				m_PlayerMySelf.tInventory[14 + iIndex].iItemID = iID;
				m_PlayerMySelf.tInventory[14 + iIndex].iCount = (uint16_t)iCount;
				m_PlayerMySelf.tInventory[14 + iIndex].iDurability = iDurability;
				m_PlayerMySelf.tInventory[14 + iIndex].iSerial = iSerial;
				m_PlayerMySelf.tInventory[14 + iIndex].iExpirationTime = iExpirationTime;

#ifdef DEBUG
				printf("RecvProcess::WIZ_ITEM_COUNT_CHANGE: %d,%d,%d,%d,%d,%d,%d,%d,%d\n",
					iDistrict,
					iIndex,
					iID,
					iCount,
					iNewItem,
					iNewItem,
					iDurability,
					iSerial,
					iExpirationTime);
#endif

			}
		}
		break;

		case WIZ_WARP_LIST:
		{
			uint8_t iOpcode = pkt.read<uint8_t>();

			switch (iOpcode)
			{
				//Warp List Load
				case 1:
				{
					if (IsRouting())
					{
						m_bRouteWarpListLoaded = true;
					}
				}
				break;
			}
		}
		break;

		case WIZ_EXP_CHANGE:
		{
			uint8_t iOpcode = pkt.read<uint8_t>();
			uint64_t iExp = pkt.read<uint64_t>();

			uint64_t iGainedExp = iExp - m_PlayerMySelf.iExp;

			m_iExpCounter += iGainedExp;

			if (m_bSlotExpLimitEnable)
			{
				if (m_iSlotExpLimit > iGainedExp)
				{
					m_bLegalMode = false;
					Drawing::SetLegalModeSettings(m_bLegalMode);
				}
				else
				{
					m_bLegalMode = true;
					Drawing::SetLegalModeSettings(m_bLegalMode);
				}
			}

			m_PlayerMySelf.iExp = iExp;
		}
		break;

		case WIZ_GOLD_CHANGE:
		{
			uint8_t iOpcode = pkt.read<uint8_t>();
			uint32_t iChangeAmount = pkt.read<uint32_t>();
			uint32_t iGold = pkt.read<uint32_t>();

			m_PlayerMySelf.iGold = iChangeAmount;
		}
		break;

		case WIZ_TRADE_NPC:
		{
			int32_t iNpcSellingGroup = pkt.read<int32_t>();

			if (m_pCurrentRunningRoute != nullptr
				&& (m_pCurrentRunningRoute->eStepType == STEP_POTION
				|| m_pCurrentRunningRoute->eStepType == STEP_DC_SUNDRIES)
				&& m_pCurrentRunningRoute->bSubProcessRequested)
			{
				new std::thread([=]()
				{
					/**
					* Selling Operations
					*/

					if (m_vecSellItemList.size() > 0)
					{
						std::vector<TItemData> vecInventoryItemList;
						GetInventoryItemList(vecInventoryItemList);

						uint8_t iSellPageCount = 0;
						std::vector<SSItemSell> vecInventoryItemSell[2];

						for (const TItemData& pItem : vecInventoryItemList)
						{
							if (pItem.iItemID == 0)
								continue;

							//Promise Of Training
							if (pItem.iItemID == 989511000)
								continue;

							__TABLE_ITEM* pItemData;
							if (!m_Bot->GetItemData(pItem.iItemID, pItemData))
								continue;

							__TABLE_ITEM_EXTENSION* pItemExtensionData;
							if (!m_Bot->GetItemExtensionData(pItem.iItemID, pItemData->byExtIndex, pItemExtensionData))
								continue;

							if (pItemData->byNeedRace == RACE_TRADEABLE_IN_72HR
								|| pItemData->byNeedRace == RACE_NO_TRADE
								|| pItemData->byNeedRace == RACE_NO_TRADE_SOLD
								|| pItemData->byNeedRace == RACE_NO_TRADE_SOLD2
								|| pItemData->byNeedRace == RACE_NO_TRADE_SOLD_STORE)
								continue;

							if (pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UNIQUE
								|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_RED
								|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UPGRADE_REVERSE
								|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UNIQUE_REVERSE
								|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_PET
								|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_EVENT
								|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_COSPRE
								|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_MINERVA)
								continue;

							uint32_t iItemBaseID = pItem.iItemID / 1000 * 1000;
							bool bSellSelected = m_vecSellItemList.count(iItemBaseID) > 0;

							if (bSellSelected)
							{
								if (vecInventoryItemSell[iSellPageCount].size() == 14)
									iSellPageCount++;

								vecInventoryItemSell[iSellPageCount].push_back(SSItemSell(pItem.iItemID, (uint8_t)(pItem.iPos - 14), pItem.iCount));
							}
						}

						if (vecInventoryItemSell[0].size() > 0)
						{
							for (size_t i = 0; i <= iSellPageCount; i++)
							{
								SendItemTradeSell(iNpcSellingGroup, m_pCurrentRunningRoute->pNpcID, vecInventoryItemSell[i]);
							}

							SendShoppingMall(ShoppingMallType::STORE_CLOSE);
							std::this_thread::sleep_for(std::chrono::milliseconds(1500));
						}

						/**
						* VIP Selling Operations
						*/

						if (m_bVIPSellSupply)
						{
							int iInventoryEmptySlotCount = GetInventoryEmptySlotCount();

							if (iInventoryEmptySlotCount > 0)
							{
								bool bRun = true;

								while (bRun)
								{
									OpenVipWarehouse();

									std::this_thread::sleep_for(std::chrono::milliseconds(1500));

									WaitConditionWithTimeout(m_bVipWarehouseLoaded == false, 3000);

									if (!m_bVipWarehouseLoaded)
										break;

									std::vector<TItemData> vecVipWarehouseItemList;
									std::vector<TItemData> vecFlaggedItemList;

									GetVipWarehouseItemList(vecVipWarehouseItemList);

									for (const TItemData& pItem : vecVipWarehouseItemList)
									{
										if (pItem.iItemID == 0)
											continue;

										//Promise Of Training
										if (pItem.iItemID == 989511000)
											continue;

										__TABLE_ITEM* pItemData;
										if (!m_Bot->GetItemData(pItem.iItemID, pItemData))
											continue;

										__TABLE_ITEM_EXTENSION* pItemExtensionData;
										if (!m_Bot->GetItemExtensionData(pItem.iItemID, pItemData->byExtIndex, pItemExtensionData))
											continue;

										if (pItemData->byNeedRace == RACE_TRADEABLE_IN_72HR
											|| pItemData->byNeedRace == RACE_NO_TRADE
											|| pItemData->byNeedRace == RACE_NO_TRADE_SOLD
											|| pItemData->byNeedRace == RACE_NO_TRADE_SOLD2
											|| pItemData->byNeedRace == RACE_NO_TRADE_SOLD_STORE)
											continue;

										if (pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UNIQUE
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_RED
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UPGRADE_REVERSE
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UNIQUE_REVERSE
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_PET
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_EVENT
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_COSPRE
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_MINERVA)
											continue;

										uint32_t iItemBaseID = pItem.iItemID / 1000 * 1000;
										bool bSellSelected = m_vecSellItemList.count(iItemBaseID) > 0;

										if (!bSellSelected)
											continue;

										vecFlaggedItemList.push_back(pItem);

										if (vecFlaggedItemList.size() == iInventoryEmptySlotCount)
											break;
									}

									if (vecFlaggedItemList.size() == 0)
									{
										std::this_thread::sleep_for(std::chrono::milliseconds(1500));
										CloseVipWarehouse();
										std::this_thread::sleep_for(std::chrono::milliseconds(1500));
										break;
									}

									for (const TItemData& pItem : vecFlaggedItemList)
									{
										__TABLE_ITEM* pItemData;
										if (!m_Bot->GetItemData(pItem.iItemID, pItemData))
											continue;

										WaitConditionWithTimeout(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_UI_LOCK"))) == 1, 3000);

										std::vector<TItemData> vecVipWarehouseInventoryItemList;
										GetVipWarehouseInventoryItemList(vecVipWarehouseInventoryItemList);

										int iTargetPosition = -1;

										if (pItemData->byContable)
										{
											auto pInventoryItem = std::find_if(vecVipWarehouseInventoryItemList.begin(), vecVipWarehouseInventoryItemList.end(),
												[&](const TItemData& a)
												{
													return a.iItemID == pItem.iItemID;
												});

											if (pInventoryItem != vecVipWarehouseInventoryItemList.end())
											{
												if ((pItem.iCount + pInventoryItem->iCount) > 9999)
													continue;

												iTargetPosition = pInventoryItem->iPos;
											}
											else
											{
												auto pInventoryEmptySlot = std::find_if(vecVipWarehouseInventoryItemList.begin(), vecVipWarehouseInventoryItemList.end(),
													[&](const TItemData& a)
													{
														return a.iItemID == 0;
													});

												if (pInventoryEmptySlot != vecVipWarehouseInventoryItemList.end())
												{
													iTargetPosition = pInventoryEmptySlot->iPos;
												}
											}
										}
										else
										{
											auto pInventoryEmptySlot = std::find_if(vecVipWarehouseInventoryItemList.begin(), vecVipWarehouseInventoryItemList.end(),
												[&](const TItemData& a)
												{
													return a.iItemID == 0;
												});

											if (pInventoryEmptySlot != vecVipWarehouseInventoryItemList.end())
											{
												iTargetPosition = pInventoryEmptySlot->iPos;
											}
										}

										if (iTargetPosition == -1)
										{
											break;
										}

										WaitConditionWithTimeout(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_UI_LOCK"))) == 1, 3000);

										VipWarehouseGetOut(pItem.iBase, pItem.iPos, iTargetPosition);

										WaitConditionWithTimeout(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_UI_LOCK"))) == 1, 3000);

										if (pItemData->byContable)
										{
											CountableDialogChangeCount(pItem.iCount);
											AcceptCountableDialog();
										}
									}

									WaitConditionWithTimeout(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_UI_LOCK"))) == 1, 3000);

									std::this_thread::sleep_for(std::chrono::milliseconds(1500));

									CloseVipWarehouse();

									std::this_thread::sleep_for(std::chrono::milliseconds(1500));

									std::vector<TItemData> vecInventoryItemList;
									GetInventoryItemList(vecInventoryItemList);

									uint8_t iSellPageCount = 0;
									std::vector<SSItemSell> vecInventoryItemSell[2];

									for (const TItemData& pItem : vecInventoryItemList)
									{
										if (pItem.iItemID == 0)
											continue;

										//Promise Of Training
										if (pItem.iItemID == 989511000)
											continue;

										__TABLE_ITEM* pItemData;
										if (!m_Bot->GetItemData(pItem.iItemID, pItemData))
											continue;

										__TABLE_ITEM_EXTENSION* pItemExtensionData;
										if (!m_Bot->GetItemExtensionData(pItem.iItemID, pItemData->byExtIndex, pItemExtensionData))
											continue;

										if (pItemData->byNeedRace == RACE_TRADEABLE_IN_72HR
											|| pItemData->byNeedRace == RACE_NO_TRADE
											|| pItemData->byNeedRace == RACE_NO_TRADE_SOLD
											|| pItemData->byNeedRace == RACE_NO_TRADE_SOLD2
											|| pItemData->byNeedRace == RACE_NO_TRADE_SOLD_STORE)
											continue;

										if (pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UNIQUE
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_RED
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UPGRADE_REVERSE
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UNIQUE_REVERSE
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_PET
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_EVENT
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_COSPRE
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_MINERVA)
											continue;

										uint32_t iItemBaseID = pItem.iItemID / 1000 * 1000;
										bool bSellSelected = m_vecSellItemList.count(iItemBaseID) > 0;

										if (bSellSelected)
										{
											if (vecInventoryItemSell[iSellPageCount].size() == 14)
												iSellPageCount++;

											vecInventoryItemSell[iSellPageCount].push_back(SSItemSell(pItem.iItemID, (uint8_t)(pItem.iPos - 14), pItem.iCount));
										}
									}

									if (vecInventoryItemSell[0].size() > 0)
									{
										for (size_t i = 0; i <= iSellPageCount; i++)
										{
											SendItemTradeSell(iNpcSellingGroup, m_pCurrentRunningRoute->pNpcID, vecInventoryItemSell[i]);
										}

										SendShoppingMall(ShoppingMallType::STORE_CLOSE);
										std::this_thread::sleep_for(std::chrono::milliseconds(1500));
									}
								}
							}
						}
					}

					/**
					* Buying Operations
					*/

					if (m_bAutoSupply)
					{
						std::vector<uint32_t> vecSellingGroupList;

						vecSellingGroupList.push_back(253000);
						vecSellingGroupList.push_back(255000);
						vecSellingGroupList.push_back(267000);

						uint8_t iBuyPageCount = 0;
						std::vector<SSItemBuy> vecInventoryItemBuy[2];
						std::vector<int32_t> vecExceptedBuyPos;

						for (auto& pSellingGroup : vecSellingGroupList)
						{
							if (iNpcSellingGroup != pSellingGroup)
								continue;

							std::string szSupplyString = "Supply_" + std::to_string(pSellingGroup);
							std::unordered_set<int> vecsupplyList = GetUserConfiguration()->GetInt(szSupplyString.c_str(), skCryptDec("Enable"), std::unordered_set<int>());

							for (auto& pSupplyItemID : vecsupplyList)
							{
								std::vector<SShopItem> vecShopItemTable;
								if (!m_Bot->GetShopItemTable(iNpcSellingGroup, vecShopItemTable))
									continue;

								uint8_t iPos = -1;
								int iItemCount = GetUserConfiguration()->GetInt(szSupplyString.c_str(), std::to_string(pSupplyItemID).c_str(), 0);

								TItemData pInventoryItem = GetInventoryItem(pSupplyItemID);

								if (pInventoryItem.iItemID != 0)
								{
									if (pInventoryItem.iCount >= iItemCount)
										continue;

									iPos = (uint8_t)pInventoryItem.iPos;
									iItemCount = (int)std::abs(pInventoryItem.iCount - iItemCount);
								}
								else
								{
									iPos = (uint8_t)GetInventoryEmptySlot(vecExceptedBuyPos);
								}

								if (iPos == -1 || iItemCount == 0)
									continue;

								auto pShopItem = std::find_if(vecShopItemTable.begin(), vecShopItemTable.end(),
									[pSupplyItemID](const SShopItem& a) { return a.m_iItemId == pSupplyItemID; });

								if (pShopItem == vecShopItemTable.end())
									continue;

								if (vecInventoryItemBuy[iBuyPageCount].size() == 14)
									iBuyPageCount++;

								vecInventoryItemBuy[iBuyPageCount].push_back(
									SSItemBuy(pShopItem->m_iItemId, (iPos - 14), (int16_t)iItemCount, pShopItem->m_iPage, pShopItem->m_iPos));

								vecExceptedBuyPos.push_back(iPos);
							}
						}

						if (vecInventoryItemBuy[0].size() > 0)
						{
							for (size_t i = 0; i <= iBuyPageCount; i++)
							{
								SendItemTradeBuy(iNpcSellingGroup, m_pCurrentRunningRoute->pNpcID, vecInventoryItemBuy[i]);
							}

							SendShoppingMall(ShoppingMallType::STORE_CLOSE);
							std::this_thread::sleep_for(std::chrono::milliseconds(1500));
						}
					}

					m_pCurrentRunningRoute->bSubProcessFinished = true;
				});
			}
		}
		break;

		case WIZ_REPAIR_NPC:
		{
			int32_t iNpcSellingGroup = pkt.read<int32_t>();

			if (m_pCurrentRunningRoute != nullptr
				&& m_pCurrentRunningRoute->eStepType == STEP_SUNDRIES
				&& m_pCurrentRunningRoute->bSubProcessRequested)
			{

				new std::thread([=]()
				{
					/**
					* Selling Operations
					*/

					if (m_vecSellItemList.size() > 0)
					{
						std::vector<TItemData> vecInventoryItemList;
						GetInventoryItemList(vecInventoryItemList);

						uint8_t iSellPageCount = 0;
						std::vector<SSItemSell> vecInventoryItemSell[2];

						for (const TItemData& pItem : vecInventoryItemList)
						{
							if (pItem.iItemID == 0)
								continue;

							//Promise Of Training
							if (pItem.iItemID == 989511000)
								continue;

							__TABLE_ITEM* pItemData;
							if (!m_Bot->GetItemData(pItem.iItemID, pItemData))
								continue;

							__TABLE_ITEM_EXTENSION* pItemExtensionData;
							if (!m_Bot->GetItemExtensionData(pItem.iItemID, pItemData->byExtIndex, pItemExtensionData))
								continue;

							if (pItemData->byNeedRace == RACE_TRADEABLE_IN_72HR
								|| pItemData->byNeedRace == RACE_NO_TRADE
								|| pItemData->byNeedRace == RACE_NO_TRADE_SOLD
								|| pItemData->byNeedRace == RACE_NO_TRADE_SOLD2
								|| pItemData->byNeedRace == RACE_NO_TRADE_SOLD_STORE)
								continue;

							if (pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UNIQUE
								|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_RED
								|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UPGRADE_REVERSE
								|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UNIQUE_REVERSE
								|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_PET
								|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_EVENT
								|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_COSPRE
								|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_MINERVA)
								continue;

							uint32_t iItemBaseID = pItem.iItemID / 1000 * 1000;
							bool bSellSelected = m_vecSellItemList.count(iItemBaseID) > 0;

							if (bSellSelected)
							{
								if (vecInventoryItemSell[iSellPageCount].size() == 14)
									iSellPageCount++;

								vecInventoryItemSell[iSellPageCount].push_back(SSItemSell(pItem.iItemID, (uint8_t)(pItem.iPos - 14), pItem.iCount));
							}
						}

						if (vecInventoryItemSell[0].size() > 0)
						{
							for (size_t i = 0; i <= iSellPageCount; i++)
							{
								SendItemTradeSell(iNpcSellingGroup, m_pCurrentRunningRoute->pNpcID, vecInventoryItemSell[i]);
							}

							SendShoppingMall(ShoppingMallType::STORE_CLOSE);
							std::this_thread::sleep_for(std::chrono::milliseconds(1500));
						}

						/**
						* VIP Selling Operations
						*/

						if (m_bVIPSellSupply)
						{
							int iInventoryEmptySlotCount = GetInventoryEmptySlotCount();

							if (iInventoryEmptySlotCount > 0)
							{
								bool bRun = true;

								while (bRun)
								{
									OpenVipWarehouse();

									std::this_thread::sleep_for(std::chrono::milliseconds(1500));

									WaitConditionWithTimeout(m_bVipWarehouseLoaded == false, 3000);

									if (!m_bVipWarehouseLoaded)
										break;

									std::vector<TItemData> vecVipWarehouseItemList;
									std::vector<TItemData> vecFlaggedItemList;

									GetVipWarehouseItemList(vecVipWarehouseItemList);

									for (const TItemData& pItem : vecVipWarehouseItemList)
									{
										if (pItem.iItemID == 0)
											continue;

										//Promise Of Training
										if (pItem.iItemID == 989511000)
											continue;

										__TABLE_ITEM* pItemData;
										if (!m_Bot->GetItemData(pItem.iItemID, pItemData))
											continue;

										__TABLE_ITEM_EXTENSION* pItemExtensionData;
										if (!m_Bot->GetItemExtensionData(pItem.iItemID, pItemData->byExtIndex, pItemExtensionData))
											continue;

										if (pItemData->byNeedRace == RACE_TRADEABLE_IN_72HR
											|| pItemData->byNeedRace == RACE_NO_TRADE
											|| pItemData->byNeedRace == RACE_NO_TRADE_SOLD
											|| pItemData->byNeedRace == RACE_NO_TRADE_SOLD2
											|| pItemData->byNeedRace == RACE_NO_TRADE_SOLD_STORE)
											continue;

										if (pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UNIQUE
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_RED
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UPGRADE_REVERSE
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UNIQUE_REVERSE
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_PET
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_EVENT
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_COSPRE
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_MINERVA)
											continue;

										uint32_t iItemBaseID = pItem.iItemID / 1000 * 1000;
										bool bSellSelected = m_vecSellItemList.count(iItemBaseID) > 0;

										if (!bSellSelected)
											continue;

										vecFlaggedItemList.push_back(pItem);

										if (vecFlaggedItemList.size() == iInventoryEmptySlotCount)
											break;
									}

									if (vecFlaggedItemList.size() == 0)
									{
										std::this_thread::sleep_for(std::chrono::milliseconds(1500));
										CloseVipWarehouse();
										std::this_thread::sleep_for(std::chrono::milliseconds(1500));
										break;
									}

									for (const TItemData& pItem : vecFlaggedItemList)
									{
										__TABLE_ITEM* pItemData;
										if (!m_Bot->GetItemData(pItem.iItemID, pItemData))
											continue;

										WaitConditionWithTimeout(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_UI_LOCK"))) == 1, 3000);

										std::vector<TItemData> vecVipWarehouseInventoryItemList;
										GetVipWarehouseInventoryItemList(vecVipWarehouseInventoryItemList);

										int iTargetPosition = -1;

										if (pItemData->byContable)
										{
											auto pInventoryItem = std::find_if(vecVipWarehouseInventoryItemList.begin(), vecVipWarehouseInventoryItemList.end(),
												[&](const TItemData& a)
												{
													return a.iItemID == pItem.iItemID;
												});

											if (pInventoryItem != vecVipWarehouseInventoryItemList.end())
											{
												if ((pItem.iCount + pInventoryItem->iCount) > 9999)
													continue;

												iTargetPosition = pInventoryItem->iPos;
											}
											else
											{
												auto pInventoryEmptySlot = std::find_if(vecVipWarehouseInventoryItemList.begin(), vecVipWarehouseInventoryItemList.end(),
													[&](const TItemData& a)
													{
														return a.iItemID == 0;
													});

												if (pInventoryEmptySlot != vecVipWarehouseInventoryItemList.end())
												{
													iTargetPosition = pInventoryEmptySlot->iPos;
												}
											}
										}
										else
										{
											auto pInventoryEmptySlot = std::find_if(vecVipWarehouseInventoryItemList.begin(), vecVipWarehouseInventoryItemList.end(),
												[&](const TItemData& a)
												{
													return a.iItemID == 0;
												});

											if (pInventoryEmptySlot != vecVipWarehouseInventoryItemList.end())
											{
												iTargetPosition = pInventoryEmptySlot->iPos;
											}
										}

										if (iTargetPosition == -1)
										{
											break;
										}

										WaitConditionWithTimeout(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_UI_LOCK"))) == 1, 3000);

										VipWarehouseGetOut(pItem.iBase, pItem.iPos, iTargetPosition);

										WaitConditionWithTimeout(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_UI_LOCK"))) == 1, 3000);

										if (pItemData->byContable)
										{
											CountableDialogChangeCount(pItem.iCount);
											AcceptCountableDialog();
										}
									}

									WaitConditionWithTimeout(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_UI_LOCK"))) == 1, 3000);

									std::this_thread::sleep_for(std::chrono::milliseconds(1500));

									CloseVipWarehouse();

									std::this_thread::sleep_for(std::chrono::milliseconds(1500));

									std::vector<TItemData> vecInventoryItemList;
									GetInventoryItemList(vecInventoryItemList);

									uint8_t iSellPageCount = 0;
									std::vector<SSItemSell> vecInventoryItemSell[2];

									for (const TItemData& pItem : vecInventoryItemList)
									{
										if (pItem.iItemID == 0)
											continue;

										//Promise Of Training
										if (pItem.iItemID == 989511000)
											continue;

										__TABLE_ITEM* pItemData;
										if (!m_Bot->GetItemData(pItem.iItemID, pItemData))
											continue;

										__TABLE_ITEM_EXTENSION* pItemExtensionData;
										if (!m_Bot->GetItemExtensionData(pItem.iItemID, pItemData->byExtIndex, pItemExtensionData))
											continue;

										if (pItemData->byNeedRace == RACE_TRADEABLE_IN_72HR
											|| pItemData->byNeedRace == RACE_NO_TRADE
											|| pItemData->byNeedRace == RACE_NO_TRADE_SOLD
											|| pItemData->byNeedRace == RACE_NO_TRADE_SOLD2
											|| pItemData->byNeedRace == RACE_NO_TRADE_SOLD_STORE)
											continue;

										if (pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UNIQUE
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_RED
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UPGRADE_REVERSE
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UNIQUE_REVERSE
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_PET
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_EVENT
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_COSPRE
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_MINERVA)
											continue;

										uint32_t iItemBaseID = pItem.iItemID / 1000 * 1000;
										bool bSellSelected = m_vecSellItemList.count(iItemBaseID) > 0;

										if (bSellSelected)
										{
											if (vecInventoryItemSell[iSellPageCount].size() == 14)
												iSellPageCount++;

											vecInventoryItemSell[iSellPageCount].push_back(SSItemSell(pItem.iItemID, (uint8_t)(pItem.iPos - 14), pItem.iCount));
										}
									}

									if (vecInventoryItemSell[0].size() > 0)
									{
										for (size_t i = 0; i <= iSellPageCount; i++)
										{
											SendItemTradeSell(iNpcSellingGroup, m_pCurrentRunningRoute->pNpcID, vecInventoryItemSell[i]);
										}

										SendShoppingMall(ShoppingMallType::STORE_CLOSE);
										std::this_thread::sleep_for(std::chrono::milliseconds(1500));
									}
								}
							}
						}
					}

					/**
					* Buying Operations
					*/

					if (m_bAutoSupply)
					{
						std::vector<uint32_t> vecSellingGroupList;

						vecSellingGroupList.push_back(253000);
						vecSellingGroupList.push_back(255000);
						vecSellingGroupList.push_back(267000);

						uint8_t iBuyPageCount = 0;
						std::vector<SSItemBuy> vecInventoryItemBuy[2];
						std::vector<int32_t> vecExceptedBuyPos;

						for (auto& pSellingGroup : vecSellingGroupList)
						{
							if (iNpcSellingGroup != pSellingGroup)
								continue;

							std::string szSupplyString = "Supply_" + std::to_string(pSellingGroup);
							std::unordered_set<int> vecsupplyList = GetUserConfiguration()->GetInt(szSupplyString.c_str(), skCryptDec("Enable"), std::unordered_set<int>());

							for (auto& pSupplyItemID : vecsupplyList)
							{
								std::vector<SShopItem> vecShopItemTable;
								if (!m_Bot->GetShopItemTable(iNpcSellingGroup, vecShopItemTable))
									continue;

								uint8_t iPos = -1;
								int iItemCount = GetUserConfiguration()->GetInt(szSupplyString.c_str(), std::to_string(pSupplyItemID).c_str(), 0);

								TItemData pInventoryItem = GetInventoryItem(pSupplyItemID);

								if (pInventoryItem.iItemID != 0)
								{
									if (pInventoryItem.iCount >= iItemCount)
										continue;

									iPos = (uint8_t)pInventoryItem.iPos;
									iItemCount = (int)std::abs(pInventoryItem.iCount - iItemCount);
								}
								else
								{
									iPos = (uint8_t)GetInventoryEmptySlot(vecExceptedBuyPos);
								}

								if (iPos == -1 || iItemCount == 0)
									continue;

								auto pShopItem = std::find_if(vecShopItemTable.begin(), vecShopItemTable.end(),
									[pSupplyItemID](const SShopItem& a) { return a.m_iItemId == pSupplyItemID; });

								if (pShopItem == vecShopItemTable.end())
									continue;

								if (vecInventoryItemBuy[iBuyPageCount].size() == 14)
									iBuyPageCount++;

								vecInventoryItemBuy[iBuyPageCount].push_back(
									SSItemBuy(pShopItem->m_iItemId, (iPos - 14), (int16_t)iItemCount, pShopItem->m_iPage, pShopItem->m_iPos));

								vecExceptedBuyPos.push_back(iPos);
							}
						}

						if (vecInventoryItemBuy[0].size() > 0)
						{
							for (size_t i = 0; i <= iBuyPageCount; i++)
							{
								SendItemTradeBuy(iNpcSellingGroup, m_pCurrentRunningRoute->pNpcID, vecInventoryItemBuy[i]);
							}

							SendShoppingMall(ShoppingMallType::STORE_CLOSE);
							std::this_thread::sleep_for(std::chrono::milliseconds(1500));
						}
					}

					/**
					 * Repair Operations
					*/

					if (m_bAutoRepair)
					{
						for (uint8_t i = 0; i < SLOT_MAX + HAVE_MAX; i++)
						{
							switch (i)
							{
								case 1:
								case 4:
								case 6:
								case 8:
								case 10:
								case 12:
								case 13:
								{
									TItemData pInventory = GetInventoryItemSlot(i);

									if (pInventory.iItemID == 0)
										continue;

									SendItemRepair(1, (uint8_t)pInventory.iPos, m_pCurrentRunningRoute->pNpcID, pInventory.iItemID);
								}
								break;
							}
						}
					}

					m_pCurrentRunningRoute->bSubProcessFinished = true;
				});
			}
		}
		break;	
	}
}

void ClientHandler::SendProcess(BYTE* byBuffer, DWORD iLength)
{
	Packet pkt = Packet(byBuffer[0], (size_t)iLength);
	pkt.append(&byBuffer[1], iLength - 1);

#ifdef DEBUG
#ifdef PRINT_SEND_PACKET
	printf("SendProcess: %s\n", pkt.convertToHex().c_str());
#endif
#endif

	uint8_t iHeader;

	pkt >> iHeader;

	switch (iHeader)
	{
		case WIZ_HOME:
		{
#ifdef DEBUG
			printf("SendProcess::WIZ_HOME\n");
#endif
			m_vecLootList.clear();

			if (!IsRouting())
			{
				if (m_bTownStopBot)
				{
					m_bAttackStatus = GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Attack"), 0);
					m_bCharacterStatus = GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Character"), 0);
				}				
			}

			if (m_bIsRoutePlanning)
			{
				Route pAutoRoute{};

				pAutoRoute.fX = GetX();
				pAutoRoute.fY = GetY();
				pAutoRoute.eStepType = RouteStepType::STEP_TOWN;

				m_vecRoutePlan.push_back(pAutoRoute);
			}

			Packet pkt = Packet(PIPE_SELECT_CHARACTER_ENTER);
		}
		break;

		case WIZ_ITEM_MOVE:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case 1:
				{
					uint8_t iDirection = pkt.read<uint8_t>();

					uint32_t iItemID = pkt.read<uint32_t>();

					uint8_t iCurrentPosition = pkt.read<uint8_t>();
					uint8_t iTargetPosition = pkt.read<uint8_t>();

					switch (iDirection)
					{
						case ITEM_INVEN_SLOT:
							break;
						case ITEM_SLOT_INVEN:
							break;
						case ITEM_INVEN_INVEN:
							break;
						case ITEM_SLOT_SLOT:
							break;
						case ITEM_INVEN_ZONE:
							break;
						case ITEM_ZONE_INVEN:
							break;
						case ITEM_INVEN_TO_COSP:
							break;
						case ITEM_COSP_TO_INVEN:
							break;
						case ITEM_INVEN_TO_MBAG:
							break;
						case ITEM_MBAG_TO_INVEN:
							break;
						case ITEM_MBAG_TO_MBAG:
							break;

						default:
	#ifdef DEBUG
							printf("SendProcess::WIZ_ITEM_MOVE: Direction %d not implemented\n", iDirection);
	#endif
							break;
					}

#ifdef DEBUG
					printf("SendProcess::WIZ_ITEM_MOVE: iDirection(%d), iItemID(%d), iCurrentPosition(%d), iTargetPosition(%d)\n",
						iDirection,
						iItemID,
						iCurrentPosition,
						iTargetPosition
					);
#endif
				}
				break;

				default:
				{
#ifdef DEBUG
					printf("SendProcess::WIZ_ITEM_MOVE: Type %d not implemented\n", iType);
#endif
				}
				break;
			}
		}
		break;

		case WIZ_MAGIC_PROCESS:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case SkillMagicType::SKILL_MAGIC_TYPE_FLYING:
				{
					uint32_t iSkillID = pkt.read<uint32_t>();
					int32_t iSourceID = pkt.read<int32_t>();
					int32_t iTargetID = pkt.read<int32_t>();

					std::map<uint32_t, __TABLE_UPC_SKILL>* pSkillTable;
					if (!m_Bot->GetSkillTable(&pSkillTable))
						return;

					auto pSkillData = pSkillTable->find(iSkillID);

					if (pSkillData != pSkillTable->end())
					{
						if (iSourceID == m_PlayerMySelf.iID)
						{
							if (IsRogue())
							{
								if (!m_bUseSkillWithPacket && m_bArcherCombo)
								{
									auto it = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
										[&](const TNpc& a) { return a.iID == iTargetID; });

									if (it != m_vecNpc.end())
									{
										if (it->eState != PSA_DYING && it->eState != PSA_DEATH && it->iHPMax != 0 && it->iHP != 0)
										{
											StepCharacterForward(true);
											StepCharacterForward(false);
										}
									}
								}
							}	
						}
					}
				}
				break;
			}
		}
		break;

		case WIZ_SKILLPT_CHANGE:
		{
			int iType = pkt.read<uint8_t>();

			LoadSkillData();

#ifdef DEBUG
			printf("SendProcess::WIZ_SKILLPT_CHANGE: %d\n", iType);
#endif
		}
		break;

		case WIZ_ATTACK:
		{
#ifdef DEBUG
			printf("SendProcess::WIZ_ATTACK\n");
#endif
			m_fAttackTimeRecent = Bot::TimeGet();
		}
		break;

		case WIZ_PARTY:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case PartyUpdateType::PARTY_UPDATE_CREATE:
				{
					m_vecPartyMembers.clear();

					PartyMember pPartyMemberMySelf;

					memset(&pPartyMemberMySelf, 0, sizeof(pPartyMemberMySelf));

					pPartyMemberMySelf.iMemberID = m_PlayerMySelf.iID;
					pPartyMemberMySelf.iIndex = 100;
					pPartyMemberMySelf.szName = m_PlayerMySelf.szName;

					pPartyMemberMySelf.iMaxHP = (uint16_t)m_PlayerMySelf.iHPMax;
					pPartyMemberMySelf.iHP = (uint16_t)m_PlayerMySelf.iHP;
					pPartyMemberMySelf.iLevel = m_PlayerMySelf.iLevel;
					pPartyMemberMySelf.iClass = (uint16_t)m_PlayerMySelf.eClass;
					pPartyMemberMySelf.iMaxMP = (uint16_t)m_PlayerMySelf.iMSPMax;
					pPartyMemberMySelf.iMP = (uint16_t)m_PlayerMySelf.iMSP;

					pPartyMemberMySelf.iNation = m_PlayerMySelf.eNation;

					m_vecPartyMembers.push_back(pPartyMemberMySelf);
				}
				break;
			}
		}
		break;

		case WIZ_OBJECT_EVENT:
		{
			if (m_bIsRoutePlanning)
			{
				Route pAutoRoute{};

				pAutoRoute.fX = GetX();
				pAutoRoute.fY = GetY();
				pAutoRoute.eStepType = RouteStepType::STEP_OBJECT_EVENT;
				pAutoRoute.szPacket = pkt.convertToHex();

				m_vecRoutePlan.push_back(pAutoRoute);
			}
		}
		break;

		case WIZ_WARP_LIST:
		{
			if (m_bIsRoutePlanning)
			{
				Route pAutoRoute{};

				pAutoRoute.fX = GetX();
				pAutoRoute.fY = GetY();
				pAutoRoute.eStepType = RouteStepType::STEP_WARP;
				pAutoRoute.szPacket = pkt.convertToHex();

				m_vecRoutePlan.push_back(pAutoRoute);
			}
		}
		break;

		case WIZ_MOVE:
		{
		}
		break;
	}
}

bool ClientHandler::SolveCaptcha(std::vector<uint8_t> vecImageBuffer)
{
	m_Bot->SendCaptcha(vecImageBuffer);

	return true;
}

TNpc ClientHandler::InitializeNpc(Packet& pkt)
{
	TNpc tNpc;
	memset(&tNpc, 0, sizeof(tNpc));

	pkt.SByte();

	tNpc.iID = pkt.read<int32_t>();
	tNpc.iProtoID = pkt.read<uint16_t>();
	tNpc.iMonsterOrNpc = pkt.read<uint8_t>();
	tNpc.iPictureId = pkt.read<uint16_t>();
	tNpc.iUnknown1 = pkt.read<uint32_t>();
	tNpc.iFamilyType = pkt.read<uint8_t>();
	tNpc.iSellingGroup = pkt.read<uint32_t>();
	tNpc.iModelsize = pkt.read<uint16_t>();
	tNpc.iWeapon1 = pkt.read<uint32_t>();
	tNpc.iWeapon2 = pkt.read<uint32_t>();

	if (tNpc.iProtoID == 0)
	{
		pkt.readString(tNpc.szPetOwnerName);
		pkt.readString(tNpc.szPetName);
	}

	tNpc.iModelGroup = pkt.read<uint8_t>();
	tNpc.iLevel = pkt.read<uint8_t>();

	tNpc.fX = (pkt.read<uint16_t>() / 10.0f);
	tNpc.fY = (pkt.read<uint16_t>() / 10.0f);
	tNpc.fZ = (pkt.read<int16_t>() / 10.0f);

	tNpc.iStatus = pkt.read<uint32_t>();

	pkt.read<uint8_t>();
	pkt.read<uint32_t>();

	tNpc.fRotation = pkt.read<int16_t>() / 100.0f;

	return tNpc;
}

TPlayer ClientHandler::InitializePlayer(Packet& pkt)
{
	TPlayer tPlayer;
	memset(&tPlayer, 0, sizeof(tPlayer));

	pkt.SByte();

	tPlayer.iID = pkt.read<int32_t>();

	pkt.readString(tPlayer.szName);

	tPlayer.eNation = (Nation)pkt.read<uint8_t>();

	pkt.read<uint8_t>();
	pkt.read<uint8_t>();
	pkt.read<uint8_t>();

	tPlayer.iKnightsID = pkt.read<int16_t>();
	tPlayer.eKnightsDuty = (KnightsDuty)pkt.read<uint8_t>();

	if (tPlayer.iKnightsID > 0)
	{
		if (m_Bot->GetPlatformType() == PlatformType::USKO 
			|| m_Bot->GetPlatformType() == PlatformType::STKO)
		{
			int16_t iAllianceID = pkt.read<int16_t>();
		}
		else if (m_Bot->GetPlatformType() == PlatformType::CNKO)
		{
			pkt.read<uint8_t>();
		}

		pkt.readString(tPlayer.szKnights);

		tPlayer.iKnightsGrade = pkt.read<uint8_t>();
		tPlayer.iKnightsRank = pkt.read<uint8_t>();

		int16_t sMarkVersion = pkt.read<int16_t>();
		int16_t sCapeID = pkt.read<int16_t>();
		uint8_t iR = pkt.read<uint8_t>();
		uint8_t iG = pkt.read<uint8_t>();
		uint8_t iB = pkt.read<uint8_t>();

		pkt.read<uint8_t>();
		pkt.read<uint8_t>();
	}
	else
	{
		pkt.read<uint32_t>();
		pkt.read<uint16_t>();

		if (m_Bot->GetPlatformType() == PlatformType::USKO 
			|| m_Bot->GetPlatformType() == PlatformType::STKO)
		{
			pkt.read<uint8_t>();
		}

		int16_t sCapeID = pkt.read<int16_t>();
		uint8_t iR = pkt.read<uint8_t>();
		uint8_t iG = pkt.read<uint8_t>();
		uint8_t iB = pkt.read<uint8_t>();

		pkt.read<uint8_t>();
		pkt.read<uint8_t>();
	}

	tPlayer.iLevel = pkt.read<uint8_t>();

	tPlayer.eRace = (Race)pkt.read<uint8_t>();
	tPlayer.eClass = (Class)pkt.read<int16_t>();

	tPlayer.fX = (pkt.read<uint16_t>()) / 10.0f;
	tPlayer.fY = (pkt.read<uint16_t>()) / 10.0f;
	tPlayer.fZ = (pkt.read<int16_t>()) / 10.0f;

	tPlayer.iFace = pkt.read<uint8_t>();
	tPlayer.iHair = pkt.read<int32_t>();

	uint8_t iResHpType = pkt.read<uint8_t>();
	uint32_t iAbnormalType = pkt.read<uint32_t>();

	switch (iAbnormalType)
	{
		case ABNORMAL_NORMAL:
			tPlayer.bBlinking = false;
			break;
		case ABNORMAL_BLINKING:
			tPlayer.bBlinking = true;
			break;
		case ABNORMAL_BLINKING_END:
			tPlayer.bBlinking = false;
			break;
	}

	uint8_t iNeedParty = pkt.read<uint8_t>();

	tPlayer.iAuthority = pkt.read<uint8_t>();

	uint8_t iPartyLeader = pkt.read<uint8_t>();
	uint8_t iInvisibilityType = pkt.read<uint8_t>();
	uint8_t iTeamColor = pkt.read<uint8_t>();
	uint8_t iIsHidingHelmet = pkt.read<uint8_t>();
	uint8_t iIsHidingCospre = pkt.read<uint8_t>();
	uint8_t iIsDevil = pkt.read<uint8_t>();
	uint8_t iIsHidingWings = pkt.read<uint8_t>();

	int16_t iDirection = pkt.read<int16_t>();

	uint8_t iIsChicken = pkt.read<uint8_t>();
	uint8_t iRank = pkt.read<uint8_t>();
	int8_t iKnightsRank = pkt.read<int8_t>();
	int8_t iPersonalRank = pkt.read<int8_t>();

	if (m_Bot->GetPlatformType() == PlatformType::USKO 
		|| m_Bot->GetPlatformType() == PlatformType::STKO)
	{
		int32_t iLoop = m_bLunarWarDressUp ? 10 : 16;

		for (int32_t i = 0; i < iLoop; i++)
		{
			tPlayer.tInventory[i].iPos = i;
			tPlayer.tInventory[i].iItemID = pkt.read<uint32_t>();
			tPlayer.tInventory[i].iDurability = pkt.read<uint16_t>();
			tPlayer.tInventory[i].iFlag = pkt.read<uint8_t>();
		}
	}
	else if (m_Bot->GetPlatformType() == PlatformType::CNKO)
	{
		int32_t iLoop = m_bLunarWarDressUp ? 10 : 15;

		for (int32_t i = 0; i < iLoop; i++)
		{
			tPlayer.tInventory[i].iPos = i;
			tPlayer.tInventory[i].iItemID = pkt.read<uint32_t>();
			tPlayer.tInventory[i].iDurability = pkt.read<uint16_t>();
			tPlayer.tInventory[i].iFlag = pkt.read<uint8_t>();
		}
	}

	tPlayer.iCity = pkt.read<uint8_t>();

	if (m_Bot->GetPlatformType() == PlatformType::USKO 
		|| m_Bot->GetPlatformType() == PlatformType::STKO)
	{
		uint8_t iTempBuffer[26];
		pkt.read(iTempBuffer, 26);
	}
	else if (m_Bot->GetPlatformType() == PlatformType::CNKO)
	{
		uint8_t iTempBuffer[17];
		pkt.read(iTempBuffer, 17);
	}

	return tPlayer;
}

void ClientHandler::SetLoginInformation(std::string szAccountId, std::string szPassword)
{
	m_szAccountId = szAccountId;
	m_szPassword = szPassword;
}

void ClientHandler::LoadSkillData()
{
#ifdef DEBUG
	printf("Client::LoadSkillData: Start Load Character Skill Data\n");
#endif

	m_vecAvailableSkill.clear();

	std::map<uint32_t, __TABLE_UPC_SKILL>* pSkillTable;
	if (!m_Bot->GetSkillTable(&pSkillTable))
		return;

	bool bEnableTournamentSkills = false;

#ifdef FEATURE_TOURNAMENT_SKILLS
	bEnableTournamentSkills = true;
#endif

	for (const auto& [key, value] : *pSkillTable)
	{
		if ((bEnableTournamentSkills
			&& (value.iID != 490803 
				&& value.iID != 490811 
				&& value.iID != 490808 
				&& value.iID != 490809 
				&& value.iID != 490810 
				&& value.iID != 490800 
				&& value.iID != 490801
				&& value.iID != 490817))
			|| !bEnableTournamentSkills)
		{
			if (0 != std::to_string(value.iNeedSkill).substr(0, 3).compare(std::to_string(m_PlayerMySelf.eClass)))
				continue;

			if (value.iTarget != SkillTargetType::TARGET_SELF 
				&& value.iTarget != SkillTargetType::TARGET_PARTY 
				&& value.iTarget != SkillTargetType::TARGET_PARTY_ALL 
				&& value.iTarget != SkillTargetType::TARGET_FRIEND_WITHME 
				&& value.iTarget != SkillTargetType::TARGET_FRIEND_ONLY
				&& value.iTarget != SkillTargetType::TARGET_ENEMY_ONLY 
				&& value.iTarget != SkillTargetType::TARGET_AREA_ENEMY)
				continue;
		
				if ((value.iSelfAnimID1 == 153 || value.iSelfAnimID1 == 154) || (value.iSelfFX1 == 32038 || value.iSelfFX1 == 32039))
					continue;

				if (value.dwNeedItem == 811071000) //New Emotes
					continue;

				switch (value.iNeedSkill % 10)
				{
					case 0:
						if (value.iNeedLevel > m_PlayerMySelf.iLevel)
							continue;
						break;

					case 5:
						if (value.iNeedLevel > GetSkillPoint(0))
							continue;
						break;

					case 6:
						if (value.iNeedLevel > GetSkillPoint(1))
							continue;
						break;

					case 7:
						if (value.iNeedLevel > GetSkillPoint(2))
							continue;
						break;

					case 8:
						if (value.iNeedLevel > GetSkillPoint(3))
							continue;
						break;
				}
		}

		m_vecAvailableSkill.push_back(value);
	}
}

void ClientHandler::MoveToTargetProcess()
{
	try
	{
		if (Bot::TimeGet() < (m_fLastMoveToTargetProcessTime + (150.0f / 1000.0f)))
			return;

		if (!m_bAttackStatus)
			return;

		if (!m_bMoveToTarget)
			return;

		if (IsZoneChanging())
			return;

		DWORD iTargetID = GetTarget();

		if (iTargetID == -1)
			return;

		if (IsMovingToLoot())
			return;

		if (IsBlinking())
			return;

		if (IsRouting())
			return;

		if (m_PlayerMySelf.eState == PSA_DEATH)
			return;

		if (!m_bDisableStun)
		{
			if (IsStunned())
				return;
		}

		if (GetActionState() == PSA_SPELLMAGIC)
			return;
		
		if (!IsAttackable(iTargetID))
			return;

		std::map<uint32_t, __TABLE_ITEM>* pItemTable;
		if (!m_Bot->GetItemTable(&pItemTable))
			return;

		Vector3 v3TargetPosition = GetTargetPosition();

		if (v3TargetPosition == Vector3(0.0f, 0.0f, 0.0f))
			return;

		//TODO: Illegal selecte burasi kapali olcak dikkat
		DWORD iTargetBase = GetEntityBase(iTargetID);

		if (iTargetBase == 0)
			return;

		float fMySelfRadius = GetRadius() * GetScaleZ();
		float fTargetRadius = GetRadius(iTargetBase) * GetScaleZ(iTargetBase);
		float fDistLimit = (fMySelfRadius + fTargetRadius) / 2.0f;

		auto iLeftHandWeapon = GetInventoryItemSlot(6);
		auto iRightHandWeapon = GetInventoryItemSlot(8);

		if (iLeftHandWeapon.iItemID != 0)
		{
			uint32_t iLeftHandWeaponBaseID = iLeftHandWeapon.iItemID / 1000 * 1000;
			auto pLeftHandWeaponItemData = pItemTable->find(iLeftHandWeaponBaseID);

			if (pLeftHandWeaponItemData != pItemTable->end())
			{
				if (pLeftHandWeaponItemData->second.byKind == ITEM_CLASS_BOW
					|| pLeftHandWeaponItemData->second.byKind == ITEM_CLASS_BOW_LONG
					|| pLeftHandWeaponItemData->second.byKind == ITEM_CLASS_BOW_CROSS)
				{
					fDistLimit += 1.0f;
				}
				else
				{
					//fDistLimit += (float)pLeftHandWeaponItemData->second.siAttackRange / 10.0f;
					fDistLimit += 1.0f;
				}
			}
		}
		else if (iRightHandWeapon.iItemID != 0)
		{
			uint32_t iRightHandWeaponBaseID = iRightHandWeapon.iItemID / 1000 * 1000;
			auto pRightHandWeaponItemData = pItemTable->find(iRightHandWeaponBaseID);

			if (pRightHandWeaponItemData != pItemTable->end())
			{
				if (pRightHandWeaponItemData->second.byKind == ITEM_CLASS_BOW
					|| pRightHandWeaponItemData->second.byKind == ITEM_CLASS_BOW_LONG
					|| pRightHandWeaponItemData->second.byKind == ITEM_CLASS_BOW_CROSS)
				{
					fDistLimit += 1.0f;
				}
				else
				{
					//fDistLimit += (float)pRightHandWeaponItemData->second.siAttackRange / 10.0f;
					fDistLimit += 1.0f;
				}
			}
		}

		if (GetDistance(v3TargetPosition) > fDistLimit)
		{
			SetMovePosition(v3TargetPosition);
		}
		else
		{
			if (GetMoveState() != PSM_STOP)
			{
				SetMovePosition(Vector3(0.0f, 0.0f, 0.0f));
			}
		}

		m_fLastMoveToTargetProcessTime = Bot::TimeGet();
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("MoveToTargetProcess:Exception: %s\n", e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}
}

void ClientHandler::BasicAttackProcess()
{
	try
	{
		if (!m_bAttackStatus)
			return;

		if (!m_bBasicAttack)
			return;

		if (IsZoneChanging())
			return;

		if (IsMovingToLoot())
			return;

		if (IsBlinking())
			return;

		if (IsRouting())
			return;

		if (m_PlayerMySelf.eState == PSA_DEATH)
			return;

		if (m_bSkillCasting == true || GetActionState() == PSA_SPELLMAGIC)
			return;

		DWORD iTargetID = GetTarget();

		if (iTargetID == -1)
			return;

		if (!IsAttackable(iTargetID))
			return;

		std::map<uint32_t, __TABLE_ITEM>* pItemTable;
		if (!m_Bot->GetItemTable(&pItemTable))
			return;

		Vector3 v3TargetPosition = GetTargetPosition();

		float fDistance = GetDistance(v3TargetPosition);

		if (m_bAttackRangeLimit
			&& fDistance > (float)m_iAttackRangeLimitValue)
			return;

		float fBasicAttackIntervalTable = 1.0f;
		bool bIsEquippedBow = false;
		float fEffectiveAttackRange = 10.0f;

		auto iLeftHandWeapon = GetInventoryItemSlot(6);
		auto iRightHandWeapon = GetInventoryItemSlot(8);

		if (iRightHandWeapon.iItemID != 0)
		{
			uint32_t iRightHandWeaponBaseID = iRightHandWeapon.iItemID / 1000 * 1000;
			auto pRightHandWeaponItemData = pItemTable->find(iRightHandWeaponBaseID);

			if (pRightHandWeaponItemData != pItemTable->end())
			{
				if (pRightHandWeaponItemData->second.byKind == ITEM_CLASS_BOW
					|| pRightHandWeaponItemData->second.byKind == ITEM_CLASS_BOW_LONG
					|| pRightHandWeaponItemData->second.byKind == ITEM_CLASS_BOW_CROSS)
				{
					bIsEquippedBow = true;
				}

				fEffectiveAttackRange = (float)pRightHandWeaponItemData->second.siAttackRange;
			}
		}

		if (iLeftHandWeapon.iItemID != 0)
		{
			uint32_t iLeftHandWeaponBaseID = iLeftHandWeapon.iItemID / 1000 * 1000;

			auto pLeftHandWeaponItemData = pItemTable->find(iLeftHandWeaponBaseID);

			if (pLeftHandWeaponItemData != pItemTable->end())
			{
				if (pLeftHandWeaponItemData->second.byKind == ITEM_CLASS_BOW
					|| pLeftHandWeaponItemData->second.byKind == ITEM_CLASS_BOW_LONG
					|| pLeftHandWeaponItemData->second.byKind == ITEM_CLASS_BOW_CROSS)
				{
					bIsEquippedBow = true;
				}

				std::map<uint32_t, __TABLE_ITEM_EXTENSION>* pItemExtensionTable;
				if (m_Bot->GetItemExtensionTable(pLeftHandWeaponItemData->second.byExtIndex, &pItemExtensionTable))
				{
					auto pLeftHandWeaponItemExtensionData = pItemExtensionTable->find(iLeftHandWeapon.iItemID % 1000);

					if (pLeftHandWeaponItemExtensionData != pItemExtensionTable->end())
					{
						fBasicAttackIntervalTable = (pLeftHandWeaponItemData->second.siAttackInterval / 100.0f)
							* (pLeftHandWeaponItemExtensionData->second.iAttackIntervalPercentage / 100.0f);
					}
				}
			}

			fEffectiveAttackRange = (float)pLeftHandWeaponItemData->second.siAttackRange;
		}

		if (!bIsEquippedBow)
		{
			float fAttackInterval = fBasicAttackIntervalTable;

			if (Bot::TimeGet() > m_fAttackTimeRecent + fAttackInterval)
			{
				if (m_bBasicAttackWithPacket)
				{
					BasicAttackWithPacket(iTargetID, fAttackInterval);
				}
				else
				{
					BasicAttack();
				}
			}
		}
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("BasicAttackPacketProcess:Exception: %s\n", e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}
}

void ClientHandler::AttackProcess()
{
	try
	{
		if (m_bAttackSpeed)
		{
			if (m_iAttackSpeedValue > 0)
			{
				if (Bot::TimeGet() < (m_fLastAttackTime + (m_iAttackSpeedValue / 1000.0f)))
					return;
			}
		}
		else 
		{
			if (Bot::TimeGet() < (m_fLastAttackTime + (1000 / 1000.0f)))
				return;
		}

		if (!m_bAttackStatus)
			return;

		if (m_vecAttackSkillList.size() == 0)
			return;

		if (IsZoneChanging())
			return;

		if (IsMovingToLoot())
			return;

		if (IsBlinking())
			return;

		if (IsRouting())
			return;

		if (m_PlayerMySelf.eState == PSA_DEATH)
			return;

		std::map<uint32_t, __TABLE_UPC_SKILL>* pSkillTable;
		if (!m_Bot->GetSkillTable(&pSkillTable))
			return;

		std::map<uint32_t, __TABLE_UPC_SKILL_EXTENSION2>* pSkillExtension2;
		if (!m_Bot->GetSkillExtension2Table(&pSkillExtension2))
			return;

		std::map<uint32_t, __TABLE_ITEM>* pItemTable;
		if (!m_Bot->GetItemTable(&pItemTable))
			return;

		if (GetActionState() == PSA_SPELLMAGIC)
			return;

		DWORD iTargetID = GetTarget();

		if (iTargetID == -1)
			return;

		if (!IsAttackable(iTargetID))
			return;

		if (m_vecAttackSkillList.size() == 0)
			return;

		//Illegal Selectte burasi kapali olacak dikkat et
		DWORD iTargetBase = GetEntityBase(iTargetID);

		if (iTargetBase == 0)
			return;

		float fTargetRadius = GetRadius(iTargetBase) * GetScaleZ(iTargetBase);
		float fMySelfRadius = GetRadius() * GetScaleZ();

		Vector3 v3TargetPosition = GetTargetPosition();

		float fDistance = GetDistance(v3TargetPosition);

		if (m_qAttackSkillQueue.size() == 0)
		{
			auto iLeftHandWeapon = GetInventoryItemSlot(6);
			auto iRightHandWeapon = GetInventoryItemSlot(8);

			float fEffectiveAttackRange = 10.0f;

			bool bIsEquippedBow = false;

			if (iRightHandWeapon.iItemID != 0)
			{
				uint32_t iRightHandWeaponBaseID = iRightHandWeapon.iItemID / 1000 * 1000;
				auto pRightHandWeaponItemData = pItemTable->find(iRightHandWeaponBaseID);

				if (pRightHandWeaponItemData != pItemTable->end())
				{
					if (pRightHandWeaponItemData->second.byKind == ITEM_CLASS_BOW
						|| pRightHandWeaponItemData->second.byKind == ITEM_CLASS_BOW_LONG
						|| pRightHandWeaponItemData->second.byKind == ITEM_CLASS_BOW_CROSS)
					{
						bIsEquippedBow = true;
					}

					fEffectiveAttackRange = (float)pRightHandWeaponItemData->second.siAttackRange;
				}
			}

			if (iLeftHandWeapon.iItemID != 0)
			{
				uint32_t iLeftHandWeaponBaseID = iLeftHandWeapon.iItemID / 1000 * 1000;

				auto pLeftHandWeaponItemData = pItemTable->find(iLeftHandWeaponBaseID);

				if (pLeftHandWeaponItemData != pItemTable->end())
				{
					if (pLeftHandWeaponItemData->second.byKind == ITEM_CLASS_BOW
						|| pLeftHandWeaponItemData->second.byKind == ITEM_CLASS_BOW_LONG
						|| pLeftHandWeaponItemData->second.byKind == ITEM_CLASS_BOW_CROSS)
					{
						bIsEquippedBow = true;
					}

					fEffectiveAttackRange = (float)pLeftHandWeaponItemData->second.siAttackRange;
				}
			}

			for (const auto& iSkillID : m_vecAttackSkillList)
			{
				auto pSkillData = pSkillTable->find(iSkillID);

				if (pSkillData == pSkillTable->end())
					continue;

				if (IsSkillHasZoneLimit(iSkillID))
					continue;

				float fCurrentTime = Bot::TimeGet();
				float fSkillNextUseTime = GetSkillNextUseTime(iSkillID);

				if (fCurrentTime < fSkillNextUseTime)
					continue;

				if (m_bSkillCasting == true && pSkillData->second.iReCastTime > 0)
					continue;

				uint32_t iNeedItem = pSkillData->second.dwNeedItem;

				uint32_t iNeedItemCount = 1;
				uint32_t iExistItemCount = 0;

				if (iNeedItem != 0)
				{
					iExistItemCount = GetInventoryItemCount(iNeedItem);

					auto pSkillExtension2Data = pSkillExtension2->find(pSkillData->second.iID);

					if (pSkillExtension2Data != pSkillExtension2->end())
						iNeedItemCount = pSkillExtension2Data->second.iArrowCount;
				}

				if (m_PlayerMySelf.iMSP < pSkillData->second.iExhaustMSP
					|| (iNeedItem != 0 && iExistItemCount < iNeedItemCount))
				{
					continue;
				}

				if ((pSkillData->second.iID == 490801
					|| pSkillData->second.iID == 490800
					|| pSkillData->second.iID == 490817))
				{
					if (GetRegionUserCount(true) > 0)
						continue;
				}

				if (pSkillData->second.IsDeBuff())
				{
					int16_t iTargetHp = GetHp(iTargetBase);
					int16_t iTargetMaxHp = GetMaxHp(iTargetBase);

					if (iTargetMaxHp != 0)
					{
						int32_t iTargetHpPercent = (int32_t)std::ceil((iTargetHp * 100) / iTargetMaxHp);
						int32_t iTargetHpNeededValue = 60;

						if (iTargetHpPercent < iTargetHpNeededValue)
							continue;
					}
				}

				if (!bIsEquippedBow
					&& pSkillData->second.dw1stTableType == 2)
				{
					continue;
				}

				if (m_bAttackRangeLimit
					&& fDistance > (float)m_iAttackRangeLimitValue)
					continue;

				if (!m_bUseSkillWithPacket)
				{
					if (pSkillData->second.iValidDist > 0
						&& fDistance > (float)pSkillData->second.iValidDist)
						continue;

					float fDistanceLimit = (fMySelfRadius + fTargetRadius) / 2.0f;
					float fCalculatedEffectiveAttackRange = fEffectiveAttackRange / 10.0f;
					fDistanceLimit += fCalculatedEffectiveAttackRange;

					if (pSkillData->second.iValidDist == 0
						&& fDistance > fDistanceLimit)
						continue;
				}

				m_qAttackSkillQueue.push(pSkillData->second);
			}
		}

		if (m_qAttackSkillQueue.size() > 0)
		{
			TABLE_UPC_SKILL pSkillData = m_qAttackSkillQueue.front();

			if (m_bSkillCasting == true && pSkillData.iReCastTime > 0)
				return;

			if ((pSkillData.iID == 490801
				|| pSkillData.iID == 490800
				|| pSkillData.iID == 490817))
			{
				if (GetRegionUserCount(true) > 0)
				{
					m_qAttackSkillQueue.pop();
					return;
				}
			}

			uint32_t iNeedItem = pSkillData.dwNeedItem;

			uint32_t iNeedItemCount = 1;
			uint32_t iExistItemCount = 0;

			if (iNeedItem != 0)
			{
				iExistItemCount = GetInventoryItemCount(iNeedItem);

				auto pSkillExtension2Data = pSkillExtension2->find(pSkillData.iID);

				if (pSkillExtension2Data != pSkillExtension2->end())
					iNeedItemCount = pSkillExtension2Data->second.iArrowCount;
			}

			if (m_PlayerMySelf.iMSP < pSkillData.iExhaustMSP
				|| (iNeedItem != 0 && iExistItemCount < iNeedItemCount))
			{
				m_qAttackSkillQueue.pop();
				return;
			}

			if (pSkillData.IsDeBuff())
			{
				int16_t iTargetHp = GetHp(iTargetBase);
				int16_t iTargetMaxHp = GetMaxHp(iTargetBase);

				if (iTargetMaxHp != 0)
				{
					int32_t iTargetHpPercent = (int32_t)std::ceil((iTargetHp * 100) / iTargetMaxHp);
					int32_t iTargetHpNeededValue = 60;

					if (iTargetHpPercent < iTargetHpNeededValue)
					{
						m_qAttackSkillQueue.pop();
						return;
					}
				}
			}

			if (m_bUseSkillWithPacket
				|| (pSkillData.iID == 490808
					|| pSkillData.iID == 490809
					|| pSkillData.iID == 490810
					|| pSkillData.iID == 490800
					|| pSkillData.iID == 490801
					|| pSkillData.iID == 490817))
			{
				new std::thread([=]() { UseSkillWithPacket(pSkillData, GetTarget()); });
			}
			else
			{
				UseSkill(pSkillData, GetTarget(), 0, false);

				if (m_bBasicAttack
					&& !pSkillData.IsDeBuff())
				{
					BasicAttack();
					BasicAttack();
				}
			}

			m_qAttackSkillQueue.pop();
		}

		m_fLastAttackTime = Bot::TimeGet();
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("AttackProcess:Exception: %s\n", e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}
}

void ClientHandler::SearchTargetProcess()
{
	try
	{
		if (m_bSearchTargetSpeed)
		{
			if (m_iSearchTargetSpeedValue > 0)
			{
				if (Bot::TimeGet() < (m_fLastSearchTargetTime + (m_iSearchTargetSpeedValue / 1000.0f)))
					return;
			}
		}
		else
		{
			if (Bot::TimeGet() < (m_fLastSearchTargetTime + (100.0f / 1000.0f)))
				return;
		}

		if (!m_bAttackStatus)
			return;

		if (IsZoneChanging())
			return;

		if (IsBlinking())
			return;

		if (IsMovingToLoot())
			return;

		if (IsRouting())
			return;

		if (m_PlayerMySelf.eState == PSA_DEATH)
			return;

		if (GetActionState() == PSA_SPELLMAGIC)
			return;

		if (!m_bClosestTarget)
		{
			DWORD iTargetID = GetTarget();

			if ((iTargetID != -1 && IsAttackable(iTargetID)))
				return;
		}

		std::map<uint32_t, __TABLE_ITEM>* pItemTable;
		if (!m_Bot->GetItemTable(&pItemTable))
			return;

		std::vector<TNpc> vecFilteredTarget;

		if (m_bAutoTarget)
		{
			std::copy_if(m_vecNpc.begin(), m_vecNpc.end(),
				std::back_inserter(vecFilteredTarget),
				[&](const TNpc& c)
				{
					return
						(c.iMonsterOrNpc == 1 || (c.iMonsterOrNpc == 2 && c.iFamilyType == 171))
						&& c.iProtoID != 0
						&& c.iProtoID != 9009
						&& c.eState != PSA_DYING
						&& c.eState != PSA_DEATH
						&& ((c.iHPMax == 0) || (c.iHPMax != 0 && c.iHP > 0))
						&& ((m_bRangeLimit && GetDistance(Vector3(c.fX, c.fZ, c.fY)) <= (float)m_iRangeLimitValue) || !m_bRangeLimit);
				});
		}
		else
		{
			if (m_vecSelectedNpcIDList.size() > 0)
			{
				std::copy_if(m_vecNpc.begin(), m_vecNpc.end(),
					std::back_inserter(vecFilteredTarget),
					[&](const TNpc& c)
					{
						return
							((c.iMonsterOrNpc == 1 || (c.iMonsterOrNpc == 2 && c.iFamilyType == 171))
								&& c.iProtoID != 0
								&& c.iProtoID != 9009
								&& c.eState != PSA_DYING
								&& c.eState != PSA_DEATH
								&& ((c.iHPMax == 0) || (c.iHPMax != 0 && c.iHP > 0))
								&& m_vecSelectedNpcIDList.count(c.iID) > 0);
					});
			}
			else
			{
				std::copy_if(m_vecNpc.begin(), m_vecNpc.end(),
					std::back_inserter(vecFilteredTarget),
					[&](const TNpc& c)
					{
						return
							((c.iMonsterOrNpc == 1 || (c.iMonsterOrNpc == 2 && c.iFamilyType == 171))
								&& c.iProtoID != 0
								&& c.iProtoID != 9009
								&& c.eState != PSA_DYING
								&& c.eState != PSA_DEATH
								&& ((c.iHPMax == 0) || (c.iHPMax != 0 && c.iHP > 0))
								&& ((m_bRangeLimit && GetDistance(Vector3(c.fX, c.fZ, c.fY)) <= (float)m_iRangeLimitValue) || !m_bRangeLimit)
								&& m_vecSelectedNpcList.count(c.iProtoID) > 0);
					});
			}	
		}

		if (vecFilteredTarget.size() > 0)
		{
			Vector3 v3CurrentPosition = GetPosition();

			struct DistanceList 
			{
				float fDistance;
				TNpc pNpc;
			};

			auto pSort = [&](const DistanceList& a, const DistanceList& b)
				{
					return a.fDistance < b.fDistance;
				};

			std::vector<DistanceList> vecDistanceList;
			vecDistanceList.reserve(vecFilteredTarget.size());

			for (const auto& pNpc : vecFilteredTarget)
			{
				float fDistance = GetDistance(v3CurrentPosition, Vector3(pNpc.fX, pNpc.fZ, pNpc.fY));
				vecDistanceList.push_back({ fDistance, pNpc });
			}

			std::sort(vecDistanceList.begin(), vecDistanceList.end(), pSort);

			if (vecDistanceList[0].pNpc.iID != GetTarget())
			{
				SetTarget(vecDistanceList[0].pNpc.iID);

				if (m_bTargetSizeEnable)
				{
					DWORD iTargetBase = GetEntityBase(vecDistanceList[0].pNpc.iID);

					if (GetScaleZ(iTargetBase) != (float)m_iTargetSize)
					{
						SetScale(iTargetBase, (float)m_iTargetSize, (float)m_iTargetSize, (float)m_iTargetSize);
					}
				}
			}
		}

		m_fLastSearchTargetTime = Bot::TimeGet();
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("SearchTargetProcess:Exception: %s\n", e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}
}

void ClientHandler::AutoLootProcess()
{
	try
	{
		if (Bot::TimeGet() < (m_fLastAutoLootProcessTime + (100.0f / 1000.0f)))
			return;

		if (!m_bAutoLoot)
			return;

		if (IsZoneChanging())
			return;

		if (IsBlinking())
			return;

		if (IsRouting())
			return;

		if (m_PlayerMySelf.eState == PSA_DEATH)
			return;

		if (IsStunned())
			return;

		std::vector<TLoot> vecFilteredLoot;
		std::copy_if(m_vecLootList.begin(), m_vecLootList.end(),
			std::back_inserter(vecFilteredLoot),
			[&](const TLoot& c)
			{
				return 
					(Bot::TimeGet() - c.fDropTime < 30000.0f / 1000.0f && Bot::TimeGet() - c.fDropTime > 1000.0f / 1000.0f);
			});

		if (vecFilteredLoot.size() == 0)
		{
			if (IsMovingToLoot())
			{
				SetMovingToLoot(false);
			}
				
			return;
		}

		auto pSort = [=](TLoot const& a, TLoot const& b)
		{
			auto fADistance = GetDistance(Vector3(a.fNpcX, a.fNpcZ, a.fNpcY), GetPosition());
			auto fBDistance = GetDistance(Vector3(b.fNpcX, b.fNpcZ, b.fNpcY), GetPosition());

			if (fADistance != fBDistance)
			{
				return fADistance < fBDistance;
			}

			return false;
		};

		std::sort(vecFilteredLoot.begin(), vecFilteredLoot.end(), pSort);

		auto& pFindedLoot = vecFilteredLoot.front();

		Vector3 vec3NpcPosition = Vector3(pFindedLoot.fNpcX, pFindedLoot.fNpcZ, pFindedLoot.fNpcY);

		if (GetDistance(vec3NpcPosition) > 10.0f
			&& GetDistance(vec3NpcPosition) < 50.0f)
		{
			if (m_bMoveToLoot) 
			{
				if (GetActionState() == PSA_SPELLMAGIC || IsStunned())
					return;

				SetMovePosition(vec3NpcPosition);
				SetMovingToLoot(true);
			}
		}
		else
		{
			if (Bot::TimeGet() > (m_fLastAutoLootBundleOpenTime + (200.0f / 1000.0f)))
			{
				SendRequestBundleOpen(pFindedLoot.iBundleID);
				m_fLastAutoLootBundleOpenTime = Bot::TimeGet();
			}
		}

		m_fLastAutoLootProcessTime = Bot::TimeGet();
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("AutoLootProcess:Exception: %s\n", e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}
}

void ClientHandler::MinorProcess()
{
	try
	{
		if (Bot::TimeGet() < (m_fLastMinorProcessTime + (500.0f / 1000.0f)))
			return;

		if (!m_bCharacterStatus)
			return;

		if (!m_bMinorProtection)
			return;

		if (IsZoneChanging())
			return;

		if (IsBlinking())
			return;

		if (m_PlayerMySelf.eState == PSA_DEATH)
			return;

		if (m_PlayerMySelf.iHP <= 0
			|| m_PlayerMySelf.iHPMax <= 0)
		{
			return;
		}

		if ((int32_t)std::ceil((m_PlayerMySelf.iHP * 100) / m_PlayerMySelf.iHPMax) < m_iHpProtectionValue)
		{
			auto it = std::find_if(m_vecAvailableSkill.begin(), m_vecAvailableSkill.end(),
				[](const TABLE_UPC_SKILL& a) { return a.iBaseId == 107705; });

			if (it == m_vecAvailableSkill.end())
				return;

			if (m_PlayerMySelf.iMSP < it->iExhaustMSP)
				return;

			if (!m_bOnlyAttackSkillUseWithPacket
				&& m_bUseSkillWithPacket)
			{
				new std::thread([=]() { UseSkillWithPacket(*it, m_PlayerMySelf.iID); });
			}
			else
			{
				UseSkill(*it, m_PlayerMySelf.iID, 1);
			}

			m_fLastMinorProcessTime = Bot::TimeGet();
		}
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("MinorProcess:Exception: %s\n", e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}
}

void ClientHandler::PotionProcess()
{
	try
	{
		if (Bot::TimeGet() < (m_fLastPotionProcessTime + (500.0f / 1000.0f)))
			return;

		if (!m_bCharacterStatus)
			return;

		if (IsZoneChanging())
			return;

		if (IsBlinking())
			return;

		if (m_PlayerMySelf.eState == PSA_DEATH)
			return;

		if (m_bHpProtectionEnable)
		{
			if (m_PlayerMySelf.iHP > 0 && m_PlayerMySelf.iHPMax > 0)
			{
				int32_t iHpProtectionPercent = (int32_t)std::ceil((m_PlayerMySelf.iHP * 100) / m_PlayerMySelf.iHPMax);

				if (iHpProtectionPercent <= m_iHpProtectionValue)
				{
					HealthPotionProcess();
				}
			}
		}

		if (m_bMpProtectionEnable)
		{
			if (m_PlayerMySelf.iMSP > 0 && m_PlayerMySelf.iMSPMax > 0)
			{
				int32_t iMpProtectionPercent = (int32_t)std::ceil((m_PlayerMySelf.iMSP * 100) / m_PlayerMySelf.iMSPMax);

				if (iMpProtectionPercent <= m_iMpProtectionValue)
				{
					ManaPotionProcess();
				}
			}
		}

		m_fLastPotionProcessTime = Bot::TimeGet();
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("PotionProcess:Exception: %s\n", e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}
}

void ClientHandler::MagicHammerProcess()
{
	try
	{
		if (Bot::TimeGet() < (m_fLastMagicHammerProcessTime + (5000.0f / 1000.0f)))
			return;

		if (!m_bAutoRepairMagicHammer)
			return;

		if (IsZoneChanging())
			return;

		if (IsBlinking())
			return;

		if (m_PlayerMySelf.eState == PSA_DEATH)
			return;

		if (!IsNeedRepair())
			return;

		std::vector<uint32_t> vecMagicHammers =
		{
			379099000, 399288000, 399291000, 914013000,
			931773000
		};

		auto pSort = [](uint32_t const& a, uint32_t const& b)
			{
				return a < b;
			};

		std::sort(vecMagicHammers.begin(), vecMagicHammers.end(), pSort);

		TItemData tInventoryItem;

		for (const auto& x : vecMagicHammers)
		{
			tInventoryItem = GetInventoryItem(x);

			if (tInventoryItem.iItemID != 0)
			{
				UseItem(tInventoryItem.iItemID);
				break;
			}
		}

		m_fLastMagicHammerProcessTime = Bot::TimeGet();
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("MagicHammerProcess:Exception: %s\n", e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}
}

void ClientHandler::SpeedHackProcess()
{
	try
	{
		if (Bot::TimeGet() < (m_fLastSpeedHackProcessTime + (500.0f / 1000.0f)))
			return;

		if (!m_bSpeedHack)
			return;

		if (IsZoneChanging())
			return;

		if (m_PlayerMySelf.eState == PSA_DEATH)
			return;

		if (GetActionState() == PSA_SPELLMAGIC)
			return;

		if (IsStunned())
			return;

		if (GetCharacterSpeed() != 1.5)
		{
			SetCharacterSpeed(1.5);
		}

		Vector3 vecMoveCoordinate = GetPosition();

		if (GetMoveState() != 0)
		{
			SendMovePacket(vecMoveCoordinate, vecMoveCoordinate, 45, GetMoveState());
		}

		m_fLastSpeedHackProcessTime = Bot::TimeGet();
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("SpeedHackProcess:Exception: %s\n", e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}
}

void ClientHandler::TransformationProcess()
{
	try
	{
		if (Bot::TimeGet() < (m_fLastTransformationProcessTime + (5000.0f / 1000.0f)))
			return;

		if (IsZoneChanging())
			return;

		if (IsBlinking())
			return;

		if (m_PlayerMySelf.eState == PSA_DEATH)
			return;

		if (m_bAttackStatus || m_bCharacterStatus)
		{
			if (m_bAutoTransformation
				&& m_iTransformationSkill > 0
				&& IsTransformationAvailable())
			{
				std::map<uint32_t, __TABLE_UPC_SKILL>* pSkillTable;
				if (m_Bot->GetSkillTable(&pSkillTable))
				{
					switch (m_iTransformationItem)
					{
					case 379090000:
					{
						auto pTransformationScroll = pSkillTable->find(470001);
						auto pTransformationSkill = pSkillTable->find(m_iTransformationSkill);

						if (pTransformationScroll != pSkillTable->end()
							&& pTransformationSkill != pSkillTable->end())
						{
							if (GetInventoryItemCount(pTransformationScroll->second.dwNeedItem) > 0
								&& m_PlayerMySelf.iLevel >= pTransformationSkill->second.iNeedLevel)
							{
								new std::thread([=]() 
								{ 
									UseSkillWithPacket(pTransformationScroll->second, m_PlayerMySelf.iID);
									UseSkillWithPacket(pTransformationSkill->second, m_PlayerMySelf.iID);
								});
							}
						}
					}
					break;

					case 379093000:
					{
						auto pTransformationScroll = pSkillTable->find(471001);
						auto pTransformationSkill = pSkillTable->find(m_iTransformationSkill);

						if (pTransformationScroll != pSkillTable->end()
							&& pTransformationSkill != pSkillTable->end())
						{
							if (GetInventoryItemCount(pTransformationScroll->second.dwNeedItem) > 0
								&& m_PlayerMySelf.iLevel >= pTransformationSkill->second.iNeedLevel)
							{
								new std::thread([=]()
								{
									UseSkillWithPacket(pTransformationScroll->second, m_PlayerMySelf.iID);
									UseSkillWithPacket(pTransformationSkill->second, m_PlayerMySelf.iID);
								});
							}
						}
					}
					break;

					case 381001000:
					{
						auto pTransformationScroll = pSkillTable->find(472001);
						auto pTransformationSkill = pSkillTable->find(m_iTransformationSkill);

						if (pTransformationScroll != pSkillTable->end()
							&& pTransformationSkill != pSkillTable->end())
						{
							if (GetInventoryItemCount(pTransformationScroll->second.dwNeedItem) > 0
								&& m_PlayerMySelf.iLevel >= pTransformationSkill->second.iNeedLevel)
							{
								new std::thread([=]()
								{
									UseSkillWithPacket(pTransformationScroll->second, m_PlayerMySelf.iID);
									UseSkillWithPacket(pTransformationSkill->second, m_PlayerMySelf.iID);
								});
							}
						}
					}
					break;

					case -1:
					{
						auto pTransformationScroll = pSkillTable->find(472001);
						auto pTransformationSkill = pSkillTable->find(m_iTransformationSkill);

						if (pTransformationScroll != pSkillTable->end()
							&& pTransformationSkill != pSkillTable->end())
						{
							if (GetInventoryItemCount(pTransformationScroll->second.dwNeedItem) > 0
								&& m_PlayerMySelf.iLevel >= pTransformationSkill->second.iNeedLevel)
							{
								new std::thread([=]()
									{
										UseSkillWithPacket(pTransformationScroll->second, m_PlayerMySelf.iID);
										UseSkillWithPacket(pTransformationSkill->second, m_PlayerMySelf.iID);
									});
							}
						}
					}
					break;
					}
				}
			}
		}

		m_fLastTransformationProcessTime = Bot::TimeGet();
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("TransformationProcess:Exception: %s\n", e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}
}

void ClientHandler::FlashProcess()
{
	try
	{
		if (IsZoneChanging())
			return;

		if (IsBlinking())
			return;

		if (m_PlayerMySelf.eState == PSA_DEATH)
			return;

		if (Bot::TimeGet() < (m_fLastFlashProcessTime + (500.0f / 1000.0f)))
			return;

		if (m_bAttackStatus || m_bCharacterStatus)
		{
			if (m_bAutoDCFlash)
			{
				TItemData tInventoryItem = GetInventoryItem(399128000);

				if (tInventoryItem.iItemID != 0
					&& m_iAutoDCFlashCount != m_iFlashCount)
				{
					UseItem(tInventoryItem.iItemID);
				}
			}

			if (m_bAutoWarFlash)
			{
				TItemData tInventoryItem = GetInventoryItem(399129000);

				if (tInventoryItem.iItemID != 0
					&& m_iAutoWarFlashCount != m_iFlashCount)
				{
					UseItem(tInventoryItem.iItemID);
				}
			}

			if (m_bAutoExpFlash)
			{
				TItemData tInventoryItem = GetInventoryItem(399127000);

				if (tInventoryItem.iItemID != 0
					&& m_iAutoExpFlashCount != m_iFlashCount)
				{
					UseItem(tInventoryItem.iItemID);
				}
			}
		}

		m_fLastFlashProcessTime = Bot::TimeGet();
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("FlashProcess:Exception: %s\n", e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}
}

void ClientHandler::RegionProcess()
{
	try
	{
		if (Bot::TimeGet() < (m_fLastRegionProcessTime + (500.0f / 1000.0f)))
			return;

		if (IsZoneChanging())
			return;

		if (IsBlinking())
			return;

		if (m_PlayerMySelf.eState == PSA_DEATH)
			return;

		if (!IsRouting()
			&& m_bStartGenieIfUserInRegion
			&& (m_bAttackStatus || m_bCharacterStatus))
		{
			std::chrono::milliseconds msNow = duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch()
			);

			if (GetRegionUserCount(true, (float)m_iStartGenieIfUserInRegionMeter) > 0
				&& m_msLastGenieStartTime == std::chrono::milliseconds(0))
			{
				SendStartGenie();

				m_msLastGenieStartTime = msNow;
			}
			else if (GetRegionUserCount(true, (float)m_iStartGenieIfUserInRegionMeter) == 0
				&& m_msLastGenieStartTime != std::chrono::milliseconds(0)
				&& (m_msLastGenieStartTime.count() + 180000) < msNow.count())
			{
				SendStopGenie();

				m_msLastGenieStartTime = std::chrono::milliseconds(0);
			}
		}

		m_fLastRegionProcessTime = Bot::TimeGet();
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("RegionProcess:Exception: %s\n", e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}
}

void ClientHandler::RouteRecorderProcess()
{
	try
	{
		if (!m_bIsRoutePlanning)
			return;

		if (m_vecRoutePlan.size() == 0)
		{
			Route pAutoRoute{};

			pAutoRoute.fX = GetX();
			pAutoRoute.fY = GetY();
			pAutoRoute.eStepType = RouteStepType::STEP_MOVE;

			m_vecRoutePlan.push_back(pAutoRoute);
		}
		else
		{
			Route pLastRoute = m_vecRoutePlan.back();

			float fDistance = GetDistance(Vector3(pLastRoute.fX, 0.0f, pLastRoute.fY));

			if (fDistance > 3.0f)
			{
				Route pAutoRoute{};
				pAutoRoute.fX = GetX();
				pAutoRoute.fY = GetY();
				pAutoRoute.eStepType = RouteStepType::STEP_MOVE;

				m_vecRoutePlan.push_back(pAutoRoute);
			}
		}
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("RouteRecorderProcess:Exception: %s\n", e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}
}

void ClientHandler::CharacterProcess()
{
	try
	{
		if (Bot::TimeGet() < (m_fLastCharacterProcessTime + (300.0f / 1000.0f)))
			return;

		if (!m_bCharacterStatus)
			return;

		if (IsZoneChanging())
			return;

		if (IsBlinking())
			return;

		if (m_PlayerMySelf.eState == PSA_DEATH)
			return;

		std::map<uint32_t, __TABLE_UPC_SKILL>* pSkillTable;
		if (!m_Bot->GetSkillTable(&pSkillTable))
			return;

		std::map<uint32_t, __TABLE_UPC_SKILL_EXTENSION2>* pSkillExtension2;
		if (!m_Bot->GetSkillExtension2Table(&pSkillExtension2))
			return;

		std::map<uint32_t, __TABLE_UPC_SKILL_EXTENSION3>* pSkillExtension3;
		if (!m_Bot->GetSkillExtension3Table(&pSkillExtension3))
			return;

		std::map<uint32_t, __TABLE_UPC_SKILL_EXTENSION4>* pSkillExtension4;
		if (!m_Bot->GetSkillExtension4Table(&pSkillExtension4))
			return;

		if (GetActionState() == PSA_SPELLMAGIC)
			return;

		if (m_pCurrentRunningRoute != nullptr
			&& (m_pCurrentRunningRoute->eStepType == RouteStepType::STEP_SUNDRIES
				|| m_pCurrentRunningRoute->eStepType == RouteStepType::STEP_POTION
				|| m_pCurrentRunningRoute->eStepType == RouteStepType::STEP_INN
				|| m_pCurrentRunningRoute->eStepType == RouteStepType::STEP_DC_SUNDRIES))
			return;

		if (m_vecCharacterSkillList.size() == 0)
			return;

		std::vector<int> vecFilteredCharacterSkillList;

		std::copy_if(m_vecCharacterSkillList.begin(), m_vecCharacterSkillList.end(),
			std::back_inserter(vecFilteredCharacterSkillList),
			[&](const int& iSkillID)
			{
				auto pSkillData = pSkillTable->find(iSkillID);

				if (pSkillData == pSkillTable->end())
					return false;

				if (IsSkillHasZoneLimit(iSkillID))
					return false;

				float fCurrentTime = Bot::TimeGet();
				float fSkillNextUseTime = GetSkillNextUseTime(iSkillID);

				if (fCurrentTime < fSkillNextUseTime)
					return false;

				if (m_bSkillCasting == true && pSkillData->second.iReCastTime > 0)
					return false;

				if (IsRouting()
					&& pSkillData->second.iBaseId != 107725		//Light Feet
					&& pSkillData->second.iBaseId != 107010		//Swift
					&& pSkillData->second.iBaseId != 101001		//Sprint
					&& pSkillData->second.iBaseId != 490811)	//Light Feet [Tournament Use Only]
				{
					return false;
				}

				if (pSkillData->second.iBaseId == 107010)		//Swift				
				{
					if (m_vecPartyMembers.size() > 0
						&& m_bPartySwift)
					{
						if (PartyMemberNeedSwift() == -1)
							return false;
					}
					else
					{
						auto pSkillExtension4Data = pSkillExtension4->find(pSkillData->second.iID);

						if (pSkillExtension4Data != pSkillExtension4->end())
						{
							if (IsBuffActive(pSkillExtension4Data->second.iBuffType))
								return false;
						}
					}
				}
				else if ((
					pSkillData->second.iBaseId == 111545 ||		//Superior Healing
					pSkillData->second.iBaseId == 111536 ||		//Massive Healing
					pSkillData->second.iBaseId == 111527 ||		//Great Healing
					pSkillData->second.iBaseId == 111518 ||		//Major Healing
					pSkillData->second.iBaseId == 111509 ||		//Healing
					pSkillData->second.iBaseId == 107705))		//Minor Healing
					
				{
					if (m_vecPartyMembers.size() > 0 
						&& m_bPriestPartyHeal)
					{
						if (PartyMemberNeedHeal(pSkillData->second.iBaseId) == -1)
							return false;
					}
					else
					{
						if (m_bHealProtection)
						{
							if (m_PlayerMySelf.iHP <= 0 
								|| m_PlayerMySelf.iHPMax <= 0)
							{
								return false;
							}

							int32_t iHpProtectionPercent = (int32_t)std::ceil((m_PlayerMySelf.iHP * 100) / m_PlayerMySelf.iHPMax);

							if (m_iHealProtectionValue < iHpProtectionPercent)
								return false;
						}
						else
						{
							auto pSkillExtension3Data = pSkillExtension3->find(pSkillData->second.iID);

							if (pSkillExtension3Data != pSkillExtension3->end())
							{
								int32_t iNeedHealValue = m_PlayerMySelf.iHPMax - m_PlayerMySelf.iHP;

								if (iNeedHealValue <= 0 
									|| iNeedHealValue < pSkillExtension3Data->second.iHealValue)
									return false;
							}
						}
					}
				}
				else if ((
					pSkillData->second.iBaseId == 112675 ||		//Superioris
					pSkillData->second.iBaseId == 112670 ||		//Imposingness
					pSkillData->second.iBaseId == 111657 ||		//massiveness
					pSkillData->second.iBaseId == 111655 ||		//Heapness
					pSkillData->second.iBaseId == 111654 ||		//Undying
					pSkillData->second.iBaseId == 111642 ||		//Mightness
					pSkillData->second.iBaseId == 111633 ||		//Hardness
					pSkillData->second.iBaseId == 111624 ||		//Strong
					pSkillData->second.iBaseId == 111615 ||		//Brave
					pSkillData->second.iBaseId == 111606 ||		//Grace

					pSkillData->second.iBaseId == 112674 ||		//Insensibility Guard
					pSkillData->second.iBaseId == 111660 ||		//Insensibility Peel
					pSkillData->second.iBaseId == 111651 ||		//Insensibility Protector
					pSkillData->second.iBaseId == 111639 ||		//Insensibility barrier
					pSkillData->second.iBaseId == 111630 ||		//Insensibility shield
					pSkillData->second.iBaseId == 111621 ||		//Insensibility armor
					pSkillData->second.iBaseId == 111612 ||		//Insensibility shell
					pSkillData->second.iBaseId == 111603 ||		//Insensibility Skin

					pSkillData->second.iBaseId == 111645 ||		//Fresh mind
					pSkillData->second.iBaseId == 111636 ||		//Calm mind
					pSkillData->second.iBaseId == 111627 ||		//Bright mind
					pSkillData->second.iBaseId == 111609))		//Resist all
				{
					if (m_vecPartyMembers.size() > 0
						&& m_bPriestPartyBuff)
					{
						if (PartyMemberNeedBuff(pSkillData->second.iBaseId) == -1)
							return false;
					}
					else
					{
						auto pSkillExtension4Data = pSkillExtension4->find(pSkillData->second.iID);

						if (pSkillExtension4Data != pSkillExtension4->end())
						{
							if (IsBuffActive(pSkillExtension4Data->second.iBuffType))
								return false;
						}
					}
				}
				else
				{
					auto pSkillExtension4Data = pSkillExtension4->find(pSkillData->second.iID);

					if (pSkillExtension4Data != pSkillExtension4->end())
					{
						if (IsBuffActive(pSkillExtension4Data->second.iBuffType))
							return false;
					}
				}

				uint32_t iNeedItem = pSkillData->second.dwNeedItem;

				uint32_t iNeedItemCount = 1;
				uint32_t iExistItemCount = 0;

				if (iNeedItem != 0)
				{
					iExistItemCount = GetInventoryItemCount(pSkillData->second.dwNeedItem);

					auto pSkillExtension2Data = pSkillExtension2->find(pSkillData->second.iID);

					if (pSkillExtension2Data != pSkillExtension2->end())
						iNeedItemCount = pSkillExtension2Data->second.iArrowCount;
				}

				if (m_PlayerMySelf.iMSP >= pSkillData->second.iExhaustMSP && (iNeedItem == 0 || (iNeedItem != 0 && iExistItemCount >= iNeedItemCount)))
					return true;
				else
					return false;
			});

		if (vecFilteredCharacterSkillList.size() > 0)
		{
			int iCharacterSkillID = vecFilteredCharacterSkillList.front();

			auto pSkillData = pSkillTable->find(iCharacterSkillID);

			if (pSkillData == pSkillTable->end())
				return;

			if (pSkillData->second.iBaseId == 107010			//Swift
				&& m_vecPartyMembers.size() > 0
				&& m_bPartySwift)
			{
				int32_t iTargetID = PartyMemberNeedSwift();

				if (iTargetID != -1)
				{
					if (!m_bOnlyAttackSkillUseWithPacket
						&& m_bUseSkillWithPacket)
					{
						new std::thread([=]() { UseSkillWithPacket(pSkillData->second, iTargetID); });
					}
					else
					{
						UseSkill(pSkillData->second, iTargetID, 1);
					}
				}
			}
			else if ((
				pSkillData->second.iBaseId == 111545 ||			//Superior Healing
				pSkillData->second.iBaseId == 111536 ||			//Massive Healing
				pSkillData->second.iBaseId == 111527 ||			//Great Healing
				pSkillData->second.iBaseId == 111518 ||			//Major Healing
				pSkillData->second.iBaseId == 111509 ||			//Healing
				pSkillData->second.iBaseId == 107705)			//Minor Healing
				&& m_vecPartyMembers.size() > 0
				&& m_bPriestPartyHeal)
			{
				int32_t iTargetID = PartyMemberNeedHeal(pSkillData->second.iBaseId);

				if (iTargetID != -1)
				{
					if (!m_bOnlyAttackSkillUseWithPacket
						&& m_bUseSkillWithPacket)
					{
						new std::thread([=]() { UseSkillWithPacket(pSkillData->second, iTargetID); });
					}
					else
					{
						UseSkill(pSkillData->second, iTargetID, 1);
					}
				}
			}
			else if ((
					pSkillData->second.iBaseId == 112675 ||		//Superioris
					pSkillData->second.iBaseId == 112670 ||		//Imposingness
					pSkillData->second.iBaseId == 111657 ||		//massiveness
					pSkillData->second.iBaseId == 111655 ||		//Heapness
					pSkillData->second.iBaseId == 111654 ||		//Undying
					pSkillData->second.iBaseId == 111642 ||		//Mightness
					pSkillData->second.iBaseId == 111633 ||		//Hardness
					pSkillData->second.iBaseId == 111624 ||		//Strong
					pSkillData->second.iBaseId == 111615 ||		//Brave
					pSkillData->second.iBaseId == 111606 ||		//Grace

					pSkillData->second.iBaseId == 112674 ||		//Insensibility Guard
					pSkillData->second.iBaseId == 111660 ||		//Insensibility Peel
					pSkillData->second.iBaseId == 111651 ||		//Insensibility Protector
					pSkillData->second.iBaseId == 111639 ||		//Insensibility barrier
					pSkillData->second.iBaseId == 111630 ||		//Insensibility shield
					pSkillData->second.iBaseId == 111621 ||		//Insensibility armor
					pSkillData->second.iBaseId == 111612 ||		//Insensibility shell
					pSkillData->second.iBaseId == 111603 ||		//Insensibility Skin

					pSkillData->second.iBaseId == 111645 ||		//Fresh mind
					pSkillData->second.iBaseId == 111636 ||		//Calm mind
					pSkillData->second.iBaseId == 111627 ||		//Bright mind
					pSkillData->second.iBaseId == 111609)		//Resist all
					&& m_vecPartyMembers.size() > 0
					&& m_bPriestPartyBuff)
			{
				int32_t iTargetID = PartyMemberNeedBuff(pSkillData->second.iBaseId);

				if (iTargetID != -1)
				{
					if (!m_bOnlyAttackSkillUseWithPacket
						&& m_bUseSkillWithPacket)
					{
						new std::thread([=]() { UseSkillWithPacket(pSkillData->second, iTargetID); });
					}
					else
					{
						UseSkill(pSkillData->second, iTargetID, 1);
					}
				}
			}
			else 
			{
				if (pSkillData->second.iID == 490803 || pSkillData->second.iID == 490811)
				{
					SendStartSkillMagicAtTargetPacket(pSkillData->second, m_PlayerMySelf.iID, Vector3(0.0f, 0.0f, 0.0f));
				}
				else
				{
					if (!m_bOnlyAttackSkillUseWithPacket
						&& m_bUseSkillWithPacket)
					{
						new std::thread([=]() { UseSkillWithPacket(pSkillData->second, m_PlayerMySelf.iID); });
					}
					else
					{
						UseSkill(pSkillData->second, m_PlayerMySelf.iID, 1);
					}
				}
			}
		}

		m_fLastCharacterProcessTime = Bot::TimeGet();
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("CharacterProcess:Exception: %s\n", e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}
}

bool ClientHandler::HealthPotionProcess()
{
	std::vector<uint32_t> vecPotions =
	{
		389064000, 910005000, 389063000, 399014000,
		810265000, 810267000, 810269000, 810272000,
		890229000, 899996000, 910004000, 930665000,
		931786000, 389062000, 900790000, 910003000,
		930664000, 389061000, 900780000, 910002000,
		389060000, 900770000, 910001000, 910012000,
		389310000, 389320000, 389330000, 389390000,
		900817000, 389015000, 389014000, 389013000,
		389012000, 389011000, 389010000, 389070000,
		389071000, 800124000, 800126000, 810189000,
		810247000, 811006000, 811008000, 814679000,
		900486000, 399284000, 389210000, 810947000,
		810117000
	};

	auto pSort = [](uint32_t const& a, uint32_t const& b)
	{
		return a < b;
	};

	std::sort(vecPotions.begin(), vecPotions.end(), pSort);

	TItemData tInventoryItem;

	for (const auto& x : vecPotions)
	{
		tInventoryItem = GetInventoryItem(x);

		if (tInventoryItem.iItemID != 0)
		{
			return UseItem(tInventoryItem.iItemID);
		}
	}

	return false;
}

bool ClientHandler::ManaPotionProcess()
{
	std::vector<uint32_t> vecPotions =
	{
		389072000, 800125000, 800127000, 810192000,
		810248000, 900487000, 811006000, 811008000,
		814679000, 900486000, 389020000, 389019000,
		389018000, 389017000, 389016000, 389340000,
		389350000, 389360000, 389400000, 900818000,
		910006000, 389078000, 910007000, 900800000,
		389079000, 910008000, 900810000, 389080000,
		910009000, 900820000, 389081000, 910010000,
		899997000, 399020000, 389082000, 931787000,
		399285000, 810118000, 389130000, 389220000,
		910116000, 900141000, 814680000
	};

	auto pSort = [](uint32_t const& a, uint32_t const& b)
	{
		return a < b;
	};

	std::sort(vecPotions.begin(), vecPotions.end(), pSort);

	TItemData tInventoryItem;

	for (const auto& x : vecPotions)
	{
		tInventoryItem = GetInventoryItem(x);

		if (tInventoryItem.iItemID != 0)
		{
			return UseItem(tInventoryItem.iItemID);
		}
	}	

	return false;
}

void ClientHandler::RouteProcess()
{
	try
	{
		if (Bot::TimeGet() < (m_fLastRouteProcessTime + (100.0f / 1000.0f)))
			return;

		if (m_vecRouteActive.size() == 0)
			return;

		if (IsZoneChanging())
			return;

		if (IsBlinking())
			return;

		if (m_PlayerMySelf.eState == PSA_DEATH)
			return;

		if (GetActionState() == PSA_SPELLMAGIC)
			return;

		m_pCurrentRunningRoute = &m_vecRouteActive.front();

		m_fLastRouteProcessTime = Bot::TimeGet();

		switch (m_pCurrentRunningRoute->eStepType)
		{
			case RouteStepType::STEP_MOVE:
			{
				float fDistance = GetDistance(Vector3(GetX(), 0.0f, GetY()), Vector3(m_pCurrentRunningRoute->fX, 0.0f, m_pCurrentRunningRoute->fY));

				if (fDistance > 3.0f)
				{
					SetMovePosition(Vector3(m_pCurrentRunningRoute->fX, 0.0f, m_pCurrentRunningRoute->fY));
					return;
				}
			}
			break;

			case RouteStepType::STEP_TOWN:
			{
				float fDistance = GetDistance(Vector3(GetX(), 0.0f, GetY()), Vector3(m_pCurrentRunningRoute->fX, 0.0f, m_pCurrentRunningRoute->fY));

				if (fDistance > 50.0f)
				{
					SetMovePosition(Vector3(m_pCurrentRunningRoute->fX, 0.0f, m_pCurrentRunningRoute->fY));
					return;
				}

				int32_t iHpPercent = (int32_t)std::ceil((m_PlayerMySelf.iHP * 100) / m_PlayerMySelf.iHPMax);
				int32_t iHpNeededValue = 60;

				if (iHpPercent <= iHpNeededValue)
					return;

				SendTownPacket();
			}
			break;

			case RouteStepType::STEP_GENIE:
			{
				float fDistance = GetDistance(Vector3(GetX(), 0.0f, GetY()), Vector3(m_pCurrentRunningRoute->fX, 0.0f, m_pCurrentRunningRoute->fY));

				if (fDistance > 3.0f)
				{
					SetMovePosition(Vector3(m_pCurrentRunningRoute->fX, 0.0f, m_pCurrentRunningRoute->fY));
					return;
				}

				SendStartGenie();
			}
			break;

			case RouteStepType::STEP_BOT_START:
			{
				float fDistance = GetDistance(Vector3(GetX(), 0.0f, GetY()), Vector3(m_pCurrentRunningRoute->fX, 0.0f, m_pCurrentRunningRoute->fY));

				if (fDistance > 3.0f)
				{
					SetMovePosition(Vector3(m_pCurrentRunningRoute->fX, 0.0f, m_pCurrentRunningRoute->fY));
					return;
				}

				GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Attack"), 1);
				GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Character"), 1);
			}
			break;

			case RouteStepType::STEP_OBJECT_EVENT:
			{
				float fDistance = GetDistance(Vector3(GetX(), 0.0f, GetY()), Vector3(m_pCurrentRunningRoute->fX, 0.0f, m_pCurrentRunningRoute->fY));

				if (fDistance > 3.0f)
				{
					SetMovePosition(Vector3(m_pCurrentRunningRoute->fX, 0.0f, m_pCurrentRunningRoute->fY));
					return;
				}

				if (GetMoveState() != PSM_STOP)
					return;

				SendPacket(m_pCurrentRunningRoute->szPacket);
			}
			break;

			case RouteStepType::STEP_WARP:
			{
				if (!m_bRouteWarpListLoaded)
					return;

				SendPacket(m_pCurrentRunningRoute->szPacket);

				m_bRouteWarpListLoaded = false;
			}
			break;

			case RouteStepType::STEP_SUNDRIES:
			{
				float fDistance = GetDistance(Vector3(GetX(), 0.0f, GetY()), Vector3(m_pCurrentRunningRoute->fX, 0.0f, m_pCurrentRunningRoute->fY));

				if (fDistance > 3.0f)
				{
					SetMovePosition(Vector3(m_pCurrentRunningRoute->fX, 0.0f, m_pCurrentRunningRoute->fY));
					return;
				}

				int iNpcSellingGroup = 255000;

				if (!m_pCurrentRunningRoute->bReRouted)
				{
					std::vector<TNpc> tmpVecNpc = m_vecNpc;

					Vector3 v3CurrentPosition = GetPosition();

					auto pSort = [&](TNpc const& a, TNpc const& b)
						{
							auto fADistance = GetDistance(v3CurrentPosition, Vector3(a.fX, a.fZ, a.fY));
							auto fBDistance = GetDistance(v3CurrentPosition, Vector3(b.fX, b.fZ, b.fY));

							if (fADistance != fBDistance)
							{
								return fADistance < fBDistance;
							}

							return false;
						};

					m_pCurrentRunningRoute->pNpcID = -1;

					for (auto& e : tmpVecNpc)
					{
						if (e.iSellingGroup == iNpcSellingGroup)
						{
							m_pCurrentRunningRoute->pNpcID = e.iID;
							m_pCurrentRunningRoute->fX = e.fX;
							m_pCurrentRunningRoute->fY = e.fY;
							break;
						}
					}

					if (m_pCurrentRunningRoute->pNpcID != -1)
					{
						m_pCurrentRunningRoute->bReRouted = true;

						return;
					}
				}
				else
				{
					if (m_pCurrentRunningRoute->bSubProcessRequested == false)
					{
						if (GetMoveState() != PSM_STOP)
							return;

						SendNpcEvent(m_pCurrentRunningRoute->pNpcID);
						m_pCurrentRunningRoute->bSubProcessRequested = true;
						return;
					}
					else
					{
						if (m_pCurrentRunningRoute->bSubProcessFinished == false)
							return;
					}
				}
			}
			break;

			case RouteStepType::STEP_POTION:
			{
				float fDistance = GetDistance(Vector3(GetX(), 0.0f, GetY()), Vector3(m_pCurrentRunningRoute->fX, 0.0f, m_pCurrentRunningRoute->fY));

				if (fDistance > 3.0f)
				{
					SetMovePosition(Vector3(m_pCurrentRunningRoute->fX, 0.0f, m_pCurrentRunningRoute->fY));
					return;
				}

				int iNpcSellingGroup = 253000;

				if (!m_pCurrentRunningRoute->bReRouted)
				{
					std::vector<TNpc> tmpVecNpc = m_vecNpc;

					Vector3 v3CurrentPosition = GetPosition();

					auto pSort = [&](TNpc const& a, TNpc const& b)
						{
							auto fADistance = GetDistance(v3CurrentPosition, Vector3(a.fX, a.fZ, a.fY));
							auto fBDistance = GetDistance(v3CurrentPosition, Vector3(b.fX, b.fZ, b.fY));

							if (fADistance != fBDistance)
							{
								return fADistance < fBDistance;
							}

							return false;
						};

					m_pCurrentRunningRoute->pNpcID = -1;

					for (auto& e : tmpVecNpc)
					{
						if (e.iSellingGroup == iNpcSellingGroup)
						{
							m_pCurrentRunningRoute->pNpcID = e.iID;
							m_pCurrentRunningRoute->fX = e.fX;
							m_pCurrentRunningRoute->fY = e.fY;
							break;
						}
					}

					if (m_pCurrentRunningRoute->pNpcID != -1)
					{
						m_pCurrentRunningRoute->bReRouted = true;
						return;
					}
				}
				else
				{
					if (m_pCurrentRunningRoute->bSubProcessRequested == false)
					{
						if (GetMoveState() != PSM_STOP)
							return;

						SendNpcEvent(m_pCurrentRunningRoute->pNpcID);
						m_pCurrentRunningRoute->bSubProcessRequested = true;
						return;
					}
					else
					{
						if (m_pCurrentRunningRoute->bSubProcessFinished == false)
							return;
					}
				}
			}
			break;

			case RouteStepType::STEP_INN:
			{
				float fDistance = GetDistance(Vector3(GetX(), 0.0f, GetY()), Vector3(m_pCurrentRunningRoute->fX, 0.0f, m_pCurrentRunningRoute->fY));

				if (fDistance > 3.0f)
				{
					SetMovePosition(Vector3(m_pCurrentRunningRoute->fX, 0.0f, m_pCurrentRunningRoute->fY));
					return;
				}

				if (!m_pCurrentRunningRoute->bReRouted)
				{
					std::vector<TNpc> tmpVecNpc = m_vecNpc;

					Vector3 v3CurrentPosition = GetPosition();

					auto pSort = [&](TNpc const& a, TNpc const& b)
					{
						auto fADistance = GetDistance(v3CurrentPosition, Vector3(a.fX, a.fZ, a.fY));
						auto fBDistance = GetDistance(v3CurrentPosition, Vector3(b.fX, b.fZ, b.fY));

						if (fADistance != fBDistance)
						{
							return fADistance < fBDistance;
						}

						return false;
					};

					m_pCurrentRunningRoute->pNpcID = -1;

					for (auto& e : tmpVecNpc)
					{
						if (e.iFamilyType == 31)
						{
							m_pCurrentRunningRoute->pNpcID = e.iID;
							m_pCurrentRunningRoute->fX = e.fX;
							m_pCurrentRunningRoute->fY = e.fY;
							break;
						}
					}

					if (m_pCurrentRunningRoute->pNpcID != -1)
					{
						m_pCurrentRunningRoute->bReRouted = true;
						return;
					}
				}
				else
				{
					if (m_pCurrentRunningRoute->bSubProcessRequested == false)
					{
						GetInventoryItemList(m_pCurrentRunningRoute->vecInventoryItemList);
						SendWarehouseOpen(m_pCurrentRunningRoute->pNpcID);
						m_pCurrentRunningRoute->bSubProcessRequested = true;
						return;
					}
					else
					{
						if (m_pCurrentRunningRoute->bSubProcessFinished == false)
							return;
					}
				}
			}
			break;

			case RouteStepType::STEP_DC_SUNDRIES:
			{
				float fDistance = GetDistance(Vector3(GetX(), 0.0f, GetY()), Vector3(m_pCurrentRunningRoute->fX, 0.0f, m_pCurrentRunningRoute->fY));

				if (fDistance > 3.0f)
				{
					SetMovePosition(Vector3(m_pCurrentRunningRoute->fX, 0.0f, m_pCurrentRunningRoute->fY));
					return;
				}

				int iNpcSellingGroup = 267000;

				if (!m_pCurrentRunningRoute->bReRouted)
				{
					std::vector<TNpc> tmpVecNpc = m_vecNpc;

					Vector3 v3CurrentPosition = GetPosition();

					auto pSort = [&](TNpc const& a, TNpc const& b)
						{
							auto fADistance = GetDistance(v3CurrentPosition, Vector3(a.fX, a.fZ, a.fY));
							auto fBDistance = GetDistance(v3CurrentPosition, Vector3(b.fX, b.fZ, b.fY));

							if (fADistance != fBDistance)
							{
								return fADistance < fBDistance;
							}

							return false;
						};

					m_pCurrentRunningRoute->pNpcID = -1;

					for (auto& e : tmpVecNpc)
					{
						if (e.iSellingGroup == iNpcSellingGroup)
						{
							m_pCurrentRunningRoute->pNpcID = e.iID;
							m_pCurrentRunningRoute->fX = e.fX;
							m_pCurrentRunningRoute->fY = e.fY;
							break;
						}
					}

					if (m_pCurrentRunningRoute->pNpcID != -1)
					{
						m_pCurrentRunningRoute->bReRouted = true;

						return;
					}
				}
				else
				{
					if (m_pCurrentRunningRoute->bSubProcessRequested == false)
					{
						if (GetMoveState() != PSM_STOP)
							return;

						SendNpcEvent(m_pCurrentRunningRoute->pNpcID);
						m_pCurrentRunningRoute->bSubProcessRequested = true;
						return;
					}
					else
					{
						if (m_pCurrentRunningRoute->bSubProcessFinished == false)
							return;
					}
				}
			}
			break;
		}

		if (m_vecRouteActive.size() > 0)
			m_vecRouteActive.erase(m_vecRouteActive.begin());

		if (m_vecRouteActive.size() == 0)
		{
			m_pCurrentRunningRoute = nullptr;
			m_fLastSupplyTime = Bot::TimeGet();
		}
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("RouteProcess:Exception: %s\n", e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}
}

void ClientHandler::SupplyProcess()
{
	try
	{
		if (Bot::TimeGet() < (m_fLastSupplyProcessTime + (1000.0f / 1000.0f)))
			return;

		if (Bot::TimeGet() < (m_fLastSupplyTime + ((60000.0f * 5.0f) / 1000.0f)))
			return;

		if (IsZoneChanging())
			return;

		if (IsBlinking())
			return;

		if (IsRouting())
			return;

		if (m_PlayerMySelf.eState == PSA_DEATH)
			return;

		if (IsMovingToLoot())
			return;

		if (m_PlayerMySelf.iHP <= 0 
			|| m_PlayerMySelf.iHPMax <= 0)
			return;

		if (m_setSelectedSupplyRouteList.size() == 0)
			return;

		int32_t iHpPercent = (int32_t)std::ceil((m_PlayerMySelf.iHP * 100) / m_PlayerMySelf.iHPMax);
		int32_t iHpNeededValue = 60;

		if (iHpPercent <= iHpNeededValue)
			return;

		if (m_bAttackStatus || m_bCharacterStatus)
		{
			bool bNeedSupply = m_bAutoSupply && IsNeedSupply();
			bool bNeedRepair = !m_bAutoRepairMagicHammer && m_bAutoRepair && IsNeedRepair();
			bool bNeedSell = m_bAutoSupply && IsNeedSell();

			if (bNeedSupply 
				|| bNeedRepair 
				|| (!m_bVIPSellSupply && IsInventoryFull() && bNeedSell) 
				|| (m_bVIPSellSupply && m_bVipWarehouseFull && IsInventoryFull() && bNeedSell))
			{
				Bot::RouteList pRouteList;

				uint8_t iZoneID = GetRepresentZone(GetZone());

				if (m_Bot->GetRouteList(iZoneID, pRouteList))
				{
					std::srand((unsigned int)std::time(0));

					auto it = std::begin(m_setSelectedSupplyRouteList);
					std::advance(it, std::rand() % m_setSelectedSupplyRouteList.size());

					auto pRoute = pRouteList.find(*it);

					if (pRoute != pRouteList.end())
					{
						SendStopGenie();
						SetRoute(pRoute->second);
					}
				}
			}
		}

		m_fLastSupplyProcessTime = Bot::TimeGet();
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("SupplyProcess:Exception: %s\n", e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}
}

void ClientHandler::StatisticsProcess()
{
	try
	{
		if (Bot::TimeGet() < (m_fLastStatisticsProcessTime + (60000.0f / 1000.0f)))
			return;

		m_iEveryMinuteCoinPrevCounter = m_iEveryMinuteCoinCounter;
		m_iEveryMinuteCoinCounter = m_iCoinCounter;

		m_iEveryMinuteExpPrevCounter = m_iEveryMinuteExpCounter;
		m_iEveryMinuteExpCounter = m_iExpCounter;

		m_fLastStatisticsProcessTime = Bot::TimeGet();
		
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("StatisticsProcess:Exception: %s\n", e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}
}

void ClientHandler::RemoveItemProcess()
{
	try
	{
		if (Bot::TimeGet() < (m_fLastRemoveItemProcessTime + (45000.0f / 1000.0f)))
			return;

		if (IsZoneChanging())
			return;

		if (IsBlinking())
			return;

		if (IsRouting())
			return;

		if (m_PlayerMySelf.eState == PSA_DEATH)
			return;

		if (m_vecDeleteItemList.size() > 0)
		{
			std::vector<TItemData> vecInventoryItemList;
			GetInventoryItemList(vecInventoryItemList);

			for (const TItemData& pItem : vecInventoryItemList)
			{
				if (pItem.iItemID == 0)
					continue;

				__TABLE_ITEM* pItemData;
				if (!m_Bot->GetItemData(pItem.iItemID, pItemData))
					continue;

				__TABLE_ITEM_EXTENSION* pItemExtensionData;
				if (!m_Bot->GetItemExtensionData(pItem.iItemID, pItemData->byExtIndex, pItemExtensionData))
					continue;

				if (pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UNIQUE
					|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_RED
					|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UPGRADE_REVERSE
					|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UNIQUE_REVERSE
					|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_PET
					|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_EVENT
					|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_COSPRE
					|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_MINERVA)
					continue;

				uint32_t iItemBaseID = pItem.iItemID / 1000 * 1000;
				bool bDeleteSelected = m_vecDeleteItemList.count(iItemBaseID) > 0;

				if (bDeleteSelected)
				{
					RemoveItem(pItem.iPos - 14);
				}
			}
		}

		m_fLastRemoveItemProcessTime = Bot::TimeGet();

	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("RemoveItemProcess:Exception: %s\n", e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}
}


void ClientHandler::VIPStorageSupplyProcess()
{
	try
	{
		if (Bot::TimeGet() < (m_fLastVIPStorageSupplyProcessTime + (15000.0f / 1000.0f)))
			return;

		if (IsZoneChanging())
			return;

		if (IsBlinking())
			return;

		if (IsRouting())
			return;

		if (m_PlayerMySelf.eState == PSA_DEATH)
			return;

		if (m_bVipWarehouseFull)
			return;

		if (!m_bVIPSellSupply)
			return;

		if (m_vecSellItemList.size() == 0)
			return;

		if (!IsInventoryFull())
			return;

		new std::thread([=]()
		{
			std::vector<TItemData> vecInventoryItemList;
			std::vector<TItemData> vecFlaggedItemList;

			GetInventoryItemList(vecInventoryItemList);

			for (const TItemData& pItem : vecInventoryItemList)
			{
				if (pItem.iItemID == 0)
					continue;

				//Promise Of Training
				if (pItem.iItemID == 989511000)
					continue;

				uint32_t iItemBaseID = pItem.iItemID / 1000 * 1000;
				bool bSellSelected = m_vecSellItemList.count(iItemBaseID) > 0;

				if (!bSellSelected)
					continue;

				__TABLE_ITEM* pItemData;
				if (!m_Bot->GetItemData(pItem.iItemID, pItemData))
					continue;

				__TABLE_ITEM_EXTENSION* pItemExtensionData;
				if (!m_Bot->GetItemExtensionData(pItem.iItemID, pItemData->byExtIndex, pItemExtensionData))
					continue;

				if (pItemData->byNeedRace == RACE_TRADEABLE_IN_72HR
					|| pItemData->byNeedRace == RACE_NO_TRADE
					|| pItemData->byNeedRace == RACE_NO_TRADE_SOLD
					|| pItemData->byNeedRace == RACE_NO_TRADE_SOLD2
					|| pItemData->byNeedRace == RACE_NO_TRADE_SOLD_STORE)
					continue;

				if (pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UNIQUE
					|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_RED
					|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UPGRADE_REVERSE
					|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UNIQUE_REVERSE
					|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_PET
					|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_EVENT
					|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_COSPRE
					|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_MINERVA)
					continue;

				vecFlaggedItemList.push_back(pItem);
			}

			if (vecFlaggedItemList.size() > 0)
			{
				OpenVipWarehouse();

				std::this_thread::sleep_for(std::chrono::milliseconds(1500));

				WaitConditionWithTimeout(m_bVipWarehouseLoaded == false, 3000);

				if (!m_bVipWarehouseLoaded)
					return;

				std::this_thread::sleep_for(std::chrono::milliseconds(100));

				for (const TItemData& pItem : vecFlaggedItemList)
				{
					__TABLE_ITEM* pItemData;
					if (!m_Bot->GetItemData(pItem.iItemID, pItemData))
						continue;

					WaitConditionWithTimeout(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_UI_LOCK"))) == 1, 3000);

					std::vector<TItemData> vecVipWarehouseItemList;
					GetVipWarehouseItemList(vecVipWarehouseItemList);

					int iTargetPosition = -1;

					if (pItemData->byContable)
					{
						auto pWarehouseItem = std::find_if(vecVipWarehouseItemList.begin(), vecVipWarehouseItemList.end(),
							[&](const TItemData& a)
							{
								return a.iItemID == pItem.iItemID;
							});

						if (pWarehouseItem != vecVipWarehouseItemList.end())
						{
							if ((pItem.iCount + pWarehouseItem->iCount) > 9999)
								continue;

							iTargetPosition = pWarehouseItem->iPos;
						}
						else
						{
							auto pWarehouseEmptySlot = std::find_if(vecVipWarehouseItemList.begin(), vecVipWarehouseItemList.end(),
								[&](const TItemData& a)
								{
									return a.iItemID == 0;
								});

							if (pWarehouseEmptySlot != vecVipWarehouseItemList.end())
							{
								iTargetPosition = pWarehouseEmptySlot->iPos;
							}
						}
					}
					else
					{
						auto pWarehouseEmptySlot = std::find_if(vecVipWarehouseItemList.begin(), vecVipWarehouseItemList.end(),
							[&](const TItemData& a)
							{
								return a.iItemID == 0;
							});

						if (pWarehouseEmptySlot != vecVipWarehouseItemList.end())
						{
							iTargetPosition = pWarehouseEmptySlot->iPos;
						}
					}

					if (iTargetPosition == -1)
					{
						m_bVipWarehouseFull = true;
						std::this_thread::sleep_for(std::chrono::milliseconds(1500));
						CloseVipWarehouse();
						std::this_thread::sleep_for(std::chrono::milliseconds(1500));
						return;
					}

					WaitConditionWithTimeout(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_UI_LOCK"))) == 1, 3000);

					VipWarehouseGetIn(pItem.iBase, pItem.iPos, iTargetPosition);

					WaitConditionWithTimeout(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_UI_LOCK"))) == 1, 3000);

					if (pItemData->byContable)
					{
						CountableDialogChangeCount(pItem.iCount);
						AcceptCountableDialog();
					}
				}

				WaitConditionWithTimeout(m_Bot->Read4Byte(m_Bot->GetAddress(skCryptDec("KO_PTR_UI_LOCK"))) == 1, 3000);

				std::this_thread::sleep_for(std::chrono::milliseconds(1500));

				CloseVipWarehouse();
			}
		});

		m_fLastVIPStorageSupplyProcessTime = Bot::TimeGet();

	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("VIPStorageSupplyProcess:Exception: %s\n", e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}
}

void ClientHandler::LevelDownerProcess()
{
	try
	{
		if (Bot::TimeGet() < (m_fLastLevelDownerProcessTime + (500.0f / 1000.0f)))
			return;

		if (IsZoneChanging())
			return;

		if (IsBlinking())
			return;

		if (IsRouting())
			return;

		if (!m_bLevelDownerEnable)
			return;

		if (m_iLevelDownerNpcId == -1)
			return;

		if (m_bLevelDownerStopNearbyPlayer
			&& GetRegionUserCount(true) > 0)
			return;

		if (m_bLevelDownerLevelLimitEnable
			&& GetLevel() < m_iLevelDownerLevelLimit)
			return;

		if (m_PlayerMySelf.eState == PSA_DEATH)
			SendRegenePacket();
		else
		{
			Packet pkt = Packet(WIZ_MAGIC_PROCESS);

			pkt
				<< uint8_t(SkillMagicType::SKILL_MAGIC_TYPE_EFFECTING)
				<< uint32_t(300215)
				<< m_iLevelDownerNpcId
				<< m_PlayerMySelf.iID;

			SendPacket(pkt);
		}

		m_fLastLevelDownerProcessTime = Bot::TimeGet();

	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("StatisticsProcess:Exception: %s\n", e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}
}

void ClientHandler::SetRoute(std::vector<Route> vecRoute)
{
	m_vecRouteActive = vecRoute;
}

void ClientHandler::ClearRoute()
{
	m_pCurrentRunningRoute = nullptr;
	m_vecRouteActive.clear();
}

int ClientHandler::GetRegionUserCount(bool bExceptPartyMember, float fRangeLimit)
{
	int iUserCount = 0;

	for (size_t i = 0; i < m_vecPlayer.size(); i++)
	{
		const auto& pPlayer = m_vecPlayer[i];

		if (bExceptPartyMember)
		{
			auto pPartyMember = std::find_if(m_vecPartyMembers.begin(), m_vecPartyMembers.end(),
				[&](const PartyMember& a)
				{
					return a.iMemberID == pPlayer.iID;
				});

			if (pPartyMember != m_vecPartyMembers.end())
				continue;
		}

		if (fRangeLimit != 0.0f)
		{
			auto iPlayerPosition = Vector3(pPlayer.fX, pPlayer.fZ, pPlayer.fY);

			if (GetDistance(iPlayerPosition) > fRangeLimit)
				continue;
		}

		iUserCount++;
	}

	return iUserCount;
}

int32_t ClientHandler::PartyMemberNeedSwift()
{
	std::map<uint32_t, __TABLE_UPC_SKILL_EXTENSION4>* pSkillExtension4;
	if (!m_Bot->GetSkillExtension4Table(&pSkillExtension4))
		return -1;

	auto pSkillData = std::find_if(m_vecAvailableSkill.begin(), m_vecAvailableSkill.end(),
		[&](const TABLE_UPC_SKILL& a) { return a.iBaseId == 107010; });

	if (pSkillData == m_vecAvailableSkill.end())
		return -1;

	std::vector<PartyMember> tmpVecPartyMembers;
	std::copy(m_vecPartyMembers.begin(), m_vecPartyMembers.end(), std::back_inserter(tmpVecPartyMembers));

	auto pSort = [](PartyMember const& a, PartyMember const& b)
		{
			return a.iIndex < b.iIndex;
		};

	std::sort(tmpVecPartyMembers.begin(), tmpVecPartyMembers.end(), pSort);

	for (auto& pMember : tmpVecPartyMembers)
	{
		if (m_PlayerMySelf.iMSP < pSkillData->iExhaustMSP)
			continue;

		if (m_PlayerMySelf.iID == pMember.iMemberID)
		{
			auto pSkillExtension4Data = pSkillExtension4->find(pSkillData->iID);

			if (pSkillExtension4Data != pSkillExtension4->end())
			{
				if (IsBuffActive(pSkillExtension4Data->second.iBuffType))
					continue;
			}
		}

		auto pTargetPlayer = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
			[&](const TPlayer& a) { return a.iID == pMember.iMemberID; });

		if (pTargetPlayer == m_vecPlayer.end())
			continue;

		auto iPlayerPosition = Vector3(pTargetPlayer->fX, pTargetPlayer->fZ, pTargetPlayer->fY);

		if (GetDistance(iPlayerPosition) > 25.0f)
			continue;

		if (IsStunned() || m_PlayerMySelf.eState == PSA_DEATH || pTargetPlayer->eState == PSA_DEATH)
			continue;

		if (IsBlinking() || IsBlinking(pMember.iMemberID))
			continue;

		if (m_PlayerMySelf.iID == pMember.iMemberID
			|| (pTargetPlayer->iMoveSpeed < 45 || pTargetPlayer->iMoveSpeed > 45))
			continue;

		if (m_bSkillCasting == true || GetActionState() == PSA_SPELLMAGIC)
			continue;

		return pMember.iMemberID;
	}

	return -1;
}

int32_t ClientHandler::PartyMemberNeedHeal(uint32_t iSkillBaseID)
{
	std::map<uint32_t, __TABLE_UPC_SKILL_EXTENSION3>* pSkillExtension3;
	if (!m_Bot->GetSkillExtension3Table(&pSkillExtension3))
		return -1;

	auto pSkillData = std::find_if(m_vecAvailableSkill.begin(), m_vecAvailableSkill.end(),
		[=](const TABLE_UPC_SKILL& a) { return a.iBaseId == iSkillBaseID; });

	if (pSkillData == m_vecAvailableSkill.end())
		return -1;

	std::vector<PartyMember> tmpVecPartyMembers;
	std::copy(m_vecPartyMembers.begin(), m_vecPartyMembers.end(), std::back_inserter(tmpVecPartyMembers));

	auto pSort = [](PartyMember const& a, PartyMember const& b)
		{
			return a.iHP < b.iHP;
		};

	std::sort(tmpVecPartyMembers.begin(), tmpVecPartyMembers.end(), pSort);

	for (auto& pMember : tmpVecPartyMembers)
	{
		if (m_PlayerMySelf.iMSP < pSkillData->iExhaustMSP)
			continue;

		if (m_bHealProtection)
		{
			if (pMember.iHP <= 0
				&& pMember.iMaxHP <= 0)
			{
				continue;
			}

			int32_t iHpProtectionPercent = (int32_t)std::ceil((pMember.iHP * 100) / pMember.iMaxHP);

			if (m_iHealProtectionValue < iHpProtectionPercent)
				continue;
		}
		else
		{
			auto pSkillExtension3Data = pSkillExtension3->find(pSkillData->iID);

			if (pSkillExtension3Data != pSkillExtension3->end())
			{
				int32_t iNeedHealValue = pMember.iMaxHP - pMember.iHP;

				if (iNeedHealValue <= 0
					|| iNeedHealValue < pSkillExtension3Data->second.iHealValue)
					continue;
			}
		}

		if (m_PlayerMySelf.iID != pMember.iMemberID)
		{
			auto pTargetPlayer = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
				[&](const TPlayer& a) { return a.iID == pMember.iMemberID; });

			if (pTargetPlayer == m_vecPlayer.end())
				continue;

			auto iPlayerPosition = Vector3(pTargetPlayer->fX, pTargetPlayer->fZ, pTargetPlayer->fY);

			if (GetDistance(iPlayerPosition) > 25.0f)
				continue;

			if (IsStunned() || m_PlayerMySelf.eState == PSA_DEATH || pTargetPlayer->eState == PSA_DEATH)
				continue;
		}
		else
		{
			if (IsStunned() || m_PlayerMySelf.eState == PSA_DEATH)
				continue;
		}

		if (IsBlinking() || IsBlinking(pMember.iMemberID))
			continue;

		if (m_bSkillCasting == true || GetActionState() == PSA_SPELLMAGIC)
			continue;

		return pMember.iMemberID;
	}

	return -1;
}

int32_t ClientHandler::PartyMemberNeedBuff(uint32_t iSkillBaseID)
{
	std::map<uint32_t, __TABLE_UPC_SKILL_EXTENSION4>* pSkillExtension4;
	if (!m_Bot->GetSkillExtension4Table(&pSkillExtension4))
		return -1;

	auto pSkillData = std::find_if(m_vecAvailableSkill.begin(), m_vecAvailableSkill.end(),
		[=](const TABLE_UPC_SKILL& a) { return a.iBaseId == iSkillBaseID; });

	if (pSkillData == m_vecAvailableSkill.end())
		return -1;

	std::vector<PartyMember> tmpVecPartyMembers;
	std::copy(m_vecPartyMembers.begin(), m_vecPartyMembers.end(), std::back_inserter(tmpVecPartyMembers));

	auto pSort = [](PartyMember const& a, PartyMember const& b)
		{
			return a.iIndex < b.iIndex;
		};

	std::sort(tmpVecPartyMembers.begin(), tmpVecPartyMembers.end(), pSort);

	for (auto& pMember : tmpVecPartyMembers)
	{
		if (m_PlayerMySelf.iMSP < pSkillData->iExhaustMSP)
			continue;

		auto pSkillExtension4Data = pSkillExtension4->find(pSkillData->iID);

		if (m_PlayerMySelf.iID == pMember.iMemberID)
		{
			if (pSkillExtension4Data != pSkillExtension4->end())
			{
				if (IsBuffActive(pSkillExtension4Data->second.iBuffType))
					continue;
			}
		}
		else
		{
			if (pSkillExtension4Data != pSkillExtension4->end())
			{
				float fBuffDuration = pSkillExtension4Data->second.iBuffDuration * 1000.0f;

				if (pSkillExtension4Data->second.iBuffType == BUFF_TYPE_HP_MP)
				{
					if (pMember.fHpBuffTime > 0
						&& Bot::TimeGet() < pMember.fHpBuffTime + (fBuffDuration / 1000.0f))
					{
						continue;
					}
				}

				if (pSkillExtension4Data->second.iBuffType == BUFF_TYPE_AC)
				{
					if (pMember.fACBuffTime > 0
						&& Bot::TimeGet() < pMember.fACBuffTime + (fBuffDuration / 1000.0f))
					{
						continue;
					}
				}

				if (pSkillExtension4Data->second.iBuffType == BUFF_TYPE_RESISTANCES)
				{
					if (pMember.fMindBuffTime > 0
						&& Bot::TimeGet() < pMember.fMindBuffTime + (fBuffDuration / 1000.0f))
					{
						continue;
					}
				}
			}
		}

		if (m_PlayerMySelf.iID != pMember.iMemberID)
		{
			auto pTargetPlayer = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
				[&](const TPlayer& a) { return a.iID == pMember.iMemberID; });

			if (pTargetPlayer == m_vecPlayer.end())
				continue;

			auto iPlayerPosition = Vector3(pTargetPlayer->fX, pTargetPlayer->fZ, pTargetPlayer->fY);

			if (GetDistance(iPlayerPosition) > 25.0f)
				continue;

			if (IsStunned() || m_PlayerMySelf.eState == PSA_DEATH || pTargetPlayer->eState == PSA_DEATH)
				continue;
		}
		else
		{
			if (IsStunned() || m_PlayerMySelf.eState == PSA_DEATH)
				continue;
		}

		if (IsBlinking() || IsBlinking(pMember.iMemberID))
			continue;

		if (m_bSkillCasting == true || GetActionState() == PSA_SPELLMAGIC)
			continue;

		return pMember.iMemberID;
	}

	return -1;
}


bool ClientHandler::IsSkillHasZoneLimit(uint32_t iSkillBaseID)
{
	int iZoneIndex = m_PlayerMySelf.iCity;

	switch (iSkillBaseID)
	{
		case 490803:
		case 490811:
		case 490808:
		case 490809:
		case 490810:
		case 490800:
		case 490801:
		case 490817:
		{
			if (iZoneIndex == ZONE_DRAKI_TOWER ||
				iZoneIndex == ZONE_MONSTER_STONE1 ||
				iZoneIndex == ZONE_MONSTER_STONE2 ||
				iZoneIndex == ZONE_MONSTER_STONE3)
				return true;
		}
		break;
	}

	return false;
}

bool ClientHandler::UseSkill(TABLE_UPC_SKILL pSkillData, int32_t iTargetID, int32_t iPriority, bool bWaitCastTime)
{
	Vector3 v3MyPosition = GetPosition();

	if (pSkillData.iReCastTime != 0)
	{
		bool bSpeedHack = GetUserConfiguration()->GetBool(skCryptDec("Feature"), skCryptDec("SpeedHack"), false);

		if (bSpeedHack
			&& m_PlayerMySelf.iMoveType != 0)
		{
			SendMovePacket(v3MyPosition, v3MyPosition, 0, 0);
		}
	}

	Packet pkt = Packet(PIPE_USE_SKILL);

	pkt << iTargetID << pSkillData.iID << iPriority << uint8_t(0) << uint8_t(0);

	m_Bot->SendInternalMailslot(pkt);

	return true;
}

bool ClientHandler::UseItem(uint32_t iItemID)
{
	std::map<uint32_t, __TABLE_ITEM>* pItemTable;
	if (!m_Bot->GetItemTable(&pItemTable))
		return false;

	std::map<uint32_t, __TABLE_UPC_SKILL>* pSkillTable;
	if (!m_Bot->GetSkillTable(&pSkillTable))
		return false;

	auto pItemData = pItemTable->find(iItemID);

	if (pItemData != pItemTable->end())
	{
		auto pSkillData = pSkillTable->find(pItemData->second.dwEffectID1);

		if (pSkillData != pSkillTable->end())
		{
			UseSkill(pSkillData->second, m_PlayerMySelf.iID, 2);
			return true;
		}
	}

	return false;
}

bool ClientHandler::IsNeedSupply()
{
	std::vector<uint32_t> vecSellingGroupList;

	vecSellingGroupList.push_back(253000);
	vecSellingGroupList.push_back(255000);
	vecSellingGroupList.push_back(267000);

	uint8_t iBuyPageCount = 0;
	std::vector<SSItemBuy> vecInventoryItemBuy[2];
	std::vector<int32_t> vecExceptedBuyPos;

	for (auto& pSellingGroup : vecSellingGroupList)
	{
		std::string szSupplyString = "Supply_" + std::to_string(pSellingGroup);
		std::unordered_set<int> vecsupplyList = GetUserConfiguration()->GetInt(szSupplyString.c_str(), skCryptDec("Enable"), std::unordered_set<int>());

		for (auto& pSupplyItemID : vecsupplyList)
		{
			TItemData pInventoryItem = GetInventoryItem(pSupplyItemID);

			if (pInventoryItem.iItemID == 0)
				return true;

			if (pInventoryItem.iItemID != 0)
			{
				int iItemCount = GetUserConfiguration()->GetInt(szSupplyString.c_str(), std::to_string(pSupplyItemID).c_str(), 0);

				if (pInventoryItem.iCount <= 5 
					&& pInventoryItem.iCount < iItemCount)
					return true;
			}
		}
	}

	return false;
}

bool ClientHandler::IsNeedSell()
{
	if (m_vecSellItemList.size() == 0)
		return false;

	bool bNeedSell = false;

	for (int i = SLOT_MAX; i < SLOT_MAX + HAVE_MAX; i++)
	{
		TItemData pInventory = GetInventoryItemSlot((uint8_t)i);

		if (pInventory.iItemID == 0)
			return false;

		uint32_t iItemBaseID = pInventory.iItemID / 1000 * 1000;

		if (m_vecSellItemList.count(pInventory.iItemID) > 0)
			bNeedSell = true;
	}

	return bNeedSell;
}

bool ClientHandler::IsNeedRepair()
{
	for (uint8_t i = 0; i < SLOT_MAX; i++)
	{
		switch (i)
		{
		case 1:
		case 4:
		case 6:
		case 8:
		case 10:
		case 12:
		case 13:
		{
			TItemData pInventory = GetInventoryItemSlot(i);

			if (pInventory.iItemID == 0)
				continue;

			if (pInventory.iDurability != 0)
				continue;

			return true;
		}
		break;
		}
	}

	return false;
}
