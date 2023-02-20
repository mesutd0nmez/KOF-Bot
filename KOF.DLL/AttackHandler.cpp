#include "pch.h"
#include "Client.h"
#include "AttackHandler.h"
#include "Bootstrap.h"

void AttackHandler::Start()
{
#ifdef _DEBUG
	printf("AttackHandler::Started\n");
#endif

	m_bWorking = true;

	new std::thread([]() { BasicAttackProcess(); });
	new std::thread([]() { AttackProcess(); });
	new std::thread([]() { SearchTargetProcess(); });
}

void AttackHandler::Stop()
{
	m_bWorking = false;

#ifdef _DEBUG
	printf("AttackHandler::Stopped\n");
#endif
}

void AttackHandler::BasicAttackProcess()
{
#ifdef _DEBUG
	printf("AttackHandler::BasicAttackProcess Started\n");
#endif

	while (m_bWorking)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if (!Client::IsWorking())
			continue;

		auto pUserConfig = Client::GetUserConfig(Client::GetName());

		if (pUserConfig == NULL)
			continue;

		if (Client::GetTarget() == -1)
			continue;

		if (Client::IsMovingToLoot())
			continue;

		if (Client::IsBlinking())
			continue;

		bool bAttackStatus = pUserConfig->GetBool("Automation", "Attack", false);

		if (!bAttackStatus)
			continue;

		bool bBasicAttack = pUserConfig->GetBool("Attack", "BasicAttack", true);

		if (!bBasicAttack)
			continue;

		bool bMoveToTarget = pUserConfig->GetBool("Attack", "MoveToTarget", false);

		if (bMoveToTarget && Client::GetDistance(Client::GetTargetPosition()) > 1.0f)
			Client::SetMovePosition(Client::GetTargetPosition());

		bool bRangeLimit = pUserConfig->GetBool("Attack", "RangeLimit", false);
		int iRangeLimitValue = pUserConfig->GetInt("Attack", "RangeLimitValue", (int)MAX_ATTACK_RANGE);

		if (bRangeLimit && Client::GetDistance(Client::GetTargetPosition()) > (float)iRangeLimitValue)
			continue;

		auto pItemTable = Bootstrap::GetItemTable().GetData();

		auto iLeftHandWeapon = Client::GetInventoryItemSlot(6);

		uint32_t iLeftHandWeaponBaseID = iLeftHandWeapon->iItemID / 1000 * 1000;
		auto pLeftHandWeaponItemData = pItemTable.find(iLeftHandWeaponBaseID);

		if (pLeftHandWeaponItemData != pItemTable.end())
		{
			if (pLeftHandWeaponItemData->second.byKind == ITEM_CLASS_BOW
				|| pLeftHandWeaponItemData->second.byKind == ITEM_CLASS_BOW_LONG
				|| pLeftHandWeaponItemData->second.byKind == ITEM_CLASS_BOW_CROSS)
				continue;
		}

		auto iRightHandWeapon = Client::GetInventoryItemSlot(8);
		uint32_t iRightHandWeaponBaseID = iRightHandWeapon->iItemID / 1000 * 1000;
		auto pRightHandWeaponItemData = pItemTable.find(iRightHandWeaponBaseID);

		if (pRightHandWeaponItemData != pItemTable.end())
		{
			if (pRightHandWeaponItemData->second.byKind == ITEM_CLASS_BOW
				|| pRightHandWeaponItemData->second.byKind == ITEM_CLASS_BOW_LONG
				|| pRightHandWeaponItemData->second.byKind == ITEM_CLASS_BOW_CROSS)
				continue;
		}

		Client::SendBasicAttackPacket(Client::GetTarget(), 1.10f, 1.0f);
		std::this_thread::sleep_for(std::chrono::milliseconds(1110));
	}

#ifdef _DEBUG
	printf("AttackHandler::BasicAttackProcess Stopped\n");
#endif
}

void AttackHandler::AttackProcess()
{
#ifdef _DEBUG
	printf("AttackHandler::AttackProcess Started\n");
#endif

	while (m_bWorking)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if (!Client::IsWorking())
			continue;

		auto pUserConfig = Client::GetUserConfig(Client::GetName());

		if (pUserConfig == NULL)
			continue;

		if (Client::GetTarget() == -1)
			continue;

		if (Client::IsMovingToLoot())
			continue;

		if (Client::IsBlinking())
			continue;

		bool bAttackStatus = pUserConfig->GetBool("Automation", "Attack", false);

		if (!bAttackStatus)
			continue;

		bool bMoveToTarget = pUserConfig->GetInt("Attack", "MoveToTarget", false);

		if (bMoveToTarget && Client::GetDistance(Client::GetTargetPosition()) > 1.0f)
			Client::SetMovePosition(Client::GetTargetPosition());

		bool bRangeLimit = pUserConfig->GetBool("Attack", "RangeLimit", false);
		int iRangeLimitValue = pUserConfig->GetInt("Attack", "RangeLimitValue", (int)MAX_ATTACK_RANGE);

		if (bRangeLimit && Client::GetDistance(Client::GetTargetPosition()) > (float)iRangeLimitValue)
			continue;

		std::vector<int> vecAttackSkillList = pUserConfig->GetInt("Automation", "AttackSkillList", std::vector<int>());

		auto pSort = [](int& a, int& b)
		{
			return a > b;
		};

		std::sort(vecAttackSkillList.begin(), vecAttackSkillList.end(), pSort);

		for (const auto& x : vecAttackSkillList)
		{
			auto pSkillTable = Bootstrap::GetSkillTable().GetData();
			auto pSkillData = pSkillTable.find(x);

			if (pSkillData != pSkillTable.end())
			{
				uint32_t iNeedItem = pSkillData->second.dwNeedItem;

				uint32_t iNeedItemCount = 1;
				uint32_t iExistItemCount = 0;

				if (iNeedItem != 0)
				{
					iExistItemCount = Client::GetInventoryItemCount(pSkillData->second.dwNeedItem);

					auto pSkillExtension2 = Bootstrap::GetSkillExtension2Table().GetData();
					auto pSkillExtension2Data = pSkillExtension2.find(pSkillData->second.iID);

					if (pSkillExtension2Data != pSkillExtension2.end())
						iNeedItemCount = pSkillExtension2Data->second.iArrowCount;
				}

				std::chrono::milliseconds msNow = duration_cast<std::chrono::milliseconds>(
					std::chrono::system_clock::now().time_since_epoch()
				);

				std::chrono::milliseconds msLastSkillUseItem = Client::GetSkillUseTime(pSkillData->second.iID);

				if (pSkillData->second.iCooldown > 0 && msLastSkillUseItem.count() > 0)
				{
					int64_t iSkillCooldownTime = static_cast<int64_t>(pSkillData->second.iCooldown) * 100;

					if ((msLastSkillUseItem.count() + iSkillCooldownTime) > msNow.count())
					{
						continue;
					}
				}

				if (Client::GetMp() < pSkillData->second.iExhaustMSP)
					continue;

				if (iNeedItem != 0 && iExistItemCount < iNeedItemCount)
					continue;

				Client::UseSkill(pSkillData->second, Client::GetTarget());
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(750));
		}

		bool bAttackSpeed = pUserConfig->GetBool("Attack", "AttackSpeed", false);

		if (bAttackSpeed)
		{
			int iAttackSpeedValue = pUserConfig->GetInt("Attack", "AttackSpeedValue", 1000);

			if(iAttackSpeedValue > 0)
				std::this_thread::sleep_for(std::chrono::milliseconds(iAttackSpeedValue));
		}
		else
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

#ifdef _DEBUG
	printf("AttackHandler::AttackProcess Stopped\n");
#endif
}

void AttackHandler::SearchTargetProcess()
{
#ifdef _DEBUG
	printf("AttackHandler::SearchTargetProcess Started\n");
#endif

	while (m_bWorking)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if (!Client::IsWorking())
			continue;

		auto pUserConfig = Client::GetUserConfig(Client::GetName());

		if (pUserConfig == NULL)
			continue;

		if (Client::IsBlinking())
			continue;

		if (Client::IsMovingToLoot())
			continue;

		bool bAttackStatus = pUserConfig->GetBool("Automation", "Attack", false);

		if (!bAttackStatus)
			continue;

		auto pNpcList = Client::GetNpcList();

		if (pNpcList.size() == 0)
			continue;

		bool bAutoTarget = pUserConfig->GetInt("Attack", "AutoTarget", true);

		std::vector<int> vecSelectedNpcList = pUserConfig->GetInt("Attack", "NpcList", std::vector<int>());

		if (Client::GetTarget() == -1)
		{
			std::vector<TNpc> vecFilteredNpc;

			if (bAutoTarget)
			{
				std::copy_if(pNpcList.begin(), pNpcList.end(),
					std::back_inserter(vecFilteredNpc),
					[](const TNpc& c) 
					{ 
						return (c.iMonsterOrNpc == 1
							|| (c.iProtoID >= 19067 && c.iProtoID <= 19069)
							|| (c.iProtoID >= 19070 && c.iProtoID <= 19072))
							&& c.iProtoID != 9009
							&& c.eState != PSA_DYING 
							&& c.eState != PSA_DEATH
							&& Client::GetDistance(c.fX, c.fY) <= (float)MAX_ATTACK_RANGE;
					});
			}
			else
			{
				std::copy_if(pNpcList.begin(), pNpcList.end(),
					std::back_inserter(vecFilteredNpc),
					[vecSelectedNpcList](const TNpc& c) 
					{ 
						return (c.iMonsterOrNpc == 1
							|| (c.iProtoID >= 19067 && c.iProtoID <= 19069)
							|| (c.iProtoID >= 19070 && c.iProtoID <= 19072))
							&& c.iProtoID != 9009
							&& c.eState != PSA_DYING 
							&& c.eState != PSA_DEATH 
							&& std::count(vecSelectedNpcList.begin(), vecSelectedNpcList.end(), c.iProtoID)
							&& Client::GetDistance(c.fX, c.fY) <= (float)MAX_ATTACK_RANGE;
					});
			}

			auto pSort = [](TNpc const& a, TNpc const& b)
			{
				return Client::GetDistance(a.fX, a.fY) < Client::GetDistance(b.fX, b.fY);
			};

			std::sort(vecFilteredNpc.begin(), vecFilteredNpc.end(), pSort);

			if (vecFilteredNpc.size() > 0)
			{
				auto pFindedTarget = vecFilteredNpc.at(0);
				
				Client::SetTarget(pFindedTarget.iID);
#ifdef _DEBUG
				printf("SearchTargetProcess:: %d, Target Selected\n", pFindedTarget.iID);
#endif
			}
		}
		else
		{
			auto it = std::find_if(pNpcList.begin(), pNpcList.end(),
				[](const TNpc& a) 
				{ 
					return a.iID == Client::GetTarget(); 
				});

			if (it != pNpcList.end())
			{
				if (!bAutoTarget && !std::count(vecSelectedNpcList.begin(), vecSelectedNpcList.end(), it->iProtoID))
				{
#ifdef _DEBUG
					printf("SearchTargetProcess:: %d, Target not selected, selecting new target\n", it->iID);
#endif
					Client::SetTarget(-1);
				}

				if (Client::GetDistance(it->fX, it->fY) > (float)MAX_ATTACK_RANGE)
				{
#ifdef _DEBUG
					printf("SearchTargetProcess:: %d, Target out of range, selecting new target\n", it->iID);
#endif
					Client::SetTarget(-1);
				}

				if (it->eState == PSA_DYING || it->eState == PSA_DEATH)
				{
#ifdef _DEBUG
					printf("SearchTargetProcess:: %d, Target Dead\n", it->iID);
#endif
					Client::SetTarget(-1);
				}
			}
			else
			{
#ifdef _DEBUG
				printf("SearchTargetProcess:: %d, Target Lost\n", it->iID);
#endif
				Client::SetTarget(-1);
			}
		}
	}

#ifdef _DEBUG
	printf("AttackHandler::SearchTargetProcess Stopped\n");
#endif
}